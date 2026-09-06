#define BOOST_TEST_MODULE pathfinding

#include "../otpch.h"

#include "../pathfinding/path_goal.h"
#include "../pathfinding/pathfinder.h"
#include "../pathfinding/search_mode.h"

#include <boost/test/unit_test.hpp>

#include <limits>

namespace
{

uint16_t alwaysWalkable(int32_t, int32_t)
{
	return 10;
}

uint16_t alwaysBlocked(int32_t, int32_t)
{
	return 0;
}

uint16_t walkable(int32_t, int32_t)
{
	return 10;
}

uint16_t blocked(int32_t, int32_t)
{
	return 0;
}

Position walkPath(Position position, const std::vector<Direction>& directions)
{
	for (auto it = directions.rbegin(); it != directions.rend(); ++it) {
		switch (*it) {
			case DIRECTION_NORTH: --position.y; break;
			case DIRECTION_EAST: ++position.x; break;
			case DIRECTION_SOUTH: ++position.y; break;
			case DIRECTION_WEST: --position.x; break;
			case DIRECTION_NORTHWEST: --position.x; --position.y; break;
			case DIRECTION_NORTHEAST: ++position.x; --position.y; break;
			case DIRECTION_SOUTHWEST: --position.x; ++position.y; break;
			case DIRECTION_SOUTHEAST: ++position.x; ++position.y; break;
			default: break;
		}
	}
	return position;
}

int32_t chebyshevDistance(const Position& lhs, const Position& rhs)
{
	return std::max(lhs.getDistanceX(rhs), lhs.getDistanceY(rhs));
}

} // namespace

BOOST_AUTO_TEST_CASE(test_same_position)
{
	const auto request = PathRequest::to(Position(3, 3, 7)).from(Position(3, 3, 7)).distance(0, 1).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysBlocked, dirList) == PathStatus::Found);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_unreachable)
{
	const auto request = PathRequest::to(Position(5, 0, 7)).from(Position(0, 0, 7)).distance(0, 1).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysBlocked, dirList) == PathStatus::NoPath);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_single_cardinal_step)
{
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 1).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysWalkable, dirList) == PathStatus::Found);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_single_diagonal_step)
{
	const auto request = PathRequest::to(Position(2, 2, 7)).from(Position(0, 0, 7)).distance(0, 1).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysWalkable, dirList) == PathStatus::Found);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_linear_path_three_steps)
{
	const auto request = PathRequest::to(Position(3, 0, 7)).from(Position(0, 0, 7)).distance(0, 1).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysWalkable, dirList) == PathStatus::Found);
	BOOST_CHECK_GE(dirList.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_cost_avoidance_chooses_cheaper_path)
{
	auto expensiveStraight = [](int32_t x, int32_t y) -> uint16_t {
		if (x == 1 && y == 0) return 200;
		if (x == 1 && y == -1) return 0;
		return 10;
	};
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0)
	                         .maxDistance(5).requiringSight(false).withLineOfSight(false);
	std::vector<Direction> dirList;
	BOOST_REQUIRE(PathFinder::find(request, expensiveStraight, dirList) == PathStatus::Found);
	BOOST_REQUIRE_EQUAL(dirList.size(), 2);
	BOOST_CHECK_EQUAL(dirList[0], DIRECTION_NORTHEAST);
	BOOST_CHECK_EQUAL(dirList[1], DIRECTION_SOUTHEAST);
}

BOOST_AUTO_TEST_CASE(test_zero_cost_blocked)
{
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysBlocked, dirList) == PathStatus::NoPath);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_minimal_cost_leads_to_shortest_path)
{
	auto expensiveDiagonal = [](int32_t x, int32_t y) -> uint16_t {
		if (x == 1 && y == 1) return 200;
		return 10;
	};
	const auto request = PathRequest::to(Position(2, 2, 7)).from(Position(0, 0, 7)).distance(0, 1)
	                         .requiringSight(false).withLineOfSight(true);
	std::vector<Direction> dirList;
	BOOST_REQUIRE(PathFinder::find(request, expensiveDiagonal, dirList) == PathStatus::Found);
	BOOST_REQUIRE_EQUAL(dirList.size(), 2);
	BOOST_CHECK_EQUAL(dirList[0], DIRECTION_SOUTHEAST);
	BOOST_CHECK_EQUAL(dirList[1], DIRECTION_SOUTH);
}

