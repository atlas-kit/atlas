// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

// Boost.MySQL backend for the Database/DBResult layer. This is an alternative implementation of the
// same public API declared in database.h, selected at build time via the USE_BOOST_MYSQL CMake
// option. Exactly one of database.cpp (libmariadb/libmysql C API) or database_boost.cpp is
// compiled into the binary; the public surface is identical so callers never change.

#include "otpch.h"

#include "configmanager.h"
#include "database.h"

// Codacy's cppcheck cannot resolve the Boost.MySQL/Asio headers in its analysis sandbox and emits
// missingIncludeSystem for each. The headers are genuinely required and available at compile time;
// the warning is a tooling limitation ("Cppcheck does not need standard library headers to get
// proper results"). Suppress the false positive for this include block.
// cppcheck-suppress-begin missingIncludeSystem
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/mysql/any_connection.hpp>
#include <boost/mysql/blob_view.hpp>
#include <boost/mysql/client_errc.hpp>
#include <boost/mysql/connect_params.hpp>
#include <boost/mysql/diagnostics.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/field_kind.hpp>
#include <boost/mysql/field_view.hpp>
#include <boost/mysql/format_sql.hpp>
#include <boost/mysql/metadata_mode.hpp>
#include <boost/mysql/results.hpp>
#include <boost/mysql/row_view.hpp>
#include <boost/mysql/ssl_mode.hpp>
// cppcheck-suppress-end missingIncludeSystem

namespace {

namespace mysql = boost::mysql;

// Connection-level (network) errors that justify a reconnect+retry, mirroring the
// CR_SERVER_LOST / CR_SERVER_GONE_ERROR handling of the C API backend. SQL errors (bad query,
// constraint violation, ...) are reported to the caller as-is and never retried.
bool isConnectionError(const boost::system::error_code& ec)
{
	return ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset ||
	       ec == boost::asio::error::connection_aborted || ec == boost::asio::error::broken_pipe ||
	       ec == boost::asio::error::not_connected || ec == boost::asio::error::operation_aborted ||
	       ec == boost::asio::error::network_down || ec == boost::asio::error::network_reset ||
	       ec == boost::asio::error::timed_out || ec == mysql::client_errc::server_unsupported;
}

} // namespace

struct Database::Impl
{
	Impl() : conn(ctx) { conn.set_meta_mode(mysql::metadata_mode::full); }

	boost::asio::io_context ctx;
	mysql::any_connection conn;
	std::recursive_mutex databaseLock;
	uint64_t maxPacketSize = 1048576;
	// Do not retry queries while in the middle of a transaction.
	bool retryQueries = true;
	uint64_t lastInsertId = 0;
};

struct DBResult::Impl
{
	mysql::results result;
	std::size_t columnCount = 0;
	std::size_t rowCount = 0;
	std::size_t rowIndex = 0;
	// The keys are std::string_view into the column-name buffers owned by `result` (retained
	// because the connection runs with metadata_mode::full). They stay valid only while `result`
	// is alive, so `result` must outlive `listNames` and must not be reassigned while this result
	// is in use. Declaration order (result before listNames) gives the correct destruction order;
	// do not reorder.
	std::map<std::string_view, std::size_t> listNames;
	// Stable, owning storage for the current row's values, rebuilt on construction and on next().
	// `tryGetColumn` returns `.c_str()` into `cells`; `getString` returns a binary-safe view over
	// it (std::string preserves embedded NUL bytes).
	std::vector<std::string> cells;
	std::vector<bool> cellNull;

	void buildCurrentRow();
};

