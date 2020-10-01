/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "../common/loottable.h"
#include "../common/misc_functions.h"
#include "../common/data_verification.h"

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "npc.h"
#include "zonedb.h"
#include "zone_store.h"
#include "global_loot_manager.h"
#include "../common/repositories/criteria/content_filter_criteria.h"
#include "../common/say_link.h"

#include <iostream>
#include <stdlib.h>

#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

// Queries the loottable: adds item & coin to the npc
void ZoneDatabase::AddLootTableToNPC(NPC* npc,uint32 loottable_id, ItemList* itemlist, uint32* copper, uint32* silver, uint32* gold, uint32* plat) {
	const LootTable_Struct* lts = nullptr;
	// global loot passes nullptr for these
	bool bGlobal = copper == nullptr && silver == nullptr && gold == nullptr && plat == nullptr;
	if (!bGlobal) {
		*copper = 0;
		*silver = 0;
		*gold = 0;
		*plat = 0;
	}

	lts = database.GetLootTable(loottable_id);
	if (!lts)
		return;

	uint32 min_cash = lts->mincash;
	uint32 max_cash = lts->maxcash;
	if(min_cash > max_cash) {
		uint32 t = min_cash;
		min_cash = max_cash;
		max_cash = t;
	}

	uint32 cash = 0;
	if (!bGlobal) {
		if(max_cash > 0 && lts->avgcoin > 0 && EQ::ValueWithin(lts->avgcoin, min_cash, max_cash)) {
			float upper_chance = (float)(lts->avgcoin - min_cash) / (float)(max_cash - min_cash);
			float avg_cash_roll = (float)zone->random.Real(0.0, 1.0);

			if(avg_cash_roll < upper_chance) {
				cash = zone->random.Int(lts->avgcoin, max_cash);
			} else {
				cash = zone->random.Int(min_cash, lts->avgcoin);
			}
		} else {
			cash = zone->random.Int(min_cash, max_cash);
		}
	}

	if(cash != 0) {
		*plat = cash / 1000;
		cash -= *plat * 1000;

		*gold = cash / 100;
		cash -= *gold * 100;

		*silver = cash / 10;
		cash -= *silver * 10;

		*copper = cash;
	}

	uint32 global_loot_multiplier = RuleI(Zone, GlobalLootMultiplier);

	// Do items
	for (uint32 i=0; i<lts->NumEntries; i++) {
		for (uint32 k = 1; k <= (lts->Entries[i].multiplier * global_loot_multiplier); k++) {
			uint8 droplimit = lts->Entries[i].droplimit;
			uint8 mindrop = lts->Entries[i].mindrop;

			//LootTable Entry probability
			float ltchance = 0.0f;
			ltchance = lts->Entries[i].probability;

			float drop_chance = 0.0f;
			if(ltchance > 0.0 && ltchance < 100.0) {
				drop_chance = (float)zone->random.Real(0.0, 100.0);
			}

			if (ltchance != 0.0 && (ltchance == 100.0 || drop_chance <= ltchance)) {
				AddLootDropToNPC(npc, lts->Entries[i].lootdrop_id, itemlist, droplimit, mindrop);
			}
		}
	}
}

