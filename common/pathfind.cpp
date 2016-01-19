#include "pathfind.h"
#include "random.h"

#include <stdio.h>
#include <string.h>
#include <zlib.h>

namespace Pathfind
{
	uint32_t InflateData(const char* buffer, uint32_t len, char* out_buffer, uint32_t out_len_max) {
		z_stream zstream;
		int zerror = 0;
		int i;

		zstream.next_in = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(buffer));
		zstream.avail_in = len;
		zstream.next_out = reinterpret_cast<unsigned char*>(out_buffer);;
		zstream.avail_out = out_len_max;
		zstream.zalloc = Z_NULL;
		zstream.zfree = Z_NULL;
		zstream.opaque = Z_NULL;

		i = inflateInit2(&zstream, 15);
		if (i != Z_OK) {
			return 0;
		}

		zerror = inflate(&zstream, Z_FINISH);
		if (zerror == Z_STREAM_END) {
			inflateEnd(&zstream);
			return zstream.total_out;
		}
		else {
			if (zerror == -4 && zstream.msg == 0)
			{
				return 0;
			}

			zerror = inflateEnd(&zstream);
			return 0;
		}
	}
}

const uint32_t nav_mesh_file_version = 2;
const float max_dest_drift = 10.0f;
const float at_waypoint_eps = 1.0f;
EQEmu::Random path_rng;

float vec_dist(const glm::vec3 &a, const glm::vec3 &b) {
	float dist_x = a.x - b.x;
	float dist_y = a.y - b.y;
	float dist_z = a.z - b.z;
	return sqrt((dist_x * dist_x) + (dist_y * dist_y) + (dist_z * dist_z));
}

PathfindingManager::PathfindingManager()
{
	m_nav_mesh = nullptr;
	m_nav_query = nullptr;
	m_filter.setIncludeFlags(NavigationPolyFlagAll);
	m_filter.setAreaCost(NavigationAreaFlagNormal, 1.0f);
	m_filter.setAreaCost(NavigationAreaFlagWater, 2.5f);
	m_filter.setAreaCost(NavigationAreaFlagLava, 2.5f);
	m_filter.setAreaCost(NavigationAreaFlagPvP, 1.0f);
	m_filter.setAreaCost(NavigationAreaFlagSlime, 1.0f);
	m_filter.setAreaCost(NavigationAreaFlagIce, 1.0f);
	m_filter.setAreaCost(NavigationAreaFlagVWater, 2.5f);
	m_filter.setAreaCost(NavigationAreaFlagGeneralArea, 1.0f);
	m_filter.setAreaCost(NavigationAreaFlagPortal, 1.0f);
}

PathfindingManager::~PathfindingManager()
{
	Clear();
}

void PathfindingManager::Load(const std::string &zone_name)
{
	Clear();

	std::string filename = MAP_DIR + std::string("/") + zone_name + ".nav";
	FILE *f = fopen(filename.c_str(), "rb");
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

		if (version != nav_mesh_file_version) {
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
		uint32_t v = Pathfind::InflateData(&data[0], data_size, &buffer[0], buffer_size);
		fclose(f);

		char *buf = &buffer[0];
		m_nav_mesh = dtAllocNavMesh();

		uint32_t number_of_tiles = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		dtNavMeshParams params = *(dtNavMeshParams*)buf;
		buf += sizeof(dtNavMeshParams);

		dtStatus status = m_nav_mesh->init(&params);
		if (dtStatusFailed(status))
		{
			dtFreeNavMesh(m_nav_mesh);
			m_nav_mesh = nullptr;
			return;
		}

		for (unsigned int i = 0; i < number_of_tiles; ++i)
		{
			uint32_t tile_ref = *(uint32_t*)buf;
			buf += sizeof(uint32_t);

			int32_t data_size = *(uint32_t*)buf;
			buf += sizeof(uint32_t);

			if (!tile_ref || !data_size) {
				dtFreeNavMesh(m_nav_mesh);
				m_nav_mesh = nullptr;
				return;
			}

			unsigned char* data = (unsigned char*)dtAlloc(data_size, DT_ALLOC_PERM);
			memcpy(data, buf, data_size);
			buf += data_size;

			m_nav_mesh->addTile(data, data_size, DT_TILE_FREE_DATA, tile_ref, 0);
		}
	}
}

void PathfindingManager::Clear()
{
	if (m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
		m_nav_mesh = nullptr;
	}

	if (m_nav_query) {
		dtFreeNavMeshQuery(m_nav_query);
		m_nav_query = nullptr;
	}
}

