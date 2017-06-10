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
#include "../common/eqemu_logsys.h"
#include "../common/misc_functions.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"

#include "entity.h"
#include "forage.h"
#include "npc.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "titles.h"
#include "water_map.h"
#include "zonedb.h"

#include <iostream>

#ifdef _WINDOWS
#define snprintf	_snprintf
#endif

struct NPCType;

//max number of items which can be in the foraging table
//for a given zone.
#define FORAGE_ITEM_LIMIT 50

uint32 ZoneDatabase::GetZoneForage(uint32 ZoneID, uint8 skill) {

	uint32 item[FORAGE_ITEM_LIMIT];
	uint32 chance[FORAGE_ITEM_LIMIT];
	uint32 ret;

	for (int c=0; c < FORAGE_ITEM_LIMIT; c++) {
		item[c] = 0;
	}

	uint32 chancepool = 0;
    std::string query = StringFormat("SELECT itemid, chance FROM "
                                    "forage WHERE zoneid = '%i' and level <= '%i' "
                                    "LIMIT %i", ZoneID, skill, FORAGE_ITEM_LIMIT);
    auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

	uint8 index = 0;
    for (auto row = results.begin(); row != results.end(); ++row, ++index) {
        if (index >= FORAGE_ITEM_LIMIT)
            break;

        item[index] = atoi(row[0]);
        chance[index] = atoi(row[1]) + chancepool;
        Log(Logs::General, Logs::Error, "Possible Forage: %d with a %d chance", item[index], chance[index]);
        chancepool = chance[index];
    }


	if(chancepool == 0 || index < 1)
		return 0;

	if(index == 1) {
		return item[0];
	}

	ret = 0;

	uint32 rindex = zone->random.Int(1, chancepool);

	for(int i = 0; i < index; i++) {
		if(rindex <= chance[i]) {
			ret = item[i];
			break;
		}
	}

	return ret;
}

uint32 ZoneDatabase::GetZoneFishing(uint32 ZoneID, uint8 skill, uint32 &npc_id, uint8 &npc_chance)
{
	uint32 item[50];
	uint32 chance[50];
	uint32 npc_ids[50];
	uint32 npc_chances[50];
	uint32 chancepool = 0;
	uint32 ret = 0;

	for (int c=0; c<50; c++) {
		item[c]=0;
		chance[c]=0;
	}

    std::string query = StringFormat("SELECT itemid, chance, npc_id, npc_chance "
                                    "FROM fishing WHERE (zoneid = '%i' || zoneid = 0) AND skill_level <= '%i'",
                                    ZoneID, skill);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return 0;
    }

    uint8 index = 0;
    for (auto row = results.begin(); row != results.end(); ++row, ++index) {
        if (index >= 50)
            break;

        item[index] = atoi(row[0]);
        chance[index] = atoi(row[1])+chancepool;
        chancepool = chance[index];

        npc_ids[index] = atoi(row[2]);
        npc_chances[index] = atoi(row[3]);
    }

	npc_id = 0;
	npc_chance = 0;
	if (index <= 0)
        return 0;

    uint32 random = zone->random.Int(1, chancepool);
    for (int i = 0; i < index; i++)
    {
        if (random > chance[i])
            continue;

        ret = item[i];
        npc_id = npc_ids[i];
        npc_chance = npc_chances[i];
        break;
    }

	return ret;
}

