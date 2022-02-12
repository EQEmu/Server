#include "../client.h"

void command_hatelist(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget();

	target->PrintHateListToClient(c);
}
