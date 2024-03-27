#include "../common/global_define.h"
#include "../common/data_verification.h"

#include "../common/loot.h"
#include "client.h"
#include "entity.h"
#include "mob.h"
#include "npc.h"
#include "zonedb.h"
#include "global_loot_manager.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/repositories/global_loot_repository.h"
#include "quest_parser_collection.h"

#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

void NPC::AddLootTable(uint32 loottable_id, bool is_global)
{
	// check if it's a GM spawn
	if (!npctype_id) {
		return;
	}
	
	if (!is_global) {
		m_loot_copper   = 0;
		m_loot_silver   = 0;
		m_loot_gold     = 0;
		m_loot_platinum = 0;
	}

	zone->LoadLootTable(loottable_id);

	const auto *l = zone->GetLootTable(loottable_id);
	if (!l) {
		return;
	}

	LogLootDetail(
		"Attempting to load loot [{}] loottable [{}] ({}) is_global [{}]",
		GetCleanName(),
		loottable_id,
		l->name,
		is_global
	);

	auto content_flags = ContentFlags{
		.min_expansion = l->min_expansion,
		.max_expansion = l->max_expansion,
		.content_flags = l->content_flags,
		.content_flags_disabled = l->content_flags_disabled
	};

	if (!content_service.DoesPassContentFiltering(content_flags)) {
		return;
	}

	uint32 min_cash = l->mincash;
	uint32 max_cash = l->maxcash;
	if (min_cash > max_cash) {
		const uint32 t = min_cash;
		min_cash = max_cash;
		max_cash = t;
	}

	uint32 cash = 0;
	if (!is_global) {
		if (max_cash > 0 && l->avgcoin > 0 && EQ::ValueWithin(l->avgcoin, min_cash, max_cash)) {
			const float upper_chance  = static_cast<float>(l->avgcoin - min_cash) /
										static_cast<float>(max_cash - min_cash);
			const float avg_cash_roll = static_cast<float>(zone->random.Real(0.0, 1.0));

			if (avg_cash_roll < upper_chance) {
				cash = zone->random.Int(l->avgcoin, max_cash);
			}
			else {
				cash = zone->random.Int(min_cash, l->avgcoin);
			}
		}
		else {
			cash = zone->random.Int(min_cash, max_cash);
		}
	}

	if (cash != 0) {
		m_loot_platinum = cash / 1000;
		cash -= m_loot_platinum * 1000;

		m_loot_gold = cash / 100;
		cash -= m_loot_gold * 100;

		m_loot_silver = cash / 10;
		cash -= m_loot_silver * 10;

		m_loot_copper = cash;
	}

	const uint32 global_loot_multiplier = RuleI(Zone, GlobalLootMultiplier);
	for (auto    &lte: zone->GetLootTableEntries(loottable_id)) {
		for (uint32 k = 1; k <= (lte.multiplier * global_loot_multiplier); k++) {
			const uint8 drop_limit   = lte.droplimit;
			const uint8 minimum_drop = lte.mindrop;
			const float probability  = lte.probability;

			float drop_chance = 0.0f;
			if (EQ::ValueWithin(probability, 0.0f, 100.0f)) {
				drop_chance = static_cast<float>(zone->random.Real(0.0, 100.0));
			}

			if (probability != 0.0 && (probability == 100.0 || drop_chance <= probability)) {
				AddLootDropTable(lte.lootdrop_id, drop_limit, minimum_drop);
			}
		}
	}

	LogLootDetail(
		"Loaded [{}] Loot Table [{}] is_global [{}]",
		GetCleanName(),
		loottable_id,
		is_global
	);
}

