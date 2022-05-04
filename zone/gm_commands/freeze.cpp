#include "../client.h"

void command_freeze(Client *c, const Seperator *sep)
{
	if (c->GetTarget()) {
		auto target = c->GetTarget();
		if (target != c) {
			target->SendAppearancePacket(AT_Anim, ANIM_FREEZE);
		} else {
			c->Message(Chat::White, "You cannot freeze yourself.");
		}
	} else {
		c->Message(Chat::White, "You must have a target to use this command.");
	}
}

