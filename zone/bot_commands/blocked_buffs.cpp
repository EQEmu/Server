#include "../bot_command.h"

void bot_command_blocked_buffs(Client* c, const Seperator* sep)
{
	if (!RuleB(Bots, AllowBotBlockedBuffs)) {
		c->Message(Chat::Yellow, "This command is disabled.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Allows you to set, view and wipe blocked buffs for the selected bots"
		};

		std::vector<std::string> notes =
		{
			"- You can 'set' spells to be blocked, 'remove' spells from the blocked list, 'list' the current blocked spells or 'wipe' the entire list."
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [add [ID] | remove [ID] | list | wipe] [actionable, default: target]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To add Courage (Spell ID #202) to the targeted bot's blocked list:",
			fmt::format(
				"{} add 202",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			)
		};
		std::vector<std::string> examples_two =
		{
			"To view the targeted bot's blocked buff list:",
			fmt::format(
				"{} list",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_three =
		{
			"To wipe all Warriors bots' blocked buff list:",
			fmt::format(
				"{} wipe byclass {}",
				sep->arg[0],
				Class::Warrior
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
				"You can also control bot buffs ({}).",
				Saylink::Silent("^blockedbuffs help", "^blockedbuffs")
			).c_str()
		);

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
	bool add = false;
	bool remove = false;
	bool list = false;
	bool wipe = false;
	uint16 spell_id;

	if (!arg1.compare("add")) {
		if (!sep->IsNumber(2) || !IsValidSpell(atoi(sep->arg[2])) || !IsBeneficialSpell(atoi(sep->arg[2]))) {
			c->Message(Chat::Yellow, "You must enter a valid spell ID.");
			return;
		}

		add = true;
		spell_id = atoi(sep->arg[2]);
		++ab_arg;
	}
	else if (!arg1.compare("remove")) {
		if (!sep->IsNumber(2) || !IsValidSpell(atoi(sep->arg[2]))) {
			c->Message(Chat::Yellow, "You must enter a valid spell ID.");
			return;
		}

		remove = true;
		spell_id = atoi(sep->arg[2]);
		++ab_arg;
	}
	else if (!arg1.compare("list")) {
		list = true;
	}
	else if (!arg1.compare("wipe")) {
		wipe = true;
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
	std::string actionable_arg = sep->arg[ab_arg];

	if (actionable_arg.empty()) {
		actionable_arg = "target";
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, actionable_arg, sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	bool is_success = false;
	uint16 success_count = 0;
	Bot* first_found = nullptr;

	for (auto bot_iter : sbl) {
		if (!bot_iter->IsInGroupOrRaid(c)) {
			continue;
		}

		if (!first_found) {
			first_found = bot_iter;
		}

		if (add) {
			bot_iter->SetBotBlockedBuff(spell_id, true);
		}
		else if (remove) {
			bot_iter->SetBotBlockedBuff(spell_id, false);
		}
		else if (list) {
			std::vector<BotBlockedBuffs_Struct> blocked_buffs = bot_iter->GetBotBlockedBuffs();
			bool found = false;

			if (!blocked_buffs.empty()) {
				for (auto& blocked_buff : blocked_buffs) {
					if (blocked_buff.blocked == 1 && IsValidSpell(blocked_buff.spell_id)) {
						found = true;
						c->Message(
							Chat::Yellow,
							fmt::format(
								"{} says, '{} [#{}] is currently blocked.'",
								bot_iter->GetCleanName(),
								spells[blocked_buff.spell_id].name,
								blocked_buff.spell_id
							).c_str()
						);
					}
				}
			}
			
			if (!found) {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"{} says, 'I am not currently blocking any buffs.'",
						bot_iter->GetCleanName()
					).c_str()
				);
			}
		}
		else if (wipe) {
			bot_iter->ClearBotBlockedBuffs();

			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} says, I have wiped my blocked buffs list.'",
					bot_iter->GetCleanName()
				).c_str()
			);
		}

		is_success = true;
		++success_count;
	}

	if (!is_success) {
		c->Message(Chat::Yellow, "No bots were selected.");
	}
	else {
		if (add || remove) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} {} {} blocking {} [#{}]",
					((success_count == 1 && first_found) ? first_found->GetCleanName() : (fmt::format("{}", success_count).c_str())),
					((success_count == 1 && first_found) ? "is" : "of your bots"),
					(add ? "now" : "no longer"),
					spells[spell_id].name,
					spell_id
				).c_str()
			);
		}	
	}
}


