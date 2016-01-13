#include "pathfind.h"
#include <stdio.h>
#include <DetourNavMeshQuery.h>

const uint32_t nav_mesh_file_version = 1;
const float max_dest_drift = 10.0f;

PathfindingManager::PathfindingManager()
{
	m_nav_mesh = nullptr;
}

PathfindingManager::~PathfindingManager()
{
	Clear();
}

void PathfindingManager::Load(const std::string &zone_name)
{
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

		m_nav_mesh = dtAllocNavMesh();

		uint32_t number_of_tiles = 0;
		if (fread(&number_of_tiles, sizeof(uint32_t), 1, f) != 1) {
			Clear();
			fclose(f);
			return;
		}

		dtNavMeshParams params;
		if (fread(&params, sizeof(dtNavMeshParams), 1, f) != 1) {
			Clear();
			fclose(f);
			return;
		}

		dtStatus status = m_nav_mesh->init(&params);
		if (dtStatusFailed(status))
		{
			Clear();
			fclose(f);
			return;
		}

		for (unsigned int i = 0; i < number_of_tiles; ++i)
		{
			uint32_t tile_ref = 0;
			if (fread(&tile_ref, sizeof(uint32_t), 1, f) != 1) {
				Clear();
				fclose(f);
				return;
			}

			int32_t data_size = 0;
			if (fread(&data_size, sizeof(int32_t), 1, f) != 1) {
				Clear();
				fclose(f);
				return;
			}

			if (!tile_ref || !data_size) {
				Clear();
				fclose(f);
				return;
			}

			unsigned char* data = (unsigned char*)dtAlloc(data_size, DT_ALLOC_PERM);
			if (fread(data, data_size, 1, f) != 1) {
				Clear();
				fclose(f);
				return;
			}

			m_nav_mesh->addTile(data, data_size, DT_TILE_FREE_DATA, tile_ref, 0);
		}

		fclose(f);
	}
}

void PathfindingManager::Clear()
{
	if (m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
		m_nav_mesh = nullptr;
	}
}

PathfindingRoute PathfindingManager::FindRoute(const glm::vec3 &src_loc, const glm::vec3 &dest_loc)
{
	glm::vec3 current_location(src_loc.x, src_loc.z, src_loc.y);
	glm::vec3 dest_location(dest_loc.x, dest_loc.z, dest_loc.y);

	PathfindingRoute ret;
	ret.m_dest = dest_location;
	ret.m_current_node = 0;

	if (!m_nav_mesh) {
		PathfindingNode src;
		src.flag = NavigationPolyFlagNormal;
		src.position = current_location;

		PathfindingNode dest;
		dest.flag = NavigationPolyFlagNormal;
		dest.position = dest_location;

		ret.m_nodes.push_back(src);
		ret.m_nodes.push_back(dest);
		return ret;
	}

	glm::vec3 ext(5.0f, 5.0f, 5.0f);
	dtQueryFilter filter;
	filter.setIncludeFlags(NavigationPolyFlagAll);
	filter.setAreaCost(NavigationAreaFlagNormal, 1.0f);
	filter.setAreaCost(NavigationAreaFlagWater, 2.5f);
	filter.setAreaCost(NavigationAreaFlagLava, 2.5f);
	filter.setAreaCost(NavigationAreaFlagPvP, 1.0f);
	filter.setAreaCost(NavigationAreaFlagSlime, 1.0f);
	filter.setAreaCost(NavigationAreaFlagIce, 1.0f);
	filter.setAreaCost(NavigationAreaFlagVWater, 2.5f);
	filter.setAreaCost(NavigationAreaFlagGeneralArea, 1.0f);
	filter.setAreaCost(NavigationAreaFlagPortal, 1.0f);

	dtNavMeshQuery *query = dtAllocNavMeshQuery();
	query->init(m_nav_mesh, 4092);
	dtPolyRef start_ref;
	dtPolyRef end_ref;

	query->findNearestPoly(&current_location[0], &ext[0], &filter, &start_ref, 0);
	query->findNearestPoly(&dest_location[0], &ext[0], &filter, &end_ref, 0);

	if (!start_ref || !end_ref) {
		dtFreeNavMeshQuery(query);

		PathfindingNode src;
		src.flag = NavigationPolyFlagNormal;
		src.position = current_location;

		PathfindingNode dest;
		dest.flag = NavigationPolyFlagNormal;
		dest.position = dest_location;

		ret.m_nodes.push_back(src);
		ret.m_nodes.push_back(dest);
		return ret;
	}

	int npoly = 0;
	dtPolyRef path[256] = { 0 };
	query->findPath(start_ref, end_ref, &current_location[0], &dest_location[0], &filter, path, &npoly, 256);

	if (npoly) {
		glm::vec3 epos = dest_location;
		if (path[npoly - 1] != end_ref)
			query->closestPointOnPoly(path[npoly - 1], &dest_location[0], &epos[0], 0);

		float straight_path[256 * 3];
		unsigned char straight_path_flags[256];
		int n_straight_polys;
		dtPolyRef straight_path_polys[256];
		query->findStraightPath(&current_location[0], &epos[0], path, npoly,
			straight_path, straight_path_flags,
			straight_path_polys, &n_straight_polys, 256, DT_STRAIGHTPATH_ALL_CROSSINGS);

		dtFreeNavMeshQuery(query);

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
		PathfindingNode src;
		src.flag = NavigationPolyFlagNormal;
		src.position = current_location;

		PathfindingNode dest;
		dest.flag = NavigationPolyFlagNormal;
		dest.position = dest_location;

		ret.m_nodes.push_back(src);
		ret.m_nodes.push_back(dest);
	}

	return ret;
}

PathfindingRoute::PathfindingRoute()
{
}

PathfindingRoute::~PathfindingRoute()
{
}

bool PathfindingRoute::Valid(const glm::vec3 &dest)
{
	auto dist = (dest - m_dest).length();
	if (dist <= max_dest_drift) {
		return true;
	}

	return false;
}

void PathfindingRoute::CalcCurrentNode()
{
	//if we're at last node then we dont need to do anything.

	//else need to see if we're at current_node
		//if so then we advance to the next node and return it
		//else just return the current node
}
