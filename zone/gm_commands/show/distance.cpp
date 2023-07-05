#include "../../client.h"

void ShowDistance(Client *c, const Seperator *sep)
{
	if (!c->GetTarget()) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	auto t = c->GetTarget();
	if (c != t) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} is {:.2f} units from you.",
				c->GetTargetDescription(t),
				Distance(
					c->GetPosition(),
					t->GetPosition()
				)
			).c_str()
		);
	}
}

