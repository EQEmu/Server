#include "../client.h"

void command_gender(Client *c, const Seperator *sep)
{
	Mob *t = c->CastToMob();

	if (sep->IsNumber(1) && atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 500) {
		if ((c->GetTarget()) && c->Admin() >= commandGenderOthers) {
			t = c->GetTarget();
		}
		t->SendIllusionPacket(t->GetRace(), atoi(sep->arg[1]));
	}
	else {
		c->Message(Chat::White, "Usage: #gender [0/1/2]");
	}
}

