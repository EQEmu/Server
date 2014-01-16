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
#include "../common/StringUtil.h"
#include "../common/crc32.h"
#include "StringIDs.h"
#include "worldserver.h"
#include "../common/rulesys.h"
#include "QuestParserCollection.h"

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

Corpse* Corpse::LoadFromDBData(uint32 in_dbid, uint32 in_charid, char* in_charname, uchar* in_data, uint32 in_datasize, float in_x, float in_y, float in_z, float in_heading, char* timeofdeath, bool rezzed, bool wasAtGraveyard) {
	if (in_datasize < sizeof(classic_db::DBPlayerCorpse_Struct)) {
		LogFile->write(EQEMuLog::Error, "Corpse::LoadFromDBData: Corrupt data: in_datasize < sizeof(DBPlayerCorpse_Struct)");
		return 0;
	}
	classic_db::DBPlayerCorpse_Struct* dbpc = (classic_db::DBPlayerCorpse_Struct*) in_data;
	bool isSoF = true;

	uint32 esize1 = (sizeof(DBPlayerCorpse_Struct) + (dbpc->itemcount * sizeof(player_lootitem::ServerLootItem_Struct)));
	uint32 esize2 = (sizeof(classic_db::DBPlayerCorpse_Struct) + (dbpc->itemcount * sizeof(player_lootitem::ServerLootItem_Struct)));
	if (in_datasize != esize1) {
		LogFile->write(EQEMuLog::Error, "Corpse::LoadFromDBData: Corrupt data: in_datasize (%i) != expected size (%i) Continuing on...", in_datasize, esize1);
		if (in_datasize != esize2) {
			LogFile->write(EQEMuLog::Error, "Corpse::LoadFromDBData: Corrupt data: in_datasize (%i) != expected size (%i) Your corpse is done broke, sir.", in_datasize, esize2);
			return 0;
		}
		else
		{
			isSoF = false;
		}
	}

	if(isSoF)
	{
		DBPlayerCorpse_Struct* dbpcs = (DBPlayerCorpse_Struct*) in_data;
		if (dbpcs->crc != CRC32::Generate(&((uchar*) dbpcs)[4], in_datasize - 4)) {
			LogFile->write(EQEMuLog::Error, "Corpse::LoadFromDBData: Corrupt data: crc failure");
			return 0;
		}
		ItemList itemlist;
		ServerLootItem_Struct* tmp = 0;
		for (unsigned int i=0; i < dbpcs->itemcount; i++) {
			tmp = new ServerLootItem_Struct;
			memcpy(tmp, &dbpcs->items[i], sizeof(player_lootitem::ServerLootItem_Struct));
			itemlist.push_back(tmp);
		}

		// Little hack to account for the fact the race in the corpse struct is a uint8 and Froglok/Drakkin race number > 255
		// and to maintain backwards compatability with existing corpses in the database.
		uint16 RealRace;

		switch(dbpcs->race) {
			case 254:
				RealRace = DRAKKIN;
				break;
			case 255:
				RealRace = FROGLOK;
				break;
			default:
				RealRace = dbpc->race;
		}

		Corpse* pc = new Corpse(in_dbid, in_charid, in_charname, &itemlist, dbpcs->copper, dbpcs->silver, dbpcs->gold, dbpcs->plat, in_x, in_y, in_z, in_heading, dbpcs->size, dbpcs->gender, RealRace, dbpcs->class_, dbpcs->deity, dbpcs->level, dbpcs->texture, dbpcs->helmtexture, dbpcs->exp, wasAtGraveyard);
		if (dbpcs->locked)
			pc->Lock();

		// load tints
		memcpy(pc->item_tint, dbpcs->item_tint, sizeof(pc->item_tint));
		// appearance
		pc->haircolor = dbpcs->haircolor;
		pc->beardcolor = dbpcs->beardcolor;
		pc->eyecolor1 = dbpcs->eyecolor1;
		pc->eyecolor2 = dbpcs->eyecolor2;
		pc->hairstyle = dbpcs->hairstyle;
		pc->luclinface = dbpcs->face;
		pc->beard = dbpcs->beard;
		pc->drakkin_heritage = dbpcs->drakkin_heritage;
		pc->drakkin_tattoo = dbpcs->drakkin_tattoo;
		pc->drakkin_details = dbpcs->drakkin_details;
		pc->Rezzed(rezzed);
		pc->become_npc = false;
		return pc;
	}
	else
	{
		if (dbpc->crc != CRC32::Generate(&((uchar*) dbpc)[4], in_datasize - 4)) {
			LogFile->write(EQEMuLog::Error, "Corpse::LoadFromDBData: Corrupt data: crc failure");
			return 0;
		}
		ItemList itemlist;
		ServerLootItem_Struct* tmp = 0;
		for (unsigned int i=0; i < dbpc->itemcount; i++) {
			tmp = new ServerLootItem_Struct;
			memcpy(tmp, &dbpc->items[i], sizeof(player_lootitem::ServerLootItem_Struct));
			itemlist.push_back(tmp);
		}

		// Little hack to account for the fact the race in the corpse struct is a uint8 and Froglok/Drakkin race number > 255
		// and to maintain backwards compatability with existing corpses in the database.
		uint16 RealRace;

		switch(dbpc->race) {
			case 254:
				RealRace = DRAKKIN;
				break;
			case 255:
				RealRace = FROGLOK;
				break;
			default:
				RealRace = dbpc->race;
		}

		Corpse* pc = new Corpse(in_dbid, in_charid, in_charname, &itemlist, dbpc->copper, dbpc->silver, dbpc->gold, dbpc->plat, in_x, in_y, in_z, in_heading, dbpc->size, dbpc->gender, RealRace, dbpc->class_, dbpc->deity, dbpc->level, dbpc->texture, dbpc->helmtexture,dbpc->exp, wasAtGraveyard);
		if (dbpc->locked)
			pc->Lock();

		// load tints
		memcpy(pc->item_tint, dbpc->item_tint, sizeof(pc->item_tint));
		// appearance
		pc->haircolor = dbpc->haircolor;
		pc->beardcolor = dbpc->beardcolor;
		pc->eyecolor1 = dbpc->eyecolor1;
		pc->eyecolor2 = dbpc->eyecolor2;
		pc->hairstyle = dbpc->hairstyle;
		pc->luclinface = dbpc->face;
		pc->beard = dbpc->beard;
		pc->drakkin_heritage = 0;
		pc->drakkin_tattoo = 0;
		pc->drakkin_details = 0;
		pc->Rezzed(rezzed);
		pc->become_npc = false;
		return pc;
	}
}

// To be used on NPC death and ZoneStateLoad
// Mongrel: added see_invis and see_invis_undead
Corpse::Corpse(NPC* in_npc, ItemList* in_itemlist, uint32 in_npctypeid, const NPCType** in_npctypedata, uint32 in_decaytime)
// vesuvias - appearence fix
: Mob("Unnamed_Corpse","",0,0,in_npc->GetGender(),in_npc->GetRace(),in_npc->GetClass(),BT_Humanoid,//bodytype added
	in_npc->GetDeity(),in_npc->GetLevel(),in_npc->GetNPCTypeID(),in_npc->GetSize(),0,
	in_npc->GetHeading(),in_npc->GetX(),in_npc->GetY(),in_npc->GetZ(),0,
	in_npc->GetTexture(),in_npc->GetHelmTexture(),
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
	pIsChanged = false;
	p_PlayerCorpse = false;
	pLocked = false;
	BeingLootedBy = 0xFFFFFFFF;
	if (in_itemlist) {
		itemlist = *in_itemlist;
		in_itemlist->clear();
	}

	SetCash(in_npc->GetCopper(), in_npc->GetSilver(), in_npc->GetGold(), in_npc->GetPlatinum());

	npctype_id = in_npctypeid;
	SetPKItem(0);
	charid = 0;
	dbid = 0;
	p_depop = false;
	strcpy(orgname, in_npc->GetName());
	strcpy(name, in_npc->GetName());
	// Added By Hogie
	for(int count = 0; count < 100; count++) {
		if ((level >= npcCorpseDecayTimes[count].minlvl) && (level <= npcCorpseDecayTimes[count].maxlvl)) {
			corpse_decay_timer.SetTimer(npcCorpseDecayTimes[count].seconds*1000);
			break;
		}
	}
	if(IsEmpty())
	{
		corpse_decay_timer.SetTimer(RuleI(NPC,EmptyNPCCorpseDecayTimeMS)+1000);
	}

	if(in_npc->HasPrivateCorpse())
	{
		corpse_delay_timer.SetTimer(corpse_decay_timer.GetRemainingTime() + 1000);
	}

	// Added By Hogie -- End
	for (int i=0; i<MAX_LOOTERS; i++)
		looters[i] = 0;
	this->rezzexp = 0;
}

