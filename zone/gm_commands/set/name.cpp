#include "../../client.h"

void SetName(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;
	if (arguments < 2) {
		c->Message(Chat::White, "Usage: #set name [Name]");
		return;
	}

	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		auto t = c->GetTarget()->CastToClient();

		std::string new_name = sep->arg[1];
		std::string old_name = t->GetCleanName();

		if (t->ChangeFirstName(new_name.c_str(), c->GetCleanName())) {
			c->Message(
				Chat::White,
				fmt::format(
					"Successfully renamed {} to {}",
					old_name,
					new_name
				).c_str()
			);

			c->Message(Chat::White, "Sending player to char select.");

			t->Kick("Name was changed");

			return;
		}

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
