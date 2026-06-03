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

PathFinder::PathFinder(uint16_t x, uint16_t y) : openCount(0), startX(x), startY(y)
{
	for (auto& row : grid) {
		for (auto& cell : row) {
			cell.state = 0;
			cell.tileCost = 0;
		}
	}

	Cell* cell = cellAt(x, y);
	if (cell) {
		cell->state = 1;
		cell->node.parent = nullptr;
		cell->node.x = x;
		cell->node.y = y;
		cell->node.g = 0;
		cell->node.f = 0;
		openList[openCount++] = &cell->node;
	}
}

bool PathFinder::solve(uint16_t targetX, uint16_t targetY, const FindPathParams& fpp, std::vector<Direction>& dirList,
                       const IPathMap& map, const FrozenPathingConditionCall& condition, bool sightClear)
{
	Position startPos(startX, startY, 0);

	AStarNode* found = nullptr;
	int32_t bestMatch = 0;
	uint16_t iterations = 0;
	uint16_t endX = targetX, endY = targetY;

	// Pre-compute pruning flags (constants for this solve call)
	const bool pruneEnabled = sightClear && !fpp.keepDistance && !fpp.summonTargetMaster && fpp.minTargetDist <= 1;
	const bool xAligned = (startX == targetX);
	const bool yAligned = (startY == targetY);
	const bool xForward = (startX < targetX);
	const bool yForward = (startY < targetY);

	AStarNode* n = getBestNode();
	while (n) {
		if (++iterations >= MAX_NODES) {
			return false;
		}

		{
			Position pos(n->x, n->y, 0);
			if (condition(startPos, pos, fpp, bestMatch)) {
				found = n;
				endX = n->x;
				endY = n->y;
				if (bestMatch == 0) {
					break;
				}
			}
		}

		int32_t nx, ny;
		for (uint8_t i = 0; i < 8; ++i) {
			nx = static_cast<int32_t>(n->x) + NEIGHBORS[i].first;
			ny = static_cast<int32_t>(n->y) + NEIGHBORS[i].second;

			// Distance check from start
			int32_t sdx = nx - startX;
			int32_t sdy = ny - startY;
			int32_t startDist = (sdx >= 0 ? sdx : -sdx) + (sdy >= 0 ? sdy : -sdy);
			if (fpp.maxSearchDist != 0) {
				if (startDist > fpp.maxSearchDist) {
					continue;
				}
			} else if (startDist > PATHFIND_VIEWPORT_X + PATHFIND_VIEWPORT_Y) {
				continue;
			}

			// keepDistance pruning
			if (fpp.keepDistance) {
				int32_t dx = nx - static_cast<int32_t>(targetX);
				int32_t dy = ny - static_cast<int32_t>(targetY);
				int32_t dist =
				    (dx >= 0 ? dx : -dx) > (dy >= 0 ? dy : -dy) ? (dx >= 0 ? dx : -dx) : (dy >= 0 ? dy : -dy);
				if (dist < fpp.minTargetDist || dist > fpp.maxTargetDist) {
					continue;
				}
			}

			// sight-clear pruning
			if (pruneEnabled) {
				if (xAligned) {
					if (nx != startX) {
						continue;
					}
				} else if (xForward) {
					if (nx < startX) {
						continue;
					}
					if (nx > targetX) {
						continue;
					}
				} else {
					if (nx > startX) {
						continue;
					}
					if (nx < targetX) {
						continue;
					}
				}

				if (yAligned) {
					if (ny != startY) {
						continue;
					}
				} else if (yForward) {
					if (ny < startY) {
						continue;
					}
					if (ny > targetY) {
						continue;
					}
				} else {
					if (ny > startY) {
						continue;
					}
					if (ny < targetY) {
						continue;
					}
				}
			}

			uint16_t nx16 = static_cast<uint16_t>(nx);
			uint16_t ny16 = static_cast<uint16_t>(ny);

			// Single grid lookup
			Cell* cell = cellAt(nx16, ny16);
			if (!cell) {
				continue;
			}

			uint16_t walkCost = (nx != n->x && ny != n->y) ? 25 : 10;

			if (cell->state != 0) {
				// Already in open or closed set
				uint16_t h = heuristic(nx16, ny16, targetX, targetY);
				uint16_t minG = n->g + walkCost;

				if (cell->node.f <= h + minG) {
					continue;
				}

				uint16_t fullG = minG + cell->tileCost;
				uint16_t newf = h + fullG;

				if (cell->node.f > newf) {
					cell->node.g = fullG;
					cell->node.f = newf;
					cell->node.parent = n;
				}
			} else {
				// New node
				uint16_t tileCost = map.getWalkCost(nx16, ny16);
				if (tileCost == 0) {
					continue;
				}

				uint16_t h = heuristic(nx16, ny16, targetX, targetY);
				uint16_t g = n->g + walkCost + tileCost;
				uint16_t newf = h + g;

				cell->state = 1;
				cell->node.parent = n;
				cell->node.x = nx16;
				cell->node.y = ny16;
				cell->node.g = g;
				cell->node.f = newf;
				cell->tileCost = tileCost;

				if (openCount >= MAX_NODES) {
					return false;
				}
				openList[openCount++] = &cell->node;
			}
		}

		n = getBestNode();
	}

	if (!found) {
		return false;
	}

	reconstructPath(found, endX, endY, dirList);
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

PathFinder::Cell* PathFinder::cellAt(uint16_t x, uint16_t y)
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

	if (Cell* cell = cellAt(best->x, best->y)) {
		cell->state = 2;
	}

	return best;
}

uint16_t PathFinder::heuristic(uint16_t x, uint16_t y, uint16_t targetX, uint16_t targetY)
{
	int32_t dx = static_cast<int32_t>(x) - static_cast<int32_t>(targetX);
	int32_t dy = static_cast<int32_t>(y) - static_cast<int32_t>(targetY);
	return static_cast<uint16_t>(dx * dx + dy * dy);
}
