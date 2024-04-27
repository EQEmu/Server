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
			"- ^behindmob, ^casterrange, ^illusionblock",
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
	for (auto my_bot : sbl) {
		if (!first_found) {
			first_found = my_bot;
		}

		if (!strcasecmp(sep->arg[1], "misc")) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				my_bot->SetBotBaseSetting(i, my_bot->GetDefaultBotBaseSetting(i));
				output = "miscellanous settings";
			}
		}
		else if (!strcasecmp(sep->arg[1], "holds")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellHold(spellType, my_bot->GetDefaultSpellHold(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellHold(i, my_bot->GetDefaultSpellHold(i));
				}
			}

			output = "hold settings";
		}
		else if (!strcasecmp(sep->arg[1], "delays")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellDelay(spellType, my_bot->GetDefaultSpellDelay(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellDelay(i, my_bot->GetDefaultSpellDelay(i));
				}
			}

			output = "delay settings";
		}
		else if (!strcasecmp(sep->arg[1], "minthresholds")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellMinThreshold(spellType, my_bot->GetDefaultSpellMinThreshold(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellMinThreshold(i, my_bot->GetDefaultSpellMinThreshold(i));
				}
			}

			output = "minimum threshold settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxthresholds")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellMaxThreshold(spellType, my_bot->GetDefaultSpellMaxThreshold(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellMaxThreshold(i, my_bot->GetDefaultSpellMaxThreshold(i));
				}
			}

			output = "maximum threshold settings";
		}
		else if (!strcasecmp(sep->arg[1], "aggrochecks")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypeAggroCheck(spellType, my_bot->GetDefaultSpellTypeAggroCheck(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeAggroCheck(i, my_bot->GetDefaultSpellTypeAggroCheck(i));
				}
			}

			output = "aggro check settings";
		}
		else if (!strcasecmp(sep->arg[1], "minmanapct")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypeMinManaLimit(spellType, my_bot->GetDefaultSpellTypeMinManaLimit(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMinManaLimit(i, my_bot->GetDefaultSpellTypeMinManaLimit(i));
				}
			}

			output = "min mana settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxmanapct")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypeMaxManaLimit(spellType, my_bot->GetDefaultSpellTypeMaxManaLimit(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMaxManaLimit(i, my_bot->GetDefaultSpellTypeMaxManaLimit(i));
				}
			}

			output = "max mana settings";
		}
		else if (!strcasecmp(sep->arg[1], "minhppct")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypeMinHPLimit(spellType, my_bot->GetDefaultSpellTypeMinHPLimit(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMinHPLimit(i, my_bot->GetDefaultSpellTypeMinHPLimit(i));	
				}
			}

			output = "min hp settings";
		}
		else if (!strcasecmp(sep->arg[1], "maxhppct")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypeMaxHPLimit(spellType, my_bot->GetDefaultSpellTypeMaxHPLimit(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeMaxHPLimit(i, my_bot->GetDefaultSpellTypeMaxHPLimit(i));
				}
			}

			output = "max hp settings";
		}
		else if (!strcasecmp(sep->arg[1], "idlepriority")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypePriority(spellType, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Idle, my_bot->GetClass()));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetClass()));
				}
			}

			output = "idle priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "engagedpriority")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypePriority(spellType, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Engaged, my_bot->GetClass()));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetClass()));
				}
			}

			output = "engaged priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "pursuepriority")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellTypePriority(spellType, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Pursue, my_bot->GetClass()));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetClass()));
				}
			}

			output = "pursue priority settings";
		}
		else if (!strcasecmp(sep->arg[1], "targetcounts")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellDelay(spellType, my_bot->GetDefaultSpellDelay(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellTypeAEOrGroupTargetCount(i, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(i));
				}
			}

			output = "ae/group count settings";
		}
		else if (!strcasecmp(sep->arg[1], "spellsettings")) {
			my_bot->ResetBotSpellSettings();
			output = "^spellsettings";
		}
		else if (!strcasecmp(sep->arg[1], "spelltypesettings")) {
			if (spellType != UINT16_MAX) {
				my_bot->SetSpellHold(spellType, my_bot->GetDefaultSpellHold(spellType));
				my_bot->SetSpellDelay(spellType, my_bot->GetDefaultSpellDelay(spellType));
				my_bot->SetSpellMinThreshold(spellType, my_bot->GetDefaultSpellMinThreshold(spellType));
				my_bot->SetSpellMaxThreshold(spellType, my_bot->GetDefaultSpellMaxThreshold(spellType));
				my_bot->SetSpellTypeAggroCheck(spellType, my_bot->GetDefaultSpellTypeAggroCheck(spellType));
				my_bot->SetSpellTypeMinManaLimit(spellType, my_bot->GetDefaultSpellTypeMinManaLimit(spellType));
				my_bot->SetSpellTypeMaxManaLimit(spellType, my_bot->GetDefaultSpellTypeMaxManaLimit(spellType));
				my_bot->SetSpellTypeMinHPLimit(spellType, my_bot->GetDefaultSpellTypeMinHPLimit(spellType));
				my_bot->SetSpellTypeMaxHPLimit(spellType, my_bot->GetDefaultSpellTypeMaxHPLimit(spellType));
				my_bot->SetSpellTypePriority(spellType, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Idle, my_bot->GetClass()));
				my_bot->SetSpellTypePriority(spellType, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Engaged, my_bot->GetClass()));
				my_bot->SetSpellTypePriority(spellType, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(spellType, BotPriorityCategories::Pursue, my_bot->GetClass()));
				my_bot->SetSpellTypeAEOrGroupTargetCount(spellType, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(spellType));
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					my_bot->SetSpellHold(i, my_bot->GetDefaultSpellHold(i));
					my_bot->SetSpellDelay(i, my_bot->GetDefaultSpellDelay(i));
					my_bot->SetSpellMinThreshold(i, my_bot->GetDefaultSpellMinThreshold(i));
					my_bot->SetSpellMaxThreshold(i, my_bot->GetDefaultSpellMaxThreshold(i));
					my_bot->SetSpellTypeAggroCheck(i, my_bot->GetDefaultSpellTypeAggroCheck(i));
					my_bot->SetSpellTypeMinManaLimit(i, my_bot->GetDefaultSpellTypeMinManaLimit(i));
					my_bot->SetSpellTypeMaxManaLimit(i, my_bot->GetDefaultSpellTypeMaxManaLimit(i));
					my_bot->SetSpellTypeMinHPLimit(i, my_bot->GetDefaultSpellTypeMinHPLimit(i));
					my_bot->SetSpellTypeMaxHPLimit(i, my_bot->GetDefaultSpellTypeMaxHPLimit(i));
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetClass()));
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetClass()));
					my_bot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetClass()));
					my_bot->SetSpellTypeAEOrGroupTargetCount(i, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(i));
				}
			}

			output = "spell type settings";
		}
		else if (!strcasecmp(sep->arg[1], "all")) {
			for (uint16 i = BotBaseSettings::START; i <= BotBaseSettings::END; ++i) {
				my_bot->SetBotBaseSetting(i, my_bot->GetDefaultBotBaseSetting(i));
			}

			for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
				my_bot->SetSpellHold(i, my_bot->GetDefaultSpellHold(i));
				my_bot->SetSpellDelay(i, my_bot->GetDefaultSpellDelay(i));
				my_bot->SetSpellMinThreshold(i, my_bot->GetDefaultSpellMinThreshold(i));
				my_bot->SetSpellMaxThreshold(i, my_bot->GetDefaultSpellMaxThreshold(i));
				my_bot->SetSpellTypeAggroCheck(i, my_bot->GetDefaultSpellTypeAggroCheck(i));
				my_bot->SetSpellTypeMinManaLimit(i, my_bot->GetDefaultSpellTypeMinManaLimit(i));
				my_bot->SetSpellTypeMaxManaLimit(i, my_bot->GetDefaultSpellTypeMaxManaLimit(i));
				my_bot->SetSpellTypeMinHPLimit(i, my_bot->GetDefaultSpellTypeMinHPLimit(i));
				my_bot->SetSpellTypeMaxHPLimit(i, my_bot->GetDefaultSpellTypeMaxHPLimit(i));
				my_bot->SetSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Idle, my_bot->GetClass()));
				my_bot->SetSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Engaged, my_bot->GetClass()));
				my_bot->SetSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetDefaultSpellTypePriority(i, BotPriorityCategories::Pursue, my_bot->GetClass()));
				my_bot->SetSpellTypeAEOrGroupTargetCount(i, my_bot->GetDefaultSpellTypeAEOrGroupTargetCount(i));
			};

			my_bot->ResetBotSpellSettings();

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
