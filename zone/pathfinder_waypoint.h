#pragma once

#include "pathfinder_interface.h"

struct PathFileHeader;

class PathfinderWaypoint : public IPathfinder
{
public:
	PathfinderWaypoint(const std::string &path);
	virtual ~PathfinderWaypoint();

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end);
	virtual glm::vec3 GetRandomLocation();
	virtual void DebugCommand(Client *c, const Seperator *sep);

private:
	void LoadV2(FILE *f, const PathFileHeader &header);
	void LoadV3(FILE *f, const PathFileHeader &header);
	void ShowNodes();
	void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);

	struct Implementation;
	std::unique_ptr<Implementation> m_impl;
};