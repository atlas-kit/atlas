#include "otpch.h"

#include "pathfinding/pathfinder.h"

#include "pathfinding/constants.h"
#include "pathfinding/distance.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <vector>

namespace
{

// The grid is centered on the start tile. Its radius is the requested Manhattan
// limit capped by kMaxGridRadius; getCell rejects positions outside that grid.
constexpr int32_t kMaxGridRadius = 64;
constexpr int32_t kDefaultManhattanLimit = 22;
// With maxDistance == 0, targets more than 12 tiles away on either axis are
// rejected before the search starts.
constexpr int32_t kDefaultAxisReach = 12;

struct NeighbourOffset
{
	int8_t x;
	int8_t y;
	uint16_t movementCost;
};

constexpr NeighbourOffset neighbourOffsets[8] = {
	{-1, 0, PATHFIND_NORMAL_COST},  {0, 1, PATHFIND_NORMAL_COST},  {1, 0, PATHFIND_NORMAL_COST},
	{0, -1, PATHFIND_NORMAL_COST}, {-1, -1, PATHFIND_DIAGONAL_COST}, {1, -1, PATHFIND_DIAGONAL_COST},
	{1, 1, PATHFIND_DIAGONAL_COST}, {-1, 1, PATHFIND_DIAGONAL_COST}};

constexpr uint16_t kNoHeapIndex = std::numeric_limits<uint16_t>::max();

struct Node
{
	// Parent pointers refer to nodes in the current thread's grid storage and
	// remain valid for the duration of the search.
	Node* parent = nullptr;
	int32_t x = 0;
	int32_t y = 0;
	int32_t g = 0;
	int32_t f = 0;
	uint32_t order = 0; // Insertion order used for deterministic tie-breaking.
	uint16_t heapIndex = kNoHeapIndex; // kNoHeapIndex means the node is not open.
};

struct Cell
{
	// A cell reuses its node and tile cost across searches. The epoch identifies
	// whether those values belong to the current search.
	Node node;
	uint32_t epoch = 0; // A mismatched stamp means the cell may be reclaimed.
	uint16_t tileCost = 0;
};

// Precomputes the target-relative bounds used by each candidate evaluation.
// Reach keeps the full target-centered rectangle; Approach and Flee keep the
// target-relative side containing the start on each axis.
struct PreparedGoal
{
	PreparedGoal(const PathGoal& goal, int32_t startX, int32_t startY)
	    : goal(goal), targetX(static_cast<int32_t>(goal.target.x)), targetY(static_cast<int32_t>(goal.target.y)),
	      distanceMin(goal.distance.min), distanceMax(goal.distance.max)
	{
		if (goal.mode == SearchMode::Reach) {
			rangeMinX = targetX - distanceMax;
			rangeMaxX = targetX + distanceMax;
			rangeMinY = targetY - distanceMax;
			rangeMaxY = targetY + distanceMax;
			return;
		}

		const auto offsetX = startX - targetX;
		const auto offsetY = startY - targetY;
		rangeMaxX = targetX + (offsetX >= 0 ? distanceMax : 0);
		rangeMinX = targetX - (offsetX <= 0 ? distanceMax : 0);
		rangeMaxY = targetY + (offsetY >= 0 ? distanceMax : 0);
		rangeMinY = targetY - (offsetY <= 0 ? distanceMax : 0);
	}

	[[nodiscard]] bool isInRange(int32_t testX, int32_t testY) const
	{
		if (testX > rangeMaxX || testX < rangeMinX) {
			return false;
		}
		if (testY > rangeMaxY || testY < rangeMinY) {
			return false;
		}
		return true;
	}

	GoalMatch evaluate(int32_t testX, int32_t testY, int32_t& bestMatch, SightCheck& sightCheck) const
	{
		// A candidate must pass the mode-dependent bounds, optional sight check, and
		// distance range. Candidates at the outer distance bound are terminal;
		// other accepted candidates update bestMatch only when they improve it.
		if (!isInRange(testX, testY)) {
			return GoalMatch::None;
		}

		if (goal.requireSight) {
			if (!sightCheck) {
				return GoalMatch::None;
			}
			const Position testPosition(testX, testY, goal.target.z);
			if (!sightCheck(testPosition, goal.target)) {
				return GoalMatch::None;
			}
		}

		const auto testDistance = chebyshevDistance(testX, testY, targetX, targetY);
		if (distanceMax == 1) {
			if (testDistance < distanceMin || testDistance > distanceMax) {
				return GoalMatch::None;
			}
			return GoalMatch::Exact;
		}

		if (testDistance > distanceMax || testDistance < distanceMin) {
			return GoalMatch::None;
		}
		if (testDistance == distanceMax) {
			bestMatch = 0;
			return GoalMatch::Exact;
		}
		if (testDistance > bestMatch) {
			bestMatch = testDistance;
			return GoalMatch::Better;
		}
		return GoalMatch::None;
	}