void NPC::AddLootDropTable(uint32 lootdrop_id, uint8 drop_limit, uint8 min_drop)
{
	const auto l  = zone->GetLootdrop(lootdrop_id);
	const auto le = zone->GetLootdropEntries(lootdrop_id);

	auto content_flags = ContentFlags{
		.min_expansion = l.min_expansion,
		.max_expansion = l.max_expansion,
		.content_flags = l.content_flags,
		.content_flags_disabled = l.content_flags_disabled
	};

	if (l.id == 0 || le.empty() || !content_service.DoesPassContentFiltering(content_flags)) {
		return;
	}

	// if this lootdrop is droplimit=0 and mindrop 0, scan list once and return
	if (drop_limit == 0 && min_drop == 0) {
		for (const auto &e: le) {
			LogLootDetail(
				"-- NPC [{}] Lootdrop [{}] Item [{}] ({}_ Chance [{}] Multiplier [{}]",
				GetCleanName(),
				lootdrop_id,
				database.GetItem(e.item_id)->Name,
				e.item_id,
				e.chance,
				e.multiplier
			);
			for (int j = 0; j < e.multiplier; ++j) {
				if (zone->random.Real(0.0, 100.0) <= e.chance && MeetsLootDropLevelRequirements(e, true)) {							
					const EQ::ItemData *database_item = database.GetItem(e.item_id);
					AddLootDrop(database_item, e);
					LogLootDetail(
						"---- NPC (Rolled) [{}] Lootdrop [{}] Item [{}] ({}) Chance [{}] Multiplier [{}]",
						GetCleanName(),
						lootdrop_id,
						database_item->Name,
						e.item_id,
						e.chance,
						e.multiplier
					);
				}
			}
		}
		return;
	}

	if (le.size() > 100 && drop_limit == 0) {
		drop_limit = 10;
	}

	if (drop_limit < min_drop) {
		drop_limit = min_drop;
	}

	float roll_t                   = 0.0f;
	float no_loot_prob             = 1.0f;
	bool  roll_table_chance_bypass = false;
	bool  active_item_list         = false;

	for (const auto &e: le) {
		const EQ::ItemData *db_item = database.GetItem(e.item_id);
		if (db_item && MeetsLootDropLevelRequirements(e)) {
			roll_t += e.chance;

			if (e.chance >= 100) {
				roll_table_chance_bypass = true;
			}
			else {
				no_loot_prob *= (100 - e.chance) / 100.0f;
			}

			active_item_list = true;
		}
	}

	if (!active_item_list) {
		return;
	}

	// This will pick one item per iteration until mindrop.
	// Don't let the compare against chance fool you.
	// The roll isn't 0-100, its 0-total and it picks the item, we're just
	// looping to find the lucky item, descremening otherwise. This is ok,
	// items with chance 60 are 6 times more likely than items chance 10.
	int drops = 0;

	// translate above for loop using l and le
	for (int i = 0; i < drop_limit; ++i) {
		if (drops < min_drop || roll_table_chance_bypass || (float) zone->random.Real(0.0, 1.0) >= no_loot_prob) {
			float           roll = (float) zone->random.Real(0.0, roll_t);
			for (const auto &e: le) {
				const auto *db_item = database.GetItem(e.item_id);
				if (db_item) {
					// if it doesn't meet the requirements do nothing
					if (!MeetsLootDropLevelRequirements(e)) {
						continue;
					}

					if (roll < e.chance) {
						AddLootDrop(db_item, e);
						drops++;

						uint8 charges = e.multiplier;
						charges = EQ::ClampLower(charges, static_cast<uint8>(1));

						for (int k = 1; k < charges; ++k) {
							float c_roll = static_cast<float>(zone->random.Real(0.0, 100.0));
							if (c_roll <= e.chance) {
								AddLootDrop(db_item, e);
							}
						}

						break;
					}
					else {
						roll -= e.chance;
					}
				}
			}
		}
	}

	UpdateEquipmentLight();
}

bool NPC::MeetsLootDropLevelRequirements(LootdropEntriesRepository::LootdropEntries loot_drop, bool verbose)
{
	if (loot_drop.npc_min_level > 0 && GetLevel() < loot_drop.npc_min_level) {
		if (verbose) {
			LogLootDetail(
				"NPC [{}] does not meet loot_drop level requirements (min_level) level [{}] current [{}] for item [{}]",
				GetCleanName(),
				loot_drop.npc_min_level,
				GetLevel(),
				database.CreateItemLink(loot_drop.item_id)
			);
		}
		return false;
	}

	if (loot_drop.npc_max_level > 0 && GetLevel() > loot_drop.npc_max_level) {
		if (verbose) {
			LogLootDetail(
				"NPC [{}] does not meet loot_drop level requirements (max_level) level [{}] current [{}] for item [{}]",
				GetCleanName(),
				loot_drop.npc_max_level,
				GetLevel(),
				database.CreateItemLink(loot_drop.item_id)
			);
		}
		return false;
	}

	return true;
}

