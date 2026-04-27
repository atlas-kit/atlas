#define BOOST_TEST_MODULE http_login

#include "../../otpch.h"

#include "../../base64.h"
#include "../../database.h"
#include "../../tools.h"
#include "../../vocation.h"
#include "../login.h"

#include <boost/test/unit_test.hpp>

extern Vocations g_vocations;

void setupVocations()
{
	struct VocData
	{
		uint16_t id;
		uint8_t clientId;
		const char* name;
		const char* description;
	};
	static constexpr VocData vocations[] = {
	    {0, 0, "None", "none"},
	    {1, 3, "Sorcerer", "a sorcerer"},
	    {2, 4, "Druid", "a druid"},
	    {3, 2, "Paladin", "a paladin"},
	    {4, 1, "Knight", "a knight"},
	    {5, 13, "Master Sorcerer", "a master sorcerer"},
	    {6, 14, "Elder Druid", "an elder druid"},
	    {7, 12, "Royal Paladin", "a royal paladin"},
	    {8, 11, "Elite Knight", "an elite knight"},
	};
	for (const auto& v : vocations) {
		auto& voc = g_vocations.registerVocation(v.id);
		voc.clientId = v.clientId;
		voc.name = v.name;
		voc.description = v.description;
	}
}

using namespace std::chrono;

struct LoginFixture
{
	LoginFixture()
	{
		setString(ConfigManager::SERVER_NAME, "Atlas");
		setString(ConfigManager::IP, "tfs.example.com");
		setNumber(ConfigManager::GAME_PORT, 7171);
		setString(ConfigManager::LOCATION, "Sweden");
		setString(ConfigManager::WORLD_TYPE, "pvp");

		setString(ConfigManager::MYSQL_HOST, "0.0.0.0");
		setString(ConfigManager::MYSQL_USER, "atlas");
		setString(ConfigManager::MYSQL_PASS, "atlas");
		setString(ConfigManager::MYSQL_DB, "atlas");
		setNumber(ConfigManager::SQL_PORT, 3306);

		setupVocations();

		db.connect();
		transaction.begin();
	}

	~LoginFixture()
	{
		// `players_online` is a memory table and does not support transactions, so we need to clear it manually
		// do NOT run this test against a running server's database
		db.executeQuery("TRUNCATE `players_online`");
	}

	Database& db = Database::getInstance();
	DBTransaction transaction;

	std::string_view ip = "74.125.224.72";
	seconds now = duration_cast<seconds>(system_clock::now().time_since_epoch());
};

using status = boost::beast::http::status;

BOOST_FIXTURE_TEST_CASE(test_login_missing_email, LoginFixture)
{
	auto&& [status, body] = tfs::http::handle_login({{"type", "login"}, {"password", "bar"}}, ip);

	BOOST_TEST(status == status::ok);
	BOOST_TEST(body.at("errorCode").as_int64() == 3);
}

BOOST_FIXTURE_TEST_CASE(test_login_account_does_not_exist, LoginFixture)
{
	auto&& [status, body] =
	    tfs::http::handle_login({{"type", "login"}, {"email", "k@example.com"}, {"password", "bar"}}, ip);

	BOOST_TEST(status == status::ok);
	BOOST_TEST(body.at("errorCode").as_int64() == 3);
}

BOOST_FIXTURE_TEST_CASE(test_login_missing_password, LoginFixture)
{
	auto&& [status, body] = tfs::http::handle_login({{"type", "login"}, {"email", "foo@example.com"}}, ip);

	BOOST_TEST(status == status::ok);
	BOOST_TEST(body.at("errorCode").as_int64() == 3);
}

