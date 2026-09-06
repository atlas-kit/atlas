#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstdint>

/**
 * Minimum and maximum Chebyshev distances accepted by a PathGoal.
 */
struct DistanceRange
{
	int32_t min = 0;
	int32_t max = 1;
};

/**
 * Returns the Manhattan distance between two coordinates.
 */
[[nodiscard]] inline int32_t manhattanDistance(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	return std::abs(x1 - x2) + std::abs(y1 - y2);
}

/**
 * Returns the Chebyshev distance between two coordinates.
 */
[[nodiscard]] inline int32_t chebyshevDistance(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	return std::max(std::abs(x1 - x2), std::abs(y1 - y2));
}

/**
 * Returns the squared Euclidean distance between two coordinates.
 */
[[nodiscard]] inline int32_t squaredDistance(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	const auto dx = x1 - x2;
	const auto dy = y1 - y2;
	return dx * dx + dy * dy;
}