PathfindingRoute PathfindingManager::FindRoute(const glm::vec3 &src_loc, const glm::vec3 &dest_loc)
{
	glm::vec3 current_location(src_loc.x, src_loc.z, src_loc.y);
	glm::vec3 dest_location(dest_loc.x, dest_loc.z, dest_loc.y);

	PathfindingRoute ret;

	ret.m_dest = dest_loc;
	ret.m_current_node = 0;
	ret.m_active = true;

	if (!m_nav_mesh) {
		PathfindingNode dest;
		dest.flag = NavigationPolyFlagNormal;
		dest.position = dest_loc;
		ret.m_nodes.push_back(dest);
		return ret;
	}

	if (!m_nav_query) {
		m_nav_query = dtAllocNavMeshQuery();
		m_nav_query->init(m_nav_mesh, 4092);
	}

	dtPolyRef start_ref;
	dtPolyRef end_ref;
	glm::vec3 ext(10.0f, 10.0f, 10.0f);

	m_nav_query->findNearestPoly(&current_location[0], &ext[0], &m_filter, &start_ref, 0);
	m_nav_query->findNearestPoly(&dest_location[0], &ext[0], &m_filter, &end_ref, 0);

	if (!start_ref || !end_ref) {
		PathfindingNode dest;
		dest.flag = NavigationPolyFlagNormal;
		dest.position = dest_loc;
		ret.m_nodes.push_back(dest);
		return ret;
	}

	int npoly = 0;
	dtPolyRef path[256] = { 0 };
	m_nav_query->findPath(start_ref, end_ref, &current_location[0], &dest_location[0], &m_filter, path, &npoly, 256);

	if (npoly) {
		glm::vec3 epos = dest_location;
		if (path[npoly - 1] != end_ref)
			m_nav_query->closestPointOnPoly(path[npoly - 1], &dest_location[0], &epos[0], 0);

		float straight_path[256 * 3];
		unsigned char straight_path_flags[256];
		int n_straight_polys;
		dtPolyRef straight_path_polys[256];
		m_nav_query->findStraightPath(&current_location[0], &epos[0], path, npoly,
			straight_path, straight_path_flags,
			straight_path_polys, &n_straight_polys, 256, DT_STRAIGHTPATH_ALL_CROSSINGS);

		if (n_straight_polys) {
			ret.m_nodes.reserve(n_straight_polys);
			for (int i = 0; i < n_straight_polys; ++i)
			{
				PathfindingNode node;
				node.position.x = straight_path[i * 3];
				node.position.z = straight_path[i * 3 + 1];
				node.position.y = straight_path[i * 3 + 2];
				if (!dtStatusSucceed(m_nav_mesh->getPolyFlags(straight_path_polys[i], &node.flag))) {
					node.flag = 0;
				}

				ret.m_nodes.push_back(node);
			}
		}
	}
	else {
		PathfindingNode dest;
		dest.flag = NavigationPolyFlagNormal;
		dest.position = dest_loc;
		ret.m_nodes.push_back(dest);
	}

	return ret;
}

bool PathfindingManager::GetRandomPoint(const glm::vec3 &start, float radius, glm::vec3 &pos)
{
	if(!m_nav_mesh)
		return false;

	if (!m_nav_query) {
		m_nav_query = dtAllocNavMeshQuery();
		m_nav_query->init(m_nav_mesh, 4092);
	}

	glm::vec3 ext(10.0f, 10.0f, 10.0f);
	dtPolyRef start_ref;
	m_nav_query->findNearestPoly(&start[0], &ext[0], &m_filter, &start_ref, 0);
	if (!start_ref) {
		return false;
	}

	dtPolyRef random_ref;
	glm::vec3 pt;

	dtStatus status = m_nav_query->findRandomPointAroundCircle(start_ref, &start[0], radius, 
		&m_filter, []() -> float { return (float)path_rng.Real(0.0, 1.0); }, &random_ref, &pt[0]);

	if (dtStatusSucceed(status))
	{
		pos.x = pt.x;
		pos.z = pt.y;
		pos.y = pt.z;
		return true;
	}

	return false;
}

PathfindingRoute::PathfindingRoute()
{
	m_active = false;
}

PathfindingRoute::~PathfindingRoute()
{
}

bool PathfindingRoute::DestinationValid(const glm::vec3 &dest)
{
	auto dist = vec_dist(dest, m_dest);
	if (dist <= max_dest_drift) {
		return true;
	}

	return false;
}

void PathfindingRoute::CalcCurrentNode(const glm::vec3 &current_pos, bool &wp_changed)
{
	wp_changed = false;
	if (m_active) {
		//if we're at last node then we dont need to do anything.
		if (m_nodes.size() - 1 == m_current_node) {
			return;
		}

		auto &current = GetCurrentNode();
		auto dist = vec_dist(current.position, current_pos);
		if (dist < at_waypoint_eps) {
			m_current_node++;
			wp_changed = true;
		}
	}
}

unsigned short PathfindingRoute::GetPreviousNodeFlag()
{
	if(m_current_node == 0)
		return 0;

	return m_nodes[m_current_node - 1].flag;
}

