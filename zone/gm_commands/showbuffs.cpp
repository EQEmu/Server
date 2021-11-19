#include "../client.h"

void command_showbuffs(Client *c, const Seperator *sep)
{
	if (c->GetTarget() == 0) {
		c->CastToMob()->ShowBuffs(c);
	}
	else {
		c->GetTarget()->CastToMob()->ShowBuffs(c);
	}
}

