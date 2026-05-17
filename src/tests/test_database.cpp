#define BOOST_TEST_MODULE database

#include "../otpch.h"

#include "../configmanager.h"
#include "../database.h"

// cppcheck-suppress missingIncludeSystem
#include <boost/test/unit_test.hpp>

#include <stdexcept>

// Most tests use a top-level DBTransaction that is never committed, so its
// destructor rolls back every change the test performed. Tests that need to
// exercise DBTransaction commit/rollback semantics directly use the
// `DatabaseNoTxFixture` below, which performs explicit cleanup instead.
struct DatabaseFixture
{
	DatabaseFixture()
	{
		setString(ConfigManager::MYSQL_HOST, "0.0.0.0");
		setString(ConfigManager::MYSQL_USER, "atlas");
		setString(ConfigManager::MYSQL_PASS, "atlas");
		setString(ConfigManager::MYSQL_DB, "atlas");
		setNumber(ConfigManager::SQL_PORT, 3306);

		if (!db.connect()) {
			throw std::runtime_error("DatabaseFixture: failed to connect to database");
		}
		if (!transaction.begin()) {
			throw std::runtime_error("DatabaseFixture: failed to start transaction");
		}
	}

	Database& db = Database::getInstance();
	DBTransaction transaction;
};

struct DatabaseNoTxFixture
{
	DatabaseNoTxFixture()
	{
		setString(ConfigManager::MYSQL_HOST, "0.0.0.0");
		setString(ConfigManager::MYSQL_USER, "atlas");
		setString(ConfigManager::MYSQL_PASS, "atlas");
		setString(ConfigManager::MYSQL_DB, "atlas");
		setNumber(ConfigManager::SQL_PORT, 3306);

		if (!db.connect()) {
			throw std::runtime_error("DatabaseNoTxFixture: failed to connect to database");
		}
		cleanup();
	}

	~DatabaseNoTxFixture() { cleanup(); }

	void cleanup()
	{
		// Test fixtures use a unique account name prefix so we can wipe leftover rows
		// without touching any pre-existing data in the schema.
		db.executeQuery("DELETE FROM `accounts` WHERE `name` LIKE '__dbtest__%'");
	}

	Database& db = Database::getInstance();
};

BOOST_FIXTURE_TEST_SUITE(database_basic, DatabaseFixture)

