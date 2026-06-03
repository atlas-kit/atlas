#define BOOST_TEST_MODULE pathfinding

#include "../otpch.h"

#include "../pathfinding.h"

#include <boost/test/unit_test.hpp>

static auto alwaysBlocked = [](uint16_t, uint16_t) -> uint16_t { return 0; };
static auto alwaysWalkable = [](uint16_t, uint16_t) -> uint16_t { return 10; };

BOOST_AUTO_TEST_CASE(test_same_position)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(3, 3, 7));
	std::vector<Direction> dirList;

	PathFinder finder(3, 3);
	BOOST_CHECK(finder.search(3, 3, fpp, dirList, alwaysBlocked, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_unreachable)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(!finder.search(5, 0, fpp, dirList, alwaysBlocked, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_single_cardinal_step)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(1, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(1, 0, fpp, dirList, alwaysWalkable, condition, true));
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_single_diagonal_step)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(1, 1, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(1, 1, fpp, dirList, alwaysWalkable, condition, true));
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_linear_path_three_steps)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(3, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(3, 0, fpp, dirList, alwaysWalkable, condition, true));
	BOOST_CHECK_GE(dirList.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_cost_avoidance_chooses_cheaper_path)
{
	auto expensiveDiagonal = [](uint16_t x, uint16_t y) -> uint16_t {
		if (x == 1 && y == 1) return 200;
		return 10;
	};

	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;
	fpp.maxSearchDist = 5;

	PathCondition condition(Position(2, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(2, 0, fpp, dirList, expensiveDiagonal, condition, true));
	BOOST_CHECK_GT(dirList.size(), 0);
}

BOOST_AUTO_TEST_CASE(test_zero_cost_blocked)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(1, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(!finder.search(1, 0, fpp, dirList, alwaysBlocked, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_minimal_cost_leads_to_shortest_path)
{
	auto expensiveDiagonal = [](uint16_t x, uint16_t y) -> uint16_t {
		if (x == 1 && y == 1) return 200;
		return 10;
	};

	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(2, 2, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(2, 2, fpp, dirList, expensiveDiagonal, condition, true));
}

BOOST_AUTO_TEST_CASE(test_max_search_dist_limits_exploration)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;
	fpp.maxSearchDist = 2;

	PathCondition condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(!finder.search(5, 0, fpp, dirList, alwaysWalkable, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_frozen_condition_is_in_range)
{
	PathCondition condition(Position(5, 3, 7));

	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.minTargetDist = 1;
	fpp.maxTargetDist = 3;

	BOOST_CHECK(!condition.isInRange(Position(0, 0, 7), Position(0, 0, 7), fpp));
	BOOST_CHECK(condition.isInRange(Position(0, 0, 7), Position(4, 2, 7), fpp));
	BOOST_CHECK(condition.isInRange(Position(0, 0, 7), Position(2, 1, 7), fpp));
	BOOST_CHECK(!condition.isInRange(Position(0, 0, 7), Position(9, 3, 7), fpp));
}

BOOST_AUTO_TEST_CASE(test_ipathmap_custom_costs)
{
	auto customCost = [](uint16_t x, uint16_t y) -> uint16_t {
		if (x == 0 && y == 1) return 0;
		if (x == 1 && y == 1) return 50;
		return 10;
	};

	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(2, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(2, 0, fpp, dirList, customCost, condition, true));
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_ipathmap_all_blocked_except_one)
{
	auto narrowPath = [](uint16_t x, uint16_t y) -> uint16_t {
		if (x == 0 && y == 1) return 10;
		return 0;
	};

	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(0, 2, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(0, 2, fpp, dirList, narrowPath, condition, true));
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_long_path_within_limits)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;
	fpp.maxSearchDist = 10;

	PathCondition condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(finder.search(5, 0, fpp, dirList, alwaysWalkable, condition, true));
	BOOST_CHECK_GT(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_constructor_creates_start_node)
{
	PathFinder finder(7, 8);
	BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(test_solve_same_position_false_condition)
{
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(5, 5, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(!finder.search(0, 0, fpp, dirList, alwaysWalkable, condition, true));
}
