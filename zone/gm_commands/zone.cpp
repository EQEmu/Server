#include "../client.h"

void command_zone(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #zone [Zone ID|Zone Short Name] [X] [Y] [Z]");
		return;
	}

	auto zone_id = (
		sep->IsNumber(1) ?
		std::stoul(sep->arg[1]) :
		ZoneID(sep->arg[1])
	);
	auto zone_short_name = ZoneName(zone_id);
	if (
		!zone_id ||
		!zone_short_name
	) {
		c->Message(
			Chat::White,
			fmt::format(
				"No zones were found matching '{}'.",
				sep->arg[1]
			).c_str()
		);
		return;
	}

	auto min_status = database.GetMinStatus(zone_id, 0);
	if (c->Admin() < min_status) {
		c->Message(Chat::White, "Your status is not high enough to go to this zone.");
		return;
	}

#ifdef BOTS
	// This block is necessary to clean up any bot objects owned by a Client
	if (zone_id != c->GetZoneID()) {
		Bot::ProcessClientZoneChange(c);
	}
#endif

	auto x = sep->IsNumber(2) ? std::stof(sep->arg[2]) : 0.0f;
	auto y = sep->IsNumber(3) ? std::stof(sep->arg[3]) : 0.0f;
	auto z = sep->IsNumber(4) ? std::stof(sep->arg[4]) : 0.0f;

	c->MovePC(
		zone_id,
		x,
		y,
		z,
		0.0f,
		sep->IsNumber(2) ? 0 : ZoneToSafeCoords
	);
}