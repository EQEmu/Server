/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
using namespace std;

#ifdef _WINDOWS
#include <process.h>
#else
#include <pthread.h>
#include "../common/unix.h"
#endif

#include "net.h"
#include "masterentity.h"
#include "worldserver.h"
#include "PlayerCorpse.h"
#include "../common/guilds.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/spdat.h"
#include "../common/features.h"
#include "StringIDs.h"
#include "parser.h"
#include "../common/dbasync.h"
#include "guild_mgr.h"
#include "raids.h"
#include "QuestParserCollection.h"

#ifdef _WINDOWS
#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern Zone* zone;
extern volatile bool ZoneLoaded;
extern WorldServer worldserver;
extern NetConnection net;
extern uint32 numclients;
extern PetitionList petition_list;
extern DBAsync *dbasync;

extern char  errorname[32];
extern uint16 adverrornum;

Entity::Entity() {
	id = 0;
	pDBAsyncWorkID = 0;
}

Entity::~Entity() {
	dbasync->CancelWork(pDBAsyncWorkID);
}

void Entity::SetID(uint16 set_id) {
	id = set_id;
}

Client* Entity::CastToClient() {
	if(this==0x00){
		cout << "CastToClient error (NULL)" << endl;
		DebugBreak();
		return 0;
	}
#ifdef _EQDEBUG
	if(!IsClient()) {
		cout << "CastToClient error (not client?)" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Client*>(this);
}

NPC* Entity::CastToNPC() {
#ifdef _EQDEBUG
	if(!IsNPC()) {	
		cout << "CastToNPC error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<NPC*>(this);
}

Mob* Entity::CastToMob() {
#ifdef _EQDEBUG
	if(!IsMob()) {	
		cout << "CastToMob error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Mob*>(this);
}

Merc* Entity::CastToMerc() {
#ifdef _EQDEBUG
	if(!IsMerc()) {	
		cout << "CastToMerc error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Merc*>(this);
}


Trap* Entity::CastToTrap()
{
#ifdef DEBUG
	if(!IsTrap())
	{
		//cout << "CastToTrap error" << endl;
		return 0;
	}
#endif
	return static_cast<Trap*>(this);
}

Corpse* Entity::CastToCorpse() {
#ifdef _EQDEBUG
	if(!IsCorpse()) {	
		cout << "CastToCorpse error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Corpse*>(this);
}
Object* Entity::CastToObject() {
#ifdef _EQDEBUG
	if(!IsObject()) {	
		cout << "CastToObject error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Object*>(this);
}

/*Group* Entity::CastToGroup() {
#ifdef _EQDEBUG
	if(!IsGroup()) {	
		cout << "CastToGroup error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Group*>(this);
}*/

Doors* Entity::CastToDoors() {
return static_cast<Doors*>(this);
}

Beacon* Entity::CastToBeacon() {
	return static_cast<Beacon*>(this);
}




const Client* Entity::CastToClient() const {
	if(this==0x00){
		cout << "CastToClient error (NULL)" << endl;
		DebugBreak();
		return 0;
	}
#ifdef _EQDEBUG
	if(!IsClient()) {
		cout << "CastToClient error (not client?)" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Client*>(this);
}

const NPC* Entity::CastToNPC() const {
#ifdef _EQDEBUG
	if(!IsNPC()) {	
		cout << "CastToNPC error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const NPC*>(this);
}

const Mob* Entity::CastToMob() const {
#ifdef _EQDEBUG
	if(!IsMob()) {	
		cout << "CastToMob error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Mob*>(this);
}

const Merc* Entity::CastToMerc() const {
#ifdef _EQDEBUG
	if(!IsMerc()) {	
		cout << "CastToMerc error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Merc*>(this);
}

const Trap* Entity::CastToTrap() const {
#ifdef DEBUG
	if(!IsTrap())
	{
		//cout << "CastToTrap error" << endl;
		return 0;
	}
#endif
	return static_cast<const Trap*>(this);
}

const Corpse* Entity::CastToCorpse() const {
#ifdef _EQDEBUG
	if(!IsCorpse()) {	
		cout << "CastToCorpse error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Corpse*>(this);
}

const Object* Entity::CastToObject() const {
#ifdef _EQDEBUG
	if(!IsObject()) {	
		cout << "CastToObject error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Object*>(this);
}

const Doors* Entity::CastToDoors() const {
return static_cast<const Doors*>(this);
}

const Beacon* Entity::CastToBeacon() const {
	return static_cast<const Beacon*>(this);
}

#ifdef BOTS
Bot* Entity::CastToBot() {
#ifdef _EQDEBUG
	if(!IsBot()) {	
		cout << "CastToBot error" << endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Bot*>(this);
}
#endif

EntityList::EntityList() {
	last_insert_id = 0;
}

EntityList::~EntityList() {
	//must call this before the list is destroyed, or else it will try to
	//delete the NPCs in the list, which it cannot do.
	RemoveAllLocalities();
}

bool EntityList::CanAddHateForMob(Mob *p) {
    LinkedListIterator<NPC*> iterator(npc_list);
    int count = 0;

    iterator.Reset();
    while( iterator.MoreElements())
    {
        NPC *npc=iterator.GetData();
        if (npc->IsOnHatelist(p))
            count++;
        // no need to continue if we already hit the limit
        if (count > 3)
            return false;
        iterator.Advance();
    }

    if (count <= 2)
        return true;
    return false;
}

void EntityList::AddClient(Client* client) {
	client->SetID(GetFreeID());
	client_list.Insert(client);
	mob_list.Insert(client);
	if(!client_list.dont_delete)
		client_list.dont_delete=true;
}


void EntityList::TrapProcess() {
	if(numclients < 1)
		return;
	_ZP(EntityList_TrapProcess);
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	uint32 count=0;
	while(iterator.MoreElements())
	{
		count++;
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
	if(count==0)
		net.trap_timer.Disable();//No traps in list, disable until one is added
}


// Debug function -- checks to see if group_list has any NULL entries.
// Meant to be called after each group-related function, in order
// to track down bugs.
void EntityList::CheckGroupList (const char *fname, const int fline)
{
	list<Group *>::iterator it;

	for (it = group_list.begin(); it != group_list.end(); it++)
	{
		if (*it == NULL)
		{
			LogFile->write(EQEMuLog::Error, "NULL group, %s:%i", fname, fline);
		}
	}
}

void EntityList::GroupProcess() {
	list<Group *>::iterator iterator;
	uint32 count = 0;

	if(numclients < 1)
		return;
	_ZP(EntityList_GroupProcess);

	iterator = group_list.begin();
	while(iterator != group_list.end())
	{
		count++;
		(*iterator)->Process();
		/*
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
		*/
		iterator++;
	}
	if(count == 0)
		net.group_timer.Disable();//No groups in list, disable until one is added

#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}

void EntityList::QueueToGroupsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app)
{

	list<Group *>::iterator iterator = group_list.begin();

	_ZP(EntityList_QueueToGroupsForNPCHealthAA);

	while(iterator != group_list.end())
	{
		(*iterator)->QueueHPPacketsForNPCHealthAA(sender, app);
		iterator++;
	}
}

void EntityList::RaidProcess() {
	list<Raid *>::iterator iterator;
	uint32 count = 0;

	if(numclients < 1)
		return;
	_ZP(EntityList_RaidProcess);

	iterator = raid_list.begin();
	while(iterator != raid_list.end())
	{
		count++;
		(*iterator)->Process();
		iterator++;
	}
	if(count == 0)
		net.raid_timer.Disable();//No groups in list, disable until one is added
}

void EntityList::DoorProcess() {
#ifdef IDLE_WHEN_EMPTY
	if(numclients < 1)
		return;
#endif
	_ZP(EntityList_DoorProcess);
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	uint32 count=0;
	while(iterator.MoreElements())
	{
		count++;
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
	if (count==0)
		net.door_timer.Disable();//No doors in list, disable until one is added
}

void EntityList::ObjectProcess() {
	_ZP(EntityList_ObjectProcess);
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	uint32 count=0;
	while(iterator.MoreElements())
	{
		count++;
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
	if(count==0)
		net.object_timer.Disable();//No objects in list, disable until one is added
}

void EntityList::CorpseProcess() {
	_ZP(EntityList_CorpseProcess);
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	uint32 count=0;
	while(iterator.MoreElements())
	{
		count++;
		if(!iterator.GetData()->Process()){
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
	if(count==0)
		net.corpse_timer.Disable();//No corpses in list, disable until one is added
}

void EntityList::MobProcess() {
#ifdef IDLE_WHEN_EMPTY
	if(numclients < 1)
		return;
#endif
	_ZP(EntityList_MobProcess);
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(!iterator.GetData())
		{
			iterator.Advance();
			continue;
		}
		if(!iterator.GetData()->Process()){
			Mob* mob=iterator.GetData();
			if(mob->IsNPC())
				entity_list.RemoveNPC(mob->CastToNPC()->GetID());
			else if(mob->IsMerc()) {
				entity_list.RemoveMerc(mob->CastToMerc()->GetID());
			}
#ifdef BOTS
			else if(mob->IsBot()) {
				entity_list.RemoveBot(mob->CastToBot()->GetID());
			}
#endif
			else{
#ifdef _WINDOWS
					struct in_addr	in;
					in.s_addr = mob->CastToClient()->GetIP();
					cout << "Dropping client: Process=false, ip=" << inet_ntoa(in) << ", port=" << mob->CastToClient()->GetPort() << endl;
#endif
					zone->StartShutdownTimer();
					Group *g = GetGroupByMob(mob);
					if(g) {
						LogFile->write(EQEMuLog::Error, "About to delete a client still in a group.");
						g->DelMember(mob);
					}
					Raid *r = entity_list.GetRaidByClient(mob->CastToClient());
					if(r) {
						LogFile->write(EQEMuLog::Error, "About to delete a client still in a raid.");
						r->MemberZoned(mob->CastToClient());
					}
					entity_list.RemoveClient(mob->GetID());
			}
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
}

void EntityList::BeaconProcess() {
	_ZP(EntityList_BeaconProcess);
	LinkedListIterator<Beacon *> iterator(beacon_list);
	int count;

	for(iterator.Reset(), count = 0; iterator.MoreElements(); count++)
	{
		if(!iterator.GetData()->Process())
			iterator.RemoveCurrent();
		else
			iterator.Advance();
	}
}


void EntityList::AddGroup(Group* group) {
	if(group == NULL)	//this seems to be happening somehow...
		return;
	
	uint32 gid = worldserver.NextGroupID();
	if(gid == 0) {
		LogFile->write(EQEMuLog::Error, "Unable to get new group ID from world server. group is going to be broken.");
		return;
	}
	
	AddGroup(group, gid);
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}


void EntityList::AddGroup(Group* group, uint32 gid) {
	group->SetID(gid);
	//group_list.Insert(group);
	group_list.push_back(group);
	if(!net.group_timer.Enabled())
		net.group_timer.Start();
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}

void EntityList::AddRaid(Raid* raid) {
	if(raid == NULL)
		return;
	
	uint32 gid = worldserver.NextGroupID();
	if(gid == 0) {
		LogFile->write(EQEMuLog::Error, "Unable to get new group ID from world server. group is going to be broken.");
		return;
	}
	
	AddRaid(raid, gid);
}
void EntityList::AddRaid(Raid* raid, uint32 gid) {
	raid->SetID(gid);
	raid_list.push_back(raid);
	if(!net.raid_timer.Enabled())
		net.raid_timer.Start();
}


void EntityList::AddCorpse(Corpse* corpse, uint32 in_id) {
	if (corpse == 0)
		return;
	
	if (in_id == 0xFFFFFFFF)
		corpse->SetID(GetFreeID());
	else
		corpse->SetID(in_id);
	corpse->CalcCorpseName();
	corpse_list.Insert(corpse);
	if(!net.corpse_timer.Enabled())
		net.corpse_timer.Start();
}

void EntityList::AddNPC(NPC* npc, bool SendSpawnPacket, bool dontqueue) {
	npc->SetID(GetFreeID());
    parse->EventNPC(EVENT_SPAWN, npc, NULL, "", 0);

	uint16 emoteid = npc->GetEmoteID();
	if(emoteid != 0)
		npc->DoNPCEmote(ONSPAWN,emoteid);
	
	if (SendSpawnPacket) {
		if (dontqueue) { // aka, SEND IT NOW BITCH!
			EQApplicationPacket* app = new EQApplicationPacket;
			npc->CreateSpawnPacket(app,npc);
			QueueClients(npc, app);
			safe_delete(app);
		}
		else {
			NewSpawn_Struct* ns = new NewSpawn_Struct;
			memset(ns, 0, sizeof(NewSpawn_Struct));
			npc->FillSpawnStruct(ns, 0);	// Not working on player newspawns, so it's safe to use a ForWho of 0
			AddToSpawnQueue(npc->GetID(), &ns);
			safe_delete(ns);
		}
		if(npc->IsFindable())
			UpdateFindableNPCState(npc, false);
	}
	
	npc_list.Insert(npc);
	if(!npc_list.dont_delete)
		npc_list.dont_delete=true;
	mob_list.Insert(npc);
}

void EntityList::AddMerc(Merc* merc, bool SendSpawnPacket, bool dontqueue) {
	if(merc) {
		merc->SetID(GetFreeID());

		if(SendSpawnPacket) {
			if(dontqueue) {
				// Send immediately
				EQApplicationPacket* outapp = new EQApplicationPacket();
				merc->CreateSpawnPacket(outapp);
				outapp->priority = 6;
				QueueClients(merc, outapp, true);
				safe_delete(outapp);
			}
			else {
				// Queue the packet
				NewSpawn_Struct* ns = new NewSpawn_Struct;
				memset(ns, 0, sizeof(NewSpawn_Struct));
				merc->FillSpawnStruct(ns, merc);
				AddToSpawnQueue(merc->GetID(), &ns);
				safe_delete(ns);
			}

            //parse->EventMERC(EVENT_SPAWN, merc, NULL, "", 0);
		}

		merc_list.Insert(merc);
		mob_list.Insert(merc);
		if(!merc_list.dont_delete)
			merc_list.dont_delete=true;
	}
}

void EntityList::AddObject(Object* obj, bool SendSpawnPacket) {
	obj->SetID(GetFreeID()); 
	if (SendSpawnPacket) {
		EQApplicationPacket app;
		obj->CreateSpawnPacket(&app);
		#if (EQDEBUG >= 6)
			DumpPacket(&app);
		#endif
		QueueClients(0, &app,false);
	}
	object_list.Insert(obj);
	if(!net.object_timer.Enabled())
		net.object_timer.Start();
}

void EntityList::AddDoor(Doors* door) {
	door->SetEntityID(GetFreeID());
	door_list.Insert(door);
	if(!net.door_timer.Enabled())
		net.door_timer.Start();
}

void EntityList::AddTrap(Trap* trap) {
	trap->SetID(GetFreeID());
	trap_list.Insert(trap);
	if(!net.trap_timer.Enabled())
		net.trap_timer.Start();
}

void EntityList::AddBeacon(Beacon *beacon)
{
	beacon->SetID(GetFreeID());
	beacon_list.Insert(beacon);
}

void EntityList::AddToSpawnQueue(uint16 entityid, NewSpawn_Struct** ns) {
	uint32 count;
	if((count=(client_list.Count()))==0)
		return;
	SpawnQueue.Append(*ns);
	NumSpawnsOnQueue++;
	if (tsFirstSpawnOnQueue == 0xFFFFFFFF)
		tsFirstSpawnOnQueue = Timer::GetCurrentTime();
	*ns = 0; // make it so the calling function cant fuck us and delete the data =)
}

void EntityList::CheckSpawnQueue() {
	// Send the stuff if the oldest packet on the queue is older than 50ms -Quagmire
	if (tsFirstSpawnOnQueue != 0xFFFFFFFF && (Timer::GetCurrentTime() - tsFirstSpawnOnQueue) > 50) {
		//if (NumSpawnsOnQueue <= 5) {
			LinkedListIterator<NewSpawn_Struct*> iterator(SpawnQueue);
			EQApplicationPacket* outapp = 0;
			
			iterator.Reset();
			while(iterator.MoreElements()) {
				outapp = new EQApplicationPacket;
				Mob::CreateSpawnPacket(outapp, iterator.GetData());
//				cout << "Sending spawn packet: " << iterator.GetData()->spawn.name << endl;
				QueueClients(0, outapp);
				safe_delete(outapp);
				iterator.RemoveCurrent();
			}
			//sending Spawns like this after zone in causes the client to freeze...
		/*}
		else {
			uint32 spawns_per_pack = MAX_SPAWNS_PER_PACKET;
			if(NumSpawnsOnQueue < spawns_per_pack)
				spawns_per_pack = NumSpawnsOnQueue;

			BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(0, spawns_per_pack);
			LinkedListIterator<NewSpawn_Struct*> iterator(SpawnQueue);
			
			iterator.Reset();
			while(iterator.MoreElements()) {
				bzsp->AddSpawn(iterator.GetData());
				iterator.RemoveCurrent();
			}
			safe_delete(bzsp);
		}*/
		
		tsFirstSpawnOnQueue = 0xFFFFFFFF;
		NumSpawnsOnQueue = 0;
	}
}

Doors* EntityList::FindDoor(uint8 door_id)
{
	if (door_id == 0)
		return 0;

	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		Doors* door=iterator.GetData();
		if (door->GetDoorID() == door_id)
		{
			return door;
		}
		iterator.Advance();
	}
	return 0;
}

Object* EntityList::FindObject(uint32 object_id)
{
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		Object* object=iterator.GetData();
		if (object->GetDBID() == object_id)
		{
			return object;
		}
		iterator.Advance();
	}
	return NULL;
}

Object* EntityList::FindNearbyObject(float x, float y, float z, float radius)
{
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();

	float ox;
	float oy;
	float oz;

	while(iterator.MoreElements())
	{
		Object* object=iterator.GetData();
		
		object->GetLocation(&ox, &oy, &oz);

		ox = (x < ox) ? (ox - x) : (x - ox);
		oy = (y < oy) ? (oy - y) : (y - oy);
		oz = (z < oz) ? (oz - z) : (z - oz);

		if ((ox <= radius) && (oy <= radius) && (oz <= radius))
		{
			return object;
		}
		iterator.Advance();
	}


	return NULL;
}


bool EntityList::MakeDoorSpawnPacket(EQApplicationPacket* app, Client *client)
{
	uint32 mask_test = client->GetClientVersionBit();
	int count = 0;
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if((iterator.GetData()->GetClientVersionMask() & mask_test) && strlen(iterator.GetData()->GetDoorName()) > 3)
		{
			count++;
		}
		iterator.Advance();
	}

	if(count == 0 || count > 500)
	{
		return false;
	}
	uint32 length = count * sizeof(Door_Struct);
	uchar* packet_buffer = new uchar[length];
	memset(packet_buffer, 0, length);
	uchar* ptr = packet_buffer;
	Doors *door;
	Door_Struct nd;

	iterator.Reset();
	while(iterator.MoreElements())
	{
		door = iterator.GetData();
		if(door && (door->GetClientVersionMask() & mask_test) && strlen(door->GetDoorName()) > 3)
		{
			memset(&nd, 0, sizeof(nd));
			memcpy(nd.name, door->GetDoorName(), 32);
			nd.xPos = door->GetX();
			nd.yPos = door->GetY();
			nd.zPos = door->GetZ();
			nd.heading = door->GetHeading();
			nd.incline = door->GetIncline();
			nd.size = door->GetSize();
			nd.doorId = door->GetDoorID();				
			nd.opentype = door->GetOpenType();
			nd.state_at_spawn = door->GetInvertState() ? !door->IsDoorOpen() : door->IsDoorOpen();
			nd.invert_state = door->GetInvertState();
			nd.door_param = door->GetDoorParam();	
			memcpy(ptr, &nd, sizeof(nd));
			ptr+=sizeof(nd);
			*(ptr-1)=0x01;
			*(ptr-3)=0x01;
		}
		iterator.Advance();
	}

	app->SetOpcode(OP_SpawnDoor);
	app->size = length;
	app->pBuffer = packet_buffer;
	return true;	
}
Entity* EntityList::GetEntityMob(uint16 id){
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
Entity* EntityList::GetEntityMerc(uint16 id){
	LinkedListIterator<Merc*> iterator(merc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
Entity* EntityList::GetEntityMob(const char *name)
{
	if (name == 0)
		return 0;

	LinkedListIterator<Mob*> iterator(mob_list);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		if (strcasecmp(iterator.GetData()->GetName(), name) == 0)
		{
			return iterator.GetData();
		}
	}
	return 0;
}
Entity* EntityList::GetEntityDoor(uint16 id){
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
Entity* EntityList::GetEntityCorpse(uint16 id){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
Entity* EntityList::GetEntityCorpse(const char *name)
{
	if (name == 0)
		return 0;

	LinkedListIterator<Corpse*> iterator(corpse_list);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		if (strcasecmp(iterator.GetData()->GetName(), name) == 0)
		{
			return iterator.GetData();
		}
	}
	return 0;
}

Entity* EntityList::GetEntityTrap(uint16 id){
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Entity* EntityList::GetEntityObject(uint16 id){
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
/*
Entity* EntityList::GetEntityGroup(uint16 id){
	LinkedListIterator<Group*> iterator(group_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
*/
Entity* EntityList::GetEntityBeacon(uint16 id) {
	LinkedListIterator<Beacon*> iterator(beacon_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}
Entity* EntityList::GetID(uint16 get_id)
{
	Entity* ent=0;
	if((ent=entity_list.GetEntityMob(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityDoor(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityCorpse(get_id))!=0)
		return ent;
//	else if((ent=entity_list.GetEntityGroup(get_id))!=0)
//		return ent;
	else if((ent=entity_list.GetEntityObject(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityTrap(get_id))!=0)
		return ent;
	else if((ent=entity_list.GetEntityBeacon(get_id))!=0)
		return ent;
	else
		return 0;
}

NPC* EntityList::GetNPCByID(uint16 id) {
	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			if (iterator.GetData()->GetID() == id) {
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

NPC* EntityList::GetNPCByNPCTypeID(uint32 npc_id)
{
	if (npc_id == 0)
		return 0;
	LinkedListIterator<NPC*> iterator(npc_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetNPCTypeID() == npc_id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Merc* EntityList::GetMercByID(uint16 id) {
	LinkedListIterator<Merc*> iterator(merc_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			if (iterator.GetData()->GetID() == id) {
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

Mob* EntityList::GetMob(uint16 get_id)
{
	Entity* ent=0;

	if (get_id == 0)
		return 0;

	if((ent=entity_list.GetEntityMob(get_id))!=0)
		return ent->CastToMob();
	else if((ent=entity_list.GetEntityCorpse(get_id))!=0)
		return ent->CastToMob();

	return 0;
}

Mob* EntityList::GetMob(const char* name)
{
	Entity* ent=0;

	if (name == 0)
		return 0;

	if((ent=entity_list.GetEntityMob(name))!=0)
		return ent->CastToMob();
	else if((ent=entity_list.GetEntityCorpse(name))!=0)
		return ent->CastToMob();

	return 0;
}

Mob* EntityList::GetMobByNpcTypeID(uint32 get_id)
{
	if (get_id == 0)
		return 0;
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetNPCTypeID() == get_id)
		{
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Object* EntityList::GetObjectByDBID(uint32 id)
{
	if (id == 0)
		return 0;

	LinkedListIterator<Object*> iterator(object_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			if (iterator.GetData()->CastToObject()->GetDBID() == id)
			{
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

Object* EntityList::GetObjectByID(uint16 id)
{
	if (id == 0)
		return 0;

	LinkedListIterator<Object*> iterator(object_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			if (iterator.GetData()->CastToObject()->GetID() == id)
			{
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

Doors* EntityList::GetDoorsByID(uint16 id)
{
	if (id == 0)
		return 0;

	LinkedListIterator<Doors*> iterator(door_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			if (iterator.GetData()->CastToDoors()->GetEntityID() == id)
			{
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

Doors* EntityList::GetDoorsByDBID(uint32 id)
{
	if (id == 0)
		return 0;

	LinkedListIterator<Doors*> iterator(door_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			if (iterator.GetData()->CastToDoors()->GetDoorDBID() == id)
			{
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

Doors* EntityList::GetDoorsByDoorID(uint32 id)
{
	if (id == 0)
		return 0;

	LinkedListIterator<Doors*> iterator(door_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			if (iterator.GetData()->CastToDoors()->GetDoorID() == id)
			{
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

uint16 EntityList::GetFreeID()
{
	if(last_insert_id > 1500)
		last_insert_id = 0;
	uint16 getid=last_insert_id;
	while(1)
	{
		getid++;
		if (GetID(getid) == 0)
		{
			last_insert_id = getid;
			return getid;
		}
	}
}

// if no language skill is specified, sent with 100 skill
void EntityList::ChannelMessage(Mob* from, uint8 chan_num, uint8 language, const char* message, ...) {
	ChannelMessage(from, chan_num, language, 100, message);
}

void EntityList::ChannelMessage(Mob* from, uint8 chan_num, uint8 language, uint8 lang_skill, const char* message, ...) {
	LinkedListIterator<Client*> iterator(client_list);
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		eqFilterType filter = FilterNone;
		if(chan_num==3)//shout
			filter=FilterShouts;
		else if(chan_num==4) //auction
			filter=FilterAuctions;
		
		if (chan_num != 8 || client->Dist(*from) < 200) // Only say is limited in range
		{
			if(filter==FilterNone || client->GetFilter(filter)!=FilterHide)
				client->ChannelMessageSend(from->GetName(), 0, chan_num, language, lang_skill, buffer);
		}
		iterator.Advance();
	}
}

void EntityList::ChannelMessageSend(Mob* to, uint8 chan_num, uint8 language, const char* message, ...) {
	LinkedListIterator<Client*> iterator(client_list);
	va_list argptr;
	char buffer[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		if (client->GetID() == to->GetID()) {
			client->ChannelMessageSend(0, 0, chan_num, language, buffer);
			break;
		}
		iterator.Advance();
	}
}

void EntityList::SendZoneSpawns(Client* client)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	
	EQApplicationPacket* app;
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob* ent = iterator.GetData();
		if (!( ent->InZone() ) || (ent->IsClient()))
		{
			if(ent->CastToClient()->GMHideMe(client) || ent->CastToClient()->IsHoveringForRespawn())
			{
				iterator.Advance();
				continue;
			}
		}
		app = new EQApplicationPacket;
		iterator.GetData()->CastToMob()->CreateSpawnPacket(app); // TODO: Use zonespawns opcode instead
		client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
		iterator.Advance();
	}	
}

void EntityList::SendZoneSpawnsBulk(Client* client)
{
	//float rate = client->Connection()->GetDataRate();
	LinkedListIterator<Mob*> iterator(mob_list);
	NewSpawn_Struct ns;
	Mob *spawn;
	uint32 maxspawns=100;

	//rate = rate > 1.0 ? (rate < 10.0 ? rate : 10.0) : 1.0;
	//maxspawns = (uint32)rate * SPAWNS_PER_POINT_DATARATE; // FYI > 10240 entities will cause BulkZoneSpawnPacket to throw exception
	if(maxspawns > mob_list.Count())
		maxspawns = mob_list.Count();
	BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(client, maxspawns);
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		spawn = iterator.GetData();
		if(spawn && spawn->InZone())
		{
			if(spawn->IsClient() && (spawn->CastToClient()->GMHideMe(client) || spawn->CastToClient()->IsHoveringForRespawn()))
				continue;
			memset(&ns, 0, sizeof(NewSpawn_Struct));
			spawn->FillSpawnStruct(&ns, client);
			bzsp->AddSpawn(&ns);
		}
	}
	safe_delete(bzsp);
}

//this is a hack to handle a broken spawn struct
void EntityList::SendZonePVPUpdates(Client *to) {
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client *c = iterator.GetData();
		if(c->GetPVP())
			c->SendAppearancePacket(AT_PVP, c->GetPVP(), true, false, to);
		iterator.Advance();
	}
}

void EntityList::SendZoneCorpses(Client* client)
{
	EQApplicationPacket* app;
	LinkedListIterator<Corpse*> iterator(corpse_list);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		Corpse *ent = iterator.GetData();
		app = new EQApplicationPacket;
		ent->CreateSpawnPacket(app);
		client->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
	}	
}

void EntityList::SendZoneCorpsesBulk(Client* client) {
	//float rate = client->Connection()->GetDataRate();
	LinkedListIterator<Corpse*> iterator(corpse_list);
	NewSpawn_Struct ns;
	Corpse *spawn;
	uint32 maxspawns=100;

	//rate = rate > 1.0 ? (rate < 10.0 ? rate : 10.0) : 1.0;
	//maxspawns = (uint32)rate * SPAWNS_PER_POINT_DATARATE; // FYI > 10240 entities will cause BulkZoneSpawnPacket to throw exception
	BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(client, maxspawns);
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		spawn = iterator.GetData();
		if(spawn && spawn->InZone())
		{
			memset(&ns, 0, sizeof(NewSpawn_Struct));
			spawn->FillSpawnStruct(&ns, client);
			bzsp->AddSpawn(&ns);
		}
	}
	safe_delete(bzsp);
}

void EntityList::SendZoneObjects(Client* client)
{
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		EQApplicationPacket *app = new EQApplicationPacket;
		iterator.GetData()->CreateSpawnPacket(app);
		client->FastQueuePacket(&app);
		iterator.Advance();
	}
}

void EntityList::Save()
{
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.GetData()->Save();
		iterator.Advance();
	}	
}

void EntityList::ReplaceWithTarget(Mob* pOldMob, Mob*pNewTarget)
{
	if(!pNewTarget)
		return;
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsAIControlled()) {
            // replace the old mob with the new one
			if (iterator.GetData()->RemoveFromHateList(pOldMob))
                    iterator.GetData()->AddToHateList(pNewTarget, 1, 0);
		}
		iterator.Advance();
	}
}

void EntityList::RemoveFromTargets(Mob* mob, bool RemoveFromXTargets)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Mob *m = iterator.GetData();
		iterator.Advance();

		if(!m)
			continue;

		m->RemoveFromHateList(mob);

		if(RemoveFromXTargets)
			if(m->IsClient())
				m->CastToClient()->RemoveXTarget(mob, false);
			// FadingMemories calls this function passing the client.
			else if(mob->IsClient())
				mob->CastToClient()->RemoveXTarget(m, false);

	}	
}

void EntityList::RemoveFromXTargets(Mob* mob)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Mob *m = iterator.GetData();
		iterator.Advance();

		if(!m)
			continue;

		if(m->IsClient())
			m->CastToClient()->RemoveXTarget(mob, false);

	}	
}

void EntityList::RemoveFromAutoXTargets(Mob* mob)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Mob *m = iterator.GetData();
		iterator.Advance();

		if(!m)
			continue;

		if(m->IsClient())
			m->CastToClient()->RemoveXTarget(mob, true);

	}	
}

void EntityList::RefreshAutoXTargets(Client *c)
{
	if(!c)
		return;

	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Mob *m = iterator.GetData();
		iterator.Advance();

		if(!m || m->GetHP() <= 0)
			continue;

		if(m->CheckAggro(c) && !c->IsXTarget(m))
		{
			c->AddAutoXTarget(m);
			break;
		}

	}	
}

void EntityList::RefreshClientXTargets(Client *c)
{
	if(!c)
		return;

	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client *c2 = iterator.GetData();
		iterator.Advance();

		if(!c2)
			continue;

		if(c2->IsClientXTarget(c))
			c2->UpdateClientXTarget(c);
	}	
}

void EntityList::QueueClientsByTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender, Mob* SkipThisMob, bool ackreq, bool HoTT,
				      uint32 ClientVersionBits)
{
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();

	while(iterator.MoreElements())
	{
		Client *c = iterator.GetData();

		iterator.Advance();

		Mob *Target = c->GetTarget();

		if(!Target)
			continue;

		Mob *TargetsTarget = NULL;

		if(Target)
			TargetsTarget = Target->GetTarget();

		bool Send = false;

		if(c == SkipThisMob)
			continue;

		if(iSendToSender)
			if(c == sender)
				Send = true;
	
		if(c != sender)
		{
			if(Target == sender)
			{
				Send = true;
			}
			else if(HoTT)
			{
				if(TargetsTarget == sender)
					Send = true;
			}
		}
		
		if(Send && (c->GetClientVersionBit() & ClientVersionBits))
			c->QueuePacket(app, ackreq);
	}	
}

void EntityList::QueueClientsByXTarget(Mob* sender, const EQApplicationPacket* app, bool iSendToSender)
{
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();

	while(iterator.MoreElements())
	{
		Client *c = iterator.GetData();

		iterator.Advance();

		if(!c || ((c == sender) && !iSendToSender))
			continue;	

		if(!c->IsXTarget(sender))
			continue;

		c->QueuePacket(app);
	}	
}

void EntityList::QueueCloseClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, float dist, Mob* SkipThisMob, bool ackreq, eqFilterType filter) {
	if (sender == NULL) {
		QueueClients(sender, app, ignore_sender);
		return;
	}
	if(dist <= 0) {
		dist = 600;
	}
	float dist2 = dist * dist; //pow(dist, 2);
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {

		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender) && (ent != SkipThisMob)) {
			eqFilterMode filter2 = ent->GetFilter(filter);
			if(ent->Connected() && 
				(  filter==FilterNone 
				||  filter2 == FilterShow 
				|| (filter2 == FilterShowGroupOnly && (sender == ent || 
					(ent->GetGroup() && ent->GetGroup()->IsGroupMember(sender))))
				|| (filter2 == FilterShowSelfOnly && ent==sender))
			&& (ent->DistNoRoot(*sender) <= dist2)) {
				ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
			}
		}
		iterator.Advance();
	}
}

//sender can be null
void EntityList::QueueClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender))
		{
			ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
		}
		iterator.Advance();
	}
}

/*
rewrite of all the queue close methods to use the update manager
void EntityList::FilterQueueCloseClients(uint8 filter, uint8 required, Mob* sender, const EQApplicationPacket* app, bool ignore_sender, float dist, Mob* SkipThisMob, bool ackreq){
	if(dist <= 0) {
		dist = 600;
	}

#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#else
	float dist2 = dist * dist; //pow(dist, 2);
#endif
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {

		Client* ent = iterator.GetData();
		uint8 filterval=ent->GetFilter(filter);
		if(required==0)
			required=1;
		if(filterval==required){
			if ((!ignore_sender || ent != sender) && (ent != SkipThisMob)
		  		) {
#ifdef PACKET_UPDATE_MANAGER
				if(ent->Connected()) {
					ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
				}
#else
				if(ent->Connected() &&  (ent->DistNoRoot(*sender) <= dist2 || dist == 0)) {
						ent->QueuePacket(app, ackreq);
				}
#endif
			}
		}
		iterator.Advance();
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}

void EntityList::QueueCloseClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, float dist, Mob* SkipThisMob, bool ackreq,uint8 filter) {
	if (sender == 0) {
		QueueClients(sender, app, ignore_sender);
		return;
	}
	if(dist <= 0) {
		dist = 600;
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#else
	float dist2 = dist * dist; //pow(dist, 2);
#endif

	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {

		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender) && (ent != SkipThisMob)) {
			uint8 filter2=ent->GetFilter(filter);
			if(ent->Connected() && 
				(filter==0 || (filter2==1 || 
				(filter2==99 && entity_list.GetGroupByClient(ent)!=0 && 
				 entity_list.GetGroupByClient(ent)->IsGroupMember(sender))
				 || (filter2==98 && ent==sender)))
#ifdef PACKET_UPDATE_MANAGER
			) {
				ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
			}
#else
			&& (ent->DistNoRoot(*sender) <= dist2 || dist == 0)) {
				ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
			}
#endif
		}
		iterator.Advance();
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}

void EntityList::QueueClients(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
	LinkedListIterator<Client*> iterator(client_list);
	
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#endif
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender))
		{
#ifdef PACKET_UPDATE_MANAGER
			ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
#else
			ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
#endif
		}
		iterator.Advance();
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}
*/

/*
void EntityList::QueueManaged(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender))
		{
			ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
		}
		iterator.Advance();
	}
}*/

void EntityList::QueueManaged(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, bool ackreq) {
	LinkedListIterator<Client*> iterator(client_list);
	
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket* tmp_app = app->Copy();
#endif
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* ent = iterator.GetData();

		if ((!ignore_sender || ent != sender))
		{
#ifdef PACKET_UPDATE_MANAGER
			ent->GetUpdateManager()->QueuePacket(tmp_app, ackreq, sender, ent->DistNoRoot(*sender));
#else
			ent->QueuePacket(app, ackreq, Client::CLIENT_CONNECTED);
#endif
		}
		iterator.Advance();
	}
#ifdef PACKET_UPDATE_MANAGER
	EQApplicationPacket::PacketUsed(&tmp_app);
#endif
}


void EntityList::QueueClientsStatus(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, uint8 minstatus, uint8 maxstatus)
{
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if ((!ignore_sender || iterator.GetData() != sender) && (iterator.GetData()->Admin() >= minstatus && iterator.GetData()->Admin() <= maxstatus))
		{
			iterator.GetData()->QueuePacket(app);
		}
		iterator.Advance();
	}	
}

void EntityList::DuelMessage(Mob* winner, Mob* loser, bool flee) {
	LinkedListIterator<Client*> iterator(client_list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		Client *cur = iterator.GetData();
		//might want some sort of distance check in here?
		if (cur != winner && cur != loser)
		{
			if (flee)
				cur->Message_StringID(15, DUEL_FLED, winner->GetName(),loser->GetName(),loser->GetName());
			else
				cur->Message_StringID(15, DUEL_FINISHED, winner->GetName(),loser->GetName());
		}
		iterator.Advance();
	}
}

Client* EntityList::GetClientByName(const char *checkname) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (strcasecmp(iterator.GetData()->GetName(), checkname) == 0) {
			return iterator.GetData();
		}
		iterator.Advance(); 
	} 
	return 0; 
}

Client* EntityList::GetClientByCharID(uint32 iCharID) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) { 
		if (iterator.GetData()->CharacterID() == iCharID) {

			return iterator.GetData();
		}
		iterator.Advance(); 
	} 
	return 0; 
}

Client* EntityList::GetClientByWID(uint32 iWID) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) {  
		if (iterator.GetData()->GetWID() == iWID) {
			return iterator.GetData();
		} 
		iterator.Advance(); 
	} 
	return 0; 
}

Client* EntityList::GetRandomClient(float x, float y, float z, float Distance, Client* ExcludeClient)
{
	vector<Client*> ClientsInRange;

	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements())
	{
		if((iterator.GetData() != ExcludeClient) && (iterator.GetData()->DistNoRoot(x, y, z) <= Distance))
		{
			ClientsInRange.push_back(iterator.GetData());
		}

		iterator.Advance();
	}

	if(ClientsInRange.size() == 0)
		return NULL;

	return ClientsInRange[MakeRandomInt(0, ClientsInRange.size() - 1)];
}

Corpse*	EntityList::GetCorpseByOwner(Client* client){
	LinkedListIterator<Corpse*> iterator(corpse_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (iterator.GetData()->IsPlayerCorpse()) 
		{ 
			if (strcasecmp(iterator.GetData()->GetOwnerName(), client->GetName()) == 0) {
				return iterator.GetData();
			}
		} 
		iterator.Advance(); 
	} 
	return 0; 
}

Corpse*	EntityList::GetCorpseByOwnerWithinRange(Client* client, Mob* center, int range){
	LinkedListIterator<Corpse*> iterator(corpse_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->IsPlayerCorpse())
		{
			if (center->DistNoRootNoZ(*iterator.GetData()) < range && strcasecmp(iterator.GetData()->GetOwnerName(), client->GetName()) == 0) {
				return iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return 0;
}

Corpse* EntityList::GetCorpseByID(uint16 id){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->id == id) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Corpse* EntityList::GetCorpseByDBID(uint32 dbid){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetDBID() == dbid) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

Corpse* EntityList::GetCorpseByName(const char* name){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (strcmp(iterator.GetData()->GetName(),name)==0) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

void EntityList::RemoveAllCorpsesByCharID(uint32 charid)
{
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetCharID() == charid)
			iterator.RemoveCurrent();
		else
			iterator.Advance();
	}
}

void EntityList::RemoveCorpseByDBID(uint32 dbid)
{
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetDBID() == dbid)
			iterator.RemoveCurrent();
		else
			iterator.Advance();
	}
}

int EntityList::RezzAllCorpsesByCharID(uint32 charid)
{
	int RezzExp = 0;

	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetCharID() == charid)
		{
			RezzExp += iterator.GetData()->GetRezzExp();
			iterator.GetData()->Rezzed(true);
			iterator.GetData()->CompleteRezz();
		}
		iterator.Advance();
	}
	return RezzExp;
}

Group* EntityList::GetGroupByMob(Mob* mob) 
{ 
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if ((*iterator)->IsGroupMember(mob)) {
			return *iterator;
		}
		iterator++;
	} 
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0; 
}

Group* EntityList::GetGroupByLeaderName(char* leader){
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if (!strcmp((*iterator)->GetLeaderName(), leader)) {
			return *iterator;
		}
		iterator++; 
	}
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0;
}
Group* EntityList::GetGroupByID(uint32 group_id){
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if ((*iterator)->GetID() == group_id) {
			return *iterator;
		}
		iterator++;
	}
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0;
}
Group* EntityList::GetGroupByClient(Client* client) 
{ 
	list <Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{ 
		if ((*iterator)->IsGroupMember(client->CastToMob())) {
			return *iterator;
		}
		iterator++; 
	} 
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return 0; 
} 

Raid* EntityList::GetRaidByLeaderName(const char *leader){
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		if((*iterator)->GetLeader()){
			if(strcmp((*iterator)->GetLeader()->GetName(), leader) == 0){
				return *iterator;
	} 
}
		iterator++;
	} 
	return 0;
}
Raid* EntityList::GetRaidByID(uint32 id){
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		if ((*iterator)->GetID() == id) {
			return *iterator;
		}
		iterator++;
	} 
	return 0;
}

Raid* EntityList::GetRaidByClient(Client* client) 
{ 
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		for(int x = 0; x < MAX_RAID_MEMBERS; x++)
		{
			if((*iterator)->members[x].member){
				if((*iterator)->members[x].member == client)
					return *iterator;
			}
		}
		iterator++;
	} 
	return 0; 
}

Raid* EntityList::GetRaidByMob(Mob* mob) { 
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{ 
		for(int x = 0; x < MAX_RAID_MEMBERS; x++)
		{
			// TODO: Implement support for Mob objects in Raid class
			/*if((*iterator)->members[x].member){
				if((*iterator)->members[x].member == mob)
					return *iterator;
			}*/
		}
		iterator++;
	} 
	return 0; 
} 

Client* EntityList::GetClientByAccID(uint32 accid) 
{ 
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (iterator.GetData()->AccountID() == accid) {
			return iterator.GetData();
		}
		iterator.Advance(); 
	} 
	return 0; 
} 
Client* EntityList::GetClientByID(uint16 id) { 
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) 
	{ 
		if (iterator.GetData()) 
		{ 
			if (iterator.GetData()->GetID() == id) {
				return iterator.GetData();
			}
		} 
		iterator.Advance(); 
	} 
	return 0; 
} 

void EntityList::ChannelMessageFromWorld(const char* from, const char* to, uint8 chan_num, uint32 guild_id, uint8 language, const char* message) {
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	for(; iterator.MoreElements(); iterator.Advance())
	{
		Client* client = iterator.GetData();
		if(chan_num == 0) {
			if(!client->IsInGuild(guild_id))
				continue;
			if(!guild_mgr.CheckPermission(guild_id, client->GuildRank(), GUILD_HEAR))
				continue;
			if(client->GetFilter(FilterGuildChat) == FilterHide)
				continue;
		} else if(chan_num == 5) {
			if(client->GetFilter(FilterOOC) == FilterHide)
				continue;
		}
		client->ChannelMessageSend(from, to, chan_num, language, message);
	}
}

void EntityList::Message(uint32 to_guilddbid, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		if (to_guilddbid == 0 || client->IsInGuild(to_guilddbid))
			client->Message(type, buffer);
		iterator.Advance();
	}
}

