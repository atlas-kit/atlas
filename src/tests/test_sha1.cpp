#define BOOST_TEST_MODULE sha1

#include "../otpch.h"

#include "../tools.h"

#include <boost/test/unit_test.hpp>

using namespace std::string_view_literals;

struct SHA1Fixture
{
	std::string_view input;
	std::string_view expected;
};

// test vectors from https://csrc.nist.gov/projects/cryptographic-algorithm-validation-program/secure-hashing
// up to 64 bytes in input length
auto shortVectors = std::vector<SHA1Fixture>{
    {.input = "", .expected = "\xda\x39\xa3\xee\x5e\x6b\x4b\x0d\x32\x55\xbf\xef\x95\x60\x18\x90\xaf\xd8\x07\x09"sv},
    {.input = "\x36", .expected = "\xc1\xdf\xd9\x6e\xea\x8c\xc2\xb6\x27\x85\x27\x5b\xca\x38\xac\x26\x12\x56\xe2\x78"sv},
    {.input = "\x19\x5a",
     .expected = "\x0a\x1c\x2d\x55\x5b\xbe\x43\x1a\xd6\x28\x8a\xf5\xa5\x4f\x93\xe0\x44\x9c\x92\x32"sv},
    {.input = "\xdf\x4b\xd2",
     .expected = "\xbf\x36\xed\x5d\x74\x72\x7d\xfd\x5d\x78\x54\xec\x6b\x1d\x49\x46\x8d\x8e\xe8\xaa"sv},
    {.input = "\x54\x9e\x95\x9e",
     .expected = "\xb7\x8b\xae\x6d\x14\x33\x8f\xfc\xcf\xd5\xd5\xb5\x67\x4a\x27\x5f\x6e\xf9\xc7\x17"sv},
    {.input = "\xf7\xfb\x1b\xe2\x05",
     .expected = "\x60\xb7\xd5\xbb\x56\x0a\x1a\xcf\x6f\xa4\x57\x21\xbd\x0a\xbb\x41\x9a\x84\x1a\x89"sv},
    {.input = "\xc0\xe5\xab\xea\xea\x63",
     .expected = "\xa6\xd3\x38\x45\x97\x80\xc0\x83\x63\x09\x0f\xd8\xfc\x7d\x28\xdc\x80\xe8\xe0\x1f"sv},
    {.input = "\x63\xbf\xc1\xed\x7f\x78\xab",
     .expected = "\x86\x03\x28\xd8\x05\x09\x50\x0c\x17\x83\x16\x9e\xbf\x0b\xa0\xc4\xb9\x4d\xa5\xe5"sv},
    {.input = "\x7e\x3d\x7b\x3e\xad\xa9\x88\x66",
     .expected = "\x24\xa2\xc3\x4b\x97\x63\x05\x27\x7c\xe5\x8c\x2f\x42\xd5\x09\x20\x31\x57\x25\x20"sv},

};

// FIPS 180-4 known answer tests for block-boundary coverage
auto blockBoundaryVectors = std::vector<SHA1Fixture>{
    // http://www.nsrl.nist.gov/testdata/ - SHA1
    // 56 bytes (forces second block due to SHA1 padding)
    {.input = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
     .expected = "\x84\x98\x3e\x44\x1c\x3b\xd2\x6e\xba\xae\x4a\xa1\xf9\x51\x29\xe5\xe5\x46\x70\xf1"sv},
    // 112 bytes (multiple blocks, from FIPS 180-4)
    {.input =
         "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
     .expected = "\xa4\x9b\x24\x46\xa0\x2c\x64\x5b\xf4\x19\xf9\x95\xb6\x70\x91\x25\x3a\x04\xa2\x59"sv},
};

BOOST_AUTO_TEST_SUITE(sha1)

BOOST_AUTO_TEST_CASE(test_sha1_short_vectors)
{
	for (auto&& [input, expected] : shortVectors) {
		std::string result = transformToSHA1(input);
		BOOST_TEST(result == expected, "expected '" << expected << "', got '" << result << "'");
	}
}

BOOST_AUTO_TEST_CASE(test_sha1_block_boundary_vectors)
{
	for (auto&& [input, expected] : blockBoundaryVectors) {
		std::string result = transformToSHA1(input);
		BOOST_TEST(result == expected, "expected '" << expected << "', got '" << result << "'");
	}
}

BOOST_AUTO_TEST_CASE(test_sha1_output_length)
{
	// SHA1 always produces 20-byte output regardless of input size
	for (size_t len : {0, 1, 55, 56, 64, 65, 128, 1024}) {
		std::string input(len, 'a');
		std::string result = transformToSHA1(input);
		BOOST_TEST(result.size() == 20, "expected 20 bytes for input length " << len << ", got " << result.size());
	}
}

BOOST_AUTO_TEST_CASE(test_sha1_deterministic)
{
	auto a = transformToSHA1("The quick brown fox jumps over the lazy dog");
	auto b = transformToSHA1("The quick brown fox jumps over the lazy dog");
	BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(test_sha1_binary_data_with_nulls)
{
	std::string input(64, '\0');
	input[0] = 'a';
	input[63] = 'z';
	auto result = transformToSHA1(input);
	BOOST_TEST(result.size() == 20);
}

BOOST_AUTO_TEST_CASE(test_sha1_all_zeros)
{
	std::string input(64, '\0');
	auto result = transformToSHA1(input);
	BOOST_TEST(result.size() == 20);
}

BOOST_AUTO_TEST_CASE(test_sha1_all_ff)
{
	std::string input(64, '\xFF');
	auto result = transformToSHA1(input);
	BOOST_TEST(result.size() == 20);
}

BOOST_AUTO_TEST_SUITE_END()