//we need this function to immediately determine, after we receive OP_Fishing, if we can even try to fish, otherwise we have to wait a while to get the failure
bool Client::CanFish() {
	//make sure we still have a fishing pole on:
	const EQEmu::ItemInstance* Pole = m_inv[EQEmu::inventory::slotPrimary];
	int32 bslot = m_inv.HasItemByUse(EQEmu::item::ItemTypeFishingBait, 1, invWhereWorn | invWherePersonal);
	const EQEmu::ItemInstance* Bait = nullptr;
	if (bslot != INVALID_INDEX)
		Bait = m_inv.GetItem(bslot);

	if (!Pole || !Pole->IsClassCommon() || Pole->GetItem()->ItemType != EQEmu::item::ItemTypeFishingPole) {
		if (m_inv.HasItemByUse(EQEmu::item::ItemTypeFishingPole, 1, invWhereWorn | invWherePersonal | invWhereBank | invWhereSharedBank | invWhereTrading | invWhereCursor))	//We have a fishing pole somewhere, just not equipped
			Message_StringID(MT_Skills, FISHING_EQUIP_POLE);	//You need to put your fishing pole in your primary hand.
		else	//We don't have a fishing pole anywhere
			Message_StringID(MT_Skills, FISHING_NO_POLE);	//You can't fish without a fishing pole, go buy one.
		return false;
	}

	if (!Bait || !Bait->IsClassCommon() || Bait->GetItem()->ItemType != EQEmu::item::ItemTypeFishingBait) {
		Message_StringID(MT_Skills, FISHING_NO_BAIT);	//You can't fish without fishing bait, go buy some.
		return false;
	}

	if(zone->zonemap != nullptr && zone->watermap != nullptr && RuleB(Watermap, CheckForWaterWhenFishing)) {

		glm::vec3 rodPosition;
		// Tweak Rod and LineLength if required
		const float RodLength = RuleR(Watermap, FishingRodLength);
		const float LineLength = RuleR(Watermap, FishingLineLength);
		int HeadingDegrees;

		HeadingDegrees = (int) ((GetHeading()*360)/256);
		HeadingDegrees = HeadingDegrees % 360;

		rodPosition.x = m_Position.x + RodLength * sin(HeadingDegrees * M_PI/180.0f);
		rodPosition.y = m_Position.y + RodLength * cos(HeadingDegrees * M_PI/180.0f);
		rodPosition.z = m_Position.z;

		float bestz = zone->zonemap->FindBestZ(rodPosition, nullptr);
		float len = m_Position.z - bestz;
		if(len > LineLength || len < 0.0f) {
			Message_StringID(MT_Skills, FISHING_LAND);
			return false;
		}

		float step_size = RuleR(Watermap, FishingLineStepSize);

		for(float i = 0.0f; i < LineLength; i += step_size) {
			glm::vec3 dest(rodPosition.x, rodPosition.y, m_Position.z - i);

			bool in_lava = zone->watermap->InLava(dest);
			bool in_water = zone->watermap->InWater(dest) || zone->watermap->InVWater(dest);

			if (in_lava) {
				Message_StringID(MT_Skills, FISHING_LAVA);	//Trying to catch a fire elemental or something?
				return false;
			}

			if(in_water) {
				return true;
			}
		}

		Message_StringID(MT_Skills, FISHING_LAND);
		return false;
	}
	return true;
}

