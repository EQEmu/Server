/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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

Assuming you want to add a new perl quest function named joe
that takes 1 integer argument....

1. Add the prototype to the quest manager:
questmgr.h: add (~line 50)
	void joe(int arg);

2. Define the actual function in questmgr.cpp:
void QuestManager::joe(int arg) {
	//... do something
}

3. Copy one of the XS routines in perlparser.cpp, preferably
 one with the same number of arguments as your routine. Rename
 as needed.
 Finally, add your routine to the list at the bottom of perlparser.cpp


4.
If you want it to work in old mode perl and .qst, edit parser.cpp
Parser::ExCommands (~line 777)
	else if (!strcmp(command,"joe")) {
		quest_manager.joe(atoi(arglist[0]));
	}

And then at then end of embparser.cpp, add:
"sub joe{push(@cmd_queue,{func=>'joe',args=>join(',',@_)});}"



*/

#include "../common/debug.h"
#include "entity.h"
#include "masterentity.h"
#include <limits.h>

#include <sstream>
#include <iostream>
#include <list>

#include "worldserver.h"
#include "net.h"
#include "../common/skills.h"
#include "../common/classes.h"
#include "../common/races.h"
#include "zonedb.h"
#include "../common/spdat.h"
#include "../common/packet_functions.h"
#include "../common/StringUtil.h"
#include "spawn2.h"
#include "zone.h"
#include "parser.h"
#include "event_codes.h"
#include "guild_mgr.h"
#include "../common/rulesys.h"
#include "QGlobals.h"
#include "QuestParserCollection.h"

#ifdef BOTS
#include "bot.h"
#endif


extern Zone* zone;
extern WorldServer worldserver;
extern EntityList entity_list;

#include "questmgr.h"

//declare our global instance
QuestManager quest_manager;

QuestManager::QuestManager() {
	depop_npc = false;
	HaveProximitySays = false;
}

QuestManager::~QuestManager() {
}

void QuestManager::Process() {
	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end, tmp;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->Timer_.Enabled() && cur->Timer_.Check()) {
			//make sure the mob is still in zone.
			if(entity_list.IsMobInZone(cur->mob)){
				if(cur->mob->IsNPC()) {
					parse->EventNPC(EVENT_TIMER, cur->mob->CastToNPC(), nullptr, cur->name, 0);
				}
				else {
					//this is inheriently unsafe if we ever make it so more than npc/client start timers
					parse->EventPlayer(EVENT_TIMER, cur->mob->CastToClient(), cur->name, 0);
				}

				//we MUST reset our iterator since the quest could have removed/added any
				//number of timers... worst case we have to check a bunch of timers twice
				cur = QTimerList.begin();
				end = QTimerList.end();	//dunno if this is needed, cant hurt...
			} else {
				tmp = cur;
				tmp++;
				QTimerList.erase(cur);
				cur = tmp;
			}
		} else
			cur++;
	}

	std::list<SignalTimer>::iterator curS, endS, tmpS;

	curS = STimerList.begin();
	endS = STimerList.end();
	while (curS != endS) {
		if(!curS->Timer_.Enabled()) {
			//remove the timer
			tmpS = curS;
			tmpS++;
			STimerList.erase(curS);
			curS = tmpS;
		} else if(curS->Timer_.Check()) {
			//disable the timer so it gets deleted.
			curS->Timer_.Disable();

			//signal the event...
			entity_list.SignalMobsByNPCID(curS->npc_id, curS->signal_id);

			//restart for the same reasons as above.
			curS = STimerList.begin();
			endS = STimerList.end();
		} else
			curS++;
	}
}

void QuestManager::StartQuest(Mob *_owner, Client *_initiator, ItemInst* _questitem) {
	quest_mutex.lock();
	owner = _owner;
	initiator = _initiator;
	questitem = _questitem;
	depop_npc = false;
}

void QuestManager::EndQuest() {
	quest_mutex.unlock();

	if(depop_npc && owner->IsNPC()) {
		//clear out any timers for them...
		std::list<QuestTimer>::iterator cur = QTimerList.begin(), end, tmp;

		end = QTimerList.end();
		while (cur != end) {
			if(cur->mob == owner) {
				tmp = cur;
				tmp++;
				QTimerList.erase(cur);
				cur = tmp;
			} else {
				cur++;
			}
		}

		owner->Depop();
		owner = nullptr;	//just to be safe
	}
}

void QuestManager::ClearAllTimers() {

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end, tmp;

	end = QTimerList.end();
	while (cur != end)
	{
		tmp = cur;
		tmp++;
		QTimerList.erase(cur);
		cur = tmp;
	}
}

//quest perl functions
void QuestManager::echo(int colour, const char *str) {
	entity_list.MessageClose(initiator, false, 200, colour, str);
}

void QuestManager::say(const char *str) {
	if (!owner) {
		LogFile->write(EQEMuLog::Quest, "QuestManager::say called with nullptr owner. Probably syntax error in quest file.");
		return;
	}
	else {
		if(RuleB(NPC, EnableNPCQuestJournal) && initiator) {
			owner->QuestJournalledSay(initiator, str);
		}
		else {
			owner->Say(str);
		}
	}
}

void QuestManager::say(const char *str, uint8 language) {
	if (!owner) {
		LogFile->write(EQEMuLog::Quest, "QuestManager::say called with nullptr owner. Probably syntax error in quest file.");
		return;
	}
	else {
		entity_list.ChannelMessage(owner, 8, language, str);
	}
}

void QuestManager::me(const char *str) {
	if (!initiator)
		return;
	entity_list.MessageClose(initiator, false, 200, 10, str);
}

void QuestManager::summonitem(uint32 itemid, int16 charges) {
	if(!initiator)
		return;
	initiator->SummonItem(itemid, charges);
}

void QuestManager::write(const char *file, const char *str) {
	FILE * pFile;
	pFile = fopen (file, "a");
	if(!pFile)
		return;
	fprintf(pFile, "%s\n", str);
	fclose (pFile);
}

uint16 QuestManager::spawn2(int npc_type, int grid, int unused, float x, float y, float z, float heading) {
	const NPCType* tmp = 0;
	if (tmp = database.GetNPCType(npc_type))
	{
		NPC* npc = new NPC(tmp, 0, x, y, z, heading, FlyMode3);
		npc->AddLootTable();
		entity_list.AddNPC(npc,true,true);
		// Sleep in main thread? ICK!
		// Sleep(200);
		// check is irrelevent, it's impossible for npc to be 0 here
		// (we're in main thread, nothing else can possibly modify it)
		if(grid > 0)
		{
			npc->AssignWaypoints(grid);
		}
		npc->SendPosUpdate();
		return(npc->GetID());
	}
	return(0);
}

uint16 QuestManager::unique_spawn(int npc_type, int grid, int unused, float x, float y, float z, float heading) {
	Mob *other = entity_list.GetMobByNpcTypeID(npc_type);
	if(other != nullptr) {
		return(other->GetID());
	}

	const NPCType* tmp = 0;
	if (tmp = database.GetNPCType(npc_type))
	{
		NPC* npc = new NPC(tmp, 0, x, y, z, heading, FlyMode3);
		npc->AddLootTable();
		entity_list.AddNPC(npc,true,true);
		// Sleep in main thread? ICK!
		// Sleep(200);
		// check is irrelevent, it's impossible for npc to be 0 here
		// (we're in main thread, nothing else can possibly modify it)
		if(grid > 0)
		{
			npc->AssignWaypoints(grid);
		}
		npc->SendPosUpdate();
		return(npc->GetID());
	}
	return(0);
}

uint16 QuestManager::spawn_from_spawn2(uint32 spawn2_id)
{
	LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
	iterator.Reset();
	Spawn2 *found_spawn = nullptr;

	while(iterator.MoreElements())
	{
		Spawn2* cur = iterator.GetData();
		iterator.Advance();
		if(cur->GetID() == spawn2_id)
		{
			found_spawn = cur;
			break;
		}
	}

	if(found_spawn)
	{
		SpawnGroup* sg = zone->spawn_group_list.GetSpawnGroup(found_spawn->SpawnGroupID());
		if(!sg)
		{
			database.LoadSpawnGroupsByID(found_spawn->SpawnGroupID(),&zone->spawn_group_list);
			sg = zone->spawn_group_list.GetSpawnGroup(found_spawn->SpawnGroupID());
			if(!sg)
			{
				return 0;
			}
		}
		uint32 npcid = sg->GetNPCType();
		if(npcid == 0)
		{
			return 0;
		}

		const NPCType* tmp = database.GetNPCType(npcid);
		if(!tmp)
		{
			return 0;
		}

		if(tmp->unique_spawn_by_name)
		{
			if(!entity_list.LimitCheckName(tmp->name))
			{
				return 0;
			}
		}

		if(tmp->spawn_limit > 0)
		{
			if(!entity_list.LimitCheckType(npcid, tmp->spawn_limit))
			{
				return 0;
			}
		}

		database.UpdateSpawn2Timeleft(spawn2_id, zone->GetInstanceID(), 0);
		found_spawn->SetCurrentNPCID(npcid);

		NPC* npc = new NPC(tmp, found_spawn, found_spawn->GetX(), found_spawn->GetY(), found_spawn->GetZ(),
			found_spawn->GetHeading(), FlyMode3);

		found_spawn->SetNPCPointer(npc);
		npc->AddLootTable();
		npc->SetSp2(found_spawn->SpawnGroupID());
		entity_list.AddNPC(npc);
		entity_list.LimitAddNPC(npc);

		if(sg->roamdist && sg->roambox[0] && sg->roambox[1] && sg->roambox[2] && sg->roambox[3] && sg->delay)
			npc->AI_SetRoambox(sg->roamdist,sg->roambox[0],sg->roambox[1],sg->roambox[2],sg->roambox[3],sg->delay);
		if(zone->InstantGrids())
		{
			found_spawn->LoadGrid();
		}

		return npc->GetID();
	}
	return 0;
}

