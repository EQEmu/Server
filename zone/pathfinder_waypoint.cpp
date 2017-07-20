#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>

#include "pathfinder_waypoint.h"
#include "zone.h"
#include "client.h"
#include "../common/eqemu_logsys.h"
#include "../common/rulesys.h"
#include <string>
#include <stdio.h>

extern Zone *zone;

#pragma pack(1)
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
	NeighbourNode Neighbours[50];
};

struct PathFileHeader {
	uint32 version;
	uint32 PathNodeCount;
};
#pragma pack()

struct Node
{
	glm::vec3 v;
	float bestz;
};

struct Edge
{
	float distance;
	bool teleport;
	int door_id;
};

template <class Graph, class CostType, class NodeMap>
class distance_heuristic : public boost::astar_heuristic<Graph, CostType>
{
public:
	typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

	distance_heuristic(NodeMap n, Vertex goal)
		: m_node(n), m_goal(goal) {}
	CostType operator()(Vertex u)
	{
		CostType dx = m_node[m_goal].v.x - m_node[u].v.x;
		CostType dy = m_node[m_goal].v.y - m_node[u].v.y;
		CostType dz = m_node[m_goal].v.z - m_node[u].v.z;
		return ::sqrt(dx * dx + dy * dy + dz * dz);
	}
private:
	NodeMap m_node;
	Vertex m_goal;
};

struct found_goal {};
template <class Vertex>
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
	astar_goal_visitor(Vertex goal) : m_goal(goal) {}
	template <class Graph>
	void examine_vertex(Vertex u, Graph& g) {
		if (u == m_goal)
			throw found_goal();
	}
private:
	Vertex m_goal;
};

typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> Point;
typedef boost::geometry::model::box<Point> Box;
typedef std::pair<Point, unsigned int> RTreeValue;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, boost::no_property,
	boost::property<boost::edge_weight_t, float>> GraphType;
typedef boost::property_map<GraphType, boost::edge_weight_t>::type WeightMap;

struct PathfinderWaypoint::Implementation {
	bool PathFileValid;
	boost::geometry::index::rtree<RTreeValue, boost::geometry::index::quadratic<16>> Tree;
	GraphType Graph;
	std::vector<Node> Nodes;
	std::map<std::pair<size_t, size_t>, Edge> Edges;
};

PathfinderWaypoint::PathfinderWaypoint(const std::string &path)
{
	PathFileHeader Head;
	m_impl.reset(new Implementation());
	m_impl->PathFileValid = false;
	Head.PathNodeCount = 0;
	Head.version = 2;

	FILE *f = fopen(path.c_str(), "rb");
	if (f) {
		char Magic[10];

		fread(&Magic, 9, 1, f);

		if (strncmp(Magic, "EQEMUPATH", 9))
		{
			Log(Logs::General, Logs::Error, "Bad Magic String in .path file.");
			return;
		}

		fread(&Head, sizeof(Head), 1, f);

		Log(Logs::General, Logs::Status, "Path File Header: Version %ld, PathNodes %ld",
			(long)Head.version, (long)Head.PathNodeCount);

		if (Head.version != 2)
		{
			Log(Logs::General, Logs::Error, "Unsupported path file version.");
			return;
		}

		std::unique_ptr<PathNode[]> PathNodes(new PathNode[Head.PathNodeCount]);

		fread(PathNodes.get(), sizeof(PathNode), Head.PathNodeCount, f);

		int MaxNodeID = Head.PathNodeCount - 1;

		m_impl->PathFileValid = true;

		m_impl->Nodes.reserve(Head.PathNodeCount);
		for (uint32 i = 0; i < Head.PathNodeCount; ++i)
		{
			auto &n = PathNodes[i];

			Point p = Point(n.v.x, n.v.y, n.v.z);
			m_impl->Tree.insert(std::make_pair(p, i));

			boost::add_vertex(m_impl->Graph);
			Node node;
			node.v = n.v;
			node.bestz = n.bestz;
			m_impl->Nodes.push_back(node);
		}

		auto weightmap = boost::get(boost::edge_weight, m_impl->Graph);
		for (uint32 i = 0; i < Head.PathNodeCount; ++i) {
			for (uint32 j = 0; j < 50; ++j)
			{
				if (PathNodes[i].Neighbours[j].id > MaxNodeID)
				{
					Log(Logs::General, Logs::Error, "Path Node %i, Neighbour %i (%i) out of range.", i, j, PathNodes[i].Neighbours[j].id);
					m_impl->PathFileValid = false;
				}
			
				if (PathNodes[i].Neighbours[j].id > 0) {
					GraphType::edge_descriptor e;
					bool inserted;
					boost::tie(e, inserted) = boost::add_edge(PathNodes[i].id, PathNodes[i].Neighbours[j].id, m_impl->Graph);
					weightmap[e] = PathNodes[i].Neighbours[j].distance;

					Edge edge;
					edge.distance = PathNodes[i].Neighbours[j].distance;
					edge.door_id = PathNodes[i].Neighbours[j].DoorID;
					edge.teleport = PathNodes[i].Neighbours[j].Teleport;

					m_impl->Edges[std::make_pair(PathNodes[i].id, PathNodes[i].Neighbours[j].id)] = edge;
				}
			}
		}

		fclose(f);
	}
}

