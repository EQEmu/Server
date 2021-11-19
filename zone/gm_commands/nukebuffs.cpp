#include "../client.h"

void command_nukebuffs(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0) {
		c->BuffFadeAll();
	}
	else {
		c->GetTarget()->BuffFadeAll();
	}
}

