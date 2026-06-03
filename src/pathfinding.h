#pragma once

#include <cstdint>
#include <vector>

#include "position.h"

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

	bool solve(uint16_t targetX, uint16_t targetY,
	           const FindPathParams& fpp,
	           std::vector<Direction>& dirList,
	           const IPathMap& map,
	           const FrozenPathingConditionCall& condition,
	           bool sightClear);

private:
	static constexpr int32_t GRID_W = PATHFIND_VIEWPORT_X * 2 + 1;
	static constexpr int32_t GRID_H = PATHFIND_VIEWPORT_Y * 2 + 1;
	static constexpr int32_t MAX_NODES = PATHFIND_VIEWPORT_X * PATHFIND_VIEWPORT_Y;

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

	Cell* getCell(uint16_t x, uint16_t y);
	AStarNode* getNodeByPosition(uint16_t x, uint16_t y) const;
	AStarNode* createNode(AStarNode* parent, uint16_t x, uint16_t y, uint16_t g, uint16_t f);
	AStarNode* getBestNode();
	void reconstructPath(AStarNode* node, uint16_t endX, uint16_t endY, std::vector<Direction>& dirList) const;

	static uint16_t getMapWalkCost(uint16_t fromX, uint16_t fromY, uint16_t toX, uint16_t toY);
	static uint16_t calculateHeuristic(uint16_t x, uint16_t y, uint16_t targetX, uint16_t targetY);
};
