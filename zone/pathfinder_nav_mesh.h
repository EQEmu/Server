#pragma once

#include "pathfinder_interface.h"
#include <string>

class PathfinderNavmesh : public IPathfinder
{
public:
	PathfinderNavmesh(const std::string &path);
	virtual ~PathfinderNavmesh();

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck);
	virtual glm::vec3 GetRandomLocation();
	virtual void DebugCommand(Client *c, const Seperator *sep);

private:
	void Clear();
	void Load(const std::string &path);
	void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);

	struct Implementation;
	std::unique_ptr<Implementation> m_impl;
};