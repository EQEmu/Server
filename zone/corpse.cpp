/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
/*
New class for handeling corpses and everything associated with them.
Child of the Mob class.
-Quagmire
*/
#include "../common/debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <sstream>
#ifdef _WINDOWS
    #define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
    #define strncasecmp	_strnicmp
    #define strcasecmp	_stricmp
#endif

#include "masterentity.h"
#include "../common/packet_functions.h"
#include "../common/string_util.h"
#include "../common/crc32.h"
#include "string_ids.h"
#include "worldserver.h"
#include "../common/rulesys.h"
#include "quest_parser_collection.h"

extern EntityList entity_list;
extern Zone* zone;
extern WorldServer worldserver;
extern npcDecayTimes_Struct npcCorpseDecayTimes[100];

void Corpse::SendEndLootErrorPacket(Client* client) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_LootComplete, 0);
	client->QueuePacket(outapp);
	safe_delete(outapp);
}

void Corpse::SendLootReqErrorPacket(Client* client, uint8 response) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoneyOnCorpse, sizeof(moneyOnCorpseStruct));
	moneyOnCorpseStruct* d = (moneyOnCorpseStruct*) outapp->pBuffer;
	d->response		= response;
	d->unknown1		= 0x5a;
	d->unknown2		= 0x40;
	client->QueuePacket(outapp);
	safe_delete(outapp);
}

Corpse* Corpse::LoadFromDBData(uint32 in_dbid, uint32 in_charid, std::string in_charname, const xyz_heading& position, std::string time_of_death, bool rezzed, bool was_at_graveyard)
{
	uint32 item_count = database.GetCharacterCorpseItemCount(in_dbid);
	char *buffer = new char[sizeof(PlayerCorpse_Struct) + (item_count * sizeof(player_lootitem::ServerLootItem_Struct))];
	PlayerCorpse_Struct *pcs = (PlayerCorpse_Struct*)buffer;
	database.LoadCharacterCorpseData(in_dbid, pcs);

	/* Load Items */
	ItemList itemlist;
	ServerLootItem_Struct* tmp = 0;
	for (unsigned int i = 0; i < pcs->itemcount; i++) {
		tmp = new ServerLootItem_Struct;
		memcpy(tmp, &pcs->items[i], sizeof(player_lootitem::ServerLootItem_Struct));
		tmp->equip_slot = CorpseToServerSlot(tmp->equip_slot);
		itemlist.push_back(tmp);
	}

	/* Create Corpse Entity */
	Corpse* pc = new Corpse(
		in_dbid,			   // uint32 in_dbid
		in_charid,			   // uint32 in_charid
		in_charname.c_str(),   // char* in_charname
		&itemlist,			   // ItemList* in_itemlist
		pcs->copper,		   // uint32 in_copper
		pcs->silver,		   // uint32 in_silver
		pcs->gold,			   // uint32 in_gold
		pcs->plat,			   // uint32 in_plat
		position.m_X,		   // float in_x
		position.m_Y,		   // float in_y
		position.m_Z,		   // float in_z
		position.m_Heading,	   // float in_heading
		pcs->size,			   // float in_size
		pcs->gender,		   // uint8 in_gender
		pcs->race,			   // uint16 in_race
		pcs->class_,		   // uint8 in_class
		pcs->deity,			   // uint8 in_deity
		pcs->level,			   // uint8 in_level
		pcs->texture,		   // uint8 in_texture
		pcs->helmtexture,	   // uint8 in_helmtexture
		pcs->exp,			   // uint32 in_rezexp
		was_at_graveyard	   // bool wasAtGraveyard
	);
	if (pcs->locked)
		pc->Lock();

	/* Load Item Tints */
	// memcpy(pc->item_tint, pcs->item_tint, sizeof(pc->item_tint));
	pc->item_tint[0].color = pcs->item_tint[0].color;
	pc->item_tint[1].color = pcs->item_tint[1].color;
	pc->item_tint[2].color = pcs->item_tint[2].color;
	pc->item_tint[3].color = pcs->item_tint[3].color;
	pc->item_tint[4].color = pcs->item_tint[4].color;
	pc->item_tint[5].color = pcs->item_tint[5].color;
	pc->item_tint[6].color = pcs->item_tint[6].color;
	pc->item_tint[7].color = pcs->item_tint[7].color;
	pc->item_tint[8].color = pcs->item_tint[8].color;


	/* Load Physical Appearance */
	pc->haircolor = pcs->haircolor;
	pc->beardcolor = pcs->beardcolor;
	pc->eyecolor1 = pcs->eyecolor1;
	pc->eyecolor2 = pcs->eyecolor2;
	pc->hairstyle = pcs->hairstyle;
	pc->luclinface = pcs->face;
	pc->beard = pcs->beard;
	pc->drakkin_heritage = pcs->drakkin_heritage;
	pc->drakkin_tattoo = pcs->drakkin_tattoo;
	pc->drakkin_details = pcs->drakkin_details;
	pc->IsRezzed(rezzed);
	pc->become_npc = false;

	safe_delete_array(pcs);

	return pc;
}

// To be used on NPC death and ZoneStateLoad
// Mongrel: added see_invis and see_invis_undead
Corpse::Corpse(NPC* in_npc, ItemList* in_itemlist, uint32 in_npctypeid, const NPCType** in_npctypedata, uint32 in_decaytime)
// vesuvias - appearence fix
: Mob("Unnamed_Corpse","",0,0,in_npc->GetGender(),in_npc->GetRace(),in_npc->GetClass(),BT_Humanoid,//bodytype added
	in_npc->GetDeity(),in_npc->GetLevel(),in_npc->GetNPCTypeID(),in_npc->GetSize(),0,
	in_npc->GetPosition(), 0, in_npc->GetTexture(),in_npc->GetHelmTexture(),
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0xff,0,0,0,0,0,0,0,0,0),
	corpse_decay_timer(in_decaytime),
	corpse_res_timer(0),
	corpse_delay_timer(RuleI(NPC, CorpseUnlockTimer)),
	corpse_graveyard_timer(0),
	loot_cooldown_timer(10)
{
	corpse_graveyard_timer.Disable();
	memset(item_tint, 0, sizeof(item_tint));
	is_corpse_changed = false;
	is_player_corpse = false;
	is_locked = false;
	being_looted_by = 0xFFFFFFFF;
	if (in_itemlist) {
		itemlist = *in_itemlist;
		in_itemlist->clear();
	}

	SetCash(in_npc->GetCopper(), in_npc->GetSilver(), in_npc->GetGold(), in_npc->GetPlatinum());

	npctype_id = in_npctypeid;
	SetPKItem(0);
	char_id = 0;
	corpse_db_id = 0;
	player_corpse_depop = false;
	strcpy(orgname, in_npc->GetName());
	strcpy(name, in_npc->GetName());
	// Added By Hogie
	for(int count = 0; count < 100; count++) {
		if ((level >= npcCorpseDecayTimes[count].minlvl) && (level <= npcCorpseDecayTimes[count].maxlvl)) {
			corpse_decay_timer.SetTimer(npcCorpseDecayTimes[count].seconds*1000);
			break;
		}
	}
	if(IsEmpty()) {
		corpse_decay_timer.SetTimer(RuleI(NPC,EmptyNPCCorpseDecayTimeMS)+1000);
	}

	if(in_npc->HasPrivateCorpse()) {
		corpse_delay_timer.SetTimer(corpse_decay_timer.GetRemainingTime() + 1000);
	}

	// Added By Hogie -- End
	for (int i = 0; i < MAX_LOOTERS; i++){
		allowed_looters[i] = 0;
	}
	this->rezzexp = 0;
}

