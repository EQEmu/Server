#include "../client.h"

void command_loc(Client *c, const Seperator *sep)
{
	Mob *target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	auto target_position = target->GetPosition();

	c->Message(
		Chat::White,
		fmt::format(
			"Location for {} | XYZ: {:.2f}, {:.2f}, {:.2f} Heading: {:.2f}",
			(
				c == target ?
				"Yourself" :
				fmt::format(
					"{} ({})",
					target->GetCleanName(),
					target->GetID()
				)
			),
			target_position.x,
			target_position.y,
			target_position.z,
			target_position.w
		).c_str()
	);
}

