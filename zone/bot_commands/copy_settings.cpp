#include "../bot_command.h"

void bot_command_copy_settings(Client* c, const Seperator* sep)
{
	if (helper_command_alias_fail(c, "bot_command_copy_settings", sep->arg[0], "copysettings")) {
		c->Message(Chat::White, "note: Copies settings from one bot to another.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		BotCommandHelpParams p;

		p.description =
		{
			"Copies settings from one bot to another."
		};
		p.notes =
		{
			"- You can put a spell type ID or shortname after any option except [all], [misc] and [spellsettings] to restore that specifc spell type only"
		};
		p.example_format = { fmt::format("{} [from] [to] [option] [optional: spelltype id/short name]", sep->arg[0]) };
		p.examples_one =
		{
			"To copy all settings from BotA to BotB:",
			fmt::format("{} BotA BotB all", sep->arg[0])
		};
		p.examples_two =
		{
			"To copy only Nuke spelltypesettings from BotA to BotB:",
			fmt::format(
				"{} BotA BotB spelltypesettings {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} BotA BotB spelltypesettings {}",
				sep->arg[0],
				Bot::GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
		};
		p.examples_three =
		{
			"To copy only spellsettings from BotA to BotB:",
			fmt::format(
				"{} BotA BotB spellsettings",
				sep->arg[0]
			),
			fmt::format(
				"{} BotA BotB spellsettings",
				sep->arg[0]
			),
		};
		p.actionables = { "target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets, mmr, byclass, byrace, spawned" };
		p.options = { "all, misc, spellsettings, spelltypesettings, spellholds, spelldelays, spellminthresholds, spellmaxthresholds, spellminmanapct, spellmaxmanapct, spellminhppct, spellmaxhppct, spellidlepriority, spellengagedpriority, spellpursuepriority, spellaggrochecks, spelltargetcounts, spellresistlimits, spellannouncecasts, blockedbuffs, blockedpetbuffs" };
		p.options_one =
		{
			"[spellsettings] will copy ^spellsettings options",
			"[spelltypesettings] copies all spell type settings",
			"[all] copies all settings" 
		};
		p.options_two =
		{
			"[misc] copies all miscellaneous options such as:",
			"- ^showhelm, ^followd, ^stopmeleelevel, ^enforcespellsettings, ^bottoggleranged, ^petsettype, ^behindmob, ^distanceranged, ^illusionblock, ^sitincombat, ^sithppercent, ^sitmanapercent, ^blockedbuffs, ^blockedpetbuffs"
		};
		p.options_three = { "The remaining options copy that specific type" };

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

	int spell_type_arg_int = 4;
	std::string spell_type_arg = sep->arg[spell_type_arg_int];
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
		"spellresistlimits",
		"spellannouncecasts",
		"blockedbuffs", 
		"blockedpetbuffs"
	};

	if (sep->IsNumber(spell_type_arg_int)) {
		spell_type = atoi(sep->arg[spell_type_arg_int]);

		if (!Bot::IsValidBotSpellType(spell_type)) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"You must choose a valid spell type. Use {} for information regarding this command.",
					Saylink::Silent(
						fmt::format("{} help", sep->arg[0])
					)
				).c_str()
			);

			return;
		}
	}
	else if (!spell_type_arg.empty()) {
		if (Bot::GetSpellTypeIDByShortName(spell_type_arg) != UINT16_MAX) {
			spell_type = Bot::GetSpellTypeIDByShortName(spell_type_arg);
		}
		else {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"You must choose a valid spell type. Use {} for information regarding this command.",
					Saylink::Silent(
						fmt::format("{} help", sep->arg[0])
					)
				).c_str()
			);

			return;
		}
	}

	for (int i = 0; i < options.size(); i++) {
		if (sep->arg[3] == options[i]) {
			setting_type = Bot::GetBotSpellCategoryIDByShortName(sep->arg[3]);
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

		output = Bot::GetBotSpellCategoryName(setting_type);
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
				from->CopySettings(to, BotSettingCategories::SpellTypeResistLimit, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, spell_type);				
				from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeAnnounceCast, spell_type);
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					from->CopySettings(to, BotSettingCategories::SpellHold, i);
					from->CopySettings(to, BotSettingCategories::SpellDelay, i);
					from->CopySettings(to, BotSettingCategories::SpellMinThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeResistLimit, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, i);					
					from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAnnounceCast, i);
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
				from->CopySettings(to, BotSettingCategories::SpellTypeResistLimit, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, spell_type);				
				from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, spell_type);
				from->CopySettings(to, BotSettingCategories::SpellTypeAnnounceCast, spell_type);
			}
			else {
				for (uint16 i = BotSpellTypes::START; i <= BotSpellTypes::END; ++i) {
					from->CopySettings(to, BotSettingCategories::SpellHold, i);
					from->CopySettings(to, BotSettingCategories::SpellDelay, i);
					from->CopySettings(to, BotSettingCategories::SpellMinThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeResistLimit, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, i);
					from->CopySettings(to, BotSettingCategories::SpellTypeAnnounceCast, i);
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
					Bot::GetSpellTypeNameByID(spell_type)
				)
				: " "
			),
			output,
			to->GetCleanName()
		).c_str()
	);
}
