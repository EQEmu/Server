#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadcontentflags(Client *c, const Seperator *sep)
{
	if (c) {
		auto pack = new ServerPacket(ServerOP_ReloadContentFlags, 0);
		worldserver.SendPacket(pack);
		c->Message(Chat::Red, "Successfully sent the packet to world to reload content flags globally.");
		safe_delete(pack);
	}
}