// To be used on PC death
// Mongrel: added see_invis and see_invis_undead
Corpse::Corpse(Client* client, int32 in_rezexp)
// vesuvias - appearence fix
: Mob
(
	"Unnamed_Corpse",
	"",
	0,
	0,
	client->GetGender(),
	client->GetRace(),
	client->GetClass(),
	BT_Humanoid, // bodytype added
	client->GetDeity(),
	client->GetLevel(),
	0,
	client->GetSize(),
	0,
	client->GetHeading(),	// heading
	client->GetX(),
	client->GetY(),
	client->GetZ(),
	0,
	client->GetTexture(),
	client->GetHelmTexture(),
	0,	// AC
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,	// CHA
	client->GetPP().haircolor,
	client->GetPP().beardcolor,
	client->GetPP().eyecolor1,
	client->GetPP().eyecolor2,
	client->GetPP().hairstyle,
	client->GetPP().face,
	client->GetPP().beard,
	client->GetPP().drakkin_heritage,
	client->GetPP().drakkin_tattoo,
	client->GetPP().drakkin_details,
	0,
	0xff,	// aa title
	0,
	0,
	0,
	0,
	0,
	0,
	0,	// qglobal
	0,	// maxlevel
	0	// scalerate
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

	if(!zone->HasGraveyard()) {
		corpse_graveyard_timer.Disable();
	}

	memset(item_tint, 0, sizeof(item_tint));
	for (i=0; i<MAX_LOOTERS; i++)
		looters[i] = 0;

	pIsChanged		= true;
	rezzexp			= in_rezexp;
	can_rez			= true;
	p_PlayerCorpse	= true;
	pLocked			= false;
	BeingLootedBy	= 0xFFFFFFFF;
	charid			= client->CharacterID();
	dbid			= 0;
	p_depop			= false;
	copper			= 0;
	silver			= 0;
	gold			= 0;
	platinum		= 0;
	strcpy(orgname, pp->name);
	strcpy(name, pp->name);

	//become_npc was not being initialized which led to some pretty funky things with newly created corpses
	become_npc = false;

	SetPKItem(0);

	if(!RuleB(Character, LeaveNakedCorpses) || RuleB(Character, LeaveCorpses) && GetLevel() >= RuleI(Character, DeathItemLossLevel)) {
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
		for(i = 0; i <= 30; i++)
		{
			if(i == 21 && client->GetClientVersion() >= EQClientSoF) {
				item = client->GetInv().GetItem(9999);
				if((item && (!client->IsBecomeNPC())) || (item && client->IsBecomeNPC() && !item->GetItem()->NoRent)) {
					std::list<uint32> slot_list = MoveItemToCorpse(client, item, 9999);
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
				if((item && (!client->IsBecomeNPC())) || (item && client->IsBecomeNPC() && !item->GetItem()->NoRent))
				{
					std::list<uint32> slot_list = MoveItemToCorpse(client, item, i);
					removed_list.merge(slot_list);
					cursor = true;
				}
			}
		}

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
			database.RunQuery(ss.str().c_str(), ss.str().length());
		}

		if(cursor) { // all cursor items should be on corpse (client < SoF or RespawnFromHover = false)
			while(!client->GetInv().CursorEmpty())
				client->DeleteItemInInventory(SLOT_CURSOR, 0, false, false);
		}
		else { // only visible cursor made it to corpse (client >= Sof and RespawnFromHover = true)
			std::list<ItemInst*>::const_iterator start = client->GetInv().cursor_begin();
			std::list<ItemInst*>::const_iterator finish = client->GetInv().cursor_end();
			database.SaveCursor(client->CharacterID(), start, finish);
		}

		client->CalcBonuses(); // will only affect offline profile viewing of dead characters..unneeded overhead
		client->Save();
	} //end "not leaving naked corpses"

	Rezzed(false);
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
	if(item->IsType(ItemClassContainer) && ((equipslot >= 22 && equipslot <=30))) // Limit the bag check to inventory and cursor slots.
	{
		for(bagindex = 0; bagindex <= 9; bagindex++)
		{
			// For empty bags in cursor queue, slot was previously being resolved as SLOT_INVALID (-1)
			interior_slot = Inventory::CalcSlotId(equipslot, bagindex);
			interior_item = client->GetInv().GetItem(interior_slot);

			if(interior_item)
			{
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
Corpse::Corpse(uint32 in_dbid, uint32 in_charid, char* in_charname, ItemList* in_itemlist, uint32 in_copper, uint32 in_silver, uint32 in_gold, uint32 in_plat, float in_x, float in_y, float in_z, float in_heading, float in_size, uint8 in_gender, uint16 in_race, uint8 in_class, uint8 in_deity, uint8 in_level, uint8 in_texture, uint8 in_helmtexture,uint32 in_rezexp, bool wasAtGraveyard)
 : Mob("Unnamed_Corpse","",0,0,in_gender, in_race, in_class, BT_Humanoid, in_deity, in_level,0, in_size, 0, in_heading, in_x, in_y, in_z,0,in_texture,in_helmtexture,
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0xff,
	0,0,0,0,0,0,0,0,0),
	corpse_decay_timer(RuleI(Character, CorpseDecayTimeMS)),
	corpse_res_timer(RuleI(Character, CorpseResTimeMS)),
	corpse_delay_timer(RuleI(NPC, CorpseUnlockTimer)),
	corpse_graveyard_timer(RuleI(Zone, GraveyardTimeMS)),
	loot_cooldown_timer(10)
{

	//we really should be loading the decay timer here...
	LoadPlayerCorpseDecayTime(in_dbid);

	if(!zone->HasGraveyard() || wasAtGraveyard)
		corpse_graveyard_timer.Disable();

	memset(item_tint, 0, sizeof(item_tint));
	pIsChanged = false;
	p_PlayerCorpse = true;
	pLocked = false;
	BeingLootedBy = 0xFFFFFFFF;
	dbid = in_dbid;
	p_depop = false;
	charid = in_charid;
	itemlist = *in_itemlist;
	in_itemlist->clear();

	strcpy(orgname, in_charname);
	strcpy(name, in_charname);
	this->copper = in_copper;
	this->silver = in_silver;
	this->gold = in_gold;
	this->platinum = in_plat;
	rezzexp = in_rezexp;
	for (int i=0; i<MAX_LOOTERS; i++)
		looters[i] = 0;
	SetPKItem(0);
}

Corpse::~Corpse() {
	if (p_PlayerCorpse && !(p_depop && dbid == 0)) {
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
	if (p_PlayerCorpse)
		snprintf(tmp, sizeof(tmp), "'s corpse%d", GetID());
	else
		snprintf(tmp, sizeof(tmp), "`s_corpse%d", GetID());
	name[(sizeof(name) - 1) - strlen(tmp)] = 0;
	strcat(name, tmp);
}

bool Corpse::Save() {
	if (!p_PlayerCorpse)
		return true;
	if (!pIsChanged)
		return true;

	uint32 tmp = this->CountItems();
	uint32 tmpsize = sizeof(DBPlayerCorpse_Struct) + (tmp * sizeof(player_lootitem::ServerLootItem_Struct));
	DBPlayerCorpse_Struct* dbpc = (DBPlayerCorpse_Struct*) new uchar[tmpsize];
	memset(dbpc, 0, tmpsize);
	dbpc->itemcount = tmp;
	dbpc->size = this->size;
	dbpc->locked = pLocked;
	dbpc->copper = this->copper;
	dbpc->silver = this->silver;
	dbpc->gold = this->gold;
	dbpc->plat = this->platinum;

	// Little hack to account for the fact the race in the corpse struct is a uint8 and Froglok/Drakkin race number > 255
	// and to maintain backwards compatability with existing corpses in the database.
	uint16 CorpseRace;

	switch(race) {
		case DRAKKIN:
			CorpseRace = 254;
			break;
		case FROGLOK:
			CorpseRace = 255;
			break;
		default:
			CorpseRace = race;
	}

	dbpc->race = CorpseRace;
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
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		ServerLootItem_Struct* item = *cur;
		memcpy((char*) &dbpc->items[x++], (char*) item, sizeof(player_lootitem::ServerLootItem_Struct));
	}

	dbpc->crc = CRC32::Generate(&((uchar*) dbpc)[4], tmpsize - 4);

	if (dbid == 0)
	{
		dbid = database.CreatePlayerCorpse(charid, orgname, zone->GetZoneID(), zone->GetInstanceID(), (uchar*) dbpc, tmpsize, x_pos, y_pos, z_pos, heading);
		if(RuleB(Zone, UsePlayerCorpseBackups) == true)
			database.CreatePlayerCorpseBackup(dbid, charid, orgname, zone->GetZoneID(), zone->GetInstanceID(), (uchar*) dbpc, tmpsize, x_pos, y_pos, z_pos, heading);
	}
	else
		dbid = database.UpdatePlayerCorpse(dbid, charid, orgname, zone->GetZoneID(), zone->GetInstanceID(), (uchar*) dbpc, tmpsize, x_pos, y_pos, z_pos, heading,Rezzed());
	safe_delete_array(dbpc);
	if (dbid == 0) {
		std::cout << "Error: Failed to save player corpse '" << this->GetName() << "'" << std::endl;
		return false;
	}
	return true;
}

void Corpse::Delete() {
	if (IsPlayerCorpse() && dbid != 0)
		database.DeletePlayerCorpse(dbid);
	dbid = 0;

	p_depop = true;
}

void Corpse::Bury() {
	if (IsPlayerCorpse() && dbid != 0)
		database.BuryPlayerCorpse(dbid);
	dbid = 0;

	p_depop = true;
}

void Corpse::Depop() {
	if (IsNPCCorpse())
		p_depop = true;
}

void Corpse::DepopCorpse() {
		p_depop = true;
}

uint32 Corpse::CountItems() {
	return itemlist.size();
}

void Corpse::AddItem(uint32 itemnum, uint16 charges, int16 slot, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5) {
	if (!database.GetItem(itemnum))
		return;
	pIsChanged = true;
	ServerLootItem_Struct* item = new ServerLootItem_Struct;
	memset(item, 0, sizeof(ServerLootItem_Struct));
	item->item_id = itemnum;
	item->charges = charges;
	item->equipSlot = slot;
	item->aug1=aug1;
	item->aug2=aug2;
	item->aug3=aug3;
	item->aug4=aug4;
	item->aug5=aug5;
	itemlist.push_back(item);
}

ServerLootItem_Struct* Corpse::GetItem(uint16 lootslot, ServerLootItem_Struct** bag_item_data)
{
	ServerLootItem_Struct *sitem = 0, *sitem2;

	// find the item
	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		if((*cur)->lootslot == lootslot)
		{
			sitem = *cur;
			break;
		}
	}

	if (sitem && bag_item_data && Inventory::SupportsContainers(sitem->equipSlot))
	{
		int16 bagstart = Inventory::CalcSlotId(sitem->equipSlot, 0);

		cur = itemlist.begin();
		end = itemlist.end();
		for(; cur != end; ++cur) {
			sitem2 = *cur;
			if(sitem2->equipSlot >= bagstart && sitem2->equipSlot < bagstart + 10)
			{
				bag_item_data[sitem2->equipSlot - bagstart] = sitem2;
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
		if (item->equipSlot == equipSlot)
		{
			return item->item_id;
		}
	}

	return 0;
}

void Corpse::RemoveItem(uint16 lootslot)
{

	if (lootslot == 0xFFFF)
		return;

	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		ServerLootItem_Struct* sitem = *cur;
		if (sitem->lootslot == lootslot)
		{
			RemoveItem(sitem);
			return;
		}
	}
}

void Corpse::RemoveItem(ServerLootItem_Struct* item_data)
{
	uint8 material;

	ItemList::iterator cur,end;
	cur = itemlist.begin();
	end = itemlist.end();
	for(; cur != end; ++cur) {
		ServerLootItem_Struct* sitem = *cur;
		if (sitem == item_data)
		{
			pIsChanged = true;
			itemlist.erase(cur);

			material = Inventory::CalcMaterialFromSlot(sitem->equipSlot);
			if(material != 0xFF)
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
	pIsChanged = true;
}

void Corpse::RemoveCash() {
	this->copper = 0;
	this->silver = 0;
	this->gold = 0;
	this->platinum = 0;
	pIsChanged = true;
}

bool Corpse::IsEmpty() const {
	if (copper != 0 || silver != 0 || gold != 0 || platinum != 0)
		return false;
	return(itemlist.size() == 0);
}

bool Corpse::Process() {
	if (p_depop)
		return false;

	if(corpse_delay_timer.Check())
	{
		for (int i=0; i<MAX_LOOTERS; i++)
			looters[i] = 0;
		corpse_delay_timer.Disable();
		return true;
	}

	if(corpse_graveyard_timer.Check()) {
		if(zone->HasGraveyard()) {
			Save();
			p_depop = true;
			database.GraveyardPlayerCorpse(dbid, zone->graveyard_zoneid(),
				(zone->GetZoneID() == zone->graveyard_zoneid()) ? zone->GetInstanceID() : 0, zone->graveyard_x(),
				zone->graveyard_y(), zone->graveyard_z(), zone->graveyard_heading());
			corpse_graveyard_timer.Disable();
			ServerPacket* pack = new ServerPacket(ServerOP_SpawnPlayerCorpse, sizeof(SpawnPlayerCorpse_Struct));
			SpawnPlayerCorpse_Struct* spc = (SpawnPlayerCorpse_Struct*)pack->pBuffer;
			spc->player_corpse_id = dbid;
			spc->zone_id = zone->graveyard_zoneid();
			worldserver.SendPacket(pack);
			safe_delete(pack);
			LogFile->write(EQEMuLog::Debug, "Moved %s player corpse to the designated graveyard in zone %s.", this->GetName(), database.GetZoneName(zone->graveyard_zoneid()));
			dbid = 0;
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
			if(database.BuryPlayerCorpse(dbid)) {
				Save();
				p_depop = true;
				dbid = 0;
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
		if(looters[i] != 0)
			z++;

		if (looters[i] == charid)
			return true;
	}
	if(z == 0)
		return true;
	else
		return false;
}

void Corpse::AllowMobLoot(Mob *them, uint8 slot)
{
	if(slot >= MAX_LOOTERS)
		return;
	if(them == nullptr || !them->IsClient())
		return;

	looters[slot] = them->CastToClient()->CharacterID();
}

// @merth: this function needs some work
void Corpse::MakeLootRequestPackets(Client* client, const EQApplicationPacket* app) {
	// Added 12/08. Started compressing loot struct on live.
	char tmp[10];
	if(p_depop) {
		SendLootReqErrorPacket(client, 0);
		return;
	}

	if(IsPlayerCorpse() && dbid == 0) {
		// SendLootReqErrorPacket(client, 0);
		client->Message(13, "Warning: Corpse's dbid = 0! Corpse will not survive zone shutdown!");
		std::cout << "Error: PlayerCorpse::MakeLootRequestPackets: dbid = 0!" << std::endl;
		// return;
	}

	if(pLocked && client->Admin() < 100) {
		SendLootReqErrorPacket(client, 0);
		client->Message(13, "Error: Corpse locked by GM.");
		return;
	}

	if(BeingLootedBy == 0) { BeingLootedBy = 0xFFFFFFFF; }

	if(this->BeingLootedBy != 0xFFFFFFFF) {
		// lets double check....
		Entity* looter = entity_list.GetID(this->BeingLootedBy);
		if(looter == 0) { this->BeingLootedBy = 0xFFFFFFFF; }
	}

	uint8 tCanLoot = 1;
	bool lootcoin = false;
	if(database.GetVariable("LootCoin", tmp, 9)) { lootcoin = (atoi(tmp) == 1); }

	if(this->BeingLootedBy != 0xFFFFFFFF && this->BeingLootedBy != client->GetID()) {
		SendLootReqErrorPacket(client, 0);
		tCanLoot = 0;
	}
	else if(IsPlayerCorpse() && charid == client->CharacterID()) { tCanLoot = 2; }
	else if((IsNPCCorpse() || become_npc) && CanMobLoot(client->CharacterID())) { tCanLoot = 2; }
	else if(GetPKItem() == -1 && CanMobLoot(client->CharacterID())) { tCanLoot = 3; } //pvp loot all items, variable cash
	else if(GetPKItem() == 1 && CanMobLoot(client->CharacterID())) { tCanLoot = 4; } //pvp loot 1 item, variable cash
	else if(GetPKItem() > 1 && CanMobLoot(client->CharacterID())) { tCanLoot = 5; } //pvp loot 1 set item, variable cash

	if(tCanLoot == 1) { if(client->Admin() < 100 || !client->GetGM()) { SendLootReqErrorPacket(client, 2); } }

	if(tCanLoot >= 2 || (tCanLoot == 1 && client->Admin() >= 100 && client->GetGM())) {
		this->BeingLootedBy = client->GetID();
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoneyOnCorpse, sizeof(moneyOnCorpseStruct));
		moneyOnCorpseStruct* d = (moneyOnCorpseStruct*) outapp->pBuffer;

		d->response		= 1;
		d->unknown1		= 0x42;
		d->unknown2		= 0xef;
		if(tCanLoot == 2 || (tCanLoot >= 3 && lootcoin)) { // dont take the coin off if it's a gm peeking at the corpse
			if(zone->lootvar != 0) {
				int admin = client->Admin();
				if(zone->lootvar == 7) { client->LogLoot(client, this, 0); }
				else if((admin >= 10) && (admin < 20)) {
					if((zone->lootvar < 8) && (zone->lootvar > 5)) { client->LogLoot(client, this, 0); }
				}
				else if(admin <= 20) {
					if((zone->lootvar < 8) && (zone->lootvar > 4)) { client->LogLoot(client, this, 0); }
				}
				else if(admin <= 80) {
					if((zone->lootvar < 8) && (zone->lootvar > 3)) { client->LogLoot(client, this, 0); }
				}
				else if(admin <= 100) {
					if((zone->lootvar < 9) && (zone->lootvar > 2)) { client->LogLoot(client, this, 0); }
				}
				else if(admin <= 150) {
					if(((zone->lootvar < 8) && (zone->lootvar > 1)) || (zone->lootvar == 9)) { client->LogLoot(client, this, 0); }
				}
				else if (admin <= 255) {
					if((zone->lootvar < 8) && (zone->lootvar > 0)) { client->LogLoot(client, this, 0); }
				}
			}

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
			client->Save();
		}

		outapp->priority = 6;
		client->QueuePacket(outapp);
		safe_delete(outapp);
		if(tCanLoot == 5) {
			int pkitem = GetPKItem();
			const Item_Struct* item = database.GetItem(pkitem);
			ItemInst* inst = database.CreateItem(item, item->MaxCharges);
			if(inst) {
				client->SendItemPacket(22, inst, ItemPacketLoot);
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
		int corpselootlimit;

		if(client->GetClientVersion() >= EQClientRoF) { corpselootlimit = 34; }
		else if(client->GetClientVersion() >= EQClientSoF) { corpselootlimit = 32; }
		else if(client->GetClientVersion() == EQClientTitanium) { corpselootlimit = 31; }
		else { corpselootlimit = 30; }

		for(; cur != end; ++cur) {
			ServerLootItem_Struct* item_data = *cur;
			item_data->lootslot = 0xFFFF;

			// Dont display the item if it's in a bag

			// Added cursor queue slots to corpse item visibility list. Nothing else should be making it to corpse.
			if(!IsPlayerCorpse() || item_data->equipSlot <= 30 || item_data->equipSlot == 9999 || tCanLoot>=3 ||
				(item_data->equipSlot >= 8000 && item_data->equipSlot <= 8999)) {
				if(i < corpselootlimit) {
					item = database.GetItem(item_data->item_id);
					if(client && item) {
						ItemInst* inst = database.CreateItem(item, item_data->charges, item_data->aug1, item_data->aug2, item_data->aug3, item_data->aug4, item_data->aug5);
						if(inst) {
							client->SendItemPacket(i + 22, inst, ItemPacketLoot); // 22 is the corpse inventory start offset for Ti(EMu)
							safe_delete(inst);
						}

						item_data->lootslot = i;
					}
				}

				i++;
			}
		}

		if(IsPlayerCorpse() && (charid == client->CharacterID() || client->GetGM())) {
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

void Corpse::LootItem(Client* client, const EQApplicationPacket* app)
{
	//this gets sent out no matter what as a sort of 'ack', so send it here.
	client->QueuePacket(app);

	if(!loot_cooldown_timer.Check())
	{
		SendEndLootErrorPacket(client);
		return;
	}

	// To prevent item loss for a player using 'Loot All' who doesn't have inventory space for all their items.
	if(RuleB(Character, CheckCursorEmptyWhenLooting) && !client->GetInv().CursorEmpty())
	{
		client->Message(13, "You may not loot an item while you have an item on your cursor.");
		SendEndLootErrorPacket(client);
		return;
	}

	LootingItem_Struct* lootitem = (LootingItem_Struct*)app->pBuffer;

	if (this->BeingLootedBy != client->GetID()) {
		client->Message(13, "Error: Corpse::LootItem: BeingLootedBy != client");
		SendEndLootErrorPacket(client);
		return;
	}
	if (IsPlayerCorpse() && !CanMobLoot(client->CharacterID()) && !become_npc && (charid != client->CharacterID() && client->Admin() < 150)) {
		client->Message(13, "Error: This is a player corpse and you dont own it.");
		SendEndLootErrorPacket(client);
		return;
	}
	if (pLocked && client->Admin() < 100) {
		SendLootReqErrorPacket(client, 0);
		client->Message(13, "Error: Corpse locked by GM.");
		return;
	}
	if(IsPlayerCorpse() && (charid != client->CharacterID()) && CanMobLoot(client->CharacterID()) && GetPKItem()==0){
		client->Message(13, "Error: You cannot loot any more items from this corpse.");
		SendEndLootErrorPacket(client);
		BeingLootedBy = 0xFFFFFFFF;
		return;
	}
	const Item_Struct* item = 0;
	ItemInst *inst = 0;
	ServerLootItem_Struct* item_data = nullptr, *bag_item_data[10];

	memset(bag_item_data, 0, sizeof(bag_item_data));
	if(GetPKItem()>1)
		item = database.GetItem(GetPKItem());
	else if(GetPKItem()==-1 || GetPKItem()==1)
		item_data = GetItem(lootitem->slot_id - 22); //dont allow them to loot entire bags of items as pvp reward
	else
		item_data = GetItem(lootitem->slot_id - 22, bag_item_data);

	if (GetPKItem()<=1 && item_data != 0)
	{
		item = database.GetItem(item_data->item_id);
	}

	if (item != 0)
	{
		if(item_data)
			inst = database.CreateItem(item, item_data?item_data->charges:0, item_data->aug1, item_data->aug2, item_data->aug3, item_data->aug4, item_data->aug5);
		else
			inst = database.CreateItem(item);
	}

	if (client && inst)
	{

		if (client->CheckLoreConflict(item))
		{
			client->Message_StringID(0,LOOT_LORE_ERROR);
			SendEndLootErrorPacket(client);
			BeingLootedBy = 0;
			delete inst;
			return;
		}

		if(inst->IsAugmented())
		{
			for(int i=0; i<MAX_AUGMENT_SLOTS; i++)
			{
				ItemInst *itm = inst->GetAugment(i);
				if(itm)
				{
					if(client->CheckLoreConflict(itm->GetItem()))
					{
						client->Message_StringID(0,LOOT_LORE_ERROR);
						SendEndLootErrorPacket(client);
						BeingLootedBy = 0;
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
		std::vector<void*> args;
		args.push_back(inst);
		args.push_back(this);
		parse->EventPlayer(EVENT_LOOT, client, buf, 0, &args);
		parse->EventItem(EVENT_LOOT, client, inst, this, buf, 0);

		if ((RuleB(Character, EnableDiscoveredItems)))
		{
			if(client && !client->GetGM() && !client->IsDiscovered(inst->GetItem()->ID))
				client->DiscoverItem(inst->GetItem()->ID);
		}

		if (zone->lootvar != 0)
		{
			int admin=client->Admin();
			if (zone->lootvar==7){
					client->LogLoot(client,this,item);
			}
			else if ((admin>=10) && (admin<20)){
				if ((zone->lootvar<8) && (zone->lootvar>5))
					client->LogLoot(client,this,item);
			}
			else if (admin<=20){
				if ((zone->lootvar<8) && (zone->lootvar>4))
					client->LogLoot(client,this,item);
			}
			else if (admin<=80){
				if ((zone->lootvar<8) && (zone->lootvar>3))
					client->LogLoot(client,this,item);
			}
			else if (admin<=100){
				if ((zone->lootvar<9) && (zone->lootvar>2))
					client->LogLoot(client,this,item);
			}
			else if (admin<=150){
				if (((zone->lootvar<8) && (zone->lootvar>1)) || (zone->lootvar==9))
					client->LogLoot(client,this,item);
			}
			else if (admin<=255){
				if ((zone->lootvar<8) && (zone->lootvar>0))
					client->LogLoot(client,this,item);
			}
		}

		if(zone->adv_data)
		{
			ServerZoneAdventureDataReply_Struct *ad = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
			if(ad->type == Adventure_Collect && !IsPlayerCorpse())
			{
				if(ad->data_id == inst->GetItem()->ID)
				{
					zone->DoAdventureCountIncrease();
				}
			}
		}

		// first add it to the looter - this will do the bag contents too
		if(lootitem->auto_loot)
		{
			if(!client->AutoPutLootInInventory(*inst, true, true, bag_item_data))
				client->PutLootInInventory(SLOT_CURSOR, *inst, bag_item_data);
		}
		else
		{
			client->PutLootInInventory(SLOT_CURSOR, *inst, bag_item_data);
		}
		// Update any tasks that have an activity to loot this item.
		if(RuleB(TaskSystem, EnableTaskSystem))
			client->UpdateTasksForItem(ActivityLoot, item->ID);
		// now remove it from the corpse
		if(item_data)
			RemoveItem(item_data->lootslot);
		// remove bag contents too
		if (item->ItemClass == ItemClassContainer && (GetPKItem()!=-1 || GetPKItem()!=1))
		{
			for (int i=0; i < 10; i++)
			{
				if (bag_item_data[i])
				{
					RemoveItem(bag_item_data[i]);
				}
			}
		}

		if(GetPKItem()!=-1)
			SetPKItem(0);

		//now send messages to all interested parties

		//creates a link for the item
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
	else
	{
		SendEndLootErrorPacket(client);
		safe_delete(inst);
		return;
	}

	if (IsPlayerCorpse())
		client->SendItemLink(inst);
	else
		client->SendItemLink(inst, true);

	safe_delete(inst);
}

void Corpse::EndLoot(Client* client, const EQApplicationPacket* app) {
	EQApplicationPacket* outapp = new EQApplicationPacket;
	outapp->SetOpcode(OP_LootComplete);
	outapp->size = 0;
	client->QueuePacket(outapp);
	safe_delete(outapp);

	//client->Save();		//inventory operations auto-commit
	this->BeingLootedBy = 0xFFFFFFFF;
	if (this->IsEmpty())
		Delete();
	else
		Save();
}

void Corpse::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho)
{
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

	int corpselootlimit;

	if (to->GetClientVersion() >= EQClientSoF) { corpselootlimit = 32; }
	else if (to->GetClientVersion() == EQClientTitanium) { corpselootlimit = 31; }
	else { corpselootlimit = 30; }

	for(; cur != end; ++cur) {
		ServerLootItem_Struct* sitem = *cur;

		if (IsPlayerCorpse()) {
			if (sitem->equipSlot >= 251 && sitem->equipSlot <= 340)
				sitem->lootslot = 0xFFFF;
			else
				x < corpselootlimit ? sitem->lootslot = x : sitem->lootslot = 0xFFFF;

			const Item_Struct* item = database.GetItem(sitem->item_id);

			if (item)
				to->Message((sitem->lootslot == 0xFFFF), "LootSlot: %i (EquipSlot: %i) Item: %s (%d), Count: %i", static_cast<int16>(sitem->lootslot), sitem->equipSlot, item->Name, item->ID, sitem->charges);
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

bool Corpse::Summon(Client* client, bool spell, bool CheckDistance)
{
	uint32 dist2 = 10000; // pow(100, 2);
	if (!spell) {
		if (this->GetCharID() == client->CharacterID()) {
			if (IsLocked() && client->Admin() < 100) {
				client->Message(13, "That corpse is locked by a GM.");
				return false;
			}
			if (!CheckDistance || (DistNoRootNoZ(*client) <= dist2))
			{
				GMMove(client->GetX(), client->GetY(), client->GetZ());
				pIsChanged = true;
			}
			else
			{
				client->Message(0, "Corpse is too far away.");
				return false;
			}
		}
		else
		{
			bool consented = false;
			std::list<std::string>::iterator itr;
			for(itr = client->consent_list.begin(); itr != client->consent_list.end(); ++itr)
			{
				if(strcmp(this->GetOwnerName(), itr->c_str()) == 0)
				{
					if (!CheckDistance || (DistNoRootNoZ(*client) <= dist2))
					{
						GMMove(client->GetX(), client->GetY(), client->GetZ());
						pIsChanged = true;
					}
					else
					{
						client->Message(0, "Corpse is too far away.");
						return false;
					}
					consented = true;
				}
			}
			if(!consented)
			{
				client->Message(0, "You do not have permission to move this corpse.");
				return false;
			}
		}
	}
	else {
		GMMove(client->GetX(), client->GetY(), client->GetZ());
		pIsChanged = true;
	}
	Save();
	return true;
}

void Corpse::CompleteRezz(){
	rezzexp = 0;
	pIsChanged = true;
	this->Save();
}

void Corpse::Spawn() {
	EQApplicationPacket* app = new EQApplicationPacket;
	this->CreateSpawnPacket(app, this);
	entity_list.QueueClients(this, app);
	safe_delete(app);
}

bool ZoneDatabase::DeleteGraveyard(uint32 zone_id, uint32 graveyard_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256];
	uint32 query_length = 0;
	uint32 affected_rows = 0;

	query_length = sprintf(query,"UPDATE zone SET graveyard_id=0 WHERE zoneidnumber=%u AND version=0", zone_id);

	if (!RunQuery(query, query_length, errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		std::cerr << "Error1 in DeleteGraveyard query " << errbuf << std::endl;
		return false;
	}

	if (affected_rows == 0) {
		std::cerr << "Error2 in DeleteGraveyard query: affected_rows = 0" << std::endl;
		return false;
	}

	query_length = sprintf(query,"DELETE FROM graveyard WHERE id=%u", graveyard_id);

	if (!RunQuery(query, query_length, errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		std::cerr << "Error3 in DeleteGraveyard query " << errbuf << std::endl;
		return false;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		std::cerr << "Error4 in DeleteGraveyard query: affected_rows = 0" << std::endl;
		return false;
	}

	return true;
}
uint32 ZoneDatabase::AddGraveyardIDToZone(uint32 zone_id, uint32 graveyard_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256];
	char* end = query;
	uint32 affected_rows = 0;

	end += sprintf(end,"UPDATE zone SET graveyard_id=%u WHERE zoneidnumber=%u AND version=0", graveyard_id, zone_id);

	if (!RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		std::cerr << "Error1 in AddGraveyardIDToZone query " << errbuf << std::endl;
		return 0;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		std::cerr << "Error2 in AddGraveyardIDToZone query: affected_rows = 0" << std::endl;
		return 0;
	}

	return zone_id;
}
uint32 ZoneDatabase::NewGraveyardRecord(uint32 graveyard_zoneid, float graveyard_x, float graveyard_y, float graveyard_z, float graveyard_heading) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256];
	char* end = query;
	uint32 affected_rows = 0;
	uint32 new_graveyard_id = 0;

	end += sprintf(end,"INSERT INTO graveyard SET zone_id=%u, x=%1.1f, y=%1.1f, z=%1.1f, heading=%1.1f", graveyard_zoneid, graveyard_x, graveyard_y, graveyard_z, graveyard_heading);

	if (!RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows, &new_graveyard_id)) {
		safe_delete_array(query);
		std::cerr << "Error1 in NewGraveyardRecord query " << errbuf << std::endl;
		return 0;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		std::cerr << "Error2 in NewGraveyardRecord query: affected_rows = 0" << std::endl;
		return 0;
	}

	if(new_graveyard_id <= 0) {
		std::cerr << "Error3 in NewGraveyardRecord query: new_graveyard_id <= 0" << std::endl;
		return 0;
	}

	return new_graveyard_id;
}
uint32 ZoneDatabase::GraveyardPlayerCorpse(uint32 dbid, uint32 zoneid, uint16 instanceid, float x, float y, float z, float heading) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256];
	char* end = query;
	uint32 affected_rows = 0;

	// We probably don't want a graveyard located in an instance.
	end += sprintf(end,"Update player_corpses SET zoneid=%u, instanceid=0, x=%1.1f, y=%1.1f, z=%1.1f, heading=%1.1f, WasAtGraveyard=1 WHERE id=%d", zoneid, x, y, z, heading, dbid);

	if (!RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		std::cerr << "Error1 in GraveyardPlayerCorpse query " << errbuf << std::endl;
		return 0;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		std::cerr << "Error2 in GraveyardPlayerCorpse query: affected_rows = 0" << std::endl;
		return 0;
	}
	return dbid;
}
uint32 ZoneDatabase::UpdatePlayerCorpse(uint32 dbid, uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, uchar* data, uint32 datasize, float x, float y, float z, float heading, bool rezzed) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256+(datasize*2)];
	char* end = query;
	uint32 affected_rows = 0;

	end += sprintf(end, "Update player_corpses SET data=");
	*end++ = '\'';
	end += DoEscapeString(end, (char*)data, datasize);
	*end++ = '\'';
	end += sprintf(end,", charname='%s', zoneid=%u, instanceid=%u, charid=%d, x=%1.1f, y=%1.1f, z=%1.1f, heading=%1.1f WHERE id=%d", charname, zoneid, instanceid, charid, x, y, z, heading, dbid);

	if (!RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		std::cerr << "Error1 in UpdatePlayerCorpse query " << errbuf << std::endl;
		return 0;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		std::cerr << "Error2 in UpdatePlayerCorpse query: affected_rows = 0" << std::endl;
		return 0;
	}
	if(rezzed){
		if (!RunQuery(query, MakeAnyLenString(&query, "update player_corpses set rezzed = 1 WHERE id=%d",dbid), errbuf)) {
			std::cerr << "Error in UpdatePlayerCorpse/Rezzed query: " << errbuf << std::endl;
		}
		safe_delete_array(query);
	}
	return dbid;
}

void ZoneDatabase::MarkCorpseAsRezzed(uint32 dbid)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	if(!database.RunQuery(query,MakeAnyLenString(&query, "UPDATE player_corpses SET rezzed = 1 WHERE id = %i", dbid), errbuf))
	{
		LogFile->write(EQEMuLog::Error, "MarkCorpseAsRezzed failed: %s, %s", query, errbuf);
	}
	safe_delete_array(query);
}

uint32 ZoneDatabase::CreatePlayerCorpse(uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, uchar* data, uint32 datasize, float x, float y, float z, float heading) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256+(datasize*2)];
	char* end = query;
	//MYSQL_RES *result;
	//MYSQL_ROW row;
	uint32 affected_rows = 0;
	uint32 last_insert_id = 0;

	end += sprintf(end, "Insert into player_corpses SET data=");
	*end++ = '\'';
	end += DoEscapeString(end, (char*)data, datasize);
	*end++ = '\'';
	end += sprintf(end,", charname='%s', zoneid=%u, instanceid=%u, charid=%d, x=%1.1f, y=%1.1f, z=%1.1f, heading=%1.1f, timeofdeath=Now(), IsBurried=0", charname, zoneid, instanceid, charid, x, y, z, heading);

	if (!RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows, &last_insert_id)) {
		safe_delete_array(query);
		std::cerr << "Error1 in CreatePlayerCorpse query " << errbuf << std::endl;
		return 0;
	}
	safe_delete_array(query);

	if (affected_rows == 0) {
		std::cerr << "Error2 in CreatePlayerCorpse query: affected_rows = 0" << std::endl;
		return 0;
	}

	if (last_insert_id == 0) {
		std::cerr << "Error3 in CreatePlayerCorpse query: last_insert_id = 0" << std::endl;
		return 0;
	}

	return last_insert_id;
}

bool ZoneDatabase::CreatePlayerCorpseBackup(uint32 dbid, uint32 charid, const char* charname, uint32 zoneid, uint16 instanceid, uchar* data, uint32 datasize, float x, float y, float z, float heading) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256+(datasize*2)];
	char* end = query;
	uint32 affected_rows = 0;
	uint32 last_insert_id = 0;
	bool result = false;
	DBPlayerCorpse_Struct* dbpcs = (DBPlayerCorpse_Struct*) data;

	if (dbid != 0) {
		if(RuleB(Character, LeaveCorpses) == true && dbpcs->level >= RuleI(Character, DeathItemLossLevel)){
			end += sprintf(end, "Insert into player_corpses_backup SET data=");
			*end++ = '\'';
			end += DoEscapeString(end, (char*)data, datasize);
			*end++ = '\'';
			end += sprintf(end,", charname='%s', zoneid=%u, instanceid=%u, charid=%d, x=%1.1f, y=%1.1f, z=%1.1f, heading=%1.1f, timeofdeath=Now(), IsBurried=0, id=%u", charname, zoneid, instanceid, charid, x, y, z, heading, dbid);

			if (RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows)) {
				if (affected_rows == 1)
					result = true;
				else
					std::cerr << "Error in CreatePlayerCorpseBackup query: affected_rows != 1" << std::endl;
			}
			else
				std::cerr << "Error in CreatePlayerCorpseBackup query " << errbuf << std::endl;
		}
		safe_delete_array(query);
	}
	else {
		std::cerr << "Error in CreatePlayerCorpseBackup: dbid = 0" << std::endl;
	}
	return result;
}

uint32 ZoneDatabase::GetPlayerBurriedCorpseCount(uint32 char_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 CorpseCount = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "select count(*) from player_corpses where charid = '%u' and IsBurried = 1", char_id), errbuf, &result)) {
		row = mysql_fetch_row(result);
		CorpseCount = atoi(row[0]);
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetPlayerBurriedCorpseCount query '" << query << "' " << errbuf << std::endl;
	}

	safe_delete_array(query);

	return CorpseCount;
}

uint32 ZoneDatabase::GetPlayerCorpseCount(uint32 char_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 CorpseCount = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "select count(*) from player_corpses where charid = '%u'", char_id), errbuf, &result)) {
		row = mysql_fetch_row(result);
		CorpseCount = atoi(row[0]);
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetPlayerCorpseCount query '" << query << "' " << errbuf << std::endl;
	}

	safe_delete_array(query);

	return CorpseCount;
}

