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
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "masterentity.h"
#include "worldserver.h"
#include "StringIDs.h"
#include "zonedb.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/StringUtil.h"
#include "guild_mgr.h"

#define OPEN_DOOR 0x02
#define CLOSE_DOOR 0x03
#define OPEN_INVDOOR 0x03
#define CLOSE_INVDOOR 0x02

extern EntityList entity_list;
extern WorldServer worldserver;

Doors::Doors(const Door* door)
:	close_timer(5000)
{
	db_id = door->db_id;
	door_id = door->door_id;
	strn0cpy(zone_name,door->zone_name,32);
	strn0cpy(door_name,door->door_name,32);
	pos_x = door->pos_x;
	pos_y = door->pos_y;
	pos_z = door->pos_z;
	heading = door->heading;
	incline = door->incline;
	opentype = door->opentype;
	guild_id = door->guild_id;
	lockpick = door->lockpick;
	keyitem = door->keyitem;
	nokeyring = door->nokeyring;
	trigger_door = door->trigger_door;
	trigger_type = door->trigger_type;
	triggered=false;
	door_param = door->door_param;
	size = door->size;
	invert_state = door->invert_state;
	SetOpenState(false);

	close_timer.Disable();

	strn0cpy(dest_zone,door->dest_zone,32);
	dest_instance_id = door->dest_instance_id;
	dest_x = door->dest_x;
	dest_y = door->dest_y;
	dest_z = door->dest_z;
	dest_heading = door->dest_heading;

	is_ldon_door = door->is_ldon_door;
	client_version_mask = door->client_version_mask;
}

Doors::Doors(const char *dmodel, float dx, float dy, float dz, float dheading, uint8 dopentype, uint16 dsize)
:	close_timer(5000)
{
	db_id = database.GetDoorsCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion());
	door_id = database.GetDoorsDBCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion());
	strn0cpy(zone_name,zone->GetShortName(),32);
	strn0cpy(door_name,dmodel,32);
	pos_x = dx;
	pos_y = dy;
	pos_z = dz;
	heading = dheading;
	incline = 0;
	opentype = dopentype;
	guild_id = 0;
	lockpick = 0;
	keyitem = 0;
	nokeyring = 0;
	trigger_door = 0;
	trigger_type = 0;
	triggered=false;
	door_param = 0;
	size = dsize;
	invert_state = 0;
	SetOpenState(false);

	close_timer.Disable();

	strn0cpy(dest_zone,"NONE",32);
	dest_instance_id = 0;
	dest_x = 0;
	dest_y = 0;
	dest_z = 0;
	dest_heading = 0;

	is_ldon_door = 0;
	client_version_mask = 4294967295u;
}


Doors::~Doors()
{
}

bool Doors::Process()
{
	if(close_timer.Enabled() && close_timer.Check() && IsDoorOpen())
	{
		if (opentype == 40 || GetTriggerType() == 1)
		{
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
			MoveDoor_Struct* md = (MoveDoor_Struct*)outapp->pBuffer;
			md->doorid = door_id;
			md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
			entity_list.QueueClients(0, outapp);
			safe_delete(outapp);
		}

		triggered=false;
		close_timer.Disable();
		SetOpenState(false);
	}
	return true;
}