BOOST_AUTO_TEST_CASE(execute_query_success)
{
	BOOST_TEST(db.executeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('exec_ok', 'exec_ok@example.com', SHA1('x'))"));
}

BOOST_AUTO_TEST_CASE(execute_query_invalid_sql_returns_false)
{
	// Malformed SQL must not crash and must report failure.
	BOOST_TEST(db.executeQuery("THIS IS NOT VALID SQL") == false);
}

BOOST_AUTO_TEST_CASE(store_query_empty_result_returns_nullptr)
{
	// Current behavior: a SELECT that yields no rows returns nullptr (not an empty result object).
	// The migration must preserve this contract — many call sites do `if (auto r = db.storeQuery(...))`.
	auto result = db.storeQuery("SELECT `id` FROM `accounts` WHERE `name` = 'this_account_does_not_exist'");
	BOOST_TEST(!result);
}

BOOST_AUTO_TEST_CASE(store_query_returns_result_with_rows)
{
	BOOST_TEST(db.executeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('store_q', 'store_q@example.com', SHA1('x'))"));

	auto result = db.storeQuery("SELECT `name`, `email` FROM `accounts` WHERE `name` = 'store_q'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getString("name") == "store_q");
	BOOST_TEST(result->getString("email") == "store_q@example.com");
}

BOOST_AUTO_TEST_CASE(get_last_insert_id_returns_inserted_row_id)
{
	BOOST_TEST(db.executeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('last_id', 'last_id@example.com', SHA1('x'))"));
	uint64_t insertId = db.getLastInsertId();
	BOOST_TEST(insertId > 0);

	auto result = db.storeQuery(std::format("SELECT `name` FROM `accounts` WHERE `id` = {:d}", insertId));
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getString("name") == "last_id");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(dbresult_column_access, DatabaseFixture)

BOOST_AUTO_TEST_CASE(get_number_various_integer_types)
{
	auto result = db.storeQuery(
	    "SELECT CAST(-2147483647 AS SIGNED) AS `i32`, CAST(4294967295 AS UNSIGNED) AS `u32`,"
	    " CAST(-9223372036854775807 AS SIGNED) AS `i64`, CAST(18446744073709551614 AS UNSIGNED) AS `u64`");
	BOOST_REQUIRE(result);

	BOOST_TEST(result->getNumber<int32_t>("i32") == -2147483647);
	BOOST_TEST(result->getNumber<uint32_t>("u32") == 4294967295u);
	BOOST_TEST(result->getNumber<int64_t>("i64") == -9223372036854775807LL);
	BOOST_TEST(result->getNumber<uint64_t>("u64") == 18446744073709551614ULL);
}

BOOST_AUTO_TEST_CASE(get_string_returns_value)
{
	auto result = db.storeQuery("SELECT 'hello world' AS `greeting`");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getString("greeting") == "hello world");
}

BOOST_AUTO_TEST_CASE(get_string_on_null_column_returns_empty)
{
	auto result = db.storeQuery("SELECT NULL AS `maybe`");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getString("maybe").empty());
}

BOOST_AUTO_TEST_CASE(get_number_on_null_column_returns_zero)
{
	auto result = db.storeQuery("SELECT NULL AS `maybe`");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getNumber<int32_t>("maybe") == 0);
	BOOST_TEST(result->getNumber<uint64_t>("maybe") == 0u);
}

BOOST_AUTO_TEST_CASE(missing_column_does_not_crash)
{
	auto result = db.storeQuery("SELECT 1 AS `present`");
	BOOST_REQUIRE(result);
	// Asking for a column that does not exist returns a default-constructed value
	// and logs an error. The contract for callers is "no crash".
	BOOST_TEST(result->getString("not_a_column").empty());
	BOOST_TEST(result->getNumber<int32_t>("not_a_column") == 0);
}

BOOST_AUTO_TEST_CASE(next_iterates_multiple_rows)
{
	BOOST_TEST(db.executeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES "
	    "('iter_a', 'iter_a@example.com', SHA1('x')),"
	    "('iter_b', 'iter_b@example.com', SHA1('x')),"
	    "('iter_c', 'iter_c@example.com', SHA1('x'))"));

	auto result = db.storeQuery("SELECT `name` FROM `accounts` WHERE `name` LIKE 'iter_%' ORDER BY `name`");
	BOOST_REQUIRE(result);

	BOOST_TEST(result->getString("name") == "iter_a");
	BOOST_TEST(result->next());
	BOOST_TEST(result->getString("name") == "iter_b");
	BOOST_TEST(result->next());
	BOOST_TEST(result->getString("name") == "iter_c");
	BOOST_TEST(result->next() == false);
	BOOST_TEST(result->hasNext() == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(database_escape, DatabaseFixture)

BOOST_AUTO_TEST_CASE(escape_string_quotes_and_special_chars_round_trip)
{
	// Strings with characters that would break a naive concatenation.
	const std::string payload = R"(He said "hello" and used a \backslash plus an 'apostrophe'.)";

	BOOST_TEST(db.executeQuery(std::format(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('escape_t', {:s}, SHA1('x'))",
	    db.escapeString(payload))));

	auto result = db.storeQuery("SELECT `email` FROM `accounts` WHERE `name` = 'escape_t'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getString("email") == payload);
}

BOOST_AUTO_TEST_CASE(escape_string_empty_round_trip)
{
	BOOST_TEST(db.executeQuery(std::format(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('escape_e', {:s}, SHA1('x'))",
	    db.escapeString(""))));

	auto result = db.storeQuery("SELECT `email` FROM `accounts` WHERE `name` = 'escape_e'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getString("email").empty());
}

BOOST_AUTO_TEST_CASE(escape_blob_with_null_bytes_round_trips_via_player_items)
{
	// The `player_items.attributes` column is a blob and is the typical consumer of escapeBlob.
	// We need a player to satisfy the foreign key, so create an account + player first.
	auto accountResult = db.storeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('blob_acc', 'blob_acc@example.com', SHA1('x')) RETURNING `id`");
	BOOST_REQUIRE(accountResult);
	auto accountId = accountResult->getNumber<uint64_t>("id");

	auto playerResult = db.storeQuery(std::format(
	    "INSERT INTO `players` (`account_id`, `name`) VALUES ({:d}, 'BlobPlayer') RETURNING `id`", accountId));
	BOOST_REQUIRE(playerResult);
	auto playerId = playerResult->getNumber<uint64_t>("id");

	const char binary[] = {0x00, 0x01, 0x02, '\'', '\\', '\n', 0x00, 'z', 0x7f};
	const uint32_t binaryLength = sizeof(binary);

	BOOST_TEST(db.executeQuery(std::format(
	    "INSERT INTO `player_items` (`player_id`, `pid`, `sid`, `itemtype`, `count`, `attributes`)"
	    " VALUES ({:d}, 0, 1, 100, 1, {:s})",
	    playerId, db.escapeBlob(binary, binaryLength))));

	auto result = db.storeQuery(
	    std::format("SELECT `attributes` FROM `player_items` WHERE `player_id` = {:d}", playerId));
	BOOST_REQUIRE(result);

	auto stored = result->getString("attributes");
	BOOST_REQUIRE_EQUAL(stored.size(), binaryLength);
	BOOST_TEST(stored == std::string_view(binary, binaryLength));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(database_dbinsert, DatabaseFixture)

BOOST_AUTO_TEST_CASE(dbinsert_single_row)
{
	DBInsert insert("INSERT INTO `accounts` (`name`, `email`, `password`) VALUES");
	BOOST_TEST(insert.addRow("'ins_one', 'ins_one@example.com', SHA1('x')"));
	BOOST_TEST(insert.execute());

	auto result = db.storeQuery("SELECT COUNT(*) AS `n` FROM `accounts` WHERE `name` = 'ins_one'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getNumber<int>("n") == 1);
}

BOOST_AUTO_TEST_CASE(dbinsert_multiple_rows_one_statement)
{
	DBInsert insert("INSERT INTO `accounts` (`name`, `email`, `password`) VALUES");
	BOOST_TEST(insert.addRow("'ins_m1', 'm1@example.com', SHA1('x')"));
	BOOST_TEST(insert.addRow("'ins_m2', 'm2@example.com', SHA1('x')"));
	BOOST_TEST(insert.addRow("'ins_m3', 'm3@example.com', SHA1('x')"));
	BOOST_TEST(insert.execute());

	auto result = db.storeQuery("SELECT COUNT(*) AS `n` FROM `accounts` WHERE `name` LIKE 'ins_m%'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getNumber<int>("n") == 3);
}

BOOST_AUTO_TEST_CASE(dbinsert_addrow_ostringstream_overload)
{
	DBInsert insert("INSERT INTO `accounts` (`name`, `email`, `password`) VALUES");
	std::ostringstream row;
	row << "'ins_oss', 'oss@example.com', SHA1('x')";
	BOOST_TEST(insert.addRow(row));
	// Stream must be cleared after addRow consumes it.
	BOOST_TEST(row.str().empty());
	BOOST_TEST(insert.execute());

	auto result = db.storeQuery("SELECT `email` FROM `accounts` WHERE `name` = 'ins_oss'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getString("email") == "oss@example.com");
}

BOOST_AUTO_TEST_CASE(dbinsert_execute_on_empty_buffer_is_noop)
{
	DBInsert insert("INSERT INTO `accounts` (`name`, `email`, `password`) VALUES");
	// No rows added; execute should be a no-op and return success.
	BOOST_TEST(insert.execute());
}

BOOST_AUTO_TEST_CASE(dbinsert_reports_cached_max_packet_size)
{
	// Sanity: connect() must have populated maxPacketSize from `SHOW VARIABLES LIKE 'max_allowed_packet'`.
	// DBInsert relies on this value to decide when to auto-flush mid-stream.
	BOOST_TEST(db.getMaxPacketSize() > 0);
}

BOOST_AUTO_TEST_SUITE_END()

// Transaction commit/rollback semantics need to run outside an enclosing transaction, otherwise
// nested BEGIN would implicitly commit the outer one (MariaDB does not support real nesting).
BOOST_FIXTURE_TEST_SUITE(database_transaction, DatabaseNoTxFixture)

BOOST_AUTO_TEST_CASE(transaction_commit_persists_changes)
{
	{
		DBTransaction tx;
		BOOST_REQUIRE(tx.begin());
		BOOST_TEST(db.executeQuery(
		    "INSERT INTO `accounts` (`name`, `email`, `password`)"
		    " VALUES ('__dbtest__commit', 'commit@example.com', SHA1('x'))"));
		BOOST_REQUIRE(tx.commit());
	}

	auto result =
	    db.storeQuery("SELECT COUNT(*) AS `n` FROM `accounts` WHERE `name` = '__dbtest__commit'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getNumber<int>("n") == 1);
}

BOOST_AUTO_TEST_CASE(transaction_destructor_rolls_back_without_commit)
{
	{
		DBTransaction tx;
		BOOST_REQUIRE(tx.begin());
		BOOST_TEST(db.executeQuery(
		    "INSERT INTO `accounts` (`name`, `email`, `password`)"
		    " VALUES ('__dbtest__rollback', 'rb@example.com', SHA1('x'))"));
		// tx goes out of scope without commit -- destructor must rollback.
	}

	auto result =
	    db.storeQuery("SELECT COUNT(*) AS `n` FROM `accounts` WHERE `name` = '__dbtest__rollback'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getNumber<int>("n") == 0);
}

BOOST_AUTO_TEST_CASE(transaction_commit_after_destruction_path_does_not_double_rollback)
{
	// After commit(), the destructor must not attempt to rollback (state moved to STATE_COMMIT).
	// Insert + commit, then immediately verify the row is visible from a second connection-less
	// query through the same singleton. If the destructor were to rollback erroneously we would
	// not see the row.
	{
		DBTransaction tx;
		BOOST_REQUIRE(tx.begin());
		BOOST_TEST(db.executeQuery(
		    "INSERT INTO `accounts` (`name`, `email`, `password`)"
		    " VALUES ('__dbtest__commit2', 'c2@example.com', SHA1('x'))"));
		BOOST_REQUIRE(tx.commit());

		auto result =
		    db.storeQuery("SELECT COUNT(*) AS `n` FROM `accounts` WHERE `name` = '__dbtest__commit2'");
		BOOST_REQUIRE(result);
		BOOST_TEST(result->getNumber<int>("n") == 1);
	}

	auto result =
	    db.storeQuery("SELECT COUNT(*) AS `n` FROM `accounts` WHERE `name` = '__dbtest__commit2'");
	BOOST_REQUIRE(result);
	BOOST_TEST(result->getNumber<int>("n") == 1);
}

BOOST_AUTO_TEST_SUITE_END()
