#include "../client.h"

void command_race(Client *c, const Seperator *sep)
{
	Mob *target = c->CastToMob();

	if (sep->IsNumber(1)) {
		auto race = atoi(sep->arg[1]);
		if ((race >= 0 && race <= RuleI(NPC, MaxRaceID)) || (race >= 2253 && race <= 2259)) {
			if ((c->GetTarget()) && c->Admin() >= commandRaceOthers) {
				target = c->GetTarget();
			}
			target->SendIllusionPacket(race);
		}
		else {
			c->Message(
				Chat::White,
				fmt::format(
					"Usage: #race [0-{}, 2253-2259] (0 for back to normal)",
					RuleI(NPC, MaxRaceID)).c_str());
		}
	}
	else {
		c->Message(
			Chat::White,
			fmt::format("Usage: #race [0-{}, 2253-2259] (0 for back to normal)", RuleI(NPC, MaxRaceID)).c_str());
	}
}

