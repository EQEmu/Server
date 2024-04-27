#include "../bot_command.h"

void bot_command_max_melee_range(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_max_melee_range", sep->arg[0], "maxmeleerange")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Toggles whether or not bots will stay at max melee range during combat."
		};

		std::vector<std::string> notes = { };

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [value] [actionable]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To set BotA to stay at max melee range:",
			fmt::format(
				"{} 1 byname BotA",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_two = { };
		std::vector<std::string> examples_three =
		{
			"To check the max melee range status for all bots:",
			fmt::format(
				"{} current spawned",
				sep->arg[0]
			)
		};

		std::vector<std::string> actionables =
		{
			"target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned"
		};

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
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Use {} for information about race/class IDs.",
				Saylink::Silent("^classracelist")
			).c_str()
		);

		return;
	}

	std::string arg1 = sep->arg[1];

	if (!arg1.compare("listid") || !arg1.compare("listname")) {
		c->CastToBot()->SendSpellTypesWindow(c, sep->arg[0], sep->arg[1], sep->arg[2]);
		return;
	}

	int ab_arg = 1;
	bool current_check = false;
	uint32 typeValue = 0;

	if (sep->IsNumber(1)) {
		typeValue = atoi(sep->arg[1]);
		++ab_arg;
		if (typeValue < 0 || typeValue > 1) {
			c->Message(Chat::Yellow, "You must enter either 0 for disabled or 1 for enabled.");

			return;
		}
	}
	else if (!arg1.compare("current")) {
		++ab_arg;
		current_check = true;
	}
	else {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Incorrect argument, use {} for information regarding this command.",
				Saylink::Silent(
					fmt::format("{} help", sep->arg[0])
				)
			).c_str()
		);

		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;
	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.remove(nullptr);

	Bot* first_found = nullptr;
	int success_count = 0;
	for (auto my_bot : sbl) {
		if (!first_found) {
			first_found = my_bot;
		}
		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I {} stay at max melee range.'",
					my_bot->GetCleanName(),
					my_bot->GetMaxMeleeRange() ? "will" : "will not"
				).c_str()
			);
		}
		else {
			my_bot->SetMaxMeleeRange(typeValue);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I {} stay at max melee range.'",
					first_found->GetCleanName(),
					first_found->GetMaxMeleeRange() ? "will now" : "will no longer"
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots {} stay at max melee range.",
					success_count,
					typeValue ? "will now" : "will no longer"
				).c_str()
			);
		}
	}
}
