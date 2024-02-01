#include "../bot_command.h"

void bot_command_item_use(Client* c, const Seperator* sep)
{
	if (helper_is_help_or_usage(sep->arg[1])) {
		c->Message(Chat::White, "usage: [%s empty] will display only bots that can use the item in an empty slot.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s byclass classID] - Example: [%s byclass 7] will display only bots that match the class that can use the item. Example is a Monk, use [^create help] for a list of class IDs.", sep->arg[0], sep->arg[0]);
		c->Message(Chat::White, "usage: [%s casteronly] will display only caster bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s hybridonly] will display only hybrid bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s meleeonly] will display only melee bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s wiscasteronly] will display only Wisdom-based Caster bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s intcasteronly] will display only Intelligence-based Caster bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s plateonly] will display only Plate-wearing bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s chainonly] will display only Chain-wearing bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s leatheronly] will display only Leather-wearing bots that can use the item.", sep->arg[0]);
		c->Message(Chat::White, "usage: [%s clothonly] will display only Cloth-wearing bots that can use the item.", sep->arg[0]);
		return;
	}

	bool empty_only = false;
	int8 class_mask = 0;
	bool caster_only = false;
	bool hybrid_only = false;
	bool melee_only = false;
	bool wis_caster_only = false;
	bool int_caster_only = false;
	bool plate_only = false;
	bool chain_only = false;
	bool leather_only = false;
	bool cloth_only = false;

	std::string arg1 = sep->arg[1];
	std::string arg2 = sep->arg[2];
	if (arg1.compare("empty") == 0) {
		empty_only = true;
	}
	else if (arg1.compare("byclass") == 0) {
		if (Strings::IsNumber(sep->arg[2])) {
			class_mask = Strings::ToUnsignedInt(sep->arg[2]);
			if (!(class_mask >= Class::Warrior && class_mask <= Class::Berserker)) {
				c->Message(Chat::White, "Invalid class range, you must choose between 1 (Warrior) and 15 (Beastlord)");
				return;
			}
		}
	}
	else if (arg1.compare("casteronly") == 0) {
		caster_only = true;
	}
	else if (arg1.compare("hybridonly") == 0) {
		hybrid_only = true;
	}
	else if (arg1.compare("meleeonly") == 0) {
		melee_only = true;
	}
	else if (arg1.compare("wiscasteronly") == 0) {
		wis_caster_only = true;
	}
	else if (arg1.compare("intcasteronly") == 0) {
		int_caster_only = true;
	}
	else if (arg1.compare("plateonly") == 0) {
		plate_only = true;
	}
	else if (arg1.compare("chainonly") == 0) {
		chain_only = true;
	}
	else if (arg1.compare("leatheronly") == 0) {
		leather_only = true;
	}
	else if (arg1.compare("clothonly") == 0) {
		cloth_only = true;
	}
	else if (!arg1.empty()) {
		c->Message(Chat::White, "Please choose the correct subtype. For help use %s help.", sep->arg[0]);
		return;
	}
	const auto item_instance = c->GetInv().GetItem(EQ::invslot::slotCursor);
	if (!item_instance) {
		c->Message(Chat::White, "No item found on cursor!");
		return;
	}

	auto item_data = item_instance->GetItem();
	if (!item_data) {
		c->Message(Chat::White, "No data found for cursor item!");
		return;
	}

	if (item_data->ItemClass != EQ::item::ItemClassCommon || item_data->Slots == 0) {
		c->Message(Chat::White, "'%s' is not an equipable item!", item_data->Name);
		return;
	}

	std::vector<int16> equipable_slot_list;
	for (int16 equipable_slot = EQ::invslot::EQUIPMENT_BEGIN; equipable_slot <= EQ::invslot::EQUIPMENT_END; ++equipable_slot) {
		if (item_data->Slots & (1 << equipable_slot)) {
			equipable_slot_list.emplace_back(equipable_slot);
		}
	}

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemInst);

	std::list<Bot*> sbl;
	MyBots::PopulateSBL_BySpawnedBots(c, sbl);

	if (class_mask) {
		ActionableBots::Filter_ByClasses(c, sbl, GetPlayerClassBit(class_mask));
	}

	for (const auto& bot_iter : sbl) {
		if (!bot_iter) {
			continue;
		}
		if (caster_only && !IsCasterClass(bot_iter->GetClass())) {
			continue;
		}
		if (hybrid_only && !IsSpellFighterClass(bot_iter->GetClass())) {
			continue;
		}
		if (melee_only && !IsNonSpellFighterClass(bot_iter->GetClass())) {
			continue;
		}
		if (wis_caster_only && !IsWISCasterClass(bot_iter->GetClass())) {
			continue;
		}
		if (int_caster_only && !IsINTCasterClass(bot_iter->GetClass())) {
			continue;
		}
		if (plate_only && !IsPlateClass(bot_iter->GetClass())) {
			continue;
		}
		if (chain_only && !IsChainClass(bot_iter->GetClass())) {
			continue;
		}
		if (leather_only && !IsLeatherClass(bot_iter->GetClass())) {
			continue;
		}
		if (cloth_only && !IsClothClass(bot_iter->GetClass())) {
			continue;
		}
		if (((~item_data->Races) & GetPlayerRaceBit(bot_iter->GetRace())) || ((~item_data->Classes) & GetPlayerClassBit(bot_iter->GetClass()))) {
			continue;
		}

		for (const auto& slot_iter : equipable_slot_list) {
			// needs more failure criteria - this should cover the bulk for now
			if (slot_iter == EQ::invslot::slotSecondary && item_data->Damage && !bot_iter->CanThisClassDualWield()) {
				continue;
			}

			auto equipped_item = bot_iter->GetInv()[slot_iter];

			if (equipped_item && !empty_only) {
				linker.SetItemInst(equipped_item);

				c->Message(
					Chat::Say,
					fmt::format(
						"{} says, 'I can use that for my {} instead of my {}! Would you like to {} my {}?'",
						Saylink::Silent(
							fmt::format(
								"^inventorygive byname {}",
								bot_iter->GetCleanName()
							),
							bot_iter->GetCleanName()
						),
						EQ::invslot::GetInvPossessionsSlotName(slot_iter),
						linker.GenerateLink(),
						Saylink::Silent(
							fmt::format(
								"^inventoryremove {} byname {}",
								slot_iter,
								bot_iter->GetCleanName()
							),
							"remove"
						),
						linker.GenerateLink()
					).c_str()
				);

				bot_iter->DoAnim(29);
			}
			else if (!equipped_item) {
				c->Message(
					Chat::Say,
					fmt::format(
						"{} says, 'I can use that for my {}! Would you like to {} it to me?'",
						Saylink::Silent(
							fmt::format(
								"^inventorygive byname {}",
								bot_iter->GetCleanName()
							),
							bot_iter->GetCleanName()
						),
						EQ::invslot::GetInvPossessionsSlotName(slot_iter),
						Saylink::Silent(
							fmt::format(
								"^inventorygive byname {}",
								bot_iter->GetCleanName()
							),
							"give"
						)
					).c_str()
				);

				bot_iter->DoAnim(29);
			}
		}
	}
}
