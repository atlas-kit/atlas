// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "database.h"

#include "configmanager.h"

#if __has_include(<mariadb/errmsg.h>)
#include <mariadb/errmsg.h>
#else
#include <mysql/errmsg.h>
#endif

namespace tfs::detail {

struct MysqlDeleter
{
	void operator()(MYSQL* handle) const { mysql_close(handle); }
	void operator()(MYSQL_RES* handle) const { mysql_free_result(handle); }
};

using Mysql_ptr = std::unique_ptr<MYSQL, MysqlDeleter>;
using MysqlResult_ptr = std::unique_ptr<MYSQL_RES, MysqlDeleter>;

} // namespace tfs::detail

struct Database::Impl
{
	tfs::detail::Mysql_ptr handle = nullptr;
	std::recursive_mutex databaseLock;
	uint64_t maxPacketSize = 1048576;
	// Do not retry queries while in the middle of a transaction.
	bool retryQueries = true;
};

struct DBResult::Impl
{
	tfs::detail::MysqlResult_ptr handle;
	MYSQL_ROW row = nullptr;
	// The keys are std::string_view into MYSQL_FIELD::name buffers owned by `handle`. They stay
	// valid only while `handle` is alive, so `handle` must outlive `listNames` and must not be
	// reset while this result is in use. Declaration order (handle before listNames) gives the
	// correct destruction order; do not reorder. A future backend that cannot guarantee stable
	// field-name storage should switch these to owning std::string keys.
	std::map<std::string_view, size_t> listNames;
};

static tfs::detail::Mysql_ptr connectToDatabase(const bool retryIfError)
{
	bool isFirstAttemptToConnect = true;

retry:
	if (!isFirstAttemptToConnect) {
		std::this_thread::sleep_for(1s);
	}
	isFirstAttemptToConnect = false;

// MariaDB requires explicit SSL settings to avoid the following error:
// "SSL is required, but the server does not support it"
// For more details see issue #4954 ( https://github.com/otland/forgottenserver/issues/4954 )
#ifdef MARIADB_VERSION_ID
	// this needs to be above "goto" otherwise it won't build
	bool ssl_enforce = false;
	bool ssl_verify = false;
#endif

	tfs::detail::Mysql_ptr handle{mysql_init(nullptr)};
	if (!handle) {
		std::cout << std::endl << "Failed to initialize MySQL connection handle." << std::endl;
		goto error;
	}

// MariaDB explicit SSL settings continued
#ifdef MARIADB_VERSION_ID
	mysql_options(handle.get(), MYSQL_OPT_SSL_ENFORCE, &ssl_enforce);
	mysql_options(handle.get(), MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_verify);
	mysql_ssl_set(handle.get(), nullptr, nullptr, nullptr, nullptr, nullptr);
#endif

	// connects to database
	if (!mysql_real_connect(handle.get(), getString(ConfigManager::MYSQL_HOST).c_str(),
	                        getString(ConfigManager::MYSQL_USER).c_str(), getString(ConfigManager::MYSQL_PASS).c_str(),
	                        getString(ConfigManager::MYSQL_DB).c_str(), getNumber(ConfigManager::SQL_PORT),
	                        getString(ConfigManager::MYSQL_SOCK).c_str(), 0)) {
		std::cout << std::endl << "MySQL Error Message: " << mysql_error(handle.get()) << std::endl;
		goto error;
	}
	return handle;

error:
	if (retryIfError) {
		goto retry;
	}
	return nullptr;
}

static bool isLostConnectionError(const unsigned error)
{
	return error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR || error == CR_CONN_HOST_ERROR ||
	       error == 1053 /*ER_SERVER_SHUTDOWN*/ || error == CR_CONNECTION_ERROR;
}

static bool executeQuery(tfs::detail::Mysql_ptr& handle, std::string_view query, const bool retryIfLostConnection)
{
	while (mysql_real_query(handle.get(), query.data(), query.length()) != 0) {
		std::cout << "[Error - mysql_real_query] Query: " << query.substr(0, 256) << std::endl
		          << "Message: " << mysql_error(handle.get()) << std::endl;
		const unsigned error = mysql_errno(handle.get());
		if (!isLostConnectionError(error) || !retryIfLostConnection) {
			return false;
		}
		handle = connectToDatabase(true);
	}
	return true;
}

Database::Database() : impl_(std::make_unique<Impl>()) {}

