#include "../client.h"

void command_zheader(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #zheader [Zone ID|Zone Short Name] [Version]");
		return;
	}

	auto zone_id = (
		sep->IsNumber(2) ?
		std::stoul(sep->arg[2]) :
		ZoneID(sep->arg[2])
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
	auto version = (
		sep->IsNumber(3) ?
		std::stoul(sep->arg[3]) :
		0
	);
	
	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);

	c->Message(
		Chat::White,
		fmt::format(
			"Zone Header Load {} | Zone: {} ({}){}",
			(
				zone->LoadZoneCFG(zone_short_name, version) ?
				"Suceeded" :
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
