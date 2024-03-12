#include "../client.h"

void command_resetaa(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "You must target a player to use this command.");
		return;
	}

	auto t = c->GetTarget()->CastToClient();
	t->ResetAA();

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully reset all Alternate Advancements for {}.",
			c->GetTargetDescription(t)
		).c_str()
	);
}