void Doors::HandleClick(Client* sender, uint8 trigger)
{
	//door debugging info dump
	_log(DOORS__INFO, "%s clicked door %s (dbid %d, eqid %d) at (%.4f,%.4f,%.4f @%.4f)", sender->GetName(), door_name, db_id, door_id, pos_x, pos_y, pos_z, heading);
	_log(DOORS__INFO, "  incline %d, opentype %d, lockpick %d, key %d, nokeyring %d, trigger %d type %d, param %d", incline, opentype, lockpick, keyitem, nokeyring, trigger_door, trigger_type, door_param);
	_log(DOORS__INFO, "  size %d, invert %d, dest: %s (%.4f,%.4f,%.4f @%.4f)", size, invert_state, dest_zone, dest_x, dest_y, dest_z, dest_heading);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct* md = (MoveDoor_Struct*)outapp->pBuffer;
	md->doorid = door_id;
	/////////////////////////////////////////////////////////////////
	//used_pawn: Locked doors! Rogue friendly too =)
	//TODO: add check for other lockpick items
	//////////////////////////////////////////////////////////////////

	//TODO: ADVENTURE DOOR
	if(IsLDoNDoor())
	{
		if(sender)
		{
			if(RuleI(Adventure, ItemIDToEnablePorts) != 0)
			{
				if(!sender->KeyRingCheck(RuleI(Adventure, ItemIDToEnablePorts)))
				{
					if(sender->GetInv().HasItem(RuleI(Adventure, ItemIDToEnablePorts)) == SLOT_INVALID)
					{
						sender->Message_StringID(13, 5141);
						safe_delete(outapp);
						return;
					}
					else
					{
						sender->KeyRingAdd(RuleI(Adventure, ItemIDToEnablePorts));
					}
				}
			}

			if(!sender->GetPendingAdventureDoorClick())
			{
				sender->PendingAdventureDoorClick();
				ServerPacket *pack = new ServerPacket(ServerOP_AdventureClickDoor, sizeof(ServerPlayerClickedAdventureDoor_Struct));
				ServerPlayerClickedAdventureDoor_Struct *ads = (ServerPlayerClickedAdventureDoor_Struct*)pack->pBuffer;
				strcpy(ads->player, sender->GetName());
				ads->zone_id = zone->GetZoneID();
				ads->id = GetDoorDBID();
				worldserver.SendPacket(pack);
				safe_delete(pack);
				safe_delete(outapp);
			}
			return;
		}
	}

	uint32 keyneeded = GetKeyItem();
	uint8 keepoffkeyring = GetNoKeyring();
	uint32 haskey = 0;
	uint32 playerkey = 0;
	const ItemInst *lockpicks = sender->GetInv().GetItem(SLOT_CURSOR);

	haskey = sender->GetInv().HasItem(keyneeded, 1);

	if(haskey != SLOT_INVALID)
	{
		playerkey = keyneeded;
	}

	if(GetTriggerType() == 255)
	{ // this object isnt triggered
		if(trigger == 1)
		{ // this door is only triggered by an object
			if(!IsDoorOpen() || (opentype == 58))
			{
				md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
			}
			else
			{
				md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
			}
		}
		else
		{
			safe_delete(outapp);
			return;
		}
	}

	// guild doors
	if(((keyneeded == 0) && (GetLockpick() == 0) && (guild_id == 0)) ||
		(IsDoorOpen() && (opentype == 58)) ||
		((guild_id > 0) && (guild_id == sender->GuildID())))
	{	//door not locked
		if(!IsDoorOpen() || (opentype == 58))
		{
			md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
		}
		else
		{
			md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
		}
	}
	else
	{	// guild doors
		if((guild_id > 0) && !sender->GetGM())
		{
			std::string tmp;
			char tmpmsg[240]; // guild doors msgs
			if(guild_mgr.GetGuildNameByID(guild_id, tmp))
			{
				sprintf(tmpmsg, "Only members of the <%s> guild may enter here", tmp.c_str());
			}
			else
			{
				strcpy(tmpmsg, "Door is locked by an unknown guild");
			}
			sender->Message(4, tmpmsg);
			// safe_delete(outapp);
			// /\ possible missing line..all other 'fail' returns seem to have it
			return;
		}
		// a key is required or the door is locked but can be picked or both
		sender->Message(4, "This is locked...");		// debug spam - should probably go
		if(sender->GetGM())		// GM can always open locks - should probably be changed to require a key
		{
			sender->Message_StringID(4,DOORS_GM);
			if(!IsDoorOpen() || (opentype == 58))
			{
				md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
			}
			else
			{
				md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
			}
		}
		else if(playerkey)
		{	// they have something they are trying to open it with
			if(keyneeded && (keyneeded == playerkey))
			{	// key required and client is using the right key
				if(!keepoffkeyring)
				{
					sender->KeyRingAdd(playerkey);
				}
				sender->Message(4, "You got it open!");
				if(!IsDoorOpen() || (opentype == 58))
				{
					md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
				}
				else
				{
					md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
				}
			}
		}
		else if(lockpicks != nullptr)
		{
			if(sender->GetSkill(PICK_LOCK))
			{
				if(lockpicks->GetItem()->ItemType == ItemTypeLockPick)
				{
					float modskill=sender->GetSkill(PICK_LOCK);
					sender->CheckIncreaseSkill(PICK_LOCK, nullptr, 1);

#if EQDEBUG>=5
					LogFile->write(EQEMuLog::Debug, "Client has lockpicks: skill=%f", modskill);
#endif

					if(GetLockpick() <= modskill)
					{
						if(!IsDoorOpen())
						{
							md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
						}
						else
						{
							md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
						}
						sender->Message_StringID(4, DOORS_SUCCESSFUL_PICK);
					}
					else
					{
						sender->Message_StringID(4, DOORS_INSUFFICIENT_SKILL);
						safe_delete(outapp);
						return;
					}
				}
				else
				{
					sender->Message_StringID(4, DOORS_NO_PICK);
					safe_delete(outapp);
					return;
				}
			}
			else
			{
				sender->Message_StringID(4, DOORS_CANT_PICK);
				safe_delete(outapp);
				return;
			}
		}
		else
		{	// locked door and nothing to open it with
			// search for key on keyring
			if(sender->KeyRingCheck(keyneeded))
			{
				playerkey = keyneeded;
				sender->Message(4, "You got it open!"); // more debug spam
				if(!IsDoorOpen() || (opentype == 58))
				{
					md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
				}
				else
				{
					md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
				}
			}
			else
			{
				sender->Message_StringID(4, DOORS_LOCKED);
				safe_delete(outapp);
				return;
			}
		}
	}

	entity_list.QueueClients(sender, outapp, false);
	if(!IsDoorOpen() || (opentype == 58))
	{
		close_timer.Start();
		SetOpenState(true);
	}
	else
	{
		close_timer.Disable();
		SetOpenState(false);
	}

	//everything past this point assumes we opened the door
	//and met all the reqs for opening
	//everything to do with closed doors has already been taken care of
	//we return because we don't want people using teleports on an unlocked door (exploit!)
	if((md->action == CLOSE_DOOR && invert_state == 0) || (md->action == CLOSE_INVDOOR && invert_state == 1))
	{
		safe_delete(outapp);
		return;
	}

	safe_delete(outapp);

	if((GetTriggerDoorID() != 0) && (GetTriggerType() == 1))
	{
		Doors* triggerdoor = entity_list.FindDoor(GetTriggerDoorID());
		if(triggerdoor && !triggerdoor->triggered)
		{
			triggered=true;
			triggerdoor->HandleClick(sender, 1);
		}
		else
		{
			triggered=false;
		}
	}
	else if((GetTriggerDoorID() != 0) && (GetTriggerType() != 1))
	{
		Doors* triggerdoor = entity_list.FindDoor(GetTriggerDoorID());
		if(triggerdoor && !triggerdoor->triggered)
		{
			triggered=true;
			triggerdoor->HandleClick(sender, 0);
		}
		else
		{
			triggered=false;
		}
	}

	if(((opentype == 57) || (opentype == 58)) && (strncmp(dest_zone, "NONE", strlen("NONE")) != 0))
	{ // Teleport door!
		if (( strncmp(dest_zone,zone_name,strlen(zone_name)) == 0) && (!keyneeded))
		{
			if(!keepoffkeyring)
			{
				sender->KeyRingAdd(playerkey);
			}
			sender->MovePC(zone->GetZoneID(), zone->GetInstanceID(), dest_x, dest_y, dest_z, dest_heading);
		}
		else if (( !IsDoorOpen() || opentype == 58 ) && (keyneeded && ((keyneeded == playerkey) || sender->GetGM())))
		{
			if(!keepoffkeyring)
			{
				sender->KeyRingAdd(playerkey);
			}
			if(database.GetZoneID(dest_zone) == zone->GetZoneID())
			{
				sender->MovePC(zone->GetZoneID(), zone->GetInstanceID(), dest_x, dest_y, dest_z, dest_heading);
			}
			else
			{
				sender->MovePC(database.GetZoneID(dest_zone), dest_instance_id, dest_x, dest_y, dest_z, dest_heading);
			}
		}
		if (( !IsDoorOpen() || opentype == 58 ) && (!keyneeded))
		{
			if(database.GetZoneID(dest_zone) == zone->GetZoneID())
			{
				sender->MovePC(zone->GetZoneID(), zone->GetInstanceID(), dest_x, dest_y, dest_z, dest_heading);
			}
			else
			{
				sender->MovePC(database.GetZoneID(dest_zone), dest_instance_id, dest_x, dest_y, dest_z, dest_heading);
			}
		}
	}
}

