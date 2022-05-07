#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadperlexportsettings(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempting to reload Perl event export settings globally.");
	auto pack = new ServerPacket(ServerOP_ReloadPerlExportSettings, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

