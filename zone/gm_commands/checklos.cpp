#include "../client.h"

void command_checklos(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	auto target = c->GetTarget();

	bool has_los = c->CheckLosFN(target);

	c->Message(
		Chat::White,
		fmt::format(
			"You {}have line of sight to {}.",
			has_los ? "" : "do not ",
			c->GetTargetDescription(target)
		).c_str()
	);
}