void Doors::NPCOpen(NPC* sender, bool alt_mode)
{
	if(sender) {
		if(GetTriggerType() == 255 || GetTriggerDoorID() > 0 || GetLockpick() != 0 || GetKeyItem() != 0 || opentype == 59 || opentype == 58 || !sender->IsNPC()) { // this object isnt triggered or door is locked - NPCs should not open locked doors!
			return;
		}

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
		MoveDoor_Struct* md=(MoveDoor_Struct*)outapp->pBuffer;
		md->doorid = door_id;
		md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
		entity_list.QueueCloseClients(sender,outapp,false,200);
		safe_delete(outapp);

		if(!alt_mode) { // original function
			if(!isopen) {
				close_timer.Start();
				isopen=true;
			}
			else {
				close_timer.Disable();
				isopen=false;
			}
		}
		else { // alternative function
			close_timer.Start();
			isopen=true;
		}
	}
}

void Doors::ForceOpen(Mob *sender, bool alt_mode)
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct* md=(MoveDoor_Struct*)outapp->pBuffer;
	md->doorid = door_id;
	md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
	entity_list.QueueClients(sender,outapp,false);
	safe_delete(outapp);

	if(!alt_mode) { // original function
		if(!isopen) {
			close_timer.Start();
			isopen=true;
		}
		else {
			close_timer.Disable();
			isopen=false;
		}
	}
	else { // alternative function
		close_timer.Start();
		isopen=true;
	}
}

