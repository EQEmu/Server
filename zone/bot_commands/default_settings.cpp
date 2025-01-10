#include "../bot_command.h"

void bot_command_default_settings(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_default_settings", sep->arg[0], "defaultsettings")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Restores a bot's setting(s) to defaults"
		};

		std::vector<std::string> notes =
		{ 
			"- You can put a spell type ID or shortname after any option except [all], [misc] and [spellsettings] to restore that specifc spell type only"
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [option] [actionable]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To restore delays for Clerics:",
			fmt::format(
				"{} delays byclass 2",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_two =
		{
			"To restore only Snare delays for BotA:",
			fmt::format(
				"{} delays {} byname BotA",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Snare)
			),
			fmt::format(
				"{} delays {} byname BotA",
				sep->arg[0],
				BotSpellTypes::Snare
			)
		};
		std::vector<std::string> examples_three = { };

		std::vector<std::string> actionables =
		{
			"target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned"
		};

		std::vector<std::string> options =
		{ 
			"all, misc, spellsettings, spelltypesettings, holds, delays, minthresholds, maxthresholds minmanapct, maxmanapct, minhppct, maxhppct, idlepriority, engagedpriority, pursuepriority, aggrocheck, targetcounts"
		};
		std::vector<std::string> options_one =
		{
			"[spellsettings] will restore ^spellsettings options",
			"[spelltypesettings] restores all spell type settings",
			"[all] restores all settings"
		};
		std::vector<std::string> options_two =
		{
			"[misc] restores all miscellaneous options such as:",
			"- ^showhelm, ^followd, ^stopmeleelevel",
			"- ^enforcespellsettings, ^bottoggleranged, ^petsettype",
			"- ^behindmob, ^distanceranged, ^illusionblock",
			"- ^sitincombat, ^sithppercent and ^sitmanapercent",

		};
		std::vector<std::string> options_three = 
		{ 
			"The remaining options restore that specific type"
		};

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

	int ab_arg = 2;
	bool valid_option = false;
	uint16 spell_type = UINT16_MAX;
	std::vector<std::string> options = 
	{ 
		"all",
		"misc"
		"spellsettings",
		"spelltypesettings",
		"holds",
		"delays",
		"minthresholds",
		"maxthresholds",
		"aggrocheck",
		"minmanapct",
		"maxmanapct",
		"minhppct",
		"maxhppct",
		"idlepriority",
		"engagedpriority",
		"pursuepriority",
		"targetcounts"
	};

	for (int i = 0; i < options.size(); i++) {
		if (sep->arg[1] == options[i]) {
			if (options[i] != "all" && options[i] != "misc" && options[i] != "spellsettings") {

				if (sep->IsNumber(2) || c->GetSpellTypeIDByShortName(sep->arg[2]) != UINT16_MAX) {
					if (sep->IsNumber(2)) {
						spell_type = atoi(sep->arg[2]);
					}

					if (c->GetSpellTypeIDByShortName(sep->arg[2]) != UINT16_MAX) {
						spell_type = c->GetSpellTypeIDByShortName(sep->arg[2]);
					}

					if (spell_type < BotSpellTypes::START || spell_type > BotSpellTypes::END) {
						c->Message(Chat::Yellow, "You must choose a valid spell type. Spell types range from %i to %i", BotSpellTypes::START, BotSpellTypes::END);

						return;
					}

					++ab_arg;
				}
			}
			else if (
				(options[i] == "all" || options[i] == "misc" || options[i] == "spellsettings") &&
				((sep->IsNumber(2) || c->GetSpellTypeIDByShortName(sep->arg[2]) != UINT16_MAX))
			) {
				break;
			}

			valid_option = true;
			break;
		}
	}

	if (!valid_option) {
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
	std::string output = "";
	uint8 bot_stance = 2;

	for (auto my_bot : sbl) {
		if (!first_found) {
			first_found = my_bot;
		}

		bot_stance = my_bot->GetBotStance();

		if (!strcasecmp(sep->arg[1], "misc")) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				my_bot->SetBotBaseSetting(i, my_bot->GetDefaultBotBaseSetting(i, bot_stance));
				output = "miscellanous settings";
			}
		}
		else if (!strcasecmp(sep->arg[1], "holds")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellHold(spell_type, my_bot->GetDefaultSpellHold(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellHold(i, my_bot->GetDefaultSpellHold(i, bot_stance));
				}
			}

			output = "hold settings";
		}
		else if (!strcasecmp(sep->arg[1], "delays")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellDelay(spell_type, my_bot->GetDefaultSpellDelay(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellDelay(i, my_bot->GetDefaultSpellDelay(i, bot_stance));
				}
			}

			output = "delay settings";
		}
		else if (!strcasecmp(sep->arg[1], "minthresholds")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellMinThreshold(spell_type, my_bot->GetDefaultSpellMinThreshold(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellMinThreshold(i, my_bot->GetDefaultSpellMinThreshold(i, bot_stance));
				}
			}

			output = "minimum threshold settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxthresholds")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellMaxThreshold(spell_type, my_bot->GetDefaultSpellMaxThreshold(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellMaxThreshold(i, my_bot->GetDefaultSpellMaxThreshold(i, bot_stance));
				}
			}

			output = "maximum threshold settings";
		}
		else if (!strcasecmp(sep->arg[1], "aggrochecks")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypeAggroCheck(spell_type, my_bot->GetDefaultSpellTypeAggroCheck(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeAggroCheck(i, my_bot->GetDefaultSpellTypeAggroCheck(i, bot_stance));
				}
			}

			output = "aggro check settings";
		}
		else if (!strcasecmp(sep->arg[1], "minmanapct")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypeMinManaLimit(spell_type, my_bot->GetDefaultSpellTypeMinManaLimit(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMinManaLimit(i, my_bot->GetDefaultSpellTypeMinManaLimit(i, bot_stance));
				}
			}

			output = "min mana settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxmanapct")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypeMaxManaLimit(spell_type, my_bot->GetDefaultSpellTypeMaxManaLimit(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMaxManaLimit(i, my_bot->GetDefaultSpellTypeMaxManaLimit(i, bot_stance));
				}
			}

			output = "max mana settings";
		}
		else if (!strcasecmp(sep->arg[1], "minhppct")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypeMinHPLimit(spell_type, my_bot->GetDefaultSpellTypeMinHPLimit(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMinHPLimit(i, my_bot->GetDefaultSpellTypeMinHPLimit(i, bot_stance));	
				}
			}

			output = "min hp settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxhppct")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypeMaxHPLimit(spell_type, my_bot->GetDefaultSpellTypeMaxHPLimit(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMaxHPLimit(i, my_bot->GetDefaultSpellTypeMaxHPLimit(i, bot_stance));
				}
			}

			output = "max hp settings";
		}
		else if (!strcasecmp(sep->arg[1], "idlepriority")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypePriority(spell_type, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(spell_type, BotPriorityCategories::Idle, my_bot->GetClass(), bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetClass(), bot_stance));
				}
			}

			output = "idle priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "engagedpriority")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypePriority(spell_type, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(spell_type, BotPriorityCategories::Engaged, my_bot->GetClass(), bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetClass(), bot_stance));
				}
			}

			output = "engaged priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "pursuepriority")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypePriority(spell_type, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(spell_type, BotPriorityCategories::Pursue, my_bot->GetClass(), bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetClass(), bot_stance));
				}
			}

			output = "pursue priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "targetcounts")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellTypeAEOrGroupTargetCount(spell_type, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeAEOrGroupTargetCount(i, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(i, bot_stance));
				}
			}

			output = "target count settings";
		}
		else if (!strcasecmp(sep->arg[1], "spellsettings")) {
			my_bot->ResetBotSpellSettings();
			output = "^spellsettings";
		}
		else if (!strcasecmp(sep->arg[1], "spelltypesettings")) {
			if (spell_type != UINT16_MAX) {
				my_bot->SetSpellHold(spell_type, my_bot->GetDefaultSpellHold(SpellType, bot_stance));
				my_bot->SetSpellDelay(spell_type, my_bot->GetDefaultSpellDelay(spell_type, bot_stance));
				my_bot->SetSpellMinThreshold(spell_type, my_bot->GetDefaultSpellMinThreshold(spell_type, bot_stance));
				my_bot->SetSpellMaxThreshold(spell_type, my_bot->GetDefaultSpellMaxThreshold(spell_type, bot_stance));
				my_bot->SetSpellTypeAggroCheck(spell_type, my_bot->GetDefaultSpellTypeAggroCheck(spell_type, bot_stance));
				my_bot->SetSpellTypeMinManaLimit(spell_type, my_bot->GetDefaultSpellTypeMinManaLimit(spell_type, bot_stance));
				my_bot->SetSpellTypeMaxManaLimit(spell_type, my_bot->GetDefaultSpellTypeMaxManaLimit(spell_type, bot_stance));
				my_bot->SetSpellTypeMinHPLimit(spell_type, my_bot->GetDefaultSpellTypeMinHPLimit(spell_type, bot_stance));
				my_bot->SetSpellTypeMaxHPLimit(spell_type, my_bot->GetDefaultSpellTypeMaxHPLimit(spell_type, bot_stance));
				my_bot->SetSpellTypePriority(spell_type, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(spell_type, BotPriorityCategories::Idle, my_bot->GetClass(), bot_stance));
				my_bot->SetSpellTypePriority(spell_type, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(spell_type, BotPriorityCategories::Engaged, my_bot->GetClass(), bot_stance));
				my_bot->SetSpellTypePriority(spell_type, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(spell_type, BotPriorityCategories::Pursue, my_bot->GetClass(), bot_stance));
				my_bot->SetSpellTypeAEOrGroupTargetCount(spell_type, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(spell_type, bot_stance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellHold(i, my_bot->GetDefaultSpellHold(i, bot_stance));
					my_bot->SetSpellDelay(i, my_bot->GetDefaultSpellDelay(i, bot_stance));
					my_bot->SetSpellMinThreshold(i, my_bot->GetDefaultSpellMinThreshold(i, bot_stance));
					my_bot->SetSpellMaxThreshold(i, my_bot->GetDefaultSpellMaxThreshold(i, bot_stance));
					my_bot->SetSpellTypeAggroCheck(i, my_bot->GetDefaultSpellTypeAggroCheck(i, bot_stance));
					my_bot->SetSpellTypeMinManaLimit(i, my_bot->GetDefaultSpellTypeMinManaLimit(i, bot_stance));
					my_bot->SetSpellTypeMaxManaLimit(i, my_bot->GetDefaultSpellTypeMaxManaLimit(i, bot_stance));
					my_bot->SetSpellTypeMinHPLimit(i, my_bot->GetDefaultSpellTypeMinHPLimit(i, bot_stance));
					my_bot->SetSpellTypeMaxHPLimit(i, my_bot->GetDefaultSpellTypeMaxHPLimit(i, bot_stance));
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetClass(), bot_stance));
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetClass(), bot_stance));
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetClass(), bot_stance));
					my_bot->SetSpellTypeAEOrGroupTargetCount(i, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(i, bot_stance));
				}
			}

			output = "spell type settings";
		}
		else if (!strcasecmp(sep->arg[1], "all")) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				my_bot->SetBotBaseSetting(i, my_bot->GetDefaultBotBaseSetting(i, bot_stance));
			}

			for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
				my_bot->SetSpellHold(i, my_bot->GetDefaultSpellHold(i, bot_stance));
				my_bot->SetSpellDelay(i, my_bot->GetDefaultSpellDelay(i, bot_stance));
				my_bot->SetSpellMinThreshold(i, my_bot->GetDefaultSpellMinThreshold(i, bot_stance));
				my_bot->SetSpellMaxThreshold(i, my_bot->GetDefaultSpellMaxThreshold(i, bot_stance));
				my_bot->SetSpellTypeAggroCheck(i, my_bot->GetDefaultSpellTypeAggroCheck(i, bot_stance));
				my_bot->SetSpellTypeMinManaLimit(i, my_bot->GetDefaultSpellTypeMinManaLimit(i, bot_stance));
				my_bot->SetSpellTypeMaxManaLimit(i, my_bot->GetDefaultSpellTypeMaxManaLimit(i, bot_stance));
				my_bot->SetSpellTypeMinHPLimit(i, my_bot->GetDefaultSpellTypeMinHPLimit(i, bot_stance));
				my_bot->SetSpellTypeMaxHPLimit(i, my_bot->GetDefaultSpellTypeMaxHPLimit(i, bot_stance));
				my_bot->SetSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetClass(), bot_stance));
				my_bot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetClass(), bot_stance));
				my_bot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetClass(), bot_stance));
				my_bot->SetSpellTypeAEOrGroupTargetCount(i, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(i, bot_stance));
			};

			my_bot->ResetBotSpellSettings();
			my_bot->ClearBotBlockedBuffs();
			
			my_bot->Save();

			output = "settings";

		}

		++success_count;
	}

	if (success_count == 1) {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} says, '{}{} were restored.'",				
				first_found->GetCleanName(),
				(
					spell_type != UINT16_MAX ?
					fmt::format("My [{}] ",
						c->GetSpellTypeNameByID(spell_type)
					)
					: "My "
				),
				output
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your bot's{}{} were restored.",
				success_count,
				(
					spell_type != UINT16_MAX ?
					fmt::format(" [{}] ",
						c->GetSpellTypeNameByID(spell_type)
					)
					: " "
				),
				output
			).c_str()
		);
	}
}