void QuestManager::enable_spawn2(uint32 spawn2_id)
{
	database.UpdateSpawn2Status(spawn2_id, 1);
	ServerPacket* pack = new ServerPacket(ServerOP_SpawnStatusChange, sizeof(ServerSpawnStatusChange_Struct));
	ServerSpawnStatusChange_Struct* ssc = (ServerSpawnStatusChange_Struct*) pack->pBuffer;
	ssc->id = spawn2_id;
	ssc->new_status = 1;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::disable_spawn2(uint32 spawn2_id)
{
	database.UpdateSpawn2Status(spawn2_id, 0);
	ServerPacket* pack = new ServerPacket(ServerOP_SpawnStatusChange, sizeof(ServerSpawnStatusChange_Struct));
	ServerSpawnStatusChange_Struct* ssc = (ServerSpawnStatusChange_Struct*) pack->pBuffer;
	ssc->id = spawn2_id;
	ssc->new_status = 0;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::setstat(int stat, int value) {
	if (initiator)
		initiator->SetStats(stat, value);
}

void QuestManager::incstat(int stat, int value) { //old setstat command aza
	if (initiator)
		initiator->IncStats(stat, value);
}

void QuestManager::castspell(int spell_id, int target_id) {
	if (owner) {
		Mob *tgt = entity_list.GetMob(target_id);
		if(tgt != nullptr)
			owner->SpellFinished(spell_id, tgt, 10, 0, -1, spells[spell_id].ResistDiff);
	}
}

void QuestManager::selfcast(int spell_id) {
	if (initiator)
		initiator->SpellFinished(spell_id, initiator, 10, 0, -1, spells[spell_id].ResistDiff);
}

void QuestManager::addloot(int item_id, int charges, bool equipitem) {
	if(item_id != 0){
		if(owner->IsNPC())
			owner->CastToNPC()->AddItem(item_id, charges, equipitem);
	}
}

void QuestManager::Zone(const char *zone_name) {
	if (initiator && initiator->IsClient())
	{
		ServerPacket* pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
		ZoneToZone_Struct* ztz = (ZoneToZone_Struct*) pack->pBuffer;
		ztz->response = 0;
		ztz->current_zone_id = zone->GetZoneID();
		ztz->current_instance_id = zone->GetInstanceID();
		ztz->requested_zone_id = database.GetZoneID(zone_name);
		ztz->admin = initiator->Admin();
		strcpy(ztz->name, initiator->GetName());
		ztz->guild_id = initiator->GuildID();
		ztz->ignorerestrictions = 3;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::settimer(const char *timer_name, int seconds) {

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->mob == owner && cur->name == timer_name) {
			cur->mob = owner;
			cur->Timer_.Enable();
			cur->Timer_.Start(seconds * 1000, false);
			printf("Resetting: %s for %d seconds\n", cur->name.c_str(), seconds);
			return;
		}
		cur++;
	}

	QTimerList.push_back(QuestTimer(seconds * 1000, owner, timer_name));
}

void QuestManager::settimerMS(const char *timer_name, int milliseconds) {

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->mob == owner && cur->name == timer_name) {
			cur->mob = owner;
			cur->Timer_.Enable();
			cur->Timer_.Start(milliseconds, false);
			printf("Resetting: %s for %d seconds\n", cur->name.c_str(), milliseconds);
			return;
		}
		cur++;
	}

	QTimerList.push_back(QuestTimer(milliseconds, owner, timer_name));
}

void QuestManager::stoptimer(const char *timer_name) {

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end)
	{
		if(cur->mob == owner && cur->name == timer_name)
		{
			QTimerList.erase(cur);
			return;
		}
		cur++;
	}
}

void QuestManager::stopalltimers() {

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end, tmp;

	end = QTimerList.end();
	while (cur != end)
	{
		if(cur->mob == owner)
		{
			tmp = cur;
			tmp++;
			QTimerList.erase(cur);
			cur = tmp;
		}
		else
		{
			cur++;
		}
	}
}

void QuestManager::emote(const char *str) {
	if (!owner) {
		LogFile->write(EQEMuLog::Quest, "QuestManager::emote called with nullptr owner. Probably syntax error in quest file.");
		return;
	}
	else {
		owner->Emote(str);
	}
}

void QuestManager::shout(const char *str) {
	if (!owner) {
		LogFile->write(EQEMuLog::Quest, "QuestManager::shout called with nullptr owner. Probably syntax error in quest file.");
		return;
	}
	else {
		owner->Shout(str);
	}
}

void QuestManager::shout2(const char *str) {
	if (!owner) {
		LogFile->write(EQEMuLog::Quest, "QuestManager::shout2 called with nullptr owner. Probably syntax error in quest file.");
		return;
	}
	else {
		worldserver.SendEmoteMessage(0,0,0,13, "%s shouts, '%s'", owner->GetCleanName(), str);
	}
}

void QuestManager::gmsay(const char *str, uint32 color, bool send_to_world, uint32 to_guilddbid, uint32 to_minstatus) {
	if(send_to_world)
		worldserver.SendEmoteMessage(0, to_guilddbid, to_minstatus, color, "%s", str);
	else
		entity_list.MessageStatus(to_guilddbid, to_minstatus, color, "%s", str);
}

void QuestManager::depop(int npc_type) { // depop NPC and don't start spawn timer
	if (!owner || !owner->IsNPC()) {
		LogFile->write(EQEMuLog::Quest, "QuestManager::depop called with nullptr owner or non-NPC owner. Probably syntax error in quest file.");
		return;
	}
	else {
		if (npc_type != 0) {
			Mob * tmp = entity_list.GetMobByNpcTypeID(npc_type);
			if (tmp) {
				if (tmp != owner) {
					tmp->CastToNPC()->Depop();
				}
				else {
					depop_npc = true;
				}
			}
		}
		else {	//depop self
			depop_npc = true;
		}
	}
}

void QuestManager::depop_withtimer(int npc_type) { // depop NPC and start spawn timer
	if (!owner || !owner->IsNPC()) {
		LogFile->write(EQEMuLog::Quest, "QuestManager::depop_withtimer called with nullptr owner or non-NPC owner. Probably syntax error in quest file.");
		return;
	}
	else {
		if (npc_type != 0) {
			Mob * tmp = entity_list.GetMobByNpcTypeID(npc_type);
			if (tmp) {
				if (tmp != owner) {
					tmp->CastToNPC()->Depop(true);
				}
				else {
					owner->Depop(true);
				}
			}
		}
		else {	//depop self
			owner->Depop(true);
		}
	}
}

void QuestManager::depopall(int npc_type) {
	if(owner && owner->IsNPC() && (npc_type > 0)) {
		entity_list.DepopAll(npc_type);
	}
	else {
		LogFile->write(EQEMuLog::Quest, "QuestManager::depopall called with nullptr owner, non-NPC owner, or invalid NPC Type ID. Probably syntax error in quest file.");
	}
}

void QuestManager::depopzone(bool StartSpawnTimer) {
	if(zone) {
		zone->Depop(StartSpawnTimer);
	}
	else {
		LogFile->write(EQEMuLog::Quest, "QuestManager::depopzone called with nullptr zone. Probably syntax error in quest file.");
	}
}

void QuestManager::repopzone() {
	if(zone) {
		zone->Repop();
	}
	else {
		LogFile->write(EQEMuLog::Quest, "QuestManager::repopzone called with nullptr zone. Probably syntax error in quest file.");
	}
}

void QuestManager::settarget(const char *type, int target_id) {
	if(!owner->IsNPC())
		return;
	Mob* tmp = nullptr;
	if (!strcasecmp(type,"npctype")) {
		tmp = entity_list.GetMobByNpcTypeID(target_id);
	}
	else if (!strcasecmp(type, "entity")) {
		tmp = entity_list.GetMob(target_id);
	}
	if(tmp != nullptr) {
		owner->SetTarget(tmp);
	}
}

void QuestManager::follow(int entity_id, int distance) {
	if(!owner->IsNPC())
		return;
	owner->SetFollowID(entity_id);
	owner->SetFollowDistance(distance * distance);
}

void QuestManager::sfollow() {
	if(!owner->IsNPC())
		return;
	owner->SetFollowID(0);
}

void QuestManager::changedeity(int diety_id) {
	//Changes the deity.
	if(initiator)
	{
		if(initiator->IsClient())
		{
			initiator->SetDeity(diety_id);
			initiator->Message(15,"Your Deity has been changed/set to: %i", diety_id);
			initiator->Save(1);
			initiator->Kick();
		}
		else
		{
			initiator->Message(15,"Error changing Deity");
		}
	}
}

void QuestManager::exp(int amt) {
	if (initiator && initiator->IsClient())
		initiator->AddEXP(amt);
}

void QuestManager::level(int newlevel) {
	if (initiator && initiator->IsClient())
		initiator->SetLevel(newlevel, true);
}

void QuestManager::traindisc(int discipline_tome_item_id) {
	if (initiator && initiator->IsClient())
		initiator->TrainDiscipline(discipline_tome_item_id);
}

bool QuestManager::isdisctome(int item_id) {
//get the item info
	const Item_Struct *item = database.GetItem(item_id);
	if(item == nullptr) {
		return(false);
	}

	if(item->ItemClass != ItemClassCommon || item->ItemType != ItemTypeSpell) {
		return(false);
	}

	//Need a way to determine the difference between a spell and a tome
	//so they cant turn in a spell and get it as a discipline
	//this is kinda a hack:
	if(!(
		item->Name[0] == 'T' &&
		item->Name[1] == 'o' &&
		item->Name[2] == 'm' &&
		item->Name[3] == 'e' &&
		item->Name[4] == ' '
		) && !(
		item->Name[0] == 'S' &&
		item->Name[1] == 'k' &&
		item->Name[2] == 'i' &&
		item->Name[3] == 'l' &&
		item->Name[4] == 'l' &&
		item->Name[5] == ':' &&
		item->Name[6] == ' '
		)) {
		return(false);
	}

	//we know for sure none of the int casters get disciplines
	uint32 cbit = 0;
	cbit |= 1 << (WIZARD-1);
	cbit |= 1 << (ENCHANTER-1);
	cbit |= 1 << (MAGICIAN-1);
	cbit |= 1 << (NECROMANCER-1);
	if(item->Classes & cbit) {
		return(false);
	}

	uint32 spell_id = item->Scroll.Effect;
	if(!IsValidSpell(spell_id)) {
		return(false);
	}

	//we know for sure none of the int casters get disciplines
	const SPDat_Spell_Struct &spell = spells[spell_id];
	if(
		spell.classes[WIZARD - 1] != 255 &&
		spell.classes[ENCHANTER - 1] != 255 &&
		spell.classes[MAGICIAN - 1] != 255 &&
		spell.classes[NECROMANCER - 1] != 255
	) {
		return(false);
	}

	return(true);
}

