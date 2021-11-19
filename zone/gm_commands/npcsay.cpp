#include "../client.h"

void command_npcsay(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsNPC() && sep->arg[1][0]) {
		c->GetTarget()->Say(sep->argplus[1]);
	}
	else {
		c->Message(Chat::White, "Usage: #npcsay message (requires NPC target");
	}
}

