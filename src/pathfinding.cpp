#include "otpch.h"

#include "pathfinding.h"

#include "combat.h"
#include "creature.h"
#include "game.h"
#include "tile.h"

extern Game g_game;

namespace {

constexpr ConditionType_t DamageToConditionType(CombatType_t type)
{
	switch (type) {
		case COMBAT_FIREDAMAGE:
			return CONDITION_FIRE;
		case COMBAT_ENERGYDAMAGE:
			return CONDITION_ENERGY;
		case COMBAT_DROWNDAMAGE:
			return CONDITION_DROWN;
		case COMBAT_EARTHDAMAGE:
			return CONDITION_POISON;
		case COMBAT_ICEDAMAGE:
			return CONDITION_FREEZING;
		case COMBAT_HOLYDAMAGE:
			return CONDITION_DAZZLED;
		case COMBAT_DEATHDAMAGE:
			return CONDITION_CURSED;
		case COMBAT_PHYSICALDAMAGE:
			return CONDITION_BLEEDING;
		default:
			return CONDITION_NONE;
	}
}

} // namespace

// PathCondition

bool PathCondition::isInRange(const Position& startPosition, const Position& testPosition,
                              const FindPathParams& parameters) const
{
	if (parameters.fullPathSearch) {
		if (testPosition.x > targetPosition.x + parameters.maxTargetDist) {
			return false;
		}
		if (testPosition.x < targetPosition.x - parameters.maxTargetDist) {
			return false;
		}
		if (testPosition.y > targetPosition.y + parameters.maxTargetDist) {
			return false;
		}
		if (testPosition.y < targetPosition.y - parameters.maxTargetDist) {
			return false;
		}
	} else {
		const auto offsetX = startPosition.getOffsetX(targetPosition);
		const auto limitHighX = (offsetX >= 0 ? parameters.maxTargetDist : 0);
		if (testPosition.x > targetPosition.x + limitHighX) {
			return false;
		}
		const auto limitLowX = (offsetX <= 0 ? parameters.maxTargetDist : 0);
		if (testPosition.x < targetPosition.x - limitLowX) {
			return false;
		}
		const auto offsetY = startPosition.getOffsetY(targetPosition);
		const auto limitHighY = (offsetY >= 0 ? parameters.maxTargetDist : 0);
		if (testPosition.y > targetPosition.y + limitHighY) {
			return false;
		}
		const auto limitLowY = (offsetY <= 0 ? parameters.maxTargetDist : 0);
		if (testPosition.y < targetPosition.y - limitLowY) {
			return false;
		}
	}
	return true;
}

bool PathCondition::operator()(const Position& startPosition, const Position& testPosition,
                               const FindPathParams& parameters, int32_t& bestMatchDistance) const
{
	if (!isInRange(startPosition, testPosition, parameters)) {
		return false;
	}

	// Sight-line check to avoid pathfinding through walls.
	if (parameters.clearSight && !g_game.isSightClear(testPosition, targetPosition, true)) {
		return false;
	}

	const auto testDistance =
	    std::max(targetPosition.getDistanceX(testPosition), targetPosition.getDistanceY(testPosition));
	if (parameters.maxTargetDist == 1) {
		// Most common case: the caller wants to get right next to the target.
		if (testDistance < parameters.minTargetDist || testDistance > parameters.maxTargetDist) {
			return false;
		}
		return true;
	}

	if (testDistance <= parameters.maxTargetDist) {
		if (testDistance < parameters.minTargetDist) {
			return false;
		}
		// Found a node at exactly the right distance, or the best so far.
		if (testDistance == parameters.maxTargetDist) {
			bestMatchDistance = 0;
			return true;
		}
		if (testDistance > bestMatchDistance) {
			bestMatchDistance = testDistance;
			return true;
		}
	}
	return false;
}

// PathFinder

