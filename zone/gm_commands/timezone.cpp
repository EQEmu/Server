#include "../client.h"

void command_timezone(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #timezone [Hour] [Minute]");
		c->Message(
			Chat::White,
			fmt::format(
				"Current timezone is {}:{}{} {}.",
				(
					((zone->zone_time.getEQTimeZoneHr() - 1) % 12) == 0 ?
					12 :
					((zone->zone_time.getEQTimeZoneHr() - 1) % 12)
				),
				(
					zone->zone_time.getEQTimeZoneMin() < 10 ?
					"0" :
					""
				),
				zone->zone_time.getEQTimeZoneMin(),
				(
					zone->zone_time.getEQTimeZoneHr() >= 13 ?
					"PM" :
					"AM"
				)
			).c_str()
		);
		return;
	}

	auto hours = static_cast<uint8>(std::stoul(sep->arg[1]));
	uint8 minutes = 0;

	if (sep->IsNumber(2)) {
		minutes = static_cast<uint8>(std::stoul(sep->arg[2]));
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Setting timezone to {}:{}{} {}.",
			(
				((hours - 1) % 12) == 0 ?
				12 :
				((hours - 1) % 12)
			),
			(
				minutes < 10 ?
				"0" :
				""
			),
			minutes,
			(
				hours >= 13 ?
				"PM" :
				"AM"
			)
		).c_str()
	);

	uint32 new_timezone = ((hours * 60) + minutes);
	zone->zone_time.setEQTimeZone(new_timezone);
	content_db.SetZoneTZ(zone->GetZoneID(), zone->GetInstanceVersion(), new_timezone);

	auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
	TimeOfDay_Struct *tod = (TimeOfDay_Struct *) outapp->pBuffer;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), tod);
	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);
}

