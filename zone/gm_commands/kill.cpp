#include "../client.h"

void command_kill(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "Error: #Kill: No target.");
	}
	else if (!c->GetTarget()->IsClient() || c->GetTarget()->CastToClient()->Admin() <= c->Admin()) {
		c->GetTarget()->Kill();
	}
}

