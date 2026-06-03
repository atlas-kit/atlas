#include "otpch.h"

#include "pathfinding.h"

#include "combat.h"
#include "creature.h"
#include "game.h"
#include "map.h"
#include "tile.h"

extern Game g_game;

namespace {

constexpr ConditionType_t DamageToConditionType(CombatType_t type)
{
	switch (type) {
		case COMBAT_FIREDAMAGE: return CONDITION_FIRE;
		case COMBAT_ENERGYDAMAGE: return CONDITION_ENERGY;
		case COMBAT_DROWNDAMAGE: return CONDITION_DROWN;
		case COMBAT_EARTHDAMAGE: return CONDITION_POISON;
		case COMBAT_ICEDAMAGE: return CONDITION_FREEZING;
		case COMBAT_HOLYDAMAGE: return CONDITION_DAZZLED;
		case COMBAT_DEATHDAMAGE: return CONDITION_CURSED;
		case COMBAT_PHYSICALDAMAGE: return CONDITION_BLEEDING;
		default: return CONDITION_NONE;
	}
}

} // namespace

bool FrozenPathingConditionCall::isInRange(const Position& startPos, const Position& testPos,
                                           const FindPathParams& fpp) const
{
	if (fpp.fullPathSearch) {
		if (testPos.x > targetPos.x + fpp.maxTargetDist) {
			return false;
		}
		if (testPos.x < targetPos.x - fpp.maxTargetDist) {
			return false;
		}
		if (testPos.y > targetPos.y + fpp.maxTargetDist) {
			return false;
		}
		if (testPos.y < targetPos.y - fpp.maxTargetDist) {
			return false;
		}
	} else {
		int32_t dx = startPos.getOffsetX(targetPos);
		int32_t dxMax = (dx >= 0 ? fpp.maxTargetDist : 0);
		if (testPos.x > targetPos.x + dxMax) {
			return false;
		}
		int32_t dxMin = (dx <= 0 ? fpp.maxTargetDist : 0);
		if (testPos.x < targetPos.x - dxMin) {
			return false;
		}
		int32_t dy = startPos.getOffsetY(targetPos);
		int32_t dyMax = (dy >= 0 ? fpp.maxTargetDist : 0);
		if (testPos.y > targetPos.y + dyMax) {
			return false;
		}
		int32_t dyMin = (dy <= 0 ? fpp.maxTargetDist : 0);
		if (testPos.y < targetPos.y - dyMin) {
			return false;
		}
	}
	return true;
}

bool FrozenPathingConditionCall::operator()(const Position& startPos, const Position& testPos,
                                            const FindPathParams& fpp, int32_t& bestMatchDist) const
{
	if (!isInRange(startPos, testPos, fpp)) {
		return false;
	}

	if (fpp.clearSight && !g_game.isSightClear(testPos, targetPos, true)) {
		return false;
	}

	int32_t testDist = std::max(targetPos.getDistanceX(testPos), targetPos.getDistanceY(testPos));
	if (fpp.maxTargetDist == 1) {
		if (testDist < fpp.minTargetDist || testDist > fpp.maxTargetDist) {
			return false;
		}
		return true;
	} else if (testDist <= fpp.maxTargetDist) {
		if (testDist < fpp.minTargetDist) {
			return false;
		}
		if (testDist == fpp.maxTargetDist) {
			bestMatchDist = 0;
			return true;
		} else if (testDist > bestMatchDist) {
			bestMatchDist = testDist;
			return true;
		}
	}
	return false;
}

void PathFinder::setStartTile(const Tile* tile)
{
	if (Cell* cell = getCell(static_cast<uint16_t>(startX), static_cast<uint16_t>(startY))) {
		cell->tile = tile;
	}
}

PathFinder::PathFinder(uint16_t x, uint16_t y) : openCount(0), startX(x), startY(y)
{
	for (auto& row : grid) {
		for (auto& cell : row) {
			cell.state = 0;
			cell.tile = nullptr;
		}
	}
	createNode(nullptr, x, y, 0, 0);
}

