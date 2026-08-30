#define BOOST_TEST_MODULE http_check_email

#include "../../otpch.h"

#include "../router.h"

#include <boost/test/unit_test.hpp>

namespace routes = tfs::http::routes;

static std::string_view ip = "74.125.224.72";

BOOST_AUTO_TEST_CASE(test_check_email_missing_field)
{
	auto&& body = routes::handle_check_email({{"other", "value"}}, ip);
	BOOST_TEST(body.at("errorCode").as_int64() == 57);
}

BOOST_AUTO_TEST_CASE(test_check_email_not_a_string)
{
	auto&& body = routes::handle_check_email({{"email", 123}}, ip);
	BOOST_TEST(body.at("errorCode").as_int64() == 59);
}

BOOST_AUTO_TEST_CASE(test_check_email_empty)
{
	auto&& body = routes::handle_check_email({{"email", ""}}, ip);
	BOOST_TEST(body.at("errorCode").as_int64() == 57);
}

BOOST_AUTO_TEST_CASE(test_check_email_invalid_format)
{
	auto&& body = routes::handle_check_email({{"email", "not-an-email"}}, ip);
	BOOST_TEST(body.at("errorCode").as_int64() == 59);
}

BOOST_AUTO_TEST_CASE(test_check_email_valid)
{
	auto&& body = routes::handle_check_email({{"email", "test@example.com"}}, ip);
	BOOST_TEST(body.at("IsValid").as_bool());
}