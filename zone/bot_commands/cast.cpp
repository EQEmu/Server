#include "../bot_command.h"

void bot_command_cast(Client* c, const Seperator* sep)
{
	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Commands bots to force cast a specific spell type, ignoring all settings (holds, delays, thresholds, etc)"
		};

		std::vector<std::string> notes =
		{
			"- This will interrupt any spell currently being cast by bots told to use the command",
			"- Bots will still check to see if they have the spell in their spell list, whether the target is immune, spell is allowed and all other sanity checks for spells",
			fmt::format(
				"- You can use {} aa # to cast any clickable AA or specifically {} harmtouch / {} layonhands"
				, sep->arg[0]
				, sep->arg[0]
				, sep->arg[0]
			)
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [Type Shortname] [actionable, default: spawned]"
				, sep->arg[0]
			),
			fmt::format(
				"{} [Type ID] [actionable, default: spawned]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To tell everyone to Nuke the target:",
			fmt::format(
				"{} {}",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} {}",
				sep->arg[0],
				BotSpellTypes::Nuke
			)
		};
		std::vector<std::string> examples_two =
		{
			"To tell Skbot to Harm Touch the target:",
			fmt::format(
				"{} aa 6000 byname Skbot",
				sep->arg[0]
			),
			fmt::format(
				"{} harmtouch byname Skbot",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_three =
		{
			"To tell all bots to try to cast spell #93 (Burst of Flame)",
			fmt::format(
				"{} spellid 93",
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
		c->SendSpellTypePrompts(true);

		c->Message(
			Chat::Yellow,
			fmt::format(
				"Use help after any command type for more subtypes to use, for example: {}.",
				Saylink::Silent("^cast invisibility help")
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

	//AA help
	if (!arg1.compare("aa") && !arg2.compare("help")) {
		c->Message(Chat::Yellow, "Enter the ID of an AA to attempt to cast.", sep->arg[0]);
	}

	//Commanded type help prompts
	if (!arg2.compare("help")) {
		c->Message(Chat::Yellow, "You can also use [single], [group], [ae]. Ex: ^cast movementspeed group.", sep->arg[0]);
	}

	if (!arg1.compare("invisibility") && !arg2.compare("help")) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Available options for {} are: {}, {}, {}, {}.",
				sep->arg[0],
				Saylink::Silent("^cast invisibility see", "see"),
				Saylink::Silent("^cast invisibility invis", "invis"),
				Saylink::Silent("^cast invisibility undead", "undead"),
				Saylink::Silent("^cast invisibility animals", "animals")
			).c_str()
		);

		return;
	}

	if (!arg1.compare("size") && !arg2.compare("help")) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Available options for {} are: {}, {}.",
				sep->arg[0],
				Saylink::Silent("^cast size grow", "grow"),
				Saylink::Silent("^cast size shrink", "shrink")
			).c_str()
		);

		return;
	}

	if (!arg1.compare("movementspeed") && !arg2.compare("help")) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"Available options for {} are: {}, {}.",
				sep->arg[0],
				Saylink::Silent("^cast movementspeed selo"), "selo"
			).c_str()
		);

		return;
	}

	if (!arg2.compare("help")) {
		c->Message(Chat::Yellow, "There are no additional options for {}.", sep->arg[0]);
		return;
	}

	int ab_arg = 2;
	uint16 spellType = UINT16_MAX;
	uint16 subType = UINT16_MAX;
	uint16 subTargetType = UINT16_MAX;
	bool aaType = false;
	int aaID = 0;
	bool bySpellID = false;
	uint16 chosenSpellID = UINT16_MAX;

	if (!arg1.compare("aa") || !arg1.compare("harmtouch") || !arg1.compare("layonhands")) {
		if (!RuleB(Bots, AllowForcedCastsBySpellID)) {
			c->Message(Chat::Yellow, "This commanded type is currently disabled.");
			return;
		}

		if (!arg1.compare("harmtouch")) {
			aaID = zone->GetAlternateAdvancementAbilityByRank(aaHarmTouch)->id;
		}
		else if (!arg1.compare("layonhands")) {
			aaID = zone->GetAlternateAdvancementAbilityByRank(aaLayonHands)->id;
		}
		else if (!sep->IsNumber(2) || !zone->GetAlternateAdvancementAbility(Strings::ToInt(arg2))) {
			c->Message(Chat::Yellow, "You must enter an AA ID.");
			return;
		}
		else {
			++ab_arg;
			aaID = Strings::ToInt(arg2);
		}

		aaType = true;		
	}

	if (!arg1.compare("spellid")) {
		if (!RuleB(Bots, AllowCastAAs)) {
			c->Message(Chat::Yellow, "This commanded type is currently disabled.");
			return;
		}

		if (sep->IsNumber(2) && IsValidSpell(atoi(sep->arg[2]))) {
			++ab_arg;
			chosenSpellID = atoi(sep->arg[2]);
			bySpellID = true;
		}
		else {
			c->Message(Chat::Yellow, "You must enter a valid spell ID.");

			return;
		}
	}

	if (!aaType && !bySpellID) {
		if (sep->IsNumber(1)) {
			spellType = atoi(sep->arg[1]);

			if (spellType < BotSpellTypes::START || (spellType > BotSpellTypes::END && spellType < BotSpellTypes::COMMANDED_START) || spellType > BotSpellTypes::COMMANDED_END) {
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

		switch (spellType) { //Allowed command checks
			case BotSpellTypes::Charm:
				if (!RuleB(Bots, AllowCommandedCharm)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::AEMez:
			case BotSpellTypes::Mez:
				if (!RuleB(Bots, AllowCommandedMez)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::Resurrect:
				if (!RuleB(Bots, AllowCommandedResurrect)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::AELull:
			case BotSpellTypes::Lull:
				if (!RuleB(Bots, AllowCommandedLull)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			case BotSpellTypes::SummonCorpse:
				if (!RuleB(Bots, AllowCommandedSummonCorpse)) {
					c->Message(Chat::Yellow, "This commanded type is currently disabled.");
					return;
				}

				break;
			default:
				break;
		}

		std::string argString = sep->arg[ab_arg];


		if (!argString.compare("shrink")) {
			subType = CommandedSubTypes::Shrink;
			++ab_arg;
		}
		else if (!argString.compare("grow")) {
			subType = CommandedSubTypes::Grow;
			++ab_arg;
		}
		else if (!argString.compare("see")) {
			subType = CommandedSubTypes::SeeInvis;
			++ab_arg;
		}
		else if (!argString.compare("invis")) {
			subType = CommandedSubTypes::Invis;
			++ab_arg;
		}
		else if (!argString.compare("undead")) {
			subType = CommandedSubTypes::InvisUndead;
			++ab_arg;
		}
		else if (!argString.compare("animals")) {
			subType = CommandedSubTypes::InvisAnimals;
			++ab_arg;
		}
		else if (!argString.compare("selo")) {
			subType = CommandedSubTypes::Selo;
			++ab_arg;
		}

		argString = sep->arg[ab_arg];

		if (!argString.compare("single")) {
			subTargetType = CommandedSubTypes::SingleTarget;
			++ab_arg;
		}
		else if (!argString.compare("group")) {
			subTargetType = CommandedSubTypes::GroupTarget;
			++ab_arg;
		}
		else if (!argString.compare("ae")) {
			subTargetType = CommandedSubTypes::AETarget;
			++ab_arg;
		}

		if (
			spellType == BotSpellTypes::PetBuffs ||
			spellType == BotSpellTypes::PetCompleteHeals ||
			spellType == BotSpellTypes::PetFastHeals ||
			spellType == BotSpellTypes::PetHoTHeals ||
			spellType == BotSpellTypes::PetRegularHeals ||
			spellType == BotSpellTypes::PetVeryFastHeals
		) {
			c->Message(Chat::Yellow, "Pet type heals and buffs are not supported, use the regular spell type.");
			return;
		}
	}

	Mob* tar = c->GetTarget();
	//LogTestDebug("{}: 'Attempting {} [{}-{}] on {}'", __LINE__, c->GetSpellTypeNameByID(spellType), (subType != UINT16_MAX ? c->GetSubTypeNameByID(subType) : "Standard"), (subTargetType != UINT16_MAX ? c->GetSubTypeNameByID(subTargetType) : "Standard"), (tar ? tar->GetCleanName() : "NOBODY")); //deleteme

	if (!tar) {
		if ((!aaType && !bySpellID) && spellType != BotSpellTypes::Escape && spellType != BotSpellTypes::Pet) {
			c->Message(Chat::Yellow, "You need a target for that.");
			return;
		}
	}

	if (!aaType && !bySpellID) {
		switch (spellType) { //Target Checks
			case BotSpellTypes::Resurrect:
				if (!tar->IsCorpse() || !tar->CastToCorpse()->IsPlayerCorpse()) {
					c->Message(
						Chat::Yellow, 
						fmt::format(
							"[{}] is not a player's corpse.", 
							tar->GetCleanName()
						).c_str()
					);

					return;
				}

				break;
			case BotSpellTypes::Identify:
			case BotSpellTypes::SendHome:
			case BotSpellTypes::BindAffinity:
			case BotSpellTypes::SummonCorpse:
				if (!tar->IsClient() || !c->IsInGroupOrRaid(tar)) {
					c->Message(
						Chat::Yellow,
						fmt::format(
							"[{}] is an invalid target. Only players in your group or raid are eligible targets.", 
							tar->GetCleanName()
						).c_str()
					);

					return;
				}

				break;
			default:
				if (
					(IsBotSpellTypeDetrimental(spellType) && !c->IsAttackAllowed(tar)) ||
					(
						spellType == BotSpellTypes::Charm &&
						(
							tar->IsClient() ||
							tar->IsCorpse() ||
							tar->GetOwner()
						)
					)
				) {
					c->Message(
						Chat::Yellow,
						fmt::format(
							"You cannot attack [{}].", 
							tar->GetCleanName()
						).c_str()
					);

					return;
				}

				if (IsBotSpellTypeBeneficial(spellType)) {
					if (
						(tar->IsNPC() && !tar->GetOwner()) ||
						(tar->GetOwner() && tar->GetOwner()->IsOfClientBot() && !c->IsInGroupOrRaid(tar->GetOwner())) ||
						(tar->IsOfClientBot() && !c->IsInGroupOrRaid(tar))
					) {
						c->Message(
							Chat::Yellow,
							fmt::format(
								"[{}] is an invalid target. Only players or their pet in your group or raid are eligible targets.",
								tar->GetCleanName()
							).c_str()
						);

						return;
					}
				}

				break;
		}
	}

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

	std::vector<Bot*> sbl;

	if (ActionableBots::PopulateSBL(c, actionableArg, sbl, ab_mask, !class_race_check ? sep->arg[ab_arg + 1] : nullptr, class_race_check ? atoi(sep->arg[ab_arg + 1]) : 0) == ActionableBots::ABT_None) {
		return;
	}

	sbl.erase(std::remove(sbl.begin(), sbl.end(), nullptr), sbl.end());

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;
	bool isSuccess = false;
	uint16 successCount = 0;
	Bot* firstFound = nullptr;

	for (auto bot_iter : sbl) {
		if (!bot_iter->IsInGroupOrRaid(c)) {
			continue;
		}

		if (bot_iter->GetBotStance() == Stance::Passive || bot_iter->GetHoldFlag() || bot_iter->GetAppearance() == eaDead || bot_iter->IsFeared() || bot_iter->IsSilenced() || bot_iter->IsAmnesiad() || bot_iter->GetHP() < 0) {
			continue;
		}

		Mob* newTar = tar;

		if (!aaType && !bySpellID) {
			//LogTestDebug("{}: {} says, 'Attempting {} [{}-{}] on {}'", __LINE__, bot_iter->GetCleanName(), c->GetSpellTypeNameByID(spellType), (subType != UINT16_MAX ? c->GetSubTypeNameByID(subType) : "Standard"), (subTargetType != UINT16_MAX ? c->GetSubTypeNameByID(subTargetType) : "Standard"), (newTar ? newTar->GetCleanName() : "NOBODY")); //deleteme
			if (!SpellTypeRequiresTarget(spellType)) {
				newTar = bot_iter;
			}

			if (!newTar) {
				continue;
			}

			if (
				IsBotSpellTypeBeneficial(spellType) &&
				!RuleB(Bots, CrossRaidBuffingAndHealing) &&
				!bot_iter->IsInGroupOrRaid(newTar, true)
			) {
				continue;
			}

			if (IsBotSpellTypeDetrimental(spellType) && !bot_iter->IsAttackAllowed(newTar)) {
				bot_iter->BotGroupSay(
					bot_iter,
					fmt::format(
						"I cannot attack [{}].",
						newTar->GetCleanName()
					).c_str()
				);

				continue;
			}
		}

		if (aaType) {
			if (!bot_iter->GetAA(zone->GetAlternateAdvancementAbility(aaID)->first_rank_id)) {
				continue;
			}

			LogTestDebug("{}: {} says, 'aaID is {}'", __LINE__, bot_iter->GetCleanName(), aaID); //deleteme
			AA::Rank* tempRank = nullptr;
			AA::Rank*& rank = tempRank;
			uint16 spell_id = bot_iter->GetSpellByAA(aaID, rank);

			if (!IsValidSpell(spell_id)) {
				continue;
			}

			if (!bot_iter->AttemptAACastSpell(tar, spell_id, rank)) {
				continue;
			}

			isSuccess = true;
			++successCount;

			continue;
		}
		else if (bySpellID) {
			SPDat_Spell_Struct spell = spells[chosenSpellID];

			LogTestDebug("Starting bySpellID checks."); //deleteme
			if (!bot_iter->CanUseBotSpell(chosenSpellID)) {
				LogTestDebug("{} does not have {} [#{}].", bot_iter->GetCleanName(), spell.name, chosenSpellID); //deleteme
				continue;
			}

			if (!tar || (spell.target_type == ST_Self && tar != bot_iter)) {
				LogTestDebug("{} set my target to myself for {} [#{}] due to !tar.", bot_iter->GetCleanName(), spell.name, chosenSpellID); //deleteme
				tar = bot_iter;
			}

			if (bot_iter->AttemptForcedCastSpell(tar, chosenSpellID)) {
				if (!firstFound) {
					firstFound = bot_iter;
				}

				isSuccess = true;
				++successCount;
			}
			else {
				c->Message(
					Chat::Red, 
					fmt::format(
						"{} says, '{} [#{}] failed to cast on [{}]. This could be due to this to any number of things: range, mana, immune, etc.'", 
						bot_iter->GetCleanName(), 
						spell.name, 
						chosenSpellID,
						tar->GetCleanName()
					).c_str()
				);
			}

			continue;
		}
		else {
			LogTestDebug("{}: {} says, 'Attempting {} [{}-{}] on [{}]'", __LINE__, bot_iter->GetCleanName(), c->GetSpellTypeNameByID(spellType), (subType != UINT16_MAX ? c->GetSubTypeNameByID(subType) : "Standard"), (subTargetType != UINT16_MAX ? c->GetSubTypeNameByID(subTargetType) : "Standard"), (newTar ? newTar->GetCleanName() : "NOBODY")); //deleteme
			bot_iter->SetCommandedSpell(true);
			
			if (bot_iter->AICastSpell(newTar, 100, spellType, subTargetType, subType)) {
				if (!firstFound) {
					firstFound = bot_iter;
				}

				isSuccess = true;
				++successCount;
			}
			else {
				c->Message(
					Chat::Red, 
					fmt::format(
						"{} says, 'Ability failed to cast [{}]. This could be due to this to any number of things: range, mana, immune, etc.'",
						bot_iter->GetCleanName(), 
						tar->GetCleanName()
					).c_str()
				);
			}

			bot_iter->SetCommandedSpell(false);

			continue;
		}

		continue;
	}

	std::string type = "";

	if (aaType) {
		type = zone->GetAAName(zone->GetAlternateAdvancementAbility(aaID)->first_rank_id);
	}
	else if (bySpellID) {
		type = "Forced";
	}
	else {
		type = c->GetSpellTypeNameByID(spellType);
	}

	if (!isSuccess) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"No bots are capable of casting [{}] on {}.",
				(bySpellID ? spells[chosenSpellID].name : type),
				tar ? tar->GetCleanName() : "your target"
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"{} {} [{}]{}",
				((successCount == 1 && firstFound) ? firstFound->GetCleanName() : (fmt::format("{}", successCount).c_str())),
				((successCount == 1 && firstFound) ? "casted" : "of your bots casted"),
				(bySpellID ? spells[chosenSpellID].name : type),
				tar ? (fmt::format(" on {}.", tar->GetCleanName()).c_str()) : "."
			).c_str()
		);
	}
}
