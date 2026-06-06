#include "../otpch.h"

#include "../base64.h"

#include <benchmark/benchmark.h>

static void bench_base64_encode(benchmark::State& state)
{
	std::string input(state.range(0), 'x');
	for (auto&& _ : state) {
		auto result = tfs::base64::encode(input);
		benchmark::DoNotOptimize(result);
	}
}
BENCHMARK(bench_base64_encode)->Range(1, 1024);

static void bench_base64_decode(benchmark::State& state)
{
	std::string input(state.range(0), 'x');
	auto encoded = tfs::base64::encode(input);
	for (auto&& _ : state) {
		auto result = tfs::base64::decode(encoded);
		benchmark::DoNotOptimize(result);
	}
}
BENCHMARK(bench_base64_decode)->Range(1, 1024);

BENCHMARK_MAIN();