// Called by AddLootTableToNPC
// maxdrops = size of the array npcd
void ZoneDatabase::AddLootDropToNPC(NPC *npc, uint32 lootdrop_id, ItemList *item_list, uint8 droplimit, uint8 mindrop)
{
	const LootDrop_Struct *loot_drop = GetLootDrop(lootdrop_id);
	if (!loot_drop) {
		return;
	}

	if (loot_drop->NumEntries == 0) {
		return;
	}

	if (droplimit == 0 && mindrop == 0) {
		for (uint32 i = 0; i < loot_drop->NumEntries; ++i) {
			int      charges = loot_drop->Entries[i].multiplier;
			for (int j       = 0; j < charges; ++j) {
				if (zone->random.Real(0.0, 100.0) <= loot_drop->Entries[i].chance &&
					npc->MeetsLootDropLevelRequirements(loot_drop->Entries[i])) {
					const EQ::ItemData *database_item = GetItem(loot_drop->Entries[i].item_id);
					npc->AddLootDrop(
						database_item,
						item_list,
						loot_drop->Entries[i]
					);
				}
			}
		}
		return;
	}

	if (loot_drop->NumEntries > 100 && droplimit == 0) {
		droplimit = 10;
	}

	if (droplimit < mindrop) {
		droplimit = mindrop;
	}

	float       roll_t           = 0.0f;
	float       roll_t_min       = 0.0f;
	bool        active_item_list = false;
	for (uint32 i                = 0; i < loot_drop->NumEntries; ++i) {
		const EQ::ItemData *db_item = GetItem(loot_drop->Entries[i].item_id);
		if (db_item) {
			roll_t += loot_drop->Entries[i].chance;
			active_item_list = true;
		}
	}

	roll_t_min = roll_t;
	roll_t     = EQ::ClampLower(roll_t, 100.0f);

	if (!active_item_list) {
		return;
	}

	for (int i = 0; i < mindrop; ++i) {
		float       roll = (float) zone->random.Real(0.0, roll_t_min);
		for (uint32 j    = 0; j < loot_drop->NumEntries; ++j) {
			const EQ::ItemData *db_item = GetItem(loot_drop->Entries[j].item_id);
			if (db_item) {
				if (roll < loot_drop->Entries[j].chance && npc->MeetsLootDropLevelRequirements(loot_drop->Entries[j])) {
					npc->AddLootDrop(
						db_item,
						item_list,
						loot_drop->Entries[j]
					);

					int charges = (int) loot_drop->Entries[i].multiplier;
					charges = EQ::ClampLower(charges, 1);

					for (int k = 1; k < charges; ++k) {
						float c_roll = (float) zone->random.Real(0.0, 100.0);
						if (c_roll <= loot_drop->Entries[i].chance) {
							npc->AddLootDrop(
								db_item,
								item_list,
								loot_drop->Entries[i]
							);
						}
					}

					j = loot_drop->NumEntries;
					break;
				}
				else {
					roll -= loot_drop->Entries[j].chance;
				}
			}
		}
	}

	for (int i = mindrop; i < droplimit; ++i) {
		float       roll = (float) zone->random.Real(0.0, roll_t);
		for (uint32 j    = 0; j < loot_drop->NumEntries; ++j) {
			const EQ::ItemData *db_item = GetItem(loot_drop->Entries[j].item_id);
			if (db_item) {
				if (roll < loot_drop->Entries[j].chance && npc->MeetsLootDropLevelRequirements(loot_drop->Entries[j])) {
					npc->AddLootDrop(
						db_item,
						item_list,
						loot_drop->Entries[j]
					);

					int charges = (int) loot_drop->Entries[i].multiplier;
					charges = EQ::ClampLower(charges, 1);

					for (int k = 1; k < charges; ++k) {
						float c_roll = (float) zone->random.Real(0.0, 100.0);
						if (c_roll <= loot_drop->Entries[i].chance) {
							npc->AddLootDrop(
								db_item,
								item_list,
								loot_drop->Entries[i]
							);
						}
					}

					j = loot_drop->NumEntries;
					break;
				}
				else {
					roll -= loot_drop->Entries[j].chance;
				}
			}
		}
	} // We either ran out of items or reached our limit.

	npc->UpdateEquipmentLight();
	// no wearchange associated with this function..so, this should not be needed
	//if (npc->UpdateActiveLightValue())
	//	npc->SendAppearancePacket(AT_Light, npc->GetActiveLightValue());
}

