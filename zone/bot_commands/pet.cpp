#include "../bot_command.h"

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
	if (helper_command_alias_fail(c, "bot_command_pet_set_type", sep->arg[0], "petsettype"))
		return;
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: %s [type: auto | water | fire | air | earth | monster | epic] ([actionable: target | byname] ([actionable_name]))", sep->arg[0]);
		c->Message(Chat::White, "if set to 'auto', bots will choose their own pet type");
		c->Message(Chat::White, "requires one of the following bot classes:");
		c->Message(Chat::White, "Magician(1)");
		return;
	}
	int ab_mask = (ActionableBots::ABM_Target | ActionableBots::ABM_ByName); // this can be expanded without code modification

	std::string pet_arg = sep->arg[1];

	uint8 pet_type = 255;
	uint8 level_req = 255;
	if (!pet_arg.compare("auto")) {
		pet_type = 0;
		level_req = 1;
	}
	else if (!pet_arg.compare("water")) {
		pet_type = 1;
		level_req = 1;
	}
	else if (!pet_arg.compare("fire")) {
		pet_type = 2;
		level_req = 3;
	}
	else if (!pet_arg.compare("air")) {
		pet_type = 3;
		level_req = 4;
	}
	else if (!pet_arg.compare("earth")) {
		pet_type = 4;
		level_req = 5;
	}
	else if (!pet_arg.compare("monster")) {
		pet_type = 5;
		level_req = 30;
	}
	else if (!pet_arg.compare("epic")) {
		pet_type = 6;
		if (!RuleB(Bots, AllowMagicianEpicPet)) {
			c->Message(Chat::Yellow, "Epic pets are currently disabled for bots.");
			return;
		}
		level_req = RuleI(Bots,AllowMagicianEpicPetLevel);
	}

	if (pet_type == 255) {
		c->Message(Chat::White, "You must specify a pet [type: auto | water | fire | air | earth | monster | epic]");
		return;
	}

	std::list<Bot*> sbl;
	if (ActionableBots::PopulateSBL(c, sep->arg[2], sbl, ab_mask, sep->arg[3]) == ActionableBots::ABT_None)
		return;

	uint16 class_mask = player_class_bitmasks[Class::Magician];
	ActionableBots::Filter_ByClasses(c, sbl, class_mask);
	if (sbl.empty()) {
		c->Message(Chat::White, "You have no spawned Magician bots");
		return;
	}

	ActionableBots::Filter_ByMinLevel(c, sbl, level_req);
	if (sbl.empty()) {
		c->Message(Chat::White, "You have no spawned Magician bots capable of using this pet type: '%s'", pet_arg.c_str());
		return;
	}

	uint16 reclaim_energy_id = 331;
	for (auto bot_iter : sbl) {
		if (!bot_iter)
			continue;

		if (RuleI(Bots, RequiredMagicianEpicPetItemID) > 0) {
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

		bot_iter->SetPetChooserID(pet_type);
		if (bot_iter->GetPet()) {
			auto pet_id = bot_iter->GetPetID();
			bot_iter->SetPetID(0);
			bot_iter->CastSpell(reclaim_energy_id, pet_id);
		}
	}
}
