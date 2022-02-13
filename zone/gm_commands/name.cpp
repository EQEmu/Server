#include "../client.h"

void command_name(Client *c, const Seperator *sep)
{
	int arguments = sep->argnum;
	if (!arguments) {
		c->Message(Chat::White, "Usage: #name [New Name] - Rename your player target");
		return;
	}

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		auto target = c->GetTarget()->CastToClient();

		std::string new_name = sep->arg[1];
		std::string old_name = target->GetCleanName();

		if (target->ChangeFirstName(new_name.c_str(), c->GetCleanName())) {
			c->Message(
				Chat::White,
				fmt::format(
					"Successfully renamed {} to {}",
					old_name,
					new_name
				).c_str()
			);

			c->Message(Chat::White, "Sending player to char select.");

			target->Kick("Name was changed");
		} else {
			c->Message(
				Chat::White,
				fmt::format(
					"Unable to rename {}. Check that the new name '{}' isn't already taken.",
					old_name,
					new_name
				).c_str()
			);
		}
	}
}

