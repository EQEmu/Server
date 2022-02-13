#include "../client.h"

void command_showspellslist(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();
	if (!target || !target->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}
	target->CastToNPC()->AISpellsList(c);
	return;
}

