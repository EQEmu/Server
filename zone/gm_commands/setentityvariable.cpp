#include "../client.h"

void command_setentityvariable(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	if (!arguments) {
		c->Message(Chat::White, "Usage: #setentityvariable [Variable Name] [Variable Value]");
		c->Message(Chat::White, "Note: You can have spaces in variable names and values by wrapping in double quotes like this");
		c->Message(Chat::White, "Example: #setentityvariable \"Test Variable\" \"Test Value\"");
		c->Message(Chat::White, "Note: Variable Value is optional and can be set to empty by not providing a value");
		return;
	}

	const std::string variable_name  = sep->arg[1];
	const std::string variable_value = sep->arg[2];

	Mob* t = c;
	if (c->GetTarget()) {
		t = c->GetTarget();
	}

	t->SetEntityVariable(variable_name, variable_value);

	c->Message(
		Chat::White,
		fmt::format(
			"Set an entity variable named '{}' to a value of '{}' for {}.",
			variable_name,
			variable_value,
			c->GetTargetDescription(t)
		).c_str()
	);
}