uint32 ZoneDatabase::GetPlayerCorpseID(uint32 char_id, uint8 corpse) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 id = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "select id from player_corpses where charid = '%u'", char_id), errbuf, &result)) {
		for (int i=0; i<corpse;i++) {
			row = mysql_fetch_row(result);
			id = (uint32)atoi(row[0]);
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetPlayerCorpseID query '" << query << "' " << errbuf << std::endl;
	}

	safe_delete_array(query);

	return id;
}

uint32 ZoneDatabase::GetPlayerCorpseItemAt(uint32 corpse_id, uint16 slotid) {
	Corpse* tmp = LoadPlayerCorpse(corpse_id);
	uint32 itemid = 0;

	if (tmp) {
		itemid = tmp->GetWornItem(slotid);
		tmp->DepopCorpse();
	}
	return itemid;
}

Corpse* ZoneDatabase::SummonBurriedPlayerCorpse(uint32 char_id, uint32 dest_zoneid, uint16 dest_instanceid, float dest_x, float dest_y, float dest_z, float dest_heading) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	Corpse* NewCorpse = 0;
	unsigned long* lengths;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, charname, data, timeofdeath, rezzed FROM player_corpses WHERE charid='%u' AND IsBurried=1 ORDER BY timeofdeath LIMIT 1", char_id), errbuf, &result)) {
		row = mysql_fetch_row(result);
		lengths = mysql_fetch_lengths(result);
		if(row) {
			NewCorpse = Corpse::LoadFromDBData(atoi(row[0]), char_id, row[1], (uchar*) row[2], lengths[2], dest_x, dest_y, dest_z, dest_heading, row[3],atoi(row[4])==1, false);
			if(NewCorpse) {
				entity_list.AddCorpse(NewCorpse);
				NewCorpse->SetDecayTimer(RuleI(Character, CorpseDecayTimeMS));
				NewCorpse->Spawn();
				if(!UnburyPlayerCorpse(NewCorpse->GetDBID(), dest_zoneid, dest_instanceid, dest_x, dest_y, dest_z, dest_heading))
					LogFile->write(EQEMuLog::Error, "Unable to unbury a summoned player corpse for character id %u.", char_id);
			}
			else
				LogFile->write(EQEMuLog::Error, "Unable to construct a player corpse from a burried player corpse for character id %u.", char_id);
		}

		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in SummonBurriedPlayerCorpse query '" << query << "' " << errbuf << std::endl;
	}

	safe_delete_array(query);

	return NewCorpse;
}

