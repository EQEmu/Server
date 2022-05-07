#include "../client.h"

void command_zunderworld(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #zunderworld [Z] [Permanent (0 = False, 1 = True)]");
		return;
	}

	auto z = std::stof(sep->arg[1]);
	auto permanent = sep->arg[2] ? atobool(sep->arg[2]) : false;
	if (permanent) {
		auto query = fmt::format(
			"UPDATE zone SET underworld = {:.2f} WHERE zoneidnumber = {} AND version = {}",
			z,
			zone->GetZoneID(),
			zone->GetInstanceVersion()
		);
		database.QueryDatabase(query);
	}

	zone->newzone_data.underworld = z;
	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);

	c->Message(
		Chat::White,
		fmt::format(
			"Underworld Z Changed | Zone: {} Z: {:.2f} Permanent: {}",
			zone->GetZoneDescription(),
			z,
			permanent ? "Yes" : "No"
		).c_str()
	);
}
