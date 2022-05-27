#include "../client.h"

void command_zone_instance(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #zoneinstance [Instance ID] [X] [Y] [Z]");
		return;
	}

	auto instance_id = std::stoul(sep->arg[1]);
	if (!instance_id) {
		c->Message(Chat::White, "You must enter a valid instance id.");
		return;
	}

	if (!database.CheckInstanceExists(instance_id)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Instance ID {} does not exist.",
				instance_id
			).c_str()
		);
		return;
	}

	auto zone_id = database.ZoneIDFromInstanceID(instance_id);
	if (!zone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Instance ID {} not found or zone is set to null.",
				instance_id
			).c_str()
		);
		return;
	}

	if (database.CharacterInInstanceGroup(instance_id, c->CharacterID())) {
		c->Message(Chat::White, "You are already a part of this instance, sending you there.");
		c->MoveZoneInstance(instance_id);
		return;
	}

	if (!database.VerifyInstanceAlive(instance_id, c->CharacterID())) {
		c->Message(
			Chat::White,
			fmt::format(
				"Instance ID {} expired or you are not apart of this instance.",
				instance_id
			).c_str()
		);
		return;
	}

	auto x = sep->IsNumber(2) ? std::stof(sep->arg[2]) : 0.0f;
	auto y = sep->IsNumber(3) ? std::stof(sep->arg[3]) : 0.0f;
	auto z = sep->IsNumber(4) ? std::stof(sep->arg[4]) : 0.0f;

	c->MovePC(
		zone_id,
		instance_id,
		x,
		y,
		z,
		0.0f,
		sep->IsNumber(2) ? 0 : ZoneToSafeCoords
	);
}
