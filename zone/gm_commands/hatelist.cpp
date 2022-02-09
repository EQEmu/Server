#include "../client.h"

void command_hatelist(Client *c, const Seperator *sep)
{
	Mob *target = c->GetTarget();
	if (target == nullptr) {
		c->Message(Chat::White, "Error: you must have a target.");
		return;
	}

	c->Message(Chat::White, "Display hate list for %s..", target->GetName());
	target->PrintHateListToClient(c);
}


