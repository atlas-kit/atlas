#define BOOST_TEST_MODULE fileloader

#include "../otpch.h"

#include "../fileloader.h"

#include <boost/test/unit_test.hpp>

using namespace std::string_view_literals;

BOOST_AUTO_TEST_SUITE(fileloader)

BOOST_AUTO_TEST_CASE(test_read_bytes)
{
	auto s = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x00"sv;

	auto first = s.data();

	auto bytes = OTB::readBytes(first, s.data() + s.size(), 8);
	BOOST_TEST(bytes == "\x01\x02\x03\x04\x05\x06\x07\x08"sv,
	           "expected '\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08', got '" << bytes << "'");
	BOOST_TEST(std::distance(first, s.data() + s.size()) == 2,
	           "expected 2 bytes left, got " << std::distance(first, s.data() + s.size()));

	bytes = OTB::readBytes(first, s.data() + s.size(), 2);
	BOOST_TEST(bytes == "\x09\x00"sv, "expected '\\x09\\x00', got '" << bytes << "'");
	BOOST_TEST(first == s.data() + s.size(),
	           "expected 0 bytes left, got " << std::distance(first, s.data() + s.size()));
}

BOOST_AUTO_TEST_CASE(test_read_bytes_escape)
{
	auto s = "\x01\x02\x03\x04\xFD\x05\x06\x07\x08\x09\x00"sv;

	auto first = s.data();

	auto bytes = OTB::readBytes(first, s.data() + s.size(), 8);
	BOOST_TEST(bytes == "\x01\x02\x03\x04\x05\x06\x07\x08"sv,
	           "expected '\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08', got '" << bytes << "'");
	BOOST_TEST(std::distance(first, s.data() + s.size()) == 2,
	           "expected 2 bytes left, got " << std::distance(first, s.data() + s.size()));

	bytes = OTB::readBytes(first, s.data() + s.size(), 2);
	BOOST_TEST(bytes == "\x09\x00"sv, "expected '\\x09\\x00', got '" << bytes << "'");
	BOOST_TEST(first == s.data() + s.size(),
	           "expected 0 bytes left, got " << std::distance(first, s.data() + s.size()));
}

BOOST_AUTO_TEST_CASE(test_read)
{
	auto s = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x00"sv;

	auto first = s.data();

	auto u64 = OTB::read<uint64_t>(first, s.data() + s.size());
	static_assert(std::is_same_v<decltype(u64), uint64_t>);
	BOOST_TEST(u64 == 0x0807060504030201,
	           "expected '0x" << std::hex << 0x0807060504030201 << "', got '0x" << u64 << "'");
	BOOST_TEST(std::distance(first, s.data() + s.size()) == 2,
	           "expected 2 bytes left, got " << std::distance(first, s.data() + s.size()));

	auto u16 = OTB::read<uint16_t>(first, s.data() + s.size());
	static_assert(std::is_same_v<decltype(u16), uint16_t>);
	BOOST_TEST(u16 == 0x0009, "expected '0x" << std::hex << 0x0009 << "', got '0x" << u16 << "'");
	BOOST_TEST(first == s.data() + s.size(),
	           "expected 0 bytes left, got " << std::distance(first, s.data() + s.size()));
}

BOOST_AUTO_TEST_CASE(test_read_escape)
{
	auto s = "\x01\x02\x03\x04\xFD\x05\x06\x07\x08"sv;

	auto first = s.data();

	auto u64 = OTB::read<uint64_t>(first, s.data() + s.size());
	BOOST_TEST(u64 == 0x0807060504030201,
	           "expected '0x" << std::hex << 0x0807060504030201 << "', got '0x" << u64 << "'");
	BOOST_TEST(first == s.data() + s.size(),
	           "expected 0 bytes left, got " << std::distance(first, s.data() + s.size()));
}