void bot_command_blocked_pet_buffs(Client* c, const Seperator* sep)
{
	if (!RuleB(Bots, AllowBotBlockedBuffs)) {
		c->Message(Chat::Yellow, "This command is disabled.");

		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Allows you to set, view and wipe blocked pet buffs for the selected bots"
		};

		std::vector<std::string> notes =
		{
			"- You can 'set' spells to be blocked, 'remove' spells from the blocked list, 'list' the current blocked spells or 'wipe' the entire list.",
			"- This controls whether or not any pet the selected bot(s) own will prevent certain buffs from being cast."
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [add [ID] | remove [ID] | list | wipe] [actionable, default: target]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To add Courage (Spell ID #202) to the targeted bot's blocked list:",
			fmt::format(
				"{} add 202",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			)
		};
		std::vector<std::string> examples_two =
		{
			"To view the targeted bot's blocked buff list:",
			fmt::format(
				"{} list",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_three =
		{
			"To wipe all Warriors bots' blocked buff list:",
			fmt::format(
				"{} wipe byclass {}",
				sep->arg[0],
				Class::Warrior
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
				"You can also control pet buffs ({}).",
				Saylink::Silent("^blockedpetbuffs help", "^blockedpetbuffs")
			).c_str()
		);

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
	bool add = false;
	bool remove = false;
	bool list = false;
	bool wipe = false;
	uint16 spell_id;

	if (!arg1.compare("add")) {
		if (!sep->IsNumber(2) || !IsValidSpell(atoi(sep->arg[2])) || !IsBeneficialSpell(atoi(sep->arg[2]))) {
			c->Message(Chat::Yellow, "You must enter a valid spell ID.");
			return;
		}

		add = true;
		spell_id = atoi(sep->arg[2]);
		++ab_arg;
	}
	else if (!arg1.compare("remove")) {
		if (!sep->IsNumber(2) || !IsValidSpell(atoi(sep->arg[2]))) {
			c->Message(Chat::Yellow, "You must enter a valid spell ID.");
			return;
		}

		remove = true;
		spell_id = atoi(sep->arg[2]);
		++ab_arg;
	}
	else if (!arg1.compare("list")) {
		list = true;
	}
	else if (!arg1.compare("wipe")) {
		wipe = true;
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
	std::string actionable_arg = sep->arg[ab_arg];

	if (actionable_arg.empty()) {
		actionable_arg = "target";
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, actionable_arg, sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	bool is_success = false;
	uint16 success_count = 0;
	Bot* first_found = nullptr;

	for (auto bot_iter : sbl) {
		if (!bot_iter->IsInGroupOrRaid(c)) {
			continue;
		}

		if (!first_found) {
			first_found = bot_iter;
		}

		if (add) {
			bot_iter->SetBotBlockedPetBuff(spell_id, true);
		}
		else if (remove) {
			bot_iter->SetBotBlockedPetBuff(spell_id, false);
		}
		else if (list) {
			std::vector<BotBlockedBuffs_Struct> blocked_buffs = bot_iter->GetBotBlockedBuffs();
			bool found = false;

			if (!blocked_buffs.empty()) {
				for (auto& blocked_buff : blocked_buffs) {
					if (blocked_buff.blocked_pet == 1 && IsValidSpell(blocked_buff.spell_id)) {
						found = true;
						c->Message(
							Chat::Yellow,
							fmt::format(
								"{} says, '{} [#{}] is currently blocked for my pet.'",
								bot_iter->GetCleanName(),
								spells[blocked_buff.spell_id].name,
								blocked_buff.spell_id
							).c_str()
						);
					}
				}
			}

			if (!found) {
				c->Message(
					Chat::Yellow,
					fmt::format(
						"{} says, 'I am not currently blocking any pet buffs.'",
						bot_iter->GetCleanName()
					).c_str()
				);
			}
		}
		else if (wipe) {
			bot_iter->ClearBotBlockedBuffs();

			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} says, I have wiped my blocked buffs list.'",
					bot_iter->GetCleanName()
				).c_str()
			);
		}

		is_success = true;
		++success_count;
	}

	if (!is_success) {
		c->Message(Chat::Yellow, "No bots were selected.");
	}
	else {
		if (add || remove) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"{} {} {} blocking {} [#{}] on their pet.",
					((success_count == 1 && first_found) ? first_found->GetCleanName() : (fmt::format("{}", success_count).c_str())),
					((success_count == 1 && first_found) ? "is" : "of your bots"),
					(add ? "now" : "no longer"),
					spells[spell_id].name,
					spell_id
				).c_str()
			);
		}
	}
}