PathFinder::PathFinder(uint16_t x, uint16_t y) : openListSize(0), startX(x), startY(y)
{
	// The start node is always valid — the creature is standing on it.
	if (Cell* cell = getCell(x, y)) {
		cell->state = CellState::Open;
		cell->node.parent = nullptr;
		cell->node.x = x;
		cell->node.y = y;
		cell->node.g = 0;
		cell->node.f = 0;
		openList[openListSize++] = &cell->node;
	}
}

bool PathFinder::search(uint16_t targetX, uint16_t targetY, const FindPathParams& parameters,
                        std::vector<Direction>& directionList, const IPathMap& map, const PathCondition& condition,
                        bool sightClear)
{
	const Position startPosition(startX, startY, 0);

	AStarNode* found = nullptr;
	int32_t bestMatchDistance = 0;
	uint16_t iterations = 0;
	uint16_t endX = targetX;
	uint16_t endY = targetY;

	// Pre-compute pruning conditions — they are invariant for this call.
	const auto pruningActive =
	    sightClear && !parameters.keepDistance && !parameters.summonTargetMaster && parameters.minTargetDist <= 1;
	const auto sameX = (startX == targetX);
	const auto sameY = (startY == targetY);
	const auto movingForwardX = (startX < targetX);
	const auto movingForwardY = (startY < targetY);

	AStarNode* currentNode = popBestNode();
	bool capacityReached = false;

	while (currentNode && !capacityReached) {
		// Limit the number of expansions to avoid pathological cases
		// (e.g., unreachable targets behind walls).
		if (++iterations >= maximumNodes) {
			if (found) {
				break;
			}
			return false;
		}

		// Check if the current node satisfies the goal condition.
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

		// Expand all eight neighbours.
		int32_t neighbourX, neighbourY;
		for (const auto& offset : neighbourOffsets) {
			neighbourX = static_cast<int32_t>(currentNode->x) + offset.first;
			neighbourY = static_cast<int32_t>(currentNode->y) + offset.second;

			// Skip nodes beyond the configured search radius.
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
				if (chebyshevDistance < parameters.minTargetDist || chebyshevDistance > parameters.maxTargetDist) {
					continue;
				}
			}

			// When the line of sight is clear we can restrict the search
			// to the rectangle between start and target.
			if (pruningActive) {
				if (sameX) {
					if (neighbourX != startX) {
						continue;
					}
				} else if (movingForwardX) {
					if (neighbourX < startX) {
						continue;
					}
					if (neighbourX > targetX) {
						continue;
					}
				} else {
					if (neighbourX > startX) {
						continue;
					}
					if (neighbourX < targetX) {
						continue;
					}
				}

				if (sameY) {
					if (neighbourY != startY) {
						continue;
					}
				} else if (movingForwardY) {
					if (neighbourY < startY) {
						continue;
					}
					if (neighbourY > targetY) {
						continue;
					}
				} else {
					if (neighbourY > startY) {
						continue;
					}
					if (neighbourY < targetY) {
						continue;
					}
				}
			}

			const auto packedX = static_cast<uint16_t>(neighbourX);
			const auto packedY = static_cast<uint16_t>(neighbourY);

			Cell* cell = getCell(packedX, packedY);
			if (!cell) {
				continue;
			}

			// Diagonal movement costs more than cardinal.
			const uint16_t movementCost = (neighbourX != currentNode->x && neighbourY != currentNode->y)
			                                  ? MAP_DIAGONALWALKCOST
			                                  : MAP_NORMALWALKCOST;

			if (cell->state != CellState::Empty) {
				// Node was already reached through a different path;
				// check whether this new path is cheaper.
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
				// First time we visit this node — query the map for walkability.
				const auto tileCost = map.getWalkCost(packedX, packedY);
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

				// If we run out of room and already have a result, use it.
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

void PathFinder::reconstructPath(AStarNode* node, uint16_t endX, uint16_t endY,
                                 std::vector<Direction>& directionList) const
{
	// Walk the parent chain from the end back to the start, pushing the
	// opposite direction so that the caller reads them from start to end.
	AStarNode* currentNode = node->parent;
	auto previousX = static_cast<int32_t>(endX);
	auto previousY = static_cast<int32_t>(endY);

	while (currentNode) {
		const auto deltaX = static_cast<int32_t>(currentNode->x) - previousX;
		const auto deltaY = static_cast<int32_t>(currentNode->y) - previousY;

		previousX = currentNode->x;
		previousY = currentNode->y;

		if (deltaX == 1 && deltaY == 1) {
			directionList.push_back(DIRECTION_NORTHWEST);
		} else if (deltaX == -1 && deltaY == 1) {
			directionList.push_back(DIRECTION_NORTHEAST);
		} else if (deltaX == 1 && deltaY == -1) {
			directionList.push_back(DIRECTION_SOUTHWEST);
		} else if (deltaX == -1 && deltaY == -1) {
			directionList.push_back(DIRECTION_SOUTHEAST);
		} else if (deltaX == 1) {
			directionList.push_back(DIRECTION_WEST);
		} else if (deltaX == -1) {
			directionList.push_back(DIRECTION_EAST);
		} else if (deltaY == 1) {
			directionList.push_back(DIRECTION_NORTH);
		} else if (deltaY == -1) {
			directionList.push_back(DIRECTION_SOUTH);
		}

		currentNode = currentNode->parent;
	}
}

PathFinder::Cell* PathFinder::getCell(uint16_t x, uint16_t y)
{
	const auto gridX = static_cast<int32_t>(x) - startX + PATHFIND_VIEWPORT_X;
	const auto gridY = static_cast<int32_t>(y) - startY + PATHFIND_VIEWPORT_Y;
	if (gridX >= 0 && gridX < gridWidth && gridY >= 0 && gridY < gridHeight) {
		return &grid[gridY][gridX];
	}
	return nullptr;
}

AStarNode* PathFinder::getNodeAt(uint16_t x, uint16_t y) const
{
	const auto gridX = static_cast<int32_t>(x) - startX + PATHFIND_VIEWPORT_X;
	const auto gridY = static_cast<int32_t>(y) - startY + PATHFIND_VIEWPORT_Y;
	if (gridX >= 0 && gridX < gridWidth && gridY >= 0 && gridY < gridHeight) {
		const Cell& cell = grid[gridY][gridX];
		if (cell.state != CellState::Empty) {
			return const_cast<AStarNode*>(&cell.node);
		}
	}
	return nullptr;
}

AStarNode* PathFinder::popBestNode()
{
	if (openListSize == 0) {
		return nullptr;
	}

	AStarNode* bestNode = nullptr;
	auto minimumF = std::numeric_limits<uint16_t>::max();
	int32_t bestIndex = -1;

	for (int32_t index = 0; index < openListSize; ++index) {
		AStarNode* node = openList[index];
		if (node->f < minimumF) {
			minimumF = node->f;
			bestNode = node;
			bestIndex = index;
		}
	}

	if (bestIndex == -1) {
		return nullptr;
	}

	// Remove the node from the open list by swapping with the last element.
	openList[bestIndex] = openList[--openListSize];

	if (Cell* cell = getCell(bestNode->x, bestNode->y)) {
		cell->state = CellState::Closed;
	}

	return bestNode;
}

uint16_t PathFinder::heuristic(uint16_t x, uint16_t y, uint16_t targetX, uint16_t targetY)
{
	// Squared Euclidean distance — fast and admissible for this cost model.
	const auto differenceX = static_cast<int32_t>(x) - static_cast<int32_t>(targetX);
	const auto differenceY = static_cast<int32_t>(y) - static_cast<int32_t>(targetY);
	return static_cast<uint16_t>(differenceX * differenceX + differenceY * differenceY);
}
