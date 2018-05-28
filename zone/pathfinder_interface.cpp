#include "../common/seperator.h"
#include "client.h"
#include "pathfinder_null.h"
#include "pathfinder_nav_mesh.h"
#include "pathfinder_waypoint.h"
#include <fmt/format.h>
#include <sys/stat.h>

IPathfinder *IPathfinder::Load(const std::string &zone) {
	struct stat statbuffer;
	std::string waypoint_path = fmt::format("maps/path/{0}.path", zone);
	std::string navmesh_path = fmt::format("maps/nav/{0}.nav", zone);
	if (stat(navmesh_path.c_str(), &statbuffer) == 0) {
		return new PathfinderNavmesh(navmesh_path);
	}
	
	if (stat(waypoint_path.c_str(), &statbuffer) == 0) {
		return new PathfinderWaypoint(waypoint_path);
	}

	return new PathfinderNull();
}
