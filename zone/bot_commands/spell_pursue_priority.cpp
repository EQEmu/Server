#include "../bot_command.h"

void bot_command_spell_pursue_priority(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_pursue_priority", sep->arg[0], "spellpursuepriority")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Sets the order of spell casts when the mob is fleeing in combat by spell type"
		};

		std::vector<std::string> notes =
		{
			"- Setting a spell type to 0 will prevent that type from being cast.",
			"- If 2 or more are set to the same priority they will sort by spell type ID."
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [Type Shortname] [value] [actionable]"
				, sep->arg[0]
			),
			fmt::format(
				"{} [Type ID] [value] [actionable]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To set all bots to cast nukes first:",
			fmt::format(
				"{} {} 1 spawned",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} {} 1 spawned",
				sep->arg[0],
				BotSpellTypes::FastHeals
			)
		};
		std::vector<std::string> examples_two =
		{
			"To set all Shaman to not cast cures:",
			fmt::format(
				"{} {} 0 byclass 10",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Cure)
			),
			fmt::format(
				"{} {} 0 byclass 10",
				sep->arg[0],
				BotSpellTypes::Cure
			)
		};
		std::vector<std::string> examples_three =
		{
			"To check the current pursue priority of buffs on all bots:",
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Buff)
			),
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				BotSpellTypes::Buff
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
		c->CastToBot()->SendSpellTypesWindow(c, sep->arg[0], "", "", true);
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

	std::string arg2 = sep->arg[2];
	int ab_arg = 2;
	bool current_check = false;
	uint16 spellType = 0;
	uint32 typeValue = 0;

	// String/Int type checks
	if (sep->IsNumber(1)) {
		spellType = atoi(sep->arg[1]);

		if (spellType < BotSpellTypes::START || spellType > BotSpellTypes::END) {
			c->Message(Chat::Yellow, "You must choose a valid spell type. Spell types range from %i to %i", BotSpellTypes::START, BotSpellTypes::END);

			return;
		}
	}
	else {
		if (c->GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
			spellType = c->GetSpellTypeIDByShortName(arg1);
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
	}

	if (sep->IsNumber(2)) {
		typeValue = atoi(sep->arg[2]);
		++ab_arg;
		if (typeValue < 0 || typeValue > 100) {
			c->Message(Chat::Yellow, "You must enter a value between 0-100.");

			return;
		}
	}
	else if (!arg2.compare("current")) {
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
					"{} says, 'My [{}] pursue cast priority is currently [{}].'",
					my_bot->GetCleanName(),
					c->GetSpellTypeNameByID(spellType),
					my_bot->GetSpellTypePriority(spellType, BotPriorityCategories::Pursue)
				).c_str()
			);
		}
		else {
			my_bot->SetSpellTypePriority(spellType, BotPriorityCategories::Pursue, typeValue);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My [{}] pursue cast priority was set to [{}].'",
					first_found->GetCleanName(),
					c->GetSpellTypeNameByID(spellType),
					first_found->GetSpellTypePriority(spellType, BotPriorityCategories::Pursue)
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots set their [{}] pursue cast priority to [{}].",
					success_count,
					c->GetSpellTypeNameByID(spellType),
					typeValue
				).c_str()
			);
		}
	}
}
