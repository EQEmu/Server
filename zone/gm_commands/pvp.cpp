#include "../client.h"

void command_pvp(Client *c, const Seperator *sep)
{
	bool   state = atobool(sep->arg[1]);
	Client *t    = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (sep->arg[1][0] != 0) {
		t->SetPVP(state);
		c->Message(Chat::White, "%s now follows the ways of %s.", t->GetName(), state ? "discord" : "order");
	}
	else {
		c->Message(Chat::White, "Usage: #pvp [on/off]");
	}
}

