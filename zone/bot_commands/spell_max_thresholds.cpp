#include "../bot_command.h"

void bot_command_spell_max_thresholds(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_spell_max_thresholds", sep->arg[0], "spellmaxthresholds")) {
		c->Message(Chat::White, "note: Controls at what target HP % the bot will start casting different spell types.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description = { "Controls at what target HP % the bot will start casting different spell types." };
		p.notes =
		{
			"- All pet types are based off the pet's owner's setting",
			"- Any remaining types use the owner's setting when a pet is the target",
			"- All Heals, Cures, Buffs (DS and resists included) are based off the target's setting, not the caster",
			"- e.g., BotA is healing BotB using BotB's settings",
		};
		p.example_format =
		{
			fmt::format("{} [Type Shortname] [value] [actionable]", sep->arg[0]),
			fmt::format("{} [Type ID] [value] [actionable]", sep->arg[0])
		};
		p.examples_one =
		{
			"To set all bots to start snaring at 99%:",
			fmt::format(
				"{} {} 99 spawned",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Snare)
			),
			fmt::format(
				"{} {} 99 spawned",
				sep->arg[0],
				BotSpellTypes::Snare
			)
		};
		p.examples_two =
		{
			"To set bot Enchbot to start casting Debuffs at 99%:",
			fmt::format(
				"{} {} 99 byname Enchbot",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Debuff)
			),
			fmt::format(
				"{} {} 99 byname Enchbot",
				sep->arg[0],
				BotSpellTypes::Debuff
			)
		};
		p.examples_three =
		{
			"To check the current Nuke max threshold on all bots:",
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} {} current spawned",
				sep->arg[0],
				BotSpellTypes::Nuke
			)
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };

		std::string popup_text = c->SendBotCommandHelpWindow(p);
		popup_text = DialogueWindow::Table(popup_text);

		c->SendPopupToClient(sep->arg[0], popup_text.c_str());
		c->SendSpellTypePrompts();

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
	std::string arg2 = sep->arg[2];
	int ab_arg = 2;
	bool current_check = false;
	uint16 spell_type = 0;
	uint32 type_value = 0;

	// String/Int type checks
	if (sep->IsNumber(1)) {
		spell_type = atoi(sep->arg[1]);

		if (spell_type < BotSpellTypes::START || spell_type > BotSpellTypes::END) {
			c->Message(Chat::Yellow, "You must choose a valid spell type. Spell types range from %i to %i", BotSpellTypes::START, BotSpellTypes::END);

			return;
		}
	}
	else {
		if (c->GetSpellTypeIDByShortName(arg1) != UINT16_MAX) {
			spell_type = c->GetSpellTypeIDByShortName(arg1);
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
		type_value = atoi(sep->arg[2]);
		++ab_arg;
		if (type_value < 0 || type_value > 100) {
			c->Message(Chat::Yellow, "You must enter a value between 0-100 (0%% to 100%% of health).");

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
					"{} says, 'My [{}] maximum threshold is currently [{}%%].'",
					my_bot->GetCleanName(),
					c->GetSpellTypeNameByID(spell_type),
					my_bot->GetSpellMaxThreshold(spell_type)
				).c_str()
			);
		}
		else {
			my_bot->SetSpellMaxThreshold(spell_type, type_value);
			++success_count;
		}
	}
	if (!current_check) {
		if (success_count == 1 && first_found) {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'My [{}] maximum threshold was set to [{}%%].'",
					first_found->GetCleanName(),
					c->GetSpellTypeNameByID(spell_type),
					first_found->GetSpellMaxThreshold(spell_type)
				).c_str()
			);
		}
		else {
			c->Message(
				Chat::Green,
				fmt::format(
					"{} of your bots set their [{}] maximum threshold to [{}%%].",
					success_count,
					c->GetSpellTypeNameByID(spell_type),
					type_value
				).c_str()
			);
		}
	}
}
