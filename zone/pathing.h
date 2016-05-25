#ifndef PATHING_H
#define PATHING_H

#include "map.h"
#include "zone_config.h"
#include <deque>

extern const ZoneConfig *Config;

class Client;
class Mob;

#define PATHNODENEIGHBOURS 50

#pragma pack(1)

struct AStarNode
{
	int PathNodeID;
	int Parent;
	float HCost;
	float GCost;
	bool Teleport;
};

struct NeighbourNode {
	int16 id;
	float distance;
	uint8 Teleport;
	int16 DoorID;
};

struct PathNode {
	uint16 id;
	glm::vec3 v;
	float bestz;
	NeighbourNode Neighbours[PATHNODENEIGHBOURS];
};

struct PathFileHeader {
	uint32 version;
	uint32 PathNodeCount;
};

#pragma pack()

struct PathNodeSortStruct
{
	int id;
	float Distance;
};

enum LOSType{ UnknownLOS, HaveLOS, NoLOS };

class PathManager {

public:
	PathManager();
	~PathManager();


	static PathManager *LoadPathFile(const char *ZoneName);
	bool loadPaths(FILE *fp);
	void PrintPathing();
	std::deque<int> FindRoute(glm::vec3 Start, glm::vec3 End);
	std::deque<int> FindRoute(int startID, int endID);

	glm::vec3 GetPathNodeCoordinates(int NodeNumber, bool BestZ = true);
	bool CheckLosFN(glm::vec3 a, glm::vec3 b);
	void SpawnPathNodes();
	void MeshTest();
	void SimpleMeshTest();
	int FindNearestPathNode(glm::vec3 Position);
	bool NoHazards(glm::vec3 From, glm::vec3 To);
	bool NoHazardsAccurate(glm::vec3 From, glm::vec3 To);
	void OpenDoors(int Node1, int Node2, Mob* ForWho);

	PathNode* FindPathNodeByCoordinates(float x, float y, float z);
	void ShowPathNodeNeighbours(Client *c);
	int GetRandomPathNode();

	void NodeInfo(Client *c);
	int32 AddNode(float x, float y, float z, float best_z, int32 requested_id = 0); //return -1 on failure, else returns the id of this node
	bool DeleteNode(Client *c);
	bool DeleteNode(int32 id); //returns true on success, false on failure, tries to delete a node from this map
	void ConnectNodeToNode(Client *c, int32 Node2, int32 teleport = 0, int32 doorid = -1); //connects a node both ways
	void ConnectNodeToNode(int32 Node1, int32 Node2, int32 teleport = 0, int32 doorid = -1);
	void ConnectNode(Client *c, int32 Node2, int32 teleport = 0, int32 doorid = -1); //connects a node one way
	void ConnectNode(int32 Node1, int32 Node2, int32 teleport = 0, int32 doorid = -1);
	void DisconnectNodeToNode(Client *c, int32 Node2);
	void DisconnectNodeToNode(int32 Node1, int32 Node2);
	void MoveNode(Client *c);
	void DisconnectAll(Client *c);
	bool NodesConnected(PathNode *a, PathNode *b);
	void DumpPath(std::string filename);
	void ProcessNodesAndSave(std::string filename);
	void ResortConnections();
	void QuickConnect(Client *c, bool set = false);
	void SortNodes();

private:
	PathFileHeader Head;
	PathNode *PathNodes;
	int QuickConnectTarget;

	int *ClosedListFlag;
};


#endif

