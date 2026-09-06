#pragma once

#include "distance.h"
#include "path_world.h"
#include "../position.h"
#include "search_mode.h"

#include <cstdint>

/**
 * Result of evaluating a node against a PathGoal.
 */
enum class GoalMatch : uint8_t
{
	None, // The node does not satisfy the goal.
	Better, // A better in-range match was found; keep searching for Exact.
	Exact, // An accepted match that terminates the search.
};

/**
 * Describes the stopping condition of a search without knowing the game world.
 * When requireSight is true, a non-empty SightCheck must accept the candidate.
 */
struct PathGoal
{
	Position target;
	SearchMode mode = SearchMode::Reach;
	DistanceRange distance;
	bool requireSight = true;

	[[nodiscard]] bool isInRange(int32_t startX, int32_t startY, int32_t testX, int32_t testY) const;
	[[nodiscard]] GoalMatch evaluate(int32_t startX, int32_t startY, int32_t testX, int32_t testY,
	                                 int32_t& bestMatch, SightCheck& sightCheck) const;
};
