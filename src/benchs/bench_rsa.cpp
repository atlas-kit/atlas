#include "../otpch.h"

#include "../rsa.h"

#include <benchmark/benchmark.h>

static constexpr std::string_view privateKey =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIICXAIBAAKBgQCbZGkDtFsHrJVlaNhzU71xZROd15QHA7A+bdB5OZZhtKg3qmBWHXzLlFL6AIBZ\n"
    "SQmIKrW8pYoaGzX4sQWbcrEhJhHGFSrT27PPvuetwUKnXT11lxUJwyHFwkpb1R/UYPAbThW+sN4Z\n"
    "MFKKXT8VwePL9cQB1nd+EKyqsz2+jVt/9QIDAQABAoGAQovTtTRtr3GnYRBvcaQxAvjIV9ZUnFRm\n"
    "C7Y3i1KwJhOZ3ozmSLrEEOLqTgoc7R+sJ1YzEiDKbbete11EC3gohlhW56ptj0WDf+7ptKOgqiEy\n"
    "Kh4qt1sYJeeGz4GiiooJoeKFGdtk/5uvMR6FDCv6H7ewigVswzf330Q3Ya7+jYECQQERBxsga6+5\n"
    "x6IofXyNF6QuMqvuiN/pUgaStUOdlnWBf/T4yUpKvNS1+I4iDzqGWOOSR6RsaYPYVhj9iRABoKyx\n"
    "AkEAkbNzB6vhLAWht4dUdGzaREF3p4SwNcu5bJRa/9wCLSHaS9JaTq4lljgVPp1zyXyJCSCWpFnl\n"
    "0WvK3Qf6nVBIhQJBANS7rK8+ONWQbxENdZaZ7Rrx8HUTwSOS/fwhsGWBbl1Qzhdq/6/sIfEHkfeH\n"
    "1hoH+IlpuPuf21MdAqvJt+cMwoECQF1LyBOYduYGcSgg6u5mKVldhm3pJCA+ZGxnjuGZEnet3qeA\n"
    "eb05++112fyvO85ABUun524z9lokKNFh45NKLjUCQGshzV43P+RioiBhtEpB/QFzijiS4L2HKNu1\n"
    "tdhudnUjWkaf6jJmQS/ppln0hhRMHlk9Vus/bPx7LtuDuo6VQDo=\n"
    "-----END RSA PRIVATE KEY-----\n";

static void bench_rsa_load_pem(benchmark::State& state)
{
	for (auto&& _ : state) {
		auto* pkey = tfs::rsa::loadPEM(privateKey);
		benchmark::DoNotOptimize(pkey);
	}
}
BENCHMARK(bench_rsa_load_pem);

static void bench_rsa_decrypt(benchmark::State& state)
{
	tfs::rsa::loadPEM(privateKey);

	// RSA decrypts a single block whose size must match the key modulus (128 bytes for a 1024-bit key).
	std::vector<uint8_t> data(state.range(0), 0x42);
	for (auto&& _ : state) {
		tfs::rsa::decrypt(data.data(), data.size());
		benchmark::DoNotOptimize(data);
	}
	state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bench_rsa_decrypt)->Arg(128);

BENCHMARK_MAIN();
