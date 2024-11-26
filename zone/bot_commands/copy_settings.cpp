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
			"target, byname, ownergroup, ownerraid",
			"targetgroup, namesgroup, healrotationtargets",
			"mmr, byclass, byrace, spawned"
		};

		std::vector<std::string> options =
		{
			"all, misc, spellsettings, spelltypesettings",
			"holds, delays, minthresholds, maxthresholds",
			"minmanapct, maxmanapct, minhppct, maxhppct",
			"idlepriority, engagedpriority, pursuepriority",
			"aggrochecks, targetcounts"
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
			"- ^showhelm, ^followd, ^stopmeleelevel",
			"- ^enforcespellsettings, ^bottoggleranged, ^petsettype",
			"- ^behindmob, ^distanceranged, ^illusionblock",
			"- ^sitincombat, ^sithppercent and ^sitmanapercent",

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
		SendSpellTypePrompts(c);

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
		"aggrochecks",
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
		if (sep->arg[3] == options[i]) {
			if (options[i] != "all" && options[i] != "misc" && options[i] != "spellsettings") {

				if (sep->IsNumber(4) || c->GetSpellTypeIDByShortName(sep->arg[4]) != UINT16_MAX) {
					if (sep->IsNumber(4)) {
						spellType = atoi(sep->arg[4]);
					}

					if (c->GetSpellTypeIDByShortName(sep->arg[4]) != UINT16_MAX) {
						spellType = c->GetSpellTypeIDByShortName(sep->arg[4]);
					}

					if (spellType < BotSpellTypes::START || spellType > BotSpellTypes::END) {
						c->Message(Chat::Yellow, "You must choose a valid spell type. Spell types range from %i to %i", BotSpellTypes::START, BotSpellTypes::END);

						return;
					}
				}
			}
			else if (
				(options[i] == "all" || options[i] == "misc" || options[i] == "spellsettings") &&
				((sep->IsNumber(2) || c->GetSpellTypeIDByShortName(sep->arg[4]) != UINT16_MAX))
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

	if (!strcasecmp(sep->arg[3], "misc")) {
		from->CopySettings(to, BotSettingCategories::BaseSetting);
		output = "Miscellaneous";
	}
	else if (!strcasecmp(sep->arg[3], "holds")) {
		from->CopySettings(to, BotSettingCategories::SpellHold, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellHold);
	}
	else if (!strcasecmp(sep->arg[3], "delays")) {
		from->CopySettings(to, BotSettingCategories::SpellDelay, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellDelay);
	}
	else if (!strcasecmp(sep->arg[3], "minthresholds")) {
		from->CopySettings(to, BotSettingCategories::SpellMinThreshold, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellMinThreshold);
	}
	else if (!strcasecmp(sep->arg[3], "maxthresholds")) {
		from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellMaxThreshold);
	}
	else if (!strcasecmp(sep->arg[3], "aggrochecks")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeAggroCheck);
	}
	else if (!strcasecmp(sep->arg[3], "minmanapct")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeMinManaPct);
	}
	else if (!strcasecmp(sep->arg[3], "maxmanapct")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeMaxManaPct);
	}
	else if (!strcasecmp(sep->arg[3], "minhppct")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeMinHPPct);
	}
	else if (!strcasecmp(sep->arg[3], "maxhppct")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeMaxHPPct);
	}
	else if (!strcasecmp(sep->arg[3], "idlepriority")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeIdlePriority);
	}
	else if (!strcasecmp(sep->arg[3], "engagedpriority")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeEngagedPriority);
	}
	else if (!strcasecmp(sep->arg[3], "pursuepriority")) {
		from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypePursuePriority);
	}
	else if (!strcasecmp(sep->arg[3], "targetcounts")) {
		from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, spellType);
		output = from->GetBotSpellCategoryName(BotSettingCategories::SpellTypeAEOrGroupTargetCount);
	}
	else if (!strcasecmp(sep->arg[3], "spellsettings")) {		
		from->CopyBotSpellSettings(to);
		output = "^spellsettings";
	}
	else if (!strcasecmp(sep->arg[3], "spelltypesettings")) {		
		from->CopySettings(to, BotSettingCategories::SpellHold, spellType);
		from->CopySettings(to, BotSettingCategories::SpellDelay, spellType);
		from->CopySettings(to, BotSettingCategories::SpellMinThreshold, spellType);
		from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, spellType);
		output = "spell type";
	}
	else if (!strcasecmp(sep->arg[3], "all")) {
		from->CopySettings(to, BotSettingCategories::BaseSetting);
		from->CopySettings(to, BotSettingCategories::SpellHold, spellType);
		from->CopySettings(to, BotSettingCategories::SpellDelay, spellType);
		from->CopySettings(to, BotSettingCategories::SpellMinThreshold, spellType);
		from->CopySettings(to, BotSettingCategories::SpellMaxThreshold, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeAggroCheck, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMinManaPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMaxManaPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMinHPPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeMaxHPPct, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeIdlePriority, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeEngagedPriority, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypePursuePriority, spellType);
		from->CopySettings(to, BotSettingCategories::SpellTypeAEOrGroupTargetCount, spellType);
		from->CopyBotSpellSettings(to);
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

	c->Message(
		Chat::Green,
		fmt::format(
			"{}'s{}{} settings were copied to {}.",
			from->GetCleanName(),
			(
				spellType != UINT16_MAX ?
				fmt::format(" [{}] ",
					c->GetSpellTypeNameByID(spellType)
				)
				: " "
			),
			output,
			to->GetCleanName()
		).c_str()
	);
}
