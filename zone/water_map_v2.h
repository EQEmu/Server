#ifndef EQEMU_WATER_MAP_V2_H
#define EQEMU_WATER_MAP_V2_H

#include "water_map.h"
#include "oriented_bounding_box.h"
#include <vector>
#include <utility>

class WaterMapV2 : public WaterMap
{
public:
	WaterMapV2();
	~WaterMapV2();

	virtual WaterRegionType ReturnRegionType(float y, float x, float z) const;
	virtual bool InWater(float y, float x, float z) const;
	virtual bool InVWater(float y, float x, float z) const;
	virtual bool InLava(float y, float x, float z) const;
	virtual bool InLiquid(float y, float x, float z) const;
	
protected:
	virtual bool Load(FILE *fp);

	std::vector<std::pair<WaterRegionType, OrientedBoundingBox>> regions;
	friend class WaterMap;
};

#endif
