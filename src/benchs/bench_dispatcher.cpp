#include "../otpch.h"

#include "../scheduler.h"
#include "../tasks.h"

#include <benchmark/benchmark.h>

static void bench_dispatcher_addTask(benchmark::State& state)
{
	for (auto&& _ : state) {
		Dispatcher dispatcher;
		int count = 0;
		dispatcher.addTask([&] { ++count; });
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_addTask);

static void bench_dispatcher_addTask_batch(benchmark::State& state)
{
	const int64_t batchSize = state.range(0);

	Dispatcher dispatcher;
	int count = 0;

	for (auto&& _ : state) {
		count = 0;
		for (int64_t i = 0; i < batchSize; ++i) {
			dispatcher.addTask([&] { ++count; });
		}
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_addTask_batch)->RangeMultiplier(2)->Range(1, 8192);

// Equivalent to reactor's schedule(0ms) — the scheduler immediately
// forwards 0ms tasks to the dispatcher, so we benchmark the
// dispatcher directly here for a fair comparison.
static void bench_dispatcher_schedule(benchmark::State& state)
{
	for (auto&& _ : state) {
		Dispatcher dispatcher;
		int count = 0;
		dispatcher.addTask([&] { ++count; });
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_schedule);

static void bench_dispatcher_schedule_batch(benchmark::State& state)
{
	const int64_t batchSize = state.range(0);

	Dispatcher dispatcher;
	int count = 0;

	for (auto&& _ : state) {
		count = 0;
		for (int64_t i = 0; i < batchSize; ++i) {
			dispatcher.addTask([&] { ++count; });
		}
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_schedule_batch)->RangeMultiplier(2)->Range(1, 8192);

static void bench_dispatcher_mixed_addTask(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	Dispatcher dispatcher;
	int count = 0;

	for (auto&& _ : state) {
		count = 0;
		for (int64_t i = 0; i < taskCount; ++i) {
			if (i % 3 == 0) {
				// simulate "schedule" — just addTask with 0ms
				dispatcher.addTask([&] { ++count; });
			} else {
				dispatcher.addTask([&] { ++count; });
			}
		}
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_mixed_addTask)->RangeMultiplier(2)->Range(1, 8192);

static void bench_dispatcher_cancel_all(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		// The old dispatcher doesn't support cancellation.
		// Instead we benchmark addTask + drain to compare
		// baseline throughput against the reactor's equivalent.
		Dispatcher dispatcher;
		int count = 0;

		// "schedule" tasks with expiration (equivalent to 1h delay)
		for (int64_t i = 0; i < taskCount; ++i) {
			dispatcher.addTask(3600000, [&] { ++count; });
		}

		// "cancel" all is not supported — just drain
		dispatcher.drain();

		// send one immediate task
		dispatcher.addTask([&] { ++count; });
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_cancel_all)->RangeMultiplier(4)->Range(1, 4096);

static void bench_dispatcher_cancel_half(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		// The old dispatcher doesn't support cancellation.
		// Benchmark dispatcher throughput for comparison.
		Dispatcher dispatcher;
		int count = 0;

		for (int64_t i = 0; i < taskCount; ++i) {
			dispatcher.addTask([&] { ++count; });
		}

		// cancel not supported — drain all instead
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_cancel_half)->RangeMultiplier(4)->Range(1, 4096);

static void bench_dispatcher_heap_pressure(benchmark::State& state)
{
	const int64_t pendingCount = state.range(0);

	Dispatcher dispatcher;
	int count = 0;

	// The old dispatcher has no heap — tasks are stored in a
	// vector and processed FIFO. Pre-fill with future-expiration tasks
	// to simulate pending scheduled tasks.
	for (int64_t i = 0; i < pendingCount; ++i) {
		dispatcher.addTask(3600000, [&] { ++count; });
	}

	for (auto&& _ : state) {
		dispatcher.addTask([&] { ++count; });
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_heap_pressure)->RangeMultiplier(8)->Range(1, 16384);

static void bench_dispatcher_addTask_with_expiration(benchmark::State& state)
{
	for (auto&& _ : state) {
		Dispatcher dispatcher;
		int count = 0;
		dispatcher.addTask(3600000, [&] { ++count; });
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_addTask_with_expiration);

static void bench_dispatcher_addTask_expired(benchmark::State& state)
{
	for (auto&& _ : state) {
		Dispatcher dispatcher;
		int count = 0;
		dispatcher.addTask(0u, [&] { ++count; });
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_addTask_expired);

static void bench_dispatcher_accumulate(benchmark::State& state)
{
	const int64_t iterations = state.range(0);

	for (auto&& _ : state) {
		Dispatcher dispatcher;
		int count = 0;

		for (int64_t i = 0; i < iterations; ++i) {
			dispatcher.addTask([&] { ++count; });
			dispatcher.drain();
		}
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_accumulate)->RangeMultiplier(2)->Range(1, 1024);

static void bench_dispatcher_expired_batch(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		Dispatcher dispatcher;
		int count = 0;

		for (int64_t i = 0; i < taskCount; ++i) {
			dispatcher.addTask(0u, [&] { ++count; });
		}
		dispatcher.drain();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_dispatcher_expired_batch)->RangeMultiplier(4)->Range(1, 4096);

// ─── Combined Scheduler + Dispatcher benchmarks ─────────────────────────────
//
// Model the full old pipeline: createSchedulerTask → addEvent → io_context
// fires → dispatcher.addTask → drain.  These are fair counterparts to the
// reactor benchmarks from PR #345.

// Drain all timers that have already fired in the io_context
// and then process the resulting dispatcher tasks.
// For 0ms timers, pollOnce() fires them synchronously — this is
// the same behaviour as the Scheduler thread running one loop iteration.
static void drainSchedulerToDispatcher(Scheduler& scheduler, Dispatcher& dispatcher)
{
	scheduler.pollOnce();
	dispatcher.drain();
}

// ─── single-task baselines ───────────────────────────────────────────────────

static void bench_combined_schedule(benchmark::State& state)
{
	for (auto&& _ : state) {
		Dispatcher dispatcher;
		Scheduler scheduler(dispatcher);
		int count = 0;

		scheduler.addEvent(createSchedulerTask(0ms, [&] {
			dispatcher.addTask([&] { ++count; });
		}));

		drainSchedulerToDispatcher(scheduler, dispatcher);
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_combined_schedule);

// ─── batch throughput ────────────────────────────────────────────────────────

static void bench_combined_schedule_batch(benchmark::State& state)
{
	const int64_t batchSize = state.range(0);

	for (auto&& _ : state) {
		Dispatcher dispatcher;
		Scheduler scheduler(dispatcher);
		int count = 0;

		for (int64_t i = 0; i < batchSize; ++i) {
			scheduler.addEvent(createSchedulerTask(0ms, [&] {
				dispatcher.addTask([&] { ++count; });
			}));
		}

		drainSchedulerToDispatcher(scheduler, dispatcher);
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_combined_schedule_batch)->RangeMultiplier(2)->Range(1, 8192);

// ─── mixed (2/3 addTask, 1/3 schedule) ───────────────────────────────────────

static void bench_combined_mixed(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		Dispatcher dispatcher;
		Scheduler scheduler(dispatcher);
		int count = 0;

		for (int64_t i = 0; i < taskCount; ++i) {
			if (i % 3 == 0) {
				scheduler.addEvent(createSchedulerTask(0ms, [&] {
					dispatcher.addTask([&] { ++count; });
				}));
			} else {
				dispatcher.addTask([&] { ++count; });
			}
		}

		drainSchedulerToDispatcher(scheduler, dispatcher);
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_combined_mixed)->RangeMultiplier(2)->Range(1, 8192);

// ─── cancellation ────────────────────────────────────────────────────────────

static void bench_combined_cancel_all(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		Dispatcher dispatcher;
		Scheduler scheduler(dispatcher);
		int count = 0;

		std::vector<uint32_t> ids;
		ids.reserve(taskCount);
		for (int64_t i = 0; i < taskCount; ++i) {
			ids.push_back(scheduler.addEvent(createSchedulerTask(3600000ms, [&] {
				dispatcher.addTask([&] { ++count; });
			})));
		}

		for (auto id : ids) {
			scheduler.stopEvent(id);
		}

		dispatcher.addTask([&] { ++count; });
		drainSchedulerToDispatcher(scheduler, dispatcher);
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_combined_cancel_all)->RangeMultiplier(4)->Range(1, 4096);

static void bench_combined_cancel_half(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		Dispatcher dispatcher;
		Scheduler scheduler(dispatcher);
		int count = 0;

		std::vector<uint32_t> ids;
		ids.reserve(taskCount);
		for (int64_t i = 0; i < taskCount; ++i) {
			ids.push_back(scheduler.addEvent(createSchedulerTask(0ms, [&] {
				dispatcher.addTask([&] { ++count; });
			})));
		}

		for (int64_t i = 0; i < taskCount; i += 2) {
			scheduler.stopEvent(ids[i]);
		}

		drainSchedulerToDispatcher(scheduler, dispatcher);
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_combined_cancel_half)->RangeMultiplier(4)->Range(1, 4096);

// ─── heap (timer) pressure ────────────────────────────────────────────────────

static void bench_combined_heap_pressure(benchmark::State& state)
{
	const int64_t pendingCount = state.range(0);

	Dispatcher dispatcher;
	Scheduler scheduler(dispatcher);
	int count = 0;

	for (int64_t i = 0; i < pendingCount; ++i) {
		scheduler.addEvent(createSchedulerTask(3600000ms, [&] {
			dispatcher.addTask([&] { ++count; });
		}));
	}

	for (auto&& _ : state) {
		dispatcher.addTask([&] { ++count; });
		drainSchedulerToDispatcher(scheduler, dispatcher);
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_combined_heap_pressure)->RangeMultiplier(8)->Range(1, 16384);

BENCHMARK_MAIN();