uint32 NPC::DoUpgradeLoot(uint32 itemID) {
	/* Affix Map (+ x000000)
	+1 Awakened
	+2 Radiant
	+10 Awakened + Hardy
	+11 Awakened + Robust
	+12 Awakened + Vital
	+13 Awakened + Mindful
	+14 Awakened + Mental
	+15 Awakened + Cerebral
	+16 Awakened + Enduring
	+17 Awakened + Stalwart
	+18 Awakened + Fortified
	+19 Awakened + Fierce
	+20 Awakened + Savage
	+21 Awakened + Vicious
	+22 Awakened + Esoteric
	+23 Awakened + Arcane
	+24 Awakened + Eldritch
	+25 Awakened + Soothing
	+26 Awakened + Mystic
	+27 Awakened + Vital
	+28 Awakened + Hardy + Forged or Wrought
	+29 Awakened + Hardy + Honed or Tempered
	+30 Awakened + Hardy + Polished or Brutal
	+31 Awakened + Robust + Forged or Wrought
	+32 Awakened + Robust + Honed or Tempered
	+33 Awakened + Robust + Polished or Brutal
	+34 Awakened + Vital + Forged or Wrought
	+35 Awakened + Vital + Honed or Tempered
	+36 Awakened + Vital + Polished or Brutal
	+37 Awakened + Mindful + Forged or Wrought
	+38 Awakened + Mindful + Honed or Tempered
	+39 Awakened + Mindful + Polished or Brutal
	+40 Awakened + Mental + Forged or Wrought
	+41 Awakened + Mental + Honed or Tempered
	+42 Awakened + Mental + Polished or Brutal
	+43 Awakened + Cerebral + Forged or Wrought
	+44 Awakened + Cerebral + Honed or Tempered
	+45 Awakened + Cerebral + Polished or Brutal
	+46 Awakened + Enduring + Forged or Wrought
	+47 Awakened + Enduring + Honed or Tempered
	+48 Awakened + Enduring + Polished or Brutal
	+49 Awakened + Stalwart + Forged or Wrought
	+50 Awakened + Stalwart + Honed or Tempered
	+51 Awakened + Stalwart + Polished or Brutal
	+52 Awakened + Fortified + Forged or Wrought
	+53 Awakened + Fortified + Honed or Tempered
	+54 Awakened + Fortified + Polished or Brutal
	+55 Awakened + Fierce + Forged or Wrought
	+56 Awakened + Fierce + Honed or Tempered
	+57 Awakened + Fierce + Polished or Brutal
	+58 Awakened + Savage + Forged or Wrought
	+59 Awakened + Savage + Honed or Tempered
	+60 Awakened + Savage + Polished or Brutal
	+61 Awakened + Vicious + Forged or Wrought
	+62 Awakened + Vicious + Honed or Tempered
	+63 Awakened + Vicious + Polished or Brutal
	+64 Awakened + Esoteric + Forged or Wrought
	+65 Awakened + Esoteric + Honed or Tempered
	+66 Awakened + Esoteric + Polished or Brutal
	+67 Awakened + Arcane + Forged or Wrought
	+68 Awakened + Arcane + Honed or Tempered
	+69 Awakened + Arcane + Polished or Brutal
	+70 Awakened + Eldritch + Forged or Wrought
	+71 Awakened + Eldritch + Honed or Tempered
	+72 Awakened + Eldritch + Polished or Brutal
	+73 Awakened + Soothing + Forged or Wrought
	+74 Awakened + Soothing + Honed or Tempered
	+75 Awakened + Soothing + Polished or Brutal
	+76 Awakened + Mystic + Forged or Wrought
	+77 Awakened + Mystic + Honed or Tempered
	+78 Awakened + Mystic + Polished or Brutal
	+79 Radiant + Hardy
	+80 Radiant + Robust
	+81 Radiant + Vital
	+82 Radiant + Mindful
	+83 Radiant + Mental
	+84 Radiant + Cerebral
	+85 Radiant + Enduring
	+86 Radiant + Stalwart
	+87 Radiant + Fortified
	+88 Radiant + Fierce
	+89 Radiant + Savage
	+90 Radiant + Vicious
	+91 Radiant + Esoteric
	+92 Radiant + Arcane
	+93 Radiant + Eldritch
	+94 Radiant + Soothing
	+95 Radiant + Mystic
	+96 Radiant + Hardy + Forged or Wrought
	+97 Radiant + Hardy + Honed or Tempered
	+98 Radiant + Hardy + Polished or Brutal
	+99 Radiant + Robust + Forged or Wrought
	+100 Radiant + Robust + Honed or Tempered
	+101 Radiant + Robust + Polished or Brutal
	+102 Radiant + Vital + Forged or Wrought
	+103 Radiant + Vital + Honed or Tempered
	+104 Radiant + Vital + Polished or Brutal
	+105 Radiant + Mindful + Forged or Wrought
	+106 Radiant + Mindful + Honed or Tempered
	+107 Radiant + Mindful + Polished or Brutal
	+108 Radiant + Mental + Forged or Wrought
	+109 Radiant + Mental + Honed or Tempered
	+110 Radiant + Mental + Polished or Brutal
	+111 Radiant + Cerebral + Forged or Wrought
	+112 Radiant + Cerebral + Honed or Tempered
	+113 Radiant + Cerebral + Polished or Brutal
	+114 Radiant + Enduring + Forged or Wrought
	+115 Radiant + Enduring + Honed or Tempered
	+116 Radiant + Enduring + Polished or Brutal
	+117 Radiant + Stalwart + Forged or Wrought
	+118 Radiant + Stalwart + Honed or Tempered
	+119 Radiant + Stalwart + Polished or Brutal
	+120 Radiant + Fortified + Forged or Wrought
	+121 Radiant + Fortified + Honed or Tempered
	+122 Radiant + Fortified + Polished or Brutal
	+123 Radiant + Fierce + Forged or Wrought
	+124 Radiant + Fierce + Honed or Tempered
	+125 Radiant + Fierce + Polished or Brutal
	+126 Radiant + Savage + Forged or Wrought
	+127 Radiant + Savage + Honed or Tempered
	+128 Radiant + Savage + Polished or Brutal
	+129 Radiant + Vicious + Forged or Wrought
	+130 Radiant + Vicious + Honed or Tempered
	+131 Radiant + Vicious + Polished or Brutal
	+132 Radiant + Esoteric + Forged or Wrought
	+133 Radiant + Esoteric + Honed or Tempered
	+134 Radiant + Esoteric + Polished or Brutal
	+135 Radiant + Arcane + Forged or Wrought
	+136 Radiant + Arcane + Honed or Tempered
	+137 Radiant + Arcane + Polished or Brutal
	+138 Radiant + Eldritch + Forged or Wrought
	+139 Radiant + Eldritch + Honed or Tempered
	+140 Radiant + Eldritch + Polished or Brutal
	+141 Radiant + Soothing + Forged or Wrought
	+142 Radiant + Soothing + Honed or Tempered
	+143 Radiant + Soothing + Polished or Brutal
	+144 Radiant + Mystic + Forged or Wrought
	+145 Radiant + Mystic + Honed or Tempered
	+146 Radiant + Mystic + Polished or Brutal
	*/

	if (RuleB(Custom, DoItemUpgrades)) {
		auto roll = zone->random.Real(0.0, 100.0); // Roll a number between 0 and 100
		auto newID = itemID;

		if (roll <= RuleR(Custom, Tier2ItemDropRate)) {
			newID += 2000000;
		} else if (roll <= RuleR(Custom, Tier1ItemDropRate)) {
			newID += 1000000;
		}

		if (database.GetItem(newID)) {
			itemID = newID;
		}
	}
	
	return itemID;
}

