#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadworld(Client *c, const Seperator *sep)
{
	uint32 world_repop = 0;

	if (sep->IsNumber(1)) {
		world_repop = std::stoul(sep->arg[1]);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Attempting to reload quests {}worldwide.",
			world_repop ? "and repop NPCs " : ""
		).c_str()
	);

	auto pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
	ReloadWorld_Struct *RW  = (ReloadWorld_Struct *) pack->pBuffer;
	RW->Option = world_repop;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