Corpse::Corpse(Client* client, int32 in_rezexp) : Mob (
	"Unnamed_Corpse",				  // const char*	in_name,
	"",								  // const char*	in_lastname,
	0,								  // int32		in_cur_hp,
	0,								  // int32		in_max_hp,
	client->GetGender(),			  // uint8		in_gender,
	client->GetRace(),				  // uint16		in_race,
	client->GetClass(),				  // uint8		in_class,
	BT_Humanoid,					  // bodyType	in_bodytype,
	client->GetDeity(),				  // uint8		in_deity,
	client->GetLevel(),				  // uint8		in_level,
	0,								  // uint32		in_npctype_id,
	client->GetSize(),				  // float		in_size,
	0,								  // float		in_runspeed,
	client->GetPosition(),
	0,								  // uint8		in_light,
	client->GetTexture(),			  // uint8		in_texture,
	client->GetHelmTexture(),		  // uint8		in_helmtexture,
	0,								  // uint16		in_ac,
	0,								  // uint16		in_atk,
	0,								  // uint16		in_str,
	0,								  // uint16		in_sta,
	0,								  // uint16		in_dex,
	0,								  // uint16		in_agi,
	0,								  // uint16		in_int,
	0,								  // uint16		in_wis,
	0,								  // uint16		in_cha,
	client->GetPP().haircolor,		  // uint8		in_haircolor,
	client->GetPP().beardcolor,		  // uint8		in_beardcolor,
	client->GetPP().eyecolor1,		  // uint8		in_eyecolor1, // the eyecolors always seem to be the same, maybe left and right eye?
	client->GetPP().eyecolor2,		  // uint8		in_eyecolor2,
	client->GetPP().hairstyle,		  // uint8		in_hairstyle,
	client->GetPP().face,			  // uint8		in_luclinface,
	client->GetPP().beard,			  // uint8		in_beard,
	client->GetPP().drakkin_heritage, // uint32		in_drakkin_heritage,
	client->GetPP().drakkin_tattoo,	  // uint32		in_drakkin_tattoo,
	client->GetPP().drakkin_details,  // uint32		in_drakkin_details,
	0,								  // uint32		in_armor_tint[_MaterialCount],
	0xff,							  // uint8		in_aa_title,
	0,								  // uint8		in_see_invis, // see through invis
	0,								  // uint8		in_see_invis_undead, // see through invis vs. undead
	0,								  // uint8		in_see_hide,
	0,								  // uint8		in_see_improved_hide,
	0,								  // int32		in_hp_regen,
	0,								  // int32		in_mana_regen,
	0,								  // uint8		in_qglobal,
	0,								  // uint8		in_maxlevel,
	0								  // uint32		in_scalerate
	),
	corpse_decay_timer(RuleI(Character, CorpseDecayTimeMS)),
	corpse_res_timer(RuleI(Character, CorpseResTimeMS)),
	corpse_delay_timer(RuleI(NPC, CorpseUnlockTimer)),
	corpse_graveyard_timer(RuleI(Zone, GraveyardTimeMS)),
	loot_cooldown_timer(10)
{
	int i;

	PlayerProfile_Struct *pp = &client->GetPP();
	ItemInst *item;

	/* Check if Zone has Graveyard First */
	if(!zone->HasGraveyard()) {
		corpse_graveyard_timer.Disable();
	}

	memset(item_tint, 0, sizeof(item_tint));

	for (i = 0; i < MAX_LOOTERS; i++){
		allowed_looters[i] = 0;
	}

	is_corpse_changed		= true;
	rezzexp			= in_rezexp;
	can_rez			= true;
	is_player_corpse	= true;
	is_locked			= false;
	being_looted_by	= 0xFFFFFFFF;
	char_id			= client->CharacterID();
	corpse_db_id	= 0;
	player_corpse_depop			= false;
	copper			= 0;
	silver			= 0;
	gold			= 0;
	platinum		= 0;

	strcpy(orgname, pp->name);
	strcpy(name, pp->name);

	/* become_npc was not being initialized which led to some pretty funky things with newly created corpses */
	become_npc = false;

	SetPKItem(0);

	/* Check Rule to see if we can leave corpses */
	if(!RuleB(Character, LeaveNakedCorpses) ||
		RuleB(Character, LeaveCorpses) &&
		GetLevel() >= RuleI(Character, DeathItemLossLevel)) {
		// cash
		// Let's not move the cash when 'RespawnFromHover = true' && 'client->GetClientVersion() < EQClientSoF' since the client doesn't.
		// (change to first client that supports 'death hover' mode, if not SoF.)
		if (!RuleB(Character, RespawnFromHover) || client->GetClientVersion() < EQClientSoF) {
			SetCash(pp->copper, pp->silver, pp->gold, pp->platinum);
			pp->copper = 0;
			pp->silver = 0;
			pp->gold = 0;
			pp->platinum = 0;
		}

		// get their tints
		memcpy(item_tint, &client->GetPP().item_tint, sizeof(item_tint));

		// solar: TODO soulbound items need not be added to corpse, but they need
		// to go into the regular slots on the player, out of bags

		// worn + inventory + cursor
		std::list<uint32> removed_list;
		bool cursor = false;
		for(i = MAIN_BEGIN; i < EmuConstants::MAP_POSSESSIONS_SIZE; i++) {
			if(i == MainAmmo && client->GetClientVersion() >= EQClientSoF) {
				item = client->GetInv().GetItem(MainPowerSource);
				if((item && (!client->IsBecomeNPC())) || (item && client->IsBecomeNPC() && !item->GetItem()->NoRent)) {
					std::list<uint32> slot_list = MoveItemToCorpse(client, item, MainPowerSource);
					removed_list.merge(slot_list);
				}

			}

			item = client->GetInv().GetItem(i);
			if((item && (!client->IsBecomeNPC())) || (item && client->IsBecomeNPC() && !item->GetItem()->NoRent)) {
				std::list<uint32> slot_list = MoveItemToCorpse(client, item, i);
				removed_list.merge(slot_list);
			}
		}

		// cursor queue // (change to first client that supports 'death hover' mode, if not SoF.)
		if (!RuleB(Character, RespawnFromHover) || client->GetClientVersion() < EQClientSoF) {

			// bumped starting assignment to 8001 because any in-memory 'slot 8000' item was moved above as 'slot 30'
			// this was mainly for client profile state reflection..should match db player inventory entries now.

			iter_queue it;
			for(it=client->GetInv().cursor_begin(),i=8001; it!=client->GetInv().cursor_end(); ++it,i++) {
				item = *it;
				if((item && (!client->IsBecomeNPC())) || (item && client->IsBecomeNPC() && !item->GetItem()->NoRent)) {
					std::list<uint32> slot_list = MoveItemToCorpse(client, item, i);
					removed_list.merge(slot_list);
					cursor = true;
				}
			}
		}

		database.TransactionBegin();
		if(removed_list.size() != 0) {
			std::stringstream ss("");
			ss << "DELETE FROM inventory WHERE charid=" << client->CharacterID();
			ss << " AND (";
			std::list<uint32>::const_iterator iter = removed_list.begin();
			bool first = true;
			while(iter != removed_list.end()) {
				if(first) {
					first = false;
				} else {
					ss << " OR ";
				}
				ss << "slotid=" << (*iter);
				++iter;
			}
			ss << ")";
			database.QueryDatabase(ss.str().c_str());
		}

		if(cursor) { // all cursor items should be on corpse (client < SoF or RespawnFromHover = false)
			while(!client->GetInv().CursorEmpty())
				client->DeleteItemInInventory(MainCursor, 0, false, false);
		}
		else { // only visible cursor made it to corpse (client >= Sof and RespawnFromHover = true)
			std::list<ItemInst*>::const_iterator start = client->GetInv().cursor_begin();
			std::list<ItemInst*>::const_iterator finish = client->GetInv().cursor_end();
			database.SaveCursor(client->CharacterID(), start, finish);
		}

		client->CalcBonuses(); // will only affect offline profile viewing of dead characters..unneeded overhead
		client->Save();

		IsRezzed(false);
		Save();
		database.TransactionCommit();

		return;
	} //end "not leaving naked corpses"

	IsRezzed(false);
	Save();
}

