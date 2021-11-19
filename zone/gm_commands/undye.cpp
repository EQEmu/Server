#include "../client.h"

void command_undye(Client *c, const Seperator *sep)
{
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		c->GetTarget()->CastToClient()->Undye();
	}
	else {
		c->Message(Chat::White, "ERROR: Client target required");
	}
}

