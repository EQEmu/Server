#include "../client.h"

void command_checklos(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
	}

	bool has_los = c->CheckLosFN(c->GetTarget());
	c->Message(
		Chat::White,
		fmt::format(
			"You {}have line of sight to {} ({}).",
			has_los ? "" : "do not ",
			c->GetTarget()->GetCleanName(),
			c->GetTarget()->GetID()
		).c_str()
	);
}

