#include "../client.h"

void command_appearance(Client *c, const Seperator *sep)
{
	Mob *t = c->CastToMob();

	// sends any appearance packet
	// Dev debug command, for appearance types
	if (sep->arg[2][0] == 0) {
		c->Message(Chat::White, "Usage: #appearance type value");
	}
	else {
		if ((c->GetTarget())) {
			t = c->GetTarget();
		}
		t->SendAppearancePacket(atoi(sep->arg[1]), atoi(sep->arg[2]));
		c->Message(
			Chat::White,
			"Sending appearance packet: target=%s, type=%s, value=%s",
			t->GetName(),
			sep->arg[1],
			sep->arg[2]
		);
	}
}