BOOST_FIXTURE_TEST_CASE(test_login_invalid_password, LoginFixture)
{
	BOOST_TEST(db.executeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('abc', 'foo@example.com', SHA1('bar'))"));

	auto&& [status, body] =
	    tfs::http::handle_login({{"type", "login"}, {"email", "foo@example.com"}, {"password", "baz"}}, ip);

	BOOST_TEST(status == status::ok);
	BOOST_TEST(body.at("errorCode").as_int64() == 3);
}

BOOST_FIXTURE_TEST_CASE(test_login_missing_token, LoginFixture)
{
	BOOST_TEST(db.executeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`, `secret`) VALUES ('abcd', 'fooba@example.com', SHA1('bar'), UNHEX('48656c6c6f21dead'))"));

	auto&& [status, body] = tfs::http::handle_login(
	    {
	        {"type", "login"},
	        {"email", "fooba@example.com"},
	        {"password", "bar"},
	    },
	    ip);

	BOOST_TEST(status == status::ok);
	BOOST_TEST(body.at("errorCode").as_int64() == 6);
}

BOOST_FIXTURE_TEST_CASE(test_login_success_no_players, LoginFixture)
{
	BOOST_TEST(db.executeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`) VALUES ('defg', 'foobar@example.com', SHA1('bar'))"));

	auto&& [status, body] =
	    tfs::http::handle_login({{"type", "login"}, {"email", "foobar@example.com"}, {"password", "bar"}}, ip);

	BOOST_TEST(status == status::ok);
	auto& characters = body.at("playdata").at("characters").as_array();
	BOOST_TEST(characters.size() == 0);
}

BOOST_FIXTURE_TEST_CASE(test_login_success, LoginFixture)
{
	auto premiumEndsAt = now + days(30);

	auto result = db.storeQuery(std::format(
	    "INSERT INTO `accounts` (`name`, `email`, `password`, `premium_ends_at`) VALUES ('ghij', 'ghij@example.com', SHA1('bar'), {:d}) RETURNING `id`",
	    premiumEndsAt.count()));
	auto id = result->getNumber<uint64_t>("id");

	DBInsert insert(
	    "INSERT INTO `players` (`account_id`, `name`, `level`, `vocation`, `lastlogin`, `sex`, `looktype`, `lookhead`, `lookbody`, `looklegs`, `lookfeet`, `lookaddons`) VALUES");
	insert.addRow(std::format("{:d}, \"{:s}\", {:d}, {:d}, {:d}, {:d}, {:d}, {:d}, {:d}, {:d}, {:d}, {:d}", id, "Test",
	                          2597, 6, 1715719401, 1, 1094, 78, 132, 114, 0, 1));
	BOOST_TEST(insert.execute());

	auto&& [status, body] =
	    tfs::http::handle_login({{"type", "login"}, {"email", "ghij@example.com"}, {"password", "bar"}}, ip);

	BOOST_TEST(status == status::ok);

	auto& session = body.at("session");
	BOOST_TEST(session.at("lastlogintime").as_uint64() == 1715719401);
	BOOST_TEST(session.at("ispremium").as_bool() == true);
	BOOST_TEST(session.at("premiumuntil").as_int64() == premiumEndsAt.count());

	result = db.storeQuery(
	    std::format("SELECT `token`, INET6_NTOA(`ip`) AS `ip` FROM `sessions` WHERE `account_id` = {:d}", id));
	BOOST_TEST(result, "Session not found in database.");
	BOOST_TEST(result->getString("token") == tfs::base64::decode(session.at("sessionkey").as_string()));
	BOOST_TEST(result->getString("ip") == ip);

	auto& worlds = body.at("playdata").at("worlds").as_array();
	BOOST_TEST(worlds.size() == 1);
	BOOST_TEST(worlds[0].at("id").as_int64() == 0);
	BOOST_TEST(worlds[0].at("name").as_string() == "Atlas");
	BOOST_TEST(worlds[0].at("externaladdressprotected").as_string() == "tfs.example.com");
	BOOST_TEST(worlds[0].at("externalportprotected").as_int64() == 7171);
	BOOST_TEST(worlds[0].at("externaladdressunprotected").as_string() == "tfs.example.com");
	BOOST_TEST(worlds[0].at("externalportunprotected").as_int64() == 7171);
	BOOST_TEST(worlds[0].at("location").as_string() == "Sweden");
	BOOST_TEST(worlds[0].at("pvptype").as_int64() == 0);

	auto& characters = body.at("playdata").at("characters").as_array();
	BOOST_TEST(characters.size() == 1);
	BOOST_TEST(characters[0].at("name").as_string() == "Test");
	BOOST_TEST(characters[0].at("level").as_uint64() == 2597);
	BOOST_TEST(characters[0].at("vocation").as_string() == "Elder Druid");
	BOOST_TEST(characters[0].at("lastlogin").as_uint64() == 1715719401);
	BOOST_TEST(characters[0].at("ismale").as_bool() == true);
	BOOST_TEST(characters[0].at("outfitid").as_uint64() == 1094);
	BOOST_TEST(characters[0].at("headcolor").as_uint64() == 78);
	BOOST_TEST(characters[0].at("torsocolor").as_uint64() == 132);
	BOOST_TEST(characters[0].at("legscolor").as_uint64() == 114);
	BOOST_TEST(characters[0].at("detailcolor").as_uint64() == 0);
	BOOST_TEST(characters[0].at("addonsflags").as_uint64() == 1);
}

BOOST_FIXTURE_TEST_CASE(test_login_success_with_token, LoginFixture)
{
	auto result = db.storeQuery(
	    "INSERT INTO `accounts` (`name`, `email`, `password`, `secret`) VALUES ('nbdj', 'nbdj@example.com', SHA1('bar'), UNHEX('')) RETURNING `id`");
	auto id = result->getNumber<uint64_t>("id");

	DBInsert insert("INSERT INTO `players` (`account_id`, `name`, `level`, `vocation`, `lastlogin`) VALUES");
	insert.addRow(std::format("{:d}, \"{:s}\", {:d}, {:d}, {:d}", id, "Testtoken", 2597, 6, 1715719401));
	BOOST_TEST(insert.execute());

	auto&& [status, body] = tfs::http::handle_login(
	    {
	        {"type", "login"},
	        {"email", "nbdj@example.com"},
	        {"password", "bar"},
	        {"token", generateToken("", now.count() / AUTHENTICATOR_PERIOD)},
	    },
	    ip);

	BOOST_TEST(status == status::ok);
}
