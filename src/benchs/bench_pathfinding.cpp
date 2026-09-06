#include "../otpch.h"

#include "../pathfinding/pathfinder.h"

#include <benchmark/benchmark.h>

#include <utility>

namespace
{

PathRequest makeRequest(const Position& start, const Position& target)
{
	return PathRequest::to(target)
	    .from(start)
	    .distance(0, 0)
	    .requiringSight(false)
	    .withLineOfSight(false);
}

template <typename TileCostFactory>
void runPathBenchmark(benchmark::State& state, const PathRequest& request, TileCostFactory tileCostFactory)
{
	std::vector<Direction> directions;
	for (auto&& _ : state) {
		const auto status = PathFinder::find(request, tileCostFactory(), directions);
		benchmark::DoNotOptimize(status);
		benchmark::DoNotOptimize(directions.data());
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations());
}

template <typename TileCostFactory, typename SightCheckFactory>
void runPathBenchmarkWithSight(benchmark::State& state, const PathRequest& request,
                               TileCostFactory tileCostFactory, SightCheckFactory sightCheckFactory)
{
	std::vector<Direction> directions;
	for (auto&& _ : state) {
		const auto status = PathFinder::find(request, tileCostFactory(), directions, sightCheckFactory());
		benchmark::DoNotOptimize(status);
		benchmark::DoNotOptimize(directions.data());
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations());
}

template <typename TileCostFactory>
void runPathBenchmarkSearchOnly(benchmark::State& state, const PathRequest& request, TileCostFactory tileCostFactory)
{
	std::vector<Direction> directions;
	for (auto&& _ : state) {
		state.PauseTiming();
		{
			auto tileCost = tileCostFactory();
			state.ResumeTiming();
			const auto status = PathFinder::find(request, std::move(tileCost), directions);
			benchmark::DoNotOptimize(status);
			benchmark::DoNotOptimize(directions.data());
			benchmark::ClobberMemory();
			state.PauseTiming();
		}
		state.ResumeTiming();
	}
	state.SetItemsProcessed(state.iterations());
}

template <typename TileCostFactory, typename SightCheckFactory>
void runPathBenchmarkWithSightSearchOnly(benchmark::State& state, const PathRequest& request,
                                         TileCostFactory tileCostFactory, SightCheckFactory sightCheckFactory)
{
	std::vector<Direction> directions;
	for (auto&& _ : state) {
		state.PauseTiming();
		{
			auto tileCost = tileCostFactory();
			auto sightCheck = sightCheckFactory();
			state.ResumeTiming();
			const auto status = PathFinder::find(request, std::move(tileCost), directions, std::move(sightCheck));
			benchmark::DoNotOptimize(status);
			benchmark::DoNotOptimize(directions.data());
			benchmark::ClobberMemory();
			state.PauseTiming();
		}
		state.ResumeTiming();
	}
	state.SetItemsProcessed(state.iterations());
}

} // namespace

static void bench_pathfinding_same_position(benchmark::State& state)
{
	const auto request = makeRequest(Position(0, 0, 7), Position(0, 0, 7));
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 0; };
	});
}
BENCHMARK(bench_pathfinding_same_position);

static void bench_pathfinding_adjacent_shortcut(benchmark::State& state)
{
	const auto request = PathRequest::to(Position(1, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .distance(0, 1)
	                         .requiringSight(false)
	                         .withLineOfSight(false);
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 0; };
	});
}
BENCHMARK(bench_pathfinding_adjacent_shortcut);

static void bench_pathfinding_short_open(benchmark::State& state)
{
	const auto request = makeRequest(Position(0, 0, 7), Position(5, 0, 7));
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 10; };
	});
}
BENCHMARK(bench_pathfinding_short_open);

static void bench_pathfinding_viewport_edge(benchmark::State& state)
{
	const auto request = makeRequest(Position(0, 0, 7), Position(12, 0, 7));
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 10; };
	});
}
BENCHMARK(bench_pathfinding_viewport_edge);

static void bench_pathfinding_large_search(benchmark::State& state)
{
	const auto request = PathRequest::to(Position(20, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .distance(0, 0)
	                         .maxDistance(64)
	                         .requiringSight(false)
	                         .withLineOfSight(false);
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 10; };
	});
}
BENCHMARK(bench_pathfinding_large_search);

static void bench_pathfinding_obstacle_detour(benchmark::State& state)
{
	const auto request = makeRequest(Position(0, 0, 7), Position(6, 0, 7));
	runPathBenchmark(state, request, [] {
		return [](int32_t x, int32_t y) -> uint16_t {
			if (x == 2 && y >= -2 && y <= 2) {
				return 0;
			}
			return 10;
		};
	});
}
BENCHMARK(bench_pathfinding_obstacle_detour);

