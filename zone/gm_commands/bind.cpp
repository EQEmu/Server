#include "../client.h"

void command_bind(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	target->SetBindPoint();

	bool in_persistent_instance = (
		zone->GetInstanceID() != 0 &&
		zone->IsInstancePersistent()
	);

	auto target_string = (
		c == target ?
		"Yourself" :
		fmt::format(
			"{} ({})",
			target->GetCleanName(),
			target->GetID()
		)
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Set Bind Point for {} | Zone: {} ({}) ID: {} {}",
			target_string,
			zone->GetLongName(),
			zone->GetShortName(),
			zone->GetZoneID(),
			(
				in_persistent_instance ?
				fmt::format(
					" Instance ID: {}",
					zone->GetInstanceID()
				) :
				""
			)
		).c_str()
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Set Bind Point for {} | XYZ: {:.2f}, {:.2f}, {:.2f}",
			target_string,
			target->GetX(),
			target->GetY(),
			target->GetZ()
		).c_str()
	);
}