void EntityList::QueueClientsGuild(Mob* sender, const EQApplicationPacket* app, bool ignore_sender, uint32 guild_id){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData()->CastToClient();
		if (client->IsInGuild(guild_id))
			client->QueuePacket(app);
		iterator.Advance();
	}
}

void EntityList::QueueClientsGuildBankItemUpdate(const GuildBankItemUpdate_Struct *gbius, uint32 GuildID)
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

	GuildBankItemUpdate_Struct *outgbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

	memcpy(outgbius, gbius, sizeof(GuildBankItemUpdate_Struct));

	const Item_Struct *Item = database.GetItem(gbius->ItemID);

	LinkedListIterator<Client*> iterator(client_list);

	iterator.Reset();

	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData()->CastToClient();

		if (client->IsInGuild(GuildID))
		{
			if(Item && (gbius->Permissions == GuildBankPublicIfUsable))
				outgbius->Useable = Item->IsEquipable(client->GetBaseRace(), client->GetBaseClass());

			client->QueuePacket(outapp);
		}

		iterator.Advance();
	}
	safe_delete(outapp);
}

void EntityList::MessageStatus(uint32 to_guild_id, int to_minstatus, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		Client* client = iterator.GetData();
		if ((to_guild_id == 0 || client->IsInGuild(to_guild_id)) && client->Admin() >= to_minstatus)
			client->Message(type, buffer);
		iterator.Advance();
	}
}