bool NPC::MeetsLootDropLevelRequirements(LootDropEntries_Struct loot_drop)
{
	if (loot_drop.npc_min_level > 0 && GetLevel() < loot_drop.npc_min_level) {
		LogLootDetail(
			"NPC [{}] does not meet loot_drop level requirements (min_level) level [{}] current [{}] for item [{}]",
			GetCleanName(),
			loot_drop.npc_min_level,
			GetLevel(),
			database.CreateItemLink(loot_drop.item_id)
		);
		return false;
	}

	if (loot_drop.npc_max_level > 0 && GetLevel() > loot_drop.npc_max_level) {
		LogLootDetail(
			"NPC [{}] does not meet loot_drop level requirements (max_level) level [{}] current [{}] for item [{}]",
			GetCleanName(),
			loot_drop.npc_max_level,
			GetLevel(),
			database.CreateItemLink(loot_drop.item_id)
		);
		return false;
	}

	return true;
}

LootDropEntries_Struct NPC::NewLootDropEntry()
{
	LootDropEntries_Struct loot_drop{};
	loot_drop.item_id           = 0;
	loot_drop.item_charges      = 1;
	loot_drop.equip_item        = 1;
	loot_drop.chance            = 0;
	loot_drop.trivial_min_level = 0;
	loot_drop.trivial_max_level = 0;
	loot_drop.npc_min_level     = 0;
	loot_drop.npc_max_level     = 0;
	loot_drop.multiplier        = 0;

	return loot_drop;
}

