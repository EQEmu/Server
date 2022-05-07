#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadworldrules(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempting to reload world only rules.");
	auto pack = new ServerPacket(ServerOP_ReloadRulesWorld, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

