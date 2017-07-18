#pragma once

#include "pathfinder_interface.h"

class PathfinderNull : public IPathfinder
{
public:
	PathfinderNull() { }
	virtual ~PathfinderNull() { }

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end);
	virtual glm::vec3 GetRandomLocation();
};