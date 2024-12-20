#include "../bot_command.h"
#include "../bot.h"

void bot_command_pet(Client *c, const Seperator *sep)
{

	std::list<const char*> subcommand_list;
	subcommand_list.push_back("petgetlost");
	subcommand_list.push_back("petremove");
	subcommand_list.push_back("petsettype");

	if (helper_command_alias_fail(c, "bot_command_pet", sep->arg[0], "pet"))
		return;

	helper_send_available_subcommands(c, "bot pet", subcommand_list);
}

void bot_command_pet_get_lost(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pet_get_lost", sep->arg[0], "petgetlost"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname | ownergroup | ownerraid | targetgroup | namesgroup | healrotationmembers | healrotationtargets | mmr | byclass | byrace | spawned] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = ActionableBots::ABM_NoFilter;

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	int summoned_pet = 0;
	for (auto bot_iter : sbl) {
		if (!bot_iter->GetPet() || bot_iter->GetPet()->IsCharmed())
			continue;

		bot_iter->GetPet()->SayString(PET_GETLOST_STRING);
		bot_iter->GetPet()->Depop(false);
		bot_iter->SetPetID(0);
		database.botdb.DeletePetItems(bot_iter->GetBotID());
		database.botdb.DeletePetBuffs(bot_iter->GetBotID());
		database.botdb.DeletePetStats(bot_iter->GetBotID());
		++summoned_pet;
	}

	c->Message(Chat::White, "%i of your bots released their summoned pet%s", summoned_pet, (summoned_pet == 1) ? "" : "s");
}

void bot_command_pet_remove(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pet_remove", sep->arg[0], "petremove"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName);

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[1], sbl, ab_mask, sep->arg[2]) == ActionableBots::ABT_None)
		return;

	uint16 class_mask = (player_class_bitmasks[Class::Druid] | player_class_bitmasks[Class::Necromancer] | player_class_bitmasks[Class::Enchanter]);
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);
	if (sbl.empty()) {
		c->Message(Chat::White, "You have no spawned bots capable of charming");
		return;
	}
	sbl.remove(nullptr);

	int charmed_pet = 0;
	int summoned_pet = 0;
	for (auto bot_iter : sbl) { // Probably needs some work to release charmed pets
		if (bot_iter->IsBotCharmer()) {
			bot_iter->SetBotCharmer(false);
			if (sbl.size() == 1)
				Bot::BotGroupSay(bot_iter, "Using a summoned pet");
			++summoned_pet;
			continue;
		}

		if (bot_iter->GetPet()) {
			bot_iter->GetPet()->SayString(PET_GETLOST_STRING);
			bot_iter->GetPet()->Depop(false);
			bot_iter->SetPetID(0);
		}
		bot_iter->SetBotCharmer(true);
		if (sbl.size() == 1)
			Bot::BotGroupSay(bot_iter, "Available for Charming");
		++charmed_pet;
	}

	if (sbl.size() != 1)
		c->Message(Chat::White, "%i of your bots set for charming, %i of your bots set for summoned pet use", charmed_pet, summoned_pet);
}

