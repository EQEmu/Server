#ifndef EQEMU_WATER_MAP_H
#define EQEMU_WATER_MAP_H

#include "../common/types.h"
#include <string>

enum WaterRegionType {
	RegionTypeUnsupported = -2,
	RegionTypeUntagged = -1,
	RegionTypeNormal = 0,
	RegionTypeWater = 1,
	RegionTypeLava = 2,
	RegionTypeZoneLine = 3,
	RegionTypePVP = 4,
	RegionTypeSlime = 5,
	RegionTypeIce = 6,
	RegionTypeVWater = 7
};

class WaterMap
{
public:
	WaterMap() { }
	~WaterMap() { }
	
	static WaterMap* LoadWaterMapfile(std::string zone_name);
	virtual WaterRegionType ReturnRegionType(float y, float x, float z) const { return RegionTypeNormal; }
	virtual bool InWater(float y, float x, float z) const { return false; }
	virtual bool InVWater(float y, float x, float z) const { return false; }
	virtual bool InLava(float y, float x, float z) const { return false; }
	virtual bool InLiquid(float y, float x, float z) const { return false; }
	
protected:
	virtual bool Load(FILE *fp) { return false; }
};

#endif
