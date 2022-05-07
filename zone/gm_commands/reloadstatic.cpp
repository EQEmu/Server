#include "../client.h"

void command_reloadstatic(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempting to reload static zone data globally.");
	auto pack = new ServerPacket(ServerOP_ReloadStaticZoneData, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

