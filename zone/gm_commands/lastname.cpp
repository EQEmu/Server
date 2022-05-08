#include "../client.h"

void command_lastname(Client *c, const Seperator *sep)
{
	Client *target = c;
	if (c->GetTarget() && c->GetTarget()->IsClient()) {
		target = c->GetTarget()->CastToClient();
	}

	LogInfo("#lastname request from [{}] for [{}]", c->GetCleanName(), target->GetCleanName());

	std::string last_name = sep->arg[1];
	if (last_name.size() > 64) {
		c->Message(Chat::White, "Usage: #lastname [Last Name] (Last Name must be 64 characters or less)");
		return;
	}
	
	target->ChangeLastName(last_name.c_str());
	c->Message(
		Chat::White,
		fmt::format(
			"{} now {} a last name of '{}'.",
			c->GetTargetDescription(target, TargetDescriptionType::UCYou),
			c == target ? "have" : "has",
			last_name
		).c_str()
	);
}

