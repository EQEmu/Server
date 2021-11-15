#include "../client.h"

void command_time(Client *c, const Seperator *sep)
{
	char timeMessage[255];
	int  minutes = 0;
	if (sep->IsNumber(1)) {
		if (sep->IsNumber(2)) {
			minutes = atoi(sep->arg[2]);
		}
		c->Message(Chat::Red, "Setting world time to %s:%i (Timezone: 0)...", sep->arg[1], minutes);
		zone->SetTime(atoi(sep->arg[1]) + 1, minutes);
		LogInfo("{} :: Setting world time to {}:{} (Timezone: 0)...", c->GetCleanName(), sep->arg[1], minutes);
	}
	else {
		c->Message(Chat::Red, "To set the Time: #time HH [MM]");
		TimeOfDay_Struct eqTime;
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eqTime);
		sprintf(
			timeMessage, "%02d:%s%d %s (Timezone: %ih %im)",
			((eqTime.hour - 1) % 12) == 0 ? 12 : ((eqTime.hour - 1) % 12),
			(eqTime.minute < 10) ? "0" : "",
			eqTime.minute,
			(eqTime.hour >= 13) ? "pm" : "am",
			zone->zone_time.getEQTimeZoneHr(),
			zone->zone_time.getEQTimeZoneMin()
		);
		c->Message(Chat::Red, "It is now %s.", timeMessage);
		LogInfo("Current Time is: {}", timeMessage);
	}
}