PathfinderWaypoint::~PathfinderWaypoint()
{
}

IPathfinder::IPath PathfinderWaypoint::FindRoute(const glm::vec3 &start, const glm::vec3 &end)
{
	std::vector<RTreeValue> result_start_n;
	m_impl->Tree.query(boost::geometry::index::nearest(Point(start.x, start.y, start.z), 1), std::back_inserter(result_start_n));
	if (result_start_n.size() == 0) {
		IPath Route;
		Route.push_back(start);
		Route.push_back(end);
		return Route;
	}

	std::vector<RTreeValue> result_end_n;
	m_impl->Tree.query(boost::geometry::index::nearest(Point(end.x, end.y, end.z), 1), std::back_inserter(result_end_n));
	if (result_end_n.size() == 0) {
		IPath Route;
		Route.push_back(start);
		Route.push_back(end);
		return Route;
	}

	auto &nearest_start = *result_start_n.begin();
	auto &nearest_end = *result_end_n.begin();
	
	if (nearest_start.second == nearest_end.second) {
		IPath Route;
		Route.push_back(start);
		Route.push_back(end);
		return Route;
	}

	std::vector<GraphType::vertex_descriptor> p(boost::num_vertices(m_impl->Graph));
	try {
		boost::astar_search(m_impl->Graph, nearest_start.second, 
			distance_heuristic<GraphType, float, Node*>(&m_impl->Nodes[0], nearest_end.second),
			boost::predecessor_map(&p[0])
				.visitor(astar_goal_visitor<size_t>(nearest_end.second)));
	}
	catch (found_goal)
	{
		IPath Route;
		
		Route.push_front(end);
		for (size_t v = nearest_end.second;; v = p[v]) {
			if (p[v] == v) {
				Route.push_front(m_impl->Nodes[v].v);
				break;
			}
			else {
				auto iter = m_impl->Edges.find(std::make_pair(p[v], p[v + 1]));
				if (iter != m_impl->Edges.end()) {
					auto &edge = iter->second;
					if (edge.teleport) {
						Route.push_front(m_impl->Nodes[v].v);
						glm::vec3 teleport(100000000.0f, 100000000.0f, 100000000.0f);
						Route.push_front(teleport);
					}
					else {
						Route.push_front(m_impl->Nodes[v].v);
					}
				}
				else {
					Route.push_front(m_impl->Nodes[v].v);
				}
			}
		}
		Route.push_front(start);

		return Route;
	}

	IPath Route;
	Route.push_back(start);
	Route.push_back(end);
	return Route;
}

glm::vec3 PathfinderWaypoint::GetRandomLocation()
{
	return glm::vec3();
}