void NPC::AddLootDrop(
	const EQ::ItemData *item2,
	LootdropEntriesRepository::LootdropEntries loot_drop,
	bool wear_change,
	uint32 augment_one,
	uint32 augment_two,
	uint32 augment_three,
	uint32 augment_four,
	uint32 augment_five,
	uint32 augment_six
)
{
	if (RuleB(Custom, DoItemUpgrades)) {
		item2 = database.GetItem(DoUpgradeLoot(item2->ID));
	}
	
	AddLootDropFixed(item2, loot_drop, wear_change, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six);
}

//if itemlist is null, just send wear changes
void NPC::AddLootDropFixed(
	const EQ::ItemData *item2,
	LootdropEntriesRepository::LootdropEntries loot_drop,
	bool wear_change,
	uint32 augment_one,
	uint32 augment_two,
	uint32 augment_three,
	uint32 augment_four,
	uint32 augment_five,
	uint32 augment_six
)
{
	if (!item2) {
		return;
	}

	auto item = new LootItem;

	if (LogSys.log_settings[Logs::Loot].is_category_enabled == 1) {
		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);
		linker.SetItemData(item2);

		LogLoot(
			"NPC [{}] Item ({}) [{}] charges [{}] chance [{}] trivial min/max [{}/{}] npc min/max [{}/{}]",
			GetName(),
			item2->ID,
			linker.GenerateLink(),
			loot_drop.item_charges,
			loot_drop.chance,
			loot_drop.trivial_min_level,
			loot_drop.trivial_max_level,
			loot_drop.npc_min_level,
			loot_drop.npc_max_level
		);
	}

	EQApplicationPacket *outapp               = nullptr;
	WearChange_Struct   *p_wear_change_struct = nullptr;
	if (wear_change) {
		outapp               = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
		p_wear_change_struct = (WearChange_Struct *) outapp->pBuffer;
		p_wear_change_struct->spawn_id = GetID();
		p_wear_change_struct->material = 0;
	}

	item->item_id           = item2->ID;
	item->charges           = loot_drop.item_charges;
	item->aug_1             = augment_one;
	item->aug_2             = augment_two;
	item->aug_3             = augment_three;
	item->aug_4             = augment_four;
	item->aug_5             = augment_five;
	item->aug_6             = augment_six;
	item->attuned           = false;
	item->trivial_min_level = loot_drop.trivial_min_level;
	item->trivial_max_level = loot_drop.trivial_max_level;
	item->equip_slot        = EQ::invslot::SLOT_INVALID;

	// unsure if required to equip, YOLO for now
	if (item2->ItemType == EQ::item::ItemTypeBow) {
		SetBowEquipped(true);
	}

	if (item2->ItemType == EQ::item::ItemTypeArrow) {
		SetArrowEquipped(true);
	}

	bool found = false; // track if we found an empty slot we fit into

	int found_slot = INVALID_INDEX; // for multi-slot items

	auto *inst = database.CreateItem(
		item2->ID,
		loot_drop.item_charges,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six
	);

	if (!inst) {
		return;
	}

	if (loot_drop.equip_item > 0) {
		uint8              equipment_slot = UINT8_MAX;
		const EQ::ItemData *compitem      = nullptr;

		// Equip rules are as follows:
		// If the item has the NoPet flag set it will not be equipped.
		// An empty slot takes priority. The first empty one that an item can
		// fit into will be the one picked for the item.
		// AC is the primary choice for which item gets picked for a slot.
		// If AC is identical HP is considered next.
		// If an item can fit into multiple slots we'll pick the last one where
		// it is an improvement.

		if (!item2->NoPet) {
			for (int i = EQ::invslot::EQUIPMENT_BEGIN; !found && i <= EQ::invslot::EQUIPMENT_END; i++) {
				const uint32 slots = (1 << i);
				if (item2->Slots & slots) {
					if (equipment[i]) {
						compitem = database.GetItem(equipment[i]);
						if (item2->AC > compitem->AC || (item2->AC == compitem->AC && item2->HP > compitem->HP)) {
							// item would be an upgrade
							// check if we're multi-slot, if yes then we have to keep
							// looking in case any of the other slots we can fit into are empty.
							if (item2->Slots != slots) {
								found_slot = i;
							}
							else {
								// Unequip old item
								auto *old_item = GetItem(i);

								old_item->equip_slot = EQ::invslot::SLOT_INVALID;

								equipment[i] = item2->ID;

								found_slot = i;
								found      = true;
							}
						}
					}
					else {
						equipment[i] = item2->ID;

						found_slot = i;
						found      = true;
					}
				}
			}
		}

		// Possible slot was found but not selected. Pick it now.
		if (!found && found_slot >= 0) {
			equipment[found_slot] = item2->ID;

			found = true;
		}

		uint32 equipment_material;
		if (
			item2->Material <= 0 ||
			(
				item2->Slots & (
					(1 << EQ::invslot::slotPrimary) |
					(1 << EQ::invslot::slotSecondary)
				)
			)
			) {
			equipment_material = Strings::ToUnsignedInt(&item2->IDFile[2]);
		}
		else {
			equipment_material = item2->Material;
		}

		if (found_slot == EQ::invslot::slotPrimary) {
			equipment_slot = EQ::textures::weaponPrimary;

			if (item2->Damage > 0) {
				SendAddPlayerState(PlayerState::PrimaryWeaponEquipped);

				if (!RuleB(Combat, ClassicNPCBackstab)) {
					SetFacestab(true);
				}
			}

			if (item2->IsType2HWeapon()) {
				SetTwoHanderEquipped(true);
			}
		}
		else if (
			found_slot == EQ::invslot::slotSecondary &&
			(
				GetOwner() ||
				(CanThisClassDualWield() && zone->random.Roll(NPC_DW_CHANCE)) ||
				item2->Damage == 0
			) &&
			(
				item2->IsType1HWeapon() ||
				item2->ItemType == EQ::item::ItemTypeShield ||
				item2->ItemType == EQ::item::ItemTypeLight
			)
			) {
			equipment_slot = EQ::textures::weaponSecondary;

			if (item2->Damage > 0) {
				SendAddPlayerState(PlayerState::SecondaryWeaponEquipped);
			}
		}
		else if (found_slot == EQ::invslot::slotHead) {
			equipment_slot = EQ::textures::armorHead;
		}
		else if (found_slot == EQ::invslot::slotChest) {
			equipment_slot = EQ::textures::armorChest;
		}
		else if (found_slot == EQ::invslot::slotArms) {
			equipment_slot = EQ::textures::armorArms;
		}
		else if (EQ::ValueWithin(found_slot, EQ::invslot::slotWrist1, EQ::invslot::slotWrist2)) {
			equipment_slot = EQ::textures::armorWrist;
		}
		else if (found_slot == EQ::invslot::slotHands) {
			equipment_slot = EQ::textures::armorHands;
		}
		else if (found_slot == EQ::invslot::slotLegs) {
			equipment_slot = EQ::textures::armorLegs;
		}
		else if (found_slot == EQ::invslot::slotFeet) {
			equipment_slot = EQ::textures::armorFeet;
		}

		if (equipment_slot != UINT8_MAX) {
			if (wear_change) {
				p_wear_change_struct->wear_slot_id = equipment_slot;
				p_wear_change_struct->material     = equipment_material;
			}
		}

		if (found) {
			item->equip_slot = found_slot;
		}
	}

	if (found_slot != INVALID_INDEX) {
		GetInv().PutItem(found_slot, *inst);
	}

	if (parse->HasQuestSub(GetNPCTypeID(), EVENT_LOOT_ADDED)) {
		std::vector<std::any> args = {inst};
		parse->EventNPC(EVENT_LOOT_ADDED, this, nullptr, "", 0, &args);
	}

	m_loot_items.push_back(item);

	if (found) {
		CalcBonuses();
	}

	if (IsRecordLootStats()) {
		m_rolled_items.emplace_back(item->item_id);
	}

	if (wear_change && outapp) {
		entity_list.QueueClients(this, outapp);
		safe_delete(outapp);
	}

	UpdateEquipmentLight();

	if (UpdateActiveLight()) {
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}

	safe_delete(inst);
}

