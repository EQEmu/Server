#include "../client.h"

void command_grantaa(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsClient()) {
		c->Message(Chat::White, "You must target a player to use this command.");
		return;
	}

	auto t = c->GetTarget()->CastToClient();
	t->GrantAllAAPoints();

	c->Message(
		Chat::White,
		fmt::format(
			"Successfully granted all Alternate Advancements for {}.",
			c->GetTargetDescription(t)
		).c_str()
	);
}