void PathfinderWaypoint::DebugCommand(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help"))
	{
		c->Message(0, "Syntax: #path shownodes: Spawns a npc to represent every npc node.");
		c->Message(0, "#path info node_id: Gives information about node info (requires shownode target).");
		c->Message(0, "#path dump file_name: Dumps the current zone->pathing to a file of your naming.");
		c->Message(0, "#path add [requested_id]: Adds a node at your current location will try to take the requested id if possible.");
		c->Message(0, "#path connect connect_to_id [is_teleport] [door_id]: Connects the currently targeted node to connect_to_id's node and connects that node back (requires shownode target).");
		c->Message(0, "#path sconnect connect_to_id [is_teleport] [door_id]: Connects the currently targeted node to connect_to_id's node (requires shownode target).");
		c->Message(0, "#path qconnect [set]: short cut connect, connects the targeted node to the node you set with #path qconnect set (requires shownode target).");
		c->Message(0, "#path disconnect [all]/disconnect_from_id: Disconnects the currently targeted node to disconnect from disconnect from id's node (requires shownode target), if passed all as the second argument it will disconnect this node from every other node.");
		c->Message(0, "#path move: Moves your targeted node to your current position");
		c->Message(0, "#path process file_name: processes the map file and tries to automatically generate a rudimentary path setup and then dumps the current zone->pathing to a file of your naming.");
		c->Message(0, "#path resort [nodes]: resorts the connections/nodes after you've manually altered them so they'll work.");
		return;
	}

	if(!strcasecmp(sep->arg[1], "shownodes"))
	{
		ShowNodes();	
		return;
	}

	if (!strcasecmp(sep->arg[1], "show"))
	{
		if (c->GetTarget() != nullptr) {
			auto target = c->GetTarget();
			glm::vec3 start(target->GetX(), target->GetY(), target->GetZ());
			glm::vec3 end(c->GetX(), c->GetY(), c->GetZ());

			ShowPath(start, end);
		}

		return;
	}
}

void PathfinderWaypoint::ShowNodes()
{
	for (size_t i = 0; i < m_impl->Nodes.size(); ++i)
	{
		auto npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));
		
		auto c = i / 1000u;
		sprintf(npc_type->name, "Node%u", c);
		npc_type->cur_hp = 4000000;
		npc_type->max_hp = 4000000;
		npc_type->race = 2254;
		npc_type->gender = 2;
		npc_type->class_ = 9;
		npc_type->deity = 1;
		npc_type->level = 75;
		npc_type->npc_id = 0;
		npc_type->loottable_id = 0;
		npc_type->texture = 1;
		npc_type->light = 0;
		npc_type->runspeed = 0;
		npc_type->d_melee_texture1 = 1;
		npc_type->d_melee_texture2 = 1;
		npc_type->merchanttype = 1;
		npc_type->bodytype = 1;
		
		npc_type->STR = 150;
		npc_type->STA = 150;
		npc_type->DEX = 150;
		npc_type->AGI = 150;
		npc_type->INT = 150;
		npc_type->WIS = 150;
		npc_type->CHA = 150;
		
		npc_type->findable = 1;
		auto position = glm::vec4(m_impl->Nodes[i].v.x, m_impl->Nodes[i].v.y, m_impl->Nodes[i].v.z, 0.0f);
		auto npc = new NPC(npc_type, nullptr, position, FlyMode1);
		npc->GiveNPCTypeData(npc_type);
		
		entity_list.AddNPC(npc, true, true);
	}
}

void PathfinderWaypoint::ShowPath(const glm::vec3 &start, const glm::vec3 &end)
{
	auto path = FindRoute(start, end);

	for (auto &node : path)
	{
		auto npc_type = new NPCType;
		memset(npc_type, 0, sizeof(NPCType));
	
		sprintf(npc_type->name, "Path");
		npc_type->cur_hp = 4000000;
		npc_type->max_hp = 4000000;
		npc_type->race = 2254;
		npc_type->gender = 2;
		npc_type->class_ = 9;
		npc_type->deity = 1;
		npc_type->level = 75;
		npc_type->npc_id = 0;
		npc_type->loottable_id = 0;
		npc_type->texture = 1;
		npc_type->light = 0;
		npc_type->runspeed = 0;
		npc_type->d_melee_texture1 = 1;
		npc_type->d_melee_texture2 = 1;
		npc_type->merchanttype = 1;
		npc_type->bodytype = 1;
	
		npc_type->STR = 150;
		npc_type->STA = 150;
		npc_type->DEX = 150;
		npc_type->AGI = 150;
		npc_type->INT = 150;
		npc_type->WIS = 150;
		npc_type->CHA = 150;
	
		npc_type->findable = 1;
		auto position = glm::vec4(node.x, node.y, node.z, 0.0f);
		auto npc = new NPC(npc_type, nullptr, position, FlyMode1);
		npc->GiveNPCTypeData(npc_type);
	
		entity_list.AddNPC(npc, true, true);
	}
}
