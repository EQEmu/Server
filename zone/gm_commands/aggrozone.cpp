#include "../client.h"

void command_aggrozone(Client *c, const Seperator *sep)
{
	Mob *target = c;
	if (c->GetTarget()) {
		target = c->GetTarget();
	}

	uint32 hate = 0;
	if (sep->IsNumber(1)) {
		hate = std::stoul(sep->arg[1]);
	}

	entity_list.AggroZone(target, hate);
	c->Message(
		Chat::White,
		fmt::format(
			"Aggroing zone on {}.",
			(
				c == target ?
				"yourself" :
				fmt::format(
					"{} ({})",
					target->GetCleanName(),
					target->GetID()
				)
			)
		).c_str()
	);
}