bool ZoneDatabase::SummonAllPlayerCorpses(uint32 char_id, uint32 dest_zoneid, uint16 dest_instanceid,
					float dest_x, float dest_y, float dest_z, float dest_heading)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	Corpse* NewCorpse = 0;
	int CorpseCount = 0;
	unsigned long* lengths;

	if(!RunQuery(query, MakeAnyLenString(&query, "UPDATE player_corpses SET zoneid = %i, instanceid = %i, x = %f, y = %f, z = %f, "
							"heading = %f, IsBurried = 0, WasAtGraveyard = 0 WHERE charid = %i",
							dest_zoneid, dest_instanceid, dest_x, dest_y, dest_z, dest_heading, char_id), errbuf))
		LogFile->write(EQEMuLog::Error, "Error moving corpses, Query = %s, Error = %s\n", query, errbuf);

	safe_delete_array(query);

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, charname, data, timeofdeath, rezzed FROM player_corpses WHERE charid='%u'"
							"ORDER BY timeofdeath", char_id), errbuf, &result))
	{
		while((row = mysql_fetch_row(result)))
		{
			lengths = mysql_fetch_lengths(result);
			NewCorpse = Corpse::LoadFromDBData(atoi(row[0]), char_id, row[1], (uchar*) row[2], lengths[2], dest_x, dest_y,
								dest_z, dest_heading, row[3],atoi(row[4])==1, false);
			if(NewCorpse) {
				entity_list.AddCorpse(NewCorpse);
				NewCorpse->SetDecayTimer(RuleI(Character, CorpseDecayTimeMS));
				NewCorpse->Spawn();
				++CorpseCount;
			}
			else
				LogFile->write(EQEMuLog::Error, "Unable to construct a player corpse for character id %u.", char_id);
		}

		mysql_free_result(result);
	}
	else
		LogFile->write(EQEMuLog::Error, "Error in SummonAllPlayerCorpses Query = %s, Error = %s\n", query, errbuf);

	safe_delete_array(query);

	return (CorpseCount > 0);
}

