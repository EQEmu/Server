#include "../bot_command.h"

void bot_command_depart(Client* c, const Seperator* sep)
{
	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Tells bots to list their port locations or port to a specific location"
		};

		std::vector<std::string> notes =
		{
			"- This will interrupt any spell currently being cast by bots told to use the command.",
			"- Bots will still check to see if they have the spell in their spell list, whether the target is immune, spell is allowed and all other sanity checks for spells"
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [list | zone shortname] [optional: single | group | ae] [actionable, default: spawned]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To tell everyone to list their portable locations:",
			fmt::format(
				"{} list spawned",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_two =
		{
			"To tell all bots to port to Nexus:",
			fmt::format(
				"{} nexus spawned",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_three =
		{
			"To tell Druidbot to single target port to Butcher:",
			fmt::format(
				"{} butcher single byname Druidbot",
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

	bool single = false;
	bool group = true;
	bool ae = false;
	std::string single_arg = sep->arg[2];
	bool list = false;
	std::string destination = sep->arg[1];
	int ab_arg = 2;
	
	if (!single_arg.compare("single")) {
		++ab_arg;
		single = true;
		group = false;
	}
	else if (!single_arg.compare("group")) {
		++ab_arg;
		single = false;
		group = true;
	}
	else if (!single_arg.compare("ae")) {
		++ab_arg;
		single = false;
		group = false;
		ae = true;
	}

	if (!destination.compare("list") || destination.empty()) {
		list = true;

		if (destination.empty()) {
			c->Message(
				Chat::Yellow,
				fmt::format(
					"Use {} for information regarding this command.",
					Saylink::Silent(
						fmt::format("{} help", sep->arg[0])
					)
				).c_str()
			);
		}
	}

	Mob* tar = c->GetTarget();

	if (!tar) {
		tar = c;
	}

	std::string argString = sep->arg[ab_arg];

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string actionableArg = sep->arg[ab_arg];

	if (actionableArg.empty()) {
		actionableArg = "spawned";
	}

	std::string class_race_arg = sep->arg[ab_arg];
	bool class_race_check = false;

	if (!class_race_arg.compare("byclass") || !class_race_arg.compare("byrace")) {
		class_race_check = true;
	}

	std::list<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, actionableArg, sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.remove(nullptr);

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	bool isSuccess = false;
	std::map<std::string, std::pair<uint8_t, uint8_t>> listZones;

	for (auto bot_iter : sbl) {
		if (!bot_iter->IsInGroupOrRaid(tar, !single)) {
			continue;
		}

		if (bot_iter->GetBotStance() == Stance::Passive || bot_iter->GetHoldFlag() || bot_iter->GetAppearance() == eaDead || bot_iter->IsFeared() || bot_iter->IsSilenced() || bot_iter->IsAmnesiad() || bot_iter->GetHP() < 0) {
			continue;
		}

		std::vector<BotSpell_wPriority> botSpellListItr = bot_iter->GetPrioritizedBotSpellsBySpellType(bot_iter, BotSpellTypes::Teleport, tar);

		for (std::vector<BotSpell_wPriority>::iterator itr = botSpellListItr.begin(); itr != botSpellListItr.end(); ++itr) {
			if (!IsValidSpell(itr->SpellId)) {
				continue;
			}

			if (
				(single && spells[itr->SpellId].target_type != ST_Target) ||
				(group && !IsGroupSpell(itr->SpellId)) ||
				(ae && !IsAnyAESpell(itr->SpellId))
			) {
				continue;
			}

			if (list) {
				auto it = listZones.find(spells[itr->SpellId].teleport_zone);

				if (it != listZones.end()) {
					const auto& [val1, val2] = it->second;

					if (val1 == spells[itr->SpellId].target_type && val2 == bot_iter->GetClass()) {
						continue;
					}
				}

				Bot::BotGroupSay(
					bot_iter,
					fmt::format(
						"I can port you to {}.",
						Saylink::Silent(
							fmt::format(
								"{} {} {} byname {}",
								sep->arg[0],
								spells[itr->SpellId].teleport_zone,
								(spells[itr->SpellId].target_type == ST_Target ? "single" : (IsGroupSpell(itr->SpellId) ? "group" : "ae")),
								bot_iter->GetCleanName()
							).c_str()
							, ZoneLongName(ZoneID(spells[itr->SpellId].teleport_zone)))
					).c_str()
				);

				listZones.insert({ spells[itr->SpellId].teleport_zone, {spells[itr->SpellId].target_type, bot_iter->GetClass()} });

				continue;
			}

			if (destination.compare(spells[itr->SpellId].teleport_zone)) {
				continue;
			}

			bot_iter->SetCommandedSpell(true);

			if (!IsValidSpellAndLoS(itr->SpellId, bot_iter->HasLoS())) {
				continue;
			}

			if (IsInvulnerabilitySpell(itr->SpellId)) {
				tar = bot_iter; //target self for invul type spells
			}

			if (bot_iter->IsCommandedSpell() && bot_iter->IsCasting()) {
				Bot::BotGroupSay(
					bot_iter,
					fmt::format(
						"Interrupting {}. I have been commanded to try to cast a [{}] spell, {} on {}.",
						bot_iter->CastingSpellID() ? spells[bot_iter->CastingSpellID()].name : "my spell",
						bot_iter->GetSpellTypeNameByID(BotSpellTypes::Teleport),
						spells[itr->SpellId].name,
						tar->GetCleanName()
					).c_str()
				);

				bot_iter->InterruptSpell();
			}

			if (bot_iter->CastSpell(itr->SpellId, tar->GetID(), EQ::spells::CastingSlot::Gem2, -1, -1)) {
				if (IsBotSpellTypeOtherBeneficial(BotSpellTypes::Teleport)) {
					bot_iter->SetCastedSpellType(UINT16_MAX);
				}
				else {
					bot_iter->SetCastedSpellType(BotSpellTypes::Teleport);
				}

				if (bot_iter->GetClass() != Class::Bard || RuleB(Bots, BardsAnnounceCasts)) {
					Bot::BotGroupSay(
						bot_iter,
						fmt::format(
							"Casting {} [{}] on {}.",
							GetSpellName(itr->SpellId),
							bot_iter->GetSpellTypeNameByID(BotSpellTypes::Teleport),
							(tar == bot_iter ? "myself" : tar->GetCleanName())
						).c_str()
					);
				}

				isSuccess = true;
			}

			bot_iter->SetCommandedSpell(false);

			if (isSuccess) {
				return;
			}
			else {
				continue;
			}
		}
	}

	if (
		(list && listZones.empty()) || 
		(!list && !isSuccess)
	) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"No bots are capable of that on {}. Be sure they are in the same group, raid or raid group if necessary.",
				tar ? tar->GetCleanName() : "you"
			).c_str()
		);
	}
}
