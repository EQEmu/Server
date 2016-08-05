#include "water_map_v1.h"

WaterMapV1::WaterMapV1() {
	BSP_Root = nullptr;
}

WaterMapV1::~WaterMapV1() {
	if (BSP_Root) {
		delete[] BSP_Root;
	}
}

WaterRegionType WaterMapV1::ReturnRegionType(const glm::vec3& location) const {
	return BSPReturnRegionType(1, glm::vec3(location.y, location.x, location.z));
}

bool WaterMapV1::InWater(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeWater;
}

bool WaterMapV1::InVWater(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeVWater;
}

bool WaterMapV1::InLava(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeLava;
}

bool WaterMapV1::InLiquid(const glm::vec3& location) const {
	return InWater(location) || InLava(location);
}

bool WaterMapV1::InPvP(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypePVP;
}

bool WaterMapV1::InZoneLine(const glm::vec3& location) const {
	return ReturnRegionType(location) == RegionTypeZoneLine;
}

bool WaterMapV1::Load(FILE *fp) {
	uint32 bsp_tree_size;
	if (fread(&bsp_tree_size, sizeof(bsp_tree_size), 1, fp) != 1) {
		return false;
	}

	BSP_Root = new ZBSP_Node[bsp_tree_size];
	if (!BSP_Root) {
		return false;
	}

	if (fread(BSP_Root, sizeof(ZBSP_Node), bsp_tree_size, fp) != bsp_tree_size) {
		return false;
	}

	return true;
}

WaterRegionType WaterMapV1::BSPReturnRegionType(int32 node_number, const glm::vec3& location) const {
	float distance;

	const ZBSP_Node *current_node = &BSP_Root[node_number - 1];

	if ((current_node->left == 0) &&
		(current_node->right == 0)) {
		return (WaterRegionType)current_node->special;
	}

	distance = (location.x * current_node->normal[0]) +
		(location.y * current_node->normal[1]) +
		(location.z * current_node->normal[2]) +
		current_node->splitdistance;

	if (distance == 0.0f) {
		return(RegionTypeNormal);
	}

	if (distance >0.0f) {
		if (current_node->left == 0) {
			return(RegionTypeNormal);
		}
		return BSPReturnRegionType(current_node->left, location);
	}

	if (current_node->right == 0) {
		return(RegionTypeNormal);
	}

	return BSPReturnRegionType(current_node->right, location);
}