	const PathGoal& goal;
	const int32_t targetX;
	const int32_t targetY;
	const int32_t distanceMin;
	const int32_t distanceMax;
	int32_t rangeMinX = 0;
	int32_t rangeMaxX = 0;
	int32_t rangeMinY = 0;
	int32_t rangeMaxY = 0;
};

// Each thread owns scratch storage for the grid, open heap, and search epoch.
// Capacities are retained between searches and grow only when a larger search needs them.
thread_local std::vector<Cell> gridStorage;
thread_local std::vector<Node*> openList;
thread_local uint32_t searchEpoch = 0;

class PathSearcher
{
public:
	PathSearcher(int32_t startX, int32_t startY, int32_t gridRadius)
	    : startX(startX), startY(startY), gridWidth(gridRadius * 2 + 1), gridHeight(gridRadius * 2 + 1),
	      gridCenterX(gridWidth / 2), gridCenterY(gridHeight / 2),
	      cellCount(static_cast<size_t>(gridWidth) * static_cast<size_t>(gridHeight)), epoch(++searchEpoch)
	{
		// On epoch wrap, clear all existing stamps before starting a new non-zero generation.
		if (epoch == 0) {
			std::fill(gridStorage.begin(), gridStorage.end(), Cell{});
			epoch = ++searchEpoch;
		}

		if (gridStorage.size() < cellCount) {
			gridStorage.resize(cellCount);
		}

		if (openList.capacity() < PATHFIND_NODE_BUDGET) {
			openList.reserve(PATHFIND_NODE_BUDGET);
		}

		openList.clear();
	}

	PathStatus search(const PathGoal& goal, const PathRequest& request, TileCost& getTileCost,
	                  std::vector<Direction>& directionList, SightCheck& sightCheck)
	{
		const PreparedGoal preparedGoal(goal, startX, startY);
		const auto targetX = preparedGoal.targetX;
		const auto targetY = preparedGoal.targetY;

		Node* found = nullptr;
		bool exactMatch = false;
		int32_t bestMatch = 0;
		size_t iterations = 0;
		int32_t endX = targetX;
		int32_t endY = targetY;

		const auto keepDistance = goal.mode == SearchMode::Flee;
		const auto pruningActive =
		    request.lineOfSightClear() && !keepDistance && !request.summoningTargetMaster() && goal.distance.min <= 1;
		const auto pruningMinX = std::min(startX, targetX);
		const auto pruningMaxX = std::max(startX, targetX);
		const auto pruningMinY = std::min(startY, targetY);
		const auto pruningMaxY = std::max(startY, targetY);

		// A clear direct line permits rectangle pruning for ordinary reach/approach
		// searches. Flee, target-master searches, and ranges with min > 1 retain
		// their broader exploration behavior.
		// When active, discard neighbours outside the rectangle spanning start and target.
		const auto maxDistance = request.maxDistance();
		const auto manhattanLimit = maxDistance ? maxDistance : kDefaultManhattanLimit;

		seedStart();

		Node* currentNode = popBestNode();
		while (currentNode) {
			// Iteration and node limits are hard failures: return NoPath rather than
			// reconstructing a retained non-exact match.
			if (++iterations >= PATHFIND_ITERATION_BUDGET) {
				return PathStatus::NoPath;
			}

			const auto match = preparedGoal.evaluate(currentNode->x, currentNode->y, bestMatch, sightCheck);
			if (match != GoalMatch::None) {
				found = currentNode;
				endX = currentNode->x;
				endY = currentNode->y;
				if (match == GoalMatch::Exact) {
					exactMatch = true;
					break;
				}
			}

			for (const auto& neighbour : neighbourOffsets) {
				const auto offsetX = neighbour.x;
				const auto offsetY = neighbour.y;
				const auto neighbourX = currentNode->x + offsetX;
				const auto neighbourY = currentNode->y + offsetY;

				if (manhattanDistance(neighbourX, neighbourY, startX, startY) > manhattanLimit) {
					continue;
				}

				if (keepDistance && !preparedGoal.isInRange(neighbourX, neighbourY)) {
					continue;
				}

				if (pruningActive &&
				    (neighbourX < pruningMinX || neighbourX > pruningMaxX || neighbourY < pruningMinY ||
				     neighbourY > pruningMaxY)) {
					continue;
				}

				Cell* cell = getCell(neighbourX, neighbourY);
				if (!cell) {
					continue;
				}

				const auto movementCost = neighbour.movementCost;

				// TileCost is queried only when a cell is first claimed; later relaxations
				// reuse the stored value and update the heap only while the node is open.
				if (cell->epoch == epoch) {
					const auto heuristicValue = squaredDistance(neighbourX, neighbourY, targetX, targetY);
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
						if (cell->node.heapIndex != kNoHeapIndex) {
							siftUp(&cell->node);
						}
					}
				} else {
					const auto tileCost = getTileCost(neighbourX, neighbourY);
					if (tileCost == 0) {
						continue;
					}

					if (!reserveNode()) {
						return PathStatus::NoPath;
					}

					const auto heuristicValue = squaredDistance(neighbourX, neighbourY, targetX, targetY);
					const auto exactG = currentNode->g + movementCost + tileCost;
					const auto totalF = heuristicValue + exactG;

					cell->epoch = epoch;
					cell->node.parent = currentNode;
					cell->node.x = neighbourX;
					cell->node.y = neighbourY;
					cell->node.g = exactG;
					cell->node.f = totalF;
					cell->node.order = nextOrder++;
					cell->tileCost = tileCost;

					pushOpenNode(cell);
				}
			}

			currentNode = popBestNode();
		}