// solar: helper function for client corpse constructor
std::list<uint32> Corpse::MoveItemToCorpse(Client *client, ItemInst *item, int16 equipslot)
{
	int bagindex;
	int16 interior_slot;
	ItemInst *interior_item;
	std::list<uint32> returnlist;

	AddItem(item->GetItem()->ID, item->GetCharges(), equipslot, item->GetAugmentItemID(0), item->GetAugmentItemID(1), item->GetAugmentItemID(2), item->GetAugmentItemID(3), item->GetAugmentItemID(4));
	returnlist.push_back(equipslot);

	// Qualified bag slot iterations. processing bag slots that don't exist is probably not a good idea.
	if(item->IsType(ItemClassContainer) && ((equipslot >= EmuConstants::GENERAL_BEGIN && equipslot <= MainCursor))) {
		for(bagindex = SUB_BEGIN; bagindex <= EmuConstants::ITEM_CONTAINER_SIZE; bagindex++) {
			// For empty bags in cursor queue, slot was previously being resolved as SLOT_INVALID (-1)
			interior_slot = Inventory::CalcSlotId(equipslot, bagindex);
			interior_item = client->GetInv().GetItem(interior_slot);

			if(interior_item) {
				AddItem(interior_item->GetItem()->ID, interior_item->GetCharges(), interior_slot, interior_item->GetAugmentItemID(0), interior_item->GetAugmentItemID(1), interior_item->GetAugmentItemID(2), interior_item->GetAugmentItemID(3), interior_item->GetAugmentItemID(4));
				returnlist.push_back(Inventory::CalcSlotId(equipslot, bagindex));
				client->DeleteItemInInventory(interior_slot, 0, true, false);
			}
		}
	}
	client->DeleteItemInInventory(equipslot, 0, true, false);
	return returnlist;
}

// To be called from LoadFromDBData
// Mongrel: added see_invis and see_invis_undead
Corpse::Corpse(uint32 in_dbid, uint32 in_charid, const char* in_charname, ItemList* in_itemlist, uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_plat, float in_x, float in_y, float in_z, float in_heading, float in_size, uint8 in_gender, uint16 in_race, uint8 in_class, uint8 in_deity, uint8 in_level, uint8 in_texture, uint8 in_helmtexture,uint32 in_rezexp, bool wasAtGraveyard)
: Mob("Unnamed_Corpse",
"",
0,
0,
in_gender,
in_race,
in_class,
BT_Humanoid,
in_deity,
in_level,
0,
in_size,
0,
xyz_heading(in_x, in_y,in_z,in_heading),
0,
in_texture,
in_helmtexture,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0xff,
0,
0,
0,
0,
0,
0,
0,
0,
0),
	corpse_decay_timer(RuleI(Character, CorpseDecayTimeMS)),
	corpse_res_timer(RuleI(Character, CorpseResTimeMS)),
	corpse_delay_timer(RuleI(NPC, CorpseUnlockTimer)),
	corpse_graveyard_timer(RuleI(Zone, GraveyardTimeMS)),
	loot_cooldown_timer(10)
{

	LoadPlayerCorpseDecayTime(in_dbid);

	if(!zone->HasGraveyard() || wasAtGraveyard)
		corpse_graveyard_timer.Disable();

	memset(item_tint, 0, sizeof(item_tint));

	is_corpse_changed = false;
	is_player_corpse = true;
	is_locked = false;
	being_looted_by = 0xFFFFFFFF;
	corpse_db_id = in_dbid;
	player_corpse_depop = false;
	char_id = in_charid;
	itemlist = *in_itemlist;
	in_itemlist->clear();

	strcpy(orgname, in_charname);
	strcpy(name, in_charname);
	this->copper = in_copper;
	this->silver = in_silver;
	this->gold = in_gold;
	this->platinum = in_plat;
	rezzexp = in_rezexp;

	for (int i = 0; i < MAX_LOOTERS; i++){
		allowed_looters[i] = 0;
	}
	SetPKItem(0);
}

Corpse::~Corpse() {
	if (is_player_corpse && !(player_corpse_depop && corpse_db_id == 0)) {
			Save();
	}
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		ServerLootItem_Struct* item = *cur;
		safe_delete(item);
	}
	itemlist.clear();
}

/*
this needs to be called AFTER the entity_id is set
the client does this too, so it's unchangable
*/
void Corpse::CalcCorpseName() {
	EntityList::RemoveNumbers(name);
	char tmp[64];
	if (is_player_corpse){
		snprintf(tmp, sizeof(tmp), "'s corpse%d", GetID());
	}
	else{
		snprintf(tmp, sizeof(tmp), "`s_corpse%d", GetID());
	}
	name[(sizeof(name) - 1) - strlen(tmp)] = 0;
	strcat(name, tmp);
}

