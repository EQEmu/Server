#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadperlexportsettings(Client *c, const Seperator *sep)
{
	if (c) {
		auto pack = new ServerPacket(ServerOP_ReloadPerlExportSettings, 0);
		worldserver.SendPacket(pack);
		c->Message(Chat::Red, "Successfully sent the packet to world to reload Perl Export settings");
		safe_delete(pack);

	}
}

