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
			c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
			target_position.x,
			target_position.y,
			target_position.z,
			target_position.w
		).c_str()
	);
}

