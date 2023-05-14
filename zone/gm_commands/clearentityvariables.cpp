#include "../client.h"

void command_clearentityvariables(Client *c, const Seperator *sep)
{
	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const auto cleared = t->ClearEntityVariables();

	if (!cleared) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have any entity variables to clear.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "do" : "does"
			).c_str()
		);

		return;
	}

	c->Message(
		Chat::White,
		fmt::format(
			"Cleared all entity variables for {}.",
			c->GetTargetDescription(t, TargetDescriptionType::LCSelf)
		).c_str()
	);
}
