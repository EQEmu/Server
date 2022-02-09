#include "../client.h"

void command_date(Client *c, const Seperator *sep)
{
	//yyyy mm dd hh mm local
	if (sep->arg[3][0] == 0 || !sep->IsNumber(1) || !sep->IsNumber(2) || !sep->IsNumber(3)) {
		c->Message(Chat::Red, "Usage: #date yyyy mm dd [HH MM]");
	}
	else {
		int              h = 0, m = 0;
		TimeOfDay_Struct eqTime;
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eqTime);
		if (!sep->IsNumber(4)) {
			h = eqTime.hour;
		}
		else {
			h = atoi(sep->arg[4]);
		}
		if (!sep->IsNumber(5)) {
			m = eqTime.minute;
		}
		else {
			m = atoi(sep->arg[5]);
		}
		c->Message(Chat::Red, "Setting world time to %s-%s-%s %i:%i...", sep->arg[1], sep->arg[2], sep->arg[3], h, m);
		zone->SetDate(atoi(sep->arg[1]), atoi(sep->arg[2]), atoi(sep->arg[3]), h, m);
	}
}

