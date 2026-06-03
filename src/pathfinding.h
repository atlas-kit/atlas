#pragma once

#include "position.h"

#include <cstdint>
#include <vector>

struct FindPathParams
{
	bool fullPathSearch = true;
	bool clearSight = true;
	bool allowDiagonal = true;
	bool keepDistance = false;
	bool summonTargetMaster = false;
	int32_t maxSearchDist = 0;
	int32_t minTargetDist = -1;
	int32_t maxTargetDist = -1;
};

class FrozenPathingConditionCall
{
public:
	explicit FrozenPathingConditionCall(Position targetPos) : targetPos(std::move(targetPos)) {}

	bool operator()(const Position& startPos, const Position& testPos, const FindPathParams& fpp,
	                int32_t& bestMatchDist) const;

	bool isInRange(const Position& startPos, const Position& testPos, const FindPathParams& fpp) const;

private:
	Position targetPos;
};

static constexpr int32_t PATHFIND_VIEWPORT_X = 11;
static constexpr int32_t PATHFIND_VIEWPORT_Y = 11;
static constexpr int32_t PATHFIND_RESERVE = (PATHFIND_VIEWPORT_X * PATHFIND_VIEWPORT_Y * 3) / 2;

struct AStarNode
{
	AStarNode* parent;
	uint16_t x, y;
	uint16_t g, f;
};

class IPathMap
{
public:
	virtual ~IPathMap() = default;
	virtual uint16_t getWalkCost(uint16_t x, uint16_t y) const = 0;
};

class PathFinder
{
public:
	PathFinder(uint16_t startX, uint16_t startY);

	bool solve(uint16_t targetX, uint16_t targetY, const FindPathParams& fpp, std::vector<Direction>& dirList,
	           const IPathMap& map, const FrozenPathingConditionCall& condition, bool sightClear);

private:
	static constexpr int32_t GRID_W = PATHFIND_VIEWPORT_X * 2 + 1;
	static constexpr int32_t GRID_H = PATHFIND_VIEWPORT_Y * 2 + 1;
	static constexpr int32_t MAX_NODES = PATHFIND_VIEWPORT_X * PATHFIND_VIEWPORT_Y;

	static constexpr std::pair<int8_t, int8_t> NEIGHBORS[8] = {{-1, 0},  {0, 1},  {1, 0}, {0, -1},
	                                                           {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

	struct Cell
	{
		AStarNode node;
		uint8_t state;
		uint16_t tileCost;
	};

	Cell grid[GRID_H][GRID_W];
	AStarNode* openList[MAX_NODES];
	int32_t openCount;
	int32_t startX, startY;

	Cell* cellAt(uint16_t x, uint16_t y);
	AStarNode* getNodeByPosition(uint16_t x, uint16_t y) const;
	AStarNode* getBestNode();
	void reconstructPath(AStarNode* node, uint16_t endX, uint16_t endY, std::vector<Direction>& dirList) const;

	static uint16_t heuristic(uint16_t x, uint16_t y, uint16_t targetX, uint16_t targetY);
};
