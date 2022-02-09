#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadworld(Client *c, const Seperator *sep)
{
	int world_repop = atoi(sep->arg[1]);
	if (world_repop == 0) {
		c->Message(Chat::White, "Reloading quest cache worldwide.");
	}
	else {
		c->Message(Chat::White, "Reloading quest cache and repopping zones worldwide.");
	}

	auto               pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
	ReloadWorld_Struct *RW  = (ReloadWorld_Struct *) pack->pBuffer;
	RW->Option = world_repop;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

