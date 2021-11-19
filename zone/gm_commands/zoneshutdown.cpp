#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_zoneshutdown(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected()) {
		c->Message(Chat::White, "Error: World server disconnected");
	}
	else if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #zoneshutdown zoneshortname");
	}
	else {
		auto                         pack = new ServerPacket(
			ServerOP_ZoneShutdown,
			sizeof(ServerZoneStateChange_struct));
		ServerZoneStateChange_struct *s   = (ServerZoneStateChange_struct *) pack->pBuffer;
		strcpy(s->adminname, c->GetName());
		if (sep->arg[1][0] >= '0' && sep->arg[1][0] <= '9') {
			s->ZoneServerID = atoi(sep->arg[1]);
		}
		else {
			s->zoneid = ZoneID(sep->arg[1]);
		}
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

