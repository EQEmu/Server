#include "../client.h"

void command_timezone(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == 0 && !sep->IsNumber(1)) {
		c->Message(Chat::Red, "Usage: #timezone HH [MM]");
		c->Message(
			Chat::Red,
			"Current timezone is: %ih %im",
			zone->zone_time.getEQTimeZoneHr(),
			zone->zone_time.getEQTimeZoneMin());
	}
	else {
		uint8 hours   = atoi(sep->arg[1]);
		uint8 minutes = atoi(sep->arg[2]);
		if (!sep->IsNumber(2)) {
			minutes = 0;
		}
		c->Message(Chat::Red, "Setting timezone to %i h %i m", hours, minutes);
		uint32 ntz = (hours * 60) + minutes;
		zone->zone_time.setEQTimeZone(ntz);
		content_db.SetZoneTZ(zone->GetZoneID(), zone->GetInstanceVersion(), ntz);

		// Update all clients with new TZ.
		auto             outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
		TimeOfDay_Struct *tod   = (TimeOfDay_Struct *) outapp->pBuffer;
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), tod);
		entity_list.QueueClients(c, outapp);
		safe_delete(outapp);
	}
}

