#include "../client.h"
#include "../groups.h"

void command_refreshgroup(Client *c, const Seperator *sep)
{
	auto target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	Group *group = target->GetGroup();

	if (!group) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} not in a group.",
				(
					c == target ?
					"You are" :
					fmt::format(
						"{} ({}} is",
						target->GetCleanName(),
						target->GetID()
					)
				)
			).c_str()
		);
		return;
	}

	database.RefreshGroupFromDB(target);

	c->Message(
		Chat::White,
		fmt::format(
			"Group has been refreshed for {}.",
			(
				c == target ?
				"yourself" :
				fmt::format(
					"{} ({})",
					target->GetCleanName(),
					target->GetID()
				)
			)
		).c_str()
	);

	if (c != target) {
		target->Message(
			Chat::White,
			fmt::format(
				"Your group has been refreshed by {}.",
				c->GetCleanName()
			).c_str()
		);
	}
}

