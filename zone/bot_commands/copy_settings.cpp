#include "../bot_command.h"

void bot_command_copy_settings(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_copy_settings", sep->arg[0], "copysettings")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Copies settings from one bot to another bot"
		};

		std::vector<std::string> notes =
		{
			"- You can put a spell type ID or shortname after any option except [all], [misc] and [spellsettings] to restore that specifc spell type only"
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [from] [to] [option]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To copy all settings from BotA to BotB:",
			fmt::format(
				"{} BotA BotB all",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_two =
		{
			"To copy only Nuke spelltypesettings from BotA to BotB:",
			fmt::format(
				"{} BotA BotB spelltypesettings {}",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} BotA BotB spelltypesettings {}",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
		};
		std::vector<std::string> examples_three = { };

		std::vector<std::string> actionables =
		{
			"target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned"
		};

		std::vector<std::string> options =
		{
			"[all], [misc], [spellsettings], [spelltypesettings], [spellholds], [spelldelays], [spellminthresholds], [spellmaxthresholds], [spellminmanapct], [spellmaxmanapct], [spellminhppct], [spellmaxhppct], [spellidlepriority], [spellengagedpriority], [spellpursuepriority], [spellaggrochecks], [spelltargetcounts], [sithppercent], [sitmanapercent], [blockedbuffs], [blockedpetbuffs]"
		};
		std::vector<std::string> options_one =
		{
			"[spellsettings] will copy ^spellsettings options",
			"[spelltypesettings] copies all spell type settings",
			"[all] copies all settings"
		};
		std::vector<std::string> options_two =
		{
			"[misc] copies all miscellaneous options such as:",
			"- ^showhelm, ^followd, ^stopmeleelevel, ^enforcespellsettings, ^bottoggleranged, ^petsettype, ^behindmob, ^distanceranged, ^illusionblock, ^sitincombat, ^sithppercent, ^sitmanapercent, ^blockedbuffs, ^blockedpetbuffs",

		};
		std::vector<std::string> options_three =
		{
			"The remaining options copy that specific type"
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
	uint16 setting_type = UINT16_MAX;
	std::vector<std::string> options =
	{
		"all",
		"misc",
		"spellsettings",
		"spelltypesettings",
		"spellholds",
		"spelldelays",
		"spellminthresholds",
		"spellmaxthresholds",
		"spellminmanapct",
		"spellmaxmanapct",
		"spellminhppct",
		"spellmaxhppct",
		"spellidlepriority",
		"spellengagedpriority",
		"spellpursuepriority",
		"spellaggrochecks",
		"spelltargetcounts",
		"blockedbuffs",
		"blockedpetbuffs"
	};

	if (sep->IsNumber(4)) {
		spell_type = atoi(sep->arg[4]);
	}
	else {
		spell_type = c->GetSpellTypeIDByShortName(sep->arg[4]);
	}

	if (spell_type < BotSpellTypes::START || spell_type > BotSpellTypes::END) {
		spell_type = UINT16_MAX;
	}

	for (int i = 0; i < options.size(); i++) {
		if (sep->arg[3] == options[i]) {
			setting_type = c->GetBotSpellCategoryIDByShortName(sep->arg[3]);
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

	auto from = entity_list.GetBotByBotName(sep->arg[1]);

	if (!from) {
		c->Message(Chat::Yellow, "Could not find %s.", sep->arg[1]);
		return;
	}

	if (!from->IsBot()) {
		c->Message(Chat::Yellow, "%s is not a bot.", from->GetCleanName());
		return;
	}

	if (!from->GetOwner()) {
		c->Message(Chat::Yellow, "Could not find %s's owner.", from->GetCleanName());
	}

	if (RuleB(Bots, CopySettingsOwnBotsOnly) && from->GetOwner() != c) {
		c->Message(Chat::Yellow, "You name a bot you own to use this command.");
		return;
	}

	if (!RuleB(Bots, AllowCopySettingsAnon) && from->GetOwner() != c && from->GetOwner()->CastToClient()->GetAnon()) {
		c->Message(Chat::Yellow, "You name a bot you own to use this command.");
		return;
	}

	auto to = ActionableBots::AsNamed_ByBot(c, sep->arg[2]);

	if (!to) {
		c->Message(Chat::Yellow, "Could not find %s.", sep->arg[1]);
		return;
	}
	
	if (!to->IsBot()) {
		c->Message(Chat::Yellow, "%s is not a bot.", to->GetCleanName());
		return;
	}

	if (!to->GetOwner()) {
		c->Message(Chat::Yellow, "Could not find %s's owner.", to->GetCleanName());
	}
	
	if (to->GetOwner() != c) {
		c->Message(Chat::Yellow, "You must name a spawned bot that you own to use this command.");
		return;
	}

	if (to == from) {
		c->Message(Chat::Yellow, "You cannot copy to the same bot that you're copying from.");
		return;
	}

	std::string output = "";

	if (setting_type != UINT16_MAX) {
		if (spell_type != UINT16_MAX) {
			from->CopySettings(to, setting_type, spell_type);
		}
		else {
			for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
				from->CopySettings(to, setting_type, i);
			}
		}

		output = from->GetBotSpellCategoryName(setting_type);
	}
	else {
		if (!strcasecmp(sep->arg[3], "misc")) {
			from->CopySettings(to, BotSettingCategories::BaseSetting);
			output = "Miscellaneous";
		}
		else if (!strcasecmp(sep->arg[3], "spellsettings")) {
			from->CopyBotSpellSettings(to);
			output = "^spellsettings";
		}
		else if (!strcasecmp(sep->arg[3], "spelltypesettings")) {
			if (spell_type != UINT16_MAX) {
				from->CopySettings(to, BotSettingCategories::SpellHold, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellDelay, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellMinThreshold, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, spell_type);
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					from->CopySettings(to, BotSettingCategories::SpellHold, i);
					from->CopySettings(to, BotSettingCategories::SpellDelay, i);
					from->CopySettings(to, BotSettingCategories::SpellMinThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, i);
				}
			}

			output = "spell type";
		}
		else if (!strcasecmp(sep->arg[3], "blockedbuffs")) {
			from->CopyBotBlockedBuffs(to);
		}
		else if (!strcasecmp(sep->arg[3], "blockedpetbuffs")) {
			from->CopyBotBlockedPetBuffs(to);
		}
		else if (!strcasecmp(sep->arg[3], "all")) {
			from->CopySettings(to, BotSettingCategories::BaseSetting);

			if (spell_type != UINT16_MAX) {
				from->CopySettings(to, BotSettingCategories::SpellHold, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellDelay, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellMinThreshold, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, spell_type);
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					from->CopySettings(to, BotSettingCategories::SpellHold, i);
					from->CopySettings(to, BotSettingCategories::SpellDelay, i);
					from->CopySettings(to, BotSettingCategories::SpellMinThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, i);
				}
			}

			from->CopyBotSpellSettings(to);
			from->CopyBotBlockedBuffs(to);
			from->CopyBotBlockedPetBuffs(to);
			output = "spell type";
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

	to->Save();

	c->Message(
		Chat::Green,
		fmt::format(
			"{}'s{}{} settings were copied to {}.",
			from->GetCleanName(),
			(
				spell_type != UINT16_MAX ?
				fmt::format(" [{}] ",
					c->GetSpellTypeNameByID(spell_type)
				)
				: " "
			),
			output,
			to->GetCleanName()
		).c_str()
	);
}