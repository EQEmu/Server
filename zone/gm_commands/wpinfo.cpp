#include "../client.h"

void command_wpinfo(Client *c, const Seperator *sep)
{
	Mob *t = c->GetTarget();

	if (t == nullptr || !t->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this.");
		return;
	}

	NPC *n = t->CastToNPC();
	n->DisplayWaypointInfo(c);
}

