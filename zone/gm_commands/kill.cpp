#include "../client.h"

void command_kill(Client *c, const Seperator *sep)
{
	auto target = c->GetTarget();
	if (!target) {
		c->Message(Chat::White, "You must have a target to use this command.");
		return;
	}

	if (
		!target->IsClient() ||
		target->CastToClient()->Admin() <= c->Admin()
	) {
		if (c != target) {
			c->Message(
				Chat::White,
				fmt::format(
					"Killing {}.",
					c->GetTargetDescription(target)
				).c_str()
			);
		}

		target->Kill();
	}
}

