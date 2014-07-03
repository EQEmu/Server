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
#include "../common/debug.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include "npc.h"
#include "masterentity.h"
#include "zonedb.h"
#include "../common/MiscFunctions.h"
#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

void ZoneDatabase::AddLootTableToNPC(NPC* npc,uint32 loottable_id, ItemList* itemlist, uint32* copper, uint32* silver, uint32* gold, uint32* plat) {
	const LootTable_Struct* lts = 0;
	*copper = 0;
	*silver = 0;
	*gold = 0;
	*plat = 0;

	lts = database.GetLootTable(loottable_id);
	if (!lts)
		return;

	if (lts->mincash > lts->maxcash)
		std::cerr << "Error in loottable #" << loottable_id << ": mincash > maxcash" << std::endl;
	else if (lts->maxcash != 0) {
		uint32 cash = 0;
		if (lts->mincash == lts->maxcash)
			cash = lts->mincash;
		else
			cash = MakeRandomInt(lts->mincash, lts->maxcash);
		if (cash != 0) {
			if (lts->avgcoin != 0) {
				uint32 mincoin = (uint32) (lts->avgcoin * 0.75 + 1);
				uint32 maxcoin = (uint32) (lts->avgcoin * 1.25 + 1);
				*copper = MakeRandomInt(mincoin, maxcoin);
				*silver = MakeRandomInt(mincoin, maxcoin);
				*gold = MakeRandomInt(mincoin, maxcoin);
				if(*copper > cash)
					*copper = cash;
				cash -= *copper;
				if(*silver > (cash / 10))
					*silver = (cash/10);
				cash -= (*silver * 10);
				if(*gold > (cash / 100))
					*gold = (cash / 100);
				cash -= (*gold * 100);
			}
			if (cash < 0)
				cash = 0;
			*plat = (cash / 1000);
			cash -= (*plat * 1000);
			uint32 gold2 = (cash / 100);
			cash -= (gold2 * 100);
			uint32 silver2 = (cash / 10);
			cash -= (silver2 * 10);
			*gold += gold2;
			*silver += silver2;
			*copper += cash;
		}
	}

	for (uint32 i = 0; i < lts->NumEntries; i++) {
		for (uint32 k = 1; k <= lts->Entries[i].multiplier; k++) {
			uint8 droplimit = lts->Entries[i].droplimit;
			uint8 mindrop = lts->Entries[i].mindrop;
			float ltchance = 0.0f;
			ltchance = lts->Entries[i].probability;
			float drop_chance = 0.0f;
			if(ltchance > 0.0 && ltchance < 100.0)
				drop_chance = MakeRandomFloat(0.0, 100.0);

			if (ltchance != 0.0 && (ltchance == 100.0 || drop_chance < ltchance))
				AddLootDropToNPC(npc,lts->Entries[i].lootdrop_id, itemlist, droplimit, mindrop);
		}
	}
}

void ZoneDatabase::AddLootDropToNPC(NPC* npc,uint32 lootdrop_id, ItemList* itemlist, uint8 droplimit, uint8 mindrop) {
	const LootDrop_Struct* lds = GetLootDrop(lootdrop_id);
	if (!lds || lds->NumEntries == 0) {
		return;
	}
	
	if(lds->NumEntries > 99 && droplimit < 1)
		droplimit = (lds->NumEntries / 100);

	uint8 limit = 0;
	uint32 item = MakeRandomInt(0, (lds->NumEntries - 1));
	for (uint32 i = 0; i < lds->NumEntries;) {
		if (item > (lds->NumEntries - 1))
			item = 0;

		uint8 charges = lds->Entries[item].multiplier;
		uint8 pickedcharges = 0;
		for (uint32 x = 1; x <= charges; x++) {
			float thischance = 0.0;
			thischance = lds->Entries[item].chance;

			float drop_chance = 0.0;
			if(thischance != 100.0)
				drop_chance = MakeRandomFloat(0.0, 100.0);

#if EQDEBUG>=11
			LogFile->write(EQEMuLog::Debug, "Drop chance for npc: %s, this chance:%f, drop roll:%f", npc->GetName(), thischance, drop_chance);
#endif
			if (thischance == 100.0 || drop_chance < thischance) {
				uint32 itemid = lds->Entries[item].item_id;
				const Item_Struct* dbitem = GetItem(itemid);
				npc->AddLootDrop(dbitem, itemlist, lds->Entries[item].item_charges, lds->Entries[item].minlevel, lds->Entries[item].maxlevel, lds->Entries[item].equip_item, false);
				pickedcharges++;
			}
		}
		if(pickedcharges > 0)
			limit++;
			
		if(limit >= droplimit && droplimit > 0)
			break;

		item++;
		i++;

		if(i == lds->NumEntries) {
			if(limit < mindrop)
				i = 0;
		}
	}
}