void NPC::AddItem(const EQ::ItemData *item, uint16 charges, bool equip_item)
{
	auto l = LootdropEntriesRepository::NewNpcEntity();

	l.equip_item   = static_cast<uint8>(equip_item ? 1 : 0);
	l.item_charges = charges;

	AddLootDrop(item, l, true);
}

void NPC::AddItem(
	uint32 item_id,
	uint16 charges,
	bool equip_item,
	uint32 augment_one,
	uint32 augment_two,
	uint32 augment_three,
	uint32 augment_four,
	uint32 augment_five,
	uint32 augment_six
)
{
	const auto *item = database.GetItem(item_id);
	if (!item) {
		return;
	}

	auto l = LootdropEntriesRepository::NewNpcEntity();

	l.equip_item   = static_cast<uint8>(equip_item ? 1 : 0);
	l.item_charges = charges;

	AddLootDrop(
		item,
		l,
		true,
		augment_one,
		augment_two,
		augment_three,
		augment_four,
		augment_five,
		augment_six
	);
}

void NPC::AddLootTable()
{
	AddLootTable(m_loottable_id);
}

void NPC::CheckGlobalLootTables()
{
	const auto &l = zone->GetGlobalLootTables(this);
	for (const auto &e: l) {
		AddLootTable(e, true);
	}
}

