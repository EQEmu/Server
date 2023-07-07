#include "../../client.h"

void SetTimeZone(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set time_zone [Hour] [Minute]");
		c->Message(
			Chat::White,
			fmt::format(
				"Current time zone is {}.",
				Strings::ZoneTime(zone->zone_time.getEQTimeZoneHr(), zone->zone_time.getEQTimeZoneMin())
			).c_str()
		);
		return;
	}

	uint8 minutes = 0;
	uint8 hours   = Strings::ToUnsignedInt(sep->arg[2]);

	if (hours > 24) {
		hours = 24;
	}

	uint8 real_hours = (
		(hours - 1) > 0 ?
		(hours - 1) :
		0
	);

	if (sep->IsNumber(3)) {
		minutes = Strings::ToUnsignedInt(sep->arg[3]);

		if (minutes > 59) {
			minutes = 59;
		}
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Setting timezone to {}.",
			Strings::ZoneTime(hours, minutes)
		).c_str()
	);

	const int new_timezone = ((hours * 60) + minutes);
	zone->zone_time.setEQTimeZone(new_timezone);
	content_db.SetZoneTZ(zone->GetZoneID(), zone->GetInstanceVersion(), new_timezone);

	auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));

	auto t = (TimeOfDay_Struct *) outapp->pBuffer;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), t);

	entity_list.QueueClients(c, outapp);
	safe_delete(outapp);
}
