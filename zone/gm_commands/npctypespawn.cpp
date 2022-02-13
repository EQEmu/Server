#include "../client.h"

void command_npctypespawn(Client *c, const Seperator *sep)
{
	if (sep->IsNumber(1)) {
		const NPCType *tmp = 0;
		if ((tmp = content_db.LoadNPCTypesData(atoi(sep->arg[1])))) {
			//tmp->fixedZ = 1;
			auto npc = new NPC(tmp, 0, c->GetPosition(), GravityBehavior::Water);
			if (npc && sep->IsNumber(2)) {
				npc->SetNPCFactionID(atoi(sep->arg[2]));
			}

			npc->AddLootTable();
			if (npc->DropsGlobalLoot()) {
				npc->CheckGlobalLootTables();
			}
			entity_list.AddNPC(npc);
		}
		else {
			c->Message(Chat::White, "NPC Type %i not found", atoi(sep->arg[1]));
		}
	}
	else {
		c->Message(Chat::White, "Usage: #npctypespawn npctypeid factionid");
	}

}

