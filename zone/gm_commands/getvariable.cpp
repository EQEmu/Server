#include "../client.h"

void command_getvariable(Client *c, const Seperator *sep)
{
	std::string variable = sep->argplus[1];
	if (variable.empty()) {
		c->Message(Chat::White, "Usage: #getvariable [Variable Name]");
		return;
	}

	std::string message;
	std::string value;
	if (database.GetVariable(variable, value)) {
		message = fmt::format(
			"Variable {}: {}",
			variable,
			value
		);
	} else {
		message = fmt::format(
			"Variable '{}' does not exist.",
			variable
		);
	}

	c->Message(
		Chat::White,
		message.c_str()
	);
}

