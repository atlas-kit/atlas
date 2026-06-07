#include "../otpch.h"

#include "../reactor.h"

#include <benchmark/benchmark.h>

static void bench_reactor_send(benchmark::State& state)
{
	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;
		reactor.send([&] { ++count; });
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_send);

static void bench_reactor_schedule(benchmark::State& state)
{
	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;
		reactor.schedule(0ms, [&] { ++count; });
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_schedule);

static void bench_reactor_send_batch(benchmark::State& state)
{
	const int64_t batchSize = state.range(0);

	TaskReactor reactor;
	int count = 0;

	for (auto&& _ : state) {
		count = 0;
		for (int64_t i = 0; i < batchSize; ++i) {
			reactor.send([&] { ++count; });
		}
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_send_batch)->RangeMultiplier(2)->Range(1, 8192);

static void bench_reactor_schedule_batch(benchmark::State& state)
{
	const int64_t batchSize = state.range(0);

	TaskReactor reactor;
	int count = 0;

	for (auto&& _ : state) {
		count = 0;
		for (int64_t i = 0; i < batchSize; ++i) {
			reactor.schedule(0ms, [&] { ++count; });
		}
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_schedule_batch)->RangeMultiplier(2)->Range(1, 8192);

static void bench_reactor_mixed_send_schedule(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	TaskReactor reactor;
	int count = 0;

	for (auto&& _ : state) {
		count = 0;
		for (int64_t i = 0; i < taskCount; ++i) {
			if (i % 3 == 0) {
				reactor.schedule(0ms, [&] { ++count; });
			} else {
				reactor.send([&] { ++count; });
			}
		}
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_mixed_send_schedule)->RangeMultiplier(2)->Range(1, 8192);

static void bench_reactor_cancel_all(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;

		std::vector<uint32_t> ids;
		ids.reserve(taskCount);
		for (int64_t i = 0; i < taskCount; ++i) {
			ids.push_back(reactor.schedule(1h, [&] { ++count; }));
		}

		for (auto id : ids) {
			reactor.cancel(id);
		}

		reactor.send([&] { ++count; });
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_cancel_all)->RangeMultiplier(4)->Range(1, 4096);

static void bench_reactor_cancel_half(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;

		std::vector<uint32_t> ids;
		ids.reserve(taskCount);
		for (int64_t i = 0; i < taskCount; ++i) {
			ids.push_back(reactor.schedule(0ms, [&] { ++count; }));
		}

		for (int64_t i = 0; i < taskCount; i += 2) {
			reactor.cancel(ids[i]);
		}

		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_cancel_half)->RangeMultiplier(4)->Range(1, 4096);

static void bench_reactor_heap_pressure(benchmark::State& state)
{
	const int64_t pendingCount = state.range(0);

	TaskReactor reactor;
	int count = 0;

	// Fill the heap with future tasks
	for (int64_t i = 0; i < pendingCount; ++i) {
		reactor.schedule(1h, [&] { ++count; });
	}

	for (auto&& _ : state) {
		reactor.send([&] { ++count; });
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_heap_pressure)->RangeMultiplier(8)->Range(1, 16384);

static void bench_reactor_send_with_deadline(benchmark::State& state)
{
	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;
		reactor.send(1h, [&] { ++count; });
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_send_with_deadline);

static void bench_reactor_send_expired_deadline(benchmark::State& state)
{
	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;
		reactor.send(-1ms, [&] { ++count; });
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_send_expired_deadline);

static void bench_reactor_accumulate(benchmark::State& state)
{
	const int64_t iterations = state.range(0);

	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;

		for (int64_t i = 0; i < iterations; ++i) {
			reactor.send([&] { ++count; });
			reactor.runOnce();
		}
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_accumulate)->RangeMultiplier(2)->Range(1, 1024);

static void bench_reactor_schedule_expired_deadline_cleanup(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;

		for (int64_t i = 0; i < taskCount; ++i) {
			reactor.send(-1ms, [&] { ++count; });
		}
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_schedule_expired_deadline_cleanup)->RangeMultiplier(4)->Range(1, 4096);

BENCHMARK_MAIN();