void Client::GoFish()
{

	//TODO: generate a message if we're already fishing
	/*if (!fishing_timer.Check()) {	//this isn't the right check, may need to add something to the Client class like 'bool is_fishing'
		Message_StringID(0, ALREADY_FISHING);	//You are already fishing!
		return;
	}*/

	fishing_timer.Disable();

	//we're doing this a second time (1st in Client::Handle_OP_Fishing) to make sure that, between when we started fishing & now, we're still able to fish (in case we move, change equip, etc)
	if (!CanFish())	//if we can't fish here, we don't need to bother with the rest
		return;

	//multiple entries yeilds higher probability of dropping...
	uint32 common_fish_ids[MAX_COMMON_FISH_IDS] = {
		1038, // Tattered Cloth Sandals
		1038, // Tattered Cloth Sandals
		1038, // Tattered Cloth Sandals
		13019, // Fresh Fish
		13076, // Fish Scales
		13076, // Fish Scales
		7007, // Rusty Dagger
		7007, // Rusty Dagger
		7007 // Rusty Dagger

	};

	//success formula is not researched at all

	int fishing_skill = GetSkill(EQEmu::skills::SkillFishing);	//will take into account skill bonuses on pole & bait

	//make sure we still have a fishing pole on:
	int32 bslot = m_inv.HasItemByUse(EQEmu::item::ItemTypeFishingBait, 1, invWhereWorn | invWherePersonal);
	const EQEmu::ItemInstance* Bait = nullptr;
	if (bslot != INVALID_INDEX)
		Bait = m_inv.GetItem(bslot);

	//if the bait isnt equipped, need to add its skill bonus
	if (bslot >= EQEmu::legacy::GENERAL_BEGIN && Bait != nullptr && Bait->GetItem()->SkillModType == EQEmu::skills::SkillFishing) {
		fishing_skill += Bait->GetItem()->SkillModValue;
	}

	if (fishing_skill > 100)
	{
		fishing_skill = 100+((fishing_skill-100)/2);
	}

	if (zone->random.Int(0,175) < fishing_skill) {
		uint32 food_id = 0;

		//25% chance to fish an item.
		if (zone->random.Int(0, 399) <= fishing_skill ) {
			uint32 npc_id = 0;
			uint8 npc_chance = 0;
			food_id = database.GetZoneFishing(m_pp.zone_id, fishing_skill, npc_id, npc_chance);

			//check for add NPC
			if(npc_chance > 0 && npc_id) {
				if(npc_chance < zone->random.Int(0, 99)) {
					const NPCType* tmp = database.LoadNPCTypesData(npc_id);
					if(tmp != nullptr) {
                        auto positionNPC = GetPosition();
                        positionNPC.x = positionNPC.x + 3;
			auto npc = new NPC(tmp, nullptr, positionNPC, FlyMode3);
			npc->AddLootTable();

			npc->AddToHateList(this, 1, 0, false); // no help yelling

			entity_list.AddNPC(npc);

			Message(MT_Emote, "You fish up a little more than you bargained for...");
					}
				}
			}
		}

		//consume bait, should we always consume bait on success?
		DeleteItemInInventory(bslot, 1, true);	//do we need client update?

		if(food_id == 0) {
			int index = zone->random.Int(0, MAX_COMMON_FISH_IDS-1);
			food_id = common_fish_ids[index];
		}

		const EQEmu::ItemData* food_item = database.GetItem(food_id);

		if (food_item->ItemType  != EQEmu::item::ItemTypeFood) {
			Message_StringID(MT_Skills, FISHING_SUCCESS);
		}
		else {
			Message_StringID(MT_Skills, FISHING_SUCCESS_FISH_NAME, food_item->Name);
		}

		EQEmu::ItemInstance* inst = database.CreateItem(food_item, 1);
		if(inst != nullptr) {
			if(CheckLoreConflict(inst->GetItem()))
			{
				Message_StringID(0, DUP_LORE);
				safe_delete(inst);
			}
			else
			{
				PushItemOnCursor(*inst);
				SendItemPacket(EQEmu::inventory::slotCursor, inst, ItemPacketLimbo);
				if(RuleB(TaskSystem, EnableTaskSystem))
					UpdateTasksForItem(ActivityFish, food_id);

				safe_delete(inst);
				inst = m_inv.GetItem(EQEmu::inventory::slotCursor);
			}

			if(inst) {
				std::vector<EQEmu::Any> args;
				args.push_back(inst);
				parse->EventPlayer(EVENT_FISH_SUCCESS, this, "", inst->GetID(), &args);
			}
		}
	}
	else
	{
		//chance to use bait when you dont catch anything...
		if (zone->random.Int(0, 4) == 1) {
			DeleteItemInInventory(bslot, 1, true);	//do we need client update?
			Message_StringID(MT_Skills, FISHING_LOST_BAIT);	//You lost your bait!
		} else {
			if (zone->random.Int(0, 15) == 1)	//give about a 1 in 15 chance to spill your beer. we could make this a rule, but it doesn't really seem worth it
				//TODO: check for & consume an alcoholic beverage from inventory when this triggers, and set it as a rule that's disabled by default
				Message_StringID(MT_Skills, FISHING_SPILL_BEER);	//You spill your beer while bringing in your line.
			else
				Message_StringID(MT_Skills, FISHING_FAILED);	//You didn't catch anything.
		}

		parse->EventPlayer(EVENT_FISH_FAILURE, this, "", 0);
	}

	//chance to break fishing pole...
	//this is potentially exploitable in that they can fish
	//and then swap out items in primary slot... too lazy to fix right now
	if (zone->random.Int(0, 49) == 1) {
		Message_StringID(MT_Skills, FISHING_POLE_BROKE);	//Your fishing pole broke!
		DeleteItemInInventory(EQEmu::inventory::slotPrimary, 0, true);
	}

	if (CheckIncreaseSkill(EQEmu::skills::SkillFishing, nullptr, 5))
	{
		if (title_manager.IsNewTradeSkillTitleAvailable(EQEmu::skills::SkillFishing, GetRawSkill(EQEmu::skills::SkillFishing)))
			NotifyNewTitlesAvailable();
	}
}