void DBResult::Impl::buildCurrentRow()
{
	cells.assign(columnCount, std::string{});
	cellNull.assign(columnCount, true);
	if (rowIndex >= rowCount) {
		return;
	}

	mysql::row_view row = result.rows().at(rowIndex);
	for (std::size_t i = 0; i < columnCount; ++i) {
		mysql::field_view field = row.at(i);
		std::string& out = cells[i];
		switch (field.kind()) {
			case mysql::field_kind::null:
				cellNull[i] = true;
				break;
			case mysql::field_kind::int64:
				out = std::to_string(field.as_int64());
				cellNull[i] = false;
				break;
			case mysql::field_kind::uint64:
				out = std::to_string(field.as_uint64());
				cellNull[i] = false;
				break;
			case mysql::field_kind::string: {
				auto sv = field.as_string();
				out.assign(sv.data(), sv.size());
				cellNull[i] = false;
				break;
			}
			case mysql::field_kind::blob: {
				auto bv = field.as_blob();
				out.assign(reinterpret_cast<const char*>(bv.data()), bv.size());
				cellNull[i] = false;
				break;
			}
			default: {
				// float / double / date / datetime / time: the documented stream formatting
				// matches MySQL's textual representation, which is what the C API backend (text
				// protocol) would have returned for these columns.
				std::ostringstream oss;
				oss << field;
				out = oss.str();
				cellNull[i] = false;
				break;
			}
		}
	}
}

namespace {

// Establishes (or re-establishes) the connection. Mirrors connectToDatabase() in the C API
// backend: blocks, optionally retrying forever with a 1s backoff between attempts.
//
// Operates on the connection object rather than on Database::Impl, because Database::Impl is a
// private nested type and cannot be named by namespace-scope free functions (the C API backend
// passes the raw handle for the same reason).
bool connectToDatabase(mysql::any_connection& conn, const bool retryIfError, const bool isReconnect)
{
	bool isFirstAttemptToConnect = true;
	bool reconnecting = isReconnect;

	for (;;) {
		if (!isFirstAttemptToConnect) {
			std::this_thread::sleep_for(1s);
		}
		isFirstAttemptToConnect = false;

		// On a reconnect the socket may still be half-open; close it first, ignoring errors.
		if (reconnecting) {
			boost::system::error_code closeEc;
			mysql::diagnostics closeDiag;
			conn.close(closeEc, closeDiag);
		}
		reconnecting = true;

		const std::string& socket = getString(ConfigManager::MYSQL_SOCK);

		mysql::connect_params params;
		if (!socket.empty()) {
			params.server_address.emplace_unix_path(socket);
		} else {
			const std::string& host = getString(ConfigManager::MYSQL_HOST);
			const auto port = static_cast<uint16_t>(getNumber(ConfigManager::SQL_PORT));
			params.server_address.emplace_host_and_port(host, port);
		}
		params.username = getString(ConfigManager::MYSQL_USER);
		params.password = getString(ConfigManager::MYSQL_PASS);
		params.database = getString(ConfigManager::MYSQL_DB);
		// Opportunistic TLS without certificate verification, matching the C API backend's
		// ssl_enforce=false / ssl_verify=false behavior.
		params.ssl = mysql::ssl_mode::enable;
		params.multi_queries = false;

		boost::system::error_code ec;
		mysql::diagnostics diag;
		conn.connect(params, ec, diag);
		if (!ec) {
			return true;
		}

		std::cout << std::endl << "MySQL Error Message: " << ec.message();
		if (!diag.server_message().empty()) {
			std::cout << " (" << diag.server_message() << ')';
		}
		std::cout << std::endl;

		if (!retryIfError) {
			return false;
		}
	}
}

// Executes a statement, transparently reconnecting and retrying on connection-level failures when
// retryIfLostConnection is set. Returns false for SQL errors (which the caller surfaces). The
// caller (a Database member) is responsible for updating Database::Impl::lastInsertId from
// `out.last_insert_id()`, since Database::Impl is private to this free function.
bool runStatement(mysql::any_connection& conn, std::string_view query, mysql::results& out,
                  const bool retryIfLostConnection)
{
	for (;;) {
		boost::system::error_code ec;
		mysql::diagnostics diag;
		conn.execute(query, out, ec, diag);
		if (!ec) {
			return true;
		}

		std::cout << "[Error - Database::execute] Query: " << query.substr(0, 256) << std::endl
		          << "Message: " << ec.message();
		if (!diag.server_message().empty()) {
			std::cout << " (" << diag.server_message() << ')';
		}
		std::cout << std::endl;

		if (!isConnectionError(ec) || !retryIfLostConnection) {
			return false;
		}
		if (!connectToDatabase(conn, true, true)) {
			return false;
		}
	}
}

} // namespace

