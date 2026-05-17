// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_DATABASE_H
#define FS_DATABASE_H

#include "lua/env.h"
#include "pugicast.h"

class DBResult;

class Database
{
public:
	Database();
	~Database();

	// non-copyable, non-movable (singleton-friendly; instances may live as members of other classes)
	Database(const Database&) = delete;
	Database& operator=(const Database&) = delete;
	Database(Database&&) = delete;
	Database& operator=(Database&&) = delete;

	/**
	 * Singleton accessor.
	 *
	 * @return database connection handler singleton
	 */
	static Database& getInstance()
	{
		static Database instance;
		return instance;
	}

	/**
	 * Connects to the database.
	 *
	 * @return true on successful connection, false on error
	 */
	bool connect();

	/**
	 * Executes a command that does not produce a result set (INSERT, UPDATE, DELETE, ...).
	 *
	 * @param query command
	 * @return true on success, false on error
	 */
	bool executeQuery(const std::string& query);

	/**
	 * Executes a query that produces a result set (typically SELECT).
	 *
	 * @return results object (nullptr on error or empty result)
	 */
	std::shared_ptr<DBResult> storeQuery(std::string_view query);

	/**
	 * Escapes a string for inclusion in a SQL query, returning the escaped value wrapped in
	 * single quotes.
	 */
	std::string escapeString(std::string_view s) const;

	/**
	 * Escapes a binary stream for inclusion in a SQL query, returning the escaped value wrapped
	 * in single quotes.
	 */
	std::string escapeBlob(const char* s, uint32_t length) const;

	/**
	 * @return id of the last inserted row, or 0 if the last query did not produce one
	 */
	uint64_t getLastInsertId() const;

	/**
	 * @return the database client library version string
	 */
	static const char* getClientVersion();

	uint64_t getMaxPacketSize() const;

private:
	/**
	 * Transaction control. Exposed to DBTransaction via friendship; not intended for direct use.
	 *
	 * @return true on success, false on error
	 */
	bool beginTransaction();
	bool rollback();
	bool commit();

	struct Impl;
	std::unique_ptr<Impl> impl_;

	friend class DBTransaction;
};

class DBResult
{
public:
	~DBResult();

	// non-copyable
	DBResult(const DBResult&) = delete;
	DBResult& operator=(const DBResult&) = delete;

	template <typename T>
	T getNumber(std::string_view column) const
	{
		if (const char* raw = tryGetColumn(column, "DBResult::getNumber")) {
			return pugi::cast<T>(raw);
		}
		return {};
	}

	std::chrono::system_clock::time_point getDateTime(std::string_view column) const;
	std::string_view getString(std::string_view column) const;

	bool hasNext() const;
	bool next();

private:
	struct Impl;
	explicit DBResult(std::unique_ptr<Impl> impl);

	/**
	 * Returns the raw C-string for `column`, or nullptr if the column is missing or its value is
	 * SQL NULL. Logs an error with the given `context` (typically "DBResult::<method>") when the
	 * column is not in the result set.
	 *
	 * Defined out-of-line so the template `getNumber<T>` does not pull MySQL internals into the
	 * public header.
	 */
	const char* tryGetColumn(std::string_view column, std::string_view context) const;

	std::unique_ptr<Impl> impl_;

	friend class Database;
};

/**
 * Multi-row INSERT statement builder. Buffers rows until either explicit execute() or the
 * accumulated query size exceeds the server's max_allowed_packet, at which point the buffer is
 * flushed and reset.
 */
class DBInsert
{
public:
	explicit DBInsert(std::string query);
	bool addRow(const std::string& row);
	bool addRow(std::ostringstream& row);
	bool execute();

private:
	std::string query;
	std::string values;
	size_t length;
};

class DBTransaction
{
public:
	constexpr DBTransaction() = default;

	~DBTransaction()
	{
		if (state == STATE_START) {
			Database::getInstance().rollback();
		}
	}

	// non-copyable
	DBTransaction(const DBTransaction&) = delete;
	DBTransaction& operator=(const DBTransaction&) = delete;

	bool begin()
	{
		state = STATE_START;
		return Database::getInstance().beginTransaction();
	}

	bool commit()
	{
		if (state != STATE_START) {
			return false;
		}

		state = STATE_COMMIT;
		return Database::getInstance().commit();
	}

private:
	enum TransactionStates_t
	{
		STATE_NO_START,
		STATE_START,
		STATE_COMMIT,
	};

	TransactionStates_t state = STATE_NO_START;
};

#endif // FS_DATABASE_H
