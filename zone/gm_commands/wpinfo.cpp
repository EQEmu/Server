#include "../client.h"

void command_wpinfo(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	auto target = c->GetTarget()->CastToNPC();

	if (!target->GetGrid()) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is not a part of any grid.",
				c->GetTargetDescription(target)
			).c_str()
		);
		return;
	}

	target->DisplayWaypointInfo(c);
}

