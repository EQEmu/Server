#include "../client.h"

void command_attack(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->arg[1] != 0) {
		Mob *sictar = entity_list.GetMob(sep->argplus[1]);
		if (sictar) {
			c->GetTarget()->CastToNPC()->AddToHateList(sictar, 1, 0);
		}
		else {
			c->Message(Chat::White, "Error: %s not found", sep->arg[1]);
		}
	}
	else {
		c->Message(Chat::White, "Usage: (needs NPC targeted) #attack targetname");
	}
}

