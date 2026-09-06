#pragma once

#include "path_goal.h"
#include "position.h"
#include "search_mode.h"

#include <cstdint>

/**
 * Describes a pathfinding request assembled through a builder.
 * The goal target is also used as the search target for pruning and heuristics.
 */
class PathRequest
{
public:
	/**
	 * Creates a request for a target. The start defaults to the target until from() is called.
	 */
	[[nodiscard]] static PathRequest to(const Position& target);

	[[nodiscard]] const Position& start() const { return start_; }
	[[nodiscard]] const PathGoal& goal() const { return goal_; }
	[[nodiscard]] int32_t maxDistance() const { return maxDistance_; }
	[[nodiscard]] bool lineOfSightClear() const { return lineOfSightClear_; }
	[[nodiscard]] bool summoningTargetMaster() const { return summoningTargetMaster_; }

	PathRequest& from(const Position& position);

	PathRequest& mode(SearchMode mode);

	/**
	 * Sets the minimum and maximum accepted Chebyshev distances from the target.
	 */
	PathRequest& distance(int32_t minTargetDist, int32_t maxTargetDist);

	/**
	 * Sets the maximum search distance. Zero selects the default pre-flight and
	 * expansion limits.
	 */
	PathRequest& maxDistance(int32_t maxSearchDist);

	/**
	 * Enables or disables sight validation for goal candidates.
	 */
	PathRequest& requiringSight(bool require);

	/**
	 * Sets whether line-of-sight pruning is allowed.
	 */
	PathRequest& withLineOfSight(bool clearSight);

	/**
	 * Disables line-of-sight pruning when searching for the target's master.
	 */
	PathRequest& summonTargetMaster();

private:
	PathRequest() = default;

	PathGoal goal_;
	Position start_;
	int32_t maxDistance_ = 0;
	bool lineOfSightClear_ = true;
	bool summoningTargetMaster_ = false;
};
