#include "../client.h"

void command_undye(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	target->Undye();
	c->Message(
		Chat::White,
		fmt::format(
			"Undyed armor for {}.",
			c->GetTargetDescription(target)
		).c_str()
	);
}
