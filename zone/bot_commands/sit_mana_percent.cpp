#include "../bot_command.h"

void bot_command_sit_mana_percent(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_sit_mana_percent", sep->arg[0], "sitmanapercent")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Mana % threshold when bots will sit in combat if allowed."
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
			"To set Clerics to sit at 45% mana:",
			fmt::format(
				"{} 45 byclass 2",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_two = { };
		std::vector<std::string> examples_three =
		{
			"To check the mana threshold for all bots:",
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

		if (RuleB(Bots, SendClassRaceOnHelp)) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Use {} for information about race/class IDs.",
					Saylink::Silent("^classracelist")
				).c_str()
			);
		}

		return;
	}

	std::string arg1 = sep->arg[1];

	int ab_arg = 1;
	bool current_check = false;
	uint32 type_value = 0;

	if (sep->IsNumber(1)) {
		type_value = atoi(sep->arg[1]);
		++ab_arg;
		if (type_value < 0 || type_value > 100) {
			c->Message(Chat::Yellow, "You must enter a value between 0-100 (0%% to 100%% of health).");

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

	std::vector<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[ab_arg], sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	Bot* first_found = nullptr;
	int success_count = 0;
	for (auto my_bot : sbl) {
		if (my_bot->BotPassiveCheck()) {
			continue;
		}

		if (!first_found) {
			first_found = my_bot;
		}

		if (current_check) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I sit in combat whem at or below [{}%%] mana.'",
					my_bot->GetCleanName(),
					my_bot->GetSitManaPct()
				).c_str()
			);
		}
		else {
			my_bot->SetSitManaPct(type_value);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I will now sit in combat whem at or below [{}%%] mana.'",
					first_found->GetCleanName(),
					first_found->GetSitManaPct()
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots will now sit in combat whem at or below [{}%%] mana.'",
					success_count,
					type_value
				).c_str()
			);
		}
	}
}