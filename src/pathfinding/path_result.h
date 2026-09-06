#pragma once

#include <cstdint>

/**
 * Outcome of a pathfinding search.
 */
enum class PathStatus : uint8_t
{
	NoPath, // No acceptable route was found, or a pre-flight or search limit rejected it.
	Found, // An exact goal match was found, or the melee shortcut was accepted.
	Partial, // Natural open-list exhaustion retained an in-range non-exact match.
};
