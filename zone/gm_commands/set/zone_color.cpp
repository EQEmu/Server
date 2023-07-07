#include "../../client.h"

void SetZoneColor(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (
		arguments < 2 ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3) ||
		!sep->IsNumber(4)
	) {
		c->Message(Chat::White, "Usage: #set zone_color [Red] [Green] [Blue] [Permanent (0 = False, 1 = True)]");
		return;
	}

	const uint8 red       = Strings::ToUnsignedInt(sep->arg[2]);
	const uint8 green     = Strings::ToUnsignedInt(sep->arg[3]);
	const uint8 blue      = Strings::ToUnsignedInt(sep->arg[4]);
	const bool  permanent = sep->arg[5] ? Strings::ToBool(sep->arg[5]) : false;

	if (
		!EQ::ValueWithin(red, 0, UINT8_MAX) ||
		!EQ::ValueWithin(green, 0, UINT8_MAX) ||
		!EQ::ValueWithin(blue, 0, UINT8_MAX)
	) {
		c->Message(Chat::White, "Colors cannot be less than 0 or greater than 255.");
		return;
	}

	if (permanent) {
		const std::string& query = fmt::format(
			"UPDATE zone SET fog_red = {}, fog_green = {}, fog_blue = {} "
			"WHERE zoneidnumber = {} AND version = {}",
			red,
			green,
			blue,
			zone->GetZoneID(),
			zone->GetInstanceVersion()
		);
		database.QueryDatabase(query);
	}

	for (uint8 fog_index = 0; fog_index < 4; fog_index++) {
		zone->newzone_data.fog_red[fog_index]   = red;
		zone->newzone_data.fog_green[fog_index] = green;
		zone->newzone_data.fog_blue[fog_index]  = blue;
	}

	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);

	c->Message(
		Chat::White,
		fmt::format(
			"Fog Color Changed | Zone: {} Red: {} Green: {} Blue: {} Permanent: {}",
			zone->GetZoneDescription(),
			red,
			green,
			blue,
			permanent ? "Yes" : "No"
		).c_str()
	);
}
