#define BOOST_TEST_MODULE http_check_password

#include "../../otpch.h"

#include "../router.h"

#include <boost/test/unit_test.hpp>

namespace routes = tfs::http::routes;

static std::string_view ip = "74.125.224.72";

BOOST_AUTO_TEST_CASE(test_check_password_missing_field)
{
	auto&& body = routes::handle_check_password({{"other", "value"}}, ip);
	BOOST_TEST(body.at("errorCode").as_int64() == 11);
}

BOOST_AUTO_TEST_CASE(test_check_password_not_a_string)
{
	auto&& body = routes::handle_check_password({{"Password1", 123}}, ip);
	BOOST_TEST(body.at("errorCode").as_int64() == 11);
}

BOOST_AUTO_TEST_CASE(test_check_password_empty)
{
	auto&& body = routes::handle_check_password({{"Password1", ""}}, ip);
	BOOST_TEST(body.at("errorCode").as_int64() == 11);
}

BOOST_AUTO_TEST_CASE(test_check_password_has_lowercase_true)
{
	auto&& body = routes::handle_check_password({{"Password1", "pass"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(reqs.at("HasLowerCase").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_has_lowercase_false)
{
	auto&& body = routes::handle_check_password({{"Password1", "PASS"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(!reqs.at("HasLowerCase").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_has_uppercase_true)
{
	auto&& body = routes::handle_check_password({{"Password1", "PASS"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(reqs.at("HasUpperCase").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_has_uppercase_false)
{
	auto&& body = routes::handle_check_password({{"Password1", "pass"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(!reqs.at("HasUpperCase").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_has_number_true)
{
	auto&& body = routes::handle_check_password({{"Password1", "123"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(reqs.at("HasNumber").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_has_number_false)
{
	auto&& body = routes::handle_check_password({{"Password1", "Pass!"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(!reqs.at("HasNumber").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_has_invalid_characters_true)
{
	auto&& body = routes::handle_check_password({{"Password1", "Pass!@#"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(reqs.at("InvalidCharacters").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_has_invalid_characters_false)
{
	auto&& body = routes::handle_check_password({{"Password1", "Pass\n123"}}, ip);
	auto reqs = body.at("PasswordRequirements").as_object();
	BOOST_TEST(!reqs.at("InvalidCharacters").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_validity_true)
{
	auto&& body = routes::handle_check_password({{"Password1", "Pass123!"}}, ip);
	BOOST_TEST(body.at("PasswordValid").as_bool());
}

BOOST_AUTO_TEST_CASE(test_check_password_validity_false)
{
	auto&& body = routes::handle_check_password({{"Password1", "pass"}}, ip);
	BOOST_TEST(!body.at("PasswordValid").as_bool());
}