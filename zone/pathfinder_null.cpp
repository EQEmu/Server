#include "pathfinder_null.h"
#pragma once

IPathfinder::IPath PathfinderNull::FindRoute(const glm::vec3 &start, const glm::vec3 &end)
{
	IPath ret;
	ret.push_back(start);
	ret.push_back(end);
	return ret;
}

glm::vec3 PathfinderNull::GetRandomLocation()
{
	return glm::vec3();
}
