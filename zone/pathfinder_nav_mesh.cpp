#include <memory>
#include <stdio.h>
#include <vector>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include "pathfinder_nav_mesh.h"

#include "zone.h"
#include "water_map.h"
#include "client.h"
#include "../common/compression.h"

extern Zone *zone;

const int MaxNavmeshNodes = 4092;

struct PathfinderNavmesh::Implementation
{
	dtNavMesh *nav_mesh;
	dtNavMeshQuery *query;
};

PathfinderNavmesh::PathfinderNavmesh(const std::string &path)
{
	m_impl.reset(new Implementation());
	m_impl->nav_mesh = nullptr;
	m_impl->query = nullptr;
	Load(path);
}

PathfinderNavmesh::~PathfinderNavmesh()
{
	Clear();
}

IPathfinder::IPath PathfinderNavmesh::FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck)
{
	partial = false;

	if (!m_impl->nav_mesh) {
		return IPath();
	}

	if (!m_impl->query) {
		m_impl->query = dtAllocNavMeshQuery();
		m_impl->query->init(m_impl->nav_mesh, MaxNavmeshNodes);
	}

	glm::vec3 current_location(start.x, start.z, start.y);
	glm::vec3 dest_location(end.x, end.z, end.y);

	dtQueryFilter filter;
	filter.setIncludeFlags(65535U ^ 2048);
	filter.setAreaCost(0, 1.0f); //Normal
	filter.setAreaCost(1, 3.0f); //Water
	filter.setAreaCost(2, 5.0f); //Lava
	filter.setAreaCost(4, 1.0f); //PvP
	filter.setAreaCost(5, 2.0f); //Slime
	filter.setAreaCost(6, 2.0f); //Ice
	filter.setAreaCost(7, 4.0f); //V Water (Frigid Water)
	filter.setAreaCost(8, 1.0f); //General Area
	filter.setAreaCost(9, 0.1f); //Portal
	filter.setAreaCost(10, 0.1f); //Prefer

	dtPolyRef start_ref;
	dtPolyRef end_ref;
	glm::vec3 ext(5.0f, 100.0f, 5.0f);

	m_impl->query->findNearestPoly(&current_location[0], &ext[0], &filter, &start_ref, 0);
	m_impl->query->findNearestPoly(&dest_location[0], &ext[0], &filter, &end_ref, 0);

	if (!start_ref || !end_ref) {
		return IPath();
	}

	int npoly = 0;
	dtPolyRef path[1024] = { 0 };
	auto status = m_impl->query->findPath(start_ref, end_ref, &current_location[0], &dest_location[0], &filter, path, &npoly, 1024);

	if (npoly) {
		glm::vec3 epos = dest_location;
		if (path[npoly - 1] != end_ref) {
			m_impl->query->closestPointOnPoly(path[npoly - 1], &dest_location[0], &epos[0], 0);
			partial = true;

			auto dist = DistanceSquared(epos, current_location);
			if (dist < 10000.0f) {
				stuck = true;
			}
		}

		float straight_path[2048 * 3];
		unsigned char straight_path_flags[2048];

		int n_straight_polys;
		dtPolyRef straight_path_polys[2048];

		status = m_impl->query->findStraightPath(&current_location[0], &epos[0], path, npoly,
			straight_path, straight_path_flags,
			straight_path_polys, &n_straight_polys, 2048, DT_STRAIGHTPATH_AREA_CROSSINGS);

		if (dtStatusFailed(status)) {
			return IPath();
		}

		if (n_straight_polys) {
			IPath Route;
			for (int i = 0; i < n_straight_polys; ++i)
			{
				glm::vec3 node;
				node.x = straight_path[i * 3];
				node.z = straight_path[i * 3 + 1];
				node.y = straight_path[i * 3 + 2];

				Route.push_back(node);

				unsigned short flag = 0;
				if (dtStatusSucceed(m_impl->nav_mesh->getPolyFlags(straight_path_polys[i], &flag))) {
					if (flag & 512) {
						Route.push_back(true);
					}
				}
			}

			return Route;
		}
	}

	IPath Route;
	Route.push_back(end);
	return Route;
}