bool ZoneDatabase::UnburyPlayerCorpse(uint32 dbid, uint32 new_zoneid, uint16 new_instanceid, float new_x, float new_y, float new_z, float new_heading) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = new char[256];
	char* end = query;
	uint32 affected_rows = 0;
	bool Result = false;

	end += sprintf(end, "UPDATE player_corpses SET IsBurried=0, zoneid=%u, instanceid=%u, x=%f, y=%f, z=%f, heading=%f, timeofdeath=Now(), WasAtGraveyard=0 WHERE id=%u", new_zoneid, new_instanceid, new_x, new_y, new_z, new_heading, dbid);

	if (RunQuery(query, (uint32) (end - query), errbuf, 0, &affected_rows)) {
		if (affected_rows == 1)
			Result = true;
		else
			std::cerr << "Error2 in UnburyPlayerCorpse query: affected_rows NOT EQUAL to 1, as expected." << std::endl;
	}
	else
		std::cerr << "Error1 in UnburyPlayerCorpse query " << errbuf << std::endl;

	safe_delete_array(query);

	return Result;
}

Corpse* ZoneDatabase::LoadPlayerCorpse(uint32 player_corpse_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	Corpse* NewCorpse = 0;
	unsigned long* lengths;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, charid, charname, x, y, z, heading, data, timeofdeath, rezzed, WasAtGraveyard FROM player_corpses WHERE id='%u'", player_corpse_id), errbuf, &result)) {
		row = mysql_fetch_row(result);
		lengths = mysql_fetch_lengths(result);
		if(row && lengths)
		{
		NewCorpse = Corpse::LoadFromDBData(atoi(row[0]), atoi(row[1]), row[2], (uchar*) row[7], lengths[7], atof(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), row[8],atoi(row[9])==1, atoi(row[10]));
		entity_list.AddCorpse(NewCorpse);
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in LoadPlayerCorpse query '" << query << "' " << errbuf << std::endl;
		std::cerr << "Note that if your missing the 'rezzed' field you can add it with:\nALTER TABLE `player_corpses` ADD `rezzed` TINYINT UNSIGNED DEFAULT \"0\";\n";
	}

	safe_delete_array(query);

	return NewCorpse;
}

