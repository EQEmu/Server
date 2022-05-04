#include "../client.h"

void command_unfreeze(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	c->GetTarget()->SendAppearancePacket(AT_Anim, ANIM_STAND);
}

