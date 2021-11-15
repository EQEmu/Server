#include "../client.h"

void command_permaclass(Client *c, const Seperator *sep)
{
	Client *t = c;

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		t = c->GetTarget()->CastToClient();
	}

	if (sep->arg[1][0] == 0) {
		c->Message(Chat::White, "Usage: #permaclass <classnum>");
	}
	else if (!t->IsClient()) {
		c->Message(Chat::White, "Target is not a client.");
	}
	else {
		c->Message(Chat::White, "Setting %s's class...Sending to char select.", t->GetName());
		LogInfo("Class change request from [{}] for [{}], requested class:[{}]",
				c->GetName(),
				t->GetName(),
				atoi(sep->arg[1]));
		t->SetBaseClass(atoi(sep->arg[1]));
		t->Save();
		t->Kick("Class was changed.");
	}
}