		if (!found) {
			return PathStatus::NoPath;
		}

		reconstructPath(found, endX, endY, directionList);
		// A terminal Exact match returns Found. If the open list is exhausted normally
		// after retaining only a Better match, reconstruct that route as Partial; the
		// route may be empty when the retained node is the start.
		return exactMatch ? PathStatus::Found : PathStatus::Partial;
	}

private:
	bool reserveNode()
	{
		if (createdNodes >= PATHFIND_NODE_BUDGET) {
			return false;
		}
		++createdNodes;
		return true;
	}

	void seedStart()
	{
		if (Cell* cell = getCell(startX, startY)) {
			if (!reserveNode()) {
				return;
			}
			cell->epoch = epoch;
			cell->node.parent = nullptr;
			cell->node.x = startX;
			cell->node.y = startY;
			cell->node.g = 0;
			cell->node.f = 0;
			cell->node.order = nextOrder++;
			pushOpenNode(cell);
		}
	}

	Cell* getCell(int32_t x, int32_t y)
	{
		const auto gridX = x - startX + gridCenterX;
		const auto gridY = y - startY + gridCenterY;
		if (gridX >= 0 && gridX < gridWidth && gridY >= 0 && gridY < gridHeight) {
			return &gridStorage[static_cast<size_t>(gridY) * gridWidth + gridX];
		}
		return nullptr;
	}

	// Open nodes are ordered by lowest f; equal f values use insertion order so
	// tie-breaking remains deterministic.
	static bool comesBefore(const Node* lhs, const Node* rhs)
	{
		return lhs->f < rhs->f || (lhs->f == rhs->f && lhs->order < rhs->order);
	}

	void swapHeapEntries(size_t firstIndex, size_t secondIndex)
	{
		std::swap(openList[firstIndex], openList[secondIndex]);
		openList[firstIndex]->heapIndex = static_cast<uint16_t>(firstIndex);
		openList[secondIndex]->heapIndex = static_cast<uint16_t>(secondIndex);
	}

	void siftUp(Node* node)
	{
		size_t index = node->heapIndex;
		while (index > 0) {
			const size_t parentIndex = (index - 1) / 2;
			if (!comesBefore(openList[index], openList[parentIndex])) {
				break;
			}

			swapHeapEntries(index, parentIndex);
			index = parentIndex;
		}
	}

	void siftDown(size_t index)
	{
		while (true) {
			const size_t leftIndex = index * 2 + 1;
			if (leftIndex >= openList.size()) {
				break;
			}

			const size_t rightIndex = leftIndex + 1;
			size_t bestChildIndex = leftIndex;
			if (rightIndex < openList.size() && comesBefore(openList[rightIndex], openList[leftIndex])) {
				bestChildIndex = rightIndex;
			}

			if (!comesBefore(openList[bestChildIndex], openList[index])) {
				break;
			}

			swapHeapEntries(index, bestChildIndex);
			index = bestChildIndex;
		}
	}

