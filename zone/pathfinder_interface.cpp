#include "../common/seperator.h"
#include "client.h"
#include "pathfinder_null.h"
#include "pathfinder_nav_mesh.h"
#include <fmt/format.h>
#include <sys/stat.h>

IPathfinder *IPathfinder::Load(const std::string &zone) {
	struct stat statbuffer;
	std::string navmesh_path = fmt::format("{}/maps/nav/{}.nav", path.GetServerPath(), zone);
	if (stat(navmesh_path.c_str(), &statbuffer) == 0) {
		return new PathfinderNavmesh(navmesh_path);
	}

	return new PathfinderNull();
}