bool ZoneDatabase::LoadPlayerCorpses(uint32 iZoneID, uint16 iInstanceID) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 query_length = 0;

	unsigned long* lengths;

	if(!RuleB(Zone, EnableShadowrest))
		query_length = MakeAnyLenString(&query, "SELECT id, charid, charname, x, y, z, heading, data, timeofdeath, rezzed, WasAtGraveyard FROM player_corpses WHERE zoneid='%u' AND instanceid='%u'", iZoneID, iInstanceID);
	else
		query_length = MakeAnyLenString(&query, "SELECT id, charid, charname, x, y, z, heading, data, timeofdeath, rezzed, 0 FROM player_corpses WHERE zoneid='%u' AND instanceid='%u' AND IsBurried=0", iZoneID, iInstanceID);

	if (RunQuery(query, query_length, errbuf, &result)) {
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result))) {
			lengths = mysql_fetch_lengths(result);
			entity_list.AddCorpse(Corpse::LoadFromDBData(atoi(row[0]), atoi(row[1]), row[2], (uchar*) row[7], lengths[7], atof(row[3]), atoi(row[4]), atoi(row[5]), atoi(row[6]), row[8],atoi(row[9])==1, atoi(row[10])));
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in LoadPlayerCorpses query '" << query << "' " << errbuf << std::endl;
		std::cerr << "Note that if your missing the 'rezzed' field you can add it with:\nALTER TABLE `player_corpses` ADD `rezzed` TINYINT UNSIGNED DEFAULT \"0\";\n";
		safe_delete_array(query);
		return false;
	}

	return true;
}