void QuestManager::safemove() {
	if (initiator && initiator->IsClient())
		initiator->GoToSafeCoords(zone->GetZoneID(), 0);
}

void QuestManager::rain(int weather) {
	zone->zone_weather = weather;
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Weather, 8);
	*((uint32*) &outapp->pBuffer[4]) = (uint32) weather; // Why not just use 0x01/2/3?
	entity_list.QueueClients(owner, outapp);
	safe_delete(outapp);
}

void QuestManager::snow(int weather) {
	zone->zone_weather = weather + 1;
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Weather, 8);
	outapp->pBuffer[0] = 0x01;
	*((uint32*) &outapp->pBuffer[4]) = (uint32)weather;
	entity_list.QueueClients(initiator, outapp);
	safe_delete(outapp);
}

void QuestManager::surname(const char *name) {
	//Changes the last name.
	if(initiator)
	{
		if(initiator->IsClient())
		{
			initiator->ChangeLastName(name);
			initiator->Message(15,"Your surname has been changed/set to: %s", name);
		}
		else
		{
			initiator->Message(15,"Error changing/setting surname");
		}
	}
}

void QuestManager::permaclass(int class_id) {
	//Makes the client the class specified
	initiator->SetBaseClass(class_id);
	initiator->Save(2);
	initiator->Kick();
}

void QuestManager::permarace(int race_id) {
	//Makes the client the race specified
	initiator->SetBaseRace(race_id);
	initiator->Save(2);
	initiator->Kick();
}

void QuestManager::permagender(int gender_id) {
	//Makes the client the gender specified
	initiator->SetBaseGender(gender_id);
	initiator->Save(2);
	initiator->Kick();
}

uint16 QuestManager::scribespells(uint8 max_level, uint8 min_level) {
	uint16 book_slot, count;
	uint16 curspell;

	uint16 Char_ID = initiator->CharacterID();
	bool SpellGlobalRule = RuleB(Spells, EnableSpellGlobals);
	bool SpellGlobalCheckResult = 0;


	for(curspell = 0, book_slot = initiator->GetNextAvailableSpellBookSlot(), count = 0; curspell < SPDAT_RECORDS && book_slot < MAX_PP_SPELLBOOK; curspell++, book_slot = initiator->GetNextAvailableSpellBookSlot(book_slot))
	{
		if
		(
			spells[curspell].classes[WARRIOR] != 0 &&       //check if spell exists
			spells[curspell].classes[initiator->GetPP().class_-1] <= max_level &&   //maximum level
			spells[curspell].classes[initiator->GetPP().class_-1] >= min_level &&   //minimum level
			spells[curspell].skill != 52 &&
			( !RuleB(Spells, UseCHAScribeHack) || spells[curspell].effectid[EFFECT_COUNT - 1] != 10 )
		)
		{
			if (book_slot == -1) //no more book slots
				break;
			if(!IsDiscipline(curspell) && !initiator->HasSpellScribed(curspell)) { //isn't a discipline & we don't already have it scribed
				if (SpellGlobalRule) {
					// Bool to see if the character has the required QGlobal to scribe it if one exists in the Spell_Globals table
					SpellGlobalCheckResult = initiator->SpellGlobalCheck(curspell, Char_ID);
					if (SpellGlobalCheckResult) {
						initiator->ScribeSpell(curspell, book_slot);
						count++;
					}
				}
				else {
					initiator->ScribeSpell(curspell, book_slot);
					count++;
				}
			}
		}
	}
	return count; //how many spells were scribed successfully
}

uint16 QuestManager::traindiscs(uint8 max_level, uint8 min_level) {
	uint16 count;
	uint16 curspell;

	uint16 Char_ID = initiator->CharacterID();
	bool SpellGlobalRule = RuleB(Spells, EnableSpellGlobals);
	bool SpellGlobalCheckResult = 0;

	for(curspell = 0, count = 0; curspell < SPDAT_RECORDS; curspell++)
	{
		if
		(
			spells[curspell].classes[WARRIOR] != 0 &&	//check if spell exists
			spells[curspell].classes[initiator->GetPP().class_-1] <= max_level &&	//maximum level
			spells[curspell].classes[initiator->GetPP().class_-1] >= min_level &&	//minimum level
			spells[curspell].skill != 52 &&
			( !RuleB(Spells, UseCHAScribeHack) || spells[curspell].effectid[EFFECT_COUNT - 1] != 10 )
		)
		{
			if(IsDiscipline(curspell)){
				//we may want to come up with a function like Client::GetNextAvailableSpellBookSlot() to help speed this up a little
				for(uint32 r = 0; r < MAX_PP_DISCIPLINES; r++) {
					if(initiator->GetPP().disciplines.values[r] == curspell) {
						initiator->Message(13, "You already know this discipline.");
						break;	//continue the 1st loop
					}
					else if(initiator->GetPP().disciplines.values[r] == 0) {
						if (SpellGlobalRule) {
							// Bool to see if the character has the required QGlobal to train it if one exists in the Spell_Globals table
							SpellGlobalCheckResult = initiator->SpellGlobalCheck(curspell, Char_ID);
							if (SpellGlobalCheckResult) {
								initiator->GetPP().disciplines.values[r] = curspell;
								initiator->SendDisciplineUpdate();
								initiator->Message(0, "You have learned a new discipline!");
								count++;	//success counter
							}
							break;	//continue the 1st loop
						}
						else {
						initiator->GetPP().disciplines.values[r] = curspell;
						initiator->SendDisciplineUpdate();
						initiator->Message(0, "You have learned a new discipline!");
						count++;	//success counter
						break;	//continue the 1st loop
						}
					}	//if we get to this point, there's already a discipline in this slot, so we skip it
				}
			}
		}
	}
	return count;	//how many disciplines were learned successfully
}

void QuestManager::unscribespells() {
	initiator->UnscribeSpellAll();
	}

void QuestManager::untraindiscs() {
	initiator->UntrainDiscAll();
}

void QuestManager::givecash(int copper, int silver, int gold, int platinum) {
	if (initiator && initiator->IsClient() && ((copper + silver + gold + platinum) > 0))
	{
		initiator->AddMoneyToPP(copper, silver, gold, platinum, true);

		std::string tmp;
		if (platinum > 0)
		{
			tmp = "You receive ";
			tmp += itoa(platinum);
			tmp += " platinum";
		}
		if (gold > 0)
		{
			if (tmp.length() == 0)
				tmp = "You receive ";
			else
				tmp += ",";

			tmp += itoa(gold);
			tmp += " gold";
		}
		if(silver > 0)
		{
			if (tmp.length() == 0)
				tmp = "You receive ";
			else
				tmp += ",";

			tmp += itoa(silver);
			tmp += " silver";
		}
		if(copper > 0)
		{
			if (tmp.length() == 0)
				tmp = "You receive ";
			else
				tmp += ",";

			tmp += itoa(copper);
			tmp += " copper";
		}
		tmp += " pieces.";
		if (initiator)
			initiator->Message(MT_OOC, tmp.c_str());
	}
}

void QuestManager::pvp(const char *mode) {
	if (!strcasecmp(mode,"on"))
	{
		if (initiator)
			initiator->SetPVP(true);
	}
	else
		if (initiator)
			initiator->SetPVP(false);
}

void QuestManager::movepc(int zone_id, float x, float y, float z, float heading) {
	if (initiator && initiator->IsClient())
		initiator->MovePC(zone_id, x, y, z, heading);
}

void QuestManager::gmmove(float x, float y, float z) {
	if (initiator && initiator->IsClient())
		initiator->GMMove(x, y, z);
}

void QuestManager::movegrp(int zoneid, float x, float y, float z) {
	if (initiator && initiator->IsClient())
	{
		Group *g = entity_list.GetGroupByClient(initiator);
		if (g != nullptr) {
			g->TeleportGroup(owner, zoneid, 0, x, y, z, 0.0f);
		}
		else {
			Raid *r = entity_list.GetRaidByClient(initiator);
			if (r != nullptr) {
				uint32 gid = r->GetGroup(initiator);
				if (gid >= 0 && gid < 12) {
					r->TeleportGroup(owner, zoneid, 0, x, y, z, 0.0f, gid);
				}
				else {
					initiator->MovePC(zoneid, x, y, z, 0.0f);
				}
			}
			else {
				initiator->MovePC(zoneid, x, y, z, 0.0f);
			}
		}
	}
}

void QuestManager::doanim(int anim_id) {
	owner->DoAnim(anim_id);
}

void QuestManager::addskill(int skill_id, int value) {
	if(skill_id < 0 || skill_id > HIGHEST_SKILL)	//must check before casting.
		return;
	if (initiator && initiator->IsClient())
		initiator->AddSkill((SkillType) skill_id, value);
}

void QuestManager::setlanguage(int skill_id, int value) {
	if (initiator && initiator->IsClient())
		initiator->SetLanguageSkill(skill_id, value);
}

void QuestManager::setskill(int skill_id, int value) {
	if(skill_id < 0 || skill_id > HIGHEST_SKILL)	//must check before casting.
		return;
	if (initiator && initiator->IsClient())
		initiator->SetSkill((SkillType) skill_id, value);
}

void QuestManager::setallskill(int value) {
	if (!initiator)
		return;
	if (initiator && initiator->IsClient()) {
		SkillType sk;
		for (sk = _1H_BLUNT; sk <= HIGHEST_SKILL; sk = (SkillType)(sk+1)) {
			initiator->SetSkill(sk, value);
		}
	}
}

void QuestManager::attack(const char *client_name) {
	if(!owner->IsNPC())
		return;
	Client* getclient = entity_list.GetClientByName(client_name);
	if(getclient && owner->IsAttackAllowed(getclient)) {
		owner->AddToHateList(getclient,1);
	} else {
		owner->Say("I am unable to attack %s.", client_name);
	}
}

