#include "../../client.h"

void ShowSpellsList(Client *c, const Seperator *sep)
{
	auto t = c->GetTarget();
	if (!t || !t->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	t->CastToNPC()->AISpellsList(c);
}
