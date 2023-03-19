#include "../client.h"

void command_bind(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	bool bind_allowed = (
		!zone->GetInstanceID() ||
		(
			zone->GetInstanceID() != 0 &&
			zone->IsInstancePersistent()
		)
	);

	if (!bind_allowed) {
		c->Message(Chat::White, "You cannot bind here.");
		return;
	}

	target->SetBindPoint();

	c->Message(
		Chat::White,
		fmt::format(
			"Set Bind Point for {} | Zone: {}",
			c->GetTargetDescription(target),
			zone->GetZoneDescription()
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Set Bind Point for {} | XYZ: {:.2f}, {:.2f}, {:.2f}",
			c->GetTargetDescription(target),
			target->GetX(),
			target->GetY(),
			target->GetZ()
		).c_str()
	);
}
