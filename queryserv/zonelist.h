#ifndef ZONELIST_H_
#define ZONELIST_H_

#include "../common/types.h"
#include <string>
#include <vector>
#include <memory>
#include <list>

class WorldTCPConnection;
class ZoneServer;

class ZSList {
public:
	std::list<std::unique_ptr<ZoneServer>> &GetZsList() { return zone_server_list; }
	void Add(ZoneServer *zoneserver);
	void Remove(const std::string &uuid);

private:
	std::list<std::unique_ptr<ZoneServer>> zone_server_list;
};

#endif /*ZONELIST_H_*/
