#include "../client.h"

void command_showbuffs(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	t->ShowBuffs(c);
}

