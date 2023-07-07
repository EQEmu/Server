#include "../../client.h"

void SetZoneUnderworld(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set zone_underworld [Z] [Permanent (0 = False, 1 = True)]");
		return;
	}

	const float z = Strings::ToFloat(sep->arg[2]);
	const bool permanent = sep->arg[3] ? Strings::ToBool(sep->arg[3]) : false;

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