//if itemlist is null, just send wear changes
void NPC::AddLootDrop(
	const EQ::ItemData *item2,
	ItemList *itemlist,
	LootDropEntries_Struct loot_drop,
	bool wear_change,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6
)
{
	if (item2 == nullptr) {
		return;
	}

	//make sure we are doing something...
	if (!itemlist && !wear_change) {
		return;
	}

	auto item = new ServerLootItem_Struct;

	if (LogSys.log_settings[Logs::Loot].is_category_enabled == 1) {
		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);
		linker.SetItemData(item2);

		LogLoot(
			"[NPC::AddLootDrop] NPC [{}] Item ({}) [{}] charges [{}] chance [{}] trivial min/max [{}/{}] npc min/max [{}/{}]",
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
	item->aug_1             = aug1;
	item->aug_2             = aug2;
	item->aug_3             = aug3;
	item->aug_4             = aug4;
	item->aug_5             = aug5;
	item->aug_6             = aug6;
	item->attuned           = 0;
	item->trivial_min_level = loot_drop.trivial_min_level;
	item->trivial_max_level = loot_drop.trivial_max_level;
	item->equip_slot        = EQ::invslot::SLOT_INVALID;

	if (loot_drop.equip_item > 0) {
		uint8 eslot = 0xFF;
		char newid[20];
		const EQ::ItemData* compitem = nullptr;
		bool found = false; // track if we found an empty slot we fit into
		int32 foundslot = -1; // for multi-slot items

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
				uint32 slots = (1 << i);
				if (item2->Slots & slots) {
					if(equipment[i])
					{
						compitem = database.GetItem(equipment[i]);
						if (item2->AC > compitem->AC ||
							(item2->AC == compitem->AC && item2->HP > compitem->HP))
						{
							// item would be an upgrade
							// check if we're multi-slot, if yes then we have to keep
							// looking in case any of the other slots we can fit into are empty.
							if (item2->Slots != slots) {
								foundslot = i;
							}
							else {
								equipment[i] = item2->ID;
								foundslot = i;
								found = true;
							}
						} // end if ac
					}
					else
					{
						equipment[i] = item2->ID;
						foundslot = i;
						found = true;
					}
				} // end if (slots)
			} // end for
		} // end if NoPet

		// Possible slot was found but not selected. Pick it now.
		if (!found && foundslot >= 0) {
			equipment[foundslot] = item2->ID;
			found = true;
		}

		// @merth: IDFile size has been increased, this needs to change
		uint16 emat;
		if(item2->Material <= 0
			|| (item2->Slots & ((1 << EQ::invslot::slotPrimary) | (1 << EQ::invslot::slotSecondary)))) {
			memset(newid, 0, sizeof(newid));
			for(int i=0;i<7;i++){
				if (!isalpha(item2->IDFile[i])){
					strn0cpy(newid, &item2->IDFile[i],6);
					i=8;
				}
			}

			emat = atoi(newid);
		} else {
			emat = item2->Material;
		}

		if (foundslot == EQ::invslot::slotPrimary) {
			if (item2->Proc.Effect != 0)
				CastToMob()->AddProcToWeapon(item2->Proc.Effect, true);

			eslot = EQ::textures::weaponPrimary;
			if (item2->Damage > 0) {
				SendAddPlayerState(PlayerState::PrimaryWeaponEquipped);
				if (!RuleB(Combat, ClassicNPCBackstab))
					SetFacestab(true);
			}
			if (item2->IsType2HWeapon())
				SetTwoHanderEquipped(true);
		}
		else if (foundslot == EQ::invslot::slotSecondary
			&& (GetOwner() != nullptr || (CanThisClassDualWield() && zone->random.Roll(NPC_DW_CHANCE)) || (item2->Damage==0)) &&
			(item2->IsType1HWeapon() || item2->ItemType == EQ::item::ItemTypeShield || item2->ItemType ==  EQ::item::ItemTypeLight))
		{
			if (item2->Proc.Effect!=0)
				CastToMob()->AddProcToWeapon(item2->Proc.Effect, true);

			eslot = EQ::textures::weaponSecondary;
			if (item2->Damage > 0)
				SendAddPlayerState(PlayerState::SecondaryWeaponEquipped);
		}
		else if (foundslot == EQ::invslot::slotHead) {
			eslot = EQ::textures::armorHead;
		}
		else if (foundslot == EQ::invslot::slotChest) {
			eslot = EQ::textures::armorChest;
		}
		else if (foundslot == EQ::invslot::slotArms) {
			eslot = EQ::textures::armorArms;
		}
		else if (foundslot == EQ::invslot::slotWrist1 || foundslot == EQ::invslot::slotWrist2) {
			eslot = EQ::textures::armorWrist;
		}
		else if (foundslot == EQ::invslot::slotHands) {
			eslot = EQ::textures::armorHands;
		}
		else if (foundslot == EQ::invslot::slotLegs) {
			eslot = EQ::textures::armorLegs;
		}
		else if (foundslot == EQ::invslot::slotFeet) {
			eslot = EQ::textures::armorFeet;
		}

		/*
		what was this about???

		if (((npc->GetRace()==127) && (npc->CastToMob()->GetOwnerID()!=0)) && (item2->Slots==24576) || (item2->Slots==8192) || (item2->Slots==16384)){
			npc->d_melee_texture2=atoi(newid);
			wc->wear_slot_id=8;
			if (item2->Material >0)
				wc->material=item2->Material;
			else
				wc->material=atoi(newid);
			npc->AC+=item2->AC;
			npc->STR+=item2->STR;
			npc->INT+=item2->INT;
		}
		*/

		//if we found an open slot it goes in...
		if(eslot != 0xFF) {
			if(wear_change) {
				p_wear_change_struct->wear_slot_id = eslot;
				p_wear_change_struct->material     = emat;
			}

		}
		if (found) {
			CalcBonuses(); // This is less than ideal for bulk adding of items
			item->equip_slot = foundslot;
		}
	}

	if (itemlist != nullptr) {
		itemlist->push_back(item);
	}
	else safe_delete(item);

	if (wear_change && outapp) {
		entity_list.QueueClients(this, outapp);
		safe_delete(outapp);
	}

	UpdateEquipmentLight();
	if (UpdateActiveLight()) {
		SendAppearancePacket(AT_Light, GetActiveLightType());
	}
}

void NPC::AddItem(const EQ::ItemData *item, uint16 charges, bool equipitem)
{
	//slot isnt needed, its determined from the item.
	auto loot_drop_entry = NPC::NewLootDropEntry();
	loot_drop_entry.equip_item   = static_cast<uint8>(equipitem ? 1 : 0);
	loot_drop_entry.item_charges = charges;

	AddLootDrop(item, &itemlist, loot_drop_entry, true);
}

