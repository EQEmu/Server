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
			"- This will interrupt any spell currently being cast by bots told to use the command.",
			"- Bots will still check to see if they have the spell in their spell list, whether the target is immune, spell is allowed and all other sanity checks for spells"
		};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [Type Shortname] [actionable]"
				, sep->arg[0]
			),
			fmt::format(
				"{} [Type ID] [actionable]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To tell everyone to Nuke the target:",
			fmt::format(
				"{} {} spawned",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Nuke)
			),
			fmt::format(
				"{} {} spawned",
				sep->arg[0],
				BotSpellTypes::Nuke
			)
		};
		std::vector<std::string> examples_two =
		{
			"To tell all Enchanters to slow the target:",
			fmt::format(
				"{} {} byclass {}",
				sep->arg[0],
				Class::Enchanter,
				c->GetSpellTypeShortNameByID(BotSpellTypes::Slow)
			),
			fmt::format(
				"{} {} byclass {}",
				sep->arg[0],
				Class::Enchanter,
				BotSpellTypes::Slow
			)
		};
		std::vector<std::string> examples_three =
		{
			"To tell Clrbot to resurrect the targeted corpse:",
			fmt::format(
				"{} {} byname Clrbot",
				sep->arg[0],
				c->GetSpellTypeShortNameByID(BotSpellTypes::Resurrect)
			),
			fmt::format(
				"{} {} byname Clrbot",
				sep->arg[0],
				BotSpellTypes::Resurrect
			)
		};

		std::vector<std::string> actionables =
		{
			"target, byname, ownergroup, ownerraid, targetgroup, namesgroup, healrotationtargets mmr, byclass, byrace, spawned"
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
	uint16 spellType = 0;

	// String/Int type checks
	if (sep->IsNumber(1)) {
		spellType = atoi(sep->arg[1]);

		if (spellType < BotSpellTypes::START || spellType > BotSpellTypes::END) {
			c->Message(Chat::Yellow, "You must choose a valid spell type. Spell types range from %i to %i", BotSpellTypes::START, BotSpellTypes::END);

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

	Mob* tar = c->GetTarget();
	LogTestDebug("{}: Attempting {} on {}", __LINE__, c->GetSpellTypeNameByID(spellType), (tar ? tar->GetCleanName() : "NOBODY")); //deleteme
	if (spellType != BotSpellTypes::Escape && spellType != BotSpellTypes::Pet) {
		if (!tar) {
			c->Message(Chat::Yellow, "You need a target for that.");
			return;
		}

		if (BOT_SPELL_TYPES_DETRIMENTAL(spellType) && !c->IsAttackAllowed(tar)) {
			c->Message(Chat::Yellow, "You cannot attack [%s].", tar->GetCleanName());
			return;
		}

		if (BOT_SPELL_TYPES_BENEFICIAL(spellType)) {
			if (!tar->IsOfClientBot() && !(tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot())) {
				c->Message(Chat::Yellow, "[%s] is an invalid target.", tar->GetCleanName());
				return;
			}
		}
	}
	LogTestDebug("{}: Attempting {} on {}", __LINE__, c->GetSpellTypeNameByID(spellType), (tar ? tar->GetCleanName() : "NOBODY")); //deleteme
	switch (spellType) {
		case BotSpellTypes::Stun:
		case BotSpellTypes::AEStun:
			if (tar->GetSpecialAbility(SpecialAbility::StunImmunity)) {
				c->Message(Chat::Yellow, "[%s] is immune to stuns.", tar->GetCleanName());
				return;
			}
			
			break;
		case BotSpellTypes::Resurrect:
			if (!tar->IsCorpse() || !tar->CastToCorpse()->IsPlayerCorpse()) {
				c->Message(Chat::Yellow, "[%s] is an invalid target. I can only resurrect player corpses.", tar->GetCleanName());
				return;
			}

			break;
		default:
			break;
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

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;
	bool isSuccess = false;
	uint16 successCount = 0;
	Bot* firstFound = nullptr;

	for (auto bot_iter : sbl) {
		if (!bot_iter->IsInGroupOrRaid()) {
			continue;
		}

		/*
		TODO bot rewrite - 
		FIX: Snares, Group Cures, OOC Song, Precombat, HateRedux, Fear/AE Fear
		ICB (SK) casting hate on friendly but not hostile?
		NEED TO CHECK: precombat, AE Dispel, AE Lifetap
		DO I NEED A PBAE CHECK???
		*/
		if (bot_iter->GetHoldFlag() || bot_iter->GetAppearance() == eaDead || bot_iter->IsFeared() || bot_iter->IsStunned() || bot_iter->IsMezzed() || bot_iter->DivineAura() || bot_iter->GetHP() < 0) {
			continue;
		}

		Mob* newTar = tar;
		LogTestDebug("{}: Attempting {} on {}", __LINE__, c->GetSpellTypeNameByID(spellType), (newTar ? newTar->GetCleanName() : "NOBODY")); //deleteme
		if (!SpellTypeRequiresTarget(spellType, bot_iter->GetClass())) {
			newTar = bot_iter;
		}

		if (!newTar) {
			continue;
		}

		if (BOT_SPELL_TYPES_DETRIMENTAL(spellType, bot_iter->GetClass()) && !bot_iter->IsAttackAllowed(newTar)) {
			bot_iter->BotGroupSay(
				bot_iter,
				fmt::format(
					"I cannot attack [{}].",
					newTar->GetCleanName()
				).c_str()
			);

			continue;
		}

		LogTestDebug("{}: Attempting {} on {}", __LINE__, c->GetSpellTypeNameByID(spellType), (newTar ? newTar->GetCleanName() : "NOBODY")); //deleteme

		bot_iter->SetCommandedSpell(true);

		if (bot_iter->AICastSpell(newTar, 100, spellType)) {
			if (!firstFound) {
				firstFound = bot_iter;
			}

			isSuccess = true;
			++successCount;
		}

		bot_iter->SetCommandedSpell(false);
		continue;
	}

	if (!isSuccess) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"No bots are capable of casting [{}] on {}.",
				c->GetSpellTypeNameByID(spellType),
				tar ? tar->GetCleanName() : "your target"
			).c_str()
		);
	}
	else {
		c->Message( Chat::Yellow,
			fmt::format(
				"{} {} [{}]{}",
				((successCount == 1 && firstFound) ? firstFound->GetCleanName() : (fmt::format("{}", successCount).c_str())),
				((successCount == 1 && firstFound) ? "casted" : "of your bots casted"),
				c->GetSpellTypeNameByID(spellType),
				tar ? (fmt::format(" on {}.", tar->GetCleanName()).c_str()) : "."
			).c_str()
		);
	}
}
