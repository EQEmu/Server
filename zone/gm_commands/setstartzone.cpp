#include "../client.h"

void command_setstartzone(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #setstartzone [Zone ID|Zone Short Name]");
		c->Message(
			Chat::White,
			"Optional Usage: Use '#setstartzone Reset' or '#setstartzone 0' to clear a starting zone. Player can select a starting zone using /setstartcity"
		);
		return;
	}

	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	auto zone_id = (
		sep->IsNumber(1) ?
		std::stoul(sep->arg[1]) :
		ZoneID(sep->arg[1])
	);

	target->SetStartZone(zone_id);

	bool is_reset = (
		!strcasecmp(sep->arg[1], "reset") ||
		zone_id == 0
	);

	c->Message(
		Chat::White,
		fmt::format(
			"Start Zone {} for {} |{}",
			is_reset ? "Reset" : "Changed",
			c->GetTargetDescription(target, TargetDescriptionType::UCSelf),
			(
				zone_id ?
				fmt::format(
					" {} ({}) ID: {}",
					ZoneLongName(zone_id),
					ZoneName(zone_id),
					zone_id
				) :
				""
			)
		).c_str()
	);
}