void Doors::ForceClose(Mob *sender, bool alt_mode)
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct* md=(MoveDoor_Struct*)outapp->pBuffer;
	md->doorid = door_id;
	md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR; // change from original (open to close)
	entity_list.QueueClients(sender,outapp,false);
	safe_delete(outapp);

	if(!alt_mode) { // original function
		if(!isopen) {
			close_timer.Start();
			isopen=true;
		}
		else {
			close_timer.Disable();
			isopen=false;
		}
	}
	else { // alternative function
		if(isopen)
			close_timer.Trigger();
	}
}

void Doors::ToggleState(Mob *sender)
{
	if(GetTriggerDoorID() > 0 || GetLockpick() != 0 || GetKeyItem() != 0 || opentype == 58 || opentype == 40) { // borrowed some NPCOpen criteria
		return;
	}

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveDoor, sizeof(MoveDoor_Struct));
	MoveDoor_Struct* md=(MoveDoor_Struct*)outapp->pBuffer;
	md->doorid = door_id;

	if(!isopen) {
		md->action = invert_state == 0 ? OPEN_DOOR : OPEN_INVDOOR;
		isopen=true;
	}
	else
	{
		md->action = invert_state == 0 ? CLOSE_DOOR : CLOSE_INVDOOR;
		isopen=false;
	}

	entity_list.QueueClients(sender,outapp,false);
	safe_delete(outapp);
}

void Doors::DumpDoor(){
	LogFile->write(EQEMuLog::Debug,
		"db_id:%i door_id:%i zone_name:%s door_name:%s pos_x:%f pos_y:%f pos_z:%f heading:%f",
		db_id, door_id, zone_name, door_name, pos_x, pos_y, pos_z, heading);
	LogFile->write(EQEMuLog::Debug,
		"opentype:%i guild_id:%i lockpick:%i keyitem:%i nokeyring:%i trigger_door:%i trigger_type:%i door_param:%i open:%s",
		opentype, guild_id, lockpick, keyitem, nokeyring, trigger_door, trigger_type, door_param, (isopen) ? "open":"closed");
	LogFile->write(EQEMuLog::Debug,
		"dest_zone:%s dest_x:%f dest_y:%f dest_z:%f dest_heading:%f",
		dest_zone, dest_x, dest_y, dest_z, dest_heading);
}

int32 ZoneDatabase::GetDoorsCount(uint32* oMaxID, const char *zone_name, int16 version) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	sprintf(query, "SELECT MAX(id), count(*) FROM doors WHERE zone='%s' AND (version=%u OR version=-1)", zone_name, version);
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row != nullptr && row[1] != 0) {
			int32 ret = atoi(row[1]);
			if (oMaxID) {
				if (row[0])
					*oMaxID = atoi(row[0]);
				else
					*oMaxID = 0;
			}
			mysql_free_result(result);
			return ret;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetDoorsCount query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return -1;
	}

	return -1;
}

int32 ZoneDatabase::GetDoorsCountPlusOne(const char *zone_name, int16 version) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 oMaxID = 0;

	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	sprintf(query, "SELECT MAX(id) FROM doors WHERE zone='%s' AND version=%u", zone_name, version);
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row != nullptr && row[1] != 0) {
				if (row[0])
					oMaxID = atoi(row[0]) + 1;
				else
					oMaxID = 0;
			mysql_free_result(result);
			return oMaxID;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetDoorsCountPlusOne query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return -1;
	}

	return -1;
}

int32 ZoneDatabase::GetDoorsDBCountPlusOne(const char *zone_name, int16 version) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 oMaxID = 0;

	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	sprintf(query, "SELECT MAX(doorid) FROM doors WHERE zone='%s' AND (version=%u OR version=-1)", zone_name, version);
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row != nullptr && row[1] != 0) {
				if (row[0])
					oMaxID = atoi(row[0]) + 1;
				else
					oMaxID = 0;
			mysql_free_result(result);
			return oMaxID;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetDoorsCountPlusOne query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return -1;
	}

	return -1;
}

