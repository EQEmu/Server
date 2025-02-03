#include "zonelist.h"
#include "zoneserver.h"
#include "../common/strings.h"

void ZSList::Add(ZoneServer* zoneserver) {
	zone_server_list.emplace_back(std::unique_ptr<ZoneServer>(zoneserver));
	zoneserver->SetIsZoneConnected(true);
}

void ZSList::Remove(const std::string &uuid)
{
	auto iter = zone_server_list.begin();
	while (iter != zone_server_list.end()) {
		if ((*iter)->GetUUID().compare(uuid) == 0) {
			zone_server_list.erase(iter);
			return;
		}
		iter++;
	}
}