bool PathFinder::solve(uint16_t targetX, uint16_t targetY,
                       const std::shared_ptr<const Creature>& creature,
                       const FindPathParams& fpp,
                       std::vector<Direction>& dirList,
                       const IPathMap& map,
                       const FrozenPathingConditionCall& condition,
                       bool sightClear)
{
	Position startPos(startX, startY, 0);
	Position targetPos(targetX, targetY, 0);

	AStarNode* found = nullptr;
	int32_t bestMatch = 0;
	uint16_t iterations = 0;

	AStarNode* n = getBestNode();
	while (n) {
		if (++iterations >= MAX_NODES) {
			return false;
		}

		Position pos(n->x, n->y, 0);
		if (condition(startPos, pos, fpp, bestMatch)) {
			found = n;
			targetPos = pos;
			if (bestMatch == 0) {
				break;
			}
		}

		static constexpr std::array<std::pair<int, int>, 8> allNeighbors = {{
		    {-1, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, -1}, {1, -1}, {1, 1}, {-1, 1}}};

		for (uint8_t i = 0; i < 8; ++i) {
			int32_t nx = static_cast<int32_t>(n->x) + allNeighbors[i].first;
			int32_t ny = static_cast<int32_t>(n->y) + allNeighbors[i].second;

			int32_t dx = std::abs(nx - startX);
			int32_t dy = std::abs(ny - startY);
			if (dx > PATHFIND_VIEWPORT_X || dy > PATHFIND_VIEWPORT_Y) {
				continue;
			}

			if (fpp.keepDistance && !condition.isInRange(startPos, Position(nx, ny, 0), fpp)) {
				continue;
			}

			if (sightClear && !fpp.keepDistance && !fpp.summonTargetMaster && fpp.minTargetDist <= 1) {
				if (startX > targetX && nx > startX) { continue; }
				else if (startX == targetX && nx != startX) { continue; }
				else if (startX < targetX && nx < startX) { continue; }

				if (startY > targetY && ny > startY) { continue; }
				else if (startY == targetY && ny != startY) { continue; }
				else if (startY < targetY && ny < startY) { continue; }

				if (startX > targetX && nx < targetX) { continue; }
				else if (startX < targetX && nx > targetX) { continue; }
				if (startY > targetY && ny < targetY) { continue; }
				else if (startY < targetY && ny > targetY) { continue; }
			}

			AStarNode* neighborNode = getNodeByPosition(static_cast<uint16_t>(nx), static_cast<uint16_t>(ny));
			if (neighborNode) {
				uint16_t walkCost = getMapWalkCost(n->x, n->y, static_cast<uint16_t>(nx), static_cast<uint16_t>(ny));
				uint16_t h = calculateHeuristic(static_cast<uint16_t>(nx), static_cast<uint16_t>(ny), targetX, targetY);
				uint16_t minG = n->g + walkCost;

				if (neighborNode->f <= h + minG) {
					continue;
				}

				Cell* cell = getCell(static_cast<uint16_t>(nx), static_cast<uint16_t>(ny));
				if (cell && cell->tile) {
					uint16_t fullG = minG + getTileWalkCost(creature, cell->tile);
					uint16_t newf = h + fullG;

					if (neighborNode->f <= newf) {
						continue;
					}

					neighborNode->g = fullG;
					neighborNode->f = newf;
					neighborNode->parent = n;
				}
			} else {
				const Tile* tile = map.getTile(creature, static_cast<uint16_t>(nx), static_cast<uint16_t>(ny));
				if (!tile) {
					continue;
				}

				uint16_t walkCost = getMapWalkCost(n->x, n->y, static_cast<uint16_t>(nx), static_cast<uint16_t>(ny));
				uint16_t h = calculateHeuristic(static_cast<uint16_t>(nx), static_cast<uint16_t>(ny), targetX, targetY);
				uint16_t g = n->g + walkCost + getTileWalkCost(creature, tile);
				uint16_t newf = h + g;

				AStarNode* newNode = createNode(n, static_cast<uint16_t>(nx), static_cast<uint16_t>(ny), g, newf);
				if (!newNode) {
					return false;
				}

				if (Cell* cell = getCell(static_cast<uint16_t>(nx), static_cast<uint16_t>(ny))) {
					cell->tile = tile;
				}
			}
		}

		n = getBestNode();
	}

	if (!found) {
		return false;
	}

	reconstructPath(found, targetX, targetY, dirList);
	return true;
}

