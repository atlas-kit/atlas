#pragma once

#include "path_request.h"
#include "path_result.h"
#include "path_world.h"
#include "position.h"

#include <expected>
#include <vector>

/**
 * Cache-friendly A* search that reuses per-thread scratch storage after its
 * initial allocation. Instances are intentionally not constructible.
 *
 * find() applies the request pre-flight rules before searching:
 * - a start and target on different floors yield NoPath;
 * - a target within melee reach yields Found with an empty path, leaving the
 *   caller's dance-step to decide;
 * - a target beyond the configured search distance yields NoPath.
 *
 * directionList is cleared before any pre-flight rejection. A required sight
 * check must be supplied and accept the candidate.
 *
 * Obstacle and cost queries are supplied through TileCost and SightCheck.
 */
class PathFinder
{
public:
	PathFinder() = delete;

	/**
	 * Searches for a route and writes its directions to directionList.
	 * Directions are stored from the selected end node toward the start node so
	 * callers can consume the last element as the next step.
	 */
	[[nodiscard]] static PathStatus find(const PathRequest& request, TileCost getTileCost,
	                                     std::vector<Direction>& directionList, SightCheck sightCheck = {});

	/**
	 * Searches for a route and returns NoPath as an unexpected value. Partial
	 * results are returned together with their directions.
	 */
	[[nodiscard]] static std::expected<std::vector<Direction>, PathStatus> find(const PathRequest& request,
	                                                                             TileCost getTileCost,
	                                                                             SightCheck sightCheck = {});
};
