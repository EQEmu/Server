#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_zonebootup(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected()) {
		c->Message(Chat::White, "Error: World server disconnected");
	}
	else if (sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #zonebootup ZoneServerID# zoneshortname");
	}
	else {
		auto                         pack = new ServerPacket(ServerOP_ZoneBootup, sizeof(ServerZoneStateChange_struct));
		ServerZoneStateChange_struct *s   = (ServerZoneStateChange_struct *) pack->pBuffer;
		s->ZoneServerID = atoi(sep->arg[1]);
		strcpy(s->adminname, c->GetName());
		s->zoneid     = ZoneID(sep->arg[2]);
		s->makestatic = (bool) (strcasecmp(sep->arg[3], "static") == 0);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