// works much like MessageClose, but with formatted strings
void EntityList::MessageClose_StringID(Mob *sender, bool skipsender, float dist, uint32 type, uint32 string_id, const char* message1,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9)
{
	Client *c;
	LinkedListIterator<Client*> iterator(client_list);
	float dist2 = dist * dist;
	
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		c = iterator.GetData();
		if(c && c->DistNoRoot(*sender) <= dist2 && (!skipsender || c != sender))
			c->Message_StringID(type, string_id, message1, message2, message3, message4, message5, message6, message7, message8, message9);
	}
}

void EntityList::Message_StringID(Mob *sender, bool skipsender, uint32 type, uint32 string_id, const char* message1,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9)
{
	Client *c;
	LinkedListIterator<Client*> iterator(client_list);
	
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
	{
		c = iterator.GetData();
		if(c && (!skipsender || c != sender))
			c->Message_StringID(type, string_id, message1, message2, message3, message4, message5, message6, message7, message8, message9);
	}
}

void EntityList::MessageClose(Mob* sender, bool skipsender, float dist, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];
	
	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);
	
	float dist2 = dist * dist;
	
	LinkedListIterator<Client*> iterator(client_list);
	
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->DistNoRoot(*sender) <= dist2 && (!skipsender || iterator.GetData() != sender)) {
			iterator.GetData()->Message(type, buffer);
		}
		iterator.Advance();
	}
}