void QuestManager::attacknpc(int npc_entity_id) {
	if(!owner->IsNPC())
		return;
	Mob *it = entity_list.GetMob(npc_entity_id);
	if(it && owner->IsAttackAllowed(it)) {
		owner->AddToHateList(it,1);
	} else {
		if(it)
			owner->Say("I am unable to attack %s.", it->GetName());
		else
			owner->Say("I am unable to locate NPC entity %i", npc_entity_id);
	}
}

void QuestManager::attacknpctype(int npc_type_id) {
	if(!owner->IsNPC())
		return;
	Mob *it = entity_list.GetMobByNpcTypeID(npc_type_id);
	if(it && owner->IsAttackAllowed(it)) {
		owner->AddToHateList(it,1);
	} else {
		if(it)
			owner->Say("I am unable to attack %s.", it->GetName());
		else
			owner->Say("I am unable to locate NPC type %i", npc_type_id);
	}
}

void QuestManager::save() {
	if (initiator && initiator->IsClient())
		initiator->Save();
}

void QuestManager::faction(int faction_id, int faction_value, int temp) {
	if (initiator && initiator->IsClient()) {
		if(faction_id != 0 && faction_value != 0) {
	// fixed faction command
			//Client *p;
			initiator->SetFactionLevel2(
				initiator->CharacterID(),
				faction_id,
				initiator->GetBaseClass(),
				initiator->GetBaseRace(),
				initiator->GetDeity(),
				faction_value,
				temp);

		}
	}
}

void QuestManager::setsky(uint8 new_sky) {
	if (zone)
		zone->newzone_data.sky = new_sky;
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(initiator, outapp);
	safe_delete(outapp);
}

void QuestManager::setguild(uint32 new_guild_id, uint8 new_rank) {
	if (initiator && initiator->IsClient()) {
		guild_mgr.SetGuild(initiator->CharacterID(), new_guild_id, new_rank);
	}
}

void QuestManager::CreateGuild(const char *guild_name, const char *leader) {
	uint32 cid = database.GetCharacterID(leader);
	char hString[250];
			if (cid == 0) {
				worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", "Guild Creation: Guild leader not found.");
				return;
			}

			uint32 tmp = guild_mgr.FindGuildByLeader(cid);
			if (tmp != GUILD_NONE) {
				sprintf(hString, "Guild Creation: Error: %s already is the leader of DB# %i '%s'.", leader, tmp, guild_mgr.GetGuildName(tmp));
				worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", hString);
			}
			else {
				uint32 gid = guild_mgr.CreateGuild(guild_name, cid);
				if (gid == GUILD_NONE)
					worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", "Guild Creation: Guild creation failed");
				else {
					sprintf(hString, "Guild Creation: Guild created: Leader: %i, number %i: %s", cid, gid, leader);
					worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", hString);
					if(!guild_mgr.SetGuild(cid, gid, GUILD_LEADER))
						worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", "Unable to set guild leader's guild in the database. Your going to have to run #guild set");
				}

			}
}

void QuestManager::settime(uint8 new_hour, uint8 new_min) {
	if (zone)
		zone->SetTime(new_hour + 1, new_min);
}

void QuestManager::itemlink(int item_id) {
	const ItemInst* inst = database.CreateItem(item_id);
	char* link = 0;
	if (initiator->MakeItemLink(link, inst))
		initiator->Message(0, "%s tells you, %c%s%s%c", owner->GetCleanName(), 0x12, link, inst->GetItem()->Name, 0x12);
	safe_delete_array(link);
	safe_delete(inst);
}

void QuestManager::signalwith(int npc_id, int signal_id, int wait_ms) {
// signal command
	// signal(npcid) - generates EVENT_SIGNAL on specified npc
	if(wait_ms > 0) {
		STimerList.push_back(SignalTimer(wait_ms, npc_id, signal_id));
		return;
	}

	if (npc_id<1)
	{
		printf("signal() bad npcid=%i\n",npc_id);
	}
	else
	{
		//initiator* signalnpc=0;
		entity_list.SignalMobsByNPCID(npc_id, signal_id);
	}
}

void QuestManager::signal(int npc_id, int wait_ms) {
	signalwith(npc_id, 0, wait_ms);
}

void QuestManager::setglobal(const char *varname, const char *newvalue, int options, const char *duration) {
// qglobal variable commands
	// setglobal(varname,value,options,duration)
	//MYSQL_ROW row;
	int qgZoneid=zone->GetZoneID();
	int qgCharid=0;
	int qgNpcid = owner->GetNPCTypeID();

	/*	options value determines the availability of global variables to NPCs when a quest begins
	------------------------------------------------------------------
		value		npcid		player		zone
	------------------------------------------------------------------
		0			this		this		this
		1			all			this		this
		2			this		all			this
		3			all			all			this
		4			this		this		all
		5			all			this		all
		6			this		all			all
		7			all			all			all
	*/
	if (initiator && initiator->IsClient()) // some events like waypoint and spawn don't have a player involved
	{
		qgCharid=initiator->CharacterID();
	}

	else
	{
		qgCharid=-qgNpcid;		// make char id negative npc id as a fudge
	}
	if (options < 0 || options > 7)
	{
		std::cerr << "Invalid options for global var " << varname << " using defaults" << std::endl;
	}	// default = 0 (only this npcid,player and zone)
	else
	{
		if (options & 1)
			qgNpcid=0;
		if (options & 2)
			qgCharid=0;
		if (options & 4)
			qgZoneid=0;
	}

	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, newvalue, QGVarDuration(duration));
}

/* Inserts global variable into quest_globals table */
int QuestManager::InsertQuestGlobal(
									int charid, int npcid, int zoneid,
									const char *varname, const char *varvalue,
									int duration)
{
	char *query = 0;
	char errbuf[MYSQL_ERRMSG_SIZE];

	// Make duration string either "unix_timestamp(now()) + xxx" or "NULL"
	std::stringstream duration_ss;
	if (duration == INT_MAX)
	{
		duration_ss << "NULL";
	}
	else
	{
		duration_ss << "unix_timestamp(now()) + " << duration;
	}

	//NOTE: this should be escaping the contents of arglist
	//npcwise a malicious script can arbitrarily alter the DB
	uint32 last_id = 0;
	if (!database.RunQuery(query, MakeAnyLenString(&query,
		"REPLACE INTO quest_globals (charid, npcid, zoneid, name, value, expdate)"
		"VALUES (%i, %i, %i, '%s', '%s', %s)",
		charid, npcid, zoneid, varname, varvalue, duration_ss.str().c_str()
		), errbuf, nullptr, nullptr, &last_id))
	{
		std::cerr << "setglobal error inserting " << varname << " : " << errbuf << std::endl;
	}
	safe_delete_array(query);

	if(zone)
	{
		//first delete our global
		ServerPacket* pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
		ServerQGlobalDelete_Struct *qgd = (ServerQGlobalDelete_Struct*)pack->pBuffer;
		qgd->npc_id = npcid;
		qgd->char_id = charid;
		qgd->zone_id = zoneid;
		qgd->from_zone_id = zone->GetZoneID();
		qgd->from_instance_id = zone->GetInstanceID();
		strcpy(qgd->name, varname);

		entity_list.DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
		zone->DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		//then create a new one with the new id
		pack = new ServerPacket(ServerOP_QGlobalUpdate, sizeof(ServerQGlobalUpdate_Struct));
		ServerQGlobalUpdate_Struct *qgu = (ServerQGlobalUpdate_Struct*)pack->pBuffer;
		qgu->npc_id = npcid;
		qgu->char_id = charid;
		qgu->zone_id = zoneid;
		if(duration == INT_MAX)
		{
			qgu->expdate = 0xFFFFFFFF;
		}
		else
		{
			qgu->expdate = Timer::GetTimeSeconds() + duration;
		}
		strcpy((char*)qgu->name, varname);
		strn0cpy((char*)qgu->value, varvalue, 128);
		qgu->id = last_id;
		qgu->from_zone_id = zone->GetZoneID();
		qgu->from_instance_id = zone->GetInstanceID();

		QGlobal temp;
		temp.npc_id = npcid;
		temp.char_id = charid;
		temp.zone_id = zoneid;
		temp.expdate = qgu->expdate;
		temp.name.assign(qgu->name);
		temp.value.assign(qgu->value);
		entity_list.UpdateQGlobal(qgu->id, temp);
		zone->UpdateQGlobal(qgu->id, temp);

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	return 0;
}

void QuestManager::targlobal(const char *varname, const char *value, const char *duration, int qgNpcid, int qgCharid, int qgZoneid)
{
	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, value, QGVarDuration(duration));
}

