#include "../otpch.h"

#include "../xtea.h"

#include <benchmark/benchmark.h>
#include <cassert>

static void bench_xtea_expand_key(benchmark::State& state)
{
	xtea::key key = {0x12345678, 0x9ABCDEF0, 0x0FEDCBA9, 0x87654321};
	for (auto&& _ : state) {
		auto result = xtea::expand_key(key);
		benchmark::DoNotOptimize(result);
	}
}
BENCHMARK(bench_xtea_expand_key);

static void bench_xtea_encrypt(benchmark::State& state)
{
	// XTEA requires data length to be a multiple of 8 bytes.
	size_t size = state.range(0);
	assert(size % 8 == 0);

	xtea::key key = {0x12345678, 0x9ABCDEF0, 0x0FEDCBA9, 0x87654321};
	auto roundKeys = xtea::expand_key(key);

	std::vector<uint8_t> data(size, 0x42);
	for (auto&& _ : state) {
		xtea::encrypt(data.data(), size, roundKeys);
		benchmark::DoNotOptimize(data);

		state.SetBytesProcessed(state.range(0) * state.iterations());
	}
}
BENCHMARK(bench_xtea_encrypt)->Range(8, 65500);

static void bench_xtea_decrypt(benchmark::State& state)
{
	// XTEA requires data length to be a multiple of 8 bytes.
	size_t size = state.range(0);
	assert(size % 8 == 0);

	xtea::key key = {0x12345678, 0x9ABCDEF0, 0x0FEDCBA9, 0x87654321};
	auto roundKeys = xtea::expand_key(key);

	std::vector<uint8_t> data(size, 0x42);
	xtea::encrypt(data.data(), size, roundKeys);
	for (auto&& _ : state) {
		xtea::decrypt(data.data(), size, roundKeys);
		benchmark::DoNotOptimize(data);

		state.SetBytesProcessed(state.range(0) * state.iterations());
	}
}
BENCHMARK(bench_xtea_decrypt)->Range(8, 65500);

BENCHMARK_MAIN();