uint32 ZoneDatabase::GetFirstCorpseID(uint32 char_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 CorpseID = 0;

	MakeAnyLenString(&query, "SELECT id FROM player_corpses WHERE charid='%u' AND IsBurried=0 ORDER BY timeofdeath LIMIT 1", char_id);
		if (RunQuery(query, strlen(query), errbuf, &result)) {
			if (mysql_num_rows(result)!= 0){
				row = mysql_fetch_row(result);
				CorpseID = atoi(row[0]);
				mysql_free_result(result);
	}
		}
	else {
		std::cerr << "Error in GetFirstCorpseID query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return 0;
	}

	safe_delete_array(query);
	return CorpseID;
}

bool ZoneDatabase::BuryPlayerCorpse(uint32 dbid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE player_corpses SET IsBurried = 1 WHERE id=%d", dbid), errbuf)) {
		std::cerr << "Error in BuryPlayerCorpse query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;
}

bool ZoneDatabase::BuryAllPlayerCorpses(uint32 charid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "UPDATE player_corpses SET IsBurried = 1 WHERE charid=%d", charid), errbuf)) {
		std::cerr << "Error in BuryPlayerCorpse query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;
}

bool ZoneDatabase::DeletePlayerCorpse(uint32 dbid) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (!RunQuery(query, MakeAnyLenString(&query, "Delete from player_corpses where id=%d", dbid), errbuf)) {
		std::cerr << "Error in DeletePlayerCorpse query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);
	return true;
}

