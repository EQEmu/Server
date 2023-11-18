#include "../../client.h"

void SetTime(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2 || !sep->IsNumber(2)) {
		c->Message(Chat::White, "Usage: #set time [Hour] [Minute]");

		TimeOfDay_Struct world_time{};
		zone->zone_time.GetCurrentEQTimeOfDay(time(0), &world_time);

		auto time_string = fmt::format(
			"{} (Timezone: {})",
			Strings::ZoneTime(world_time.hour, world_time.minute),
			Strings::ZoneTime(zone->zone_time.getEQTimeZoneHr(), zone->zone_time.getEQTimeZoneHr())
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

	uint8 minutes = 0;
	uint8 hours = Strings::ToUnsignedInt(sep->arg[2]) + 1;

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
			"Setting world time to {} (Timezone: {}).",
			Strings::ZoneTime(hours, minutes),
			Strings::ZoneTime(zone->zone_time.getEQTimeZoneHr(), zone->zone_time.getEQTimeZoneHr())
		).c_str()
	);

	zone->SetTime(real_hours, minutes);

	LogInfo(
		"{} :: Setting world time to {} (Timezone: {})",
		c->GetCleanName(),
		Strings::ZoneTime(hours, minutes),
		Strings::ZoneTime(zone->zone_time.getEQTimeZoneHr(), zone->zone_time.getEQTimeZoneHr())
	);
}
