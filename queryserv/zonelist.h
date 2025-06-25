#ifndef ZONELIST_H_
#define ZONELIST_H_

#include "../common/types.h"
#include <string>
#include <vector>
#include <memory>
#include <list>
#include "zoneserver.h"

class WorldTCPConnection;

class ZSList {
public:
	std::list<std::unique_ptr<ZoneServer>>& GetZsList() { return zone_server_list; }
	void                                    Add(ZoneServer* zoneserver);
	void                                    Remove(const std::string& uuid);
	void                                    SendPlayerEventLogSettings();

	static ZSList* Instance()
	{
		static ZSList instance;
		return &instance;
	}

private:
	std::list<std::unique_ptr<ZoneServer>> zone_server_list;
};

#endif /*ZONELIST_H_*/
