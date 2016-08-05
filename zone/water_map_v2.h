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

	virtual WaterRegionType ReturnRegionType(const glm::vec3& location) const;
	virtual bool InWater(const glm::vec3& location) const;
	virtual bool InVWater(const glm::vec3& location) const;
	virtual bool InLava(const glm::vec3& location) const;
	virtual bool InLiquid(const glm::vec3& location) const;
	virtual bool InPvP(const glm::vec3& location) const;
	virtual bool InZoneLine(const glm::vec3& location) const;

protected:
	virtual bool Load(FILE *fp);

	std::vector<std::pair<WaterRegionType, OrientedBoundingBox>> regions;
	friend class WaterMap;
};

#endif