void ZoneDatabase::LoadGlobalLoot()
{
	const auto &l = GlobalLootRepository::GetWhere(
		*this,
		fmt::format(
			"`enabled` = 1 {}",
			ContentFilterCriteria::apply()
		)
	);

	if (l.empty()) {
		return;
	}

	LogInfo(
		"Loaded [{}] Global Loot Entr{}.",
		Strings::Commify(l.size()),
		l.size() != 1 ? "ies" : "y"
	);

	const std::string &zone_id = std::to_string(zone->GetZoneID());

	for (const auto &e: l) {
		if (!e.zone.empty()) {
			const auto &zones = Strings::Split(e.zone, "|");

			if (!Strings::Contains(zones, zone_id)) {
				continue;
			}
		}

		GlobalLootEntry gle(e.id, e.loottable_id, e.description);

		if (e.min_level) {
			gle.AddRule(GlobalLoot::RuleTypes::LevelMin, e.min_level);
		}

		if (e.max_level) {
			gle.AddRule(GlobalLoot::RuleTypes::LevelMax, e.max_level);
		}

		if (e.rare) {
			gle.AddRule(GlobalLoot::RuleTypes::Rare, e.rare);
		}

		if (e.raid) {
			gle.AddRule(GlobalLoot::RuleTypes::Raid, e.raid);
		}

		if (!e.race.empty()) {
			const auto &races = Strings::Split(e.race, "|");

			for (const auto &r: races) {
				gle.AddRule(GlobalLoot::RuleTypes::Race, Strings::ToInt(r));
			}
		}

		if (!e.class_.empty()) {
			const auto &classes = Strings::Split(e.class_, "|");

			for (const auto &c: classes) {
				gle.AddRule(GlobalLoot::RuleTypes::Class, Strings::ToInt(c));
			}
		}

		if (!e.bodytype.empty()) {
			const auto &bodytypes = Strings::Split(e.bodytype, "|");

			for (const auto &b: bodytypes) {
				gle.AddRule(GlobalLoot::RuleTypes::BodyType, Strings::ToInt(b));
			}
		}

		if (e.hot_zone) {
			gle.AddRule(GlobalLoot::RuleTypes::HotZone, e.hot_zone);
		}

		zone->AddGlobalLootEntry(gle);
	}
}


