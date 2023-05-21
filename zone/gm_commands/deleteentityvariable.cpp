#include "../client.h"

void command_deleteentityvariable(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	if (!arguments) {
		c->Message(Chat::White, "Usage: #deleteentityvariable [Variable Name]");
		c->Message(Chat::White, "Example: #deleteentityvariable Test Variable");
		return;
	}

	Mob *t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	const std::string variable_name = sep->argplus[1];

	if (!t->EntityVariableExists(variable_name)) {
		c->Message(
			Chat::White,
			fmt::format(
				"{} {} not have an entity variable named '{}'.",
				c->GetTargetDescription(t, TargetDescriptionType::UCYou),
				c == t ? "do" : "does",
				variable_name
			).c_str()
		);

		return;
	}

	t->DeleteEntityVariable(variable_name);

	c->Message(
		Chat::White,
		fmt::format(
			"Deleted an entity variable named '{}' from {}.",
			variable_name,
			c->GetTargetDescription(t)
		).c_str()
	);
}