static void bench_pathfinding_approach_range(benchmark::State& state)
{
	const auto request = PathRequest::to(Position(5, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .mode(SearchMode::Approach)
	                         .distance(1, 3)
	                         .requiringSight(false)
	                         .withLineOfSight(false);
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 10; };
	});
}
BENCHMARK(bench_pathfinding_approach_range);

static void bench_pathfinding_flee_range(benchmark::State& state)
{
	const auto request = PathRequest::to(Position(5, 0, 7))
	                         .from(Position(4, 0, 7))
	                         .mode(SearchMode::Flee)
	                         .distance(1, 3)
	                         .requiringSight(false)
	                         .withLineOfSight(false);
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 10; };
	});
}
BENCHMARK(bench_pathfinding_flee_range);

static void bench_pathfinding_required_sight(benchmark::State& state)
{
	const auto request = PathRequest::to(Position(5, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .distance(0, 0)
	                         .requiringSight(true)
	                         .withLineOfSight(false);
	runPathBenchmarkWithSight(
	    state, request,
	    [] { return [](int32_t, int32_t) -> uint16_t { return 10; }; },
	    [] { return [](const Position&, const Position&) { return true; }; });
}
BENCHMARK(bench_pathfinding_required_sight);

static void bench_pathfinding_budget_exhaustion(benchmark::State& state)
{
	const auto request = PathRequest::to(Position(4, 0, 7))
	                         .from(Position(0, 2, 7))
	                         .mode(SearchMode::Approach)
	                         .distance(0, 2)
	                         .requiringSight(false)
	                         .withLineOfSight(false);
	runPathBenchmark(state, request, [] {
		return [](int32_t x, int32_t y) -> uint16_t {
			if (x == 2 && y >= 0 && y <= 2) {
				return 0;
			}
			if (y == 2 && x >= 2 && x <= 4) {
				return 0;
			}
			return 10;
		};
	});
}
BENCHMARK(bench_pathfinding_budget_exhaustion);

static void bench_pathfinding_no_path(benchmark::State& state)
{
	const auto request = makeRequest(Position(0, 0, 7), Position(5, 0, 7));
	runPathBenchmark(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 0; };
	});
}
BENCHMARK(bench_pathfinding_no_path);

static void bench_pathfinding_short_open_search_only(benchmark::State& state)
{
	const auto request = makeRequest(Position(0, 0, 7), Position(5, 0, 7));
	runPathBenchmarkSearchOnly(state, request, [] {
		return [](int32_t, int32_t) -> uint16_t { return 10; };
	});
}
BENCHMARK(bench_pathfinding_short_open_search_only);

static void bench_pathfinding_required_sight_search_only(benchmark::State& state)
{
	const auto request = PathRequest::to(Position(5, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .distance(0, 0)
	                         .requiringSight(true)
	                         .withLineOfSight(false);
	runPathBenchmarkWithSightSearchOnly(
	    state, request,
	    [] { return [](int32_t, int32_t) -> uint16_t { return 10; }; },
	    [] { return [](const Position&, const Position&) { return true; }; });
}
BENCHMARK(bench_pathfinding_required_sight_search_only);

static void bench_pathfinding_tile_cost_callback_construction(benchmark::State& state)
{
	for (auto&& _ : state) {
		TileCost callback = [](int32_t, int32_t) -> uint16_t { return 10; };
		benchmark::DoNotOptimize(callback);
	}
}
BENCHMARK(bench_pathfinding_tile_cost_callback_construction);

static void bench_pathfinding_tile_cost_callback_invocation(benchmark::State& state)
{
	TileCost callback = [](int32_t, int32_t) -> uint16_t { return 10; };
	for (auto&& _ : state) {
		const auto value = callback(1, 2);
		benchmark::DoNotOptimize(value);
	}
}
BENCHMARK(bench_pathfinding_tile_cost_callback_invocation);

static void bench_pathfinding_sight_callback_construction(benchmark::State& state)
{
	for (auto&& _ : state) {
		SightCheck callback = [](const Position&, const Position&) { return true; };
		benchmark::DoNotOptimize(callback);
	}
}
BENCHMARK(bench_pathfinding_sight_callback_construction);

static void bench_pathfinding_sight_callback_invocation(benchmark::State& state)
{
	SightCheck callback = [](const Position&, const Position&) { return true; };
	const Position from(1, 2, 7);
	const Position to(3, 4, 7);
	for (auto&& _ : state) {
		const auto visible = callback(from, to);
		benchmark::DoNotOptimize(visible);
	}
}
BENCHMARK(bench_pathfinding_sight_callback_invocation);

BENCHMARK_MAIN();