void Client::ForageItem(bool guarantee) {

	int skill_level = GetSkill(EQEmu::skills::SkillForage);

	//be wary of the string ids in switch below when changing this.
	uint32 common_food_ids[MAX_COMMON_FOOD_IDS] = {
		13046, // Fruit
		13045, // Berries
		13419, // Vegetables
		13048, // Rabbit Meat
		13047, // Roots
		13044, // Pod Of Water
		14905, // mushroom
		13106 // Fishing Grubs
	};

	// these may need to be fine tuned, I am just guessing here
	if (guarantee || zone->random.Int(0,199) < skill_level) {
		uint32 foragedfood = 0;
		uint32 stringid = FORAGE_NOEAT;

		if (zone->random.Roll(25)) {
			foragedfood = database.GetZoneForage(m_pp.zone_id, skill_level);
		}

		//not an else in case theres no DB food
		if(foragedfood == 0) {
			uint8 index = 0;
			index = zone->random.Int(0, MAX_COMMON_FOOD_IDS-1);
			foragedfood = common_food_ids[index];
		}

		const EQEmu::ItemData* food_item = database.GetItem(foragedfood);

		if(!food_item) {
			Log(Logs::General, Logs::Error, "nullptr returned from database.GetItem in ClientForageItem");
			return;
		}

		if(foragedfood == 13106)
			stringid = FORAGE_GRUBS;
		else
			switch(food_item->ItemType) {
			case EQEmu::item::ItemTypeFood:
				stringid = FORAGE_FOOD;
				break;
			case EQEmu::item::ItemTypeDrink:
				if(strstr(food_item->Name, "ater"))
					stringid = FORAGE_WATER;
				else
					stringid = FORAGE_DRINK;
				break;
			default:
				break;
			}

		Message_StringID(MT_Skills, stringid);
		EQEmu::ItemInstance* inst = database.CreateItem(food_item, 1);
		if(inst != nullptr) {
			// check to make sure it isn't a foraged lore item
			if(CheckLoreConflict(inst->GetItem()))
			{
				Message_StringID(0, DUP_LORE);
				safe_delete(inst);
			}
			else {
				PushItemOnCursor(*inst);
				SendItemPacket(EQEmu::inventory::slotCursor, inst, ItemPacketLimbo);
				if(RuleB(TaskSystem, EnableTaskSystem))
					UpdateTasksForItem(ActivityForage, foragedfood);

				safe_delete(inst);
				inst = m_inv.GetItem(EQEmu::inventory::slotCursor);
			}

			if(inst) {
				std::vector<EQEmu::Any> args;
				args.push_back(inst);
				parse->EventPlayer(EVENT_FORAGE_SUCCESS, this, "", inst->GetID(), &args);
			}
		}

		int ChanceSecondForage = aabonuses.ForageAdditionalItems + itembonuses.ForageAdditionalItems + spellbonuses.ForageAdditionalItems;
		if(!guarantee && zone->random.Roll(ChanceSecondForage)) {
			Message_StringID(MT_Skills, FORAGE_MASTERY);
			ForageItem(true);
		}

	} else {
		Message_StringID(MT_Skills, FORAGE_FAILED);
		parse->EventPlayer(EVENT_FORAGE_FAILURE, this, "", 0);
	}

	CheckIncreaseSkill(EQEmu::skills::SkillForage, nullptr, 5);

}

