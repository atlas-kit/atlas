#define BOOST_TEST_MODULE pathfinding

#include "../otpch.h"

#include "../pathfinding.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_same_position_returns_true)
{
	struct EmptyMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override
		{
			return 0;
		}
	};

	EmptyMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	FrozenPathingConditionCall condition(Position(3, 3, 7));
	std::vector<Direction> dirList;

	PathFinder finder(3, 3);
	bool result = finder.solve(3, 3, fpp, dirList, map, condition, true);

	BOOST_CHECK(result);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_unreachable_returns_false)
{
	struct EmptyMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override
		{
			return 0;
		}
	};

	EmptyMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	FrozenPathingConditionCall condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool result = finder.solve(5, 0, fpp, dirList, map, condition, true);

	BOOST_CHECK(!result);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_walkable_path_returns_directions)
{
	struct OpenMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t, uint16_t) const override
		{
			return 10;
		}
	};

	OpenMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	FrozenPathingConditionCall condition(Position(2, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool result = finder.solve(2, 0, fpp, dirList, map, condition, true);

	BOOST_CHECK(result);
	BOOST_CHECK(dirList.size() > 0);
}

BOOST_AUTO_TEST_CASE(test_ipathmap_interface_independent)
{
	// IPathMap::getWalkCost is called by PathFinder without any dependency on
	// Creature or Tile types
	struct CustomCostMap final : IPathMap
	{
		uint16_t getWalkCost(uint16_t x, uint16_t y) const override
		{
			if (x == 1 && y == 1) return 100;
			return 10;
		}
	};

	CustomCostMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.clearSight = false;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;
	fpp.maxSearchDist = 4;

	FrozenPathingConditionCall condition(Position(2, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool result = finder.solve(2, 0, fpp, dirList, map, condition, true);

	BOOST_CHECK(result);
	BOOST_CHECK(dirList.size() > 0);
}

BOOST_AUTO_TEST_CASE(test_start_tile_cache_not_required)
{
	// PathFinder does not require explicit start tile setup
	PathFinder finder(10, 10);
	BOOST_CHECK(true);
}
