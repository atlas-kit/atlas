#pragma once

#include <cstdint>

/**
 * Describes the target behavior of a pathfinding search.
 */
enum class SearchMode : uint8_t
{
	Reach, // Search for an accepted position in the target range.
	Approach, // Search while retaining the target-relative approach bounds.
	Flee, // Keep expansion inside the target-relative bounds and seek their far edge.
};
