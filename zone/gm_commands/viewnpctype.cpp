#include "../client.h"

void command_viewnpctype(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(1)) {
		uint32        npc_id         = std::stoul(sep->arg[1]);
		const NPCType *npc_type_data = content_db.LoadNPCTypesData(npc_id);
		if (npc_type_data) {
			auto npc = new NPC(
				npc_type_data,
				nullptr,
				c->GetPosition(),
				GravityBehavior::Water
			);
			npc->ShowStats(c);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"NPC ID {} was not found.",
					npc_id
				).c_str()
			);
		}
	}
	else {
		c->Message(Chat::White, "Usage: #viewnpctype [NPC ID]");
	}
}

