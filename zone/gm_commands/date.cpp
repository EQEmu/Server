#include "../client.h"

void command_date(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (
		!arguments ||
		!sep->IsNumber(1) ||
		!sep->IsNumber(2) ||
		!sep->IsNumber(3)
	) {
		c->Message(Chat::White, "Usage: #date [Year] [Month] [Day] [Hour] [Minute]");
		return;
	}

	TimeOfDay_Struct eq_time;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), &eq_time);

	auto year = static_cast<uint16>(std::stoul(sep->arg[1]));
	auto month = static_cast<uint8>(std::stoul(sep->arg[2]));
	auto day = static_cast<uint8>(std::stoul(sep->arg[3]));

	auto hour = !sep->IsNumber(4) ? eq_time.hour : static_cast<uint8>(std::stoul(sep->arg[4]) + 1);
	auto minute = !sep->IsNumber(5) ? eq_time.minute : static_cast<uint8>(std::stoul(sep->arg[5]));

	c->Message(
		Chat::White,
		fmt::format("Setting world time to {}/{}/{} {:02}:{:02} {}.",
			year,
			month,
			day,
			(
				(hour % 12) == 0 ?
				12 :
				(hour % 12)
			),
			minute,
			(
				hour >= 13 ?
				"PM" :
				"AM"
			)
		).c_str()
	);

	zone->SetDate(year, month, day, hour, minute);

	LogInfo(
		"{} :: Setting world time to {}/{}/{} {:02}:{:02} {}.",
		c->GetCleanName(),
		year,
		month,
		day,
		(
			(hour % 12) == 0 ?
			12 :
			(hour % 12)
		),
		minute,
		(
			hour >= 13 ?
			"PM" :
			"AM"
		)
	);
}

