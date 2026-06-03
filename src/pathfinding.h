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

	bool isInRange(const Position& startPosition, const Position& testPosition,
	               const FindPathParams& parameters) const;

private:
	Position targetPosition;
};

static constexpr uint16_t PATHFIND_NORMAL_COST = 10;
static constexpr uint16_t PATHFIND_DIAGONAL_COST = 25;

static constexpr int32_t PATHFIND_VIEWPORT_X = 11;
static constexpr int32_t PATHFIND_VIEWPORT_Y = 11;
static constexpr int32_t PATHFIND_RESERVE = (PATHFIND_VIEWPORT_X * PATHFIND_VIEWPORT_Y * 3) / 2;

class PathFinder
{
public:
	PathFinder(uint16_t startX, uint16_t startY);

	template<typename F>
	bool search(uint16_t targetX, uint16_t targetY, const FindPathParams& parameters,
	            std::vector<Direction>& directionList, F getTileCost, const PathCondition& condition, bool sightClear)
	{
		const Position startPosition(startX, startY, 0);

		Node* found = nullptr;
		int32_t bestMatchDistance = 0;
		uint16_t iterations = 0;
		uint16_t endX = targetX;
		uint16_t endY = targetY;

		const auto pruningActive =
		    sightClear && !parameters.keepDistance && !parameters.summonTargetMaster && parameters.minTargetDist <= 1;
		const auto sameX = (startX == targetX);
		const auto sameY = (startY == targetY);
		const auto movingForwardX = (startX < targetX);
		const auto movingForwardY = (startY < targetY);

		Node* currentNode = popBestNode();
		bool capacityReached = false;

		while (currentNode && !capacityReached) {
			if (++iterations >= maximumNodes) {
				if (found) {
					break;
				}
				return false;
			}

			{
				const Position currentPosition(currentNode->x, currentNode->y, 0);
				if (condition(startPosition, currentPosition, parameters, bestMatchDistance)) {
					found = currentNode;
					endX = currentNode->x;
					endY = currentNode->y;
					if (bestMatchDistance == 0) {
						break;
					}
				}
			}

			int32_t neighbourX, neighbourY;
			for (const auto& offset : neighbourOffsets) {
				neighbourX = static_cast<int32_t>(currentNode->x) + offset.first;
				neighbourY = static_cast<int32_t>(currentNode->y) + offset.second;

				// Limit search to the configured radius from the start position.
				const auto distanceX = neighbourX - startX;
				const auto distanceY = neighbourY - startY;
				const auto manhattanDistance =
				    (distanceX >= 0 ? distanceX : -distanceX) + (distanceY >= 0 ? distanceY : -distanceY);

				if (parameters.maxSearchDist != 0) {
					if (manhattanDistance > parameters.maxSearchDist) {
						continue;
					}
				} else if (manhattanDistance > PATHFIND_VIEWPORT_X + PATHFIND_VIEWPORT_Y) {
					continue;
				}

				// keepDistance prunes nodes outside the desired range from the target.
				if (parameters.keepDistance) {
					const auto deltaX = neighbourX - static_cast<int32_t>(targetX);
					const auto deltaY = neighbourY - static_cast<int32_t>(targetY);
					const auto chebyshevDistance = (deltaX >= 0 ? deltaX : -deltaX) > (deltaY >= 0 ? deltaY : -deltaY)
					                                   ? (deltaX >= 0 ? deltaX : -deltaX)
					                                   : (deltaY >= 0 ? deltaY : -deltaY);
					if (chebyshevDistance < parameters.minTargetDist ||
					    chebyshevDistance > parameters.maxTargetDist) {
						continue;
					}
				}

				// When the line of sight is clear we can restrict the search
				// to the rectangle between start and target.
				if (pruningActive) {
					if (sameX) {
						if (neighbourX != startX) { continue; }
					} else if (movingForwardX) {
						if (neighbourX < startX) { continue; }
						if (neighbourX > targetX) { continue; }
					} else {
						if (neighbourX > startX) { continue; }
						if (neighbourX < targetX) { continue; }
					}

					if (sameY) {
						if (neighbourY != startY) { continue; }
					} else if (movingForwardY) {
						if (neighbourY < startY) { continue; }
						if (neighbourY > targetY) { continue; }
					} else {
						if (neighbourY > startY) { continue; }
						if (neighbourY < targetY) { continue; }
					}
				}

				const auto packedX = static_cast<uint16_t>(neighbourX);
				const auto packedY = static_cast<uint16_t>(neighbourY);

				Cell* cell = getCell(packedX, packedY);
				if (!cell) {
					continue;
				}

				const uint16_t movementCost = (neighbourX != currentNode->x && neighbourY != currentNode->y)
				                                  ? PATHFIND_DIAGONAL_COST
				                                  : PATHFIND_NORMAL_COST;

				if (cell->state != CellState::Empty) {
					const auto heuristicValue = heuristic(packedX, packedY, targetX, targetY);
					const auto minimumG = currentNode->g + movementCost;

					if (cell->node.f <= heuristicValue + minimumG) {
						continue;
					}

					const auto exactG = minimumG + cell->tileCost;
					const auto totalF = heuristicValue + exactG;

					if (cell->node.f > totalF) {
						cell->node.g = exactG;
						cell->node.f = totalF;
						cell->node.parent = currentNode;
					}
				} else {
					const auto tileCost = getTileCost(packedX, packedY);
					if (tileCost == 0) {
						continue;
					}

					const auto heuristicValue = heuristic(packedX, packedY, targetX, targetY);
					const auto exactG = currentNode->g + movementCost + tileCost;
					const auto totalF = heuristicValue + exactG;

					cell->state = CellState::Open;
					cell->node.parent = currentNode;
					cell->node.x = packedX;
					cell->node.y = packedY;
					cell->node.g = exactG;
					cell->node.f = totalF;
					cell->tileCost = tileCost;

					if (openListSize >= maximumNodes) {
						if (found) {
							capacityReached = true;
							break;
						}
						return false;
					}
					openList[openListSize++] = &cell->node;
				}
			}

			if (!capacityReached) {
				currentNode = popBestNode();
			}
		}

		if (capacityReached && !found) {
			return false;
		}
		if (!found) {
			return false;
		}

		reconstructPath(found, endX, endY, directionList);
		return true;
	}

private:
	enum class CellState : uint8_t
	{
		Empty = 0,
		Open = 1,
		Closed = 2,
	};

	struct Node
	{
		Node* parent;
		uint16_t x, y;
		uint16_t g, f;
	};

	struct Cell
	{
		Node node;
		CellState state = CellState::Empty;
		uint16_t tileCost = 0;
	};

	static constexpr int32_t gridWidth = PATHFIND_VIEWPORT_X * 2 + 1;
	static constexpr int32_t gridHeight = PATHFIND_VIEWPORT_Y * 2 + 1;
	static constexpr int32_t maximumNodes = PATHFIND_VIEWPORT_X * PATHFIND_VIEWPORT_Y;

	static constexpr std::pair<int8_t, int8_t> neighbourOffsets[8] = {{-1, 0},  {0, 1},  {1, 0}, {0, -1},
	                                                                  {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

	Cell grid[gridHeight][gridWidth];
	Node* openList[maximumNodes];
	int32_t openListSize;
	const int32_t startX;
	const int32_t startY;

	Cell* getCell(uint16_t x, uint16_t y);

	Node* popBestNode();
	void reconstructPath(Node* node, uint16_t endX, uint16_t endY, std::vector<Direction>& directionList) const;

	static uint16_t heuristic(uint16_t x, uint16_t y, uint16_t targetX, uint16_t targetY);
};