	void pushOpenNode(Cell* cell)
	{
		Node* node = &cell->node;
		node->heapIndex = static_cast<uint16_t>(openList.size());
		openList.push_back(node);
		siftUp(node);
	}

	Node* popBestNode()
	{
		if (openList.empty()) {
			return nullptr;
		}

		Node* bestNode = openList.front();
		const size_t lastIndex = openList.size() - 1;
		if (lastIndex != 0) {
			openList.front() = openList.back();
			openList.front()->heapIndex = 0;
		}

		openList.pop_back();

		bestNode->heapIndex = kNoHeapIndex;

		if (!openList.empty()) {
			siftDown(0);
		}

		return bestNode;
	}

	void reconstructPath(Node* node, int32_t endX, int32_t endY, std::vector<Direction>& directionList) const
	{
		// Follow parents from the selected end node toward the start. Directions are
		// appended end-to-start because Creature consumes listWalkDir from back().
		Node* currentNode = node->parent;

		auto previousX = endX;
		auto previousY = endY;

		while (currentNode) {
			const auto deltaX = currentNode->x - previousX;
			const auto deltaY = currentNode->y - previousY;

			previousX = currentNode->x;
			previousY = currentNode->y;

			if (deltaX == 1 && deltaY == 1) { directionList.push_back(DIRECTION_NORTHWEST); }
			else if (deltaX == -1 && deltaY == 1) { directionList.push_back(DIRECTION_NORTHEAST); }
			else if (deltaX == 1 && deltaY == -1) { directionList.push_back(DIRECTION_SOUTHWEST); }
			else if (deltaX == -1 && deltaY == -1) { directionList.push_back(DIRECTION_SOUTHEAST); }
			else if (deltaX == 1) { directionList.push_back(DIRECTION_WEST); }
			else if (deltaX == -1) { directionList.push_back(DIRECTION_EAST); }
			else if (deltaY == 1) { directionList.push_back(DIRECTION_NORTH); }
			else if (deltaY == -1) { directionList.push_back(DIRECTION_SOUTH); }

			currentNode = currentNode->parent;
		}
	}

	const int32_t startX;
	const int32_t startY;
	const int32_t gridWidth;
	const int32_t gridHeight;
	const int32_t gridCenterX;
	const int32_t gridCenterY;
	const size_t cellCount;
	uint32_t epoch = 0;
	size_t createdNodes = 0;
	uint32_t nextOrder = 0;
};

} // namespace

PathStatus PathFinder::find(const PathRequest& request, TileCost getTileCost, std::vector<Direction>& directionList,
                            SightCheck sightCheck)
{
	const auto& goal = request.goal();
	const auto startX = static_cast<int32_t>(request.start().x);
	const auto startY = static_cast<int32_t>(request.start().y);

	directionList.clear();

	// Different start and target floors are rejected before searching.
	if (request.start().z != goal.target.z) {
		return PathStatus::NoPath;
	}

	const auto targetX = static_cast<int32_t>(goal.target.x);
	const auto targetY = static_cast<int32_t>(goal.target.y);
	const auto distanceX = std::abs(startX - targetX);
	const auto distanceY = std::abs(startY - targetY);

	const auto targetDistance = std::max(distanceX, distanceY);

	// Reach requests at one-tile range can defer to the caller's dance-step logic
	// when the current position already satisfies the requested distance range.
	if (goal.mode == SearchMode::Reach && goal.distance.max == 1 &&
	    targetDistance >= goal.distance.min && targetDistance <= goal.distance.max) {
		return PathStatus::Found;
	}

	// Reject targets beyond the configured axis reach before allocating the search grid.
	const auto maxDistance = request.maxDistance();
	const auto axisReach = maxDistance ? maxDistance : kDefaultAxisReach;
	if (distanceX > axisReach || distanceY > axisReach) {
		return PathStatus::NoPath;
	}

	const auto manhattanLimit = maxDistance ? maxDistance : kDefaultManhattanLimit;
	const auto gridRadius = std::min(manhattanLimit, kMaxGridRadius);

	PathSearcher searcher(startX, startY, gridRadius);
	return searcher.search(goal, request, getTileCost, directionList, sightCheck);
}

std::expected<std::vector<Direction>, PathStatus> PathFinder::find(const PathRequest& request, TileCost getTileCost,
                                                                   SightCheck sightCheck)
{
	std::vector<Direction> directionList;
	const auto status = find(request, std::move(getTileCost), directionList, std::move(sightCheck));
	if (status == PathStatus::NoPath) {
		return std::unexpected(status);
	}
	return directionList;
}