bool Corpse::Save() {
	if (!is_player_corpse)
		return true;
	if (!is_corpse_changed)
		return true;

	uint32 tmp = this->CountItems();
	uint32 tmpsize = sizeof(PlayerCorpse_Struct) + (tmp * sizeof(player_lootitem::ServerLootItem_Struct));

	PlayerCorpse_Struct* dbpc = (PlayerCorpse_Struct*) new uchar[tmpsize];
	memset(dbpc, 0, tmpsize);
	dbpc->itemcount = tmp;
	dbpc->size = this->size;
	dbpc->locked = is_locked;
	dbpc->copper = this->copper;
	dbpc->silver = this->silver;
	dbpc->gold = this->gold;
	dbpc->plat = this->platinum;
	dbpc->race = this->race;
	dbpc->class_ = class_;
	dbpc->gender = gender;
	dbpc->deity = deity;
	dbpc->level = level;
	dbpc->texture = this->texture;
	dbpc->helmtexture = this->helmtexture;
	dbpc->exp = rezzexp;

	memcpy(dbpc->item_tint, item_tint, sizeof(dbpc->item_tint));
	dbpc->haircolor = haircolor;
	dbpc->beardcolor = beardcolor;
	dbpc->eyecolor2 = eyecolor1;
	dbpc->hairstyle = hairstyle;
	dbpc->face = luclinface;
	dbpc->beard = beard;
	dbpc->drakkin_heritage = drakkin_heritage;
	dbpc->drakkin_tattoo = drakkin_tattoo;
	dbpc->drakkin_details = drakkin_details;

	uint32 x = 0;
	ItemList::iterator cur, end;
	cur = itemlist.begin();
	end = itemlist.end();
	for (; cur != end; ++cur) {
		ServerLootItem_Struct* item = *cur;
		item->equip_slot = ServerToCorpseSlot(item->equip_slot); // temp hack until corpse blobs are removed
		memcpy((char*)&dbpc->items[x++], (char*)item, sizeof(ServerLootItem_Struct));
	}

	/* Create New Corpse*/
	if (corpse_db_id == 0) {
		corpse_db_id = database.SaveCharacterCorpse(char_id, orgname, zone->GetZoneID(), zone->GetInstanceID(), dbpc, m_Position.m_X, m_Position.m_Y, m_Position.m_Z, m_Position.m_Heading);
	}
	/* Update Corpse Data */
	else{
		corpse_db_id = database.UpdateCharacterCorpse(corpse_db_id, char_id, orgname, zone->GetZoneID(), zone->GetInstanceID(), dbpc, m_Position.m_X, m_Position.m_Y, m_Position.m_Z, m_Position.m_Heading, IsRezzed());
	}

	safe_delete_array(dbpc);

	return true;
}

void Corpse::Delete() {
	if (IsPlayerCorpse() && corpse_db_id != 0)
		database.DeleteCharacterCorpse(corpse_db_id);

	corpse_db_id = 0;
	player_corpse_depop = true;
}

void Corpse::Bury() {
	if (IsPlayerCorpse() && corpse_db_id != 0)
		database.BuryCharacterCorpse(corpse_db_id);
	corpse_db_id = 0;

	player_corpse_depop = true;
}

void Corpse::Depop() {
	if (IsNPCCorpse())
		player_corpse_depop = true;
}

void Corpse::DepopCorpse() {
		player_corpse_depop = true;
}

uint32 Corpse::CountItems() {
	return itemlist.size();
}

void Corpse::AddItem(uint32 itemnum, uint16 charges, int16 slot, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	if (!database.GetItem(itemnum))
		return;

	is_corpse_changed = true;
	ServerLootItem_Struct* item = new ServerLootItem_Struct;
	memset(item, 0, sizeof(ServerLootItem_Struct));
	item->item_id = itemnum;
	item->charges = charges;
	item->equip_slot = slot;
	item->aug_1=aug1;
	item->aug_2=aug2;
	item->aug_3=aug3;
	item->aug_4=aug4;
	item->aug_5=aug5;
	itemlist.push_back(item);
}

ServerLootItem_Struct* Corpse::GetItem(uint16 lootslot, ServerLootItem_Struct** bag_item_data) {
	ServerLootItem_Struct *sitem = 0, *sitem2;

	// find the item
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		if((*cur)->lootslot == lootslot) {
			sitem = *cur;
			break;
		}
	}

	if (sitem && bag_item_data && Inventory::SupportsContainers(sitem->equip_slot)) {
		int16 bagstart = Inventory::CalcSlotId(sitem->equip_slot, SUB_BEGIN);

		cur = itemlist.begin();
		end = itemlist.end();
		for(; cur != end; ++cur) {
			sitem2 = *cur;
			if(sitem2->equip_slot >= bagstart && sitem2->equip_slot < bagstart + 10) {
				bag_item_data[sitem2->equip_slot - bagstart] = sitem2;
			}
		}
	}

	return sitem;
}

uint32 Corpse::GetWornItem(int16 equipSlot) const {
	ItemList::const_iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		ServerLootItem_Struct* item = *cur;
		if (item->equip_slot == equipSlot) {
			return item->item_id;
		}
	}

	return 0;
}

void Corpse::RemoveItem(uint16 lootslot) {
	if (lootslot == 0xFFFF)
		return;

	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		ServerLootItem_Struct* sitem = *cur;
		if (sitem->lootslot == lootslot) {
			RemoveItem(sitem);
			return;
		}
	}
}

void Corpse::RemoveItem(ServerLootItem_Struct* item_data){
	uint8 material;
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		ServerLootItem_Struct* sitem = *cur;
		if (sitem == item_data) {
			is_corpse_changed = true;
			itemlist.erase(cur);

			material = Inventory::CalcMaterialFromSlot(sitem->equip_slot);
			if(material != _MaterialInvalid)
				SendWearChange(material);

			safe_delete(sitem);

			return;
		}
	}
}

void Corpse::SetCash(uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_platinum) {
	this->copper = in_copper;
	this->silver = in_silver;
	this->gold = in_gold;
	this->platinum = in_platinum;
	is_corpse_changed = true;
}

void Corpse::RemoveCash() {
	this->copper = 0;
	this->silver = 0;
	this->gold = 0;
	this->platinum = 0;
	is_corpse_changed = true;
}

bool Corpse::IsEmpty() const {
	if (copper != 0 || silver != 0 || gold != 0 || platinum != 0)
		return false;
	return(itemlist.size() == 0);
}

