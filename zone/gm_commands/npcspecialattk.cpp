#include "../client.h"

void command_npcspecialattk(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0 || c->GetTarget()->IsClient() || strlen(sep->arg[1]) <= 0 || strlen(sep->arg[2]) <= 0) {
		c->Message(
			Chat::White,
			"Usage: #npcspecialattk *flagchar* *permtag* (Flags are E(nrage) F(lurry) R(ampage) S(ummon), permtag is 1 = True, 0 = False)."
		);
	}
	else {
		c->GetTarget()->CastToNPC()->NPCSpecialAttacks(sep->arg[1], atoi(sep->arg[2]));
		c->Message(Chat::White, "NPC Special Attack set.");
	}
}