// these functions operate with a material slot, which is from 0 to 8
uint32 Corpse::GetEquipment(uint8 material_slot) const {
	int invslot;

	if(material_slot > 8)
	{
		return 0;
	}

	invslot = Inventory::CalcSlotFromMaterial(material_slot);
	if(invslot == -1)
		return 0;

	return GetWornItem(invslot);
}

uint32 Corpse::GetEquipmentColor(uint8 material_slot) const {
	const Item_Struct *item;

	if(material_slot > 8)
	{
		return 0;
	}

	item = database.GetItem(GetEquipment(material_slot));
	if(item != 0)
	{
		return item_tint[material_slot].rgb.use_tint ?
			item_tint[material_slot].color :
			item->Color;
	}

	return 0;
}

void Corpse::AddLooter(Mob* who)
{
	for (int i=0; i<MAX_LOOTERS; i++)
	{
		if (looters[i] == 0)
		{
			looters[i] = who->CastToClient()->CharacterID();
			break;
		}
	}
}

void Corpse::LoadPlayerCorpseDecayTime(uint32 dbid){
	if(!dbid)
		return;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (database.RunQuery(query, MakeAnyLenString(&query, "SELECT (UNIX_TIMESTAMP() - UNIX_TIMESTAMP(timeofdeath)) FROM player_corpses WHERE id=%d and not timeofdeath=0", dbid), errbuf, &result)) {
		safe_delete_array(query);
		while ((row = mysql_fetch_row(result))) {
			if(atoi(row[0]) > 0 && RuleI(Character, CorpseDecayTimeMS) > (atoi(row[0]) * 1000)) {
				corpse_decay_timer.SetTimer(RuleI(Character, CorpseDecayTimeMS) - (atoi(row[0]) * 1000));
				/*
				if(RuleI(Character, CorpseResTimeMS) > (atoi(row[0]) * 1000)) {
					corpse_res_timer.SetTimer(RuleI(Character, CorpseResTimeMS) - (atoi(row[0]) * 1000));
				}
				else {
					corpse_res_timer.Disable();
					can_rez = false;
				}
				*/
			}
			else {
				corpse_decay_timer.SetTimer(2000);
				//corpse_res_timer.SetTimer(300000);
			}
			if(atoi(row[0]) > 0 && RuleI(Zone, GraveyardTimeMS) > (atoi(row[0]) * 1000)) {
				corpse_graveyard_timer.SetTimer(RuleI(Zone, GraveyardTimeMS) - (atoi(row[0]) * 1000));
			}
			else {
				corpse_graveyard_timer.SetTimer(3000);
			}

		}
		mysql_free_result(result);
	}
	else
		safe_delete_array(query);
}

/*
void Corpse::CastRezz(uint16 spellid, Mob* Caster){
	if(Rezzed()){
		if(Caster && Caster->IsClient())
			Caster->Message(13,"This character has already been resurrected.");
		return;
	}

	APPLAYER* outapp = new APPLAYER(OP_RezzRequest, sizeof(Resurrect_Struct));
	Resurrect_Struct* rezz = (Resurrect_Struct*) outapp->pBuffer;
	memcpy(rezz->your_name,this->orgname,30);
	memcpy(rezz->corpse_name,this->name,30);
	memcpy(rezz->rezzer_name,Caster->GetName(),30);
	memcpy(rezz->zone,zone->GetShortName(),15);
	rezz->spellid = spellid;
	rezz->x = this->x_pos;
	rezz->y = this->y_pos;
	rezz->z = (float)this->z_pos;
	worldserver.RezzPlayer(outapp, rezzexp, OP_RezzRequest);
	//DumpPacket(outapp);
	safe_delete(outapp);
}
*/