void QuestManager::delglobal(const char *varname) {
	// delglobal(varname)
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int qgZoneid=zone->GetZoneID();
	int qgCharid=0;
	int qgNpcid=owner->GetNPCTypeID();
	if (initiator && initiator->IsClient()) // some events like waypoint and spawn don't have a player involved
	{
		qgCharid=initiator->CharacterID();
	}

	else
	{
		qgCharid=-qgNpcid;		// make char id negative npc id as a fudge
	}
	if (!database.RunQuery(query,
		MakeAnyLenString(&query,
		"DELETE FROM quest_globals WHERE name='%s'"
		" && (npcid=0 || npcid=%i) && (charid=0 || charid=%i) && (zoneid=%i || zoneid=0)",
		varname,qgNpcid,qgCharid,qgZoneid),errbuf))
	{
		std::cerr << "delglobal error deleting " << varname << " : " << errbuf << std::endl;
	}
	safe_delete_array(query);

	if(zone)
	{
		ServerPacket* pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
		ServerQGlobalDelete_Struct *qgu = (ServerQGlobalDelete_Struct*)pack->pBuffer;

		qgu->npc_id = qgNpcid;
		qgu->char_id = qgCharid;
		qgu->zone_id = qgZoneid;
		strcpy(qgu->name, varname);

		entity_list.DeleteQGlobal(std::string((char*)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);
		zone->DeleteQGlobal(std::string((char*)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

// Converts duration string to duration value (in seconds)
// Return of INT_MAX indicates infinite duration
int QuestManager::QGVarDuration(const char *fmt)
{
	int duration = 0;

	// format:	Y#### or D## or H## or M## or S## or T###### or C#######

	int len = strlen(fmt);

	// Default to no duration
	if (len < 1)
		return 0;

	// Set val to value after type character
	// e.g., for "M3924", set to 3924
	int val = atoi(&fmt[0] + 1);

	switch (fmt[0])
	{
		// Forever
		case 'F':
		case 'f':
			duration = INT_MAX;
			break;
		// Years
		case 'Y':
		case 'y':
			duration = val * 31556926;
			break;
		case 'D':
		case 'd':
			duration = val * 86400;
			break;
		// Hours
		case 'H':
		case 'h':
			duration = val * 3600;
			break;
		// Minutes
		case 'M':
		case 'm':
			duration = val * 60;
			break;
		// Seconds
		case 'S':
		case 's':
			duration = val;
			break;
		// Invalid
		default:
			duration = 0;
			break;
	}

	return duration;
}

void QuestManager::ding() {
	//makes a sound.
	if (initiator && initiator->IsClient())
		initiator->SendSound();

}

void QuestManager::rebind(int zoneid, float x, float y, float z) {
	if(initiator && initiator->IsClient()) {
		initiator->SetBindPoint(zoneid, x, y, z);
	}
}

void QuestManager::start(int32 wp) {
	if(!owner->IsNPC())
		return;
	owner->CastToNPC()->AssignWaypoints(wp);
}

void QuestManager::stop() {
	if(!owner->IsNPC())
		return;
	owner->CastToNPC()->StopWandering();
}

void QuestManager::pause(int duration) {
	if(!owner->IsNPC())
		return;
	owner->CastToNPC()->PauseWandering(duration);
}

void QuestManager::moveto(float x, float y, float z, float h, bool saveguardspot) {
	if(!owner->IsNPC())
		return;
	owner->CastToNPC()->MoveTo(x, y, z, h, saveguardspot);
}

void QuestManager::resume() {
	if(!owner->IsNPC())
		return;
	owner->CastToNPC()->ResumeWandering();
}

void QuestManager::addldonpoints(int32 points, uint32 theme) {
	if(initiator)
		initiator->UpdateLDoNPoints(points, theme);
}

void QuestManager::addldonwin(int32 wins, uint32 theme) {
	if(initiator)
		initiator->UpdateLDoNWins(theme, wins);
}

void QuestManager::addldonloss(int32 losses, uint32 theme) {
	if(initiator)
		initiator->UpdateLDoNLosses(theme, losses);
}

void QuestManager::setnexthpevent(int at) {
	owner->SetNextHPEvent( at );
}

void QuestManager::setnextinchpevent(int at) {
	owner->SetNextIncHPEvent( at );
}

void QuestManager::respawn(int npc_type, int grid) {
	if(!owner->IsNPC())
		return;
	//char tempa[100];
	float x,y,z,h;
	if ( !owner )
		return;

	x = owner->GetX();
	y = owner->GetY();
	z = owner->GetZ();
	h = owner->GetHeading();
	depop_npc = true;

	const NPCType* tmp = 0;
	if ((tmp = database.GetNPCType(npc_type)))
	{
		owner = new NPC(tmp, 0, x, y, z, h, FlyMode3);
		owner->CastToNPC()->AddLootTable();
		entity_list.AddNPC(owner->CastToNPC(),true,true);
		if(grid > 0)
			owner->CastToNPC()->AssignWaypoints(grid);

		owner->SendPosUpdate();
	}
}

void QuestManager::set_proximity(float minx, float maxx, float miny, float maxy, float minz, float maxz) {
	if(!owner->IsNPC())
		return;

	entity_list.AddProximity(owner->CastToNPC());

	owner->CastToNPC()->proximity->min_x = minx;
	owner->CastToNPC()->proximity->max_x = maxx;
	owner->CastToNPC()->proximity->min_y = miny;
	owner->CastToNPC()->proximity->max_y = maxy;
	owner->CastToNPC()->proximity->min_z = minz;
	owner->CastToNPC()->proximity->max_z = maxz;

	owner->CastToNPC()->proximity->say = parse->HasQuestSub(owner->CastToNPC()->GetNPCTypeID(),"EVENT_PROXIMITY_SAY");

	if(owner->CastToNPC()->proximity->say)
		HaveProximitySays = true;
}

void QuestManager::clear_proximity() {
	if(!owner->IsNPC())
		return;
	entity_list.RemoveProximity(owner->GetID());
	safe_delete(owner->CastToNPC()->proximity);
}

void QuestManager::setanim(int npc_type, int animnum) {
	//adds appearance changes
	Mob* thenpc = entity_list.GetMobByNpcTypeID(npc_type);
	if(animnum < 0 || animnum >= _eaMaxAppearance)
		return;
	thenpc->SetAppearance(EmuAppearance(animnum));
}


//displays an in game path based on a waypoint grid
void QuestManager::showgrid(int grid) {
	if(initiator == nullptr)
		return;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	FindPerson_Point pt;
	std::vector<FindPerson_Point> pts;

	pt.x = initiator->GetX();
	pt.y = initiator->GetY();
	pt.z = initiator->GetZ();
	pts.push_back(pt);

	// Retrieve all waypoints for this grid
	if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT `x`,`y`,`z` FROM grid_entries WHERE `gridid`=%i AND `zoneid`=%i ORDER BY `number`",grid,zone->GetZoneID()),errbuf,&result))
	{
		while((row = mysql_fetch_row(result)))
		{
			pt.x = atof(row[0]);
			pt.y = atof(row[1]);
			pt.z = atof(row[2]);
			pts.push_back(pt);
		}
		mysql_free_result(result);

		initiator->SendPathPacket(pts);
	}
	else	// DB query error!
	{
		LogFile->write(EQEMuLog::Quest, "Error loading grid %d for showgrid(): %s", grid, errbuf);
		return;
	}
	safe_delete_array(query);
}

//displays an in game path based on path finding.
void QuestManager::showpath(float x, float y, float z) {
	say("showpath not implemented yet.");
}

//causes the npc to use path finding to walk to x,y,z
void QuestManager::pathto(float x, float y, float z) {
	say("pathto not implemented yet.");
}

//change the value of a spawn condition
void QuestManager::spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id, short new_value) {
	zone->spawn_conditions.SetCondition(zone_short, instance_id, condition_id, new_value);
}

//get the value of a spawn condition
short QuestManager::get_spawn_condition(const char *zone_short, uint32 instance_id, uint16 condition_id) {
	return(zone->spawn_conditions.GetCondition(zone_short, instance_id, condition_id));
}

//toggle a spawn event
void QuestManager::toggle_spawn_event(int event_id, bool enable, bool reset_base) {
	zone->spawn_conditions.ToggleEvent(event_id, enable, reset_base);
}

bool QuestManager::has_zone_flag(int zone_id) {
	return(initiator->HasZoneFlag(zone_id));
}

void QuestManager::set_zone_flag(int zone_id) {
	initiator->SetZoneFlag(zone_id);
}

void QuestManager::clear_zone_flag(int zone_id) {
	initiator->ClearZoneFlag(zone_id);
}

void QuestManager::sethp(int hpperc) {
	int newhp;

	newhp = (owner->GetMaxHP()*(100-hpperc))/100;
	owner->Damage(owner, newhp, SPELL_UNKNOWN, HAND_TO_HAND, false, 0, false);
}

bool QuestManager::summonburriedplayercorpse(uint32 char_id, float dest_x, float dest_y, float dest_z, float dest_heading) {
	bool Result = false;

	if(char_id > 0) {
		Corpse* PlayerCorpse = database.SummonBurriedPlayerCorpse(char_id, zone->GetZoneID(), zone->GetInstanceID(), dest_x, dest_y, dest_z, dest_heading);
		if(PlayerCorpse) {
			Result = true;
		}
	}
	return Result;
}

bool QuestManager::summonallplayercorpses(uint32 char_id, float dest_x, float dest_y, float dest_z, float dest_heading) {
	bool Result = false;

	if(char_id > 0) {
		Client* c = entity_list.GetClientByCharID(char_id);
		c->SummonAllCorpses(dest_x, dest_y, dest_z, dest_heading);
		Result = true;
	}
	return Result;
}

uint32 QuestManager::getplayerburriedcorpsecount(uint32 char_id) {
	uint32 Result = 0;

	if(char_id > 0) {
		Result = database.GetPlayerBurriedCorpseCount(char_id);
	}
	return Result;
}

bool QuestManager::buryplayercorpse(uint32 char_id)
{
	bool Result = false;

	if(char_id > 0)
	{
		uint32 PlayerCorpse = database.GetFirstCorpseID(char_id);
		if(PlayerCorpse > 0)
		{
			database.BuryPlayerCorpse(PlayerCorpse);
			Corpse* corpse = entity_list.GetCorpseByDBID(PlayerCorpse);
			if(corpse)
			{
				corpse->Save();
				corpse->DepopCorpse();
			}
			else
			{
				Client *c = entity_list.GetClientByCharID(char_id);
				c->DepopPlayerCorpse(PlayerCorpse);
			}
			Result = true;
		}
	}
	return Result;
}

void QuestManager::forcedooropen(uint32 doorid, bool altmode) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		if(GetInitiator())
			d->ForceOpen(GetInitiator(), altmode);
		else if(GetOwner())
			d->ForceOpen(GetOwner(), altmode);
	}
}

void QuestManager::forcedoorclose(uint32 doorid, bool altmode) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		if(GetInitiator())
			d->ForceClose(GetInitiator(), altmode);
		else if(GetOwner())
			d->ForceClose(GetOwner(), altmode);
	}
}

void QuestManager::toggledoorstate(uint32 doorid) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		if(GetInitiator())
			d->ToggleState(GetInitiator());
		else if(GetOwner())
			d->ToggleState(GetOwner());
	}
}

bool QuestManager::isdooropen(uint32 doorid) {
	Doors* d = entity_list.FindDoor(doorid);
	if(d){
		return d->IsDoorOpen();
	}
	return false;
}
void QuestManager::npcrace(int race_id)
{
	owner->SendIllusionPacket(race_id);
}

