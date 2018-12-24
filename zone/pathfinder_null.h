#pragma once

#include "pathfinder_interface.h"

class PathfinderNull : public IPathfinder
{
public:
	PathfinderNull() { }
	virtual ~PathfinderNull() { }

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags = PathingNotDisabled);
	virtual IPath FindPath(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, const PathfinderOptions& opts);
	virtual glm::vec3 GetRandomLocation(const glm::vec3 &start);
	virtual void DebugCommand(Client *c, const Seperator *sep) { }
};