BOOST_AUTO_TEST_CASE(test_max_search_dist_limits_exploration)
{
	const auto request = PathRequest::to(Position(5, 0, 7)).from(Position(0, 0, 7)).distance(0, 1)
	                         .maxDistance(2).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysWalkable, dirList) == PathStatus::NoPath);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_goal_is_in_range)
{
	PathGoal goal;
	goal.target = Position(5, 3, 7);
	goal.mode = SearchMode::Reach;
	goal.distance = { 1, 3 };

	BOOST_CHECK(!goal.isInRange(0, 0, 0, 0));
	BOOST_CHECK(goal.isInRange(0, 0, 4, 2));
	BOOST_CHECK(goal.isInRange(0, 0, 2, 1));
	BOOST_CHECK(!goal.isInRange(0, 0, 9, 3));
}

BOOST_AUTO_TEST_CASE(test_ipathmap_custom_costs)
{
	auto customCost = [](int32_t x, int32_t y) -> uint16_t {
		if (x == 0 && y == 1) return 0;
		if (x == 1 && y == 1) return 50;
		return 10;
	};
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 1).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, customCost, dirList) == PathStatus::Found);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_ipathmap_all_blocked_except_one)
{
	auto narrowPath = [](int32_t x, int32_t y) -> uint16_t {
		if (x == 0 && y == 1) return 10;
		return 0;
	};
	const auto request = PathRequest::to(Position(0, 2, 7)).from(Position(0, 0, 7)).distance(0, 1).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, narrowPath, dirList) == PathStatus::Found);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_long_path_within_limits)
{
	const auto request = PathRequest::to(Position(5, 0, 7)).from(Position(0, 0, 7)).distance(0, 1)
	                         .maxDistance(10).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysWalkable, dirList) == PathStatus::Found);
	BOOST_CHECK_GT(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_goal_at_start_position)
{
	const auto request = PathRequest::to(Position(7, 8, 7)).from(Position(7, 8, 7)).distance(0, 0).requiringSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, alwaysBlocked, dirList) == PathStatus::Found);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_pruned_path_to_exact_target)
{
	const auto request = PathRequest::to(Position(5, 5, 7)).from(Position(0, 0, 7)).distance(0, 0)
	                         .requiringSight(false).withLineOfSight(true);
	std::vector<Direction> dirList;
	BOOST_REQUIRE(PathFinder::find(request, alwaysWalkable, dirList) == PathStatus::Found);
	BOOST_REQUIRE(!dirList.empty());
	const auto end = walkPath(Position(0, 0, 7), dirList);
	BOOST_CHECK_EQUAL(end.x, 5);
	BOOST_CHECK_EQUAL(end.y, 5);
}

BOOST_AUTO_TEST_CASE(test_no_path_when_search_budget_exhausted)
{
	auto blockedExactEdge = [](int32_t x, int32_t y) -> uint16_t {
		if (x == 2 && y >= 0 && y <= 2) return 0;
		if (y == 2 && x >= 2 && x <= 4) return 0;
		return 10;
	};
	const auto request = PathRequest::to(Position(4, 0, 7)).from(Position(0, 2, 7)).mode(SearchMode::Approach)
	                         .distance(0, 2).requiringSight(false).withLineOfSight(false);
	std::vector<Direction> dirList;
	BOOST_CHECK(PathFinder::find(request, blockedExactEdge, dirList) == PathStatus::NoPath);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_reaches_twelve_tile_edge)
{
	const auto request = PathRequest::to(Position(12, 0, 7)).from(Position(0, 0, 7)).distance(0, 0)
	                         .requiringSight(false).withLineOfSight(false);
	std::vector<Direction> dirList;
	BOOST_REQUIRE(PathFinder::find(request, alwaysWalkable, dirList) == PathStatus::Found);
	BOOST_CHECK_EQUAL(dirList.size(), 12);
	const auto end = walkPath(Position(0, 0, 7), dirList);
	BOOST_CHECK_EQUAL(end.x, 12);
	BOOST_CHECK_EQUAL(end.y, 0);
}

