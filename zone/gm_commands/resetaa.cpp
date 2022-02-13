#include "../client.h"

void command_resetaa(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		c->GetTarget()->CastToClient()->ResetAA();
		c->Message(Chat::Red, "Successfully reset %s's AAs", c->GetTarget()->GetName());
	}
	else {
		c->Message(Chat::White, "Usage: Target a client and use #resetaa to reset the AA data in their Profile.");
	}
}

