#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_uptime(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected()) {
		c->Message(Chat::White, "Error: World server disconnected");
	}
	else {
		auto                pack = new ServerPacket(ServerOP_Uptime, sizeof(ServerUptime_Struct));
		ServerUptime_Struct *sus = (ServerUptime_Struct *) pack->pBuffer;
		strcpy(sus->adminname, c->GetName());
		if (sep->IsNumber(1) && atoi(sep->arg[1]) > 0) {
			sus->zoneserverid = atoi(sep->arg[1]);
		}
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