void QuestManager::npcgender(int gender_id)
{
	owner->SendIllusionPacket(owner->GetRace(), gender_id);
}
void QuestManager::npcsize(int newsize)
{
	owner->ChangeSize(newsize, true);
}
void QuestManager::npctexture(int newtexture)
{
	owner->SendIllusionPacket(owner->GetRace(), 0xFF, newtexture);
}

void QuestManager::playerrace(int race_id)
{
	initiator->SendIllusionPacket(race_id);
}

void QuestManager::playergender(int gender_id)
{
	initiator->SendIllusionPacket(initiator->GetRace(), gender_id);
}

void QuestManager::playersize(int newsize)
{
	initiator->ChangeSize(newsize, true);
}

void QuestManager::playertexture(int newtexture)
{
	initiator->SendIllusionPacket(initiator->GetRace(), 0xFF, newtexture);
}

void QuestManager::playerfeature(char *feature, int setting)
{
	uint16 Race = initiator->GetRace();
	uint8 Gender = initiator->GetGender();
	uint8 Texture = 0xFF;
	uint8 HelmTexture = 0xFF;
	uint8 HairColor = initiator->GetHairColor();
	uint8 BeardColor = initiator->GetBeardColor();
	uint8 EyeColor1 = initiator->GetEyeColor1();
	uint8 EyeColor2 = initiator->GetEyeColor2();
	uint8 HairStyle = initiator->GetHairStyle();
	uint8 LuclinFace = initiator->GetLuclinFace();
	uint8 Beard = initiator->GetBeard();
	uint32 DrakkinHeritage = initiator->GetDrakkinHeritage();
	uint32 DrakkinTattoo = initiator->GetDrakkinTattoo();
	uint32 DrakkinDetails = initiator->GetDrakkinDetails();
	float Size = initiator->GetSize();

	if (!strcasecmp(feature,"race"))
		Race = setting;
	else if (!strcasecmp(feature,"gender"))
		Gender = setting;
	else if (!strcasecmp(feature,"texture"))
		Texture = setting;
	else if (!strcasecmp(feature,"helm"))
		HelmTexture = setting;
	else if (!strcasecmp(feature,"haircolor"))
		HairColor = setting;
	else if (!strcasecmp(feature,"beardcolor"))
		BeardColor = setting;
	else if (!strcasecmp(feature,"eyecolor1"))
		EyeColor1 = setting;
	else if (!strcasecmp(feature,"eyecolor2"))
		EyeColor2 = setting;
	else if (!strcasecmp(feature,"hair"))
		HairStyle = setting;
	else if (!strcasecmp(feature,"face"))
		LuclinFace = setting;
	else if (!strcasecmp(feature,"beard"))
		Beard = setting;
	else if (!strcasecmp(feature,"heritage"))
		DrakkinHeritage = setting;
	else if (!strcasecmp(feature,"tattoo"))
		DrakkinTattoo = setting;
	else if (!strcasecmp(feature,"details"))
		DrakkinDetails = setting;
	else if (!strcasecmp(feature,"size"))
		Size = (float)setting / 10;	//dividing by 10 to allow 1 decimal place for adjusting size
	else
		return;

	initiator->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
										EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
										DrakkinHeritage, DrakkinTattoo, DrakkinDetails, Size);
}

void QuestManager::npcfeature(char *feature, int setting)
{
	uint16 Race = owner->GetRace();
	uint8 Gender = owner->GetGender();
	uint8 Texture = 0xFF;
	uint8 HelmTexture = 0xFF;
	uint8 HairColor = owner->GetHairColor();
	uint8 BeardColor = owner->GetBeardColor();
	uint8 EyeColor1 = owner->GetEyeColor1();
	uint8 EyeColor2 = owner->GetEyeColor2();
	uint8 HairStyle = owner->GetHairStyle();
	uint8 LuclinFace = owner->GetLuclinFace();
	uint8 Beard = owner->GetBeard();
	uint32 DrakkinHeritage = owner->GetDrakkinHeritage();
	uint32 DrakkinTattoo = owner->GetDrakkinTattoo();
	uint32 DrakkinDetails = owner->GetDrakkinDetails();
	float Size = owner->GetSize();

	if (!strcasecmp(feature,"race"))
		Race = setting;
	else if (!strcasecmp(feature,"gender"))
		Gender = setting;
	else if (!strcasecmp(feature,"texture"))
		Texture = setting;
	else if (!strcasecmp(feature,"helm"))
		HelmTexture = setting;
	else if (!strcasecmp(feature,"haircolor"))
		HairColor = setting;
	else if (!strcasecmp(feature,"beardcolor"))
		BeardColor = setting;
	else if (!strcasecmp(feature,"eyecolor1"))
		EyeColor1 = setting;
	else if (!strcasecmp(feature,"eyecolor2"))
		EyeColor2 = setting;
	else if (!strcasecmp(feature,"hair"))
		HairStyle = setting;
	else if (!strcasecmp(feature,"face"))
		LuclinFace = setting;
	else if (!strcasecmp(feature,"beard"))
		Beard = setting;
	else if (!strcasecmp(feature,"heritage"))
		DrakkinHeritage = setting;
	else if (!strcasecmp(feature,"tattoo"))
		DrakkinTattoo = setting;
	else if (!strcasecmp(feature,"details"))
		DrakkinDetails = setting;
	else if (!strcasecmp(feature,"size"))
		Size = (float)setting / 10;	//dividing by 10 to allow 1 decimal place for adjusting size
	else
		return;

	owner->SendIllusionPacket(Race, Gender, Texture, HelmTexture, HairColor, BeardColor,
										EyeColor1, EyeColor2, HairStyle, LuclinFace, Beard, 0xFF,
										DrakkinHeritage, DrakkinTattoo, DrakkinDetails, Size);
}

void QuestManager::popup(char *title, char *text, uint32 popupid, uint32 buttons, uint32 Duration)
{
	if(initiator)
		initiator->SendPopupToClient(title, text, popupid, buttons, Duration);
}

#ifdef BOTS

int QuestManager::createbotcount() {
	return RuleI(Bots, CreateBotCount);
}

int QuestManager::spawnbotcount() {
	return RuleI(Bots, SpawnBotCount);
}

bool QuestManager::botquest()
{
	return RuleB(Bots, BotQuest);
}

bool QuestManager::createBot(const char *name, const char *lastname, uint8 level, uint16 race, uint8 botclass, uint8 gender)
{
	std::string TempErrorMessage;
	uint32 MaxBotCreate = RuleI(Bots, CreateBotCount);

	if (initiator && initiator->IsClient())
	{
		if(Bot::SpawnedBotCount(initiator->CharacterID(), &TempErrorMessage) >= MaxBotCreate)
		{
			initiator->Message(15,"You have the maximum number of bots allowed.");
			return false;
		}

		if(!TempErrorMessage.empty())
		{
			initiator->Message(13, "Database Error: %s", TempErrorMessage.c_str());
			return false;
		}

		NPCType DefaultNPCTypeStruct = Bot::CreateDefaultNPCTypeStructForBot(name, lastname, level, race, botclass, gender);
		Bot* NewBot = new Bot(DefaultNPCTypeStruct, initiator);

		if(NewBot)
		{
			if(!NewBot->IsValidRaceClassCombo()) {
				initiator->Message(0, "That Race/Class combination cannot be created.");
				return false;
			}

			if(!NewBot->IsValidName()) {
				initiator->Message(0, "%s has invalid characters. You can use only the A-Z, a-z and _ characters in a bot name.", NewBot->GetCleanName());
				return false;
			}

			if(!NewBot->IsBotNameAvailable(&TempErrorMessage)) {
				initiator->Message(0, "The name %s is already being used. Please choose a different name.", NewBot->GetCleanName());
				return false;
			}

			if(!TempErrorMessage.empty()) {
				initiator->Message(13, "Database Error: %s", TempErrorMessage.c_str());
				return false;
			}

			// Now that all validation is complete, we can save our newly created bot
			if(!NewBot->Save())
			{
				initiator->Message(0, "Unable to save %s as a bot.", NewBot->GetCleanName());
			}
			else
			{
				initiator->Message(0, "%s saved as bot %u.", NewBot->GetCleanName(), NewBot->GetBotID());
				return true;
			}
		}
	}
	return false;
}

#endif //BOTS

void QuestManager::taskselector(int taskcount, int *tasks) {
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && taskmanager)
		taskmanager->SendTaskSelector(initiator, owner, taskcount, tasks);
}
void QuestManager::enabletask(int taskcount, int *tasks) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && taskmanager)
		initiator->EnableTask(taskcount, tasks);
}

void QuestManager::disabletask(int taskcount, int *tasks) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && taskmanager)
		initiator->DisableTask(taskcount, tasks);
}

bool QuestManager::istaskenabled(int taskid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && taskmanager)
		return initiator->IsTaskEnabled(taskid);

	return false;
}

void QuestManager::tasksetselector(int tasksetid) {
	_log(TASKS__UPDATE, "TaskSetSelector called for task set %i", tasksetid);
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && taskmanager)
		initiator->TaskSetSelector(owner, tasksetid);
}

bool QuestManager::istaskactive(int task) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskActive(task);

	return false;
}
bool QuestManager::istaskactivityactive(int task, int activity) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskActivityActive(task, activity);

	return false;
}
int QuestManager::gettaskactivitydonecount(int task, int activity) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->GetTaskActivityDoneCountFromTaskID(task, activity);

	return 0; //improper args

}
void QuestManager::updatetaskactivity(int task, int activity, int count) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->UpdateTaskActivity(task, activity, count);
}

void QuestManager::resettaskactivity(int task, int activity) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->ResetTaskActivity(task, activity);
}

void QuestManager::taskexploredarea(int exploreid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->UpdateTasksOnExplore(exploreid);
}

void QuestManager::assigntask(int taskid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		initiator->AssignTask(taskid, owner->GetID());
}

void QuestManager::failtask(int taskid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->FailTask(taskid);
}

int QuestManager::tasktimeleft(int taskid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->TaskTimeLeft(taskid);

	return -1;
}

