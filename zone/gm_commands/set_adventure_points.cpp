#include "../client.h"

void command_set_adventure_points(Client *c, const Seperator *sep)
{
	Client *t = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (!sep->arg[1][0]) {
		c->Message(Chat::White, "Usage: #setadventurepoints [theme] [points]");
		return;
	}

	if (!sep->IsNumber(1) || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #setadventurepoints [theme] [points]");
		return;
	}

	c->Message(Chat::White, "Updating adventure points for %s", t->GetName());
	t->UpdateLDoNPoints(atoi(sep->arg[1]), atoi(sep->arg[2]));
}

