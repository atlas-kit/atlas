#include "otpch.h"

#include "pathfinding/path_request.h"

PathRequest PathRequest::to(const Position& target)
{
	PathRequest request;
	request.start_ = target;
	request.goal_.target = target;
	return request;
}

PathRequest& PathRequest::from(const Position& position)
{
	start_ = position;
	return *this;
}

PathRequest& PathRequest::mode(SearchMode mode)
{
	goal_.mode = mode;
	return *this;
}

PathRequest& PathRequest::distance(int32_t minTargetDist, int32_t maxTargetDist)
{
	goal_.distance = { minTargetDist, maxTargetDist };
	return *this;
}

PathRequest& PathRequest::maxDistance(int32_t maxSearchDist)
{
	maxDistance_ = maxSearchDist;
	return *this;
}

PathRequest& PathRequest::requiringSight(bool require)
{
	goal_.requireSight = require;
	return *this;
}

PathRequest& PathRequest::withLineOfSight(bool clearSight)
{
	lineOfSightClear_ = clearSight;
	return *this;
}

PathRequest& PathRequest::summonTargetMaster()
{
	summoningTargetMaster_ = true;
	return *this;
}