int QuestManager::enabledtaskcount(int taskset) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->EnabledTaskCount(taskset);

	return -1;
}
int QuestManager::firsttaskinset(int taskset) {

	if(RuleB(TaskSystem, EnableTaskSystem) && taskmanager)
		return taskmanager->FirstTaskInSet(taskset);

	return -1;
}
int QuestManager::lasttaskinset(int taskset) {

	if(RuleB(TaskSystem, EnableTaskSystem) && taskmanager)
		return taskmanager->LastTaskInSet(taskset);

	return -1;
}
int QuestManager::nexttaskinset(int taskset, int taskid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && taskmanager)
		return taskmanager->NextTaskInSet(taskset, taskid);

	return -1;
}
int QuestManager::activespeaktask() {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		return initiator->ActiveSpeakTask(owner->GetNPCTypeID());
	return 0;
}
int QuestManager::activespeakactivity(int taskid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		return initiator->ActiveSpeakActivity(owner->GetNPCTypeID(), taskid);

	return 0;
}
int QuestManager::istaskcompleted(int taskid) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskCompleted(taskid);

	return -1;
}
int QuestManager::activetasksinset(int taskset) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->ActiveTasksInSet(taskset);

	return -1;
}
int QuestManager::completedtasksinset(int taskset) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->CompletedTasksInSet(taskset);

	return -1;
}

bool QuestManager::istaskappropriate(int task) {

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && taskmanager)
		return taskmanager->AppropriateLevel(task, initiator->GetLevel());

	return false;
}
void QuestManager::clearspawntimers() {
	if(zone) {
		//TODO: Dec 19, 2008, replace with code updated for current spawn timers.
		LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
		iterator.Reset();
		while (iterator.MoreElements())
		{
			char errbuf[MYSQL_ERRMSG_SIZE];
			char *query = 0;
			database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM respawn_times WHERE id=%lu AND "
				"instance_id=%lu",(unsigned long)iterator.GetData()->GetID(), (unsigned long)zone->GetInstanceID()), errbuf);
			safe_delete_array(query);
			iterator.Advance();
		}
	}
}
void QuestManager::ze(int type, const char *str) {
	entity_list.Message(0, type, str);
}

void QuestManager::we(int type, const char *str) {
	worldserver.SendEmoteMessage(0, 0, type, str);
}

int QuestManager::getlevel(uint8 type)
{
	if (type == 0)
	{
		return (initiator->GetLevel());
	}
	else if(type == 1)
	{
		Group *g = entity_list.GetGroupByClient(initiator);
		if (g != nullptr)
			return (g->GetAvgLevel());
		else
			return 0;
	}
	else if(type == 2)
	{
		Raid *r = entity_list.GetRaidByClient(initiator);
		if (r != nullptr)
			return (r->GetAvgLevel());
		else
			return 0;
	}
	else if(type == 3)
	{
		Raid *r = entity_list.GetRaidByClient(initiator);
		if(r != nullptr)
		{
			return (r->GetAvgLevel());
		}
		Group *g = entity_list.GetGroupByClient(initiator);
		if(g != nullptr)
		{
			return (g->GetAvgLevel());
		}
		else
			return (initiator->GetLevel());
	}
	else if(type == 4 && initiator->IsClient())
	{
		return (initiator->CastToClient()->GetLevel2());
	}
	else
		return 0;
}

uint16 QuestManager::CreateGroundObject(uint32 itemid, float x, float y, float z, float heading, uint32 decay_time)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateGroundObject(itemid, x, y, z, heading, decay_time);
	return entid;
}

uint16 QuestManager::CreateGroundObjectFromModel(const char *model, float x, float y, float z, float heading, uint8 type, uint32 decay_time)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateGroundObjectFromModel(model, x, y, z, heading, type, decay_time);
	return entid;
}

void QuestManager::ModifyNPCStat(const char *identifier, const char *newValue)
{
	if(owner){
		if(owner->IsNPC())
		{
			owner->CastToNPC()->ModifyNPCStat(identifier, newValue);
		}
	}
}

int QuestManager::collectitems_processSlot(int16 slot_id, uint32 item_id,
	bool remove)
{
	ItemInst *item;
	int quantity = 0;

	item = initiator->GetInv().GetItem(slot_id);

	// If we have found matching item, add quantity
	if (item && item->GetID() == item_id)
	{
		// If item is stackable, add its charges (quantity)
		if (item->IsStackable())
		{
			quantity = item->GetCharges();
		}
		else
		{
			quantity = 1;
		}

		// Remove item from inventory
		if (remove)
		{
			initiator->DeleteItemInInventory(slot_id, 0, true);
		}
	}

	return quantity;
}

// Returns number of item_id that exist in inventory
// If remove is true, items are removed as they are counted.
int QuestManager::collectitems(uint32 item_id, bool remove)
{
	int quantity = 0;
	int slot_id;

	for (slot_id = 22; slot_id <= 29; ++slot_id)
	{
		quantity += collectitems_processSlot(slot_id, item_id, remove);
	}

	for (slot_id = 251; slot_id <= 330; ++slot_id)
	{
		quantity += collectitems_processSlot(slot_id, item_id, remove);
	}

	return quantity;
}

