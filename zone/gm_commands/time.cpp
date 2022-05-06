#include "../client.h"

void command_time(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments || !sep->IsNumber(1)) {
		c->Message(Chat::White, "Usage: #time [Hour] [Minute]");

		TimeOfDay_Struct world_time;
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), &world_time);
		
		auto time_string = fmt::format(
			"{}:{}{} {} (Timezone: {}:{}{} {})",
			(
				((world_time.hour - 1) % 12) == 0 ?
				12 :
				((world_time.hour - 1) % 12)
			),
			(
				world_time.minute < 10 ?
				"0" :
				""
			),
			world_time.minute,
			(
				world_time.hour >= 13 ?
				"PM" :
				"AM"
			),
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
		);

		c->Message(
			Chat::White,
			fmt::format(
				"It is currently {}.",
				time_string
			).c_str()
		);

		return;
	}

	if (sep->IsNumber(1)) {
		uint8 minutes = 0;
		auto hours = static_cast<uint8>(std::stoul(sep->arg[1]) + 1);
		if (sep->IsNumber(2)) {
			minutes = static_cast<uint8>(std::stoul(sep->arg[2]));
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Setting world time to {}:{}{} {} (Timezone: {}:{}{} {}).",
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
				),
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

		zone->SetTime(hours, minutes);

		LogInfo(
			"{} :: Setting world time to {}:{} {} (Timezone: {}:{}{} {})",
			c->GetCleanName(),
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
			),
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
		);
	}
}