LootItem *NPC::GetItem(int slot_id)
{
	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		if (item->equip_slot == slot_id) {
			return item;
		}
	}
	return (nullptr);
}

void NPC::RemoveItem(uint32 item_id, uint16 quantity, uint16 slot)
{
	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for(; cur != end; ++cur) {
		LootItem *item = *cur;
		if (item->item_id == item_id && slot <= 0 && quantity <= 0) {			
			m_loot_items.erase(cur);
			UpdateEquipmentLight();
			if (UpdateActiveLight()) { SendAppearancePacket(AppearanceType::Light, GetActiveLightType()); }
			if (item->equip_slot >= EQ::invslot::EQUIPMENT_BEGIN && item->equip_slot <= EQ::invslot::EQUIPMENT_END) {
				equipment[item->equip_slot] = 0;
				SendWearChange(EQ::InventoryProfile::CalcMaterialFromSlot(item->equip_slot));
				GetInv().DeleteItem(item->equip_slot);
			}
			CalcBonuses();
			return;
		}
		else if (item->item_id == item_id && item->equip_slot == slot && quantity >= 1) {
			if (item->charges <= quantity) {				
				m_loot_items.erase(cur);
				UpdateEquipmentLight();
				if (UpdateActiveLight()) { SendAppearancePacket(AppearanceType::Light, GetActiveLightType()); }
				if (item->equip_slot >= EQ::invslot::EQUIPMENT_BEGIN && item->equip_slot <= EQ::invslot::EQUIPMENT_END) {
					equipment[item->equip_slot] = 0;
					SendWearChange(EQ::InventoryProfile::CalcMaterialFromSlot(item->equip_slot));
					GetInv().DeleteItem(item->equip_slot);
				}
				CalcBonuses();
			}
			else {
				item->charges -= quantity;
			}
			return;
		}
	}	
}



void NPC::CheckTrivialMinMaxLevelDrop(Mob *killer)
{
	if (killer == nullptr || !killer->IsClient()) {
		return;
	}

	uint16 killer_level = killer->GetLevel();
	uint8  material;

	auto cur = m_loot_items.begin();
	while (cur != m_loot_items.end()) {
		if (!(*cur)) {
			return;
		}

		uint16 trivial_min_level     = (*cur)->trivial_min_level;
		uint16 trivial_max_level     = (*cur)->trivial_max_level;
		bool   fits_trivial_criteria = (
			(trivial_min_level > 0 && killer_level < trivial_min_level) ||
			(trivial_max_level > 0 && killer_level > trivial_max_level)
		);

		if (fits_trivial_criteria) {
			material = EQ::InventoryProfile::CalcMaterialFromSlot((*cur)->equip_slot);
			if (material != EQ::textures::materialInvalid) {
				SendWearChange(material);
			}

			cur = m_loot_items.erase(cur);
			continue;
		}
		++cur;
	}

	UpdateEquipmentLight();
	if (UpdateActiveLight()) {
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}
}

