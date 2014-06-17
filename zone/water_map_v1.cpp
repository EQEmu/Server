#include "water_map_v1.h"

WaterMapV1::WaterMapV1() {
	BSP_Root = nullptr;
}

WaterMapV1::~WaterMapV1() {
	if (BSP_Root) {
		delete[] BSP_Root;
	}
}

WaterRegionType WaterMapV1::ReturnRegionType(float y, float x, float z) const {
	return BSPReturnRegionType(1, y, x, z);
}

bool WaterMapV1::InWater(float y, float x, float z) const {
	return ReturnRegionType(y, x, z) == RegionTypeWater;
}

bool WaterMapV1::InVWater(float y, float x, float z) const {
	return ReturnRegionType(y, x, z) == RegionTypeVWater;
}

bool WaterMapV1::InLava(float y, float x, float z) const {
	return ReturnRegionType(y, x, z) == RegionTypeLava;
}

bool WaterMapV1::InLiquid(float y, float x, float z) const {
	return InWater(y, x, z) || InLava(y, x, z);
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

WaterRegionType WaterMapV1::BSPReturnRegionType(int32 node_number, float y, float x, float z) const {
	float distance;

	const ZBSP_Node *current_node = &BSP_Root[node_number - 1];

	if ((current_node->left == 0) &&
		(current_node->right == 0)) {
		return (WaterRegionType)current_node->special;
	}

	distance = (x * current_node->normal[0]) +
		(y * current_node->normal[1]) +
		(z * current_node->normal[2]) +
		current_node->splitdistance;

	if (distance == 0.0f) {
		return(RegionTypeNormal);
	}

	if (distance >0.0f) {
		if (current_node->left == 0) {
			return(RegionTypeNormal);
		}
		return BSPReturnRegionType(current_node->left, y, x, z);
	}

	if (current_node->right == 0) {
		return(RegionTypeNormal);
	}

	return BSPReturnRegionType(current_node->right, y, x, z);
}
