#include "../client.h"

void command_unfreeze(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	auto target = c->GetTarget();
	target->SendAppearancePacket(AT_Anim, ANIM_STAND);

	c->Message(
		Chat::White,
		fmt::format(
			"You have unfrozen {}.",
			c->GetTargetDescription(target)
		).c_str()
	);
}

