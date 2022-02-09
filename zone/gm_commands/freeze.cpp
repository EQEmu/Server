#include "../client.h"

void command_freeze(Client *c, const Seperator *sep)
{
	if (c->GetTarget() != 0) {
		c->GetTarget()->SendAppearancePacket(AT_Anim, ANIM_FREEZE);
	}
	else {
		c->Message(Chat::White, "ERROR: Freeze requires a target.");
	}
}

