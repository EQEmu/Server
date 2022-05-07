#include "../client.h"
#include "../worldserver.h"

extern WorldServer worldserver;

void command_reloadworld(Client *c, const Seperator *sep)
{
	uint8 global_repop = ReloadWorld::NoRepop;

	if (sep->IsNumber(1)) {
		global_repop = static_cast<uint8>(std::stoul(sep->arg[1]));

		if (global_repop > ReloadWorld::ForceRepop) {
			global_repop = ReloadWorld::ForceRepop;
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Attempting to reload quests {}worldwide.",
			(
				global_repop ?
				(
					global_repop == ReloadWorld::Repop ?
					"and repop NPCs " :
					"and forcefully repop NPCs "
				) :
				""
			)
		).c_str()
	);

	auto pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
	ReloadWorld_Struct *RW  = (ReloadWorld_Struct *) pack->pBuffer;
	RW->global_repop = global_repop;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

