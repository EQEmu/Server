#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadtitles(Client *c, const Seperator *sep)
{
	auto pack = new ServerPacket(ServerOP_ReloadTitles, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
	c->Message(Chat::Yellow, "Player Titles Reloaded.");

}

