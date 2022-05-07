#include "../client.h"

void command_randomfeatures(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	auto target = c->GetTarget();
	
	if (target->RandomizeFeatures()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) has had their features randomized.",
				target->GetCleanName(),
				target->GetID()
			).c_str()
		);
	} else {
		c->Message(
			Chat::White,
			fmt::format(
				"{} ({}) is not a player race, their race is {} ({}).",
				target->GetCleanName(),
				target->GetID(),
				GetRaceIDName(target->GetRace()),
				target->GetRace()
			).c_str()
		);
	}
}