Database::Database() : impl_(std::make_unique<Impl>()) {}

Database::~Database() = default;

bool Database::connect()
{
	if (!connectToDatabase(impl_->conn, false, false)) {
		return false;
	}

	if (const auto& result = storeQuery("SHOW VARIABLES LIKE 'max_allowed_packet'")) {
		impl_->maxPacketSize = result->getNumber<uint64_t>("Value");
	}
	return true;
}

bool Database::beginTransaction()
{
	impl_->databaseLock.lock();
	const bool result = executeQuery("START TRANSACTION");
	impl_->retryQueries = !result;
	if (!result) {
		impl_->databaseLock.unlock();
	}
	return result;
}

bool Database::rollback()
{
	const bool result = executeQuery("ROLLBACK");
	impl_->retryQueries = true;
	impl_->databaseLock.unlock();
	return result;
}

bool Database::commit()
{
	const bool result = executeQuery("COMMIT");
	impl_->retryQueries = true;
	impl_->databaseLock.unlock();
	return result;
}

bool Database::executeQuery(const std::string& query)
{
	std::lock_guard<std::recursive_mutex> lockGuard(impl_->databaseLock);
	mysql::results result;
	if (!runStatement(impl_->conn, query, result, impl_->retryQueries)) {
		return false;
	}
	// Mirror mysql_insert_id(): only statements that generate an AUTO_INCREMENT value update the
	// remembered id, so a later SELECT does not clobber it before getLastInsertId() is read.
	if (const auto id = result.last_insert_id(); id != 0) {
		impl_->lastInsertId = id;
	}
	return true;
}

std::shared_ptr<DBResult> Database::storeQuery(std::string_view query)
{
	std::lock_guard<std::recursive_mutex> lockGuard(impl_->databaseLock);

	auto resultImpl = std::make_unique<DBResult::Impl>();
	if (!runStatement(impl_->conn, query, resultImpl->result, impl_->retryQueries)) {
		return nullptr;
	}
	if (const auto id = resultImpl->result.last_insert_id(); id != 0) {
		impl_->lastInsertId = id;
	}

	const auto meta = resultImpl->result.meta();
	resultImpl->columnCount = meta.size();
	for (std::size_t i = 0; i < meta.size(); ++i) {
		resultImpl->listNames.emplace(meta[i].column_name(), i);
	}
	resultImpl->rowCount = resultImpl->result.rows().size();
	resultImpl->rowIndex = 0;

	// std::make_shared would require a public constructor; keep it private (Database is the only
	// friend able to fabricate DBResult) at the cost of one extra allocation.
	std::shared_ptr<DBResult> result{new DBResult(std::move(resultImpl))};
	if (result->hasNext()) {
		return result;
	}
	return nullptr;
}

std::string Database::escapeString(std::string_view s) const
{
	const auto opts = impl_->conn.format_opts();
	if (!opts.has_value()) {
		return "''";
	}

	// Prefer a charset-correct quoted string literal so text columns keep collation-aware
	// comparison semantics (player/account name lookups, etc.). Boost.MySQL rejects values that
	// are not valid in the connection character set, but the codebase also routes binary data
	// (session tokens, packed IPs, ...) through escapeString, relying on the C client's byte-wise
	// mysql_real_escape_string. For such data fall back to a binary-safe hex literal, which
	// round-trips identically through BINARY/VARBINARY/BLOB columns. The error is reported via
	// the result of get() rather than thrown.
	mysql::format_context ctx(*opts);
	ctx.append_value(s);
	if (auto formatted = std::move(ctx).get()) {
		return *formatted;
	}
	return escapeBlob(s.data(), s.size());
}