bool Corpse::Process() {
	if (player_corpse_depop)
		return false;

	if(corpse_delay_timer.Check()) {
		for (int i=0; i<MAX_LOOTERS; i++)
			allowed_looters[i] = 0;
		corpse_delay_timer.Disable();
		return true;
	}

	if(corpse_graveyard_timer.Check()) {
		if(zone->HasGraveyard()) {
			Save();
			player_corpse_depop = true;
			database.SendCharacterCorpseToGraveyard(corpse_db_id, zone->graveyard_zoneid(),
				(zone->GetZoneID() == zone->graveyard_zoneid()) ? zone->GetInstanceID() : 0, zone->graveyard_x(),
				zone->graveyard_y(), zone->graveyard_z(), zone->graveyard_heading());
			corpse_graveyard_timer.Disable();
			ServerPacket* pack = new ServerPacket(ServerOP_SpawnPlayerCorpse, sizeof(SpawnPlayerCorpse_Struct));
			SpawnPlayerCorpse_Struct* spc = (SpawnPlayerCorpse_Struct*)pack->pBuffer;
			spc->player_corpse_id = corpse_db_id;
			spc->zone_id = zone->graveyard_zoneid();
			worldserver.SendPacket(pack);
			safe_delete(pack);
			LogFile->write(EQEMuLog::Debug, "Moved %s player corpse to the designated graveyard in zone %s.", this->GetName(), database.GetZoneName(zone->graveyard_zoneid()));
			corpse_db_id = 0;
		}

		corpse_graveyard_timer.Disable();
		return false;
	}
	/*
	if(corpse_res_timer.Check()) {
		can_rez = false;
		corpse_res_timer.Disable();
	}
	*/
	if(corpse_decay_timer.Check()) {
		if(!RuleB(Zone, EnableShadowrest))
			Delete();
		else {
			if(database.BuryCharacterCorpse(corpse_db_id)) {
				Save();
				player_corpse_depop = true;
				corpse_db_id = 0;
				LogFile->write(EQEMuLog::Debug, "Tagged %s player corpse has burried.", this->GetName());
			}
			else
			{
				LogFile->write(EQEMuLog::Error, "Unable to bury %s player corpse.", this->GetName());
				return true;
			}
		}
		corpse_decay_timer.Disable();
		return false;
	}

	return true;
}

void Corpse::SetDecayTimer(uint32 decaytime) {
	if (decaytime == 0)
		corpse_decay_timer.Trigger();
	else
		corpse_decay_timer.Start(decaytime);
}

bool Corpse::CanMobLoot(int charid) {
	uint8 z=0;
	for(int i=0; i<MAX_LOOTERS; i++) {
		if(allowed_looters[i] != 0)
			z++;

		if (allowed_looters[i] == charid)
			return true;
	}
	if(z == 0)
		return true;
	else
		return false;
}

void Corpse::AllowMobLoot(Mob *them, uint8 slot) {
	if(slot >= MAX_LOOTERS)
		return;
	if(them == nullptr || !them->IsClient())
		return;

	allowed_looters[slot] = them->CastToClient()->CharacterID();
}

// @merth: this function needs some work
void Corpse::MakeLootRequestPackets(Client* client, const EQApplicationPacket* app) {
	// Added 12/08. Started compressing loot struct on live.
	char tmp[10];
	if(player_corpse_depop) {
		SendLootReqErrorPacket(client, 0);
		return;
	}

	if(IsPlayerCorpse() && corpse_db_id == 0) {
		// SendLootReqErrorPacket(client, 0);
		client->Message(13, "Warning: Corpse's dbid = 0! Corpse will not survive zone shutdown!");
		std::cout << "Error: PlayerCorpse::MakeLootRequestPackets: dbid = 0!" << std::endl;
		// return;
	}

	if(is_locked && client->Admin() < 100) {
		SendLootReqErrorPacket(client, 0);
		client->Message(13, "Error: Corpse locked by GM.");
		return;
	}

	if(being_looted_by == 0) { being_looted_by = 0xFFFFFFFF; }

	if(this->being_looted_by != 0xFFFFFFFF) {
		// lets double check....
		Entity* looter = entity_list.GetID(this->being_looted_by);
		if(looter == 0) { this->being_looted_by = 0xFFFFFFFF; }
	}

	uint8 tCanLoot = 1;
	bool lootcoin = false;
	if(database.GetVariable("LootCoin", tmp, 9)) { lootcoin = (atoi(tmp) == 1); }

	if(this->being_looted_by != 0xFFFFFFFF && this->being_looted_by != client->GetID()) {
		SendLootReqErrorPacket(client, 0);
		tCanLoot = 0;
	}
	else if(IsPlayerCorpse() && char_id == client->CharacterID()) { tCanLoot = 2; }
	else if((IsNPCCorpse() || become_npc) && CanMobLoot(client->CharacterID())) { tCanLoot = 2; }
	else if(GetPKItem() == -1 && CanMobLoot(client->CharacterID())) { tCanLoot = 3; } //pvp loot all items, variable cash
	else if(GetPKItem() == 1 && CanMobLoot(client->CharacterID())) { tCanLoot = 4; } //pvp loot 1 item, variable cash
	else if(GetPKItem() > 1 && CanMobLoot(client->CharacterID())) { tCanLoot = 5; } //pvp loot 1 set item, variable cash

	if(tCanLoot == 1) { if(client->Admin() < 100 || !client->GetGM()) { SendLootReqErrorPacket(client, 2); } }

	if(tCanLoot >= 2 || (tCanLoot == 1 && client->Admin() >= 100 && client->GetGM())) {
		this->being_looted_by = client->GetID();
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoneyOnCorpse, sizeof(moneyOnCorpseStruct));
		moneyOnCorpseStruct* d = (moneyOnCorpseStruct*) outapp->pBuffer;

		d->response		= 1;
		d->unknown1		= 0x42;
		d->unknown2		= 0xef;
		if(tCanLoot == 2 || (tCanLoot >= 3 && lootcoin)) { // dont take the coin off if it's a gm peeking at the corpse

			if(!IsPlayerCorpse() && client->IsGrouped() && client->AutoSplitEnabled() && client->GetGroup()) {
				d->copper		= 0;
				d->silver		= 0;
				d->gold			= 0;
				d->platinum		= 0;
				Group *cgroup = client->GetGroup();
				cgroup->SplitMoney(GetCopper(), GetSilver(), GetGold(), GetPlatinum(), client);
			}
			else {
				d->copper		= this->GetCopper();
				d->silver		= this->GetSilver();
				d->gold			= this->GetGold();
				d->platinum		= this->GetPlatinum();
				client->AddMoneyToPP(GetCopper(), GetSilver(), GetGold(), GetPlatinum(), false);
			}

			RemoveCash();
			Save();
		}

		outapp->priority = 6;
		client->QueuePacket(outapp);
		safe_delete(outapp);
		if(tCanLoot == 5) {
			int pkitem = GetPKItem();
			const Item_Struct* item = database.GetItem(pkitem);
			ItemInst* inst = database.CreateItem(item, item->MaxCharges);
			if(inst) {
				client->SendItemPacket(EmuConstants::CORPSE_BEGIN, inst, ItemPacketLoot);
				safe_delete(inst);
			}
			else { client->Message(13, "Could not find item number %i to send!!", GetPKItem()); }

			client->QueuePacket(app);
			return;
		}

		int i = 0;
		const Item_Struct* item = 0;
		ItemList::iterator cur,end;
		cur = itemlist.begin();
		end = itemlist.end();

		uint8 containercount = 0;

		int corpselootlimit = EQLimits::InventoryMapSize(MapCorpse, client->GetClientVersion());

		for(; cur != end; ++cur) {
			ServerLootItem_Struct* item_data = *cur;
			item_data->lootslot = 0xFFFF;

			// Dont display the item if it's in a bag

			// Added cursor queue slots to corpse item visibility list. Nothing else should be making it to corpse.
			if(!IsPlayerCorpse() || item_data->equip_slot <= MainCursor || item_data->equip_slot == MainPowerSource || tCanLoot>=3 ||
				(item_data->equip_slot >= 8000 && item_data->equip_slot <= 8999)) {
				if(i < corpselootlimit) {
					item = database.GetItem(item_data->item_id);
					if(client && item) {
						ItemInst* inst = database.CreateItem(item, item_data->charges, item_data->aug_1, item_data->aug_2, item_data->aug_3, item_data->aug_4, item_data->aug_5);
						if(inst) {
							// MainGeneral1 is the corpse inventory start offset for Ti(EMu) - CORPSE_END = MainGeneral1 + MainCursor
							client->SendItemPacket(i + EmuConstants::CORPSE_BEGIN, inst, ItemPacketLoot);
							safe_delete(inst);
						}

						item_data->lootslot = i;
					}
				}

				i++;
			}
		}

		if(IsPlayerCorpse() && (char_id == client->CharacterID() || client->GetGM())) {
			if(i > corpselootlimit) {
				client->Message(15, "*** This corpse contains more items than can be displayed! ***");
				client->Message(0, "Remove items and re-loot corpse to access remaining inventory.");
				client->Message(0, "(%s contains %i additional %s.)", GetName(), (i - corpselootlimit), (i - corpselootlimit) == 1 ? "item" : "items");
			}

			if(IsPlayerCorpse() && i == 0 && itemlist.size() > 0) { // somehow, player corpse contains items, but client doesn't see them...
				client->Message(13, "This corpse contains items that are inaccessable!");
				client->Message(15, "Contact a GM for item replacement, if necessary.");
				client->Message(15, "BUGGED CORPSE [DBID: %i, Name: %s, Item Count: %i]", GetDBID(), GetName(), itemlist.size());

				cur = itemlist.begin();
				end = itemlist.end();
				for(; cur != end; ++cur) {
					ServerLootItem_Struct* item_data = *cur;
					item = database.GetItem(item_data->item_id);
					LogFile->write(EQEMuLog::Debug, "Corpse Looting: %s was not sent to client loot window (corpse_dbid: %i, charname: %s(%s))", item->Name, GetDBID(), client->GetName(), client->GetGM() ? "GM" : "Owner");
					client->Message(0, "Inaccessable Corpse Item: %s", item->Name);
				}
			}
		}
	}

	// Disgrace: Client seems to require that we send the packet back...
	client->QueuePacket(app);

	// This is required for the 'Loot All' feature to work for SoD clients. I expect it is to tell the client that the
	// server has now sent all the items on the corpse.
	if(client->GetClientVersion() >= EQClientSoD) { SendLootReqErrorPacket(client, 6); }
}

