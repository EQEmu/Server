#include "../../client.h"

void SetZoneSky(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set zone_sky [Sky Type] [Permanent (0 = False, 1 = True)]");
		return;
	}

	const uint8 sky_type  = Strings::ToUnsignedInt(sep->arg[2]);
	const bool  permanent = sep->arg[3] ? Strings::ToBool(sep->arg[3]) : false;

	if (!EQ::ValueWithin(sky_type, 0, UINT8_MAX)) {
		c->Message(Chat::White, "Sky Type cannot be less than 0 or greater than 255!");
		return;
	}

	if (permanent) {
		const std::string& query = fmt::format(
			"UPDATE zone SET sky = {} WHERE zoneidnumber = {} AND version = {}",
			sky_type,
			zone->GetZoneID(),
			zone->GetInstanceVersion()
		);
		database.QueryDatabase(query);
	}

	zone->newzone_data.sky = sky_type;
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