BOOST_AUTO_TEST_CASE(test_read_not_enough_bytes)
{
	auto s = "\x01\x02\x03\x04\x05\x06\x07"sv;

	auto first = s.data();
	BOOST_CHECK_THROW(std::ignore = OTB::read<uint64_t>(first, s.data() + s.size()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_read_escape_not_enough_bytes)
{
	auto s = "\x01\x02\x03\x04\xFD\x05\x06\x07"sv; // 8 bytes, but with escape it should be 7 actual bytes, not enough
	                                               // for length 8

	auto first = s.data();
	BOOST_CHECK_THROW(std::ignore = OTB::read<uint64_t>(first, s.data() + s.size()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_read_string)
{
	auto s =
	    "\x05\x00"
	    "atlas"
	    "\x06\x00"
	    "server"sv;
	BOOST_TEST(s.size() == 15, "expected 15 bytes, got " << s.size());

	auto first = s.data();

	auto result = OTB::readString(first, s.data() + s.size());
	BOOST_TEST(result == "atlas", "expected 'atlas', got '" << result << "'");
	BOOST_TEST(std::distance(first, s.data() + s.size()) == 8,
	           "expected 8 bytes left, got " << std::distance(first, s.data() + s.size()));

	result = OTB::readString(first, s.data() + s.size());
	BOOST_TEST(result == "server", "expected 'server', got '" << result << "'");
	BOOST_TEST(first == s.data() + s.size(),
	           "expected 0 bytes left, got " << std::distance(first, s.data() + s.size()));
}

BOOST_AUTO_TEST_CASE(test_read_string_escape)
{
	auto s =
	    "\x05\x00"
	    "atl\x{FD}as"
	    "\x07\x00"
	    "ser\xFD\xFDver"sv;
	BOOST_TEST(s.size() == 18, "expected 18 bytes, got " << s.size());

	auto first = s.data();

	auto result = OTB::readString(first, s.data() + s.size());
	BOOST_TEST(result == "atlas", "expected 'atlas', got '" << result << "'");
	BOOST_TEST(std::distance(first, s.data() + s.size()) == 10,
	           "expected 10 bytes left, got " << std::distance(first, s.data() + s.size()));

	result = OTB::readString(first, s.data() + s.size());
	BOOST_TEST(result == "ser\xFDver", "expected 'ser\xFDver', got '" << result << "'");
	BOOST_TEST(first == s.data() + s.size(),
	           "expected 0 bytes left, got " << std::distance(first, s.data() + s.size()));
}

BOOST_AUTO_TEST_CASE(test_read_string_not_enough_bytes)
{
	auto s = "\x09\x00"sv;

	auto first = s.data();
	BOOST_CHECK_THROW(std::ignore = OTB::readString(first, s.data() + s.size()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_read_string_escape_not_enough_bytes)
{
	auto s = "\x05\x00gh\xFDij"sv; // 5 bytes, but with escape it should be 4 actual bytes, not enough for length 5

	auto first = s.data();
	BOOST_CHECK_THROW(std::ignore = OTB::readString(first, s.data() + s.size()), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_skip)
{
	auto s = "\x01\x02\x03\x04\x05\x06"sv;

	auto first = s.data();
	OTB::skip(first, s.data() + s.size(), 4);
	BOOST_TEST(first == s.data() + 4, "expected 4 bytes skipped, got " << first - s.data());
}

BOOST_AUTO_TEST_CASE(test_skip_escape)
{
	auto s = "\x01\x02\xFD\x03\x04\x05\x06"sv;

	auto first = s.data();
	OTB::skip(first, s.data() + s.size(), 4);
	BOOST_TEST(first == s.data() + 5, "expected 5 bytes skipped, got " << first - s.data());
}

BOOST_AUTO_TEST_CASE(test_skip_not_enough_bytes)
{
	auto s = "\x01\x02\x03"sv;

	auto first = s.data();
	BOOST_CHECK_THROW(OTB::skip(first, s.data() + s.size(), 4), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_skip_escape_not_enough_bytes)
{
	auto s = "gh\xFDij"sv; // 5 bytes, but with escape it should be 4 actual bytes, not enough for length 5

	auto first = s.data();
	BOOST_CHECK_THROW(OTB::skip(first, s.data() + s.size(), 5), std::invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
