#include "../client.h"

void command_reloadmerchants(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempting to reload merchants globally.");
	auto pack = new ServerPacket(ServerOP_ReloadMerchants, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

