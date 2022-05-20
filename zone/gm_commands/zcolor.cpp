#include "../client.h"

void command_zcolor(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (
		!arguments ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3)
	) {
		c->Message(Chat::White, "Usage: #zcolor [Red] [Green] [Blue] [Permanent (0 = False, 1 = True)]");
		return;
	}

	auto red = std::stoul(sep->arg[1]);
	auto green = std::stoul(sep->arg[2]);
	auto blue = std::stoul(sep->arg[3]);
	auto permanent = sep->arg[4] ? atobool(sep->arg[4]) : false;
	if (
		red < 0 ||
		red > 255 ||
		green < 0 ||
		green > 255 ||
		blue < 0 ||
		blue > 255
	) {
		c->Message(Chat::White, "Colors cannot be less than 0 or greater than 255.");
		return;
	}

	if (permanent) {		
		auto query = fmt::format(
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

	for (int fog_index = 0; fog_index < 4; fog_index++) {
		zone->newzone_data.fog_red[fog_index] = static_cast<uint8>(red);
		zone->newzone_data.fog_green[fog_index] = static_cast<uint8>(green);
		zone->newzone_data.fog_blue[fog_index] = static_cast<uint8>(blue);
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
