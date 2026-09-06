#pragma once

#include <cstddef>
#include <cstdint>

// Movement costs are added to the TileCost value for each newly claimed cell.
static constexpr uint16_t PATHFIND_NORMAL_COST = 10;
static constexpr uint16_t PATHFIND_DIAGONAL_COST = 25;

// Iteration and node limits are hard failures: the search returns NoPath instead
// of reconstructing a retained non-exact match when either limit is exhausted.
inline constexpr size_t PATHFIND_ITERATION_BUDGET = 121;
inline constexpr size_t PATHFIND_NODE_BUDGET = 181;
