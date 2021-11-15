#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadworldrules(Client *c, const Seperator *sep)
{
	if (c) {
		auto pack = new ServerPacket(ServerOP_ReloadRulesWorld, 0);
		worldserver.SendPacket(pack);
		c->Message(Chat::Red, "Successfully sent the packet to world to reload rules. (only world)");
		safe_delete(pack);
	}
}