void EntityList::RemoveAllMobs(){
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}
void EntityList::RemoveAllClients(){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent(false);
}
void EntityList::RemoveAllNPCs(){
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		iterator.RemoveCurrent(false);
	}
	npc_limit_list.clear();
}
void EntityList::RemoveAllMercs(){
	LinkedListIterator<Merc*> iterator(merc_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		iterator.RemoveCurrent(false);
	}
}
void EntityList::RemoveAllGroups(){
	while (group_list.size())
		group_list.pop_front();
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
}

void EntityList::RemoveAllRaids(){
	while (raid_list.size())
		raid_list.pop_front();
}

void EntityList::RemoveAllDoors(){
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
	DespawnAllDoors();
}

void EntityList::DespawnAllDoors(){
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RemoveAllDoors, 0);
	this->QueueClients(0,outapp);
	safe_delete(outapp);
}

void EntityList::RespawnAllDoors(){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData() != 0)
		{
		EQApplicationPacket* outapp = new EQApplicationPacket();
		MakeDoorSpawnPacket(outapp, iterator.GetData());
		iterator.GetData()->FastQueuePacket(&outapp);
		}
		iterator.Advance();
	}
}

void EntityList::RemoveAllCorpses(){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}
void EntityList::RemoveAllObjects(){
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}
void EntityList::RemoveAllTraps(){
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent();
}
bool EntityList::RemoveMob(uint16 delete_id){
	if(delete_id==0)
		return true;
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			if(iterator.GetData()->IsNPC())
				entity_list.RemoveNPC(delete_id);
			else if(iterator.GetData()->IsClient())
				entity_list.RemoveClient(delete_id);
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveMob(Mob *delete_mob) {
	if(delete_mob==0)
		return true;
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()==delete_mob){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveNPC(uint16 delete_id){
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			//make sure its proximity is removed
			RemoveProximity(iterator.GetData()->GetID());
			//take it out of the list
			iterator.RemoveCurrent(false);//Already Deleted
			//take it out of our limit list
			if(npc_limit_list.count(delete_id) == 1)
				npc_limit_list.erase(delete_id);
			return true;
		}
		iterator.Advance();
	}
	return false;
}
bool EntityList::RemoveMerc(uint16 delete_id){
	LinkedListIterator<Merc*> iterator(merc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent(false);//Already Deleted
			return true;
		}
		iterator.Advance();
	}
	return false;
}
bool EntityList::RemoveClient(uint16 delete_id){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent(false);//Already Deleted
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveClient(Client *delete_client){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()==delete_client){
			iterator.RemoveCurrent(false);//Already Deleted
			return true;
		}
		iterator.Advance();
	}
	return false;
}

bool EntityList::RemoveObject(uint16 delete_id){
	LinkedListIterator<Object*> iterator(object_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}
bool EntityList::RemoveTrap(uint16 delete_id){
	LinkedListIterator<Trap*> iterator(trap_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}
bool EntityList::RemoveDoor(uint16 delete_id){
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}
bool EntityList::RemoveCorpse(uint16 delete_id){
	LinkedListIterator<Corpse*> iterator(corpse_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetID()==delete_id){
			iterator.RemoveCurrent();
			return true;
		}
		iterator.Advance();
	}
	return false;
}
bool EntityList::RemoveGroup(uint32 delete_id){
	list<Group *>::iterator iterator;

	iterator = group_list.begin();

	while(iterator != group_list.end())
	{
		if((*iterator)->GetID() == delete_id) {
			group_list.remove (*iterator);
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
			return true;
		}
		iterator++;
	}
#if EQDEBUG >= 5
	CheckGroupList (__FILE__, __LINE__);
#endif
	return false;
}

bool EntityList::RemoveRaid(uint32 delete_id){
	list<Raid *>::iterator iterator;

	iterator = raid_list.begin();

	while(iterator != raid_list.end())
	{
		if((*iterator)->GetID() == delete_id) {
			raid_list.remove (*iterator);
			return true;
		}
		iterator++;
	}
	return false;
}

void EntityList::Clear()
{
	RemoveAllClients();
	entity_list.RemoveAllTraps(); //we can have child npcs so we go first
	entity_list.RemoveAllNPCs();
	entity_list.RemoveAllMobs();
	entity_list.RemoveAllCorpses();
	entity_list.RemoveAllGroups();
	entity_list.RemoveAllDoors();
	entity_list.RemoveAllObjects();
	entity_list.RemoveAllRaids();
	entity_list.RemoveAllLocalities();
	last_insert_id = 0;
}

void EntityList::UpdateWho(bool iSendFullUpdate) {
	if ((!worldserver.Connected()) || !ZoneLoaded)
		return;
	LinkedListIterator<Client*> iterator(client_list);
	uint32 tmpNumUpdates = numclients + 5;
	ServerPacket* pack = 0;
	ServerClientListKeepAlive_Struct* sclka = 0;
	if (!iSendFullUpdate) {
		pack = new ServerPacket(ServerOP_ClientListKA, sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4));
		sclka = (ServerClientListKeepAlive_Struct*) pack->pBuffer;
	}
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->InZone()) {
			if (iSendFullUpdate) {
				iterator.GetData()->UpdateWho();
			}
			else {

				if (sclka->numupdates >= tmpNumUpdates) {
					tmpNumUpdates += 10;
					uint8* tmp = pack->pBuffer;
					pack->pBuffer = new uint8[sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4)];
					memset(pack->pBuffer, 0, sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4));
					memcpy(pack->pBuffer, tmp, pack->size);
					pack->size = sizeof(ServerClientListKeepAlive_Struct) + (tmpNumUpdates * 4);
					safe_delete_array(tmp);
				}
				sclka->wid[sclka->numupdates] = iterator.GetData()->GetWID();
				sclka->numupdates++;
			}
		}
		iterator.Advance();
	}
	if (!iSendFullUpdate) {
		pack->size = sizeof(ServerClientListKeepAlive_Struct) + (sclka->numupdates * 4);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void EntityList::RemoveEntity(uint16 id)
{
	if (id == 0)
		return;
	if(entity_list.RemoveMob(id))
		return;
	else if(entity_list.RemoveCorpse(id))
		return;
	else if(entity_list.RemoveDoor(id))
		return;
	else if(entity_list.RemoveGroup(id))
		return;
	else if(entity_list.RemoveTrap(id))
		return;
	else if(entity_list.RemoveMerc(id))
		return;

#ifdef BOTS
	// This block of code is necessary to clean up bot objects
	else if(entity_list.RemoveBot(id))
		return;
#endif //BOTS

	else 
		entity_list.RemoveObject(id);
}

void EntityList::Process()
{
	_ZP(EntityList_Process);
	CheckSpawnQueue();
}

void EntityList::CountNPC(uint32* NPCCount, uint32* NPCLootCount, uint32* gmspawntype_count) {
	LinkedListIterator<NPC*> iterator(npc_list);
	*NPCCount = 0;
	*NPCLootCount = 0;
	
	iterator.Reset();
	while(iterator.MoreElements())	
	{
		(*NPCCount)++;
		(*NPCLootCount) += iterator.GetData()->CastToNPC()->CountLoot();
		if (iterator.GetData()->CastToNPC()->GetNPCTypeID() == 0)
			(*gmspawntype_count)++;
		iterator.Advance();
	}
}

void EntityList::DoZoneDump(ZSDump_Spawn2* spawn2_dump, ZSDump_NPC* npc_dump, ZSDump_NPC_Loot* npcloot_dump, NPCType* gmspawntype_dump) {
	uint32 spawn2index = 0;
	uint32 NPCindex = 0;
	uint32 NPCLootindex = 0;
	uint32 gmspawntype_index = 0;
	
	if (npc_dump != 0) {
		LinkedListIterator<NPC*> iterator(npc_list);
		NPC* npc = 0;
		iterator.Reset();
		while(iterator.MoreElements())	
		{
			npc = iterator.GetData()->CastToNPC();
			if (spawn2_dump != 0)
				npc_dump[NPCindex].spawn2_dump_index = zone->DumpSpawn2(spawn2_dump, &spawn2index, npc->respawn2);
			npc_dump[NPCindex].npctype_id = npc->GetNPCTypeID();
			npc_dump[NPCindex].cur_hp = npc->GetHP();
			if (npc->IsCorpse()) {
				if (npc->CastToCorpse()->IsLocked())
					npc_dump[NPCindex].corpse = 2;
				else
					npc_dump[NPCindex].corpse = 1;
			}
			else
				npc_dump[NPCindex].corpse = 0;
			npc_dump[NPCindex].decay_time_left = 0xFFFFFFFF;
			npc_dump[NPCindex].x = npc->GetX();
			npc_dump[NPCindex].y = npc->GetY();
			npc_dump[NPCindex].z = npc->GetZ();
			npc_dump[NPCindex].heading = npc->GetHeading();
			npc_dump[NPCindex].copper = npc->copper;
			npc_dump[NPCindex].silver = npc->silver;
			npc_dump[NPCindex].gold = npc->gold;
			npc_dump[NPCindex].platinum = npc->platinum;
			if (npcloot_dump != 0)
				npc->DumpLoot(NPCindex, npcloot_dump, &NPCLootindex);
			if (gmspawntype_dump != 0) {
				if (npc->GetNPCTypeID() == 0) {
					memcpy(&gmspawntype_dump[gmspawntype_index], npc->NPCTypedata, sizeof(NPCType));
					npc_dump[NPCindex].gmspawntype_index = gmspawntype_index;
					gmspawntype_index++;
				}
			}
			NPCindex++;
			iterator.Advance();
		}
	}
	if (spawn2_dump != 0)
		zone->DumpAllSpawn2(spawn2_dump, &spawn2index);
}

void EntityList::Depop(bool StartSpawnTimer) {
	LinkedListIterator<NPC*> iterator(npc_list);
	
	iterator.Reset();
	for(; iterator.MoreElements(); iterator.Advance())
	{
		NPC *it = iterator.GetData();
		if(it) {
			Mob *own = it->GetOwner();
			//do not depop player's pets...
			if(own && own->IsClient())
				continue;

			if(it->IsFindable())
				UpdateFindableNPCState(it, true);

			it->Depop(StartSpawnTimer);
		}
	}
}

void EntityList::DepopAll(int NPCTypeID, bool StartSpawnTimer) {
	LinkedListIterator<NPC*> iterator(npc_list);
	
	iterator.Reset();
	for(; iterator.MoreElements(); iterator.Advance())
	{
		NPC *it = iterator.GetData();
		if(it && (it->GetNPCTypeID() == (uint32)NPCTypeID))
			it->Depop(StartSpawnTimer);
	}
}

void EntityList::SendTraders(Client* client){
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	Client* trader;
	while(iterator.MoreElements()) {
		trader=iterator.GetData();
		if(trader->IsTrader())
			client->SendTraderPacket(trader);

		if(trader->IsBuyer())
			client->SendBuyerPacket(trader);

		iterator.Advance();
	}
}

void EntityList::RemoveFromHateLists(Mob* mob, bool settoone) {
	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckAggro(mob)) {
			if (!settoone)
			{
				iterator.GetData()->RemoveFromHateList(mob);
			}
			else
			{
				iterator.GetData()->SetHate(mob,1);
			}
		}
		iterator.Advance();
	}
}

void EntityList::RemoveDebuffs(Mob* caster)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		iterator.GetData()->BuffFadeDetrimentalByCaster(caster);
		iterator.Advance();
	}
}


// Currently, a new packet is sent per entity.
// @todo: Come back and use FLAG_COMBINED to pack
// all updates into one packet.
void EntityList::SendPositionUpdates(Client* client, uint32 cLastUpdate, float range, Entity* alwayssend, bool iSendEvenIfNotChanged) {
	range = range * range;
	LinkedListIterator<Mob*> iterator(mob_list);
	
	EQApplicationPacket* outapp = 0;
	PlayerPositionUpdateServer_Struct* ppu = 0;
	Mob* mob = 0;
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (outapp == 0) {
			outapp = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
			ppu = (PlayerPositionUpdateServer_Struct*)outapp->pBuffer;
		}
		mob = iterator.GetData()->CastToMob();
		if (mob && !mob->IsCorpse() && (iterator.GetData() != client)
			&& (mob->IsClient() || iSendEvenIfNotChanged || (mob->LastChange() >= cLastUpdate)) 
			&& (!iterator.GetData()->IsClient() || !iterator.GetData()->CastToClient()->GMHideMe(client))) {

			//bool Grouped = client->HasGroup() && mob->IsClient() && (client->GetGroup() == mob->CastToClient()->GetGroup());

			//if (range == 0 || (iterator.GetData() == alwayssend) || Grouped || (mob->DistNoRootNoZ(*client) <= range)) {
			if (range == 0 || (iterator.GetData() == alwayssend) || mob->IsClient() || (mob->DistNoRoot(*client) <= range)) {
				mob->MakeSpawnUpdate(ppu);
			}
			if(mob && mob->IsClient() && mob->GetID()>0) {
				client->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
			}
		}
		safe_delete(outapp);
		outapp = 0;	
		iterator.Advance();
	}
	
	safe_delete(outapp);
}

