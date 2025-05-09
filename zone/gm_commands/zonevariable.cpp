#include "../client.h"

void command_zonevariable(Client *c, const Seperator *sep)
{
	const auto arguments = sep->argnum;

	if (!arguments) {
		c->Message(Chat::White, "Usage: #zonevariable clear - Clear all zone variables");
		c->Message(Chat::White, "Usage: #zonevariable delete [Variable Name] - Delete a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable set [Variable Name] [Variable Value] - Set a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable view [Variable Name] - View a zone variable");
		c->Message(Chat::White, "Note: You can have spaces in variable names and values by wrapping in double quotes like this");
		c->Message(Chat::White, "Example: #zonevariable set \"Test Variable\" \"Test Value\"");
		c->Message(Chat::White, "Note: Variable Value is optional and can be set to empty by not providing a value");
		return;
	}

	const bool is_clear  = !strcasecmp(sep->arg[1], "clear");
	const bool is_delete = !strcasecmp(sep->arg[1], "delete");
	const bool is_set    = !strcasecmp(sep->arg[1], "set");
	const bool is_view   = !strcasecmp(sep->arg[1], "view");

	if (
		!is_clear &&
		!is_delete &&
		!is_set &&
		!is_view
	) {
		c->Message(Chat::White, "Usage: #zonevariable clear - Clear all zone variables");
		c->Message(Chat::White, "Usage: #zonevariable delete [Variable Name] - Delete a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable set [Variable Name] [Variable Value] - Set a zone variable");
		c->Message(Chat::White, "Usage: #zonevariable view [Variable Name] - View a zone variable");
		c->Message(Chat::White, "Note: You can have spaces in variable names and values by wrapping in double quotes like this");
		c->Message(Chat::White, "Example: #zonevariable set \"Test Variable\" \"Test Value\"");
		c->Message(Chat::White, "Note: Variable Value is optional and can be set to empty by not providing a value");
		return;
	}

	if (is_clear) {
		const bool cleared = zone->ClearVariables();

		if (!cleared) {
			c->Message(Chat::White, "There are no zone variables to clear.");
			return;
		}

		c->Message(Chat::White, "Cleared all zone variables.");
	} else if (is_delete) {
		const std::string variable_name = sep->argplus[2];

		if (!zone->VariableExists(variable_name)) {
			c->Message(
				Chat::White,
				fmt::format(
					"A zone variable named '{}' does not exist.",
					variable_name
				).c_str()
			);

			return;
		}

		zone->DeleteVariable(variable_name);

		c->Message(
			Chat::White,
			fmt::format(
				"Deleted a zone variable named '{}'.",
				variable_name
			).c_str()
		);

		return;
	} else if (is_set) {
		const std::string variable_name  = sep->arg[2];
		const std::string variable_value = sep->arg[3];

		zone->SetVariable(variable_name, variable_value);

		c->Message(
			Chat::White,
			fmt::format(
				"Set a zone variable named '{}' to a value of '{}'.",
				variable_name,
				variable_value
			).c_str()
		);
	} else if (is_view) {
		const auto &l = zone->GetVariables();

		uint32 variable_count  = 0;
		uint32 variable_number = 1;

		const std::string search_criteria = arguments >= 2 ? sep->argplus[2] : "";

		for (const auto &e: l) {
			if (
				search_criteria.empty() ||
				Strings::Contains(Strings::ToLower(e), Strings::ToLower(search_criteria))
			) {
				c->Message(
					Chat::White,
					fmt::format(
						"Variable {} | Name: {} Value: {} | {}",
						variable_number,
						e,
						zone->GetVariable(e),
						Saylink::Silent(
							fmt::format(
								"#zonevariable delete {}",
								e
							),
							"Delete"
						)
					).c_str()
				);

				variable_count++;
				variable_number++;
			}
		}

		if (!variable_count) {
			c->Message(
				Chat::White,
				fmt::format(
					"There are no zone variables{}.",
					(
						!search_criteria.empty() ?
						fmt::format(
							" matching '{}'",
							search_criteria
						) :
						""
					)
				).c_str()
			);

			return;
		}

		c->Message(
			Chat::White,
			fmt::format(
				"There {} {} zone variable{}{}, would you like to {} zone variables?",
				variable_count != 1 ? "are" : "is",
				variable_count,
				variable_count != 1 ? "s" : "",
				(
					!search_criteria.empty() ?
					fmt::format(
						" matching '{}'",
						search_criteria
					) :
					""
				),
				Saylink::Silent(
					"#zonevariable clear",
					"clear"
				)
			).c_str()
		);
	}
}
