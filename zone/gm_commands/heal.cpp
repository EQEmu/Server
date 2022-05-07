#include "../client.h"

void command_heal(Client *c, const Seperator *sep)
{
	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	target->Heal();
	
	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to full Health ({}).",
			c->GetTargetDescription(target),
			target->GetMaxHP()
		).c_str()
	);
}

