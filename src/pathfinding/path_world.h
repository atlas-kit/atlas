#pragma once

#include "../position.h"

#include <cstdint>
#include <functional>

/**
 * Returns the additional cost of entering a tile. A cost of zero means blocked.
 */
using TileCost = std::move_only_function<uint16_t(int32_t, int32_t)>;

/**
 * Returns whether a straight line between two positions is clear of obstacles.
 */
using SightCheck = std::move_only_function<bool(const Position&, const Position&)>;
