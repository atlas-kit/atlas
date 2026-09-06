#include "otpch.h"

#include "pathfinding/path_goal.h"

bool PathGoal::isInRange(int32_t startX, int32_t startY, int32_t testX, int32_t testY) const
{
	const auto targetX = static_cast<int32_t>(target.x);
	const auto targetY = static_cast<int32_t>(target.y);

	if (mode == SearchMode::Reach) {
		if (testX > targetX + distance.max || testX < targetX - distance.max) {
			return false;
		}
		
		if (testY > targetY + distance.max || testY < targetY - distance.max) {
			return false;
		}
		return true;
	}

	// Approach and Flee keep the target-relative bounds on the side containing the
	// start; an axis aligned with the target keeps both sides available.
	const auto offsetX = startX - targetX;
	const auto offsetY = startY - targetY;

	const auto limitHighX = offsetX >= 0 ? distance.max : 0;
	if (testX > targetX + limitHighX) {
		return false;
	}

	const auto limitLowX = offsetX <= 0 ? distance.max : 0;
	if (testX < targetX - limitLowX) {
		return false;
	}

	const auto limitHighY = offsetY >= 0 ? distance.max : 0;
	if (testY > targetY + limitHighY) {
		return false;
	}

	const auto limitLowY = offsetY <= 0 ? distance.max : 0;
	if (testY < targetY - limitLowY) {
		return false;
	}
	return true;
}

GoalMatch PathGoal::evaluate(int32_t startX, int32_t startY, int32_t testX, int32_t testY, int32_t& bestMatch,
                             SightCheck& sightCheck) const
{
	// A candidate must pass the mode-dependent bounds, optional sight check, and
	// distance range. Candidates at the outer distance bound are terminal;
	// other accepted candidates update bestMatch only when they improve it.
	if (!isInRange(startX, startY, testX, testY)) {
		return GoalMatch::None;
	}

	if (requireSight) {
		if (!sightCheck) {
			return GoalMatch::None;
		}

		const Position testPosition(testX, testY, target.z);
		if (!sightCheck(testPosition, target)) {
			return GoalMatch::None;
		}
	}

	const auto testDistance = chebyshevDistance(testX, testY, target.x, target.y);
	if (distance.max == 1) {
		if (testDistance < distance.min || testDistance > distance.max) {
			return GoalMatch::None;
		}
		return GoalMatch::Exact;
	}

	if (testDistance > distance.max || testDistance < distance.min) {
		return GoalMatch::None;
	}

	if (testDistance == distance.max) {
		bestMatch = 0;
		return GoalMatch::Exact;
	}

	if (testDistance > bestMatch) {
		bestMatch = testDistance;
		return GoalMatch::Better;
	}
	return GoalMatch::None;
}