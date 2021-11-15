#include "../client.h"

void command_invul(Client *c, const Seperator *sep)
{
	bool   state = atobool(sep->arg[1]);
	Client *t    = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (sep->arg[1][0] != 0) {
		t->SetInvul(state);
		c->Message(Chat::White, "%s is %s invulnerable from attack.", t->GetName(), state ? "now" : "no longer");
	}
	else {
		c->Message(Chat::White, "Usage: #invulnerable [on/off]");
	}
}

