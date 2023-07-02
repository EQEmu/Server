#include "../client.h"

void command_showstats(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	t->ShowStats(c);
}

