#include "../client.h"

void command_npcstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsNPC()) {
		NPC *target = c->GetTarget()->CastToNPC();

		// Stats
		target->ShowStats(c);

		// Loot Data
		target->QueryLoot(c);
	}
	else {
		c->Message(Chat::White, "You must target an NPC to use this command.");
	}
}

