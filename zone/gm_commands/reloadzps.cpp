#include "../client.h"

void command_reloadzps(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempting to reloading server zone points globally.");
	auto pack = new ServerPacket(ServerOP_ReloadZonePoints, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

