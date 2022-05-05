#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadallrules(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempting to reload all rules globally.");
	auto pack = new ServerPacket(ServerOP_ReloadRules, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

