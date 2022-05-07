#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadtitles(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempting to reload titles globally.");
	auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