BOOST_AUTO_TEST_CASE(different_floors_return_no_path_and_clear_output)
{
	std::vector<Direction> directions{DIRECTION_EAST};
	const auto request = PathRequest::to(Position(2, 0, 8)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, walkable, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(default_axis_limit_rejects_distant_target)
{
	std::vector<Direction> directions;
	const auto request = PathRequest::to(Position(13, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, walkable, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(explicit_search_limit_rejects_target_before_search)
{
	std::vector<Direction> directions;
	const auto request = PathRequest::to(Position(5, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).maxDistance(2).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, walkable, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(negative_search_limit_does_not_search)
{
	std::vector<Direction> directions;
	const auto request = PathRequest::to(Position(5, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).maxDistance(-1).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, walkable, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(inverted_distance_range_returns_no_path)
{
	std::vector<Direction> directions;
	const auto request = PathRequest::to(Position(5, 0, 7)).from(Position(0, 0, 7)).distance(3, 1).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, walkable, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(required_sight_without_callback_returns_no_path)
{
	std::vector<Direction> directions;
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0);
	BOOST_CHECK(PathFinder::find(request, walkable, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(rejected_sight_returns_no_path)
{
	std::vector<Direction> directions;
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0);
	auto sightBlocked = [](const Position&, const Position&) { return false; };
	BOOST_CHECK(PathFinder::find(request, walkable, directions, std::move(sightBlocked)) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(disabled_sight_does_not_invoke_callback)
{
	std::vector<Direction> directions;
	int sightCalls = 0;
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	auto sight = [&sightCalls](const Position&, const Position&) {
		++sightCalls;
		return false;
	};
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions, std::move(sight)) == PathStatus::Found);
	BOOST_CHECK_EQUAL(sightCalls, 0);
}

BOOST_AUTO_TEST_CASE(blocked_target_returns_no_path)
{
	std::vector<Direction> directions;
	auto targetBlocked = [](int32_t x, int32_t y) -> uint16_t { return x == 2 && y == 0 ? 0 : 10; };
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, targetBlocked, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(start_tile_is_not_required_to_have_positive_cost)
{
	std::vector<Direction> directions;
	auto blockedStart = [](int32_t x, int32_t y) -> uint16_t { return x == 0 && y == 0 ? 0 : 10; };
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, blockedStart, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(directions.size(), 2);
}

BOOST_AUTO_TEST_CASE(adjacent_target_uses_dance_step_shortcut)
{
	std::vector<Direction> directions;
	const auto request = PathRequest::to(Position(1, 0, 7)).from(Position(0, 0, 7)).distance(0, 1).requiringSight(false);
	BOOST_CHECK(PathFinder::find(request, blocked, directions) == PathStatus::Found);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(expected_overload_returns_value_or_error)
{
	const auto foundRequest = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	const auto noPathRequest = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	auto found = PathFinder::find(foundRequest, walkable);
	auto noPath = PathFinder::find(noPathRequest, blocked);
	BOOST_REQUIRE(found.has_value());
	BOOST_CHECK_EQUAL(found->size(), 2);
	BOOST_REQUIRE(!noPath.has_value());
	BOOST_CHECK(noPath.error() == PathStatus::NoPath);
}

BOOST_AUTO_TEST_CASE(reach_mode_finds_exact_target)
{
	const Position start(0, 0, 7);
	const Position target(4, 0, 7);
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Reach).distance(0, 0).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	const auto end = walkPath(start, directions);
	BOOST_CHECK_EQUAL(end.x, target.x);
	BOOST_CHECK_EQUAL(end.y, target.y);
	BOOST_CHECK_EQUAL(end.z, target.z);
}

BOOST_AUTO_TEST_CASE(reach_mode_stops_at_requested_maximum_distance)
{
	const Position start(0, 0, 7);
	const Position target(5, 0, 7);
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Reach).distance(1, 3).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(chebyshevDistance(walkPath(start, directions), target), 3);
}

BOOST_AUTO_TEST_CASE(approach_mode_reaches_near_side_of_target)
{
	const Position start(0, 0, 7);
	const Position target(5, 0, 7);
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Approach).distance(1, 3).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(chebyshevDistance(walkPath(start, directions), target), 3);
}

BOOST_AUTO_TEST_CASE(approach_mode_respects_direction_from_right_side)
{
	const Position start(10, 0, 7);
	const Position target(5, 0, 7);
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Approach).distance(1, 3).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(chebyshevDistance(walkPath(start, directions), target), 3);
}

BOOST_AUTO_TEST_CASE(flee_mode_moves_to_the_far_edge_of_range)
{
	const Position start(4, 0, 7);
	const Position target(5, 0, 7);
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Flee).distance(1, 3).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	const auto end = walkPath(start, directions);
	BOOST_CHECK_EQUAL(chebyshevDistance(end, target), 3);
	BOOST_CHECK_LT(end.x, start.x);
}

BOOST_AUTO_TEST_CASE(minimum_distance_is_enforced)
{
	const Position start(0, 0, 7);
	const Position target(4, 0, 7);
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Reach).distance(2, 3).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	const auto distance = chebyshevDistance(walkPath(start, directions), target);
	BOOST_CHECK_GE(distance, 2);
	BOOST_CHECK_LE(distance, 3);
}

BOOST_AUTO_TEST_CASE(line_of_sight_pruning_still_reaches_diagonal_target)
{
	const Position start(0, 0, 7);
	const Position target(5, 5, 7);
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Reach).distance(0, 0).requiringSight(false).withLineOfSight(true);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	const auto end = walkPath(start, directions);
	BOOST_CHECK_EQUAL(end.x, target.x);
	BOOST_CHECK_EQUAL(end.y, target.y);
	BOOST_CHECK_EQUAL(end.z, target.z);
}

BOOST_AUTO_TEST_CASE(no_path_is_returned_when_fleeing_has_no_reachable_edge)
{
	const Position start(0, 0, 7);
	const Position target(10, 0, 7);
	auto onlyStart = [start](int32_t x, int32_t y) -> uint16_t { return x == start.x && y == start.y ? 10 : 0; };
	const auto request = PathRequest::to(target).from(start).mode(SearchMode::Flee).distance(1, 3).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_CHECK(PathFinder::find(request, onlyStart, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(storage_is_reusable_after_a_large_search)
{
	const auto largeRequest = PathRequest::to(Position(20, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).maxDistance(64).requiringSight(false);
	std::vector<Direction> largePath;
	BOOST_REQUIRE(PathFinder::find(largeRequest, walkable, largePath) == PathStatus::Found);
	BOOST_CHECK_EQUAL(largePath.size(), 20);
	auto shortCorridor = [](int32_t x, int32_t y) -> uint16_t { return y == 0 && x >= 1 && x <= 2 ? 10 : 0; };
	const auto shortRequest = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).maxDistance(2).requiringSight(false);
	std::vector<Direction> shortPath;
	BOOST_REQUIRE(PathFinder::find(shortRequest, shortCorridor, shortPath) == PathStatus::Found);
	BOOST_CHECK_EQUAL(shortPath.size(), 2);
}

BOOST_AUTO_TEST_CASE(failed_search_does_not_poison_the_next_search)
{
	const auto request = PathRequest::to(Position(3, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_CHECK(PathFinder::find(request, [](int32_t, int32_t) -> uint16_t { return 0; }, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(directions.size(), 3);
}

BOOST_AUTO_TEST_CASE(output_is_replaced_on_success_after_previous_path)
{
	const auto firstRequest = PathRequest::to(Position(3, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	const auto secondRequest = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(firstRequest, walkable, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(directions.size(), 3);
	BOOST_REQUIRE(PathFinder::find(secondRequest, walkable, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(directions.size(), 2);
}

BOOST_AUTO_TEST_CASE(tile_cost_zero_is_the_only_blocked_value)
{
	auto positiveCost = [](int32_t x, int32_t y) -> uint16_t {
		if (x == 1 && y == 0) return 1;
		return x == 2 && y == 0 ? 1 : 0;
	};
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_REQUIRE(PathFinder::find(request, positiveCost, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(directions.size(), 2);
}

BOOST_AUTO_TEST_CASE(maximum_tile_cost_does_not_overflow_path_cost)
{
	auto expensive = [](int32_t, int32_t) -> uint16_t { return std::numeric_limits<uint16_t>::max(); };
	const auto request = PathRequest::to(Position(2, 0, 7)).from(Position(0, 0, 7)).distance(0, 0).requiringSight(false);
	std::vector<Direction> directions;
	BOOST_CHECK(PathFinder::find(request, expensive, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(directions.size(), 2);
}

BOOST_AUTO_TEST_CASE(explicit_search_limit_allows_target_beyond_default_viewport)
{
	const auto request = PathRequest::to(Position(13, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .distance(0, 0)
	                         .maxDistance(13)
	                         .requiringSight(false)
	                         .withLineOfSight(false);
	std::vector<Direction> directions;

	BOOST_REQUIRE(PathFinder::find(request, walkable, directions) == PathStatus::Found);
	BOOST_CHECK_EQUAL(directions.size(), 13);
	const auto end = walkPath(Position(0, 0, 7), directions);
	BOOST_CHECK_EQUAL(end.x, 13);
	BOOST_CHECK_EQUAL(end.y, 0);
}

BOOST_AUTO_TEST_CASE(negative_maximum_target_distance_returns_no_path)
{
	const auto request = PathRequest::to(Position(5, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .distance(0, -1)
	                         .requiringSight(false);
	std::vector<Direction> directions;

	BOOST_CHECK(PathFinder::find(request, walkable, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(accepted_sight_callback_is_called_with_target)
{
	int sightCalls = 0;
	int32_t lastFromX = -1;
	int32_t lastFromY = -1;
	int32_t lastToX = -1;
	int32_t lastToY = -1;
	auto sight = [&](const Position& from, const Position& to) {
		++sightCalls;
		lastFromX = from.x;
		lastFromY = from.y;
		lastToX = to.x;
		lastToY = to.y;
		return true;
	};
	const auto request = PathRequest::to(Position(2, 0, 7))
	                         .from(Position(0, 0, 7))
	                         .distance(0, 0)
	                         .requiringSight(true)
	                         .withLineOfSight(false);
	std::vector<Direction> directions;

	BOOST_REQUIRE(PathFinder::find(request, walkable, directions, std::move(sight)) == PathStatus::Found);
	BOOST_CHECK_GT(sightCalls, 0);
	BOOST_CHECK_EQUAL(lastFromX, 2);
	BOOST_CHECK_EQUAL(lastFromY, 0);
	BOOST_CHECK_EQUAL(lastToX, 2);
	BOOST_CHECK_EQUAL(lastToY, 0);
}

BOOST_AUTO_TEST_CASE(best_effort_match_returns_partial_when_exact_ring_is_blocked)
{
	const Position start(3, 0, 7);
	const Position target(5, 0, 7);
	auto blockExactRing = [target](int32_t x, int32_t y) -> uint16_t {
		return std::max(std::abs(x - static_cast<int32_t>(target.x)), std::abs(y - static_cast<int32_t>(target.y))) == 3 ? 0 : 10;
	};
	const auto request = PathRequest::to(target)
	                         .from(start)
	                         .mode(SearchMode::Reach)
	                         .distance(1, 3)
	                         .requiringSight(false)
	                         .withLineOfSight(false);
	std::vector<Direction> directions;

	BOOST_CHECK(PathFinder::find(request, blockExactRing, directions) == PathStatus::Partial);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(failed_search_clears_previous_successful_output)
{
	const auto successRequest = PathRequest::to(Position(2, 0, 7))
	                                  .from(Position(0, 0, 7))
	                                  .distance(0, 0)
	                                  .requiringSight(false);
	const auto failureRequest = PathRequest::to(Position(5, 0, 7))
	                                  .from(Position(0, 0, 7))
	                                  .distance(0, 0)
	                                  .requiringSight(false);
	std::vector<Direction> directions;

	BOOST_REQUIRE(PathFinder::find(successRequest, walkable, directions) == PathStatus::Found);
	BOOST_REQUIRE(!directions.empty());
	BOOST_CHECK(PathFinder::find(failureRequest, blocked, directions) == PathStatus::NoPath);
	BOOST_CHECK(directions.empty());
}

BOOST_AUTO_TEST_CASE(expected_overload_accepts_partial_path)
{
	const Position start(3, 0, 7);
	const Position target(5, 0, 7);
	auto blockExactRing = [target](int32_t x, int32_t y) -> uint16_t {
		return std::max(std::abs(x - static_cast<int32_t>(target.x)), std::abs(y - static_cast<int32_t>(target.y))) == 3 ? 0 : 10;
	};
	const auto request = PathRequest::to(target)
	                         .from(start)
	                         .mode(SearchMode::Reach)
	                         .distance(1, 3)
	                         .requiringSight(false)
	                         .withLineOfSight(false);

	auto result = PathFinder::find(request, std::move(blockExactRing));
	BOOST_REQUIRE(result.has_value());
	BOOST_CHECK(result->empty());
}
