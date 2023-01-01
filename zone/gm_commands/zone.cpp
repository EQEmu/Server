#include "../client.h"

void command_zone(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #zone [Zone ID|Zone Short Name] [X] [Y] [Z]");
		return;
	}

	// input: (first arg)
	// zone identifier can be a string of a zone name or its ID
	std::string zone_input = sep->arg[1];
	uint32      zone_id    = 0;

	// if input is id
	if (Strings::IsNumber(zone_input)) {
		zone_id = std::stoi(zone_input);

		// validate
		if (zone_id != 0 && !GetZone(zone_id)) {
			c->Message(Chat::White, fmt::format("Could not find zone by id [{}]", zone_id).c_str());
			return;
		}
	}
	else {
		// validate
		if (!zone_store.GetZone(zone_input)) {
			c->Message(Chat::White, fmt::format("Could not find zone by short_name [{}]", zone_input).c_str());
			return;
		}

		// validate we got id
		zone_id = ZoneID(zone_input);
		if (zone_id == 0) {
			c->Message(Chat::White, fmt::format("Could not find zone id by short_name [{}]", zone_input).c_str());
			return;
		}
	}

	// if zone identifier is a number and the id is 0, send to safe coordinates of the local zone
	if (Strings::IsNumber(zone_input) && zone_id == 0) {
		c->Message(Chat::White, "Sending you to the safe coordinates of this zone.");

		c->MovePC(
			0.0f,
			0.0f,
			0.0f,
			0.0f,
			0,
			ZoneToSafeCoords
		);
		return;
	}

	// status checking
	auto min_status = content_db.GetMinStatus(zone_id, 0);
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

	// fetch zone data
	auto zd = GetZoneVersionWithFallback(zone_id, 0);
	if (zone_id == 0) {
		c->Message(Chat::White, fmt::format("Failed to find zone with fallback [{}]", zone_id).c_str());
		return;
	}

	// coordinates
	auto x         = sep->IsNumber(2) ? std::stof(sep->arg[2]) : 0.0f;
	auto y         = sep->IsNumber(3) ? std::stof(sep->arg[3]) : 0.0f;
	auto z         = sep->IsNumber(4) ? std::stof(sep->arg[4]) : 0.0f;
	auto zone_mode = sep->IsNumber(2) ? ZoneSolicited : ZoneToSafeCoords;

	c->MovePC(
		zone_id,
		x,
		y,
		z,
		zd ? zd->safe_heading : 0.0f,
		0,
		zone_mode
	);
}
