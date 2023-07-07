#include "../../client.h"
#include "../../worldserver.h"

extern WorldServer worldserver;

void SetZoneLocked(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set zone_locked [on|off] [Zone ID|Zone Short Name]");
		return;
	}

	const bool is_locked = Strings::ToBool(sep->arg[2]);

	const uint32 zone_id = (
		sep->IsNumber(3) ?
		Strings::ToUnsignedInt(sep->arg[3]) :
		ZoneID(sep->arg[3])
	);
	const std::string& zone_short_name = Strings::ToLower(ZoneName(zone_id, true));

	const bool is_unknown_zone = Strings::EqualFold(zone_short_name, "unknown");
	if (!zone_id || is_unknown_zone) {
		c->Message(Chat::White, "Usage: #set zone_locked [on|off] [Zone ID|Zone Short Name]");
		return;
	}

	auto pack = new ServerPacket(ServerOP_LockZone, sizeof(ServerLockZone_Struct));

	auto l = (ServerLockZone_Struct *) pack->pBuffer;
	strn0cpy(l->adminname, c->GetName(), sizeof(l->adminname));
	l->op     = is_locked ? ServerLockType::Lock : ServerLockType::Unlock;
	l->zoneID = zone_id;

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

