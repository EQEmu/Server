#include "../client.h"

void command_shownpcgloballoot(Client *c, const Seperator *sep)
{
	auto tar = c->GetTarget();

	if (!tar || !tar->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto npc = tar->CastToNPC();
	c->Message(Chat::White, "GlobalLoot for %s (%d)", npc->GetName(), npc->GetNPCTypeID());
	zone->ShowNPCGlobalLoot(c, npc);
}

