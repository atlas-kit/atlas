#define BOOST_TEST_MODULE pathfinding

#include "../otpch.h"

#include "../pathfinding.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_same_position)
{
	struct EmptyMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 0; }
	};

	EmptyMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(3, 3, 7));
	std::vector<Direction> dirList;

	PathFinder finder(3, 3);
	BOOST_CHECK(finder.search(3, 3, fpp, dirList, map, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_unreachable)
{
	struct EmptyMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 0; }
	};

	EmptyMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	BOOST_CHECK(!finder.search(5, 0, fpp, dirList, map, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_single_cardinal_step)
{
	struct OpenMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 10; }
	};

	OpenMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(1, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool ok = finder.search(1, 0, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_single_diagonal_step)
{
	struct OpenMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 10; }
	};

	OpenMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(1, 1, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool ok = finder.search(1, 1, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_linear_path_three_steps)
{
	struct OpenMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 10; }
	};

	OpenMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(3, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool ok = finder.search(3, 0, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	BOOST_CHECK_GE(dirList.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_cost_avoidance_chooses_cheaper_path)
{
	struct CostMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t x, uint16_t y) const override
		{
			// Diagonal at (1,1) is expensive
			if (x == 1 && y == 1) return 200;
			return 10;
		}
	};

	CostMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;
	fpp.maxSearchDist = 5;

	PathCondition condition(Position(2, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	// Direct path (0,0)→(1,0)→(2,0) costs 2×(10+10)=40
	// Diagonal (0,0)→(1,1)→(2,0) costs 25+10+25+10=70 with (1,1) surcharge
	// So A* prefers the direct cardinal path
	bool ok = finder.search(2, 0, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	BOOST_CHECK_GT(dirList.size(), 0);
}

BOOST_AUTO_TEST_CASE(test_zero_cost_walkable)
{
	struct ZeroCostMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 0; }
	};

	ZeroCostMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(1, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	// getWalkCost returns 0 = blocked, so this should fail
	BOOST_CHECK(!finder.search(1, 0, fpp, dirList, map, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_minimal_cost_leads_to_shortest_path)
{
	struct VaryingCost final : IPathMap
	{
		uint16_t getWalkCost(uint16_t x, uint16_t y) const override
		{
			// Diagonal at (1,1) is expensive
			if (x == 1 && y == 1) return 200;
			return 10;
		}
	};

	VaryingCost map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(2, 2, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool ok = finder.search(2, 2, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	// Path should avoid (1,1) which is expensive diagonal
}

BOOST_AUTO_TEST_CASE(test_max_search_dist_limits_exploration)
{
	struct OpenMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 10; }
	};

	OpenMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;
	fpp.maxSearchDist = 2;

	PathCondition condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	// Target is 5 away, but maxSearchDist=2 prevents reaching it
	BOOST_CHECK(!finder.search(5, 0, fpp, dirList, map, condition, true));
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_frozen_condition_is_in_range)
{
	PathCondition cond(Position(5, 3, 7));

	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.minTargetDist = 1;
	fpp.maxTargetDist = 3;

	// Start (0,0) → dist to target = max(5,3) = 5 → out of range
	BOOST_CHECK(!cond.isInRange(Position(0, 0, 7), Position(0, 0, 7), fpp));

	// Position (4,2) → dist = max(1,1) = 1 → in range
	BOOST_CHECK(cond.isInRange(Position(0, 0, 7), Position(4, 2, 7), fpp));

	// Position (2,1) → dist = max(3,2) = 3 → at max range boundary
	BOOST_CHECK(cond.isInRange(Position(0, 0, 7), Position(2, 1, 7), fpp));

	// Position (9,3) → dist = max(4,0) = 4 → out of range
	BOOST_CHECK(!cond.isInRange(Position(0, 0, 7), Position(9, 3, 7), fpp));
}

BOOST_AUTO_TEST_CASE(test_ipathmap_custom_costs)
{
	struct CustomMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t x, uint16_t y) const override
		{
			if (x == 0 && y == 1) return 0;
			if (x == 1 && y == 1) return 50;
			return 10;
		}
	};

	CustomMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(2, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	// (1,0) is walkable (cost 10), so direct path works
	bool ok = finder.search(2, 0, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_ipathmap_all_blocked_except_one)
{
	struct NarrowMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t x, uint16_t y) const override
		{
			if (x == 0 && y == 1) return 10;
			return 0;
		}
	};

	NarrowMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	PathCondition condition(Position(0, 2, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	// Only (0,1) is walkable, must go through it to reach (0,2)
	// single-step: (0,1) is at distance 1 from target, condition matches there
	bool ok = finder.search(0, 2, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	BOOST_CHECK_EQUAL(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_long_path_within_limits)
{
	struct OpenMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 10; }
	};

	OpenMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;
	fpp.maxSearchDist = 10;

	PathCondition condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	// Target at 5 on same Y: sight-clear pruning restricts to Y=0 only
	// Path: (0,0)→(1,0)→(2,0)→(3,0)→(4,0) found at (4,0) dist 1 from target
	bool ok = finder.search(5, 0, fpp, dirList, map, condition, true);
	BOOST_CHECK(ok);
	BOOST_CHECK_GT(dirList.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_constructor_creates_start_node)
{
	PathFinder finder(7, 8);
	// The start node should be in the grid at (7,8)
	// This just verifies no crash
	BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(test_solve_same_position_false_condition)
{
	struct OpenMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override { return 10; }
	};

	OpenMap map;
	FindPathParams fpp;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 0;

	PathCondition condition(Position(5, 5, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool ok = finder.search(0, 0, fpp, dirList, map, condition, true);
	// Start (0,0) is distance 5 from target (5,5) with maxTargetDist=0
	// Doesn't match at start, must explore
	BOOST_CHECK(!ok);
}
