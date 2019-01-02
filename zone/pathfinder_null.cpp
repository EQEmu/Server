#include "pathfinder_null.h"

IPathfinder::IPath PathfinderNull::FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags)
{
	partial = false;
	stuck = false;
	IPath ret;
	ret.push_back(start);
	ret.push_back(end);
	return ret;
}

IPathfinder::IPath PathfinderNull::FindPath(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, const PathfinderOptions &opts)
{
	partial = false;
	stuck = false;
	IPath ret;
	ret.push_back(start);
	ret.push_back(end);
	return ret;
}

glm::vec3 PathfinderNull::GetRandomLocation(const glm::vec3 &start)
{
	return glm::vec3(0.0f);
}