void Corpse::LootItem(Client* client, const EQApplicationPacket* app) {
	/* This gets sent no matter what as a sort of ACK */
	client->QueuePacket(app);

	if (!loot_cooldown_timer.Check()) {
		SendEndLootErrorPacket(client);
		//unlock corpse for others
		if (this->being_looted_by = client->GetID()) {
			being_looted_by = 0xFFFFFFFF;
		}
		return;
	}

	/* To prevent item loss for a player using 'Loot All' who doesn't have inventory space for all their items. */
	if (RuleB(Character, CheckCursorEmptyWhenLooting) && !client->GetInv().CursorEmpty()) {
		client->Message(13, "You may not loot an item while you have an item on your cursor.");
		SendEndLootErrorPacket(client);
		/* Unlock corpse for others */
		if (this->being_looted_by = client->GetID()) {
			being_looted_by = 0xFFFFFFFF;
		}
		return;
	}

	LootingItem_Struct* lootitem = (LootingItem_Struct*)app->pBuffer;

	if (this->being_looted_by != client->GetID()) {
		client->Message(13, "Error: Corpse::LootItem: BeingLootedBy != client");
		SendEndLootErrorPacket(client);
		return;
	}
	if (IsPlayerCorpse() && !CanMobLoot(client->CharacterID()) && !become_npc && (char_id != client->CharacterID() && client->Admin() < 150)) {
		client->Message(13, "Error: This is a player corpse and you dont own it.");
		SendEndLootErrorPacket(client);
		return;
	}
	if (is_locked && client->Admin() < 100) {
		SendLootReqErrorPacket(client, 0);
		client->Message(13, "Error: Corpse locked by GM.");
		return;
	}
	if (IsPlayerCorpse() && (char_id != client->CharacterID()) && CanMobLoot(client->CharacterID()) && GetPKItem() == 0){
		client->Message(13, "Error: You cannot loot any more items from this corpse.");
		SendEndLootErrorPacket(client);
		being_looted_by = 0xFFFFFFFF;
		return;
	}
	const Item_Struct* item = 0;
	ItemInst *inst = 0;
	ServerLootItem_Struct* item_data = nullptr, *bag_item_data[10];

	memset(bag_item_data, 0, sizeof(bag_item_data));
	if (GetPKItem() > 1){
		item = database.GetItem(GetPKItem());
	}
	else if (GetPKItem() == -1 || GetPKItem() == 1){
		item_data = GetItem(lootitem->slot_id - EmuConstants::CORPSE_BEGIN); //dont allow them to loot entire bags of items as pvp reward
	}
	else{
		item_data = GetItem(lootitem->slot_id - EmuConstants::CORPSE_BEGIN, bag_item_data);
	}

	if (GetPKItem()<=1 && item_data != 0) {
		item = database.GetItem(item_data->item_id);
	}

	if (item != 0) {
		if (item_data){
			inst = database.CreateItem(item, item_data ? item_data->charges : 0, item_data->aug_1, item_data->aug_2, item_data->aug_3, item_data->aug_4, item_data->aug_5);
		}
		else {
			inst = database.CreateItem(item);
		}
	}

	if (client && inst) {
		if (client->CheckLoreConflict(item)) {
			client->Message_StringID(0, LOOT_LORE_ERROR);
			SendEndLootErrorPacket(client);
			being_looted_by = 0;
			delete inst;
			return;
		}

		if (inst->IsAugmented()) {
			for (int i = AUG_BEGIN; i < EmuConstants::ITEM_COMMON_SIZE; i++) {
				ItemInst *itm = inst->GetAugment(i);
				if (itm) {
					if (client->CheckLoreConflict(itm->GetItem())) {
						client->Message_StringID(0, LOOT_LORE_ERROR);
						SendEndLootErrorPacket(client);
						being_looted_by = 0;
						delete inst;
						return;
					}
				}
			}
		}

		char buf[88];
		char corpse_name[64];
		strcpy(corpse_name, orgname);
		snprintf(buf, 87, "%d %d %s", inst->GetItem()->ID, inst->GetCharges(), EntityList::RemoveNumbers(corpse_name));
		buf[87] = '\0';
		std::vector<EQEmu::Any> args;
		args.push_back(inst);
		args.push_back(this);
		parse->EventPlayer(EVENT_LOOT, client, buf, 0, &args);
		parse->EventItem(EVENT_LOOT, client, inst, this, buf, 0);

		if ((RuleB(Character, EnableDiscoveredItems))) {
			if (client && !client->GetGM() && !client->IsDiscovered(inst->GetItem()->ID))
				client->DiscoverItem(inst->GetItem()->ID);
		}

		if (zone->adv_data) {
			ServerZoneAdventureDataReply_Struct *ad = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
			if (ad->type == Adventure_Collect && !IsPlayerCorpse()) {
				if (ad->data_id == inst->GetItem()->ID) {
					zone->DoAdventureCountIncrease();
				}
			}
		}

		/* First add it to the looter - this will do the bag contents too */
		if (lootitem->auto_loot) {
			if (!client->AutoPutLootInInventory(*inst, true, true, bag_item_data))
				client->PutLootInInventory(MainCursor, *inst, bag_item_data);
		}
		else {
			client->PutLootInInventory(MainCursor, *inst, bag_item_data);
		}

		/* Update any tasks that have an activity to loot this item */
		if (RuleB(TaskSystem, EnableTaskSystem))
			client->UpdateTasksForItem(ActivityLoot, item->ID);

		/* Remove it from Corpse */
		if (item_data){
			/* Delete needs to be before RemoveItem because its deletes the pointer for item_data/bag_item_data */
			database.DeleteItemOffCharacterCorpse(this->corpse_db_id, item_data->equip_slot, item_data->item_id);
			/* Delete Item Instance */
			RemoveItem(item_data->lootslot);
		}

		/* Remove Bag Contents */
		if (item->ItemClass == ItemClassContainer && (GetPKItem() != -1 || GetPKItem() != 1)) {
			for (int i = SUB_BEGIN; i < EmuConstants::ITEM_CONTAINER_SIZE; i++) {
				if (bag_item_data[i]) {
					/* Delete needs to be before RemoveItem because its deletes the pointer for item_data/bag_item_data */
					database.DeleteItemOffCharacterCorpse(this->corpse_db_id, bag_item_data[i]->equip_slot, bag_item_data[i]->item_id);
					/* Delete Item Instance */
					RemoveItem(bag_item_data[i]);
				}
			}
		}

		if (GetPKItem() != -1){
			SetPKItem(0);
		}

		/* Send message with item link to groups and such */
		char *link = 0, *link2 = 0; //just like a db query :-)
		client->MakeItemLink(link2, inst);
		MakeAnyLenString(&link, "%c" "%s" "%s" "%c",
			0x12,
			link2,
			item->Name,
			0x12);
		safe_delete_array(link2);

		client->Message_StringID(MT_LootMessages, LOOTED_MESSAGE, link);
		if(!IsPlayerCorpse()) {
			Group *g = client->GetGroup();
			if(g != nullptr) {
				g->GroupMessage_StringID(client, MT_LootMessages, OTHER_LOOTED_MESSAGE, client->GetName(), link);
			} else {
				Raid *r = client->GetRaid();
				if(r != nullptr) {
					r->RaidMessage_StringID(client, MT_LootMessages, OTHER_LOOTED_MESSAGE, client->GetName(), link);
				}
			}
		}
		safe_delete_array(link);
	}
	else {
		SendEndLootErrorPacket(client);
		safe_delete(inst);
		return;
	}

	if (IsPlayerCorpse()){
		client->SendItemLink(inst);
	}
	else{
		client->SendItemLink(inst, true);
	}

	safe_delete(inst);
}