void QuestManager::UpdateSpawnTimer(uint32 id, uint32 newTime)
{
	bool found = false;

	database.UpdateSpawn2Timeleft(id, 0, (newTime/1000));
	LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
	iterator.Reset();
	while (iterator.MoreElements())
	{
		if(iterator.GetData()->GetID() == id)
		{
			if(!iterator.GetData()->NPCPointerValid())
			{
				iterator.GetData()->SetTimer(newTime);
			}
			found = true;
			break;
		}
		iterator.Advance();
	}

	if(!found)
	{
		//Spawn wasn't in this zone...
		//Tell the other zones to update their spawn time for this spawn point
		ServerPacket *pack = new ServerPacket(ServerOP_UpdateSpawn, sizeof(UpdateSpawnTimer_Struct));
		UpdateSpawnTimer_Struct *ust = (UpdateSpawnTimer_Struct*)pack->pBuffer;
		ust->id = id;
		ust->duration = newTime;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

// used to set the number of an item in the selected merchant's temp item list. Defaults to zero if no quantity is specified.
void QuestManager::MerchantSetItem(uint32 NPCid, uint32 itemid, uint32 quantity) {
	Mob* merchant = entity_list.GetMobByNpcTypeID(NPCid);

	if (merchant == 0 || !merchant->IsNPC() || (merchant->GetClass() != MERCHANT))
		return;	// don't do anything if NPCid isn't a merchant

	const Item_Struct* item = nullptr;
	item = database.GetItem(itemid);
	if (!item) return;		// if the item id doesn't correspond to a real item, do nothing

	zone->SaveTempItem(merchant->CastToNPC()->MerchantType, NPCid, itemid, quantity);
}

uint32 QuestManager::MerchantCountItem(uint32 NPCid, uint32 itemid) {
	Mob* merchant = entity_list.GetMobByNpcTypeID(NPCid);

	if (merchant == 0 || !merchant->IsNPC() || (merchant->GetClass() != MERCHANT))
		return 0;	// if it isn't a merchant, it doesn't have any items

	const Item_Struct* item = nullptr;
	item = database.GetItem(itemid);
	if (!item) return 0;		// likewise, if it isn't a valid item, the merchant doesn't have any

	// look for the item in the merchant's temporary list
	std::list<TempMerchantList> MerchList = zone->tmpmerchanttable[NPCid];
	std::list<TempMerchantList>::const_iterator itr;
	TempMerchantList ml;
	uint32 Quant = 0;

	for(itr = MerchList.begin(); itr != MerchList.end(); itr++){
		ml = *itr;
		if (ml.item == itemid) {	// if this is the item we're looking for
			Quant = ml.charges;
			break;
		}
	}

	return Quant;	// return the quantity of itemid (0 if it was never found)
}

// Item Link for use in Variables - "my $example_link = quest::varlink(item_id);"
const char* QuestManager::varlink(char* perltext, int item_id) {
	const ItemInst* inst = database.CreateItem(item_id);
	if (!inst)
		return "INVALID ITEM ID IN VARLINK";
	char* link = 0;
	char* tempstr = 0;
	if (initiator->MakeItemLink(link, inst)) {	// make a link to the item
		MakeAnyLenString(&tempstr, "%c%s%s%c", 0x12, link, inst->GetItem()->Name, 0x12);
		strn0cpy(perltext, tempstr,250);	// the perl string is only 250 chars, so make sure the link isn't too large
		safe_delete_array(tempstr);	// MakeAnyLenString() uses new, so clean up after it
	}
	safe_delete_array(link);	// MakeItemLink() uses new also
	safe_delete(inst);
	return perltext;
}

uint16 QuestManager::CreateInstance(const char *zone, int16 version, uint32 duration)
{
	if(initiator)
	{
		uint32 zone_id = database.GetZoneID(zone);
		if(zone_id == 0)
			return 0;

		uint16 id = 0;
		if(!database.GetUnusedInstanceID(id))
		{
			initiator->Message(13, "Server was unable to find a free instance id.");
			return 0;
		}

		if(!database.CreateInstance(id, zone_id, version, duration))
		{
			initiator->Message(13, "Server was unable to create a new instance.");
			return 0;
		}
		return id;
	}
	return 0;
}

void QuestManager::DestroyInstance(uint16 instance_id)
{
	database.DeleteInstance(instance_id);
}

uint16 QuestManager::GetInstanceID(const char *zone, int16 version)
{
	if(initiator)
	{
		return database.GetInstanceID(zone, initiator->CharacterID(), version);
	}
	return 0;
}

void QuestManager::AssignToInstance(uint16 instance_id)
{
	if(initiator)
	{
		database.AddClientToInstance(instance_id, initiator->CharacterID());
	}
}

void QuestManager::AssignGroupToInstance(uint16 instance_id)
{
	if(initiator)
	{
		Group *g = initiator->GetGroup();
		if(g)
		{
			uint32 gid = g->GetID();
			database.AssignGroupToInstance(gid, instance_id);
		}
	}
}

void QuestManager::AssignRaidToInstance(uint16 instance_id)
{
	if(initiator)
	{
		Raid *r = initiator->GetRaid();
		if(r)
		{
			uint32 rid = r->GetID();
			database.AssignRaidToInstance(rid, instance_id);
		}
	}
}

void QuestManager::MovePCInstance(int zone_id, int instance_id, float x, float y, float z, float heading)
{
	if(initiator)
	{
		initiator->MovePC(zone_id, instance_id, x, y, z, heading);
	}
}

void QuestManager::FlagInstanceByGroupLeader(uint32 zone, int16 version)
{
	if(initiator)
	{
		Group *g = initiator->GetGroup();
		if(g){
			database.FlagInstanceByGroupLeader(zone, version, initiator->CharacterID(), g->GetID());
		}
	}
}

void QuestManager::FlagInstanceByRaidLeader(uint32 zone, int16 version)
{
	if(initiator)
	{
		Raid *r = initiator->GetRaid();
		if(r)
		{
			database.FlagInstanceByRaidLeader(zone, version, initiator->CharacterID(), r->GetID());
		}
	}
}

const char* QuestManager::saylink(char* Phrase, bool silent, char* LinkName) {

	const char *ERR_MYSQLERROR = "Error in saylink phrase queries";
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int sayid = 0;

	int sz = strlen(Phrase);
	char *escaped_string = new char[sz * 2];
	database.DoEscapeString(escaped_string, Phrase, sz);

	// Query for an existing phrase and id in the saylink table
	if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT `id` FROM `saylink` WHERE `phrase` = '%s'", escaped_string),errbuf,&result))
	{
		if (mysql_num_rows(result) >= 1)
		{
			while((row = mysql_fetch_row(result)))
			{
				sayid = atoi(row[0]);
			}
			mysql_free_result(result);
		}
		else // Add a new saylink entry to the database and query it again for the new sayid number
		{
			safe_delete_array(query);

			database.RunQuery(query,MakeAnyLenString(&query,"INSERT INTO `saylink` (`phrase`) VALUES ('%s')", escaped_string),errbuf);
			safe_delete_array(query);

			if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT `id` FROM saylink WHERE `phrase` = '%s'", escaped_string),errbuf,&result))
			{
				if (mysql_num_rows(result) >= 1)
				{
					while((row = mysql_fetch_row(result)))
					{
						sayid = atoi(row[0]);
					}
					mysql_free_result(result);
				}
			}
			else
			{
				LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
			}
			safe_delete_array(query);
		}
	}
	safe_delete_array(query);
	safe_delete_array(escaped_string);

	if(silent)
		sayid = sayid + 750000;
	else
		sayid = sayid + 500000;

		//Create the say link as an item link hash
		char linktext[250];

	if(initiator)
	{
		if (initiator->GetClientVersion() >= EQClientRoF)
		{
			sprintf(linktext,"%c%06X%s%s%c",0x12,sayid,"0000000000000000000000000000000000000000000000000",LinkName,0x12);
		}
		else if (initiator->GetClientVersion() >= EQClientSoF)
		{
			sprintf(linktext,"%c%06X%s%s%c",0x12,sayid,"00000000000000000000000000000000000000000000",LinkName,0x12);
		}
		else
		{
			sprintf(linktext,"%c%06X%s%s%c",0x12,sayid,"000000000000000000000000000000000000000",LinkName,0x12);
		}
	}
	else {	// If no initiator, create an RoF saylink, since older clients handle RoF ones better than RoF handles older ones.
		sprintf(linktext,"%c%06X%s%s%c",0x12,sayid,"0000000000000000000000000000000000000000000000000",LinkName,0x12);
	}
	strcpy(Phrase,linktext);
	return Phrase;

}

const char* QuestManager::getguildnamebyid(int guild_id) {
	if (guild_id > 0)
		return guild_mgr.GetGuildName(guild_id);
	else
		return("");
}

void QuestManager::SetRunning(bool val)
{
	if(!owner)
		return;
	owner->SetRunning(val);
}

bool QuestManager::IsRunning()
{
	if(!owner)
		return false;
	return owner->IsRunning();
}

void QuestManager::FlyMode(uint8 flymode)
{
	if(initiator)
	{
		if (flymode >= 0 && flymode < 3) {
			initiator->SendAppearancePacket(AT_Levitate, flymode);
			return;
		}
	}
	if(owner)
	{
		if (flymode >= 0 && flymode < 3) {
			owner->SendAppearancePacket(AT_Levitate, flymode);
			return;
		}
	}
}

uint8 QuestManager::FactionValue()
{
	FACTION_VALUE oldfac;
	uint8 newfac = 0;
	if(initiator && owner->IsNPC()) {
		oldfac = initiator->GetFactionLevel(initiator->GetID(), owner->GetID(), initiator->GetRace(), initiator->GetClass(), initiator->GetDeity(), owner->GetPrimaryFaction(), owner);

		// now, reorder the faction to have it make sense (higher values are better)
		switch (oldfac) {
			case FACTION_SCOWLS:
				newfac = 1;
				break;
			case FACTION_THREATENLY:
				newfac = 2;
				break;
			case FACTION_DUBIOUS:
				newfac = 3;
				break;
			case FACTION_APPREHENSIVE:
				newfac = 4;
				break;
			case FACTION_INDIFFERENT:
				newfac = 5;
				break;
			case FACTION_AMIABLE:
				newfac = 6;
				break;
			case FACTION_KINDLY:
				newfac = 7;
				break;
			case FACTION_WARMLY:
				newfac = 8;
				break;
			case FACTION_ALLY:
				newfac = 9;
				break;
		}
	}

	return newfac;
}

void QuestManager::enabletitle(int titleset) {
	initiator->EnableTitle(titleset);
}



bool QuestManager::checktitle(int titleset) {
	return initiator->CheckTitle(titleset);
}

void QuestManager::removetitle(int titleset) {
	initiator->RemoveTitle(titleset);
}

void QuestManager::wearchange(uint8 slot, uint16 texture)
{
	if(owner){
		owner->SendTextureWC(slot, texture);
		if(owner->IsNPC()) {
			owner->CastToNPC()->NPCSlotTexture(slot, texture);
		}
	}
}

void QuestManager::voicetell(char *str, int macronum, int racenum, int gendernum)
{
	if(owner && str)
	{
		Client *c = entity_list.GetClientByName(str);

		if(c)
		{
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_VoiceMacroOut, sizeof(VoiceMacroOut_Struct));

			VoiceMacroOut_Struct* vmo = (VoiceMacroOut_Struct*)outapp->pBuffer;

			strn0cpy(vmo->From, owner->GetCleanName(), sizeof(vmo->From));

			vmo->Type = 1;

			vmo->Voice = (racenum * 2) + gendernum;

			vmo->MacroNumber = macronum;

			c->QueuePacket(outapp);

			safe_delete(outapp);
		}
		else
			LogFile->write(EQEMuLog::Quest, "QuestManager::voicetell from %s. Client %s not found.", owner->GetName(), str);
	}
}

void QuestManager::LearnRecipe(uint32 recipe_id) {
	if(!initiator)
		return;
	initiator->LearnRecipe(recipe_id);
}

void QuestManager::SendMail(const char *to, const char *from, const char *subject, const char *message) {
	if(to == nullptr || from == nullptr || subject == nullptr || message == nullptr) {
		return;
	}

	uint32 message_len = strlen(message) + 1;
	ServerPacket* pack = new ServerPacket(ServerOP_UCSMailMessage, sizeof(ServerMailMessageHeader_Struct) + message_len);
	ServerMailMessageHeader_Struct* mail = (ServerMailMessageHeader_Struct*) pack->pBuffer;

	strn0cpy(mail->to, to, 64);
	strn0cpy(mail->from, from, 64);
	strn0cpy(mail->subject, subject, 128);
	strcpy(mail->message, message);

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint16 QuestManager::CreateDoor(const char* model, float x, float y, float z, float heading, uint8 opentype, uint16 size)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateDoor(model, x, y, z, heading, opentype, size);
	return entid;
}

int32 QuestManager::GetZoneID(const char *zone) {
	return static_cast<int32>(database.GetZoneID(zone));
}

const char* QuestManager::GetZoneLongName(const char *zone) {
	char *long_name;
	database.GetZoneLongName(zone, &long_name);
	std::string ln = long_name;
	safe_delete_array(long_name);

	return ln.c_str();
}

bool QuestManager::TurnInItem(uint32 itm, int charges)
{
	if ( owner && owner->IsNPC() )
	{
		if ( owner->CastToNPC()->DoesQuestItemExist(itm, charges, true) )
			return true;
	}

	return false;
}

void QuestManager::CompleteHandIn()
{
	if ( owner && owner->IsNPC() )
	{
		owner->CastToNPC()->RemoveQuestDeleteItems();
	}
}

void QuestManager::ResetHandIn()
{
	if ( owner && owner->IsNPC() )
	{
		owner->CastToNPC()->ResetQuestDeleteList();
	}
}

void QuestManager::ClearHandIn()
{
	if ( owner && owner->IsNPC() )
	{
		owner->CastToNPC()->ClearQuestLists();
	}
}

void QuestManager::CrossZoneSignalPlayerByCharID(int charid, uint32 data){
	ServerPacket* pack = new ServerPacket(ServerOP_CZSignalClient, sizeof(CZClientSignal_Struct));
	CZClientSignal_Struct* CZSC = (CZClientSignal_Struct*) pack->pBuffer;
	CZSC->charid = charid;
	CZSC->data = data;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignalPlayerByName(const char *CharName, uint32 data){
	uint32 message_len = strlen(CharName) + 1;
	ServerPacket* pack = new ServerPacket(ServerOP_CZSignalClientByName, sizeof(CZClientSignalByName_Struct) + message_len);
	CZClientSignalByName_Struct* CZSC = (CZClientSignalByName_Struct*) pack->pBuffer;
	strn0cpy(CZSC->Name, CharName, 64);
	CZSC->data = data;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}


void QuestManager::CrossZoneMessagePlayerByName(uint32 Type, const char *CharName, const char *Message){
	uint32 message_len = strlen(CharName) + 1;
	uint32 message_len2 = strlen(Message) + 1;
	ServerPacket* pack = new ServerPacket(ServerOP_CZMessagePlayer, sizeof(CZMessagePlayer_Struct) + message_len + message_len2);
	CZMessagePlayer_Struct* CZSC = (CZMessagePlayer_Struct*) pack->pBuffer;
	CZSC->Type = Type;
	strn0cpy(CZSC->CharName, CharName, 64);
	strn0cpy(CZSC->Message, Message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