glm::vec3 PathfinderNavmesh::GetRandomLocation()
{
	if (!m_impl->nav_mesh) {
		return glm::vec3();
	}

	if (!m_impl->query) {
		m_impl->query = dtAllocNavMeshQuery();
		m_impl->query->init(m_impl->nav_mesh, MaxNavmeshNodes);
	}

	dtQueryFilter filter;
	filter.setIncludeFlags(65535U);

	dtPolyRef randomRef;
	float point[3];

	if (dtStatusSucceed(m_impl->query->findRandomPoint(&filter, []() {
		return (float)zone->random.Real(0.0, 1.0);
	}, &randomRef, point))) 
	{
		return glm::vec3(point[0], point[2], point[1]);
	}

	return glm::vec3();
}

void PathfinderNavmesh::DebugCommand(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help"))
	{
		c->Message(0, "#path show: Plots a path from the user to their target.");
		return;
	}

	if (!strcasecmp(sep->arg[1], "show"))
	{
		if (c->GetTarget() != nullptr) {
			auto target = c->GetTarget();
			glm::vec3 start(c->GetX(), c->GetY(), c->GetZ());
			glm::vec3 end(target->GetX(), target->GetY(), target->GetZ());

			ShowPath(c, start, end);
		}

		return;
	}
}

void PathfinderNavmesh::Clear()
{
	if (m_impl->nav_mesh) {
		dtFreeNavMesh(m_impl->nav_mesh);
	}

	if (m_impl->query) {
		dtFreeNavMeshQuery(m_impl->query);
	}
}

void PathfinderNavmesh::Load(const std::string &path)
{
	Clear();

	FILE *f = fopen(path.c_str(), "rb");
	if (f) {
		char magic[9] = { 0 };
		if (fread(magic, 9, 1, f) != 1) {
			fclose(f);
			return;
		}

		if (strncmp(magic, "EQNAVMESH", 9) != 0)
		{
			fclose(f);
			return;
		}

		uint32_t version = 0;
		if (fread(&version, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (version != 2) {
			fclose(f);
			return;
		}

		uint32_t data_size;
		if (fread(&data_size, sizeof(data_size), 1, f) != 1) {
			fclose(f);
			return;
		}

		uint32_t buffer_size;
		if (fread(&buffer_size, sizeof(buffer_size), 1, f) != 1) {
			fclose(f);
			return;
		}

		std::vector<char> data;
		data.resize(data_size);
		if (fread(&data[0], data_size, 1, f) != 1) {
			fclose(f);
			return;
		}

		std::vector<char> buffer;
		buffer.resize(buffer_size);
		uint32_t v = EQEmu::InflateData(&data[0], data_size, &buffer[0], buffer_size);
		fclose(f);

		char *buf = &buffer[0];
		m_impl->nav_mesh = dtAllocNavMesh();

		uint32_t number_of_tiles = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		dtNavMeshParams params = *(dtNavMeshParams*)buf;
		buf += sizeof(dtNavMeshParams);

		dtStatus status = m_impl->nav_mesh->init(&params);
		if (dtStatusFailed(status))
		{
			dtFreeNavMesh(m_impl->nav_mesh);
			m_impl->nav_mesh = nullptr;
			return;
		}

		for (unsigned int i = 0; i < number_of_tiles; ++i)
		{
			uint32_t tile_ref = *(uint32_t*)buf;
			buf += sizeof(uint32_t);

			int32_t data_size = *(uint32_t*)buf;
			buf += sizeof(uint32_t);

			if (!tile_ref || !data_size) {
				dtFreeNavMesh(m_impl->nav_mesh);
				m_impl->nav_mesh = nullptr;
				return;
			}

			unsigned char* data = (unsigned char*)dtAlloc(data_size, DT_ALLOC_PERM);
			memcpy(data, buf, data_size);
			buf += data_size;

			m_impl->nav_mesh->addTile(data, data_size, DT_TILE_FREE_DATA, tile_ref, 0);
		}

		Log(Logs::General, Logs::Status, "Loaded Navmesh V%u file %s", version, path.c_str());
	}
}

void PathfinderNavmesh::ShowPath(Client * c, const glm::vec3 &start, const glm::vec3 &end)
{
	bool partial = false;
	bool stuck = false;
	auto path = FindRoute(start, end, partial, stuck);
	std::vector<FindPerson_Point> points;

	if (!partial) {
		FindPerson_Point p;
		for (auto &node : path)
		{
			if (!node.teleport) {
				p.x = node.pos.x;
				p.y = node.pos.y;
				p.z = node.pos.z;

				points.push_back(p);
			}
		}
	}

	c->SendPathPacket(points);
}