std::string Database::escapeBlob(const char* s, uint32_t length) const
{
	const auto opts = impl_->conn.format_opts();
	if (!opts.has_value()) {
		return "''";
	}
	// Binary data may contain bytes that are not valid in the connection character set, which
	// would make string formatting fail. A blob is rendered as a hex literal (x'...'), which is
	// binary-safe and round-trips identically through BLOB columns.
	const mysql::blob_view blob{reinterpret_cast<const unsigned char*>(s), length};
	return mysql::format_sql(*opts, "{}", blob);
}

uint64_t Database::getLastInsertId() const { return impl_->lastInsertId; }

const char* Database::getClientVersion() { return "Boost.MySQL"; }

uint64_t Database::getMaxPacketSize() const { return impl_->maxPacketSize; }

DBResult::DBResult(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) { impl_->buildCurrentRow(); }

DBResult::~DBResult() = default;

const char* DBResult::tryGetColumn(std::string_view column, std::string_view context) const
{
	auto it = impl_->listNames.find(column);
	if (it == impl_->listNames.end()) {
		std::cout << "[Error - " << context << "] Column '" << column << "' doesn't exist in the result set"
		          << std::endl;
		return nullptr;
	}
	if (impl_->cellNull[it->second]) {
		return nullptr;
	}
	return impl_->cells[it->second].c_str();
}

std::chrono::system_clock::time_point DBResult::getDateTime(std::string_view column) const
{
	return std::chrono::system_clock::time_point{std::chrono::seconds{getNumber<int64_t>(column)}};
}

std::string_view DBResult::getString(std::string_view column) const
{
	auto it = impl_->listNames.find(column);
	if (it == impl_->listNames.end()) {
		std::cout << "[Error - DBResult::getString] Column '" << column << "' does not exist in result set."
		          << std::endl;
		return {};
	}

	if (impl_->cellNull[it->second]) {
		return {};
	}

	return std::string_view{impl_->cells[it->second]};
}

bool DBResult::hasNext() const { return impl_->rowIndex < impl_->rowCount; }

bool DBResult::next()
{
	if (impl_->rowIndex >= impl_->rowCount) {
		return false;
	}

	++impl_->rowIndex;
	if (impl_->rowIndex >= impl_->rowCount) {
		return false;
	}

	impl_->buildCurrentRow();
	return true;
}

DBInsert::DBInsert(std::string query) : query(std::move(query)) { this->length = this->query.length(); }

bool DBInsert::addRow(const std::string& row)
{
	// adds new row to buffer
	const size_t rowLength = row.length();
	// Flush the buffer before adding this row if it would push the statement past the packet
	// limit. execute() resets length to query.length(), so the current row must be accounted for
	// *after* the potential flush; incrementing before the check loses this row's bytes from the
	// running total whenever a flush happens, letting later checks undercount.
	if (length + rowLength > Database::getInstance().getMaxPacketSize() && !execute()) {
		return false;
	}
	length += rowLength;

	if (values.empty()) {
		values.reserve(rowLength + 2);
		values.push_back('(');
		values.append(row);
		values.push_back(')');
	} else {
		values.reserve(values.length() + rowLength + 3);
		values.push_back(',');
		values.push_back('(');
		values.append(row);
		values.push_back(')');
	}
	return true;
}

bool DBInsert::addRow(std::ostringstream& row)
{
	bool ret = addRow(row.str());
	row.str(std::string());
	return ret;
}

bool DBInsert::execute()
{
	if (values.empty()) {
		return true;
	}

	// executes buffer
	bool res = Database::getInstance().executeQuery(query + values);
	values.clear();
	length = query.length();
	return res;
}