void NPC::ClearLootItems()
{
	LootItems::iterator cur, end;
	cur = m_loot_items.begin();
	end = m_loot_items.end();
	for (; cur != end; ++cur) {
		LootItem *item = *cur;
		safe_delete(item);
	}
	m_loot_items.clear();

	UpdateEquipmentLight();
	if (UpdateActiveLight()) {
		SendAppearancePacket(AppearanceType::Light, GetActiveLightType());
	}
}

void NPC::QueryLoot(Client *to, bool is_pet_query)
{
	if (!m_loot_items.empty()) {
		if (!is_pet_query) {
			to->Message(
				Chat::White,
				fmt::format(
					"Loot | {} ({}) ID: {} Loottable ID: {}",
					GetName(),
					GetID(),
					GetNPCTypeID(),
					GetLoottableID()
				).c_str()
			);
		}

		int item_count = 0;

		for (auto current_item: m_loot_items) {
			int item_number = (item_count + 1);
			if (!current_item) {
				LogError("ItemList error, null item.");
				continue;
			}

			if (!current_item->item_id || !database.GetItem(current_item->item_id)) {
				LogError("Database error, invalid item.");
				continue;
			}

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkLootItem);
			linker.SetLootData(current_item);

			to->Message(
				Chat::White,
				fmt::format(
					"Item {} | {} ({}){}",
					item_number,
					linker.GenerateLink().c_str(),
					current_item->item_id,
					(
						current_item->charges > 1 ?
							fmt::format(
								" Amount: {}",
								current_item->charges
							) :
							""
					)
				).c_str()
			);
			item_count++;
		}
	}

	if (!is_pet_query) {
		if (m_loot_platinum || m_loot_gold || m_loot_silver || m_loot_copper) {
			to->Message(
				Chat::White,
				fmt::format(
					"Money | {}",
					Strings::Money(
						m_loot_platinum,
						m_loot_gold,
						m_loot_silver,
						m_loot_copper
					)
				).c_str()
			);
		}
	}
}

bool NPC::HasItem(uint32 item_id)
{
	if (!database.GetItem(item_id)) {
		return false;
	}

	for (auto loot_item: m_loot_items) {
		if (!loot_item) {
			LogError("NPC::HasItem() - ItemList error, null item");
			continue;
		}

		if (!loot_item->item_id || !database.GetItem(loot_item->item_id)) {
			LogError("NPC::HasItem() - Database error, invalid item");
			continue;
		}

		if (loot_item->item_id == item_id) {
			return true;
		}
	}
	return false;
}

uint16 NPC::CountItem(uint32 item_id)
{
	uint16 item_count = 0;
	if (!database.GetItem(item_id)) {
		return item_count;
	}

	for (auto loot_item: m_loot_items) {
		if (!loot_item) {
			LogError("NPC::CountItem() - ItemList error, null item");
			continue;
		}

		if (!loot_item->item_id || !database.GetItem(loot_item->item_id)) {
			LogError("NPC::CountItem() - Database error, invalid item");
			continue;
		}

		if (loot_item->item_id == item_id) {
			item_count += loot_item->charges > 0 ? loot_item->charges : 1;
		}
	}
	return item_count;
}

uint32 NPC::GetLootItemIDBySlot(uint16 loot_slot)
{
	for (auto loot_item: m_loot_items) {
		if (loot_item->lootslot == loot_slot) {
			return loot_item->item_id;
		}
	}
	return 0;
}

uint16 NPC::GetFirstLootSlotByItemID(uint32 item_id)
{
	for (auto loot_item: m_loot_items) {
		if (loot_item->item_id == item_id) {
			return loot_item->lootslot;
		}
	}
	return 0;
}

void NPC::AddLootCash(
	uint32 in_copper,
	uint32 in_silver,
	uint32 in_gold,
	uint32 in_platinum
)
{
	m_loot_copper   = in_copper >= 0 ? in_copper : 0;
	m_loot_silver   = in_silver >= 0 ? in_silver : 0;
	m_loot_gold     = in_gold >= 0 ? in_gold : 0;
	m_loot_platinum = in_platinum >= 0 ? in_platinum : 0;
}

void NPC::RemoveLootCash()
{
	m_loot_copper   = 0;
	m_loot_silver   = 0;
	m_loot_gold     = 0;
	m_loot_platinum = 0;
}
