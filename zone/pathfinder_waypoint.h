#pragma once

#include "pathfinder_interface.h"
#include <memory>

class PathfinderWaypoint : public IPathfinder
{
public:
	PathfinderWaypoint(const std::string &path);
	virtual ~PathfinderWaypoint();

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end);
	virtual glm::vec3 GetRandomLocation();
	virtual void DebugCommand(Client *c, const Seperator *sep);

private:
	void ShowNodes();
	void ShowPath(const glm::vec3 &start, const glm::vec3 &end);

	struct Implementation;
	std::unique_ptr<Implementation> m_impl;
};