Database::~Database() = default;

bool Database::connect()
{
	auto newHandle = connectToDatabase(false);
	if (!newHandle) {
		return false;
	}

	impl_->handle = std::move(newHandle);
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
	auto success = ::executeQuery(impl_->handle, query, impl_->retryQueries);

	// executeQuery can be called with a command that produces a result (e.g. SELECT). We have to
	// store that result, even though we do not need it, otherwise the handle will get blocked.
	auto mysql_res = mysql_store_result(impl_->handle.get());
	mysql_free_result(mysql_res);

	return success;
}

std::shared_ptr<DBResult> Database::storeQuery(std::string_view query)
{
	std::lock_guard<std::recursive_mutex> lockGuard(impl_->databaseLock);

retry:
	if (!::executeQuery(impl_->handle, query, impl_->retryQueries) && !impl_->retryQueries) {
		return nullptr;
	}

	// we should call that every time as someone would call executeQuery('SELECT...')
	// as it is described in MySQL manual: "it doesn't hurt" :P
	tfs::detail::MysqlResult_ptr res{mysql_store_result(impl_->handle.get())};
	if (!res) {
		std::cout << "[Error - mysql_store_result] Query: " << query << std::endl
		          << "Message: " << mysql_error(impl_->handle.get()) << std::endl;
		const unsigned error = mysql_errno(impl_->handle.get());
		if (!isLostConnectionError(error) || !impl_->retryQueries) {
			return nullptr;
		}
		goto retry;
	}

	auto resultImpl = std::make_unique<DBResult::Impl>();
	resultImpl->handle = std::move(res);

	size_t i = 0;
	MYSQL_FIELD* field = mysql_fetch_field(resultImpl->handle.get());
	while (field) {
		resultImpl->listNames[field->name] = i++;
		field = mysql_fetch_field(resultImpl->handle.get());
	}
	resultImpl->row = mysql_fetch_row(resultImpl->handle.get());

	// `std::make_shared<DBResult>` would require a publicly accessible constructor. Keeping the
	// constructor private (with `Database` as the only friend able to invoke it) means we pay one
	// extra allocation in exchange for not exposing a way to fabricate `DBResult` instances from
	// outside this translation unit.
	std::shared_ptr<DBResult> result{new DBResult(std::move(resultImpl))};
	if (result->hasNext()) {
		return result;
	}
	return nullptr;
}

std::string Database::escapeString(std::string_view s) const { return escapeBlob(s.data(), s.length()); }

std::string Database::escapeBlob(const char* s, uint32_t length) const
{
	// the worst case is 2n + 1
	size_t maxLength = (length * 2) + 1;

	std::string escaped;
	escaped.reserve(maxLength + 2);
	escaped.push_back('\'');

	if (length != 0) {
		char* output = new char[maxLength];
		mysql_real_escape_string(impl_->handle.get(), output, s, length);
		escaped.append(output);
		delete[] output;
	}

	escaped.push_back('\'');
	return escaped;
}

uint64_t Database::getLastInsertId() const { return static_cast<uint64_t>(mysql_insert_id(impl_->handle.get())); }

const char* Database::getClientVersion() { return mysql_get_client_info(); }

uint64_t Database::getMaxPacketSize() const { return impl_->maxPacketSize; }

DBResult::DBResult(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

DBResult::~DBResult() = default;

const char* DBResult::tryGetColumn(std::string_view column, std::string_view context) const
{
	auto it = impl_->listNames.find(column);
	if (it == impl_->listNames.end()) {
		std::cout << "[Error - " << context << "] Column '" << column << "' doesn't exist in the result set"
		          << std::endl;
		return nullptr;
	}
	return impl_->row[it->second];
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

	if (!impl_->row[it->second]) {
		return {};
	}

	auto size = mysql_fetch_lengths(impl_->handle.get())[it->second];
	return {impl_->row[it->second], size};
}

bool DBResult::hasNext() const { return impl_->row; }

bool DBResult::next()
{
	impl_->row = mysql_fetch_row(impl_->handle.get());
	return impl_->row;
}

DBInsert::DBInsert(std::string query) : query(std::move(query)) { this->length = this->query.length(); }

bool DBInsert::addRow(const std::string& row)
{
	// adds new row to buffer
	const size_t rowLength = row.length();
	length += rowLength;
	if (length > Database::getInstance().getMaxPacketSize() && !execute()) {
		return false;
	}

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
