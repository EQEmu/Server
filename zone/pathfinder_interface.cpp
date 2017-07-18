#include "pathfinder_null.h"
#include "pathfinder_nav_mesh.h"
#include "pathfinder_waypoint.h"

IPathfinder *IPathfinder::Load(const std::string &zone) {
	return new PathfinderNull();
}