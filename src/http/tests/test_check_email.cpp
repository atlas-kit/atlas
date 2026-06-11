#define BOOST_TEST_MODULE http_check_email

#include "../../otpch.h"

#include "../check_email.h"

#include <boost/test/unit_test.hpp>

using namespace std::chrono;

struct CheckEmailFixture
{
	std::string_view ip = "74.125.224.72";
};

using status = boost::beast::http::status;

BOOST_FIXTURE_TEST_CASE(test_check_email_missing_field, CheckEmailFixture)
{
	auto&& [stat, body] = tfs::http::handle_check_email({{"type", "CheckEMail"}}, ip);
	BOOST_TEST(stat == status::bad_request);
}

BOOST_FIXTURE_TEST_CASE(test_check_email_not_string, CheckEmailFixture)
{
	auto&& [stat, body] = tfs::http::handle_check_email({{"type", "CheckEMail"}, {"email", 123}}, ip);
	BOOST_TEST(stat == status::bad_request);
}

BOOST_FIXTURE_TEST_CASE(test_check_email_empty, CheckEmailFixture)
{
	auto&& [stat, body] = tfs::http::handle_check_email({{"type", "CheckEMail"}, {"email", ""}}, ip);
	BOOST_TEST(stat == status::bad_request);
}

BOOST_FIXTURE_TEST_CASE(test_check_email_invalid_format, CheckEmailFixture)
{
	auto&& [stat, body] = tfs::http::handle_check_email({{"type", "CheckEMail"}, {"email", "not-an-email"}}, ip);
	BOOST_TEST(stat == status::bad_request);
}

BOOST_FIXTURE_TEST_CASE(test_check_email_success, CheckEmailFixture)
{
	auto&& [stat, body] = tfs::http::handle_check_email({{"type", "CheckEMail"}, {"email", "valid@example.com"}}, ip);
	BOOST_TEST(stat == status::ok);
	BOOST_TEST(body.at("IsValid").as_bool() == true);
	BOOST_TEST(body.at("EMail").get_string() == "valid@example.com");
}