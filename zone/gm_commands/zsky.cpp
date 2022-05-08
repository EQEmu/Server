#include "../client.h"

void command_zsky(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #zsky [Sky Type] [Permanent (0 = False, 1 = True)]");
		return;
	}

	auto sky_type = std::stoul(sep->arg[1]);
	auto permanent = sep->arg[2] ? atobool(sep->arg[2]) : false;
	if (sky_type < 0 || sky_type > 255) {
		c->Message(Chat::White, "Sky Type cannot be less than 0 or greater than 255!");
		return;
	}

	if (permanent) {
		auto query = fmt::format(
			"UPDATE zone SET sky = {} WHERE zoneidnumber = {} AND version = {}",
			sky_type,
			zone->GetZoneID(),
			zone->GetInstanceVersion()
		);
		database.QueryDatabase(query);
	}

	zone->newzone_data.sky = static_cast<uint8>(sky_type);
	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);

	c->Message(
		Chat::White,
		fmt::format(
			"Sky Changed | Zone: {} Sky Type: {} Permanent: {}",
			zone->GetZoneDescription(),
			sky_type,
			permanent ? "Yes" : "No"
		).c_str()
	);
}
