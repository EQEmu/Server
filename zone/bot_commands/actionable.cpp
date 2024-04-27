#include "../bot_command.h"

void bot_command_actionable(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_actionable", sep->arg[0], "actionable")) {
		c->Message(Chat::White, "note: Lists actionable command arguments and use descriptions");
		return;
	}

	std::vector<std::string> description =
	{
		"Lists actionable command arguments and use descriptions."
	};

	std::vector<std::string> notes = 
	{ 
		"[target] - uses the command on the target. Some commands will default to target if no actionable is selected.",
		"[byname] [name] - selects a bot by name their name.",
		"[ownergroup] - selects all bots in the owner's group.",
		"[ownerraid] - selects all bots in the owner's raid.",
		"[targetgroup] - selects all bots in the target's group.",
		"[namesgroup] [name] - selects all bots in [name]'s group.",
		"[healrotation] [name] - selects all member and target bots of a heal rotation where [name] is a member.",
		"[healrotationmembers] [name] - selects all member bots of a heal rotation where [name] is a member.",
		"[healrotationtargets] [name] - selects all target bots of a heal rotation where [name] is a member.",
		"[mmr] - selects all bots that are currently at max melee range.",
		"[byclass] - selects all bots of the chosen class.",
		"[byrace] - selects all bots of the chosen race.",
		"[spawned] - selects all spawned bots.",
		"[all] - selects all spawned bots.",
		"<br>",
		"You may only select your bots as actionable"
	};

	std::vector<std::string> example_format = { };
	std::vector<std::string> examples_one = { };
	std::vector<std::string> examples_two = { };
	std::vector<std::string> examples_three = { };

	std::vector<std::string> actionables = { };

	std::vector<std::string> options = { };
	std::vector<std::string> options_one = { };
	std::vector<std::string> options_two = { };
	std::vector<std::string> options_three = { };

	std::string popup_text = c->SendCommandHelpWindow(
		c,
		description,
		notes,
		example_format,
		examples_one, examples_two, examples_three,
		actionables,
		options,
		options_one, options_two, options_three
	);

	popup_text = DialogueWindow::Table(popup_text);

	c->SendPopupToClient(sep->arg[0], popup_text.c_str());

	return;
}