bool ZoneDatabase::LoadDoors(int32 iDoorCount, Door *into, const char *zone_name, int16 version) {
	LogFile->write(EQEMuLog::Status, "Loading Doors from database...");
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

//	Door tmpDoor;
	MakeAnyLenString(&query, "SELECT id,doorid,zone,name,pos_x,pos_y,pos_z,heading,"
		"opentype,guild,lockpick,keyitem,nokeyring,triggerdoor,triggertype,dest_zone,dest_instance,dest_x,"
		"dest_y,dest_z,dest_heading,door_param,invert_state,incline,size,is_ldon_door,client_version_mask "
		"FROM doors WHERE zone='%s' AND (version=%u OR version=-1) ORDER BY doorid asc", zone_name, version);
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		int32 r;
		for(r = 0; (row = mysql_fetch_row(result)); r++) {
			if(r >= iDoorCount) {
				std::cerr << "Error, Door Count of " << iDoorCount << " exceeded." << std::endl;
				break;
			}
			memset(&into[r], 0, sizeof(Door));
			into[r].db_id = atoi(row[0]);
			into[r].door_id = atoi(row[1]);
			strn0cpy(into[r].zone_name,row[2],32);
			strn0cpy(into[r].door_name,row[3],32);
			into[r].pos_x = (float)atof(row[4]);
			into[r].pos_y = (float)atof(row[5]);
			into[r].pos_z = (float)atof(row[6]);
			into[r].heading = (float)atof(row[7]);
			into[r].opentype = atoi(row[8]);
			into[r].guild_id = atoi(row[9]);
			into[r].lockpick = atoi(row[10]);
			into[r].keyitem = atoi(row[11]);
			into[r].nokeyring = atoi(row[12]);
			into[r].trigger_door = atoi(row[13]);
			into[r].trigger_type = atoi(row[14]);
			strn0cpy(into[r].dest_zone, row[15], 32);
			into[r].dest_instance_id = atoi(row[16]);
			into[r].dest_x = (float) atof(row[17]);
			into[r].dest_y = (float) atof(row[18]);
			into[r].dest_z = (float) atof(row[19]);
			into[r].dest_heading = (float) atof(row[20]);
			into[r].door_param=atoi(row[21]);
			into[r].invert_state=atoi(row[22]);
			into[r].incline=atoi(row[23]);
			into[r].size=atoi(row[24]);
			into[r].is_ldon_door=atoi(row[25]);
			into[r].client_version_mask = (uint32)strtoul(row[26], nullptr, 10);
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error in DBLoadDoors query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}
	return true;
}


void Doors::SetLocation(float x, float y, float z)
{
	entity_list.DespawnAllDoors();
	pos_x = x;
	pos_y = y;
	pos_z = z;
	entity_list.RespawnAllDoors();
}

void Doors::SetX(float in) {
	entity_list.DespawnAllDoors();
	pos_x = in;
	entity_list.RespawnAllDoors();
}
void Doors::SetY(float in) {
	entity_list.DespawnAllDoors();
	pos_y = in;
	entity_list.RespawnAllDoors();
}
void Doors::SetZ(float in) {
	entity_list.DespawnAllDoors();
	pos_z = in;
	entity_list.RespawnAllDoors();
}
void Doors::SetHeading(float in) {
	entity_list.DespawnAllDoors();
	heading = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetIncline(int in) {
	entity_list.DespawnAllDoors();
	incline = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetOpenType(uint8 in) {
	entity_list.DespawnAllDoors();
	opentype = in;
	entity_list.RespawnAllDoors();
}

void Doors::SetDoorName(char* name) {
	entity_list.DespawnAllDoors();
	memset(door_name, 0, sizeof(door_name));
	strncpy(door_name, name, sizeof(door_name));
	entity_list.RespawnAllDoors();
}

void Doors::SetSize(uint16 in) {
	entity_list.DespawnAllDoors();
	size = in;
	entity_list.RespawnAllDoors();
}

void Doors::CreateDatabaseEntry()
{
	if(database.GetDoorsDBCountPlusOne(zone->GetShortName(), zone->GetInstanceVersion()) - 1 >= 255)
	{
		return;
	}
	database.InsertDoor(GetDoorDBID(), GetDoorID(), GetDoorName(), GetX(), GetY(), GetZ(), GetHeading(), GetOpenType(), GetGuildID(), GetLockpick(), GetKeyItem(), GetDoorParam(), GetInvertState(), GetIncline(), GetSize());
}