void Corpse::EndLoot(Client* client, const EQApplicationPacket* app) {
	EQApplicationPacket* outapp = new EQApplicationPacket;
	outapp->SetOpcode(OP_LootComplete);
	outapp->size = 0;
	client->QueuePacket(outapp);
	safe_delete(outapp);

	//client->Save();		//inventory operations auto-commit
	this->being_looted_by = 0xFFFFFFFF;
	if (this->IsEmpty())
		Delete();
	else
		Save();
}

void Corpse::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	Mob::FillSpawnStruct(ns, ForWho);

	ns->spawn.max_hp = 120;

	if (IsPlayerCorpse())
		ns->spawn.NPC = 3;
	else
		ns->spawn.NPC = 2;
}

void Corpse::QueryLoot(Client* to) {
	int x = 0, y = 0; // x = visible items, y = total items
	to->Message(0, "Coin: %ip, %ig, %is, %ic", platinum, gold, silver, copper);

	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();

	int corpselootlimit = EQLimits::InventoryMapSize(MapCorpse, to->GetClientVersion());

	for(; cur != end; ++cur) {
		ServerLootItem_Struct* sitem = *cur;

		if (IsPlayerCorpse()) {
			if (sitem->equip_slot >= EmuConstants::GENERAL_BAGS_BEGIN && sitem->equip_slot <= EmuConstants::CURSOR_BAG_END)
				sitem->lootslot = 0xFFFF;
			else
				x < corpselootlimit ? sitem->lootslot = x : sitem->lootslot = 0xFFFF;

			const Item_Struct* item = database.GetItem(sitem->item_id);

			if (item)
				to->Message((sitem->lootslot == 0xFFFF), "LootSlot: %i (EquipSlot: %i) Item: %s (%d), Count: %i", static_cast<int16>(sitem->lootslot), sitem->equip_slot, item->Name, item->ID, sitem->charges);
			else
				to->Message((sitem->lootslot == 0xFFFF), "Error: 0x%04x", sitem->item_id);

			if (sitem->lootslot != 0xFFFF)
				x++;

			y++;
		}
		else {
			sitem->lootslot=y;
			const Item_Struct* item = database.GetItem(sitem->item_id);

			if (item)
				to->Message(0, "LootSlot: %i Item: %s (%d), Count: %i", sitem->lootslot, item->Name, item->ID, sitem->charges);
			else
				to->Message(0, "Error: 0x%04x", sitem->item_id);

			y++;
		}
	}

	if (IsPlayerCorpse()) {
		to->Message(0, "%i visible %s (%i total) on %s (DBID: %i).", x, x==1?"item":"items", y, this->GetName(), this->GetDBID());
	}
	else {
		to->Message(0, "%i %s on %s.", y, y==1?"item":"items", this->GetName());
	}
}

