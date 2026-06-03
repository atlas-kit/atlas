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

void PathFinder::reconstructPath(Node* node, uint16_t endX, uint16_t endY,
                                 std::vector<Direction>& directionList) const
{
	// Walk the parent chain from the end back to the start, pushing the
	// opposite direction so that the caller reads them from start to end.
	Node* currentNode = node->parent;
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

auto PathFinder::getCell(uint16_t x, uint16_t y) -> Cell*
{
	const auto gridX = static_cast<int32_t>(x) - startX + PATHFIND_VIEWPORT_X;
	const auto gridY = static_cast<int32_t>(y) - startY + PATHFIND_VIEWPORT_Y;
	if (gridX >= 0 && gridX < gridWidth && gridY >= 0 && gridY < gridHeight) {
		return &grid[gridY][gridX];
	}
	return nullptr;
}

auto PathFinder::popBestNode() -> Node*
{
	if (openListSize == 0) {
		return nullptr;
	}

	Node* bestNode = nullptr;
	auto minimumF = std::numeric_limits<uint16_t>::max();
	int32_t bestIndex = -1;

	for (int32_t index = 0; index < openListSize; ++index) {
		Node* node = openList[index];
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
