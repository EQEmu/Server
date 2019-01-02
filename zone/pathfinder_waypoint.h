#pragma once

#include "pathfinder_interface.h"

struct PathFileHeader;
struct Node;

class PathfinderWaypoint : public IPathfinder
{
public:
	PathfinderWaypoint(const std::string &path);
	virtual ~PathfinderWaypoint();

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags = PathingNotDisabled);
	virtual glm::vec3 GetRandomLocation(const glm::vec3 &start);
	virtual void DebugCommand(Client *c, const Seperator *sep);

private:
	void Load(const std::string &filename);
	void LoadV2(FILE *f, const PathFileHeader &header);
	void LoadV3(FILE *f, const PathFileHeader &header);
	void ShowNodes();
	void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);
	void NodeInfo(Client *c);
	Node *FindPathNodeByCoordinates(float x, float y, float z);
	void BuildGraph();
	void ShowNode(const Node &n);

	struct Implementation;
	std::unique_ptr<Implementation> m_impl;
};
