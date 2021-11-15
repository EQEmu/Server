#include "../client.h"

void command_lastname(Client *c, const Seperator *sep)
{
	Client *t = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}
	LogInfo("#lastname request from [{}] for [{}]", c->GetName(), t->GetName());

	if (strlen(sep->arg[1]) <= 70) {
		t->ChangeLastName(sep->arg[1]);
	}
	else {
		c->Message(Chat::White, "Usage: #lastname <lastname> where <lastname> is less than 70 chars long");
	}
}

