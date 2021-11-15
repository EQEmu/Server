#include "../client.h"

void command_bind(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0) {
		if (c->GetTarget()->IsClient()) {
			c->GetTarget()->CastToClient()->SetBindPoint();
		}
		else {
			c->Message(Chat::White, "Error: target not a Player");
		}
	}
	else {
		c->SetBindPoint();
	}
}