char* EntityList::MakeNameUnique(char* name) {
	bool used[300];
	memset(used, 0, sizeof(used));
	name[61] = 0; name[62] = 0; name[63] = 0;

	LinkedListIterator<Mob*> iterator(mob_list);

	iterator.Reset();
	int len = strlen(name);
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsMob()) {
			if (strncasecmp(iterator.GetData()->CastToMob()->GetName(), name, len) == 0) {
				if (Seperator::IsNumber(&iterator.GetData()->CastToMob()->GetName()[len])) {
					used[atoi(&iterator.GetData()->CastToMob()->GetName()[len])] = true;
				}
			}
		}
		iterator.Advance();
	}
	for (int i=0; i < 300; i++) {
		if (!used[i]) {
			#ifdef _WINDOWS
			snprintf(name, 64, "%s%03d", name, i);
			#else
			//glibc clears destination of snprintf
			//make a copy of name before snprintf--misanthropicfiend
			char temp_name[64];
			strn0cpy(temp_name, name, 64);
			snprintf(name, 64, "%s%03d", temp_name, i);
			#endif
			return name;
		}
	}
	LogFile->write(EQEMuLog::Error, "Fatal error in EntityList::MakeNameUnique: Unable to find unique name for '%s'", name);
	char tmp[64] = "!";
	strn0cpy(&tmp[1], name, sizeof(tmp) - 1);
	strcpy(name, tmp);
	return MakeNameUnique(name);
}

char* EntityList::RemoveNumbers(char* name) {
	char	tmp[64];
	memset(tmp, 0, sizeof(tmp));
	int k = 0;
	for (unsigned int i=0; i<strlen(name) && i<sizeof(tmp); i++) {
		if (name[i] < '0' || name[i] > '9')
			tmp[k++] = name[i];
	}
	strn0cpy(name, tmp, sizeof(tmp));
	return name;
}
void EntityList::ListNPCs(Client* client, const char* arg1, const char* arg2, uint8 searchtype) {
	if (arg1 == 0)
		searchtype = 0;
	else if (arg2 == 0 && searchtype >= 2)
		searchtype = 0;
	LinkedListIterator<NPC*> iterator(npc_list);
	uint32 x = 0;
	uint32 z = 0;
	char sName[36];
	
	iterator.Reset();
	client->Message(0, "NPCs in the zone:");
	if(searchtype == 0) {
		while(iterator.MoreElements())
		{
			NPC *n = iterator.GetData();

			client->Message(0, "  %5d: %s (%.0f, %0.f, %.0f)", n->GetID(), n->GetName(), n->GetX(), n->GetY(), n->GetZ());
			x++;
			z++;
			iterator.Advance();
		}
	}
	else if(searchtype == 1) {
		client->Message(0, "Searching by name method. (%s)",arg1);
		char* tmp = new char[strlen(arg1) + 1];
		strcpy(tmp, arg1);
		strupr(tmp);
		while(iterator.MoreElements()) {
			z++;
			strcpy(sName, iterator.GetData()->GetName());
			strupr(sName);
			if (strstr(sName, tmp)) {
				NPC *n = iterator.GetData();
				client->Message(0, "  %5d: %s (%.0f, %.0f, %.0f)", n->GetID(), n->GetName(), n->GetX(), n->GetY(), n->GetZ());
				x++;
			}
			iterator.Advance();
		}
		safe_delete_array(tmp);
	}
	else if(searchtype == 2) {
		client->Message(0, "Searching by number method. (%s %s)",arg1,arg2);
		while(iterator.MoreElements()) {
			z++;
			if ((iterator.GetData()->GetID() >= atoi(arg1)) && (iterator.GetData()->GetID() <= atoi(arg2)) && (atoi(arg1) <= atoi(arg2))) {
				client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
				x++;
			}
			iterator.Advance();
		}
	}
	client->Message(0, "%d npcs listed. There is a total of %d npcs in this zone.", x, z);
}

void EntityList::ListNPCCorpses(Client* client) {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	uint32 x = 0;
	
	iterator.Reset();
	client->Message(0, "NPC Corpses in the zone:");
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsNPCCorpse()) {
			client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
			x++;
		}
		iterator.Advance();
	}
	client->Message(0, "%d npc corpses listed.", x);
}

void EntityList::ListPlayerCorpses(Client* client) {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	uint32 x = 0;
	
	iterator.Reset();
	client->Message(0, "Player Corpses in the zone:");
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsPlayerCorpse()) {
			client->Message(0, "  %5d: %s", iterator.GetData()->GetID(), iterator.GetData()->GetName());
			x++;
		}
		iterator.Advance();
	}
	client->Message(0, "%d player corpses listed.", x);
}

void EntityList::FindPathsToAllNPCs()
{
	if(!zone->pathing)
		return;

	LinkedListIterator<NPC*> Iterator(npc_list);
	
	Iterator.Reset();

	while(Iterator.MoreElements())
	{
		VERTEX Node0 = zone->pathing->GetPathNodeCoordinates(0, false);
		VERTEX Dest(Iterator.GetData()->GetX(), Iterator.GetData()->GetY(), Iterator.GetData()->GetZ());
		list<int> Route = zone->pathing->FindRoute(Node0, Dest);
		if(Route.size() == 0)
			printf("Unable to find a route to %s\n", Iterator.GetData()->GetName());
		else
			printf("Found a route to %s\n", Iterator.GetData()->GetName());

		Iterator.Advance();
	}

	fflush(stdout);

}

// returns the number of corpses deleted. A negative number indicates an error code.
int32 EntityList::DeleteNPCCorpses() {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	int32 x = 0;
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsNPCCorpse()) {
			iterator.GetData()->Depop();
			x++;
		}
		iterator.Advance();
	}
	return x;
}

// returns the number of corpses deleted. A negative number indicates an error code.
int32 EntityList::DeletePlayerCorpses() {
	LinkedListIterator<Corpse*> iterator(corpse_list);
	int32 x = 0;
	
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->IsPlayerCorpse()) {
			iterator.GetData()->CastToCorpse()->Delete();
			x++;
		}
		iterator.Advance();
	}
	return x;
}
void EntityList::SendPetitionToAdmins(){
	LinkedListIterator<Client*> iterator(client_list);
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionUpdate,sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pcus = (PetitionUpdate_Struct*) outapp->pBuffer;
	pcus->petnumber = 0;		// Petition Number
	pcus->color = 0;
	pcus->status = 0xFFFFFFFF;
	pcus->senttime = 0;
	strcpy(pcus->accountid, "");
	strcpy(pcus->gmsenttoo, "");
	pcus->quetotal=0;
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CastToClient()->Admin() >= 80)
			iterator.GetData()->CastToClient()->QueuePacket(outapp);
		iterator.Advance();
	}
	safe_delete(outapp);
}
void EntityList::SendPetitionToAdmins(Petition* pet) {
	LinkedListIterator<Client*> iterator(client_list);
	
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionUpdate,sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pcus = (PetitionUpdate_Struct*) outapp->pBuffer;
	pcus->petnumber = pet->GetID();		// Petition Number
	if (pet->CheckedOut()) {
		pcus->color = 0x00;
		pcus->status = 0xFFFFFFFF;
		pcus->senttime = pet->GetSentTime();
		strcpy(pcus->accountid, "");
		strcpy(pcus->gmsenttoo, "");
	}
	else {
		pcus->color = pet->GetUrgency();	// 0x00 = green, 0x01 = yellow, 0x02 = red
		pcus->status = pet->GetSentTime();
		pcus->senttime = pet->GetSentTime();			// 4 has to be 0x1F
		strcpy(pcus->accountid, pet->GetAccountName());
		strcpy(pcus->charname, pet->GetCharName());
	}
	pcus->quetotal = petition_list.GetTotalPetitions();
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CastToClient()->Admin() >= 80) {
			if (pet->CheckedOut())
				strcpy(pcus->gmsenttoo, "");
			else
				strcpy(pcus->gmsenttoo, iterator.GetData()->CastToClient()->GetName());
			iterator.GetData()->CastToClient()->QueuePacket(outapp);
		}
		iterator.Advance();
	}
	safe_delete(outapp);
}

void EntityList::ClearClientPetitionQueue() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_PetitionUpdate,sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pet = (PetitionUpdate_Struct*) outapp->pBuffer;
	pet->color = 0x00;
	pet->status = 0xFFFFFFFF;
	pet->senttime = 0;
	strcpy(pet->accountid, "");
	strcpy(pet->gmsenttoo, "");
	strcpy(pet->charname, "");
	pet->quetotal = petition_list.GetTotalPetitions();
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CastToClient()->Admin() >= 100) {
			int x = 0;
			for (x=0;x<64;x++) {
				pet->petnumber = x;
				iterator.GetData()->CastToClient()->QueuePacket(outapp);
			}
		}
		iterator.Advance();
	}
	safe_delete(outapp);
	return;
}

void EntityList::WriteEntityIDs() {
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		cout << "ID: " << iterator.GetData()->GetID() << "  Name: " << iterator.GetData()->GetName() << endl;
		iterator.Advance();
	}
}

BulkZoneSpawnPacket::BulkZoneSpawnPacket(Client* iSendTo, uint32 iMaxSpawnsPerPacket) {
	data = 0;
	pSendTo = iSendTo;
	pMaxSpawnsPerPacket = iMaxSpawnsPerPacket;
}

BulkZoneSpawnPacket::~BulkZoneSpawnPacket() {
	SendBuffer();
	safe_delete_array(data)
}

bool BulkZoneSpawnPacket::AddSpawn(NewSpawn_Struct* ns) {
	if (!data) {
		data = new NewSpawn_Struct[pMaxSpawnsPerPacket];
		memset(data, 0, sizeof(NewSpawn_Struct) * pMaxSpawnsPerPacket);
		index = 0;
	}
	memcpy(&data[index], ns, sizeof(NewSpawn_Struct));
	index++;
	if (index >= pMaxSpawnsPerPacket) {
		SendBuffer();
		return true;
	}
	return false;
}

void BulkZoneSpawnPacket::SendBuffer() {
	if (!data)
		return;
	
	uint32 tmpBufSize = (index * sizeof(NewSpawn_Struct));
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_ZoneSpawns, (unsigned char *)data, tmpBufSize);
	
	if (pSendTo) {
		pSendTo->FastQueuePacket(&outapp);
	} else {
		entity_list.QueueClients(0, outapp);
		safe_delete(outapp);
	}
	memset(data, 0, sizeof(NewSpawn_Struct) * pMaxSpawnsPerPacket);
	index = 0;
}

void EntityList::DoubleAggro(Mob* who)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CheckAggro(who))
		  iterator.GetData()->SetHate(who,iterator.GetData()->CastToNPC()->GetHateAmount(who),iterator.GetData()->CastToNPC()->GetHateAmount(who)*2);
		iterator.Advance();
	}
}

void EntityList::HalveAggro(Mob* who)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CastToNPC()->CheckAggro(who))
		  iterator.GetData()->CastToNPC()->SetHate(who,iterator.GetData()->CastToNPC()->GetHateAmount(who)/2);
		iterator.Advance();
	}
}


void EntityList::Evade(Mob *who)
{
	uint32 flatval = who->GetLevel() * 13;
	int amt = 0;
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CastToNPC()->CheckAggro(who)){
			amt = iterator.GetData()->CastToNPC()->GetHateAmount(who);
			amt -= flatval;
			if(amt > 0)
				iterator.GetData()->CastToNPC()->SetHate(who, amt);
			else
				iterator.GetData()->CastToNPC()->SetHate(who, 0);
		}
		iterator.Advance();
	}
}

//removes "targ" from all hate lists, including feigned, in the zone
void EntityList::ClearAggro(Mob* targ) {
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckAggro(targ))
			iterator.GetData()->RemoveFromHateList(targ);
		iterator.GetData()->RemoveFromFeignMemory(targ->CastToClient()); //just in case we feigned
		iterator.Advance();
	}
}

void EntityList::ClearFeignAggro(Mob* targ)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CheckAggro(targ))
		{
			if(iterator.GetData()->SpecAttacks[IMMUNE_FEIGN_DEATH])
			{
				iterator.GetData()->SetHate(targ, 0);
				iterator.Advance();
				continue;
			}

			iterator.GetData()->RemoveFromHateList(targ);
			// EverHood 6/24/06
			// For client targets if the mob that hated us is 35+ 
			// there is a 3 outta 5 chance he adds us to feign memory
			if(targ->IsClient()){
				if (iterator.GetData()->GetLevel() >= 35 && (MakeRandomInt(1,100)<=60)){
					iterator.GetData()->AddFeignMemory(targ->CastToClient());
				} else {
					targ->CastToClient()->RemoveXTarget(iterator.GetData(), false);
				}
			}
		}
		iterator.Advance();
	}
}
// EverHood 6/17/06
void EntityList::ClearZoneFeignAggro(Client* targ)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.GetData()->RemoveFromFeignMemory(targ);
		targ->CastToClient()->RemoveXTarget(iterator.GetData(), false);
		iterator.Advance();
	}
}

void EntityList::AggroZone(Mob* who, int hate) {
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.GetData()->AddToHateList(who, hate);
		iterator.Advance();
	}
}

// Signal Quest command function
void EntityList::SignalMobsByNPCID(uint32 snpc, int signal_id)
{
	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		NPC *it = iterator.GetData();
		if (it->GetNPCTypeID() == snpc)
		{
			it->SignalNPC(signal_id);
		}
		iterator.Advance();
	}
}


bool EntityList::MakeTrackPacket(Client* client)
{
	uint32 distance = 0;
	float MobDistance;

	if(client->GetClass() == DRUID)
		distance = (client->GetSkill(TRACKING)*10);
	else if(client->GetClass() == RANGER)
		distance = (client->GetSkill(TRACKING)*12);
	else if(client->GetClass() == BARD)
		distance = (client->GetSkill(TRACKING)*7); 
	if(distance <= 0)
		return false;
	if(distance<300)
		distance=300;
	
	uint32 spe= 0;
	bool ret = false;
	
	spe = mob_list.Count() + 50;

	uchar* buffer1 = new uchar[sizeof(Track_Struct)];
	Track_Struct* track_ent = (Track_Struct*) buffer1;
	
	uchar* buffer2 = new uchar[sizeof(Track_Struct)*spe];
	Tracking_Struct* track_array = (Tracking_Struct*) buffer2;
	memset(track_array, 0, sizeof(Track_Struct)*spe);
	
	uint32 array_counter = 0;
	
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();

	Group *g = client->GetGroup();

	while(iterator.MoreElements())
	{
		if (iterator.GetData() && ((MobDistance = iterator.GetData()->DistNoZ(*client))<=distance))
		{
			if((iterator.GetData() != client) && iterator.GetData()->IsTrackable()) {
				memset(track_ent, 0, sizeof(Track_Struct));
				Mob* cur_entity = iterator.GetData();
				track_ent->entityid = cur_entity->GetID();
				track_ent->distance = MobDistance;
				track_ent->level = cur_entity->GetLevel();
				track_ent->NPC = !cur_entity->IsClient();
				if(g && cur_entity->IsClient() && g->IsGroupMember(cur_entity->CastToMob()))
					track_ent->GroupMember = 1;
				else
					track_ent->GroupMember = 0;
				strn0cpy(track_ent->name, cur_entity->GetName(), sizeof(track_ent->name));
				memcpy(&track_array->Entrys[array_counter], track_ent, sizeof(Track_Struct));
				array_counter++;
			}
		}

		iterator.Advance();
	}

	if(array_counter <= spe) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Track,sizeof(Track_Struct)*(array_counter));
		memcpy(outapp->pBuffer, track_array,sizeof(Track_Struct)*(array_counter));
		outapp->priority = 6;
		client->QueuePacket(outapp);
		safe_delete(outapp);
		ret = true;
	}
	else {
		LogFile->write(EQEMuLog::Status, "ERROR: Unable to transmit a Tracking_Struct packet. Mobs in zone = %i. Mobs in packet = %i", array_counter, spe);
	}
	
	safe_delete_array(buffer1);
	safe_delete_array(buffer2);
	
	return ret;
}

