#include "../client.h"

void command_endurance(Client *c, const Seperator *sep)
{
	Mob* target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	int endurance = 0;
	if (target->IsClient()) {
		endurance = target->CastToClient()->GetMaxEndurance();
		target->CastToClient()->SetEndurance(endurance);
	} else {
		endurance = target->GetMaxEndurance();
		target->SetEndurance(endurance);
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Set {} to full Endurance ({}).",
			c->GetTargetDescription(target),
			endurance
		).c_str()
	);
}

