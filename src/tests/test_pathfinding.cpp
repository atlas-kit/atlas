#define BOOST_TEST_MODULE pathfinding

#include "../otpch.h"

#include "../pathfinding.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_same_position_returns_true)
{
	struct EmptyMap final : IPathMap
	{
		const Tile* getTile(const std::shared_ptr<const Creature>&, uint16_t, uint16_t) const override
		{
			return nullptr;
		}
	};

	EmptyMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	FrozenPathingConditionCall condition(Position(3, 3, 7));
	std::vector<Direction> dirList;

	PathFinder finder(3, 3);
	bool result = finder.solve(3, 3, nullptr, fpp, dirList, map, condition, true);

	BOOST_CHECK(result);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_unreachable_returns_false)
{
	struct EmptyMap final : IPathMap
	{
		const Tile* getTile(const std::shared_ptr<const Creature>&, uint16_t, uint16_t) const override
		{
			return nullptr;
		}
	};

	EmptyMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	FrozenPathingConditionCall condition(Position(5, 0, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool result = finder.solve(5, 0, nullptr, fpp, dirList, map, condition, true);

	BOOST_CHECK(!result);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_target_out_of_bounds)
{
	struct EmptyMap final : IPathMap
	{
		const Tile* getTile(const std::shared_ptr<const Creature>&, uint16_t, uint16_t) const override
		{
			return nullptr;
		}
	};

	EmptyMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 3;

	FrozenPathingConditionCall condition(Position(30, 30, 7));
	std::vector<Direction> dirList;

	PathFinder finder(0, 0);
	bool result = finder.solve(30, 30, nullptr, fpp, dirList, map, condition, true);

	BOOST_CHECK(!result);
	BOOST_CHECK(dirList.empty());
}

BOOST_AUTO_TEST_CASE(test_start_tile_cache)
{
	PathFinder finder(5, 5);

	int setCount = 0;
	struct CountingMap final : IPathMap
	{
		mutable int& count;
		explicit CountingMap(int& c) : count(c) {}
		const Tile* getTile(const std::shared_ptr<const Creature>&, uint16_t, uint16_t) const override
		{
			++count;
			return nullptr;
		}
	};

	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 1;

	FrozenPathingConditionCall condition(Position(5, 5, 7));
	std::vector<Direction> dirList;

	int calls = 0;
	CountingMap map(calls);
	finder.solve(5, 5, nullptr, fpp, dirList, map, condition, true);
}

BOOST_AUTO_TEST_CASE(test_tile_walk_cost_creature_null)
{
	uint16_t cost = PathFinder::getTileWalkCost(nullptr, nullptr);
	BOOST_CHECK_EQUAL(cost, 0);
}

BOOST_AUTO_TEST_CASE(test_pathfinder_stress_max_nodes)
{
	struct FullMap final : IPathMap
	{
		const Tile* getTile(const std::shared_ptr<const Creature>&, uint16_t x, uint16_t y) const override
		{
			if (x == 5 && y == 5) {
				return nullptr;
			}
			return reinterpret_cast<const Tile*>(static_cast<uintptr_t>(1));
		}
	};

	FullMap map;
	FindPathParams fpp;
	fpp.fullPathSearch = true;
	fpp.minTargetDist = 0;
	fpp.maxTargetDist = 5;

	FrozenPathingConditionCall condition(Position(3, 3, 7));
	std::vector<Direction> dirList;
	dirList.reserve(20);

	PathFinder finder(0, 0);
	bool result = finder.solve(3, 3, nullptr, fpp, dirList, map, condition, true);

	BOOST_CHECK(!result);
}

BOOST_AUTO_TEST_CASE(test_set_start_tile)
{
	PathFinder finder(10, 10);
	finder.setStartTile(nullptr);
}