bool Corpse::Summon(Client* client, bool spell, bool CheckDistance) {
	uint32 dist2 = 10000; // pow(100, 2);
	if (!spell) {
		if (this->GetCharID() == client->CharacterID()) {
			if (IsLocked() && client->Admin() < 100) {
				client->Message(13, "That corpse is locked by a GM.");
				return false;
			}
			if (!CheckDistance || (DistNoRootNoZ(*client) <= dist2)) {
				GMMove(client->GetX(), client->GetY(), client->GetZ());
				is_corpse_changed = true;
			}
			else {
				client->Message(0, "Corpse is too far away.");
				return false;
			}
		}
		else
		{
			bool consented = false;
			std::list<std::string>::iterator itr;
			for(itr = client->consent_list.begin(); itr != client->consent_list.end(); ++itr) {
				if(strcmp(this->GetOwnerName(), itr->c_str()) == 0) {
					if (!CheckDistance || (DistNoRootNoZ(*client) <= dist2)) {
						GMMove(client->GetX(), client->GetY(), client->GetZ());
						is_corpse_changed = true;
					}
					else {
						client->Message(0, "Corpse is too far away.");
						return false;
					}
					consented = true;
				}
			}
			if(!consented) {
				client->Message(0, "You do not have permission to move this corpse.");
				return false;
			}
		}
	}
	else {
		GMMove(client->GetX(), client->GetY(), client->GetZ());
		is_corpse_changed = true;
	}
	Save();
	return true;
}

void Corpse::CompleteRezz(){
	rezzexp = 0;
	is_corpse_changed = true;
	this->Save();
}

void Corpse::Spawn() {
	EQApplicationPacket* app = new EQApplicationPacket;
	this->CreateSpawnPacket(app, this);
	entity_list.QueueClients(this, app);
	safe_delete(app);
}

uint32 Corpse::GetEquipment(uint8 material_slot) const {
	int invslot;

	if(material_slot > EmuConstants::MATERIAL_END) {
		return NO_ITEM;
	}

	invslot = Inventory::CalcSlotFromMaterial(material_slot);
	if(invslot == INVALID_INDEX) // GetWornItem() should be returning a NO_ITEM for any invalid index...
		return NO_ITEM;

	return GetWornItem(invslot);
}

uint32 Corpse::GetEquipmentColor(uint8 material_slot) const {
	const Item_Struct *item;

	if(material_slot > EmuConstants::MATERIAL_END) {
		return 0;
	}

	item = database.GetItem(GetEquipment(material_slot));
	if(item != NO_ITEM) {
		return item_tint[material_slot].rgb.use_tint ?
			item_tint[material_slot].color :
			item->Color;
	}

	return 0;
}

void Corpse::AddLooter(Mob* who) {
	for (int i=0; i<MAX_LOOTERS; i++) {
		if (allowed_looters[i] == 0) {
			allowed_looters[i] = who->CastToClient()->CharacterID();
			break;
		}
	}
}

void Corpse::LoadPlayerCorpseDecayTime(uint32 corpse_db_id){
	if(!corpse_db_id)
		return;

	uint32 active_corpse_decay_timer = database.GetCharacterCorpseDecayTimer(corpse_db_id);
	if (active_corpse_decay_timer > 0 && RuleI(Character, CorpseDecayTimeMS) > (active_corpse_decay_timer * 1000)) {
		corpse_decay_timer.SetTimer(RuleI(Character, CorpseDecayTimeMS) - (active_corpse_decay_timer * 1000));
	}
	else {
		corpse_decay_timer.SetTimer(2000);
	}
	if (active_corpse_decay_timer > 0 && RuleI(Zone, GraveyardTimeMS) > (active_corpse_decay_timer * 1000)) {
		corpse_graveyard_timer.SetTimer(RuleI(Zone, GraveyardTimeMS) - (active_corpse_decay_timer * 1000));
	}
	else {
		corpse_graveyard_timer.SetTimer(3000);
	}
}

/*
**	Corpse slot translations are needed until corpse database blobs are converted
**
**	To account for the addition of MainPowerSource, MainGeneral9 and MainGeneral10 into
**	the contiguous possessions slot enumeration, the following designations will be used:
**
**	Designatiom			Server		Corpse		Offset
**	--------------------------------------------------
**	MainCharm			0			0			0
**	...					...			...			0
**	MainWaist			20			20			0
**	MainPowerSource		21			9999		+9978
**	MainAmmo			22			21			-1
**
**	MainGeneral1		23			22			-1
**	...					...			...			-1
**	MainGeneral8		30			29			-1
**	MainGeneral9		31			9997		+9966
**	MainGeneral10		32			9998		+9966
**
**	MainCursor			33			30			-3
**
**	MainGeneral1_1		251			251			0
**	...					...			...			0
**	MainGeneral8_10		330			330			0
**	MainGeneral9_1		331			341			+10
**	...					...			...			+10
**	MainGeneral10_10	350			360			+10
**
**	MainCursor_1		351			331			-20
**	...					...			...			-20
**	MainCursor_10		360			340			-20
**
**	(Not all slot designations are valid to all clients..see <client>##_constants.h files for valid slot enumerations)
*/
int16 Corpse::ServerToCorpseSlot(int16 server_slot)
{
	return server_slot; // temporary return

	/*
	switch (server_slot)
	{
	case MainPowerSource:
		return 9999;
	case MainGeneral9:
		return 9997;
	case MainGeneral10:
		return 9998;
	case MainCursor:
		return 30;
	case MainAmmo:
	case MainGeneral1:
	case MainGeneral2:
	case MainGeneral3:
	case MainGeneral4:
	case MainGeneral5:
	case MainGeneral6:
	case MainGeneral7:
	case MainGeneral8:
		return server_slot - 1;
	default:
		if (server_slot >= EmuConstants::CURSOR_BAG_BEGIN && server_slot <= EmuConstants::CURSOR_BAG_END)
			return server_slot - 20;
		else if (server_slot >= EmuConstants::GENERAL_BAGS_END - 19 && server_slot <= EmuConstants::GENERAL_BAGS_END)
			return server_slot + 10;
		else
			return server_slot;
	}
	*/
}

int16 Corpse::CorpseToServerSlot(int16 corpse_slot)
{
	return corpse_slot; // temporary return

	/*
	switch (corpse_slot)
	{
	case 9999:
		return MainPowerSource;
	case 9997:
		return MainGeneral9;
	case 9998:
		return MainGeneral10;
	case 30:
		return MainCursor;
	case 21: // old SLOT_AMMO
	case 22: // old PERSONAL_BEGIN
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29: // old PERSONAL_END
		return corpse_slot + 1;
	default:
		if (corpse_slot >= 331 && corpse_slot <= 340)
			return corpse_slot + 20;
		else if (corpse_slot >= 341 && corpse_slot <= 360)
			return corpse_slot - 10;
		else
			return corpse_slot;
	}
	*/
}
