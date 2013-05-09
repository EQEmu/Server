/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/debug.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "watermap.h"
#include "../common/MiscFunctions.h"
#ifdef _WINDOWS
#define snprintf _snprintf
#endif




WaterMap::WaterMap()
: BSP_Root(nullptr)
{
}

WaterMap::~WaterMap() {
	safe_delete_array(BSP_Root);
}


WaterRegionType WaterMap::BSPReturnRegionType(int32 node_number, float y, float x, float z) const
{
	float distance;

	const ZBSP_Node *current_node = &BSP_Root[node_number-1];

	// Are we at a leaf

	if ((current_node->left==0) &&
		(current_node->right==0)) {
		return (WaterRegionType) current_node->special;
	}


	// No, so determine which side of the split plane we are on
	//

	distance = (x * current_node->normal[0]) +
					(y * current_node->normal[1]) +
					(z * current_node->normal[2]) +
					current_node->splitdistance;

	// If we are exactly on the split plane, I don't know what should happen.
	//
	if(distance == 0.0f) {
		return(RegionTypeNormal);
	}
	if(distance >0.0f) {
		if(current_node->left==0) {
				// This shouldn't happen
				return(RegionTypeNormal);
		}
		return BSPReturnRegionType(current_node->left, y, x, z);
	}
	if(current_node->right==0) {
		// This should't happen
		return(RegionTypeNormal);
	}

	return BSPReturnRegionType(current_node->right, y, x, z);
}

bool WaterMap::InWater(float y, float x, float z) const {
	if(BSP_Root == nullptr) {
		return false;
	}
	return(BSPReturnRegionType(1, y, x, z) == RegionTypeWater);
}

bool WaterMap::InVWater(float y, float x, float z) const {
	if(BSP_Root == nullptr) {
		return false;
	}
	return(BSPReturnRegionType(1, y, x, z) == RegionTypeVWater);
}

bool WaterMap::InLava(float y, float x, float z) const {
	if(BSP_Root == nullptr) {
		return false;
	}
	return(BSPReturnRegionType(1, y, x, z) == RegionTypeLava);
}

bool WaterMap::InLiquid(float y, float x, float z) const {
	if(BSP_Root == nullptr)	//if the water map isn't loaded, this will save ~1 CPU cycle
		return false;
	return (InWater(y, x, z) || InLava(y, x, z));
}

WaterMap* WaterMap::LoadWaterMapfile(const char* in_zonename, const char *directory) {
	FILE *fp;
	char zBuf[64];
	char cWork[256];
	WaterMap* ret = nullptr;


	//have to convert to lower because the short names im getting
	//are not all lower anymore, copy since strlwr edits the str.
	strn0cpy(zBuf, in_zonename, 64);

	if(directory == nullptr)
		directory = MAP_DIR;
	snprintf(cWork, 250, "%s/%s.wtr", directory, strlwr(zBuf));

	if ((fp = fopen( cWork, "rb" ))) {
		ret = new WaterMap();
		if(ret != nullptr) {
			if(ret->loadWaterMap(fp)) {
				printf("Water Map %s loaded.\n", cWork);
			} else {
				safe_delete(ret);
				printf("Water Map %s exists but could not be processed.\n", cWork);
				return nullptr;
			}
		} else {
			printf("Water Map %s loading failed.\n", cWork);
		}
		fclose(fp);
	}
	else {
		printf("Water Map %s not found.\n", cWork);
	}
	return ret;
}

bool WaterMap::loadWaterMap(FILE *fp) {
	char EQWMagic[10];
	uint32 BSPTreeSize;
	uint32 EQWVersion;

	if(fread(EQWMagic, 10, 1, fp)!=1) {
		printf("Error reading Water region map.\n");
		return(false);
	}
	if(strncmp(EQWMagic,"EQEMUWATER",10)) {
		printf("Bad header in Water region map.\n");
		return(false);
	}
	if(fread(&EQWVersion, sizeof(EQWVersion), 1, fp)!=1) {
		printf("Error reading Water region map.\n");
		return(false);
	}
	if(EQWVersion!=WATERMAP_VERSION) {
		printf("Incompatible Water region map version.\n");
		return(false);
	}

	if(fread(&BSPTreeSize, sizeof(BSPTreeSize), 1, fp)!=1) {
		printf("Error reading Water region map.\n");
		return(false);
	}

	BSP_Root = (ZBSP_Node *) new ZBSP_Node[BSPTreeSize];

	if(BSP_Root==nullptr) {
		printf("Memory allocation failed while reading water map.\n");
		return(false);
	}

	if(fread(BSP_Root, sizeof(ZBSP_Node), BSPTreeSize, fp) != BSPTreeSize) {
		printf("Error reading Water region map.\n");
		safe_delete_array(BSP_Root);
		return(false);
	}

	printf("Water region map has %d nodes.\n", BSPTreeSize);
	return(true);
}


