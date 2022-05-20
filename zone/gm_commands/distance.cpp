#include "../client.h"

void command_distance(Client *c, const Seperator *sep)
{
	if (c->GetTarget()) {
		Mob *target = c->GetTarget();
		if (c != target) {
			c->Message(
				Chat::White,
				fmt::format(
					"{} is {:.2f} units from you.",
					c->GetTargetDescription(target),
					Distance(
						c->GetPosition(),
						target->GetPosition()
					)
				).c_str()
			);
		}
	}
}

