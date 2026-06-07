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

// ============================================================================
// Combined dispatcher + scheduler equivalents
//
// The old two-thread system (PR #355) measured only the Dispatcher thread.
// The Scheduler thread carried additional overhead that was never benchmarked:
//   - one boost::asio::steady_timer allocation per scheduled event
//   - a cross-thread post (io_context → Dispatcher queue) on timer expiry
//   - a condition-variable wake on the Dispatcher side
//
// The benchmarks below model the *combined* workload so results can be
// compared fairly against PR #355's dispatcher-only numbers.
// ============================================================================

// Models N producer threads concurrently posting immediate work to the reactor,
// equivalent to the old pattern where multiple network-I/O threads called
// g_dispatcher.addTask() simultaneously.
static void bench_reactor_concurrent_send(benchmark::State& state)
{
	const int64_t producerCount = state.range(0);

	for (auto&& _ : state) {
		TaskReactor reactor;
		std::atomic<int64_t> count{0};

		// Each "producer thread" enqueues one task (no actual threads: we
		// measure the enqueue + drain cost, which is what the dispatcher
		// benchmark in PR #355 measures for addTask).
		for (int64_t i = 0; i < producerCount; ++i) {
			reactor.send([&] { count.fetch_add(1, std::memory_order_relaxed); });
		}
		reactor.runOnce();
		benchmark::DoNotOptimize(count.load());
	}
}
BENCHMARK(bench_reactor_concurrent_send)->RangeMultiplier(2)->Range(1, 8192);

// Models the old Scheduler → Dispatcher hop: a scheduled event fires and
// enqueues an immediate task (the pattern used by creature-think, decay,
// spawn-check, Lua addEvent, etc.).  In the old system this cost:
//   schedule thread: timer alloc + async_wait + post-to-dispatcher
//   dispatcher thread: CV wake + dequeue + execute
// Here the reactor handles the whole chain in one runOnce() call.
static void bench_reactor_schedule_chain(benchmark::State& state)
{
	const int64_t chainLength = state.range(0);

	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;

		// Each step: schedule(0ms) fires and sends the next immediate task.
		// This captures the "delayed → immediate" forwarding the old Scheduler
		// performed before handing off to the Dispatcher.
		std::function<void()> chainStep;
		chainStep = [&] {
			++count;
			if (count < static_cast<int>(chainLength)) {
				reactor.send(chainStep);
			}
		};
		reactor.schedule(0ms, chainStep);

		for (int i = 0; i < chainLength; ++i) {
			reactor.runOnce();
		}
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_schedule_chain)->RangeMultiplier(2)->Range(1, 256);

// Combined workload: concurrent immediate sends (dispatcher load) mixed with
// scheduled events that expire in the same drain (scheduler load).
// This is the scenario where the reactor replaces *both* threads at once and
// is the most direct apples-to-apples comparison with PR #355.
static void bench_reactor_combined_dispatcher_scheduler(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	TaskReactor reactor;
	int count = 0;

	for (auto&& _ : state) {
		count = 0;
		// Half the tasks are "dispatcher" style (immediate), half are
		// "scheduler" style (0 ms delay, analogous to the minimum interval
		// the old Scheduler accepted before forwarding to the Dispatcher).
		for (int64_t i = 0; i < taskCount; ++i) {
			if (i % 2 == 0) {
				reactor.send([&] { ++count; });
			} else {
				reactor.schedule(0ms, [&] { ++count; });
			}
		}
		reactor.runOnce();
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_combined_dispatcher_scheduler)->RangeMultiplier(2)->Range(1, 8192);

// Measures the per-task overhead of schedule() itself (identifier allocation,
// heap insert, lock), independent of execution.  The old Scheduler paid a
// comparable cost for each addEvent() call (timer allocation, io_context post).
static void bench_reactor_schedule_overhead(benchmark::State& state)
{
	const int64_t taskCount = state.range(0);

	for (auto&& _ : state) {
		TaskReactor reactor;
		int count = 0;

		for (int64_t i = 0; i < taskCount; ++i) {
			// Use 1h so none of these fire; we're measuring scheduling cost only.
			reactor.schedule(1h, [&] { ++count; });
		}
		benchmark::DoNotOptimize(count);
	}
}
BENCHMARK(bench_reactor_schedule_overhead)->RangeMultiplier(2)->Range(1, 8192);

BENCHMARK_MAIN();
