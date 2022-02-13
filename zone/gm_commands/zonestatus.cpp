#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_zonestatus(Client *c, const Seperator *sep)
{
	if (!worldserver.Connected()) {
		c->Message(Chat::White, "Error: World server disconnected");
	}
	else {
		auto pack = new ServerPacket(ServerOP_ZoneStatus, strlen(c->GetName()) + 2);
		memset(pack->pBuffer, (uint8) c->Admin(), 1);
		strcpy((char *) &pack->pBuffer[1], c->GetName());
		worldserver.SendPacket(pack);
		delete pack;
	}
}

