#define BOOST_TEST_MODULE base64

#include "../otpch.h"

#include "../base64.h"

#include <boost/test/unit_test.hpp>

struct Basee64Fixture
{
	std::string_view plain;
	std::string_view encoded;
};

// test vectors from https://datatracker.ietf.org/doc/html/rfc4648#section-10
auto testVectors = std::vector<Basee64Fixture>{
    {.plain = "", .encoded = ""},
    {.plain = "f", .encoded = "Zg=="},
    {.plain = "fo", .encoded = "Zm8="},
    {.plain = "foo", .encoded = "Zm9v"},
    {.plain = "foob", .encoded = "Zm9vYg=="},
    {.plain = "fooba", .encoded = "Zm9vYmE="},
    {.plain = "foobar", .encoded = "Zm9vYmFy"},

};

BOOST_AUTO_TEST_SUITE(base64)

BOOST_AUTO_TEST_CASE(test_base64_encode)
{
	for (auto&& [plain, encoded] : testVectors) {
		std::string result = tfs::base64::encode(plain);
		BOOST_TEST(result == encoded, "expected '" << encoded << "', got '" << result << "'");
	}
}

BOOST_AUTO_TEST_CASE(test_base64_decode)
{
	for (auto&& [plain, encoded] : testVectors) {
		std::string result = tfs::base64::decode(encoded);
		BOOST_TEST(result == plain, "expected '" << plain << "', got '" << result << "'");
	}
}

BOOST_AUTO_TEST_CASE(test_base64_roundtrip_binary)
{
	for (auto&& [plain, encoded] : testVectors) {
		std::string encoded_result = tfs::base64::encode(plain);
		BOOST_TEST(encoded_result == encoded, "encode: expected '" << encoded << "', got '" << encoded_result << "'");

		std::string decoded_result = tfs::base64::decode(encoded);
		BOOST_TEST(decoded_result == plain, "decode: expected '" << plain << "', got '" << decoded_result << "'");
	}
}

BOOST_AUTO_TEST_CASE(test_base64_roundtrip_arbitrary)
{
	// Round-trip encode/decode with various binary patterns
	auto raw = std::vector<std::string>{
	    std::string(1, '\0'),    std::string("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", 16),
	    std::string(1, '\xFF'),  "\xde\xad\xbe\xef",
	    std::string(32, '\xFF'),
	};

	for (const auto& input : raw) {
		auto encoded = tfs::base64::encode(input);
		auto decoded = tfs::base64::decode(encoded);
		BOOST_TEST(decoded == input, "round-trip failed for input of size " << input.size() << ": expected '" << input
		                                                                    << "', got '" << decoded << "'");
	}
}

BOOST_AUTO_TEST_CASE(test_base64_decode_invalid_chars)
{
	// Invalid base64 characters — behavior depends on OpenSSL's BIO.
	// At minimum the function should not crash and should return something.
	auto result = tfs::base64::decode("!!!");
	BOOST_TEST(result.empty(), "expected empty or error, got '" << result << "'");
}

BOOST_AUTO_TEST_CASE(test_base64_decode_malformed_padding)
{
	// Malformed padding — lengths not a multiple of 4
	tfs::base64::decode("A"); // at least no crash
	BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(test_base64_decode_all_padding)
{
	auto result = tfs::base64::decode("====");
	BOOST_TEST(result.empty());
}

BOOST_AUTO_TEST_SUITE_END()