void EntityList::MessageGroup(Mob* sender, bool skipclose, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, message);
	vsnprintf(buffer, 4095, message, argptr);
	va_end(argptr);

	float dist2 = 100;

	if (skipclose)
		dist2 = 0;

	LinkedListIterator<Client*> iterator(client_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData() != sender && (iterator.GetData()->Dist(*sender) <= dist2 || iterator.GetData()->GetGroup() == sender->CastToClient()->GetGroup())) {
			iterator.GetData()->Message(type, buffer);
		}
		iterator.Advance();
	}
}


bool EntityList::Fighting(Mob* targ) {
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->CheckAggro(targ))
		{
			return true;
		}
		iterator.Advance();
	}
	return false;
}

void EntityList::AddHealAggro(Mob* target, Mob* caster, uint16 thedam)
{
	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	NPC *cur = NULL;
	uint16 count = 0;
	while(iterator.MoreElements())
	{
		cur = iterator.GetData();
		
		if(!cur->CheckAggro(target))
		{
			iterator.Advance();
			continue;
		}
		if (!cur->IsMezzed() && !cur->IsStunned() && !cur->IsFeared())
		{
			++count;
		}
		iterator.Advance();
	}

	if(thedam > 1)
	{
		if(count > 0)
			thedam = (thedam / count);

		if(thedam < 1)
			thedam = 1;
	}

	cur = NULL;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		cur = iterator.GetData();
		if(!cur->CheckAggro(target)){
			iterator.Advance();
			continue;
		}

		if (!cur->IsMezzed() && !cur->IsStunned() && !cur->IsFeared())
		{
			if(cur->IsPet()){
				if(caster){
					if(cur->CheckAggro(caster))
					{
						cur->AddToHateList(caster, thedam);
					}
				}
			}
			else{
				if(caster){
					if(cur->CheckAggro(caster))
					{
						cur->AddToHateList(caster, thedam);
					}
					else
					{
						cur->AddToHateList(caster, thedam*0.33);
					}
				}
			}
		}
		iterator.Advance();
	}
}

void EntityList::OpenDoorsNear(NPC* who)
{
	LinkedListIterator<Doors*> iterator(door_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Doors *cdoor = iterator.GetData();
		if(cdoor && !cdoor->IsDoorOpen()) {
			float zdiff = who->GetZ() - cdoor->GetZ();
			if(zdiff < 0)
				zdiff = 0 - zdiff;
			float curdist = 0;
			float tmp = who->GetX() - cdoor->GetX();
			curdist += tmp * tmp;
			tmp = who->GetY() - cdoor->GetY();
			curdist += tmp * tmp;
			if (zdiff < 10 && curdist <= 100) {
				cdoor->NPCOpen(who);
			}
		}
		iterator.Advance();
	}
}

void EntityList::SendAlarm(Trap* trap, Mob* currenttarget, uint8 kos) 
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	
	float val2 = trap->effectvalue * trap->effectvalue;
	
	while(iterator.MoreElements())
	{
		NPC *cur = iterator.GetData();
		float curdist = 0;
		float tmp = cur->GetX() - trap->x;
		curdist += tmp*tmp;
		tmp = cur->GetY() - trap->y;
		curdist += tmp*tmp;
		tmp = cur->GetZ() - trap->z;
		curdist += tmp*tmp;
		if (!cur->GetOwner() && 
			/*!cur->CastToMob()->dead && */
			!cur->IsEngaged() && 
			curdist <= val2 )
		{
			if(kos)
			{
				uint8 factioncon = currenttarget->GetReverseFactionCon(cur);
				if(factioncon == FACTION_THREATENLY || factioncon == FACTION_SCOWLS)
				{
					cur->AddToHateList(currenttarget,1);
				}
			}
			else
			{
				cur->AddToHateList(currenttarget,1);
			}
		}
		iterator.Advance();
	}
}

void EntityList::AddProximity(NPC *proximity_for) {
	RemoveProximity(proximity_for->GetID());
	
	proximity_list.Insert(proximity_for);
	
	proximity_for->proximity = new NPCProximity;
}

bool EntityList::RemoveProximity(uint16 delete_npc_id) {
	LinkedListIterator<NPC*> iterator(proximity_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		NPC *d = iterator.GetData();
		if(d->GetID() == delete_npc_id) {
			//safe_delete(d->proximity);
			iterator.RemoveCurrent(false);
			return true;
		}
		iterator.Advance();
	}
	return false;
}

void EntityList::RemoveAllLocalities() {
	LinkedListIterator<NPC*> iterator(proximity_list);
	iterator.Reset();
	while(iterator.MoreElements())
		iterator.RemoveCurrent(false);
}

void EntityList::ProcessMove(Client *c, float x, float y, float z) {
	/*
		We look through each proximity, looking to see if last_* was in(out)
		the proximity, and the new supplied coords are out(in)...
	*/
	LinkedListIterator<NPC*> iterator(proximity_list);
	std::list<int> skip_ids;
	
	float last_x = c->ProximityX();
	float last_y = c->ProximityY();
	float last_z = c->ProximityZ();
	
	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance()) {
		NPC *d = iterator.GetData();
		NPCProximity *l = d->proximity;
		if(l == NULL)
			continue;

		//This is done to address the issue of this code not being reentrant 
		//because perl can call clear_proximity() while we're still iterating through the list
		//This causes our list to become invalid but we don't know it.  On GCC it's basic heap 
		//corruption and it doesn't appear to catch it at all.
		//MSVC it's a crash with 0xfeeefeee debug address (freed memory off the heap)
		std::list<int>::iterator iter = skip_ids.begin();
		bool skip = false;
		while(iter != skip_ids.end())
		{
			if(d->GetID() == (*iter))
			{
				skip = true;
				break;
			}
			iter++;
		}

		if(skip)
		{
			continue;
		}
		
		//check both bounding boxes, if either coords pairs
		//cross a boundary, send the event.
		bool old_in = true;
		bool new_in = true;
		if(   last_x < l->min_x || last_x > l->max_x
		   || last_y < l->min_y || last_y > l->max_y
		   || last_z < l->min_z || last_z > l->max_z ) {
			old_in = false;
		}
		if(   x < l->min_x || x > l->max_x
		   || y < l->min_y || y > l->max_y
		   || z < l->min_z || z > l->max_z ) {
			new_in = false;
		}
		
		if(old_in && !new_in) {
			//we were in the proximity, we are no longer, send event exit
            parse->EventNPC(EVENT_EXIT, d, c, "", 0);
			
			//Reentrant fix
			iterator.Reset();
			skip_ids.push_back(d->GetID());
		} else if(new_in && !old_in) {
			//we were not in the proximity, we are now, send enter event
			parse->EventNPC(EVENT_ENTER, d, c, "", 0);

			//Reentrant fix
			iterator.Reset();
			skip_ids.push_back(d->GetID());
		}
	}
	
}

void EntityList::ProcessProximitySay(const char *Message, Client *c, uint8 language) {

	if(!Message || !c)
		return;

	LinkedListIterator<NPC*> iterator(proximity_list);

	for(iterator.Reset(); iterator.MoreElements(); iterator.Advance()) {
		NPC *d = iterator.GetData();
		NPCProximity *l = d->proximity;
		if(l == NULL || !l->say)
			continue;

		if(   c->GetX() < l->min_x || c->GetX() > l->max_x
		   || c->GetY() < l->min_y || c->GetY() > l->max_y
		   || c->GetZ() < l->min_z || c->GetZ() > l->max_z )
			continue;

        parse->EventNPC(EVENT_PROXIMITY_SAY, d, c, Message, language);
	}
}

void EntityList::SaveAllClientsTaskState() {

	if(!taskmanager) return;

	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		if(client->IsTaskStateLoaded()) {
			client->SaveTaskState();
		}

		iterator.Advance();
	}
}

void EntityList::ReloadAllClientsTaskState(int TaskID) {

	if(!taskmanager) return;

	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* client = iterator.GetData();
		if(client->IsTaskStateLoaded()) {
			// If we have been passed a TaskID, only reload the client state if they have
			// that Task active.
			if((!TaskID) || (TaskID && client->IsTaskActive(TaskID))) {
				_log(TASKS__CLIENTLOAD, "Reloading Task State For Client %s", client->GetName());
				client->RemoveClientTaskState();
				client->LoadClientTaskState();
				taskmanager->SendActiveTasksToClient(client);
			}
		}
		iterator.Advance();
	}
}

bool EntityList::IsMobInZone(Mob *who) {
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(who == iterator.GetData())
			return(true);
		iterator.Advance();
	}
	return(false);
}

/*
Code to limit the ammount of certain NPCs in a given zone.
Primarily used to make a named mob unique within the zone, but written
to be more generic allowing limits larger than 1.

Maintain this stuff in a seperate list since the number
of limited NPCs will most likely be much smaller than the number
of NPCs in the entire zone.
*/
void EntityList::LimitAddNPC(NPC *npc) {
	if(!npc)
		return;
	
	SpawnLimitRecord r;
	
	uint16 eid = npc->GetID();
	r.spawngroup_id = npc->GetSp2();
	r.npc_type = npc->GetNPCTypeID();
	
	npc_limit_list[eid] = r;
}

void EntityList::LimitRemoveNPC(NPC *npc) {
	if(!npc)
		return;
	
	uint16 eid = npc->GetID();
	npc_limit_list.erase(eid);
}

//check a limit over the entire zone.
//returns true if the limit has not been reached
bool EntityList::LimitCheckType(uint32 npc_type, int count) {
	if(count < 1)
		return(true);
	
	map<uint16, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();
	
	for(; cur != end; cur++) {
		if(cur->second.npc_type == npc_type) {
			count--;
			if(count == 0) {
				return(false);
			}
		}
	}
	return(true);
}

//check limits on an npc type in a given spawn group.
//returns true if the limit has not been reached
bool EntityList::LimitCheckGroup(uint32 spawngroup_id, int count) {
	if(count < 1)
		return(true);
	
	map<uint16, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();
	
	for(; cur != end; cur++) {
		if(cur->second.spawngroup_id == spawngroup_id) {
			count--;
			if(count == 0) {
				return(false);
			}
		}
	}
	return(true);
}

//check limits on an npc type in a given spawn group, and
//checks limits on the entire zone in one pass.
//returns true if neither limit has been reached
bool EntityList::LimitCheckBoth(uint32 npc_type, uint32 spawngroup_id, int group_count, int type_count) {
	if(group_count < 1 && type_count < 1)
		return(true);
	
	map<uint16, SpawnLimitRecord>::iterator cur,end;
	cur = npc_limit_list.begin();
	end = npc_limit_list.end();
	
	for(; cur != end; cur++) {
		if(cur->second.npc_type == npc_type) {
			type_count--;
			if(type_count == 0) {
				return(false);
			}
		}
		if(cur->second.spawngroup_id == spawngroup_id) {
			group_count--;
			if(group_count == 0) {
				return(false);
			}
		}
	}
	return(true);
}

bool EntityList::LimitCheckName(const char *npc_name)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		NPC* npc = iterator.GetData();
		if(npc)
		{
			if(strcasecmp(npc_name, npc->GetRawNPCTypeName()) == 0)
			{
				return false;
			}
		}
		iterator.Advance();
	}
	return true;
}

void EntityList::RadialSetLogging(Mob *around, bool enabled, bool clients, bool non_clients, float range) {
	float range2 = range * range;
	
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob* mob = iterator.GetData();

		iterator.Advance();

		if(mob->IsClient()) {
			if(!clients)
				continue;
		} else {
			if(!non_clients)
				continue;
		}
		
		if(around->DistNoRoot(*mob) > range2)
			continue;
		
		if(enabled)
			mob->EnableLogging();
		else
			mob->DisableLogging();
	}
}

void EntityList::UpdateHoTT(Mob* target) {
	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Client* c = iterator.GetData();
		if (c->GetTarget() == target) {
			if (target->GetTarget()) c->SetHoTT(target->GetTarget()->GetID());
			else c->SetHoTT(0);

			c->UpdateXTargetType(TargetsTarget, target->GetTarget());
		}
		iterator.Advance();
	}
}

void EntityList::DestroyTempPets(Mob *owner)
{
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		NPC* n = iterator.GetData();
		if(n->GetSwarmInfo())
		{
			if(n->GetSwarmInfo()->owner_id == owner->GetID())
			{
				n->Depop();
			}
		}
		iterator.Advance();
	}
}

bool Entity::CheckCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z, float trg_x, float trg_y, float trg_z, float perwalk)
{
	if(zone->zonemap == NULL) {
		return(true);
	}
	VERTEX myloc;
	VERTEX oloc;
	VERTEX hit;

	myloc.x = cur_x;
	myloc.y = cur_y;
	myloc.z = cur_z+5;

	oloc.x = trg_x;
	oloc.y = trg_y;
	oloc.z = trg_z+5;

	if (myloc.x == oloc.x && myloc.y == oloc.y && myloc.z == oloc.z)
		return true;

	FACE *onhit;

	if (!zone->zonemap->LineIntersectsZoneNoZLeaps(myloc,oloc,perwalk,&hit,&onhit))
		return true;
	return false;
}

void EntityList::QuestJournalledSayClose(Mob *sender, Client *QuestInitiator, float dist, const char* mobname, const char* message)
{
       Client *c;
       LinkedListIterator<Client*> iterator(client_list);
       float dist2 = dist * dist;

       // Send the message to the quest initiator such that the client will enter it into the NPC Quest Journal
       if(QuestInitiator) {

               char *buf = new char[strlen(mobname) + strlen(message) + 10];
               sprintf(buf, "%s says, '%s'", mobname, message);
               QuestInitiator->QuestJournalledMessage(mobname, buf);
               safe_delete_array(buf);
       }
       // Use the old method for all other nearby clients
       for(iterator.Reset(); iterator.MoreElements(); iterator.Advance())
       {
               c = iterator.GetData();
               if(c && (c != QuestInitiator) && c->DistNoRoot(*sender) <= dist2)
                       c->Message_StringID(10, GENERIC_SAY, mobname, message);
       }
}

Corpse* EntityList::GetClosestCorpse(Mob* sender, const char* Name)
{
	if(!sender) 
		return NULL;

	uint32 CurrentDistance, ClosestDistance = 4294967295u;

	Corpse *CurrentCorpse, *ClosestCorpse = NULL;

	LinkedListIterator<Corpse*> iterator(corpse_list);

	iterator.Reset();

	while(iterator.MoreElements())
	{
		CurrentCorpse = iterator.GetData();

		iterator.Advance();
		
		if(Name && strcasecmp(CurrentCorpse->GetOwnerName(), Name))
			continue;

		CurrentDistance = ((CurrentCorpse->GetY() - sender->GetY()) * (CurrentCorpse->GetY() - sender->GetY())) + 
				  ((CurrentCorpse->GetX() - sender->GetX()) * (CurrentCorpse->GetX() - sender->GetX()));

		if(CurrentDistance < ClosestDistance)
		{
			ClosestDistance = CurrentDistance;

			ClosestCorpse = CurrentCorpse;

		}
	}
	return ClosestCorpse;
}