void PathFinder::reconstructPath(AStarNode* node, uint16_t endX, uint16_t endY, std::vector<Direction>& dirList) const
{
	AStarNode* n = node->parent;
	int32_t prevX = endX;
	int32_t prevY = endY;

	while (n) {
		int32_t dx = static_cast<int32_t>(n->x) - prevX;
		int32_t dy = static_cast<int32_t>(n->y) - prevY;

		prevX = n->x;
		prevY = n->y;

		if (dx == 1 && dy == 1) {
			dirList.push_back(DIRECTION_NORTHWEST);
		} else if (dx == -1 && dy == 1) {
			dirList.push_back(DIRECTION_NORTHEAST);
		} else if (dx == 1 && dy == -1) {
			dirList.push_back(DIRECTION_SOUTHWEST);
		} else if (dx == -1 && dy == -1) {
			dirList.push_back(DIRECTION_SOUTHEAST);
		} else if (dx == 1) {
			dirList.push_back(DIRECTION_WEST);
		} else if (dx == -1) {
			dirList.push_back(DIRECTION_EAST);
		} else if (dy == 1) {
			dirList.push_back(DIRECTION_NORTH);
		} else if (dy == -1) {
			dirList.push_back(DIRECTION_SOUTH);
		}

		n = n->parent;
	}
}

PathFinder::Cell* PathFinder::getCell(uint16_t x, uint16_t y)
{
	int32_t dx = static_cast<int32_t>(x) - startX + PATHFIND_VIEWPORT_X;
	int32_t dy = static_cast<int32_t>(y) - startY + PATHFIND_VIEWPORT_Y;
	if (dx >= 0 && dx < GRID_W && dy >= 0 && dy < GRID_H) {
		return &grid[dy][dx];
	}
	return nullptr;
}

AStarNode* PathFinder::getNodeByPosition(uint16_t x, uint16_t y) const
{
	int32_t dx = static_cast<int32_t>(x) - startX + PATHFIND_VIEWPORT_X;
	int32_t dy = static_cast<int32_t>(y) - startY + PATHFIND_VIEWPORT_Y;
	if (dx >= 0 && dx < GRID_W && dy >= 0 && dy < GRID_H) {
		const Cell& cell = grid[dy][dx];
		if (cell.state != 0) {
			return const_cast<AStarNode*>(&cell.node);
		}
	}
	return nullptr;
}

AStarNode* PathFinder::createNode(AStarNode* parent, uint16_t x, uint16_t y, uint16_t g, uint16_t f)
{
	if (openCount >= MAX_NODES) {
		return nullptr;
	}

	Cell* cell = getCell(x, y);
	if (!cell || cell->state != 0) {
		return nullptr;
	}

	cell->state = 1;
	cell->node.parent = parent;
	cell->node.x = x;
	cell->node.y = y;
	cell->node.g = g;
	cell->node.f = f;

	openList[openCount++] = &cell->node;
	return &cell->node;
}

AStarNode* PathFinder::getBestNode()
{
	if (openCount == 0) {
		return nullptr;
	}

	AStarNode* best = nullptr;
	uint16_t bestF = std::numeric_limits<uint16_t>::max();
	int32_t bestIndex = -1;

	for (int32_t i = 0; i < openCount; ++i) {
		AStarNode* node = openList[i];
		if (node->f < bestF) {
			bestF = node->f;
			best = node;
			bestIndex = i;
		}
	}

	if (bestIndex == -1) {
		return nullptr;
	}

	openList[bestIndex] = openList[--openCount];

	if (Cell* cell = getCell(best->x, best->y)) {
		cell->state = 2;
	}

	return best;
}

uint16_t PathFinder::getMapWalkCost(uint16_t fromX, uint16_t fromY, uint16_t toX, uint16_t toY)
{
	uint16_t dx = static_cast<uint16_t>(std::abs(static_cast<int16_t>(fromX) - static_cast<int16_t>(toX)));
	uint16_t dy = static_cast<uint16_t>(std::abs(static_cast<int16_t>(fromY) - static_cast<int16_t>(toY)));
	if (dx == dy) {
		return 25;
	}
	return 10;
}

uint16_t PathFinder::getTileWalkCost(const std::shared_ptr<const Creature>& creature, const Tile* tile)
{
	uint16_t cost = 0;
	if (tile->getTopVisibleCreature(creature)) {
		cost += 30;
	}

	if (const auto& field = tile->getFieldItem()) {
		CombatType_t combatType = field->getCombatType();
		const auto& monster = creature->asMonster();
		if (!creature->isImmune(combatType) && !creature->hasCondition(DamageToConditionType(combatType)) &&
		    (monster && !monster->canWalkOnFieldType(combatType))) {
			cost += 180;
		}
	}
	return cost;
}

uint16_t PathFinder::calculateHeuristic(uint16_t x, uint16_t y, uint16_t targetX, uint16_t targetY)
{
	int32_t dx = static_cast<int32_t>(x) - static_cast<int32_t>(targetX);
	int32_t dy = static_cast<int32_t>(y) - static_cast<int32_t>(targetY);
	return static_cast<uint16_t>(dx * dx + dy * dy);
}
