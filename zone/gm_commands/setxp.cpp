#include "../client.h"

void command_setxp(Client *c, const Seperator *sep)
{
	Client *t = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (sep->IsNumber(1)) {
		if (atoi(sep->arg[1]) > 9999999) {
			c->Message(Chat::White, "Error: Value too high.");
		}
		else {
			t->AddEXP(atoi(sep->arg[1]));
		}
	}
	else {
		c->Message(Chat::White, "Usage: #setxp number");
	}
}

