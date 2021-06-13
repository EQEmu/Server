#ifndef WORLD_DYNAMIC_ZONE_MANAGER_H
#define WORLD_DYNAMIC_ZONE_MANAGER_H

#include "../common/timer.h"
#include <memory>
#include <unordered_map>
#include <vector>

extern class DynamicZoneManager dynamic_zone_manager;

class DynamicZone;
struct DynamicZoneMember;
class ServerPacket;

class DynamicZoneManager
{
public:
	DynamicZoneManager();

	void CacheAllFromDatabase();
	void CacheNewDynamicZone(ServerPacket* pack);
	DynamicZone* CreateNew(DynamicZone& dz_request, const std::vector<DynamicZoneMember>& members);
	void Process();
	void PurgeExpiredDynamicZones();

	std::unordered_map<uint32_t, std::unique_ptr<DynamicZone>> dynamic_zone_cache;

private:
	Timer m_process_throttle_timer{};
};

#endif
