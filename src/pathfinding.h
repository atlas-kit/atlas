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

class PathCondition
{
public:
	explicit PathCondition(Position targetPosition) : targetPosition(std::move(targetPosition)) {}

	bool operator()(const Position& startPosition, const Position& testPosition, const FindPathParams& parameters,
	                int32_t& bestMatchDistance) const;

	bool isInRange(const Position& startPosition, const Position& testPosition, const FindPathParams& parameters) const;

private:
	Position targetPosition;
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

	bool search(uint16_t targetX, uint16_t targetY, const FindPathParams& parameters,
	            std::vector<Direction>& directionList, const IPathMap& map, const PathCondition& condition,
	            bool sightClear);

private:
	static constexpr int32_t gridWidth = PATHFIND_VIEWPORT_X * 2 + 1;
	static constexpr int32_t gridHeight = PATHFIND_VIEWPORT_Y * 2 + 1;
	static constexpr int32_t maximumNodes = PATHFIND_VIEWPORT_X * PATHFIND_VIEWPORT_Y;

	static constexpr std::pair<int8_t, int8_t> neighbourOffsets[8] = {{-1, 0},  {0, 1},  {1, 0}, {0, -1},
	                                                                  {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

	enum class CellState : uint8_t
	{
		Empty = 0,
		Open = 1,
		Closed = 2,
	};

	struct Cell
	{
		AStarNode node;
		CellState state = CellState::Empty;
		uint16_t tileCost = 0;
	};

	Cell grid[gridHeight][gridWidth];
	AStarNode* openList[maximumNodes];
	int32_t openListSize;
	const int32_t startX;
	const int32_t startY;

	Cell* getCell(uint16_t x, uint16_t y);
	AStarNode* getNodeAt(uint16_t x, uint16_t y) const;

	AStarNode* popBestNode();
	void reconstructPath(AStarNode* node, uint16_t endX, uint16_t endY, std::vector<Direction>& directionList) const;

	static uint16_t heuristic(uint16_t x, uint16_t y, uint16_t targetX, uint16_t targetY);
};
