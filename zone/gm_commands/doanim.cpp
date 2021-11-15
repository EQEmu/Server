#include "../client.h"

void command_doanim(Client *c, const Seperator *sep)
{
	if (!sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #DoAnim [number]");
	}
	else if (c->Admin() >= commandDoAnimOthers) {
		if (c->GetTarget() == 0) {
			c->Message(Chat::White, "Error: You need a target.");
		}
		else {
			c->GetTarget()->DoAnim(atoi(sep->arg[1]), atoi(sep->arg[2]));
		}
	}
	else {
		c->DoAnim(atoi(sep->arg[1]), atoi(sep->arg[2]));
	}
}

