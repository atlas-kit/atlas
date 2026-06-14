#define BOOST_TEST_MODULE http_validation

#include "../../otpch.h"

#include "../validation.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_create_account_invalid_character_name_empty)
{
	auto&& ret = tfs::http::is_valid_character_name("");

	BOOST_TEST(ret.has_value());
}

BOOST_AUTO_TEST_CASE(test_create_account_invalid_character_name_too_long)
{
	auto&& ret =
	    tfs::http::is_valid_character_name("ThisIsAVeryLongNameThatShouldExceedTheSystemLimitOfTwentyNineCharacters");

	BOOST_TEST(ret.has_value());
}

BOOST_AUTO_TEST_CASE(test_create_account_invalid_character_name_word_too_long)
{
	auto&& ret = tfs::http::is_valid_character_name("A Supercalifragilisticexpialidocious Name");

	BOOST_TEST(ret.has_value());
}

BOOST_AUTO_TEST_CASE(test_create_account_invalid_character_name_multiple_spaces)
{
	auto&& ret = tfs::http::is_valid_character_name("Name  WithTwoSpaces");

	BOOST_TEST(ret.has_value());
}

BOOST_AUTO_TEST_CASE(test_create_account_invalid_character_name_leading_space)
{
	auto&& ret = tfs::http::is_valid_character_name(" TestChar");

	BOOST_TEST(ret.has_value());
}

BOOST_AUTO_TEST_CASE(test_create_account_invalid_character_name_trailing_space)
{
	auto&& ret = tfs::http::is_valid_character_name("TestChar ");

	BOOST_TEST(ret.has_value());
}

BOOST_AUTO_TEST_CASE(test_create_account_invalid_character_name_invalid_characters)
{
	auto&& ret = tfs::http::is_valid_character_name("TestChar!@#$%^&*()");

	BOOST_TEST(ret.has_value());
}

BOOST_AUTO_TEST_CASE(test_create_account_valid_character_name)
{
	auto&& ret = tfs::http::is_valid_character_name("Test Char");

	BOOST_TEST(!ret.has_value());
}