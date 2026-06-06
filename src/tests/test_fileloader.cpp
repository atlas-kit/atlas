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

BOOST_AUTO_TEST_SUITE(propstream)

BOOST_AUTO_TEST_CASE(test_propstream_init_and_size)
{
	PropStream stream;
	BOOST_TEST(stream.size() == 0);

	const char data[] = {0x01, 0x02, 0x03, 0x04};
	stream.init(data, sizeof(data));
	BOOST_TEST(stream.size() == 4);
}

BOOST_AUTO_TEST_CASE(test_propstream_read_success)
{
	const char data[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
	PropStream stream;
	stream.init(data, sizeof(data));

	uint16_t val;
	BOOST_TEST(stream.read<uint16_t>(val));
	BOOST_TEST(val == 0x0201);
	BOOST_TEST(stream.size() == 4);

	uint32_t val2;
	BOOST_TEST(stream.read<uint32_t>(val2));
	BOOST_TEST(val2 == 0x06050403);
	BOOST_TEST(stream.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_propstream_read_not_enough_bytes)
{
	const char data[] = {0x01, 0x02};
	PropStream stream;
	stream.init(data, sizeof(data));

	uint32_t val;
	BOOST_TEST(!stream.read<uint32_t>(val));
	BOOST_TEST(stream.size() == 2);
}

BOOST_AUTO_TEST_CASE(test_propstream_read_string_success)
{
	const char data[] = {0x05, 0x00, 'a', 't', 'l', 'a', 's'};
	PropStream stream;
	stream.init(data, sizeof(data));

	auto [result, ok] = stream.readString();
	BOOST_TEST(ok);
	BOOST_TEST(result == "atlas");
	BOOST_TEST(stream.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_propstream_read_string_empty)
{
	const char data[] = {0x00, 0x00};
	PropStream stream;
	stream.init(data, sizeof(data));

	auto [result, ok] = stream.readString();
	BOOST_TEST(ok);
	BOOST_TEST(result.empty());
	BOOST_TEST(stream.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_propstream_read_string_not_enough_bytes)
{
	const char data[] = {0x05, 0x00, 'a', 't'};
	PropStream stream;
	stream.init(data, sizeof(data));

	auto [result, ok] = stream.readString();
	BOOST_TEST(!ok);
	BOOST_TEST(result.empty());
}

BOOST_AUTO_TEST_CASE(test_propstream_read_string_truncated_length)
{
	const char data[] = {0x01};
	PropStream stream;
	stream.init(data, sizeof(data));

	auto [result, ok] = stream.readString();
	BOOST_TEST(!ok);
	BOOST_TEST(result.empty());
}

BOOST_AUTO_TEST_CASE(test_propstream_skip_success)
{
	const char data[] = {0x01, 0x02, 0x03, 0x04};
	PropStream stream;
	stream.init(data, sizeof(data));

	BOOST_TEST(stream.skip(3));
	BOOST_TEST(stream.size() == 1);

	uint8_t val;
	BOOST_TEST(stream.read<uint8_t>(val));
	BOOST_TEST(val == 0x04);
}

BOOST_AUTO_TEST_CASE(test_propstream_skip_zero)
{
	const char data[] = {0x01, 0x02, 0x03};
	PropStream stream;
	stream.init(data, sizeof(data));

	BOOST_TEST(stream.skip(0));
	BOOST_TEST(stream.size() == 3);
}

BOOST_AUTO_TEST_CASE(test_propstream_skip_not_enough)
{
	const char data[] = {0x01, 0x02};
	PropStream stream;
	stream.init(data, sizeof(data));

	BOOST_TEST(!stream.skip(5));
	BOOST_TEST(stream.size() == 2);
}

BOOST_AUTO_TEST_CASE(test_propstream_read_from_empty)
{
	PropStream stream;
	stream.init(nullptr, 0);

	uint8_t val;
	BOOST_TEST(!stream.read<uint8_t>(val));

	auto [result, ok] = stream.readString();
	BOOST_TEST(!ok);
	BOOST_TEST(result.empty());

	BOOST_TEST(!stream.skip(1));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(propwritestream)

BOOST_AUTO_TEST_CASE(test_propwritestream_empty)
{
	PropWriteStream stream;
	BOOST_TEST(stream.getStream().empty());
}

BOOST_AUTO_TEST_CASE(test_propwritestream_write_uint8)
{
	PropWriteStream stream;
	stream.write<uint8_t>(0x42);

	auto sv = stream.getStream();
	BOOST_TEST(sv.size() == 1);
	BOOST_TEST(static_cast<uint8_t>(sv[0]) == 0x42);
}

BOOST_AUTO_TEST_CASE(test_propwritestream_write_uint16)
{
	PropWriteStream stream;
	stream.write<uint16_t>(0x0102);

	auto sv = stream.getStream();
	BOOST_TEST(sv.size() == 2);
	BOOST_TEST(static_cast<uint8_t>(sv[0]) == 0x02);
	BOOST_TEST(static_cast<uint8_t>(sv[1]) == 0x01);
}

BOOST_AUTO_TEST_CASE(test_propwritestream_write_uint32)
{
	PropWriteStream stream;
	stream.write<uint32_t>(0x01020304);

	auto sv = stream.getStream();
	BOOST_TEST(sv.size() == 4);
	BOOST_TEST(static_cast<uint8_t>(sv[0]) == 0x04);
	BOOST_TEST(static_cast<uint8_t>(sv[1]) == 0x03);
	BOOST_TEST(static_cast<uint8_t>(sv[2]) == 0x02);
	BOOST_TEST(static_cast<uint8_t>(sv[3]) == 0x01);
}

BOOST_AUTO_TEST_CASE(test_propwritestream_write_multiple)
{
	PropWriteStream stream;
	stream.write<uint8_t>(0x11);
	stream.write<uint16_t>(0x3344);
	stream.write<uint32_t>(0x778899AA);

	auto sv = stream.getStream();
	BOOST_TEST(sv.size() == 7);
	BOOST_TEST(static_cast<uint8_t>(sv[0]) == 0x11);
	BOOST_TEST(static_cast<uint8_t>(sv[1]) == 0x44);
	BOOST_TEST(static_cast<uint8_t>(sv[2]) == 0x33);
	BOOST_TEST(static_cast<uint8_t>(sv[3]) == 0xAA);
	BOOST_TEST(static_cast<uint8_t>(sv[4]) == 0x99);
	BOOST_TEST(static_cast<uint8_t>(sv[5]) == 0x88);
	BOOST_TEST(static_cast<uint8_t>(sv[6]) == 0x77);
}

BOOST_AUTO_TEST_CASE(test_propwritestream_write_string)
{
	PropWriteStream stream;
	stream.writeString("atlas");

	auto sv = stream.getStream();
	BOOST_TEST(sv.size() == 7); // 2 bytes length + 5 bytes data
	BOOST_TEST(static_cast<uint8_t>(sv[0]) == 0x05);
	BOOST_TEST(static_cast<uint8_t>(sv[1]) == 0x00);
	BOOST_TEST(std::string_view(sv.data() + 2, 5) == "atlas");
}

BOOST_AUTO_TEST_CASE(test_propwritestream_write_empty_string)
{
	PropWriteStream stream;
	stream.writeString("");

	auto sv = stream.getStream();
	BOOST_TEST(sv.size() == 2); // 2 bytes length only
	BOOST_TEST(static_cast<uint8_t>(sv[0]) == 0x00);
	BOOST_TEST(static_cast<uint8_t>(sv[1]) == 0x00);
}

BOOST_AUTO_TEST_CASE(test_propwritestream_write_string_truncated)
{
	PropWriteStream stream;
	// String longer than UINT16_MAX should write length 0 (existing behavior)
	std::string huge(65536, 'x');
	stream.writeString(huge);

	auto sv = stream.getStream();
	BOOST_TEST(sv.size() == 2); // only length 0 written
	BOOST_TEST(static_cast<uint8_t>(sv[0]) == 0x00);
	BOOST_TEST(static_cast<uint8_t>(sv[1]) == 0x00);
}

BOOST_AUTO_TEST_CASE(test_propwritestream_clear)
{
	PropWriteStream stream;
	stream.write<uint32_t>(0xdeadbeef);
	BOOST_TEST(!stream.getStream().empty());

	stream.clear();
	BOOST_TEST(stream.getStream().empty());
}

BOOST_AUTO_TEST_CASE(test_propwritestream_roundtrip_via_propstream)
{
	PropWriteStream writer;
	writer.write<uint8_t>(0x42);
	writer.write<uint16_t>(0x0102);
	writer.writeString("hello");

	auto sv = writer.getStream();

	PropStream reader;
	reader.init(sv.data(), sv.size());

	uint8_t u8;
	BOOST_REQUIRE(reader.read<uint8_t>(u8));
	BOOST_TEST(u8 == 0x42);

	uint16_t u16;
	BOOST_REQUIRE(reader.read<uint16_t>(u16));
	BOOST_TEST(u16 == 0x0102);

	auto [str, ok] = reader.readString();
	BOOST_REQUIRE(ok);
	BOOST_TEST(str == "hello");

	BOOST_TEST(reader.size() == 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
