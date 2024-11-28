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
			"target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets mmr, byclass, byrace, spawned"
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
	bool validOption = false;
	uint16 spellType = UINT16_MAX;
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
						spellType = atoi(sep->arg[2]);
					}

					if (c->GetSpellTypeIDByShortName(sep->arg[2]) != UINT16_MAX) {
						spellType = c->GetSpellTypeIDByShortName(sep->arg[2]);
					}

					if (spellType < BotSpellTypes::START || spellType > BotSpellTypes::END) {
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

			validOption = true;
			break;
		}
	}

	if (!validOption) {
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
	std::string output = "";
	uint8 botStance = 2;

	for (auto myBot : sbl) {
		if (!first_found) {
			first_found = myBot;
		}

		botStance = myBot->GetBotStance();

		if (!strcasecmp(sep->arg[1], "misc")) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				myBot->SetBotBaseSetting(i, myBot->GetDefaultBotBaseSetting(i, botStance));
				output = "miscellanous settings";
			}
		}
		else if (!strcasecmp(sep->arg[1], "holds")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellHold(spellType, myBot->GetDefaultSpellHold(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellHold(i, myBot->GetDefaultSpellHold(i, botStance));
				}
			}

			output = "hold settings";
		}
		else if (!strcasecmp(sep->arg[1], "delays")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellDelay(spellType, myBot->GetDefaultSpellDelay(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellDelay(i, myBot->GetDefaultSpellDelay(i, botStance));
				}
			}

			output = "delay settings";
		}
		else if (!strcasecmp(sep->arg[1], "minthresholds")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellMinThreshold(spellType, myBot->GetDefaultSpellMinThreshold(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellMinThreshold(i, myBot->GetDefaultSpellMinThreshold(i, botStance));
				}
			}

			output = "minimum threshold settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxthresholds")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellMaxThreshold(spellType, myBot->GetDefaultSpellMaxThreshold(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellMaxThreshold(i, myBot->GetDefaultSpellMaxThreshold(i, botStance));
				}
			}

			output = "maximum threshold settings";
		}
		else if (!strcasecmp(sep->arg[1], "aggrochecks")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypeAggroCheck(spellType, myBot->GetDefaultSpellTypeAggroCheck(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypeAggroCheck(i, myBot->GetDefaultSpellTypeAggroCheck(i, botStance));
				}
			}

			output = "aggro check settings";
		}
		else if (!strcasecmp(sep->arg[1], "minmanapct")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypeMinManaLimit(spellType, myBot->GetDefaultSpellTypeMinManaLimit(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypeMinManaLimit(i, myBot->GetDefaultSpellTypeMinManaLimit(i, botStance));
				}
			}

			output = "min mana settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxmanapct")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypeMaxManaLimit(spellType, myBot->GetDefaultSpellTypeMaxManaLimit(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypeMaxManaLimit(i, myBot->GetDefaultSpellTypeMaxManaLimit(i, botStance));
				}
			}

			output = "max mana settings";
		}
		else if (!strcasecmp(sep->arg[1], "minhppct")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypeMinHPLimit(spellType, myBot->GetDefaultSpellTypeMinHPLimit(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypeMinHPLimit(i, myBot->GetDefaultSpellTypeMinHPLimit(i, botStance));	
				}
			}

			output = "min hp settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxhppct")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypeMaxHPLimit(spellType, myBot->GetDefaultSpellTypeMaxHPLimit(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypeMaxHPLimit(i, myBot->GetDefaultSpellTypeMaxHPLimit(i, botStance));
				}
			}

			output = "max hp settings";
		}
		else if (!strcasecmp(sep->arg[1], "idlepriority")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypePriority(spellType, BotPriorityCategories::Idle, myBot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Idle, myBot->GetClass(), botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypePriority(i, BotPriorityCategories::Idle, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, myBot->GetClass(), botStance));
				}
			}

			output = "idle priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "engagedpriority")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypePriority(spellType, BotPriorityCategories::Engaged, myBot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Engaged, myBot->GetClass(), botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, myBot->GetClass(), botStance));
				}
			}

			output = "engaged priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "pursuepriority")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellTypePriority(spellType, BotPriorityCategories::Pursue, myBot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Pursue, myBot->GetClass(), botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, myBot->GetClass(), botStance));
				}
			}

			output = "pursue priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "targetcounts")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellDelay(spellType, myBot->GetDefaultSpellDelay(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellTypeAEOrGroupTargetCount(i, myBot->GetDefaultSpellTypeAEOrGroupTargetCount(i, botStance));
				}
			}

			output = "ae/group count settings";
		}
		else if (!strcasecmp(sep->arg[1], "spellsettings")) {
			myBot->ResetBotSpellSettings();
			output = "^spellsettings";
		}
		else if (!strcasecmp(sep->arg[1], "spelltypesettings")) {
			if (spellType != UINT16_MAX) {
				myBot->SetSpellHold(spellType, myBot->GetDefaultSpellHold(spellType, botStance));
				myBot->SetSpellDelay(spellType, myBot->GetDefaultSpellDelay(spellType, botStance));
				myBot->SetSpellMinThreshold(spellType, myBot->GetDefaultSpellMinThreshold(spellType, botStance));
				myBot->SetSpellMaxThreshold(spellType, myBot->GetDefaultSpellMaxThreshold(spellType, botStance));
				myBot->SetSpellTypeAggroCheck(spellType, myBot->GetDefaultSpellTypeAggroCheck(spellType, botStance));
				myBot->SetSpellTypeMinManaLimit(spellType, myBot->GetDefaultSpellTypeMinManaLimit(spellType, botStance));
				myBot->SetSpellTypeMaxManaLimit(spellType, myBot->GetDefaultSpellTypeMaxManaLimit(spellType, botStance));
				myBot->SetSpellTypeMinHPLimit(spellType, myBot->GetDefaultSpellTypeMinHPLimit(spellType, botStance));
				myBot->SetSpellTypeMaxHPLimit(spellType, myBot->GetDefaultSpellTypeMaxHPLimit(spellType, botStance));
				myBot->SetSpellTypePriority(spellType, BotPriorityCategories::Idle, myBot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Idle, myBot->GetClass(), botStance));
				myBot->SetSpellTypePriority(spellType, BotPriorityCategories::Engaged, myBot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Engaged, myBot->GetClass(), botStance));
				myBot->SetSpellTypePriority(spellType, BotPriorityCategories::Pursue, myBot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Pursue, myBot->GetClass(), botStance));
				myBot->SetSpellTypeAEOrGroupTargetCount(spellType, myBot->GetDefaultSpellTypeAEOrGroupTargetCount(spellType, botStance));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					myBot->SetSpellHold(i, myBot->GetDefaultSpellHold(i, botStance));
					myBot->SetSpellDelay(i, myBot->GetDefaultSpellDelay(i, botStance));
					myBot->SetSpellMinThreshold(i, myBot->GetDefaultSpellMinThreshold(i, botStance));
					myBot->SetSpellMaxThreshold(i, myBot->GetDefaultSpellMaxThreshold(i, botStance));
					myBot->SetSpellTypeAggroCheck(i, myBot->GetDefaultSpellTypeAggroCheck(i, botStance));
					myBot->SetSpellTypeMinManaLimit(i, myBot->GetDefaultSpellTypeMinManaLimit(i, botStance));
					myBot->SetSpellTypeMaxManaLimit(i, myBot->GetDefaultSpellTypeMaxManaLimit(i, botStance));
					myBot->SetSpellTypeMinHPLimit(i, myBot->GetDefaultSpellTypeMinHPLimit(i, botStance));
					myBot->SetSpellTypeMaxHPLimit(i, myBot->GetDefaultSpellTypeMaxHPLimit(i, botStance));
					myBot->SetSpellTypePriority(i, BotPriorityCategories::Idle, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, myBot->GetClass(), botStance));
					myBot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, myBot->GetClass(), botStance));
					myBot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, myBot->GetClass(), botStance));
					myBot->SetSpellTypeAEOrGroupTargetCount(i, myBot->GetDefaultSpellTypeAEOrGroupTargetCount(i, botStance));
				}
			}

			output = "spell type settings";
		}
		else if (!strcasecmp(sep->arg[1], "all")) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				myBot->SetBotBaseSetting(i, myBot->GetDefaultBotBaseSetting(i, botStance));
			}

			for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
				myBot->SetSpellHold(i, myBot->GetDefaultSpellHold(i, botStance));
				myBot->SetSpellDelay(i, myBot->GetDefaultSpellDelay(i, botStance));
				myBot->SetSpellMinThreshold(i, myBot->GetDefaultSpellMinThreshold(i, botStance));
				myBot->SetSpellMaxThreshold(i, myBot->GetDefaultSpellMaxThreshold(i, botStance));
				myBot->SetSpellTypeAggroCheck(i, myBot->GetDefaultSpellTypeAggroCheck(i, botStance));
				myBot->SetSpellTypeMinManaLimit(i, myBot->GetDefaultSpellTypeMinManaLimit(i, botStance));
				myBot->SetSpellTypeMaxManaLimit(i, myBot->GetDefaultSpellTypeMaxManaLimit(i, botStance));
				myBot->SetSpellTypeMinHPLimit(i, myBot->GetDefaultSpellTypeMinHPLimit(i, botStance));
				myBot->SetSpellTypeMaxHPLimit(i, myBot->GetDefaultSpellTypeMaxHPLimit(i, botStance));
				myBot->SetSpellTypePriority(i, BotPriorityCategories::Idle, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, myBot->GetClass(), botStance));
				myBot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, myBot->GetClass(), botStance));
				myBot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, myBot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, myBot->GetClass(), botStance));
				myBot->SetSpellTypeAEOrGroupTargetCount(i, myBot->GetDefaultSpellTypeAEOrGroupTargetCount(i, botStance));
			};

			myBot->ResetBotSpellSettings();

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
					spellType != UINT16_MAX ?
					fmt::format("My [{}] ",
						c->GetSpellTypeNameByID(spellType)
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
					spellType != UINT16_MAX ?
					fmt::format(" [{}] ",
						c->GetSpellTypeNameByID(spellType)
					)
					: " "
				),
				output
			).c_str()
		);
	}
}
