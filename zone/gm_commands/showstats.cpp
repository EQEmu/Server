#include "../client.h"

void command_showstats(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0) {
		c->GetTarget()->ShowStats(c);
	}
	else {
		c->ShowStats(c);
	}
}

