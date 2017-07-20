#pragma once

#include "pathfinder_interface.h"

class PathfinderNavmesh : public IPathfinder
{
public:
	PathfinderNavmesh() { }
	virtual ~PathfinderNavmesh() { }

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end);
	virtual glm::vec3 GetRandomLocation();
	virtual void DebugCommand(Client *c, const Seperator *sep);
};