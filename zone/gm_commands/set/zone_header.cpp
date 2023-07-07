#include "../../client.h"

void SetZoneHeader(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #set zone_header [Zone ID|Zone Short Name] [Version]");
		return;
	}

	const auto zone_id = (
		sep->IsNumber(1) ?
			Strings::ToUnsignedInt(sep->arg[1]) :
			ZoneID(sep->arg[1])
	);
	if (!zone_id) {
		c->Message(
			Chat::White,
			fmt::format(
				"Zone ID {} could not be found.",
				zone_id
			).c_str()
		);
		return;
	}

	auto zone_short_name = ZoneName(zone_id);
	auto zone_long_name = ZoneLongName(zone_id);
	const auto version = (
		sep->IsNumber(2) ?
			Strings::ToUnsignedInt(sep->arg[2]) :
			0
	);

	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);

	zone_store.LoadZones(content_db);

	c->Message(
		Chat::White,
		fmt::format(
			"Zone Header Load {} | Zone: {} ({}){}",
			(
				zone->LoadZoneCFG(zone_short_name, version) ?
					"Succeeded" :
					"Failed"
			),
			zone_long_name,
			zone_short_name,
			(
				version ?
					fmt::format(
						" Version: {}",
						version
					) :
					""
			)
		).c_str()
	);
}