void bot_command_pet_set_type(Client *c, const Seperator *sep)
{
	if (helper_command_alias_fail(c, "bot_command_pet_set_type", sep->arg[0], "petsettype")) {
		return;
	}

	if (helper_is_help_or_usage(sep->arg[1])) {
		std::vector<std::string> description =
		{
			"Allows you to change the type of pet Magician bots will cast"
		};

		std::vector<std::string> notes = {};

		std::vector<std::string> example_format =
		{
			fmt::format(
				"{} [current | water | fire | air | earth | monster | epic] [actionable, default: target]"
				, sep->arg[0]
			)
		};
		std::vector<std::string> examples_one =
		{
			"To set all spawned bots to use Water pets:",
			fmt::format(
				"{} fire spawned",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_two =
		{
			"To set Magelulz to use Fire pets:",
			fmt::format(
				"{} fire byname Magelulz",
				sep->arg[0]
			)
		};
		std::vector<std::string> examples_three =
		{
			"To check the current pet type for all bots:",
			fmt::format(
				"{} current spawned",
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

	std::string arg1 = sep->arg[1];
	int ab_arg = 2;
	bool current_check = false;
	uint8 pet_type = 255;
	uint8 level_req = 255;

	if (!arg1.compare("auto")) {
		pet_type = 0;
	}
	else if (!arg1.compare("water")) {
		pet_type = 1;
	}
	else if (!arg1.compare("fire")) {
		pet_type = 2;
	}
	else if (!arg1.compare("air")) {
		pet_type = 3;
	}
	else if (!arg1.compare("earth")) {
		pet_type = 4;
	}
	else if (!arg1.compare("monster")) {
		pet_type = 5;
	}
	else if (!arg1.compare("epic")) {
		if (!RuleB(Bots, AllowMagicianEpicPet)) {
			c->Message(Chat::Yellow, "Epic pets are currently disabled for bots.");
			return;
		}

		pet_type = 6;
	}
	else if (!arg1.compare("current")) {
		current_check = true;
	}

	if (!current_check && pet_type == 255) {
		c->Message(
			Chat::Yellow,
			fmt::format(
				"You must specify a pet [type: auto | water | fire | air | earth | monster | epic]. Use {} for information regarding this command.",
				Saylink::Silent(
					fmt::format("{} help", sep->arg[0])
				)
			).c_str()
		);
		return;
	}

	const int ab_mask = ActionableBots::ABM_Type1;
	std::string actionableArg = sep->arg[ab_arg];

	if (actionableArg.empty()) {
		actionableArg = "target";
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

	std::string currentType;
	uint16 reclaim_energy_id = RuleI(Bots, ReclaimEnergySpellID);
	bool isSuccess = false;
	uint16 successCount = 0;
	Bot* firstFound = nullptr;

	for (auto bot_iter : sbl) {
		if (bot_iter->GetClass() != Class::Magician) {
			continue;
		}

		if (!bot_iter->IsInGroupOrRaid(c)) {
			continue;
		}

		if (pet_type == 6 && RuleI(Bots, RequiredMagicianEpicPetItemID) > 0) {
			bool has_item = bot_iter->HasBotItem(RuleI(Bots, RequiredMagicianEpicPetItemID)) != INVALID_INDEX;
			
			if (!has_item) {
				c->Message(
					Chat::Say,
					fmt::format(
						"{} says, 'I require {} to cast an epic pet which I do not currently possess.'",
						bot_iter->GetCleanName(),
						(database.GetItem(RuleI(Bots, RequiredMagicianEpicPetItemID)) ? database.CreateItemLink(RuleI(Bots, RequiredMagicianEpicPetItemID)) : "an item")
					).c_str()
				);

				continue;
			}	
		}

		if (current_check) {
			switch (bot_iter->GetPetChooserID()) {
				case 0:
					currentType = "auto";
					break;
				case SumWater:
					currentType = "water";
					break;
				case SumFire:
					currentType = "fire";
					break;
				case SumAir:
					currentType = "air";
					break;
				case SumEarth:
					currentType = "earth";
					break;
				case MonsterSum:
					currentType = "monster";
					break;
				case SumMageMultiElement:
					currentType = "epic";
					break;
				default:
					currentType = "null";
					break;
			}

			c->Message(
				Chat::Green,
				fmt::format(
					"{} says, 'I'm currently summoning {} pets.'",
					bot_iter->GetCleanName(),
					currentType
				).c_str()
			);

			continue;
		}

		uint8 airMinLevel = 255;
		uint8 fireMinLevel = 255;
		uint8 waterMinLevel = 255;
		uint8 earthMinLevel = 255;
		uint8 monsterMinLevel = 255;
		uint8 epicMinLevel = 255;
		std::list<BotSpell> botSpellList = bot_iter->GetBotSpellsBySpellType(bot_iter, BotSpellTypes::Pet);

		for (const auto& s : botSpellList) {
			if (!IsValidSpell(s.SpellId)) {
				continue;
			}

			if (!IsEffectInSpell(s.SpellId, SE_SummonPet)) {
				continue;
			}

			auto spell = spells[s.SpellId];

			if (!strncmp(spell.teleport_zone, "SumWater", 8) && spell.classes[Class::Magician - 1] < waterMinLevel) {
				waterMinLevel = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "SumFire", 7) && spell.classes[Class::Magician - 1] < fireMinLevel) {
				fireMinLevel = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "SumAir", 6) && spell.classes[Class::Magician - 1] < airMinLevel) {
				airMinLevel = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "SumEarth", 8) && spell.classes[Class::Magician - 1] < earthMinLevel) {
				earthMinLevel = spell.classes[Class::Magician - 1];
			}
			else if (!strncmp(spell.teleport_zone, "MonsterSum", 10) && spell.classes[Class::Magician - 1] < monsterMinLevel) {
				monsterMinLevel = spell.classes[Class::Magician - 1];
			}
		}

		uint8 minLevel = std::min({
			waterMinLevel,
			fireMinLevel,
			airMinLevel,
			earthMinLevel,
			monsterMinLevel
		});

		epicMinLevel = RuleI(Bots, AllowMagicianEpicPetLevel);

		LogTestDebug("{} says, 'minLevel = {} | waterMinLevel = {} | fireMinLevel = {} | airMinLevel = {} | earthMinLevel = {} | monsterMinLevel = {} | epicMinLevel = {}'",
			bot_iter->GetCleanName(),
			minLevel,
			waterMinLevel,
			fireMinLevel,
			airMinLevel,
			earthMinLevel,
			monsterMinLevel,
			epicMinLevel
		); //deleteme

		switch (pet_type) {
			case 0:
				level_req = minLevel;
				break;
			case SumWater:
				level_req = waterMinLevel;
				break;
			case SumFire:
				level_req = fireMinLevel;
				break;
			case SumAir:
				level_req = airMinLevel;
				break;
			case SumEarth:
				level_req = earthMinLevel;
				break;
			case MonsterSum:
				level_req = monsterMinLevel;
				break;
			case SumMageMultiElement:
				level_req = epicMinLevel;
				break;
			default:
				break;
		}

		if (bot_iter->GetLevel() < level_req) {
			continue;
		}

		bot_iter->SetPetChooserID(pet_type);

		if (bot_iter->GetPet()) {
			auto pet_id = bot_iter->GetPetID();
			bot_iter->SetPetID(0);
			bot_iter->CastSpell(reclaim_energy_id, pet_id);
		}

		if (!firstFound) {
			firstFound = bot_iter;
		}

		isSuccess = true;
		++successCount;
	}

	if (current_check) {
		return;
	}

	if (!isSuccess) {
		c->Message(Chat::Yellow, "No bots were selected.");

		return;
	}
	
	if (successCount == 1 && firstFound) {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} says, 'I will now summon {} pets.'",
				firstFound->GetCleanName(),
				currentType
			).c_str()
		);
	}
	else {
		c->Message(
			Chat::Green,
			fmt::format(
				"{} of your bots will now summon {} pets.",
				successCount,
				arg1
			).c_str()
		);
	}
}
