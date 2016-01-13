#pragma once

#include <string>
#include <vector>
#include <DetourNavMesh.h>
#include <glm/vec3.hpp>

enum NavigationAreaFlags
{
	NavigationAreaFlagNormal,
	NavigationAreaFlagWater,
	NavigationAreaFlagLava,
	NavigationAreaFlagZoneLine,
	NavigationAreaFlagPvP,
	NavigationAreaFlagSlime,
	NavigationAreaFlagIce,
	NavigationAreaFlagVWater,
	NavigationAreaFlagGeneralArea,
	NavigationAreaFlagPortal,
	NavigationAreaFlagDisabled,
};

enum NavigationPolyFlags
{
	NavigationPolyFlagNormal = 1,
	NavigationPolyFlagWater = 2,
	NavigationPolyFlagLava = 4,
	NavigationPolyFlagZoneLine = 8,
	NavigationPolyFlagPvP = 16,
	NavigationPolyFlagSlime = 32,
	NavigationPolyFlagIce = 64,
	NavigationPolyFlagVWater = 128,
	NavigationPolyFlagGeneralArea = 256,
	NavigationPolyFlagPortal = 512,
	NavigationPolyFlagDisabled = 1024,
	NavigationPolyFlagAll = 0xFFFF
};

struct PathfindingNode
{
	glm::vec3 position;
	unsigned short flag;
};

class PathfindingRoute
{
public:
	PathfindingRoute();
	~PathfindingRoute();

	bool Valid(const glm::vec3 &dest);
	void CalcCurrentNode();
	const PathfindingNode& GetCurrentNode() { return m_nodes[m_current_node]; }
	const std::vector<PathfindingNode>& GetNodes() const { return m_nodes; }
	std::vector<PathfindingNode>& GetNodesEdit() { return m_nodes; }
private:
	glm::vec3 m_dest;
	std::vector<PathfindingNode> m_nodes;
	int m_current_node;

	friend class PathfindingManager;
};

class PathfindingManager
{
public:
	PathfindingManager();
	~PathfindingManager();

	void Load(const std::string &filename);
	void Clear();

	//Expects locations in EQEmu internal format eg what #loc returns not what /loc returns.
	PathfindingRoute FindRoute(const glm::vec3 &current_location, const glm::vec3 &dest_location);
private:
	dtNavMesh *m_nav_mesh;
};
