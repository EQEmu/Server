#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_zonelock(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #zonelock list - Lists Locked Zones");
		if (c->Admin() >= commandLockZones) {
			c->Message(
				Chat::White,
				"Usage: #zonelock lock [Zone ID] or #zonelock lock [Zone Short Name] - Locks a Zone by ID or Short Name"
			);
			c->Message(
				Chat::White,
				"Usage: #zonelock unlock [Zone ID] or #zonelock unlock [Zone Short Name] - Unlocks a Zone by ID or Short Name"
			);
		}
		return;
	}

	std::string lock_type = str_tolower(sep->arg[1]);
	bool        is_list   = lock_type.find("list") != std::string::npos;
	bool        is_lock   = lock_type.find("lock") != std::string::npos;
	bool        is_unlock = lock_type.find("unlock") != std::string::npos;
	if (!is_list && !is_lock && !is_unlock) {
		c->Message(Chat::White, "Usage: #zonelock list - Lists Locked Zones");
		if (c->Admin() >= commandLockZones) {
			c->Message(
				Chat::White,
				"Usage: #zonelock lock [Zone ID] or #zonelock lock [Zone Short Name] - Locks a Zone by ID or Short Name"
			);
			c->Message(
				Chat::White,
				"Usage: #zonelock unlock [Zone ID] or #zonelock unlock [Zone Short Name] - Unlocks a Zone by ID or Short Name"
			);
		}
		return;
	}

	auto                  pack       = new ServerPacket(ServerOP_LockZone, sizeof(ServerLockZone_Struct));
	ServerLockZone_Struct *lock_zone = (ServerLockZone_Struct *) pack->pBuffer;
	strn0cpy(lock_zone->adminname, c->GetName(), sizeof(lock_zone->adminname));

	if (is_list) {
		lock_zone->op = ServerLockType::List;
		worldserver.SendPacket(pack);
	}
	else if (!is_list && c->Admin() >= commandLockZones) {
		auto        zone_id         = (
			sep->IsNumber(2) ?
				static_cast<uint16>(std::stoul(sep->arg[2])) :
				static_cast<uint16>(ZoneID(sep->arg[2]))
		);
		std::string zone_short_name = str_tolower(ZoneName(zone_id, true));
		bool        is_unknown_zone = zone_short_name.find("unknown") != std::string::npos;
		if (zone_id && !is_unknown_zone) {
			lock_zone->op     = is_lock ? ServerLockType::Lock : ServerLockType::Unlock;
			lock_zone->zoneID = zone_id;
			worldserver.SendPacket(pack);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Usage: #zonelock {} [Zone ID] or #zonelock {} [Zone Short Name]",
					is_lock ? "lock" : "unlock"
				).c_str()
			);
		}
	}
	safe_delete(pack);
}