void EntityList::ForceGroupUpdate(uint32 gid) {
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		if(iterator.GetData()){
			Group *g = NULL;
			g = iterator.GetData()->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					database.RefreshGroupFromDB(iterator.GetData());
				}
			}
		}
		iterator.Advance(); 
	} 
}

void EntityList::SendGroupLeave(uint32 gid, const char *name) {
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		Client *c = iterator.GetData();
		if(c){
			Group *g = NULL;
			g = c->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
					strcpy(gj->membername, name);
					gj->action = groupActLeave;
					strcpy(gj->yourname, c->GetName());
					Mob *Leader = g->GetLeader();
					if(Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);					
					c->QueuePacket(outapp);
					safe_delete(outapp);
					g->DelMemberOOZ(name);
					if(g->IsLeader(c) && c->IsLFP())
						c->UpdateLFP();
				}
			}
		}
		iterator.Advance(); 
	} 
}

void EntityList::SendGroupJoin(uint32 gid, const char *name) {
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		if(iterator.GetData()){
			Group *g = NULL;
			g = iterator.GetData()->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
					GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;	
					strcpy(gj->membername, name);
					gj->action = groupActJoin;
					strcpy(gj->yourname, iterator.GetData()->GetName());
					Mob *Leader = g->GetLeader();
					if(Leader)
						Leader->CastToClient()->GetGroupAAs(&gj->leader_aas);
					
					iterator.GetData()->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
		}
		iterator.Advance(); 
	} 
}

void EntityList::GroupMessage(uint32 gid, const char *from, const char *message)
{
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset(); 
	while(iterator.MoreElements()) {
		if(iterator.GetData()){
			Group *g = NULL;
			g = iterator.GetData()->GetGroup();
			if(g){
				if(g->GetID() == gid)
				{
					iterator.GetData()->ChannelMessageSend(from, iterator.GetData()->GetName(),2,0,message);
				}
			}
		}
		iterator.Advance(); 
	} 
}

uint16 EntityList::CreateGroundObject(uint32 itemid, float x, float y, float z, float heading, uint32 decay_time)
{
	const Item_Struct* is = database.GetItem(itemid);
	if(is)
	{
		ItemInst *i = new ItemInst(is, is->MaxCharges);
		if(i)
		{
			Object* object = new Object(i,x,y,z,heading,decay_time);
			entity_list.AddObject(object, true);

			safe_delete(i);
			if(object)
				return object->GetID();
		}
		return 0; // fell through itemstruct
	}
	return 0; // fell through everything, this is bad/incomplete from perl
}

uint16 EntityList::CreateGroundObjectFromModel(const char *model, float x, float y, float z, float heading, uint8 type, uint32 decay_time)
{
	if(model)
	{
			Object* object = new Object(model,x,y,z,heading,type);
			entity_list.AddObject(object, true);

			if(object)
				return object->GetID();
	}
	return 0; // fell through everything, this is bad/incomplete from perl
}

uint16 EntityList::CreateDoor(const char *model, float x, float y, float z, float heading, uint8 opentype, uint16 size)
{
	if(model)
	{
			Doors* door = new Doors(model,x,y,z,heading,opentype, size);
			RemoveAllDoors();
			zone->LoadZoneDoors(zone->GetShortName(), zone->GetInstanceVersion());
			entity_list.AddDoor(door);
			entity_list.RespawnAllDoors();

			if(door)
				return door->GetEntityID();
	}
	return 0; // fell through everything, this is bad/incomplete from perl
}


Mob* EntityList::GetTargetForMez(Mob* caster)
{
	if(!caster)
		return NULL;

	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	//TODO: make this smarter and not mez targets being damaged by dots
	while(iterator.MoreElements()) {
		Mob* d = iterator.GetData();
		if(d){
			if(d == caster){ //caster can't pick himself
				iterator.Advance();
				continue;
			}

			if(caster->GetTarget() == d){ //caster can't pick his target
				iterator.Advance();			
				continue;
			}

			if(!caster->CheckAggro(d)){ //caster can't pick targets that aren't aggroed on himself
				iterator.Advance();
				continue;
			}

			if(caster->DistNoRoot(*d) > 22250){ //only pick targets within 150 range
				iterator.Advance();
				continue;
			}

			if(!caster->CheckLosFN(d)){ //this is wasteful but can't really think of another way to do it 
				iterator.Advance();		//that wont have us trying to los the same target every time
				continue;			   //it's only in combat so it's impact should be minimal.. but stil.
			}
			return d;
		}
		iterator.Advance();
	}
	return NULL;
}

void EntityList::SendZoneAppearance(Client *c)
{
	if(!c)
		return;

	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob *cur = iterator.GetData();

		if(cur)
		{
			if(cur == c)
			{
				iterator.Advance();
				continue;
			}
			if(cur->GetAppearance() != eaStanding)
			{
				cur->SendAppearancePacket(AT_Anim, cur->GetAppearanceValue(cur->GetAppearance()), false, true, c);
			}
			if(cur->GetSize() != cur->GetBaseSize())
			{
				cur->SendAppearancePacket(AT_Size, (uint32)cur->GetSize(), false, true, c);
			}
		}
		iterator.Advance();
	}
}

void EntityList::SendNimbusEffects(Client *c)
{
	if(!c)
		return;

	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob *cur = iterator.GetData();

		if(cur)
		{
			if(cur == c)
			{
				iterator.Advance();
				continue;
			}
			if(cur->GetNimbusEffect1() != 0)
			{
				cur->SendSpellEffect(cur->GetNimbusEffect1(), 1000, 0, 1, 3000, false, c);
			}
			if(cur->GetNimbusEffect2() != 0)
			{
				cur->SendSpellEffect(cur->GetNimbusEffect2(), 2000, 0, 1, 3000, false, c);
			}
			if(cur->GetNimbusEffect3() != 0)
			{
				cur->SendSpellEffect(cur->GetNimbusEffect3(), 3000, 0, 1, 3000, false, c);
			}
		}
		iterator.Advance();
	}
}

void EntityList::SendUntargetable(Client *c)
{
	if(!c)
		return;

	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	while(iterator.MoreElements()) {
		Mob *cur = iterator.GetData();

		if(cur)
		{
			if(cur == c)
			{
				iterator.Advance();
				continue;
			}
			if(!cur->IsTargetable()) {
                cur->SendTargetable(false, c);
            }
		}
		iterator.Advance();
	}
}

void EntityList::ZoneWho(Client *c, Who_All_Struct* Who) {

	// This is only called for SoF clients, as regular /who is now handled server-side for that client.
	//
	uint32 PacketLength = 0;

	uint32 Entries = 0;

	uint8 WhomLength = strlen(Who->whom);

	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 

	while(iterator.MoreElements())  {

		Client *ClientEntry = iterator.GetData();

		iterator.Advance(); 

		if(ClientEntry) {

			if(ClientEntry->GMHideMe(c))
				continue;

			if((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace))
				continue;

			if((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass))
				continue;

			if((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow))
				continue;

			if((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh))
				continue;

			if(Who->guildid != 0xFFFFFFFF) {

				if((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader())
					continue;

				if((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer())
					continue;

				if(Who->guildid != ClientEntry->GuildID())
					continue;
			}

			if(WhomLength && strncasecmp(Who->whom, ClientEntry->GetName(), WhomLength) &&
			   strncasecmp(guild_mgr.GetGuildName(ClientEntry->GuildID()), Who->whom, WhomLength))
				continue;

			Entries++;

			PacketLength = PacketLength + strlen(ClientEntry->GetName());

			if(strlen(guild_mgr.GetGuildName(ClientEntry->GuildID())) > 0) 
				PacketLength = PacketLength + strlen(guild_mgr.GetGuildName(ClientEntry->GuildID())) + 2;
		}
	} 

	PacketLength = PacketLength + sizeof(WhoAllReturnStruct) + (47 * Entries);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_WhoAllResponse, PacketLength);

	char *Buffer = (char *)outapp->pBuffer;

	WhoAllReturnStruct *WARS = (WhoAllReturnStruct *)Buffer;

	WARS->id = 0;

	WARS->playerineqstring = 5001;

	strncpy(WARS->line, "---------------------------", sizeof(WARS->line));

	WARS->unknown35 = 0x0a;

	WARS->unknown36 = 0;
	
	switch(Entries) {
		case 0:
			WARS->playersinzonestring = 5029;
			break;
		case 1:
			WARS->playersinzonestring = 5028; // 5028 There is %1 player in EverQuest.
			break;
		default:
			WARS->playersinzonestring = 5036; // 5036 There are %1 players in EverQuest.
	}

	WARS->unknown44[0] = 0;

	WARS->unknown44[1] = 0;

	WARS->unknown52 = Entries;

	WARS->unknown56 = Entries;

	WARS->playercount = Entries;

	Buffer += sizeof(WhoAllReturnStruct);

	iterator.Reset(); 

	while(iterator.MoreElements()) {

		Client *ClientEntry = iterator.GetData();

		iterator.Advance();

		if(ClientEntry) {

			if(ClientEntry->GMHideMe(c)) continue;

			if((Who->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != Who->wrace))
				continue;

			if((Who->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != Who->wclass))
				continue;

			if((Who->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < Who->lvllow))
				continue;

			if((Who->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > Who->lvlhigh))
				continue;

			if(Who->guildid != 0xFFFFFFFF) {

				if((Who->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader())
					continue;

				if((Who->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer())
					continue;

				if(Who->guildid != ClientEntry->GuildID())
					continue;
			}

			if(WhomLength && strncasecmp(Who->whom, ClientEntry->GetName(), WhomLength) &&
			   strncasecmp(guild_mgr.GetGuildName(ClientEntry->GuildID()), Who->whom, WhomLength))
				continue;

			string GuildName;

			if((ClientEntry->GuildID() != GUILD_NONE) && (ClientEntry->GuildID() > 0)) {

				GuildName = "<";

				GuildName += guild_mgr.GetGuildName(ClientEntry->GuildID());

				GuildName += ">";
			}

			uint32 FormatMSGID=5025; // 5025 %T1[%2 %3] %4 (%5) %6 %7 %8 %9

			if(ClientEntry->GetAnon() == 1)
				FormatMSGID = 5024; // 5024 %T1[ANONYMOUS] %2 %3
			else if(ClientEntry->GetAnon() == 2)
				FormatMSGID = 5023; // 5023 %T1[ANONYMOUS] %2 %3 %4

			uint32 PlayerClass = 0;

			uint32 PlayerLevel = 0;

			uint32 PlayerRace = 0;

			uint32 ZoneMSGID = 0xFFFFFFFF;

			if(ClientEntry->GetAnon()==0) {

				PlayerClass = ClientEntry->GetClass();

				PlayerLevel = ClientEntry->GetLevel();

				PlayerRace = ClientEntry->GetRace();
			}				

			WhoAllPlayerPart1* WAPP1 = (WhoAllPlayerPart1*)Buffer;

			WAPP1->FormatMSGID = FormatMSGID;

			WAPP1->PIDMSGID = 0xFFFFFFFF;

			strcpy(WAPP1->Name, ClientEntry->GetName());

			Buffer += sizeof(WhoAllPlayerPart1) + strlen(WAPP1->Name);

			WhoAllPlayerPart2* WAPP2 = (WhoAllPlayerPart2*)Buffer;

			if(ClientEntry->IsTrader())
				WAPP2->RankMSGID = 12315;
			else if(ClientEntry->IsBuyer())
				WAPP2->RankMSGID = 6056;
			else if(ClientEntry->Admin() >= 10)
				WAPP2->RankMSGID = 12312;
			else
				WAPP2->RankMSGID = 0xFFFFFFFF;

			strcpy(WAPP2->Guild, GuildName.c_str());

			Buffer += sizeof(WhoAllPlayerPart2) + strlen(WAPP2->Guild);

			WhoAllPlayerPart3* WAPP3 = (WhoAllPlayerPart3*)Buffer;

			WAPP3->Unknown80[0] = 0xFFFFFFFF;

			if(ClientEntry->IsLD())
				WAPP3->Unknown80[1] = 12313; // LinkDead
			else
				WAPP3->Unknown80[1] = 0xFFFFFFFF;

			WAPP3->ZoneMSGID = ZoneMSGID;

			WAPP3->Zone = 0;

			WAPP3->Class_ = PlayerClass;

			WAPP3->Level =  PlayerLevel;

			WAPP3->Race = PlayerRace;

			WAPP3->Account[0] = 0;

			Buffer += sizeof(WhoAllPlayerPart3);

			WhoAllPlayerPart4* WAPP4 = (WhoAllPlayerPart4*)Buffer;

			WAPP4->Unknown100 = 0;

			Buffer += sizeof(WhoAllPlayerPart4);
		}

	}

	c->QueuePacket(outapp);

	safe_delete(outapp);
}

void EntityList::UnMarkNPC(uint16 ID) 
{
	// Designed to be called from the Mob destructor, this method calls Group::UnMarkNPC for
	// each group to remove the dead mobs entity ID from the groups list of NPCs marked via the
	// Group Leadership AA Mark NPC ability.
	//
	LinkedListIterator<Client*> iterator(client_list); 

	iterator.Reset(); 

	while(iterator.MoreElements())
	{
		if(iterator.GetData())
		{
			Group *g = NULL;

			g = iterator.GetData()->GetGroup();

			if(g)
				g->UnMarkNPC(ID);
		}
		iterator.Advance(); 
	} 
}

uint32 EntityList::CheckNPCsClose(Mob *center)
{
    LinkedListIterator<NPC*> iterator(npc_list);
    uint32 count = 0;

    iterator.Reset();
    while(iterator.MoreElements())
    {
		NPC *current = iterator.GetData();
		if(!current)
		{
			iterator.Advance();
			continue;
		}

		if(current == center)
		{
			iterator.Advance();
			continue;
		}

		if(current->IsPet())
		{
			iterator.Advance();
			continue;
		}

		if(current->GetClass() == LDON_TREASURE)
		{
			iterator.Advance();
			continue;
		}

		if(current->GetBodyType() == BT_NoTarget ||
			current->GetBodyType() == BT_Special)
		{
			iterator.Advance();
			continue;
		}

		float xDiff = current->GetX() - center->GetX();
		float yDiff = current->GetY() - center->GetY();
		float zDiff = current->GetZ() - center->GetZ();
		float dist = ((xDiff * xDiff) + (yDiff * yDiff) + (zDiff * zDiff));

		if(dist <= RuleR(Adventure, DistanceForRescueAccept))
		{
			count++;
		}
		iterator.Advance();
	}
	return count;
}

void EntityList::GateAllClients()
{
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Client *c = iterator.GetData();
		if(c)
		{
			c->GoToBind();
		}
		iterator.Advance();
	}
}

void EntityList::SignalAllClients(uint32 data)
{
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Client *ent = iterator.GetData();
		if(ent)
		{
			ent->Signal(data);
		}
		iterator.Advance();
	}
}

void EntityList::GetMobList(list<Mob*> &m_list)
{
	m_list.clear();
	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Mob *ent = iterator.GetData();
		m_list.push_back(ent);
		iterator.Advance();
	}
}