void NPC::AddLootDrop(const Item_Struct *item2, ItemList* itemlist, int16 charges, uint8 minlevel, uint8 maxlevel, bool equipit, bool wearchange) {
	if(item2 == nullptr || (!itemlist && !wearchange))
		return;

	ServerLootItem_Struct* item = new ServerLootItem_Struct;
#if EQDEBUG>=11
		LogFile->write(EQEMuLog::Debug, "Adding drop to npc: %s, Item: %i", GetName(), item2->ID);
#endif

	EQApplicationPacket* outapp = nullptr;
	WearChange_Struct* wc = nullptr;
	if(wearchange) {
		outapp = new EQApplicationPacket(OP_WearChange, sizeof(WearChange_Struct));
		wc = (WearChange_Struct*)outapp->pBuffer;
		wc->spawn_id = GetID();
		wc->material = 0;
	}

	item->item_id = item2->ID;
	item->charges = charges;
	item->aug1 = 0;
	item->aug2 = 0;
	item->aug3 = 0;
	item->aug4 = 0;
	item->aug5 = 0;
	item->minlevel = minlevel;
	item->maxlevel = maxlevel;
	if (equipit) {
		uint8 eslot = 0xFF;
		char newid[20];
		const Item_Struct* compitem = nullptr;
		bool found = false;
		int32 foundslot = -1;
		if (!item2->NoPet) {
			for (int i=0; !found && i<MAX_WORN_INVENTORY; i++) {
				uint32 slots = (1 << i);
				if (item2->Slots & slots) {
					if(equipment[i]) {
						compitem = database.GetItem(equipment[i]);
						if (item2->AC > compitem->AC || (item2->AC == compitem->AC && item2->HP > compitem->HP)) {
							if (item2->Slots != slots)
								foundslot = i;
							else {
								equipment[i] = item2->ID;
								foundslot = i;
								found = true;
							}
						}
					}
					else {
						equipment[i] = item2->ID;
						foundslot = i;
						found = true;
					}
				}
			}
		}

		if (!found && foundslot >= 0) {
			equipment[foundslot] = item2->ID;
			found = true;
		}
		
		uint16 emat;
		if(item2->Material <= 0 || item2->Slots & (1 << SLOT_PRIMARY | 1 << SLOT_SECONDARY)) {
			memset(newid, 0, sizeof(newid));
			for(int i = 0; i < 7; i++){
				if (!isalpha(item2->IDFile[i])) {
					strn0cpy(newid, &item2->IDFile[i],6);
					i=8;
				}
			}
			emat = atoi(newid);
		}
		else
			emat = item2->Material;

		if (foundslot == SLOT_PRIMARY) {
			if (item2->Proc.Effect != 0)
				CastToMob()->AddProcToWeapon(item2->Proc.Effect, true);

			eslot = MaterialPrimary;
		}
		else if (foundslot == SLOT_SECONDARY && (GetOwner() != nullptr || (GetLevel() >= 13 && MakeRandomInt(0,99) < NPC_DW_CHANCE) || (item2->Damage == 0)) && (item2->ItemType == ItemType1HSlash || item2->ItemType == ItemType1HBlunt || item2->ItemType == ItemTypeShield || item2->ItemType == ItemType1HPiercing)) {
			if (item2->Proc.Effect != 0)
				CastToMob()->AddProcToWeapon(item2->Proc.Effect, true);
			eslot = MaterialSecondary;
		}
		else if (foundslot == SLOT_HEAD)
			eslot = MaterialHead;
		else if (foundslot == SLOT_CHEST)
			eslot = MaterialChest;
		else if (foundslot == SLOT_ARMS)
			eslot = MaterialArms;
		else if (foundslot == SLOT_BRACER01 || foundslot == SLOT_BRACER02)
			eslot = MaterialWrist;
		else if (foundslot == SLOT_HANDS)
			eslot = MaterialHands;
		else if (foundslot == SLOT_LEGS)
			eslot = MaterialLegs;
		else if (foundslot == SLOT_FEET)
			eslot = MaterialFeet;
			
		if(eslot != 0xFF) {
			if(wearchange) {
				wc->wear_slot_id = eslot;
				wc->material = emat;
			}

		}
		if (found)
			CalcBonuses();
		item->equipSlot = item2->Slots;
	}

	if(itemlist != nullptr)
		itemlist->push_back(item);
	else
		safe_delete(item);

	if(wearchange && outapp) {
		entity_list.QueueClients(this, outapp);
		safe_delete(outapp);
	}
}

void NPC::AddItem(const Item_Struct* item, uint16 charges, bool equipitem) {
	AddLootDrop(item, &itemlist, charges, 1, 127, equipitem, equipitem);
}

void NPC::AddItem(uint32 itemid, uint16 charges, bool equipitem) {
	const Item_Struct * i = database.GetItem(itemid);
	if(i == nullptr)
		return;
	AddLootDrop(i, &itemlist, charges, 1, 127, equipitem, equipitem);
}

void NPC::AddLootTable() {
	if (npctype_id != 0)
		database.AddLootTableToNPC(this,loottable_id, &itemlist, &copper, &silver, &gold, &platinum);
}

void NPC::AddLootTable(uint32 ldid) {
	if (npctype_id != 0)
	  database.AddLootTableToNPC(this,ldid, &itemlist, &copper, &silver, &gold, &platinum);
}