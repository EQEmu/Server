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
#ifndef WATERMAP_H
#define WATERMAP_H

#define WATERMAP_VERSION 1

#pragma pack(1)
typedef struct ZBSP_Node {
	int32 node_number;
	float normal[3], splitdistance;
	int32 region;
	int32 special;
	int32 left, right;
} ZBSP_Node;
#pragma pack()

typedef enum {
	RegionTypeUnsupported = -2,
	RegionTypeUntagged = -1,
	RegionTypeNormal = 0,
	RegionTypeWater = 1,
	RegionTypeLava = 2,
	RegionTypeZoneLine = 3,
	RegionTypePVP = 4,
	RegionTypeSlime = 5,
	RegionTypeIce = 6,
	RegionTypeVWater =7
} WaterRegionType;

class WaterMap {

public:
	static WaterMap* LoadWaterMapfile(const char* in_zonename, const char *directory = nullptr);
	WaterRegionType BSPReturnRegionType(int32 node_number, float y, float x, float z) const;
	bool InWater(float y, float x, float z) const;
	bool InVWater(float y, float x, float z) const;
	bool InLava(float y, float x, float z) const;
	bool InLiquid(float y, float x, float z) const;

	WaterMap();
	~WaterMap();

private:
	bool loadWaterMap(FILE *fp);

	ZBSP_Node* BSP_Root;
};

#endif



