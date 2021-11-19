#include "../client.h"

void command_gmspeed(Client *c, const Seperator *sep)
{
	bool   state = atobool(sep->arg[1]);
	Client *t    = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (sep->arg[1][0] != 0) {
		database.SetGMSpeed(t->AccountID(), state ? 1 : 0);
		c->Message(Chat::White, "Turning GMSpeed %s for %s (zone to take effect)", state ? "On" : "Off", t->GetName());
	}
	else {
		c->Message(Chat::White, "Usage: #gmspeed [on/off]");
	}
}

