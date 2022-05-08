#include "../client.h"
#include "../../common/file_util.h"

void command_reloadaa(Client *c, const Seperator *sep)
{
	c->Message(Chat::White, "Attempted to reload Alternate Advancement data globally.");	
	auto pack = new ServerPacket(ServerOP_ReloadAAData, 0);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