void NPC::AddItem(
	uint32 itemid,
	uint16 charges,
	bool equipitem,
	uint32 aug1,
	uint32 aug2,
	uint32 aug3,
	uint32 aug4,
	uint32 aug5,
	uint32 aug6
)
{
	//slot isnt needed, its determined from the item.
	const EQ::ItemData *i = database.GetItem(itemid);
	if (i == nullptr) {
		return;
	}

	auto loot_drop_entry = NPC::NewLootDropEntry();
	loot_drop_entry.equip_item   = static_cast<uint8>(equipitem ? 1 : 0);
	loot_drop_entry.item_charges = charges;

	AddLootDrop(i, &itemlist, loot_drop_entry, true, aug1, aug2, aug3, aug4, aug5, aug6);
}

void NPC::AddLootTable() {
	if (npctype_id != 0) { // check if it's a GM spawn
		database.AddLootTableToNPC(this,loottable_id, &itemlist, &copper, &silver, &gold, &platinum);
	}
}

void NPC::AddLootTable(uint32 ldid) {
	if (npctype_id != 0) { // check if it's a GM spawn
	  database.AddLootTableToNPC(this,ldid, &itemlist, &copper, &silver, &gold, &platinum);
	}
}

void NPC::CheckGlobalLootTables()
{
	auto tables = zone->GetGlobalLootTables(this);

	for (auto &id : tables)
		database.AddLootTableToNPC(this, id, &itemlist, nullptr, nullptr, nullptr, nullptr);
}

void ZoneDatabase::LoadGlobalLoot()
{
	auto query = fmt::format(
		SQL
		(
			SELECT
			  id,
			  loottable_id,
			  description,
			  min_level,
			  max_level,
			  rare,
			  raid,
			  race,
			  class,
			  bodytype,
			  zone,
			  hot_zone
			FROM
			  global_loot
			WHERE
			  enabled = 1
			  {}
		),
		ContentFilterCriteria::apply()
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() == 0) {
		return;
	}

	// we might need this, lets not keep doing it in a loop
	auto      zoneid = std::to_string(zone->GetZoneID());
	for (auto row    = results.begin(); row != results.end(); ++row) {
		// checking zone limits
		if (row[10]) {
			auto zones = SplitString(row[10], '|');

			auto it = std::find(zones.begin(), zones.end(), zoneid);
			if (it == zones.end()) {  // not in here, skip
				continue;
			}
		}

		GlobalLootEntry e(atoi(row[0]), atoi(row[1]), row[2] ? row[2] : "");

		auto min_level = atoi(row[3]);
		if (min_level) {
			e.AddRule(GlobalLoot::RuleTypes::LevelMin, min_level);
		}

		auto max_level = atoi(row[4]);
		if (max_level) {
			e.AddRule(GlobalLoot::RuleTypes::LevelMax, max_level);
		}

		// null is not used
		if (row[5]) {
			e.AddRule(GlobalLoot::RuleTypes::Rare, atoi(row[5]));
		}

		// null is not used
		if (row[6]) {
			e.AddRule(GlobalLoot::RuleTypes::Raid, atoi(row[6]));
		}

		if (row[7]) {
			auto races = SplitString(row[7], '|');

			for (auto &r : races)
				e.AddRule(GlobalLoot::RuleTypes::Race, std::stoi(r));
		}

		if (row[8]) {
			auto classes = SplitString(row[8], '|');

			for (auto &c : classes)
				e.AddRule(GlobalLoot::RuleTypes::Class, std::stoi(c));
		}

		if (row[9]) {
			auto bodytypes = SplitString(row[9], '|');

			for (auto &b : bodytypes)
				e.AddRule(GlobalLoot::RuleTypes::BodyType, std::stoi(b));
		}

		// null is not used
		if (row[11]) {
			e.AddRule(GlobalLoot::RuleTypes::HotZone, atoi(row[11]));
		}

		zone->AddGlobalLootEntry(e);
	}
}