void EntityList::GetNPCList(list<NPC*> &n_list)
{
	n_list.clear();
	LinkedListIterator<NPC*> iterator(npc_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		NPC *ent = iterator.GetData();
		n_list.push_back(ent);
		iterator.Advance();
	}
}

void EntityList::GetClientList(list<Client*> &c_list)
{
	c_list.clear();
	LinkedListIterator<Client*> iterator(client_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Client *ent = iterator.GetData();
		c_list.push_back(ent);
		iterator.Advance();
	}
}

void EntityList::GetCorpseList(list<Corpse*> &c_list)
{
	c_list.clear();
	LinkedListIterator<Corpse*> iterator(corpse_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Corpse *ent = iterator.GetData();
		c_list.push_back(ent);
		iterator.Advance();
	}
}

void EntityList::GetObjectList(list<Object*> &o_list)
{
	o_list.clear();
	LinkedListIterator<Object*> iterator(object_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Object *ent = iterator.GetData();
		o_list.push_back(ent);
		iterator.Advance();
	}
}

void EntityList::GetDoorsList(list<Doors*> &o_list)
{
	o_list.clear();
	LinkedListIterator<Doors*> iterator(door_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Doors *ent = iterator.GetData();
		o_list.push_back(ent);
		iterator.Advance();
	}
}

void EntityList::UpdateQGlobal(uint32 qid, QGlobal newGlobal)
{
	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Mob *ent = iterator.GetData();
		
		if(ent->IsClient())
		{
			QGlobalCache *qgc = ent->CastToClient()->GetQGlobals();
			if(qgc)
			{
				uint32 char_id = ent->CastToClient()->CharacterID();
				if(newGlobal.char_id == char_id && newGlobal.npc_id == 0)
				{
					qgc->AddGlobal(qid, newGlobal);
				}
			}
		}
		else if(ent->IsNPC())
		{
			QGlobalCache *qgc = ent->CastToNPC()->GetQGlobals();
			if(qgc)
			{
				uint32 npc_id = ent->GetNPCTypeID();
				if(newGlobal.npc_id == npc_id)
				{
					qgc->AddGlobal(qid, newGlobal);
				}
			}
		}

		iterator.Advance();
	}
}

void EntityList::DeleteQGlobal(std::string name, uint32 npcID, uint32 charID, uint32 zoneID)
{
	LinkedListIterator<Mob*> iterator(mob_list); 
	iterator.Reset();
	while(iterator.MoreElements()) 
	{
		Mob *ent = iterator.GetData();
		
		if(ent->IsClient())
		{
			QGlobalCache *qgc = ent->CastToClient()->GetQGlobals();
			if(qgc)
			{
				qgc->RemoveGlobal(name, npcID, charID, zoneID);
			}
		}
		else if(ent->IsNPC())
		{
			QGlobalCache *qgc = ent->CastToNPC()->GetQGlobals();
			if(qgc)
			{
				qgc->RemoveGlobal(name, npcID, charID, zoneID);
			}
		}

		iterator.Advance();
	}
}

void EntityList::SendFindableNPCList(Client *c)
{
	if(!c)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SendFindableNPCs, sizeof(FindableNPC_Struct));

	FindableNPC_Struct *fnpcs = (FindableNPC_Struct *)outapp->pBuffer;

	fnpcs->Unknown109 = 0x16;
	fnpcs->Unknown110 = 0x06;
	fnpcs->Unknown111 = 0x24;

	fnpcs->Action = 0;


	LinkedListIterator<NPC*> iterator(npc_list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData())
		{
			NPC *n = iterator.GetData();

			if(n->IsFindable())
			{
				fnpcs->EntityID = n->GetID();
				strn0cpy(fnpcs->Name, n->GetCleanName(), sizeof(fnpcs->Name));
				strn0cpy(fnpcs->LastName, n->GetLastName(), sizeof(fnpcs->LastName));
				fnpcs->Race = n->GetRace();
				fnpcs->Class = n->GetClass();

				c->QueuePacket(outapp);
			}

		}
		iterator.Advance();
	}
	safe_delete(outapp);
}

void EntityList::UpdateFindableNPCState(NPC *n, bool Remove)
{
	if(!n || !n->IsFindable())
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SendFindableNPCs, sizeof(FindableNPC_Struct));

	FindableNPC_Struct *fnpcs = (FindableNPC_Struct *)outapp->pBuffer;

	fnpcs->Unknown109 = 0x16;
	fnpcs->Unknown110 = 0x06;
	fnpcs->Unknown111 = 0x24;

	fnpcs->Action = Remove ? 1: 0;

	fnpcs->EntityID = n->GetID();

	strn0cpy(fnpcs->Name, n->GetCleanName(), sizeof(fnpcs->Name));

	strn0cpy(fnpcs->LastName, n->GetLastName(), sizeof(fnpcs->LastName));

	fnpcs->Race = n->GetRace();

	fnpcs->Class = n->GetClass();

	LinkedListIterator<Client*> iterator(client_list); 

	iterator.Reset();

	while(iterator.MoreElements()) 
	{
		Client *c = iterator.GetData();
		if(c && (c->GetClientVersion() >= EQClientSoD))
			c->QueuePacket(outapp);
		
		iterator.Advance();
	}

	safe_delete(outapp);
}

void EntityList::HideCorpses(Client *c, uint8 CurrentMode, uint8 NewMode)
{
	if(!c)
		return;

	if(NewMode == HideCorpseNone)
	{
		SendZoneCorpses(c);
		return;
	}

	Group *g = NULL;

	if(NewMode == HideCorpseAllButGroup)
	{
		g = c->GetGroup();

		if(!g)
			NewMode = HideCorpseAll;
	}

	LinkedListIterator<Corpse*> iterator(corpse_list);

	iterator.Reset();

	while(iterator.MoreElements())
	{
		Corpse *b = iterator.GetData();

		if(b && (b->GetCharID() != c->CharacterID()))
		{
			if((NewMode == HideCorpseAll) || ((NewMode == HideCorpseNPC) && (b->IsNPCCorpse())))
			{
				EQApplicationPacket outapp;
		        	b->CreateDespawnPacket(&outapp, false);
				c->QueuePacket(&outapp);
			}
			else if(NewMode == HideCorpseAllButGroup)
			{
				if(!g->IsGroupMember(b->GetOwnerName()))
				{
					EQApplicationPacket outapp;
			        	b->CreateDespawnPacket(&outapp, false);
					c->QueuePacket(&outapp);
				}
				else if((CurrentMode == HideCorpseAll))
				{
					EQApplicationPacket outapp;
			        	b->CreateSpawnPacket(&outapp);
					c->QueuePacket(&outapp);
				}
			}
				
		}
		iterator.Advance();
	}
}

void EntityList::AddLootToNPCS(uint32 item_id, uint32 count)
{
	if(count == 0)
		return;

	int npc_count = 0;
	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(!iterator.GetData()->IsPet() 
			&& iterator.GetData()->GetClass() != LDON_TREASURE
			&& iterator.GetData()->GetBodyType() != BT_NoTarget 
			&& iterator.GetData()->GetBodyType() != BT_NoTarget2 
			&& iterator.GetData()->GetBodyType() != BT_Special)
		{
			npc_count++;
		}
		iterator.Advance();
	}

	if(npc_count == 0)
	{
		return;
	}

	NPC **npcs = new NPC*[npc_count];
	int *counts = new int[npc_count];
	bool *marked = new bool[npc_count];
	memset(counts, 0, sizeof(int) * npc_count);
	memset(marked, 0, sizeof(bool) * npc_count);

	int i = 0;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		if(!iterator.GetData()->IsPet()
			&& iterator.GetData()->GetClass() != LDON_TREASURE
			&& iterator.GetData()->GetBodyType() != BT_NoTarget 
			&& iterator.GetData()->GetBodyType() != BT_NoTarget2 
			&& iterator.GetData()->GetBodyType() != BT_Special)
		{
			npcs[i++] = iterator.GetData();
		}
		iterator.Advance();
	}

	while(count > 0)
	{
		vector<int> selection;
		selection.reserve(npc_count);
		for(int j = 0; j < npc_count; ++j)
		{
			selection.push_back(j);
		}

		while(selection.size() > 0 && count > 0)
		{
			int k = MakeRandomInt(0, selection.size() - 1);
			counts[selection[k]]++;
			count--;
			selection.erase(selection.begin() + k);
		}
	}

	for(int j = 0; j < npc_count; ++j)
	{
		if(counts[j] > 0)
		{
			for(int k = 0; k < counts[j]; ++k)
			{
				npcs[j]->AddItem(item_id, 1);
			}
		}
	}

	safe_delete_array(npcs);
	safe_delete_array(counts);
	safe_delete_array(marked);
}

void EntityList::CameraEffect(uint32 duration, uint32 intensity) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_CameraEffect, sizeof(Camera_Struct));
	memset(outapp->pBuffer, 0, sizeof(outapp->pBuffer));
	Camera_Struct* cs = (Camera_Struct*) outapp->pBuffer;
	cs->duration = duration;	// Duration in milliseconds
	cs->intensity = ((intensity * 6710886) + 1023410176);	// Intensity ranges from 1023410176 to 1090519040, so simplify it from 0 to 10.
	entity_list.QueueClients(0, outapp);
	safe_delete(outapp);
}


NPC* EntityList::GetClosestBanker(Mob* sender, uint32 &distance)
{
	if(!sender) 
		return NULL;

	distance = 4294967295u;
	NPC* nc = NULL;

	LinkedListIterator<NPC*> iterator(npc_list);
	iterator.Reset();

	while(iterator.MoreElements())
	{
		if(iterator.GetData()->GetClass() == BANKER)
		{
			uint32 nd = ((iterator.GetData()->GetY() - sender->GetY()) * (iterator.GetData()->GetY() - sender->GetY())) + 
				((iterator.GetData()->GetX() - sender->GetX()) * (iterator.GetData()->GetX() - sender->GetX()));
			if(nd < distance){
				distance = nd;
				nc = iterator.GetData();
			}
		}
		iterator.Advance();
	}
	return nc;
}

void EntityList::ExpeditionWarning(uint32 minutes_left)
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_DzExpeditionEndsWarning, sizeof(ExpeditionExpireWarning));
	ExpeditionExpireWarning *ew = (ExpeditionExpireWarning*)outapp->pBuffer;
	ew->minutes_remaining = minutes_left;

	LinkedListIterator<Client*> iterator(client_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		iterator.GetData()->Message_StringID(15, 3551, itoa((int)minutes_left));
		iterator.GetData()->QueuePacket(outapp);
		iterator.Advance();
	}
}

Mob* EntityList::GetClosestMobByBodyType(Mob* sender, bodyType BodyType)
{

	if(!sender) 
		return NULL;

	uint32 CurrentDistance, ClosestDistance = 4294967295u;

	Mob *CurrentMob, *ClosestMob = NULL;

	LinkedListIterator<Mob*> iterator(mob_list);

	iterator.Reset();

	while(iterator.MoreElements())
	{
		CurrentMob = iterator.GetData();

		iterator.Advance();
		
		if(CurrentMob->GetBodyType() != BodyType)
			continue;

		CurrentDistance = ((CurrentMob->GetY() - sender->GetY()) * (CurrentMob->GetY() - sender->GetY())) + 
				  ((CurrentMob->GetX() - sender->GetX()) * (CurrentMob->GetX() - sender->GetX()));

		if(CurrentDistance < ClosestDistance)
		{
			ClosestDistance = CurrentDistance;

			ClosestMob = CurrentMob;

		}
	}
	return ClosestMob;
}

void EntityList::GetTargetsForConeArea(Mob *start, uint32 radius, uint32 height, list<Mob*> &m_list)
{
	LinkedListIterator<Mob*> iterator(mob_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		Mob *ptr = iterator.GetData();
		if(ptr == start)
		{
			iterator.Advance();
			continue;
		}
		int32 x_diff = ptr->GetX() - start->GetX();
		int32 y_diff = ptr->GetY() - start->GetY();
		int32 z_diff = ptr->GetZ() - start->GetZ();

		x_diff *= x_diff;
		y_diff *= y_diff;
		z_diff *= z_diff;

		if((x_diff + y_diff) <= (radius * radius))
		{
			if(z_diff <= (height * height))
			{
				m_list.push_back(ptr);
			}
		}

		iterator.Advance();
	}
}

Client* EntityList::FindCorpseDragger(const char *CorpseName)
{
	LinkedListIterator<Client*> iterator(client_list); 
	
	iterator.Reset(); 

	while(iterator.MoreElements()) 
	{ 
		if (iterator.GetData()->IsDraggingCorpse(CorpseName))
		{
			return iterator.GetData();
		}
		iterator.Advance(); 
	} 
	return 0; 
}

Mob* EntityList::GetTargetForVirus(Mob* spreader)
{
	int max_spread_range = RuleI(Spells, VirusSpreadDistance);
	
	vector<Mob*> TargetsInRange;
	LinkedListIterator<Mob*> iterator(mob_list); 
	
	iterator.Reset(); 
	while(iterator.MoreElements())
	{
		// Make sure the target is in range, has los and is not the mob doing the spreading
		if ((iterator.GetData()->GetID() != spreader->GetID()) && 
			(iterator.GetData()->CalculateDistance(spreader->GetX(), spreader->GetY(), spreader->GetZ()) <= max_spread_range) &&
			(spreader->CheckLosFN(iterator.GetData())))
		{
			// If the spreader is an npc it can only spread to other npc controlled mobs
			if (spreader->IsNPC() && !spreader->IsPet() && iterator.GetData()->IsNPC()) {
				TargetsInRange.push_back(iterator.GetData());
			}
			// If the spreader is an npc controlled pet it can spread to any other npc or an npc controlled pet
			else if (spreader->IsNPC() && spreader->IsPet() && spreader->GetOwner()->IsNPC()) {
				if(iterator.GetData()->IsNPC() && !iterator.GetData()->IsPet()) {
					TargetsInRange.push_back(iterator.GetData());
				}
				else if(iterator.GetData()->IsNPC() && iterator.GetData()->IsPet() && iterator.GetData()->GetOwner()->IsNPC()) {
					TargetsInRange.push_back(iterator.GetData());
				}
			}
			// if the spreader is anything else(bot, pet, etc) then it should spread to everything but non client controlled npcs
			else if(!spreader->IsNPC() && !iterator.GetData()->IsNPC()) {
				TargetsInRange.push_back(iterator.GetData());
			}
			// if its a pet we need to determine appropriate targets(pet to client, pet to pet, pet to bot, etc)
			else if (spreader->IsNPC() && spreader->IsPet() && !spreader->GetOwner()->IsNPC()) {
				if(!iterator.GetData()->IsNPC()) {
					TargetsInRange.push_back(iterator.GetData());
				}
				else if (iterator.GetData()->IsNPC() && iterator.GetData()->IsPet() && !iterator.GetData()->GetOwner()->IsNPC()) {
					TargetsInRange.push_back(iterator.GetData());
				}
			}
		}
		iterator.Advance();
	}

	if(TargetsInRange.size() == 0)
		return NULL;

	return TargetsInRange[MakeRandomInt(0, TargetsInRange.size() - 1)];
}
