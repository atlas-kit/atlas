#define BOOST_TEST_MODULE http_create_account

#include "../../otpch.h"

#include "../../configmanager.h"
#include "../../database.h"
#include "../router.h"
#include "vocations.h"

#include <boost/test/unit_test.hpp>

using namespace std::chrono;

struct CreateAccountFixture
{
	CreateAccountFixture()
	{
		setString(ConfigManager::MYSQL_HOST, "0.0.0.0");
		setString(ConfigManager::MYSQL_USER, "atlas");
		setString(ConfigManager::MYSQL_PASS, "atlas");
		setString(ConfigManager::MYSQL_DB, "atlas");
		setNumber(ConfigManager::SQL_PORT, 3306);

		tfs::http::tests::mock_vocations();

		db.connect();
		transaction.begin();
	}

	Database& db = Database::getInstance();
	DBTransaction transaction;

	std::string_view ip = "74.125.224.72";
	seconds now = duration_cast<seconds>(system_clock::now().time_since_epoch());
};

using status = boost::beast::http::status;

BOOST_FIXTURE_TEST_CASE(test_create_account_missing_character_name, CreateAccountFixture)
{
	auto&& body = tfs::http::routes::handle_create_account(
	    {
	        {"type", "CreateAccountAndCharacter"},
	        {"EMail", "test@example.com"},
	        {"Password", "password"},
	        {"CharacterSex", "female"},
	    },
	    ip);

	BOOST_TEST(body.at("errorCode").as_int64() == 6);
}

BOOST_FIXTURE_TEST_CASE(test_create_account_empty_character_name, CreateAccountFixture)
{
	auto&& body = tfs::http::routes::handle_create_account(
	    {
	        {"type", "CreateAccountAndCharacter"},
	        {"EMail", "test@example.com"},
	        {"Password", "password"},
	        {"CharacterName", ""},
	        {"CharacterSex", "female"},
	    },
	    ip);

	BOOST_TEST(body.at("errorCode").as_int64() == 6);
}

BOOST_FIXTURE_TEST_CASE(test_create_account_invalid_email_formats, CreateAccountFixture)
{
	auto invalid_emails = {
	    "plainaddress",     // missing @
	    "@missinguser.com", // missing user
	    "user@.com",        // missing domain name
	    // "user@domain..com",     // double dot
	    "user@domain@extra.com" // multiple @
	};

	for (const auto& email : invalid_emails) {
		auto&& body = tfs::http::routes::handle_create_account(
		    {
		        {"type", "CreateAccountAndCharacter"},
		        {"EMail", email},
		        {"Password", "password"},
		        {"CharacterName", "New Char"},
		        {"CharacterSex", "female"},
		    },
		    ip);

		std::cout << "Testing invalid email: " << email << std::endl;
		BOOST_TEST(body.at("errorCode").as_int64() == 59);
	}
}

BOOST_FIXTURE_TEST_CASE(test_create_account_missing_email, CreateAccountFixture)
{
	auto&& body = tfs::http::routes::handle_create_account(
	    {
	        {"type", "CreateAccountAndCharacter"},
	        {"Password", "password"},
	        {"CharacterName", "New Char"},
	        {"CharacterSex", "female"},
	    },
	    ip);

	BOOST_TEST(body.at("errorCode").as_int64() == 57);
}

BOOST_FIXTURE_TEST_CASE(test_create_account_empty_password, CreateAccountFixture)
{
	auto&& body = tfs::http::routes::handle_create_account(
	    {
	        {"type", "CreateAccountAndCharacter"},
	        {"EMail", "test@example.com"},
	        {"Password", ""},
	        {"CharacterName", "New Char"},
	        {"CharacterSex", "female"},
	    },
	    ip);

	BOOST_TEST(body.at("errorCode").as_int64() == 87);
}

BOOST_FIXTURE_TEST_CASE(test_create_account_success, CreateAccountFixture)
{
	auto&& body = tfs::http::routes::handle_create_account(
	    {
	        {"type", "CreateAccountAndCharacter"},
	        {"EMail", "newuser@example.com"},
	        {"Password", "StrongPass123!"},
	        {"CharacterName", "New Char"},
	        {"CharacterSex", "female"},
	    },
	    ip);

	BOOST_TEST(body.at("session").is_object());
	BOOST_TEST(body.at("playdata").is_object());

	auto result = db.storeQuery("SELECT `email` FROM `accounts` WHERE `email` = 'newuser@example.com'");
	BOOST_REQUIRE(result);

	BOOST_TEST(result->getString("email") == "newuser@example.com");
}