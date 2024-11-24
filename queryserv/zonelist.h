#ifndef ZONELIST_H_
#define ZONELIST_H_
//
#include "../common/types.h"
#include <string>
//../common/eqtime.h"
// #include "../common/timer.h"
// #include "../common/event/timer.h"
#include <vector>
#include <memory>
#include <list>

class WorldTCPConnection;
class ZoneServer;

class ZSList {
public:
	void Add(ZoneServer *zoneserver);
	void Remove(const std::string &uuid);

private:
	std::list<std::unique_ptr<ZoneServer>> zone_server_list;
};

#endif /*ZONELIST_H_*/