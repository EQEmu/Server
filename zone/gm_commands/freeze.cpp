#include "../client.h"

void command_freeze(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	auto target = c->GetTarget();
	if (c == target) {
		c->Message(Chat::White, "You cannot freeze yourself.");
		return;
	}

	target->SendAppearancePacket(AT_Anim, ANIM_FREEZE);

	c->Message(
		Chat::White,
		fmt::format(
			"You have frozen {}.",
			c->GetTargetDescription(target)
		).c_str()
	);
}

