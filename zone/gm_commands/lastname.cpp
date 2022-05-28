#include "../client.h"

void command_lastname(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	LogInfo("#lastname request from [{}] for [{}]", c->GetCleanName(), target->GetCleanName());

	bool is_remove = !strcasecmp(sep->argplus[1], "-1");
	std::string last_name = is_remove ? "" : sep->argplus[1];

	if (last_name.size() > 64) {
		c->Message(Chat::White, "Last name must be 64 characters or less.");
		return;
	}
	
	target->ChangeLastName(last_name);

	c->Message(
		Chat::White,
		fmt::format(
			"Last name has been {}{} for {}{}",
			is_remove ? "removed" : "changed",
			!is_remove ? " and saved" : "",
			c->GetTargetDescription(target),
			(
				is_remove ?
				"." :
				fmt::format(
					" to '{}'.",
					last_name
				)
			)
		).c_str()
	);
}

