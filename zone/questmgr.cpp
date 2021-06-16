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

#include "../common/classes.h"
#include "../common/global_define.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/string_util.h"
#include "../common/say_link.h"

#include "entity.h"
#include "event_codes.h"
#include "guild_mgr.h"
#include "qglobals.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "questmgr.h"
#include "spawn2.h"
#include "worldserver.h"
#include "zone.h"
#include "zonedb.h"
#include "zone_store.h"

#include <iostream>
#include <limits.h>
#include <list>

#ifdef BOTS
#include "bot.h"
#endif

extern QueryServ* QServ;
extern Zone* zone;
extern WorldServer worldserver;
extern EntityList entity_list;

QuestManager quest_manager;

#define QuestManagerCurrentQuestVars() \
	Mob *owner = nullptr; \
	Client *initiator = nullptr; \
	EQ::ItemInstance* questitem = nullptr; \
	bool depop_npc = false; \
	std::string encounter; \
	do { \
		if(!quests_running_.empty()) { \
			running_quest e = quests_running_.top(); \
			owner = e.owner; \
			initiator = e.initiator; \
			questitem = e.questitem; \
			depop_npc = e.depop_npc; \
			encounter = e.encounter; \
		} \
	} while(0)

QuestManager::QuestManager() {
	HaveProximitySays = false;
	item_timers = 0;
}

QuestManager::~QuestManager() {
}

void QuestManager::Process() {
	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->Timer_.Enabled() && cur->Timer_.Check()) {
			if(entity_list.IsMobInZone(cur->mob)) {
				if(cur->mob->IsNPC()) {
					parse->EventNPC(EVENT_TIMER, cur->mob->CastToNPC(), nullptr, cur->name, 0);
				} else if (cur->mob->IsEncounter()) {
					parse->EventEncounter(EVENT_TIMER, cur->mob->CastToEncounter()->GetEncounterName(), cur->name, 0, nullptr);
				} else {
					//this is inheriently unsafe if we ever make it so more than npc/client start timers
					parse->EventPlayer(EVENT_TIMER, cur->mob->CastToClient(), cur->name, 0);
				}

				//we MUST reset our iterator since the quest could have removed/added any
				//number of timers... worst case we have to check a bunch of timers twice
				cur = QTimerList.begin();
				end = QTimerList.end();	//dunno if this is needed, cant hurt...
			} else {
				cur = QTimerList.erase(cur);
			}
		} else
			++cur;
	}

	auto cur_iter = STimerList.begin();
	while(cur_iter != STimerList.end()) {
		if(!cur_iter->Timer_.Enabled()) {
			cur_iter = STimerList.erase(cur_iter);
		} else if(cur_iter->Timer_.Check()) {
			entity_list.SignalMobsByNPCID(cur_iter->npc_id, cur_iter->signal_id);
			cur_iter = STimerList.erase(cur_iter);
		} else {
			++cur_iter;
		}
	}
}

void QuestManager::StartQuest(Mob *_owner, Client *_initiator, EQ::ItemInstance* _questitem, std::string encounter) {
	running_quest run;
	run.owner = _owner;
	run.initiator = _initiator;
	run.questitem = _questitem;
	run.depop_npc = false;
	run.encounter = encounter;
	quests_running_.push(run);
}

void QuestManager::EndQuest() {
	running_quest run = quests_running_.top();
	if(run.depop_npc && run.owner->IsNPC()) {
		//clear out any timers for them...
		std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

		end = QTimerList.end();
		while (cur != end) {
			if (cur->mob == run.owner)
				cur = QTimerList.erase(cur);
			else
				++cur;
		}
		run.owner->Depop();
	}
	quests_running_.pop();
}

void QuestManager::ClearAllTimers() {
	QTimerList.clear();
}

//quest perl functions
void QuestManager::echo(int colour, const char *str) {
	QuestManagerCurrentQuestVars();
	entity_list.MessageClose(initiator, false, 200, colour, str);
}

void QuestManager::say(const char *str, Journal::Options &opts) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::say called with nullptr owner. Probably syntax error in quest file");
		return;
	}
	else {
		// if there is no initiator we still want stuff to work (timers, signals, waypoints, etc)
		if (!RuleB(NPC, EnableNPCQuestJournal) || initiator == nullptr)
			opts.journal_mode = Journal::Mode::None;
		owner->QuestJournalledSay(initiator, str, opts);
	}
}

void QuestManager::me(const char *str) {
	QuestManagerCurrentQuestVars();
	if (!initiator)
		return;
	entity_list.MessageClose(initiator, false, 200, 10, str);
}

void QuestManager::summonitem(uint32 itemid, int16 charges) {
	QuestManagerCurrentQuestVars();
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

Mob* QuestManager::spawn2(int npc_type, int grid, int unused, const glm::vec4& position) {
	const NPCType* tmp = 0;
	if (tmp = content_db.LoadNPCTypesData(npc_type))
	{
		auto npc = new NPC(tmp, nullptr, position, GravityBehavior::Water);
		npc->AddLootTable();
		if (npc->DropsGlobalLoot())
			npc->CheckGlobalLootTables();
		entity_list.AddNPC(npc,true,true);
		if(grid > 0)
		{
			npc->AssignWaypoints(grid);
		}

		return npc;
	}
	return nullptr;
}

Mob* QuestManager::unique_spawn(int npc_type, int grid, int unused, const glm::vec4& position) {
	Mob *other = entity_list.GetMobByNpcTypeID(npc_type);
	if(other != nullptr) {
		return other;
	}

	const NPCType* tmp = 0;
	if (tmp = content_db.LoadNPCTypesData(npc_type))
	{
		auto npc = new NPC(tmp, nullptr, position, GravityBehavior::Water);
		npc->AddLootTable();
		if (npc->DropsGlobalLoot())
			npc->CheckGlobalLootTables();
		entity_list.AddNPC(npc,true,true);
		if(grid > 0)
		{
			npc->AssignWaypoints(grid);
		}

		return npc;
	}
	return nullptr;
}

Mob *QuestManager::spawn_from_spawn2(uint32 spawn2_id)
{
	LinkedListIterator<Spawn2 *> iterator(zone->spawn2_list);
	iterator.Reset();
	Spawn2 *found_spawn = nullptr;

	while (iterator.MoreElements()) {
		Spawn2 *cur = iterator.GetData();
		iterator.Advance();
		if (cur->GetID() == spawn2_id) {
			found_spawn = cur;
			break;
		}
	}

	if (found_spawn) {
		SpawnGroup *spawn_group = zone->spawn_group_list.GetSpawnGroup(found_spawn->SpawnGroupID());
		if (!spawn_group) {
			content_db.LoadSpawnGroupsByID(found_spawn->SpawnGroupID(), &zone->spawn_group_list);
			spawn_group = zone->spawn_group_list.GetSpawnGroup(found_spawn->SpawnGroupID());
			if (!spawn_group) {
				return nullptr;
			}
		}

		uint16 condition_value=1;
		uint16 condition_id=found_spawn->GetSpawnCondition();

		if (condition_id > 0) {
			condition_value = zone->spawn_conditions.GetCondition(zone->GetShortName(), zone->GetInstanceID(), condition_id);
		}

		uint32 npcid = spawn_group->GetNPCType(condition_value);

		if (npcid == 0) {
			return nullptr;
		}

		const NPCType *tmp = content_db.LoadNPCTypesData(npcid);
		if (!tmp) {
			return nullptr;
		}

		if (tmp->unique_spawn_by_name) {
			if (!entity_list.LimitCheckName(tmp->name)) {
				return nullptr;
			}
		}

		if (tmp->spawn_limit > 0) {
			if (!entity_list.LimitCheckType(npcid, tmp->spawn_limit)) {
				return nullptr;
			}
		}

		database.UpdateRespawnTime(spawn2_id, zone->GetInstanceID(), 0);
		found_spawn->SetCurrentNPCID(npcid);

		auto position = glm::vec4(
			found_spawn->GetX(),
			found_spawn->GetY(),
			found_spawn->GetZ(),
			found_spawn->GetHeading()
		);

		auto npc = new NPC(tmp, found_spawn, position, GravityBehavior::Water);

		found_spawn->SetNPCPointer(npc);
		npc->AddLootTable();
		if (npc->DropsGlobalLoot()) {
			npc->CheckGlobalLootTables();
		}
		npc->SetSpawnGroupId(found_spawn->SpawnGroupID());
		entity_list.AddNPC(npc);
		entity_list.LimitAddNPC(npc);

		if (spawn_group->roamdist > 0) {
			npc->AI_SetRoambox(
				spawn_group->roamdist,
				spawn_group->roambox[0],
				spawn_group->roambox[1],
				spawn_group->roambox[2],
				spawn_group->roambox[3],
				spawn_group->delay,
				spawn_group->min_delay
			);
		}
		if (zone->InstantGrids()) {
			found_spawn->LoadGrid();
		}

		return npc;
	}

	return nullptr;
}

void QuestManager::enable_spawn2(uint32 spawn2_id)
{
	database.UpdateSpawn2Status(spawn2_id, 1);
	auto pack = new ServerPacket(ServerOP_SpawnStatusChange, sizeof(ServerSpawnStatusChange_Struct));
	ServerSpawnStatusChange_Struct* ssc = (ServerSpawnStatusChange_Struct*) pack->pBuffer;
	ssc->id = spawn2_id;
	ssc->new_status = 1;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::disable_spawn2(uint32 spawn2_id)
{
	database.UpdateSpawn2Status(spawn2_id, 0);
	auto pack = new ServerPacket(ServerOP_SpawnStatusChange, sizeof(ServerSpawnStatusChange_Struct));
	ServerSpawnStatusChange_Struct* ssc = (ServerSpawnStatusChange_Struct*) pack->pBuffer;
	ssc->id = spawn2_id;
	ssc->new_status = 0;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::setstat(int stat, int value) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->SetStats(stat, value);
}

void QuestManager::incstat(int stat, int value) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->IncStats(stat, value);
}

void QuestManager::castspell(int spell_id, int target_id) {
	QuestManagerCurrentQuestVars();
	if (owner) {
		Mob *tgt = entity_list.GetMob(target_id);
		if(tgt != nullptr)
			owner->SpellFinished(spell_id, tgt, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);
	}
}

void QuestManager::selfcast(int spell_id) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->SpellFinished(spell_id, initiator, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].ResistDiff);
}

void QuestManager::addloot(int item_id, int charges, bool equipitem, int aug1, int aug2, int aug3, int aug4, int aug5, int aug6) {
	QuestManagerCurrentQuestVars();
	if(item_id != 0){
		if(owner->IsNPC())
			owner->CastToNPC()->AddItem(item_id, charges, equipitem, aug1, aug2, aug3, aug4, aug5, aug6);
	}
}

void QuestManager::Zone(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
	{
		auto pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
		ZoneToZone_Struct* ztz = (ZoneToZone_Struct*) pack->pBuffer;
		ztz->response = 0;
		ztz->current_zone_id = zone->GetZoneID();
		ztz->current_instance_id = zone->GetInstanceID();
		ztz->requested_zone_id = ZoneID(zone_name);
		ztz->admin = initiator->Admin();
		strcpy(ztz->name, initiator->GetName());
		ztz->guild_id = initiator->GuildID();
		ztz->ignorerestrictions = 3;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::ZoneGroup(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient()) {
		if (!initiator->GetGroup()) {
			auto pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
			ZoneToZone_Struct* ztz = (ZoneToZone_Struct*) pack->pBuffer;
			ztz->response = 0;
			ztz->current_zone_id = zone->GetZoneID();
			ztz->current_instance_id = zone->GetInstanceID();
			ztz->requested_zone_id = ZoneID(zone_name);
			ztz->admin = initiator->Admin();
			strcpy(ztz->name, initiator->GetName());
			ztz->guild_id = initiator->GuildID();
			ztz->ignorerestrictions = 3;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		} else {
			auto client_group = initiator->GetGroup();
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					auto pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
					ZoneToZone_Struct* ztz = (ZoneToZone_Struct*) pack->pBuffer;
					ztz->response = 0;
					ztz->current_zone_id = zone->GetZoneID();
					ztz->current_instance_id = zone->GetInstanceID();
					ztz->requested_zone_id = ZoneID(zone_name);
					ztz->admin = group_member->Admin();
					strcpy(ztz->name, group_member->GetName());
					ztz->guild_id = group_member->GuildID();
					ztz->ignorerestrictions = 3;
					worldserver.SendPacket(pack);
					safe_delete(pack);
				}
			}
		}
	}
}

void QuestManager::ZoneRaid(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient()) {
		if (!initiator->GetRaid()) {
			auto pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
			ZoneToZone_Struct* ztz = (ZoneToZone_Struct*) pack->pBuffer;
			ztz->response = 0;
			ztz->current_zone_id = zone->GetZoneID();
			ztz->current_instance_id = zone->GetInstanceID();
			ztz->requested_zone_id = ZoneID(zone_name);
			ztz->admin = initiator->Admin();
			strcpy(ztz->name, initiator->GetName());
			ztz->guild_id = initiator->GuildID();
			ztz->ignorerestrictions = 3;
			worldserver.SendPacket(pack);
			safe_delete(pack);
		} else {
			auto client_raid = initiator->GetRaid();
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					auto pack = new ServerPacket(ServerOP_ZoneToZoneRequest, sizeof(ZoneToZone_Struct));
					ZoneToZone_Struct* ztz = (ZoneToZone_Struct*) pack->pBuffer;
					ztz->response = 0;
					ztz->current_zone_id = zone->GetZoneID();
					ztz->current_instance_id = zone->GetInstanceID();
					ztz->requested_zone_id = ZoneID(zone_name);
					ztz->admin = raid_member->Admin();
					strcpy(ztz->name, raid_member->GetName());
					ztz->guild_id = raid_member->GuildID();
					ztz->ignorerestrictions = 3;
					worldserver.SendPacket(pack);
					safe_delete(pack);
				}
			}
		}
	}
}

void QuestManager::settimer(const char *timer_name, int seconds) {
	QuestManagerCurrentQuestVars();

	if(questitem) {
		questitem->SetTimer(timer_name, seconds * 1000);
		return;
	}

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if(cur->mob && cur->mob == owner && cur->name == timer_name)
		{
			cur->Timer_.Enable();
			cur->Timer_.Start(seconds * 1000, false);
			return;
		}
		++cur;
	}

	QTimerList.push_back(QuestTimer(seconds * 1000, owner, timer_name));
}

void QuestManager::settimerMS(const char *timer_name, int milliseconds) {
	QuestManagerCurrentQuestVars();

	if(questitem) {
		questitem->SetTimer(timer_name, milliseconds);
		return;
	}

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if(cur->mob && cur->mob == owner && cur->name == timer_name)
		{
			cur->Timer_.Enable();
			cur->Timer_.Start(milliseconds, false);
			return;
		}
		++cur;
	}

	QTimerList.push_back(QuestTimer(milliseconds, owner, timer_name));
}

void QuestManager::settimerMS(const char *timer_name, int milliseconds, EQ::ItemInstance *inst) {
	if (inst) {
		inst->SetTimer(timer_name, milliseconds);
	}
}

void QuestManager::settimerMS(const char *timer_name, int milliseconds, Mob *mob) {
	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->mob && cur->mob == mob && cur->name == timer_name)
		{
			cur->Timer_.Enable();
			cur->Timer_.Start(milliseconds, false);
			return;
		}
		++cur;
	}

	QTimerList.push_back(QuestTimer(milliseconds, mob, timer_name));
}

void QuestManager::stoptimer(const char *timer_name) {
	QuestManagerCurrentQuestVars();

	if (questitem) {
		questitem->StopTimer(timer_name);
		return;
	}

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->mob && cur->mob == owner && cur->name == timer_name) {
			QTimerList.erase(cur);
			return;
		}
		++cur;
	}
}

void QuestManager::stoptimer(const char *timer_name, EQ::ItemInstance *inst) {
	if (inst) {
		inst->StopTimer(timer_name);
	}
}

void QuestManager::stoptimer(const char *timer_name, Mob *mob) {
	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->mob && cur->mob == mob && cur->name == timer_name) {
			QTimerList.erase(cur);
			return;
		}
		++cur;
	}
}

void QuestManager::stopalltimers() {
	QuestManagerCurrentQuestVars();

	if(questitem) {
		questitem->ClearTimers();
		return;
	}

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end, tmp;

	end = QTimerList.end();
	while (cur != end) {
		if(cur->mob && cur->mob == owner)
			cur = QTimerList.erase(cur);
		else
			++cur;
	}
}

void QuestManager::stopalltimers(EQ::ItemInstance *inst) {
	if (inst) {
		inst->ClearTimers();
	}
}

void QuestManager::stopalltimers(Mob *mob) {
	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end, tmp;

	end = QTimerList.end();
	while (cur != end) {
		if (cur->mob && cur->mob == mob)
			cur = QTimerList.erase(cur);
		else
			++cur;
	}
}

void QuestManager::pausetimer(const char *timer_name) {
	QuestManagerCurrentQuestVars();

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;
	std::list<PausedTimer>::iterator pcur = PTimerList.begin(), pend;
	PausedTimer pt;
	uint32 milliseconds = 0;

	pend = PTimerList.end();
	while (pcur != pend)
	{
		if (pcur->owner && pcur->owner == owner && pcur->name == timer_name)
		{
			LogQuests("Timer [{}] is already paused for [{}]. Returning", timer_name, owner->GetName());
			return;
		}
		++pcur;
	}

	end = QTimerList.end();
	while (cur != end)
	{
		if (cur->mob && cur->mob == owner && cur->name == timer_name)
		{
			milliseconds = cur->Timer_.GetRemainingTime();
			QTimerList.erase(cur);
			break;
		}
		++cur;
	}

	std::string timername = timer_name;
	pt.name = timername;
	pt.owner = owner;
	pt.time = milliseconds;
	LogQuests("Pausing timer [{}] for [{}] with [{}] ms remaining", timer_name, owner->GetName(), milliseconds);
	PTimerList.push_back(pt);
}

void QuestManager::resumetimer(const char *timer_name) {
	QuestManagerCurrentQuestVars();

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;
	std::list<PausedTimer>::iterator pcur = PTimerList.begin(), pend;
	PausedTimer pt;
	uint32 milliseconds = 0;

	pend = PTimerList.end();
	while (pcur != pend)
	{
		if (pcur->owner && pcur->owner == owner && pcur->name == timer_name)
		{
			milliseconds = pcur->time;
			PTimerList.erase(pcur);
			break;
		}
		++pcur;
	}

	if (milliseconds == 0)
	{
		LogQuests("Paused timer [{}] not found or has expired. Returning", timer_name);
		return;
	}

	end = QTimerList.end();
	while (cur != end)
	{
		if (cur->mob && cur->mob == owner && cur->name == timer_name)
		{
			cur->Timer_.Enable();
			cur->Timer_.Start(milliseconds, false);
			LogQuests("Resuming timer [{}] for [{}] with [{}] ms remaining", timer_name, owner->GetName(), milliseconds);
			return;
		}
		++cur;
	}

	QTimerList.push_back(QuestTimer(milliseconds, owner, timer_name));
	LogQuests("Creating a new timer and resuming [{}] for [{}] with [{}] ms remaining", timer_name, owner->GetName(), milliseconds);

}

bool QuestManager::ispausedtimer(const char *timer_name) {
	QuestManagerCurrentQuestVars();

	std::list<PausedTimer>::iterator pcur = PTimerList.begin(), pend;

	pend = PTimerList.end();
	while (pcur != pend)
	{
		if (pcur->owner && pcur->owner == owner && pcur->name == timer_name)
		{
			return true;
		}
		++pcur;
	}

	return false;
}

void QuestManager::emote(const char *str) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::emote called with nullptr owner. Probably syntax error in quest file");
		return;
	}
	else {
		owner->Emote(str);
	}
}

void QuestManager::shout(const char *str) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::shout called with nullptr owner. Probably syntax error in quest file");
		return;
	}
	else {
		owner->Shout(str);
	}
}

void QuestManager::shout2(const char *str) {
	QuestManagerCurrentQuestVars();
	if (!owner) {
		LogQuests("QuestManager::shout2 called with nullptr owner. Probably syntax error in quest file");
		return;
	}
	else {
		worldserver.SendEmoteMessage(0,0,0,13, "%s shouts, '%s'", owner->GetCleanName(), str);
	}
}

void QuestManager::gmsay(const char *str, uint32 color, bool send_to_world, uint32 to_guilddbid, uint32 to_minstatus) {
	QuestManagerCurrentQuestVars();
	if(send_to_world)
		worldserver.SendEmoteMessage(0, to_guilddbid, to_minstatus, color, "%s", str);
	else
		entity_list.MessageStatus(to_guilddbid, to_minstatus, color, "%s", str);
}

void QuestManager::depop(int npc_type) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		LogQuests("QuestManager::depop called with nullptr owner or non-NPC owner. Probably syntax error in quest file");
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
					running_quest e = quests_running_.top();
					e.depop_npc = true;
					quests_running_.pop();
					quests_running_.push(e);
				}
			}
		}
		else {	//depop self
			running_quest e = quests_running_.top();
			e.depop_npc = true;
			quests_running_.pop();
			quests_running_.push(e);
		}
	}
}

void QuestManager::depop_withtimer(int npc_type) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		LogQuests("QuestManager::depop_withtimer called with nullptr owner or non-NPC owner. Probably syntax error in quest file");
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
	QuestManagerCurrentQuestVars();
	if(owner && owner->IsNPC() && (npc_type > 0)) {
		entity_list.DepopAll(npc_type);
	}
	else {
		LogQuests("QuestManager::depopall called with nullptr owner, non-NPC owner, or invalid NPC Type ID. Probably syntax error in quest file");
	}
}

void QuestManager::depopzone(bool StartSpawnTimer) {
	if(zone) {
		zone->Depop(StartSpawnTimer);
	}
	else {
		LogQuests("QuestManager::depopzone called with nullptr zone. Probably syntax error in quest file");
	}
}

void QuestManager::repopzone() {
	if(zone) {
		zone->Repop();
	}
	else {
		LogQuests("QuestManager::repopzone called with nullptr zone. Probably syntax error in quest file");
	}
}

void QuestManager::processmobswhilezoneempty(bool on) {
	if(zone) {
		zone->process_mobs_while_empty = on;
	}
	else {
		LogQuests("QuestManager::processmobswhilezoneempty called with nullptr zone. Probably syntax error in quest file");
	}
}

void QuestManager::settarget(const char *type, int target_id) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Mob* tmp = nullptr;
	if (!strcasecmp(type,"npctype"))
		tmp = entity_list.GetMobByNpcTypeID(target_id);
	else if (!strcasecmp(type, "entity"))
		tmp = entity_list.GetMob(target_id);

	if (tmp != nullptr)
		owner->SetTarget(tmp);
}

void QuestManager::follow(int entity_id, int distance) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->SetFollowID(entity_id);
	owner->SetFollowDistance(distance * distance);
}

void QuestManager::sfollow() {
	QuestManagerCurrentQuestVars();
	if (owner == nullptr || !owner->IsNPC())
		return;
	owner->SetFollowID(0);
}

void QuestManager::changedeity(int deity_id) {
	QuestManagerCurrentQuestVars();
	//Changes the deity.
	if(initiator)
	{
		if(initiator->IsClient())
		{
			initiator->SetDeity(deity_id);
			initiator->Message(Chat::Yellow,"Your Deity has been changed/set to: %i", deity_id);
			initiator->Save(1);
			initiator->Kick("Deity change by QuestManager");
		}
		else
		{
			initiator->Message(Chat::Yellow,"Error changing Deity");
		}
	}
}

void QuestManager::exp(int amt) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->AddEXP(amt);
}

void QuestManager::level(int newlevel) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->SetLevel(newlevel, true);
}

void QuestManager::traindisc(int discipline_tome_item_id) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->TrainDiscipline(discipline_tome_item_id);
}

bool QuestManager::isdisctome(int item_id) {
	const EQ::ItemData *item = database.GetItem(item_id);
	if(item == nullptr) {
		return(false);
	}

	if (!item->IsClassCommon() || item->ItemType != EQ::item::ItemTypeSpell) {
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

std::string QuestManager::getracename(uint16 race_id) {
	return GetRaceIDName(race_id);
}

std::string QuestManager::getspellname(uint32 spell_id) {
	if (!IsValidSpell(spell_id)) {
		return "INVALID SPELL ID IN GETSPELLNAME";
	}

	std::string spell_name = GetSpellName(spell_id);
	return spell_name;
}

std::string QuestManager::getskillname(int skill_id) {
	if (skill_id >= 0 && skill_id < EQ::skills::SkillCount) {
		std::map<EQ::skills::SkillType, std::string> Skills = EQ::skills::GetSkillTypeMap();
		for (auto skills_iter : Skills) {
			if (skill_id == skills_iter.first) {
				return skills_iter.second;
			}
		}
	}
	return std::string();
}

void QuestManager::safemove() {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->GoToSafeCoords(zone->GetZoneID(), zone->GetInstanceID());
}

void QuestManager::rain(int weather) {
	QuestManagerCurrentQuestVars();
	zone->zone_weather = weather;
	auto outapp = new EQApplicationPacket(OP_Weather, 8);
	*((uint32*) &outapp->pBuffer[4]) = (uint32) weather; // Why not just use 0x01/2/3?
	entity_list.QueueClients(owner, outapp);
	safe_delete(outapp);
}

void QuestManager::snow(int weather) {
	QuestManagerCurrentQuestVars();
	zone->zone_weather = weather + 1;
	auto outapp = new EQApplicationPacket(OP_Weather, 8);
	outapp->pBuffer[0] = 0x01;
	*((uint32*) &outapp->pBuffer[4]) = (uint32)weather;
	entity_list.QueueClients(initiator, outapp);
	safe_delete(outapp);
}

void QuestManager::rename(std::string name) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient()) {
		std::string current_name = initiator->GetName();
		if (initiator->ChangeFirstName(name.c_str(), current_name.c_str())) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"Successfully renamed to {}, kicking to character select.",
					name
				).c_str()
			);
			initiator->Kick("Name was changed.");
		} else {
			initiator->Message(
				Chat::Red,
				fmt::format(
					"Failed to rename {} to {}.",
					current_name, name
				).c_str()
			);
		}
	}
}

void QuestManager::surname(const char *name) {
	QuestManagerCurrentQuestVars();
	//Changes the last name.
	if(initiator)
	{
		if(initiator->IsClient())
		{
			initiator->ChangeLastName(name);
			initiator->Message(Chat::Yellow,"Your surname has been changed/set to: %s", name);
		}
		else
		{
			initiator->Message(Chat::Yellow,"Error changing/setting surname");
		}
	}
}

void QuestManager::permaclass(int class_id) {
	QuestManagerCurrentQuestVars();
	//Makes the client the class specified
	initiator->SetBaseClass(class_id);
	initiator->Save(2);
	initiator->Kick("Base class change by QuestManager");
}

void QuestManager::permarace(int race_id) {
	QuestManagerCurrentQuestVars();
	//Makes the client the race specified
	initiator->SetBaseRace(race_id);
	initiator->Save(2);
	initiator->Kick("Base race change by QuestManager");
}

void QuestManager::permagender(int gender_id) {
	QuestManagerCurrentQuestVars();
	//Makes the client the gender specified
	initiator->SetBaseGender(gender_id);
	initiator->Save(2);
	initiator->Kick("Base gender change by QuestManager");
}

uint16 QuestManager::scribespells(uint8 max_level, uint8 min_level) {
	QuestManagerCurrentQuestVars();
	int book_slot = initiator->GetNextAvailableSpellBookSlot();
	std::vector<int> spell_ids = initiator->GetScribeableSpells(min_level, max_level);
	int spell_count = spell_ids.size();
	int spells_learned = 0;
	if (spell_count > 0) {
		for (auto spell_id : spell_ids) {
			if (book_slot == -1) {			
				initiator->Message(
					Chat::Red,
					"Unable to scribe spell %s (%i) to Spell Book: Spell Book is Full.", spells[spell_id].name, spell_id
				);
				break;
			}
			
			if (initiator->HasSpellScribed(spell_id))
				continue;
				
			initiator->ScribeSpell(spell_id, book_slot);
			book_slot = initiator->GetNextAvailableSpellBookSlot(book_slot);
			spells_learned++;
		}
	}

	if (spells_learned > 0) {
		std::string spell_message = (spells_learned == 1 ? "a new spell" : fmt::format("{} new spells", spells_learned));
		initiator->Message(Chat::White, fmt::format("You have learned {}!", spell_message).c_str());
	}
	return spells_learned;
}

uint16 QuestManager::traindiscs(uint8 max_level, uint8 min_level) {
	QuestManagerCurrentQuestVars();
	int character_id = initiator->CharacterID();
	std::vector<int> spell_ids = initiator->GetLearnableDisciplines(min_level, max_level);
	int discipline_count = spell_ids.size();
	int disciplines_learned = 0;
	if (discipline_count > 0) {
		for (auto spell_id : spell_ids) {
			if (initiator->HasDisciplineLearned(spell_id))
				continue;

			for (uint32 index = 0; index < MAX_PP_DISCIPLINES; index++) {
				if (initiator->GetPP().disciplines.values[index] == 0) {
					initiator->GetPP().disciplines.values[index] = spell_id;
					database.SaveCharacterDisc(character_id, index, spell_id);
					disciplines_learned++;
					break;
				}
			}
		}
	}

	if (disciplines_learned > 0) {
		std::string discipline_message = (disciplines_learned == 1 ? "a new discipline" : fmt::format("{} new disciplines", disciplines_learned));
		initiator->SendDisciplineUpdate();
		initiator->Message(Chat::White, fmt::format("You have learned {}!", discipline_message).c_str());
	}

	return disciplines_learned;
}

void QuestManager::unscribespells() {
	QuestManagerCurrentQuestVars();
	initiator->UnscribeSpellAll();
}

void QuestManager::untraindiscs() {
	QuestManagerCurrentQuestVars();
	initiator->UntrainDiscAll();
}

void QuestManager::givecash(int copper, int silver, int gold, int platinum) {
	QuestManagerCurrentQuestVars();
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
			initiator->Message(Chat::OOC, tmp.c_str());
	}
}

void QuestManager::pvp(const char *mode) {
	QuestManagerCurrentQuestVars();
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
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->MovePC(zone_id, x, y, z, heading);
}

void QuestManager::gmmove(float x, float y, float z) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->GMMove(x, y, z);
}

void QuestManager::movegrp(int zoneid, float x, float y, float z) {
	QuestManagerCurrentQuestVars();
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
	QuestManagerCurrentQuestVars();
	owner->DoAnim(anim_id);
}

void QuestManager::addskill(int skill_id, int value) {
	QuestManagerCurrentQuestVars();
	if (skill_id < 0 || skill_id > EQ::skills::HIGHEST_SKILL)
		return;
	if (initiator && initiator->IsClient())
		initiator->AddSkill((EQ::skills::SkillType) skill_id, value);
}

void QuestManager::setlanguage(int skill_id, int value) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->SetLanguageSkill(skill_id, value);
}

void QuestManager::setskill(int skill_id, int value) {
	QuestManagerCurrentQuestVars();
	if (skill_id < 0 || skill_id > EQ::skills::HIGHEST_SKILL)
		return;
	if (initiator && initiator->IsClient())
		initiator->SetSkill((EQ::skills::SkillType) skill_id, value);
}

void QuestManager::setallskill(int value) {
	QuestManagerCurrentQuestVars();
	if (!initiator)
		return;
	if (initiator && initiator->IsClient()) {
		EQ::skills::SkillType sk;
		for (sk = EQ::skills::Skill1HBlunt; sk <= EQ::skills::HIGHEST_SKILL; sk = (EQ::skills::SkillType)(sk + 1)) {
			initiator->SetSkill(sk, value);
		}
	}
}

void QuestManager::attack(const char *client_name) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Client* getclient = entity_list.GetClientByName(client_name);
	if (getclient && owner->IsAttackAllowed(getclient))
		owner->AddToHateList(getclient,1);
	else
		owner->Say("I am unable to attack %s.", client_name);
}

void QuestManager::attacknpc(int npc_entity_id) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Mob *it = entity_list.GetMob(npc_entity_id);
	if (it && owner->IsAttackAllowed(it)) {
		owner->AddToHateList(it,1);
	} else {
		if (it)
			owner->Say("I am unable to attack %s.", it->GetName());
		else
			owner->Say("I am unable to locate NPC entity %i", npc_entity_id);
	}
}

void QuestManager::attacknpctype(int npc_type_id) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	Mob *it = entity_list.GetMobByNpcTypeID(npc_type_id);
	if (it && owner->IsAttackAllowed(it)) {
		owner->AddToHateList(it,1);
	} else {
		if (it)
			owner->Say("I am unable to attack %s.", it->GetName());
		else
			owner->Say("I am unable to locate NPC type %i", npc_type_id);
	}
}

void QuestManager::save() {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient())
		initiator->Save();
}

void QuestManager::faction(int faction_id, int faction_value, int temp) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient()) {
		if(faction_id != 0 && faction_value != 0) {
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
	QuestManagerCurrentQuestVars();
	if (zone)
		zone->newzone_data.sky = new_sky;
	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(initiator, outapp);
	safe_delete(outapp);
}

void QuestManager::setguild(uint32 new_guild_id, uint8 new_rank) {
	QuestManagerCurrentQuestVars();
	if (initiator && initiator->IsClient()) {
		guild_mgr.SetGuild(initiator->CharacterID(), new_guild_id, new_rank);
	}
}

void QuestManager::CreateGuild(const char *guild_name, const char *leader) {
	QuestManagerCurrentQuestVars();
	uint32 cid = database.GetCharacterID(leader);
	char hString[250];
			if (cid == 0) {
				worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", "Guild Creation: Guild leader not found.");
				return;
			}

			uint32 tmp = guild_mgr.FindGuildByLeader(cid);
			if (tmp != GUILD_NONE) {
				sprintf(hString, "Guild Creation: Error: %s already is the leader of DB# %u '%s'.", leader, tmp, guild_mgr.GetGuildName(tmp));
				worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", hString);
			}
			else {
				uint32 gid = guild_mgr.CreateGuild(guild_name, cid);
				if (gid == GUILD_NONE)
					worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", "Guild Creation: Guild creation failed");
				else {
					sprintf(hString, "Guild Creation: Guild created: Leader: %u, number %u: %s", cid, gid, leader);
					worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", hString);
					if(!guild_mgr.SetGuild(cid, gid, GUILD_LEADER))
						worldserver.SendEmoteMessage(0, 0, 80, 15, "%s", "Unable to set guild leader's guild in the database. Your going to have to run #guild set");
				}

			}
}

void QuestManager::settime(uint8 new_hour, uint8 new_min, bool update_world /*= true*/)
{
	if (zone)
		zone->SetTime(new_hour + 1, new_min, update_world);
}

void QuestManager::itemlink(int item_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		const EQ::ItemData* item = database.GetItem(item_id);
		if (item == nullptr)
			return;

		EQ::SayLinkEngine linker;
		linker.SetLinkType(EQ::saylink::SayLinkItemData);
		linker.SetItemData(item);

		initiator->Message(Chat::White, "%s tells you, %s", owner->GetCleanName(), linker.GenerateLink().c_str());
	}
}

void QuestManager::signalwith(int npc_id, int signal_id, int wait_ms) {
	if(wait_ms > 0) {
		STimerList.push_back(SignalTimer(wait_ms, npc_id, signal_id));
		return;
	} else {
		STimerList.push_back(SignalTimer(0, npc_id, signal_id));
		return;
	}
}

void QuestManager::signal(int npc_id, int wait_ms) {
	signalwith(npc_id, 0, wait_ms);
}

void QuestManager::setglobal(const char *varname, const char *newvalue, int options, const char *duration) {
	QuestManagerCurrentQuestVars();
	int qgZoneid = zone->GetZoneID();
	int qgCharid = 0;
	int qgNpcid = owner ? owner->GetNPCTypeID() : 0; // encounter scripts don't have an owner

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

	if (initiator && initiator->IsClient()){ // some events like waypoint and spawn don't have a player involved
		qgCharid=initiator->CharacterID();
	}
	else {
		qgCharid=-qgNpcid;		// make char id negative npc id as a fudge
	}
	if (options < 0 || options > 7) {
		std::cerr << "Invalid options for global var " << varname << " using defaults" << std::endl;
	}	// default = 0 (only this npcid,player and zone)
	else {
		if (options & 1)
			qgNpcid=0;
		if (options & 2)
			qgCharid=0;
		if (options & 4)
			qgZoneid=0;
	}

	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, newvalue, QGVarDuration(duration));

	/* QS: PlayerLogQGlobalUpdate */
	if (RuleB(QueryServ, PlayerLogQGlobalUpdate) && qgCharid && qgCharid > 0 && initiator && initiator->IsClient()){
		std::string event_desc = StringFormat("Update :: qglobal:%s to qvalue:%s zoneid:%i instid:%i", varname, newvalue, initiator->GetZoneID(), initiator->GetInstanceID());
		QServ->PlayerLogEvent(Player_Log_QGlobal_Update, qgCharid, event_desc);
	}
}

/* Inserts global variable into quest_globals table */
int QuestManager::InsertQuestGlobal(int charid, int npcid, int zoneid, const char *varname, const char *varvalue, int duration) {

	// Make duration string either "unix_timestamp(now()) + xxx" or "NULL"
	std::string durationText = (duration == INT_MAX)? "NULL": StringFormat("unix_timestamp(now()) + %i", duration);

	/*
		NOTE: this should be escaping the contents of arglist
		npcwise a malicious script can arbitrarily alter the DB
	*/

	std::string query = StringFormat("REPLACE INTO quest_globals "
                                    "(charid, npcid, zoneid, name, value, expdate)"
                                    "VALUES (%i, %i, %i, '%s', '%s', %s)",
                                    charid, npcid, zoneid, varname, varvalue, durationText.c_str());
    auto results = database.QueryDatabase(query);
	if (!results.Success())
		std::cerr << "setglobal error inserting " << varname << " : " << results.ErrorMessage() << std::endl;

	if(!zone)
        return 0;

    /* Delete existing qglobal data and update zone processes */
	auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
	ServerQGlobalDelete_Struct *qgd = (ServerQGlobalDelete_Struct *)pack->pBuffer;
	qgd->npc_id = npcid;
	qgd->char_id = charid;
	qgd->zone_id = zoneid;
	qgd->from_zone_id = zone->GetZoneID();
	qgd->from_instance_id = zone->GetInstanceID();
	strcpy(qgd->name, varname);

	entity_list.DeleteQGlobal(std::string((char *)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
	zone->DeleteQGlobal(std::string((char *)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);

	worldserver.SendPacket(pack);
	safe_delete(pack);

	/* Create new qglobal data and update zone processes */
	pack = new ServerPacket(ServerOP_QGlobalUpdate, sizeof(ServerQGlobalUpdate_Struct));
	ServerQGlobalUpdate_Struct *qgu = (ServerQGlobalUpdate_Struct*) pack->pBuffer;
	qgu->npc_id = npcid;
	qgu->char_id = charid;
	qgu->zone_id = zoneid;

	qgu->expdate = (duration == INT_MAX)? 0xFFFFFFFF: Timer::GetTimeSeconds() + duration;

    strcpy((char*)qgu->name, varname);
    strn0cpy((char*)qgu->value, varvalue, 128);
	qgu->id = results.LastInsertedID();
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

	return 0;
}

void QuestManager::targlobal(const char *varname, const char *value, const char *duration, int qgNpcid, int qgCharid, int qgZoneid) {
	InsertQuestGlobal(qgCharid, qgNpcid, qgZoneid, varname, value, QGVarDuration(duration));
}

void QuestManager::delglobal(const char *varname) {
	QuestManagerCurrentQuestVars();
	int qgZoneid = zone->GetZoneID();
	int qgCharid = 0;
	int qgNpcid = owner ? owner->GetNPCTypeID() : 0; // encounter scripts don't have an owner

	if (initiator && initiator->IsClient()) // some events like waypoint and spawn don't have a player involved
		qgCharid=initiator->CharacterID();
	else
		qgCharid=-qgNpcid;		// make char id negative npc id as a fudge

	/* QS: PlayerLogQGlobalUpdate */
	if (RuleB(QueryServ, PlayerLogQGlobalUpdate) && qgCharid && qgCharid > 0 && initiator && initiator->IsClient()){
		std::string event_desc = StringFormat("Deleted :: qglobal:%s zoneid:%i instid:%i", varname, initiator->GetZoneID(), initiator->GetInstanceID());
		QServ->PlayerLogEvent(Player_Log_QGlobal_Update, qgCharid, event_desc);
	}

    std::string query = StringFormat("DELETE FROM quest_globals "
                                    "WHERE name = '%s' "
                                    "&& (npcid=0 || npcid=%i) "
                                    "&& (charid=0 || charid=%i) "
                                    "&& (zoneid=%i || zoneid=0)",
                                    varname, qgNpcid, qgCharid, qgZoneid);
    auto results = database.QueryDatabase(query);
	if (!results.Success())
		std::cerr << "delglobal error deleting " << varname << " : " << results.ErrorMessage() << std::endl;

	if(!zone)
        return;

	auto pack = new ServerPacket(ServerOP_QGlobalDelete, sizeof(ServerQGlobalDelete_Struct));
	ServerQGlobalDelete_Struct *qgu = (ServerQGlobalDelete_Struct *)pack->pBuffer;

	qgu->npc_id = qgNpcid;
	qgu->char_id = qgCharid;
	qgu->zone_id = qgZoneid;
	strcpy(qgu->name, varname);

	entity_list.DeleteQGlobal(std::string((char *)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);
	zone->DeleteQGlobal(std::string((char *)qgu->name), qgu->npc_id, qgu->char_id, qgu->zone_id);

	worldserver.SendPacket(pack);
	safe_delete(pack);
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
	QuestManagerCurrentQuestVars();
	//makes a sound.
	if (initiator && initiator->IsClient())
		initiator->SendSound();

}

void QuestManager::rebind(int zone_id, const glm::vec3& location) {
	QuestManagerCurrentQuestVars();
	if(initiator && initiator->IsClient()) {
		initiator->SetBindPoint(0, zone_id, 0, location);
	}
}

void QuestManager::rebind(int zone_id, const glm::vec4& location) {
	QuestManagerCurrentQuestVars();
	if(initiator && initiator->IsClient()) {
		initiator->SetBindPoint2(0, zone_id, 0, location);
	}
}

void QuestManager::start(int32 wp) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->AssignWaypoints(wp);
}

void QuestManager::stop() {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->StopWandering();
}

void QuestManager::pause(int duration) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->PauseWandering(duration);
}

void QuestManager::moveto(const glm::vec4& position, bool saveguardspot) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->MoveTo(position, saveguardspot);
}

void QuestManager::resume() {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	owner->CastToNPC()->ResumeWandering();
}

void QuestManager::addldonpoints(uint32 theme_id, int points) {
	QuestManagerCurrentQuestVars();
	if(initiator)
		initiator->UpdateLDoNPoints(theme_id, points);
}

void QuestManager::addldonloss(uint32 theme_id) {
	QuestManagerCurrentQuestVars();
	if(initiator)
		initiator->AddLDoNLoss(theme_id);
}

void QuestManager::addldonwin(uint32 theme_id) {
	QuestManagerCurrentQuestVars();
	if(initiator)
		initiator->AddLDoNWin(theme_id);
}

void QuestManager::setnexthpevent(int at) {
	QuestManagerCurrentQuestVars();
	if (owner)
		owner->SetNextHPEvent(at);
}

void QuestManager::setnextinchpevent(int at) {
	QuestManagerCurrentQuestVars();
	if (owner)
		owner->SetNextIncHPEvent(at);
}

void QuestManager::respawn(int npcTypeID, int grid) {
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC())
		return;

	running_quest e = quests_running_.top();
	e.depop_npc = true;
	quests_running_.pop();
	quests_running_.push(e);

	const NPCType* npcType = nullptr;
	if ((npcType = content_db.LoadNPCTypesData(npcTypeID)))
	{
		owner = new NPC(npcType, nullptr, owner->GetPosition(), GravityBehavior::Water);
		owner->CastToNPC()->AddLootTable();
		if (owner->CastToNPC()->DropsGlobalLoot())
			owner->CastToNPC()->CheckGlobalLootTables();
		entity_list.AddNPC(owner->CastToNPC(),true,true);
		if(grid > 0)
			owner->CastToNPC()->AssignWaypoints(grid);
	}
}

void QuestManager::set_proximity(float minx, float maxx, float miny, float maxy, float minz, float maxz, bool bSay)
{
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		return;
	}

	entity_list.AddProximity(owner->CastToNPC());

	owner->CastToNPC()->proximity->min_x         = minx;
	owner->CastToNPC()->proximity->max_x         = maxx;
	owner->CastToNPC()->proximity->min_y         = miny;
	owner->CastToNPC()->proximity->max_y         = maxy;
	owner->CastToNPC()->proximity->min_z         = minz;
	owner->CastToNPC()->proximity->max_z         = maxz;
	owner->CastToNPC()->proximity->say           = bSay;
	owner->CastToNPC()->proximity->proximity_set = true;
}

void QuestManager::clear_proximity() {
	QuestManagerCurrentQuestVars();
	if(!owner || !owner->IsNPC())
		return;

	entity_list.RemoveProximity(owner->GetID());
	safe_delete(owner->CastToNPC()->proximity);
}

void QuestManager::enable_proximity_say() {
	HaveProximitySays = true;
}

void QuestManager::disable_proximity_say() {
	HaveProximitySays = false;
}

void QuestManager::setanim(int npc_type, int animnum) {
	//adds appearance changes
	Mob* thenpc = entity_list.GetMobByNpcTypeID(npc_type);
	if(!thenpc || animnum < 0 || animnum >= _eaMaxAppearance)
		return;
	thenpc->SetAppearance(EmuAppearance(animnum));
}

//displays an in game path based on a waypoint grid
void QuestManager::showgrid(int grid) {
	QuestManagerCurrentQuestVars();
	if(initiator == nullptr)
		return;

	FindPerson_Point pt;
	std::vector<FindPerson_Point> pts;

	pt.x = initiator->GetX();
	pt.y = initiator->GetY();
	pt.z = initiator->GetZ();
	pts.push_back(pt);

	// Retrieve all waypoints for this grid
	std::string query = StringFormat("SELECT `x`,`y`,`z` FROM grid_entries "
                                    "WHERE `gridid` = %i AND `zoneid` = %i "
                                    "ORDER BY `number`", grid, zone->GetZoneID());
    auto results = content_db.QueryDatabase(query);
    if (!results.Success()) {
        LogQuests("Error loading grid [{}] for showgrid(): [{}]", grid, results.ErrorMessage().c_str());
		return;
    }

    for(auto row = results.begin(); row != results.end(); ++row) {
        pt.x = atof(row[0]);
        pt.y = atof(row[1]);
        pt.z = atof(row[2]);

        pts.push_back(pt);
    }

    initiator->SendPathPacket(pts);

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
void QuestManager::toggle_spawn_event(int event_id, bool enable, bool strict, bool reset_base) {
	zone->spawn_conditions.ToggleEvent(event_id, enable, strict, reset_base);
}

bool QuestManager::has_zone_flag(int zone_id) {
	QuestManagerCurrentQuestVars();
	return initiator ? initiator->HasZoneFlag(zone_id) : false;
}

void QuestManager::set_zone_flag(int zone_id) {
	QuestManagerCurrentQuestVars();
	initiator->SetZoneFlag(zone_id);
}

void QuestManager::clear_zone_flag(int zone_id) {
	QuestManagerCurrentQuestVars();
	initiator->ClearZoneFlag(zone_id);
}

void QuestManager::sethp(int hpperc) {
	QuestManagerCurrentQuestVars();
	int newhp = (owner->GetMaxHP() * (100 - hpperc)) / 100;
	owner->Damage(owner, newhp, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand, false, 0, false);
}

bool QuestManager::summonburiedplayercorpse(uint32 char_id, const glm::vec4& position) {
	bool Result = false;

	if(char_id <= 0)
        return false;

	Corpse* PlayerCorpse = database.SummonBuriedCharacterCorpses(char_id, zone->GetZoneID(), zone->GetInstanceID(), position);
	if(!PlayerCorpse)
		return false;

	return true;
}

bool QuestManager::summonallplayercorpses(uint32 char_id, const glm::vec4& position) {

	if(char_id <= 0)
        return false;

	Client* c = entity_list.GetClientByCharID(char_id);
	c->SummonAllCorpses(position);

	return true;
}

int QuestManager::getplayercorpsecount(uint32 char_id) {
	if (char_id > 0) {
		return database.CountCharacterCorpses(char_id);
	}
	return 0;

}

int QuestManager::getplayercorpsecountbyzoneid(uint32 char_id, uint32 zone_id) {
	if (char_id > 0 && zone_id > 0) {
		return database.CountCharacterCorpsesByZoneID(char_id, zone_id);
	}
	return 0;
}

uint32 QuestManager::getplayerburiedcorpsecount(uint32 char_id) {
	uint32 Result = 0;

	if(char_id > 0) {
		Result = database.GetCharacterBuriedCorpseCount(char_id);
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
			database.BuryCharacterCorpse(PlayerCorpse);
			Corpse* corpse = entity_list.GetCorpseByDBID(PlayerCorpse);
			if(corpse)
			{
				corpse->Save();
				corpse->DepopPlayerCorpse();
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
	QuestManagerCurrentQuestVars();
	owner->SendIllusionPacket(race_id);
}

void QuestManager::npcgender(int gender_id)
{
	QuestManagerCurrentQuestVars();
	owner->SendIllusionPacket(owner->GetRace(), gender_id);
}
void QuestManager::npcsize(int newsize)
{
	QuestManagerCurrentQuestVars();
	owner->ChangeSize(newsize, true);
}
void QuestManager::npctexture(int newtexture)
{
	QuestManagerCurrentQuestVars();
	owner->SendIllusionPacket(owner->GetRace(), 0xFF, newtexture);
}

void QuestManager::playerrace(int race_id)
{
	QuestManagerCurrentQuestVars();
	initiator->SendIllusionPacket(race_id);
}

void QuestManager::playergender(int gender_id)
{
	QuestManagerCurrentQuestVars();
	initiator->SendIllusionPacket(initiator->GetRace(), gender_id);
}

void QuestManager::playersize(int newsize)
{
	QuestManagerCurrentQuestVars();
	initiator->ChangeSize(newsize, true);
}

void QuestManager::playertexture(int newtexture)
{
	QuestManagerCurrentQuestVars();
	initiator->SendIllusionPacket(initiator->GetRace(), 0xFF, newtexture);
}

void QuestManager::playerfeature(char *feature, int setting)
{
	QuestManagerCurrentQuestVars();
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
	QuestManagerCurrentQuestVars();
	uint16 Race = owner->GetRace();
	uint8 Gender = owner->GetGender();
	uint8 Texture = owner->GetTexture();
	uint8 HelmTexture = owner->GetHelmTexture();
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

void QuestManager::popup(const char *title, const char *text, uint32 popupid, uint32 buttons, uint32 Duration)
{
	QuestManagerCurrentQuestVars();
	if(initiator)
		initiator->SendPopupToClient(title, text, popupid, buttons, Duration);
}

#ifdef BOTS

int QuestManager::createbotcount() {
	return RuleI(Bots, CreationLimit);
}

int QuestManager::spawnbotcount() {
	return RuleI(Bots, SpawnLimit);
}

bool QuestManager::botquest()
{
	return RuleB(Bots, QuestableSpawnLimit);
}

bool QuestManager::createBot(const char *name, const char *lastname, uint8 level, uint16 race, uint8 botclass, uint8 gender)
{
	QuestManagerCurrentQuestVars();
	uint32 MaxBotCreate = RuleI(Bots, CreationLimit);

	if (initiator && initiator->IsClient())
	{
		if(Bot::SpawnedBotCount(initiator->CharacterID()) >= MaxBotCreate)
		{
			initiator->Message(Chat::Yellow,"You have the maximum number of bots allowed.");
			return false;
		}

		std::string test_name = name;
		bool available_flag = false;
		if(!database.botdb.QueryNameAvailablity(test_name, available_flag)) {
			initiator->Message(Chat::White, "%s for '%s'", BotDatabase::fail::QueryNameAvailablity(), (char*)name);
			return false;
		}
		if (!available_flag) {
			initiator->Message(Chat::White, "The name %s is already being used or is invalid. Please choose a different name.", (char*)name);
			return false;
		}

		Bot* NewBot = new Bot(Bot::CreateDefaultNPCTypeStructForBot(name, lastname, level, race, botclass, gender), initiator);

		if(NewBot)
		{
			if(!NewBot->IsValidRaceClassCombo()) {
				initiator->Message(Chat::White, "That Race/Class combination cannot be created.");
				return false;
			}

			if(!NewBot->IsValidName()) {
				initiator->Message(Chat::White, "%s has invalid characters. You can use only the A-Z, a-z and _ characters in a bot name.", NewBot->GetCleanName());
				return false;
			}

			// Now that all validation is complete, we can save our newly created bot
			if(!NewBot->Save())
			{
				initiator->Message(Chat::White, "Unable to save %s as a bot.", NewBot->GetCleanName());
			}
			else
			{
				initiator->Message(Chat::White, "%s saved as bot %u.", NewBot->GetCleanName(), NewBot->GetBotID());
				return true;
			}
		}
	}
	return false;
}

#endif //BOTS

void QuestManager::taskselector(int taskcount, int *tasks) {
	QuestManagerCurrentQuestVars();
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && task_manager)
		initiator->TaskQuestSetSelector(owner, taskcount, tasks);
}
void QuestManager::enabletask(int taskcount, int *tasks) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && task_manager)
		initiator->EnableTask(taskcount, tasks);
}

void QuestManager::disabletask(int taskcount, int *tasks) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && task_manager)
		initiator->DisableTask(taskcount, tasks);
}

bool QuestManager::istaskenabled(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && task_manager)
		return initiator->IsTaskEnabled(taskid);

	return false;
}

void QuestManager::tasksetselector(int tasksetid) {
	QuestManagerCurrentQuestVars();
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSetSelector called for task set %i", tasksetid);
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && task_manager)
		initiator->TaskSetSelector(owner, tasksetid);
}

bool QuestManager::istaskactive(int task) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskActive(task);

	return false;
}

bool QuestManager::istaskactivityactive(int task, int activity) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskActivityActive(task, activity);

	return false;
}

int QuestManager::gettaskactivitydonecount(int task, int activity) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->GetTaskActivityDoneCountFromTaskID(task, activity);

	return 0;

}

void QuestManager::updatetaskactivity(int task, int activity, int count, bool ignore_quest_update /*= false*/)
{
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->UpdateTaskActivity(task, activity, count, ignore_quest_update);
}

void QuestManager::resettaskactivity(int task, int activity) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->ResetTaskActivity(task, activity);
}

void QuestManager::taskexploredarea(int exploreid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->UpdateTasksOnExplore(exploreid);
}

void QuestManager::assigntask(int taskid, bool enforce_level_requirement) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		initiator->AssignTask(taskid, owner->GetID(), enforce_level_requirement);
}

void QuestManager::failtask(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		initiator->FailTask(taskid);
}

int QuestManager::tasktimeleft(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->TaskTimeLeft(taskid);

	return -1;
}

int QuestManager::enabledtaskcount(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->EnabledTaskCount(taskset);

	return -1;
}

int QuestManager::firsttaskinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && task_manager)
		return task_manager->FirstTaskInSet(taskset);

	return -1;
}

int QuestManager::lasttaskinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && task_manager)
		return task_manager->LastTaskInSet(taskset);

	return -1;
}

int QuestManager::nexttaskinset(int taskset, int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && task_manager)
		return task_manager->NextTaskInSet(taskset, taskid);

	return -1;
}

int QuestManager::activespeaktask() {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		return initiator->ActiveSpeakTask(owner->GetNPCTypeID());
	return 0;
}

int QuestManager::activespeakactivity(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner)
		return initiator->ActiveSpeakActivity(owner->GetNPCTypeID(), taskid);

	return 0;
}

int QuestManager::istaskcompleted(int taskid) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->IsTaskCompleted(taskid);

	return -1;
}

int QuestManager::activetasksinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->ActiveTasksInSet(taskset);

	return -1;
}

int QuestManager::completedtasksinset(int taskset) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator)
		return initiator->CompletedTasksInSet(taskset);

	return -1;
}

bool QuestManager::istaskappropriate(int task) {
	QuestManagerCurrentQuestVars();

	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && task_manager)
		return task_manager->ValidateLevel(task, initiator->GetLevel());

	return false;
}

std::string QuestManager::gettaskname(uint32 task_id) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem)) {
		return task_manager->GetTaskName(task_id);
	}

	return std::string();
}

void QuestManager::clearspawntimers() {
	if(!zone)
        return;

	//TODO: Dec 19, 2008, replace with code updated for current spawn timers.
    LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		std::string query = StringFormat("DELETE FROM respawn_times "
                                        "WHERE id = %lu AND instance_id = %lu",
                                        (unsigned long)iterator.GetData()->GetID(),
                                        (unsigned long)zone->GetInstanceID());
        auto results = database.QueryDatabase(query);
		iterator.Advance();
	}
}

void QuestManager::ze(int type, const char *str) {
	entity_list.Message(0, type, str);
}

void QuestManager::we(int type, const char *str) {
	worldserver.SendEmoteMessage(0, 0, type, str);
}

void QuestManager::message(int color, const char *message) {
	QuestManagerCurrentQuestVars();
	if (!initiator)
		return;
	
	initiator->Message(color, message);
}

void QuestManager::whisper(const char *message) {
	QuestManagerCurrentQuestVars();
	if (!initiator || !owner)
		return;

	std::string mob_name = owner->GetCleanName();
	std::string new_message = fmt::format("{} whispers, '{}'", mob_name, message);
	initiator->Message(315, new_message.c_str());
}

int QuestManager::getlevel(uint8 type)
{
	QuestManagerCurrentQuestVars();
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

uint16 QuestManager::CreateGroundObject(uint32 itemid, const glm::vec4& position, uint32 decay_time)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateGroundObject(itemid, position, decay_time);
	return entid;
}

uint16 QuestManager::CreateGroundObjectFromModel(const char *model, const glm::vec4& position, uint8 type, uint32 decay_time)
{
	uint16 entid = 0; //safety check
	entid = entity_list.CreateGroundObjectFromModel(model, position, type, decay_time);
	return entid;
}

void QuestManager::ModifyNPCStat(const char *identifier, const char *newValue)
{
	QuestManagerCurrentQuestVars();
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
	QuestManagerCurrentQuestVars();
	EQ::ItemInstance *item = nullptr;
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

	for (slot_id = EQ::invslot::GENERAL_BEGIN; slot_id <= EQ::invslot::GENERAL_END; ++slot_id)
	{
		quantity += collectitems_processSlot(slot_id, item_id, remove);
	}

	for (slot_id = EQ::invbag::GENERAL_BAGS_BEGIN; slot_id <= EQ::invbag::GENERAL_BAGS_END; ++slot_id)
	{
		quantity += collectitems_processSlot(slot_id, item_id, remove);
	}

	return quantity;
}

int QuestManager::countitem(uint32 item_id) {
	QuestManagerCurrentQuestVars();
	return initiator->CountItem(item_id);
}

void QuestManager::removeitem(uint32 item_id, uint32 quantity) {
	QuestManagerCurrentQuestVars();
	initiator->RemoveItem(item_id, quantity);
}

void QuestManager::UpdateSpawnTimer(uint32 id, uint32 newTime)
{
	bool found = false;

	database.UpdateRespawnTime(id, 0, (newTime/1000));
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
		auto pack = new ServerPacket(ServerOP_UpdateSpawn, sizeof(UpdateSpawnTimer_Struct));
		UpdateSpawnTimer_Struct *ust = (UpdateSpawnTimer_Struct*) pack->pBuffer;
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

	const EQ::ItemData* item = nullptr;
	item = database.GetItem(itemid);
	if (!item) return;		// if the item id doesn't correspond to a real item, do nothing

	zone->SaveTempItem(merchant->CastToNPC()->MerchantType, NPCid, itemid, quantity);
}

uint32 QuestManager::MerchantCountItem(uint32 NPCid, uint32 itemid) {
	Mob* merchant = entity_list.GetMobByNpcTypeID(NPCid);

	if (merchant == 0 || !merchant->IsNPC() || (merchant->GetClass() != MERCHANT))
		return 0;	// if it isn't a merchant, it doesn't have any items

	const EQ::ItemData* item = nullptr;
	item = database.GetItem(itemid);
	if (!item)
		return 0;	// if it isn't a valid item, the merchant doesn't have any

	// look for the item in the merchant's temporary list
	std::list<TempMerchantList> MerchList = zone->tmpmerchanttable[NPCid];
	std::list<TempMerchantList>::const_iterator itr;
	uint32 Quant = 0;

	for (itr = MerchList.begin(); itr != MerchList.end(); ++itr) {
		if (itr->item == itemid) {	// if this is the item we're looking for
			Quant = itr->charges;
			break;
		}
	}

	return Quant;	// return the quantity of itemid (0 if it was never found)
}

// Item Link for use in Variables - "my $example_link = quest::varlink(item_id);"
const char* QuestManager::varlink(char* perltext, int item_id) {
	QuestManagerCurrentQuestVars();
	const EQ::ItemData* item = database.GetItem(item_id);
	if (!item)
		return "INVALID ITEM ID IN VARLINK";

	EQ::SayLinkEngine linker;
	linker.SetLinkType(EQ::saylink::SayLinkItemData);
	linker.SetItemData(item);

	strcpy(perltext, linker.GenerateLink().c_str());

	return perltext;
}

std::string QuestManager::getitemname(uint32 item_id) {
	const EQ::ItemData* item_data = database.GetItem(item_id);
	if (!item_data) {
		return "INVALID ITEM ID IN GETITEMNAME";
	}

	std::string item_name = item_data->Name;
	return item_name;
}

std::string QuestManager::getnpcnamebyid(uint32 npc_id) {
	std::string res;
	if (npc_id > 0) {
		res = database.GetNPCNameByID(npc_id);
	}
	return res;
}

std::string QuestManager::getcleannpcnamebyid(uint32 npc_id) {
	std::string res;
	if (npc_id > 0) {
		res = database.GetCleanNPCNameByID(npc_id);
	}
	return res;
}

uint16 QuestManager::CreateInstance(const char *zone, int16 version, uint32 duration)
{
	QuestManagerCurrentQuestVars();
	if(initiator)
	{
		uint32 zone_id = ZoneID(zone);
		if(zone_id == 0)
			return 0;

		uint16 id = 0;
		if(!database.GetUnusedInstanceID(id))
		{
			initiator->Message(Chat::Red, "Server was unable to find a free instance id.");
			return 0;
		}

		if(!database.CreateInstance(id, zone_id, version, duration))
		{
			initiator->Message(Chat::Red, "Server was unable to create a new instance.");
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

void QuestManager::UpdateInstanceTimer(uint16 instance_id, uint32 new_duration)
{
	std::string query = StringFormat("UPDATE instance_list SET duration = %lu, start_time = UNIX_TIMESTAMP() WHERE id = %lu",
		(unsigned long)new_duration, (unsigned long)instance_id);
	auto results = database.QueryDatabase(query);

	if (results.Success()) {
		auto pack = new ServerPacket(ServerOP_InstanceUpdateTime, sizeof(ServerInstanceUpdateTime_Struct));
		ServerInstanceUpdateTime_Struct *ut = (ServerInstanceUpdateTime_Struct*) pack->pBuffer;
		ut->instance_id = instance_id;
		ut->new_duration = new_duration;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

uint32 QuestManager::GetInstanceTimer() {
	if (zone && zone->GetInstanceID() > 0 && zone->GetInstanceTimer()) {
		uint32 ttime = zone->GetInstanceTimer()->GetRemainingTime();
		return ttime;
	}
	return 0;
}

uint32 QuestManager::GetInstanceTimerByID(uint16 instance_id) {
	if (instance_id == 0)
		return 0;

	std::string query = StringFormat("SELECT ((start_time + duration) - UNIX_TIMESTAMP()) AS `remaining` FROM `instance_list` WHERE `id` = %lu", (unsigned long)instance_id);
	auto results = database.QueryDatabase(query);

	if (results.Success()) {
		auto row = results.begin();
		uint32 timer = atoi(row[0]);
		return timer;
	}
	return 0;
}

uint16 QuestManager::GetInstanceID(const char *zone, int16 version)
{
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		return database.GetInstanceID(ZoneID(zone), initiator->CharacterID(), version);
	}
	return 0;
}

uint16 QuestManager::GetInstanceIDByCharID(const char *zone, int16 version, uint32 char_id) {
	return database.GetInstanceID(ZoneID(zone), char_id, version);
}

void QuestManager::AssignToInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		database.AddClientToInstance(instance_id, initiator->CharacterID());
	}
}

void QuestManager::AssignToInstanceByCharID(uint16 instance_id, uint32 char_id) {
	database.AddClientToInstance(instance_id, char_id);
}

void QuestManager::AssignGroupToInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		Group *g = initiator->GetGroup();
		if (g)
		{
			uint32 gid = g->GetID();
			database.AssignGroupToInstance(gid, instance_id);
		}
	}
}

void QuestManager::AssignRaidToInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		Raid *r = initiator->GetRaid();
		if(r)
		{
			uint32 rid = r->GetID();
			database.AssignRaidToInstance(rid, instance_id);
		}
	}
}

void QuestManager::RemoveFromInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		if (database.RemoveClientFromInstance(instance_id, initiator->CharacterID()))
			initiator->Message(Chat::Say, "Removed client from instance.");
		else
			initiator->Message(Chat::Say, "Failed to remove client from instance.");
	}
}

void QuestManager::RemoveFromInstanceByCharID(uint16 instance_id, uint32 char_id) {
	database.RemoveClientFromInstance(instance_id, char_id);
}

bool QuestManager::CheckInstanceByCharID(uint16 instance_id, uint32 char_id) {
	return database.CharacterInInstanceGroup(instance_id, char_id);
}

void QuestManager::RemoveAllFromInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		std::list<uint32> charid_list;

		if (database.RemoveClientsFromInstance(instance_id))
			initiator->Message(Chat::Say, "Removed all players from instance.");
		else
		{
			database.GetCharactersInInstance(instance_id, charid_list);
			initiator->Message(Chat::Say, "Failed to remove %i player(s) from instance.", charid_list.size()); // once the expedition system is in, this message it not relevant
		}
	}
}

void QuestManager::MovePCInstance(int zone_id, int instance_id, const glm::vec4& position)
{
	QuestManagerCurrentQuestVars();
	if(initiator)
	{
		initiator->MovePC(zone_id, instance_id, position.x, position.y, position.z, position.w);
	}
}

void QuestManager::FlagInstanceByGroupLeader(uint32 zone, int16 version)
{
	QuestManagerCurrentQuestVars();
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
	QuestManagerCurrentQuestVars();
	if(initiator)
	{
		Raid *r = initiator->GetRaid();
		if(r)
		{
			database.FlagInstanceByRaidLeader(zone, version, initiator->CharacterID(), r->GetID());
		}
	}
}

std::string QuestManager::saylink(char *saylink_text, bool silent, const char *link_name)
{
	QuestManagerCurrentQuestVars();

	return EQ::SayLinkEngine::GenerateQuestSaylink(saylink_text, silent, link_name);
}

std::string QuestManager::getcharnamebyid(uint32 char_id) {
	std::string res;
	if (char_id > 0) {
		res = database.GetCharNameByID(char_id);
	}
	return res;
}

uint32 QuestManager::getcharidbyname(const char* name) {
	return database.GetCharacterID(name);
}

std::string QuestManager::getclassname(uint8 class_id, uint8 level) {
	return GetClassIDName(class_id, level);
}

int QuestManager::getcurrencyid(uint32 item_id) {
	auto iter = zone->AlternateCurrencies.begin();
	while (iter != zone->AlternateCurrencies.end()) {
		if (item_id == (*iter).item_id) {
			return (*iter).id;
		}
		++iter;
	}
	return 0;
}

int QuestManager::getcurrencyitemid(int currency_id) {
	if (currency_id > 0) {
		auto iter = zone->AlternateCurrencies.begin();
		while (iter != zone->AlternateCurrencies.end()) {
			if (currency_id == (*iter).id) {
				return (*iter).item_id;
			}
			++iter;
		}
	}
	return 0;
}

const char* QuestManager::getguildnamebyid(int guild_id) {
	if (guild_id > 0)
		return guild_mgr.GetGuildName(guild_id);
	else
		return("");
}

int QuestManager::getguildidbycharid(uint32 char_id) {
    if (char_id > 0) {
        return database.GetGuildIDByCharID(char_id);
    }
    return 0;
}

int QuestManager::getgroupidbycharid(uint32 char_id) {
    if (char_id > 0) {
        return database.GetGroupIDByCharID(char_id);
    }
    return 0;
}

int QuestManager::getraididbycharid(uint32 char_id) {
    if (char_id > 0) {
        return database.GetRaidIDByCharID(char_id);
    }
    return 0;
}

void QuestManager::SetRunning(bool val)
{
	QuestManagerCurrentQuestVars();
	if(!owner)
		return;
	owner->SetRunning(val);
}

bool QuestManager::IsRunning()
{
	QuestManagerCurrentQuestVars();
	if(!owner)
		return false;
	return owner->IsRunning();
}

void QuestManager::FlyMode(GravityBehavior flymode)
{
	QuestManagerCurrentQuestVars();
	if(initiator)
	{
		initiator->SendAppearancePacket(AT_Levitate, static_cast<int>(flymode));
		initiator->SetFlyMode(flymode);
	}
	else if(owner)
	{
		owner->SendAppearancePacket(AT_Levitate, static_cast<int>(flymode));
		owner->SetFlyMode(flymode);
	}
}

uint8 QuestManager::FactionValue()
{
	QuestManagerCurrentQuestVars();
	FACTION_VALUE oldfac;
	uint8 newfac = 0;
	if(initiator && owner->IsNPC()) {
		oldfac = initiator->GetFactionLevel(initiator->GetID(), owner->GetID(), initiator->GetFactionRace(), initiator->GetClass(), initiator->GetDeity(), owner->GetPrimaryFaction(), owner);

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
	QuestManagerCurrentQuestVars();
	initiator->EnableTitle(titleset);
}

bool QuestManager::checktitle(int titleset) {
	QuestManagerCurrentQuestVars();
	return initiator ? initiator->CheckTitle(titleset) : false;
}

void QuestManager::removetitle(int titleset) {
	QuestManagerCurrentQuestVars();
	initiator->RemoveTitle(titleset);
}

void QuestManager::wearchange(uint8 slot, uint16 texture, uint32 hero_forge_model /*= 0*/, uint32 elite_material /*= 0*/)
{
	QuestManagerCurrentQuestVars();
	if(owner){
		owner->SendTextureWC(slot, texture, hero_forge_model, elite_material);
		if(owner->IsNPC()) {
			owner->CastToNPC()->NPCSlotTexture(slot, texture);
		}
	}
}

void QuestManager::voicetell(const char *str, int macronum, int racenum, int gendernum)
{
	QuestManagerCurrentQuestVars();
	if(owner && str)
	{
		Client *c = entity_list.GetClientByName(str);

		if(c)
		{
			auto outapp = new EQApplicationPacket(OP_VoiceMacroOut, sizeof(VoiceMacroOut_Struct));

			VoiceMacroOut_Struct* vmo = (VoiceMacroOut_Struct*)outapp->pBuffer;

			strn0cpy(vmo->From, owner->GetCleanName(), sizeof(vmo->From));

			vmo->Type = 1;

			vmo->Voice = (racenum * 2) + gendernum;

			vmo->MacroNumber = macronum;

			c->QueuePacket(outapp);

			safe_delete(outapp);
		}
		else
			LogQuests("QuestManager::voicetell from [{}]. Client [{}] not found", owner->GetName(), str);
	}
}

void QuestManager::LearnRecipe(uint32 recipe_id) {
	QuestManagerCurrentQuestVars();
	if(!initiator)
		return;
	initiator->LearnRecipe(recipe_id);
}

void QuestManager::SendMail(const char *to, const char *from, const char *subject, const char *message) {
	if(to == nullptr || from == nullptr || subject == nullptr || message == nullptr) {
		return;
	}

	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_UCSMailMessage, sizeof(ServerMailMessageHeader_Struct) + message_len);
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
	entid = entity_list.CreateDoor(model, glm::vec4(x, y, z, heading), opentype, size);
	return entid;
}

int32 QuestManager::GetZoneID(const char *zone) {
	return static_cast<int32>(ZoneID(zone));
}

std::string QuestManager::GetZoneLongName(std::string zone_short_name)
{
	return ZoneLongName(ZoneID(zone_short_name));
}

std::string QuestManager::GetZoneLongNameByID(uint32 zone_id) {
	return ZoneLongName(zone_id);
}

std::string QuestManager::GetZoneShortName(uint32 zone_id) {
	return ZoneName(zone_id);
}

void QuestManager::CrossZoneAssignTaskByCharID(int character_id, uint32 task_id, bool enforce_level_requirement) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskAssignPlayer, sizeof(CZTaskAssignPlayer_Struct));
		CZTaskAssignPlayer_Struct* CZTA = (CZTaskAssignPlayer_Struct*) pack->pBuffer;
		CZTA->npc_entity_id = owner->GetID();
		CZTA->character_id = character_id;
		CZTA->task_id = task_id;
		CZTA->enforce_level_requirement = enforce_level_requirement;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneAssignTaskByGroupID(int group_id, uint32 task_id, bool enforce_level_requirement) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskAssignGroup, sizeof(CZTaskAssignGroup_Struct));
		CZTaskAssignGroup_Struct* CZTA = (CZTaskAssignGroup_Struct*) pack->pBuffer;
		CZTA->npc_entity_id = owner->GetID();
		CZTA->group_id = group_id;
		CZTA->task_id = task_id;
		CZTA->enforce_level_requirement = enforce_level_requirement;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneAssignTaskByRaidID(int raid_id, uint32 task_id, bool enforce_level_requirement) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskAssignRaid, sizeof(CZTaskAssignRaid_Struct));
		CZTaskAssignRaid_Struct* CZTA = (CZTaskAssignRaid_Struct*) pack->pBuffer;
		CZTA->npc_entity_id = owner->GetID();
		CZTA->raid_id = raid_id;
		CZTA->task_id = task_id;
		CZTA->enforce_level_requirement = enforce_level_requirement;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneAssignTaskByGuildID(int guild_id, uint32 task_id, bool enforce_level_requirement) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskAssignGuild, sizeof(CZTaskAssignGuild_Struct));
		CZTaskAssignGuild_Struct* CZTA = (CZTaskAssignGuild_Struct*) pack->pBuffer;
		CZTA->npc_entity_id = owner->GetID();
		CZTA->guild_id = guild_id;
		CZTA->task_id = task_id;
		CZTA->enforce_level_requirement = enforce_level_requirement;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneCastSpellByCharID(int character_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZCastSpellPlayer, sizeof(CZCastSpellPlayer_Struct));
	CZCastSpellPlayer_Struct* CZCS = (CZCastSpellPlayer_Struct*) pack->pBuffer;
	CZCS->character_id = character_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneCastSpellByGroupID(int group_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZCastSpellGroup, sizeof(CZCastSpellGroup_Struct));
	CZCastSpellGroup_Struct* CZCS = (CZCastSpellGroup_Struct*) pack->pBuffer;
	CZCS->group_id = group_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneCastSpellByRaidID(int raid_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZCastSpellRaid, sizeof(CZCastSpellRaid_Struct));
	CZCastSpellRaid_Struct* CZCS = (CZCastSpellRaid_Struct*) pack->pBuffer;
	CZCS->raid_id = raid_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneCastSpellByGuildID(int guild_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZCastSpellGuild, sizeof(CZCastSpellGuild_Struct));
	CZCastSpellGuild_Struct* CZCS = (CZCastSpellGuild_Struct*) pack->pBuffer;
	CZCS->guild_id = guild_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneDisableTaskByCharID(int character_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskDisablePlayer, sizeof(CZTaskDisablePlayer_Struct));
	CZTaskDisablePlayer_Struct* CZTD = (CZTaskDisablePlayer_Struct*) pack->pBuffer;
	CZTD->character_id = character_id;
	CZTD->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneDisableTaskByGroupID(int group_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskDisableGroup, sizeof(CZTaskDisableGroup_Struct));
	CZTaskDisableGroup_Struct* CZTD = (CZTaskDisableGroup_Struct*) pack->pBuffer;
	CZTD->group_id = group_id;
	CZTD->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneDisableTaskByRaidID(int raid_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskDisableRaid, sizeof(CZTaskDisableRaid_Struct));
	CZTaskDisableRaid_Struct* CZTD = (CZTaskDisableRaid_Struct*) pack->pBuffer;
	CZTD->raid_id = raid_id;
	CZTD->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneDisableTaskByGuildID(int guild_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskDisableGuild, sizeof(CZTaskDisableGuild_Struct));
	CZTaskDisableGuild_Struct* CZTD = (CZTaskDisableGuild_Struct*) pack->pBuffer;
	CZTD->guild_id = guild_id;
	CZTD->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneEnableTaskByCharID(int character_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskEnablePlayer, sizeof(CZTaskEnablePlayer_Struct));
	CZTaskEnablePlayer_Struct* CZTE = (CZTaskEnablePlayer_Struct*) pack->pBuffer;
	CZTE->character_id = character_id;
	CZTE->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneEnableTaskByGroupID(int group_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskEnableGroup, sizeof(CZTaskEnableGroup_Struct));
	CZTaskEnableGroup_Struct* CZTE = (CZTaskEnableGroup_Struct*) pack->pBuffer;
	CZTE->group_id = group_id;
	CZTE->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneEnableTaskByRaidID(int raid_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskEnableRaid, sizeof(CZTaskEnableRaid_Struct));
	CZTaskEnableRaid_Struct* CZTE = (CZTaskEnableRaid_Struct*) pack->pBuffer;
	CZTE->raid_id = raid_id;
	CZTE->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneEnableTaskByGuildID(int guild_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskEnableGuild, sizeof(CZTaskEnableGuild_Struct));
	CZTaskEnableGuild_Struct* CZTE = (CZTaskEnableGuild_Struct*) pack->pBuffer;
	CZTE->guild_id = guild_id;
	CZTE->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneFailTaskByCharID(int character_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskFailPlayer, sizeof(CZTaskFailPlayer_Struct));
	CZTaskFailPlayer_Struct* CZTF = (CZTaskFailPlayer_Struct*) pack->pBuffer;
	CZTF->character_id = character_id;
	CZTF->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneFailTaskByGroupID(int group_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskFailGroup, sizeof(CZTaskFailGroup_Struct));
	CZTaskFailGroup_Struct* CZTF = (CZTaskFailGroup_Struct*) pack->pBuffer;
	CZTF->group_id = group_id;
	CZTF->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneFailTaskByRaidID(int raid_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskFailRaid, sizeof(CZTaskFailRaid_Struct));
	CZTaskFailRaid_Struct* CZTF = (CZTaskFailRaid_Struct*) pack->pBuffer;
	CZTF->raid_id = raid_id;
	CZTF->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneFailTaskByGuildID(int guild_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskFailGuild, sizeof(CZTaskFailGuild_Struct));
	CZTaskFailGuild_Struct* CZTF = (CZTaskFailGuild_Struct*) pack->pBuffer;
	CZTF->guild_id = guild_id;
	CZTF->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMarqueeByCharID(int character_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMarqueePlayer, sizeof(CZMarqueePlayer_Struct) + message_len);
	CZMarqueePlayer_Struct* CZMS = (CZMarqueePlayer_Struct*) pack->pBuffer;
	CZMS->character_id = character_id;
	CZMS->type = type;
	CZMS->priority = priority;
	CZMS->fade_in = fade_in;
	CZMS->fade_out = fade_out;
	CZMS->duration = duration;
	strn0cpy(CZMS->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMarqueeByGroupID(int group_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMarqueeGroup, sizeof(CZMarqueeGroup_Struct) + message_len);
	CZMarqueeGroup_Struct* CZMS = (CZMarqueeGroup_Struct*) pack->pBuffer;
	CZMS->group_id = group_id;
	CZMS->type = type;
	CZMS->priority = priority;
	CZMS->fade_in = fade_in;
	CZMS->fade_out = fade_out;
	CZMS->duration = duration;
	strn0cpy(CZMS->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMarqueeByRaidID(int raid_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMarqueeRaid, sizeof(CZMarqueeRaid_Struct) + message_len);
	CZMarqueeRaid_Struct* CZMS = (CZMarqueeRaid_Struct*) pack->pBuffer;
	CZMS->raid_id = raid_id;
	CZMS->type = type;
	CZMS->priority = priority;
	CZMS->fade_in = fade_in;
	CZMS->fade_out = fade_out;
	CZMS->duration = duration;
	strn0cpy(CZMS->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMarqueeByGuildID(int guild_id, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMarqueeGuild, sizeof(CZMarqueeGuild_Struct) + message_len);
	CZMarqueeGuild_Struct* CZMS = (CZMarqueeGuild_Struct*) pack->pBuffer;
	CZMS->guild_id = guild_id;
	CZMS->type = type;
	CZMS->priority = priority;
	CZMS->fade_in = fade_in;
	CZMS->fade_out = fade_out;
	CZMS->duration = duration;
	strn0cpy(CZMS->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMessagePlayerByName(uint32 type, const char *character_name, const char *message) {
	uint32 message_len = strlen(character_name) + 1;
	uint32 message_len2 = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMessagePlayer, sizeof(CZMessagePlayer_Struct) + message_len + message_len2);
	CZMessagePlayer_Struct* CZSC = (CZMessagePlayer_Struct*) pack->pBuffer;
	CZSC->type = type;
	strn0cpy(CZSC->character_name, character_name, 64);
	strn0cpy(CZSC->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMessagePlayerByGroupID(uint32 type, int group_id, const char *message) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMessageGroup, sizeof(CZMessageGroup_Struct) + message_len);
	CZMessageGroup_Struct* CZGM = (CZMessageGroup_Struct*) pack->pBuffer;
	CZGM->type = type;
	CZGM->group_id = group_id;
	strn0cpy(CZGM->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMessagePlayerByRaidID(uint32 type, int raid_id, const char *message) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMessageRaid, sizeof(CZMessageRaid_Struct) + message_len);
	CZMessageRaid_Struct* CZRM = (CZMessageRaid_Struct*) pack->pBuffer;
	CZRM->type = type;
	CZRM->raid_id = raid_id;
	strn0cpy(CZRM->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMessagePlayerByGuildID(uint32 type, int guild_id, const char *message) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_CZMessageGuild, sizeof(CZMessageGuild_Struct) + message_len);
	CZMessageGuild_Struct* CZGM = (CZMessageGuild_Struct*) pack->pBuffer;
	CZGM->type = type;
	CZGM->guild_id = guild_id;
	strn0cpy(CZGM->message, message, 512);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMovePlayerByCharID(int character_id, const char *zone_short_name) {
	uint32 message_len = strlen(zone_short_name) + 1;
	auto pack = new ServerPacket(ServerOP_CZMovePlayer, sizeof(CZMovePlayer_Struct) + message_len);
	CZMovePlayer_Struct* CZGM = (CZMovePlayer_Struct*) pack->pBuffer;
	CZGM->character_id = character_id;
	strn0cpy(CZGM->zone_short_name, zone_short_name, 32);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMovePlayerByGroupID(int group_id, const char *zone_short_name) {
	uint32 message_len = strlen(zone_short_name) + 1;
	auto pack = new ServerPacket(ServerOP_CZMoveGroup, sizeof(CZMoveGroup_Struct) + message_len);
	CZMoveGroup_Struct* CZGM = (CZMoveGroup_Struct*) pack->pBuffer;
	CZGM->group_id = group_id;
	strn0cpy(CZGM->zone_short_name, zone_short_name, 32);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMovePlayerByRaidID(int raid_id, const char *zone_short_name) {
	uint32 message_len = strlen(zone_short_name) + 1;
	auto pack = new ServerPacket(ServerOP_CZMoveRaid, sizeof(CZMoveRaid_Struct) + message_len);
	CZMoveRaid_Struct* CZRM = (CZMoveRaid_Struct*) pack->pBuffer;
	CZRM->raid_id = raid_id;
	strn0cpy(CZRM->zone_short_name, zone_short_name, 32);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMovePlayerByGuildID(int guild_id, const char *zone_short_name) {
	uint32 message_len = strlen(zone_short_name) + 1;
	auto pack = new ServerPacket(ServerOP_CZMoveGuild, sizeof(CZMoveGuild_Struct) + message_len);
	CZMoveGuild_Struct* CZGM = (CZMoveGuild_Struct*) pack->pBuffer;
	CZGM->guild_id = guild_id;
	strn0cpy(CZGM->zone_short_name, zone_short_name, 32);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMoveInstanceByCharID(int character_id, uint16 instance_id) {
	auto pack = new ServerPacket(ServerOP_CZMoveInstancePlayer, sizeof(CZMoveInstancePlayer_Struct));
	CZMoveInstancePlayer_Struct* CZMS = (CZMoveInstancePlayer_Struct*) pack->pBuffer;
	CZMS->character_id = character_id;
	CZMS->instance_id = instance_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMoveInstanceByGroupID(int group_id, uint16 instance_id) {
	auto pack = new ServerPacket(ServerOP_CZMoveInstanceGroup, sizeof(CZMoveInstanceGroup_Struct));
	CZMoveInstanceGroup_Struct* CZMS = (CZMoveInstanceGroup_Struct*) pack->pBuffer;
	CZMS->group_id = group_id;
	CZMS->instance_id = instance_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMoveInstanceByRaidID(int raid_id, uint16 instance_id) {
	auto pack = new ServerPacket(ServerOP_CZMoveInstanceRaid, sizeof(CZMoveInstanceRaid_Struct));
	CZMoveInstanceRaid_Struct* CZMS = (CZMoveInstanceRaid_Struct*) pack->pBuffer;
	CZMS->raid_id = raid_id;
	CZMS->instance_id = instance_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMoveInstanceByGuildID(int guild_id, uint16 instance_id) {
	auto pack = new ServerPacket(ServerOP_CZMoveInstanceGuild, sizeof(CZMoveInstanceGuild_Struct));
	CZMoveInstanceGuild_Struct* CZMS = (CZMoveInstanceGuild_Struct*) pack->pBuffer;
	CZMS->guild_id = guild_id;
	CZMS->instance_id = instance_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveSpellByCharID(int character_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZRemoveSpellPlayer, sizeof(CZRemoveSpellPlayer_Struct));
	CZRemoveSpellPlayer_Struct* CZCS = (CZRemoveSpellPlayer_Struct*) pack->pBuffer;
	CZCS->character_id = character_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveSpellByGroupID(int group_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZRemoveSpellGroup, sizeof(CZRemoveSpellGroup_Struct));
	CZRemoveSpellGroup_Struct* CZCS = (CZRemoveSpellGroup_Struct*) pack->pBuffer;
	CZCS->group_id = group_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveSpellByRaidID(int raid_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZRemoveSpellRaid, sizeof(CZRemoveSpellRaid_Struct));
	CZRemoveSpellRaid_Struct* CZCS = (CZRemoveSpellRaid_Struct*) pack->pBuffer;
	CZCS->raid_id = raid_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveSpellByGuildID(int guild_id, uint32 spell_id) {
	auto pack = new ServerPacket(ServerOP_CZRemoveSpellGuild, sizeof(CZRemoveSpellGuild_Struct));
	CZRemoveSpellGuild_Struct* CZCS = (CZRemoveSpellGuild_Struct*) pack->pBuffer;
	CZCS->guild_id = guild_id;
	CZCS->spell_id = spell_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveTaskByCharID(int character_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskRemovePlayer, sizeof(CZTaskRemovePlayer_Struct));
	CZTaskRemovePlayer_Struct* CZCS = (CZTaskRemovePlayer_Struct*) pack->pBuffer;
	CZCS->character_id = character_id;
	CZCS->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveTaskByGroupID(int group_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskRemoveGroup, sizeof(CZTaskRemoveGroup_Struct));
	CZTaskRemoveGroup_Struct* CZCS = (CZTaskRemoveGroup_Struct*) pack->pBuffer;
	CZCS->group_id = group_id;
	CZCS->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveTaskByRaidID(int raid_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskRemoveRaid, sizeof(CZTaskRemoveRaid_Struct));
	CZTaskRemoveRaid_Struct* CZCS = (CZTaskRemoveRaid_Struct*) pack->pBuffer;
	CZCS->raid_id = raid_id;
	CZCS->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneRemoveTaskByGuildID(int guild_id, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_CZTaskRemoveGuild, sizeof(CZTaskRemoveGuild_Struct));
	CZTaskRemoveGuild_Struct* CZCS = (CZTaskRemoveGuild_Struct*) pack->pBuffer;
	CZCS->guild_id = guild_id;
	CZCS->task_id = task_id;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneResetActivityByCharID(int character_id, uint32 task_id, int activity_id) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityResetPlayer, sizeof(CZResetActivityPlayer_Struct));
		CZResetActivityPlayer_Struct* CZCA = (CZResetActivityPlayer_Struct*) pack->pBuffer;
		CZCA->character_id = character_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneResetActivityByGroupID(int group_id, uint32 task_id, int activity_id) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityResetGroup, sizeof(CZResetActivityGroup_Struct));
		CZResetActivityGroup_Struct* CZCA = (CZResetActivityGroup_Struct*) pack->pBuffer;
		CZCA->group_id = group_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneResetActivityByRaidID(int raid_id, uint32 task_id, int activity_id) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityResetRaid, sizeof(CZResetActivityRaid_Struct));
		CZResetActivityRaid_Struct* CZCA = (CZResetActivityRaid_Struct*) pack->pBuffer;
		CZCA->raid_id = raid_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneResetActivityByGuildID(int guild_id, uint32 task_id, int activity_id) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityResetGuild, sizeof(CZResetActivityGuild_Struct));
		CZResetActivityGuild_Struct* CZCA = (CZResetActivityGuild_Struct*) pack->pBuffer;
		CZCA->guild_id = guild_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneSignalNPCByNPCTypeID(uint32 npctype_id, uint32 signal) {
	auto pack = new ServerPacket(ServerOP_CZSignalNPC, sizeof(CZNPCSignal_Struct));
	CZNPCSignal_Struct* CZSN = (CZNPCSignal_Struct*) pack->pBuffer;
	CZSN->npctype_id = npctype_id;
	CZSN->signal = signal;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignalPlayerByCharID(int character_id, uint32 signal) {
	auto pack = new ServerPacket(ServerOP_CZSignalClient, sizeof(CZClientSignal_Struct));
	CZClientSignal_Struct* CZSC = (CZClientSignal_Struct*) pack->pBuffer;
	CZSC->character_id = character_id;
	CZSC->signal = signal;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSetEntityVariableByClientName(const char *character_name, const char *variable_name, const char *variable_value) {
	uint32 message_len = strlen(variable_name) + 1;
	uint32 message_len2 = strlen(variable_value) + 1;
	uint32 message_len3 = strlen(character_name) + 1;
	auto pack = new ServerPacket(ServerOP_CZSetEntityVariableByClientName, sizeof(CZSetEntVarByClientName_Struct) + message_len + message_len2 + message_len3);
	CZSetEntVarByClientName_Struct* CZ = (CZSetEntVarByClientName_Struct*) pack->pBuffer;
	strn0cpy(CZ->character_name, character_name, 64);
	strn0cpy(CZ->variable_name, variable_name, 256);
	strn0cpy(CZ->variable_value, variable_value, 256);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSetEntityVariableByGroupID(int group_id, const char *variable_name, const char *variable_value) {
	uint32 message_len = strlen(variable_name) + 1;
	uint32 message_len2 = strlen(variable_value) + 1;
	auto pack = new ServerPacket(ServerOP_CZSetEntityVariableByGroupID, sizeof(CZSetEntVarByGroupID_Struct) + message_len + message_len2);
	CZSetEntVarByGroupID_Struct* CZ = (CZSetEntVarByGroupID_Struct*) pack->pBuffer;
	CZ->group_id = group_id;
	strn0cpy(CZ->variable_name, variable_name, 256);
	strn0cpy(CZ->variable_value, variable_value, 256);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSetEntityVariableByRaidID(int raid_id, const char *variable_name, const char *variable_value) {
	uint32 message_len = strlen(variable_name) + 1;
	uint32 message_len2 = strlen(variable_value) + 1;
	auto pack = new ServerPacket(ServerOP_CZSetEntityVariableByRaidID, sizeof(CZSetEntVarByRaidID_Struct) + message_len + message_len2);
	CZSetEntVarByRaidID_Struct* CZ = (CZSetEntVarByRaidID_Struct*) pack->pBuffer;
	CZ->raid_id = raid_id;
	strn0cpy(CZ->variable_name, variable_name, 256);
	strn0cpy(CZ->variable_value, variable_value, 256);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSetEntityVariableByGuildID(int guild_id, const char *variable_name, const char *variable_value) {
	uint32 message_len = strlen(variable_name) + 1;
	uint32 message_len2 = strlen(variable_value) + 1;
	auto pack = new ServerPacket(ServerOP_CZSetEntityVariableByGuildID, sizeof(CZSetEntVarByGuildID_Struct) + message_len + message_len2);
	CZSetEntVarByGuildID_Struct* CZ = (CZSetEntVarByGuildID_Struct*) pack->pBuffer;
	CZ->guild_id = guild_id;
	strn0cpy(CZ->variable_name, variable_name, 256);
	strn0cpy(CZ->variable_value, variable_value, 256);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSetEntityVariableByNPCTypeID(uint32 npctype_id, const char *variable_name, const char *variable_value) {
	uint32 message_len = strlen(variable_name) + 1;
	uint32 message_len2 = strlen(variable_value) + 1;
	auto pack = new ServerPacket(ServerOP_CZSetEntityVariableByNPCTypeID, sizeof(CZSetEntVarByNPCTypeID_Struct) + message_len + message_len2);
	CZSetEntVarByNPCTypeID_Struct* CZSNBYNID = (CZSetEntVarByNPCTypeID_Struct*) pack->pBuffer;
	CZSNBYNID->npctype_id = npctype_id;
	strn0cpy(CZSNBYNID->variable_name, variable_name, 256);
	strn0cpy(CZSNBYNID->variable_value, variable_value, 256);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignalPlayerByGroupID(int group_id, uint32 signal) {
	auto pack = new ServerPacket(ServerOP_CZSignalGroup, sizeof(CZGroupSignal_Struct));
	CZGroupSignal_Struct* CZGS = (CZGroupSignal_Struct*) pack->pBuffer;
	CZGS->group_id = group_id;
	CZGS->signal = signal;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignalPlayerByRaidID(int raid_id, uint32 signal) {
	auto pack = new ServerPacket(ServerOP_CZSignalRaid, sizeof(CZRaidSignal_Struct));
	CZRaidSignal_Struct* CZRS = (CZRaidSignal_Struct*) pack->pBuffer;
	CZRS->raid_id = raid_id;
	CZRS->signal = signal;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignalPlayerByGuildID(int guild_id, uint32 signal) {
	auto pack = new ServerPacket(ServerOP_CZSignalGuild, sizeof(CZGuildSignal_Struct));
	CZGuildSignal_Struct* CZGS = (CZGuildSignal_Struct*) pack->pBuffer;
	CZGS->guild_id = guild_id;
	CZGS->signal = signal;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignalPlayerByName(const char *character_name, uint32 signal) {
	uint32 message_len = strlen(character_name) + 1;
	auto pack = new ServerPacket(ServerOP_CZSignalClientByName, sizeof(CZClientSignalByName_Struct) + message_len);
	CZClientSignalByName_Struct* CZSC = (CZClientSignalByName_Struct*) pack->pBuffer;
	strn0cpy(CZSC->character_name, character_name, 64);
	CZSC->signal = signal;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneUpdateActivityByCharID(int character_id, uint32 task_id, int activity_id, int activity_count) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityUpdatePlayer, sizeof(CZTaskActivityUpdatePlayer_Struct));
		CZTaskActivityUpdatePlayer_Struct* CZCA = (CZTaskActivityUpdatePlayer_Struct*) pack->pBuffer;
		CZCA->character_id = character_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		CZCA->activity_count = activity_count;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneUpdateActivityByGroupID(int group_id, uint32 task_id, int activity_id, int activity_count) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityUpdateGroup, sizeof(CZTaskActivityUpdateGroup_Struct));
		CZTaskActivityUpdateGroup_Struct* CZCA = (CZTaskActivityUpdateGroup_Struct*) pack->pBuffer;
		CZCA->group_id = group_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		CZCA->activity_count = activity_count;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneUpdateActivityByRaidID(int raid_id, uint32 task_id, int activity_id, int activity_count) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityUpdateRaid, sizeof(CZTaskActivityUpdateRaid_Struct));
		CZTaskActivityUpdateRaid_Struct* CZCA = (CZTaskActivityUpdateRaid_Struct*) pack->pBuffer;
		CZCA->raid_id = raid_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		CZCA->activity_count = activity_count;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::CrossZoneUpdateActivityByGuildID(int guild_id, uint32 task_id, int activity_id, int activity_count) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_CZTaskActivityUpdateGuild, sizeof(CZTaskActivityUpdateGuild_Struct));
		CZTaskActivityUpdateGuild_Struct* CZCA = (CZTaskActivityUpdateGuild_Struct*) pack->pBuffer;
		CZCA->guild_id = guild_id;
		CZCA->task_id = task_id;
		CZCA->activity_id = activity_id;
		CZCA->activity_count = activity_count;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::WorldWideAssignTask(uint32 task_id, bool enforce_level_requirement, uint8 min_status, uint8 max_status) {
	QuestManagerCurrentQuestVars();
	if (initiator && owner) {
		auto pack = new ServerPacket(ServerOP_WWAssignTask, sizeof(WWAssignTask_Struct));
		WWAssignTask_Struct* WWTA = (WWAssignTask_Struct*) pack->pBuffer;
		WWTA->npc_entity_id = owner->GetID();
		WWTA->task_id = task_id;
		WWTA->enforce_level_requirement = enforce_level_requirement;
		WWTA->min_status = min_status;
		WWTA->max_status = max_status;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void QuestManager::WorldWideCastSpell(uint32 spell_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWCastSpell, sizeof(WWCastSpell_Struct));
	WWCastSpell_Struct* WWCS = (WWCastSpell_Struct*) pack->pBuffer;
	WWCS->spell_id = spell_id;
	WWCS->min_status = min_status;
	WWCS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideDisableTask(uint32 task_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWDisableTask, sizeof(WWDisableTask_Struct));
	WWDisableTask_Struct* WWDT = (WWDisableTask_Struct*) pack->pBuffer;
	WWDT->task_id = task_id;
	WWDT->min_status = min_status;
	WWDT->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideEnableTask(uint32 task_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWEnableTask, sizeof(WWEnableTask_Struct));
	WWEnableTask_Struct* WWET = (WWEnableTask_Struct*) pack->pBuffer;
	WWET->task_id = task_id;
	WWET->min_status = min_status;
	WWET->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideFailTask(uint32 task_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWFailTask, sizeof(WWFailTask_Struct));
	WWFailTask_Struct* WWFT = (WWFailTask_Struct*) pack->pBuffer;
	WWFT->task_id = task_id;
	WWFT->min_status = min_status;
	WWFT->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char *message, uint8 min_status, uint8 max_status) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_WWMarquee, sizeof(WWMarquee_Struct) + message_len);
	WWMarquee_Struct* WWMS = (WWMarquee_Struct*) pack->pBuffer;
	WWMS->type = type;
	WWMS->priority = priority;
	WWMS->fade_in = fade_in;
	WWMS->fade_out = fade_out;
	WWMS->duration = duration;
	strn0cpy(WWMS->message, message, 512);
	WWMS->min_status = min_status;
	WWMS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMessage(uint32 type, const char *message, uint8 min_status, uint8 max_status) {
	uint32 message_len = strlen(message) + 1;
	auto pack = new ServerPacket(ServerOP_WWMessage, sizeof(WWMessage_Struct) + message_len);
	WWMessage_Struct* WWMS = (WWMessage_Struct*) pack->pBuffer;
	WWMS->type = type;
	strn0cpy(WWMS->message, message, 512);
	WWMS->min_status = min_status;
	WWMS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMove(const char *zone_short_name, uint8 min_status, uint8 max_status) {
	uint32 message_len = strlen(zone_short_name) + 1;
	auto pack = new ServerPacket(ServerOP_WWMove, sizeof(WWMove_Struct) + message_len);
	WWMove_Struct* WWMS = (WWMove_Struct*) pack->pBuffer;;
	strn0cpy(WWMS->zone_short_name, zone_short_name, 32);
	WWMS->min_status = min_status;
	WWMS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMoveInstance(uint16 instance_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWMoveInstance, sizeof(WWMoveInstance_Struct));
	WWMoveInstance_Struct* WWMS = (WWMoveInstance_Struct*) pack->pBuffer;
	WWMS->instance_id = instance_id;
	WWMS->min_status = min_status;
	WWMS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideRemoveSpell(uint32 spell_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWRemoveSpell, sizeof(WWRemoveSpell_Struct));
	WWRemoveSpell_Struct* WWRS = (WWRemoveSpell_Struct*) pack->pBuffer;
	WWRS->spell_id = spell_id;
	WWRS->min_status = min_status;
	WWRS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideRemoveTask(uint32 task_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWRemoveTask, sizeof(WWRemoveTask_Struct));
	WWRemoveTask_Struct* WWRT = (WWRemoveTask_Struct*) pack->pBuffer;
	WWRT->task_id = task_id;
	WWRT->min_status = min_status;
	WWRT->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideResetActivity(uint32 task_id, int activity_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWResetActivity, sizeof(WWResetActivity_Struct));
	WWResetActivity_Struct* WWRA = (WWResetActivity_Struct*) pack->pBuffer;
	WWRA->task_id = task_id;
	WWRA->activity_id = activity_id;
	WWRA->min_status = min_status;
	WWRA->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSetEntityVariableClient(const char *variable_name, const char *variable_value, uint8 min_status, uint8 max_status) {
	uint32 message_len = strlen(variable_name) + 1;
	uint32 message_len2 = strlen(variable_value) + 1;
	auto pack = new ServerPacket(ServerOP_WWSetEntityVariableClient, sizeof(WWSetEntVarClient_Struct) + message_len + message_len2);
	WWSetEntVarClient_Struct* WWSC = (WWSetEntVarClient_Struct*) pack->pBuffer;
	strn0cpy(WWSC->variable_name, variable_name, 256);
	strn0cpy(WWSC->variable_value, variable_value, 256);
	WWSC->min_status = min_status;
	WWSC->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSetEntityVariableNPC(const char *variable_name, const char *variable_value) {
	uint32 message_len = strlen(variable_name) + 1;
	uint32 message_len2 = strlen(variable_value) + 1;
	auto pack = new ServerPacket(ServerOP_WWSetEntityVariableNPC, sizeof(WWSetEntVarNPC_Struct) + message_len + message_len2);
	WWSetEntVarNPC_Struct* WWSN = (WWSetEntVarNPC_Struct*) pack->pBuffer;
	strn0cpy(WWSN->variable_name, variable_name, 256);
	strn0cpy(WWSN->variable_value, variable_value, 256);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSignalClient(uint32 signal, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWSignalClient, sizeof(WWSignalClient_Struct));
	WWSignalClient_Struct* WWSC = (WWSignalClient_Struct*) pack->pBuffer;
	WWSC->signal = signal;
	WWSC->min_status = min_status;
	WWSC->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSignalNPC(uint32 signal) {
	auto pack = new ServerPacket(ServerOP_WWSignalNPC, sizeof(WWSignalNPC_Struct));
	WWSignalNPC_Struct* WWSN = (WWSignalNPC_Struct*) pack->pBuffer;
	WWSN->signal = signal;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideUpdateActivity(uint32 task_id, int activity_id, int activity_count, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWUpdateActivity, sizeof(WWUpdateActivity_Struct));
	WWUpdateActivity_Struct* WWUA = (WWUpdateActivity_Struct*) pack->pBuffer;
	WWUA->task_id = task_id;
	WWUA->activity_id = activity_id;
	WWUA->activity_count = activity_count;
	WWUA->min_status = min_status;
	WWUA->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

bool QuestManager::EnableRecipe(uint32 recipe_id)
{
	bool success = false;
	if (recipe_id > 0)
		success = content_db.EnableRecipe(recipe_id);
	return (success);
}

bool QuestManager::DisableRecipe(uint32 recipe_id)
{
	bool success = false;
	if (recipe_id > 0)
		success = content_db.DisableRecipe(recipe_id);
	return (success);
}

void QuestManager::ClearNPCTypeCache(int npctype_id) {
	if (zone) {
		zone->ClearNPCTypeCache(npctype_id);
	}
}

void QuestManager::ReloadZoneStaticData()
{
	if (zone) {
		zone->ReloadStaticData();
	}
}

Client *QuestManager::GetInitiator() const {
	if(!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return e.initiator;
	}

	return nullptr;
}

NPC *QuestManager::GetNPC() const {
	if(!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return (e.owner && e.owner->IsNPC()) ? e.owner->CastToNPC() : nullptr;
	}

	return nullptr;
}

Mob *QuestManager::GetOwner() const {
	if(!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return e.owner;
	}

	return nullptr;
}

EQ::InventoryProfile *QuestManager::GetInventory() const {
	if(!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return &e.initiator->GetInv();
	}

	return nullptr;
}

EQ::ItemInstance *QuestManager::GetQuestItem() const {
	if(!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return e.questitem;
	}

	return nullptr;
}

std::string QuestManager::GetEncounter() const {
	if(!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return e.encounter;
	}

	return "";
}

void QuestManager::UpdateZoneHeader(std::string type, std::string value) {
	if (strcasecmp(type.c_str(), "ztype") == 0)
		zone->newzone_data.ztype = atoi(value.c_str());
	else if (strcasecmp(type.c_str(), "fog_red") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_red[i] = atoi(value.c_str());
		}
	} else if (strcasecmp(type.c_str(), "fog_green") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_green[i] = atoi(value.c_str());
		}
	} else if (strcasecmp(type.c_str(), "fog_blue") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_blue[i] = atoi(value.c_str());
		}
	} else if (strcasecmp(type.c_str(), "fog_minclip") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_minclip[i] = atof(value.c_str());
		}
	} else if (strcasecmp(type.c_str(), "fog_maxclip") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.fog_maxclip[i] = atof(value.c_str());
		}
	}
	else if (strcasecmp(type.c_str(), "gravity") == 0)
		zone->newzone_data.gravity = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "time_type") == 0)
		zone->newzone_data.time_type = atoi(value.c_str());
	else if (strcasecmp(type.c_str(), "rain_chance") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.rain_chance[i] = atoi(value.c_str());
		}
	} else if (strcasecmp(type.c_str(), "rain_duration") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.rain_duration[i] = atoi(value.c_str());
		}
	} else if (strcasecmp(type.c_str(), "snow_chance") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.snow_chance[i] = atoi(value.c_str());
		}
	} else if (strcasecmp(type.c_str(), "snow_duration") == 0) {
		for (int i = 0; i < 4; i++) {
			zone->newzone_data.snow_duration[i] = atoi(value.c_str());
		}
	}
	else if (strcasecmp(type.c_str(), "sky") == 0)
		zone->newzone_data.sky = atoi(value.c_str());
	else if (strcasecmp(type.c_str(), "safe_x") == 0)
		zone->newzone_data.safe_x = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "safe_y") == 0)
		zone->newzone_data.safe_y = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "safe_z") == 0)
		zone->newzone_data.safe_z = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "max_z") == 0)
		zone->newzone_data.max_z = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "underworld") == 0)
		zone->newzone_data.underworld = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "minclip") == 0)
		zone->newzone_data.minclip = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "maxclip") == 0)
		zone->newzone_data.maxclip = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "fog_density") == 0)
		zone->newzone_data.fog_density = atof(value.c_str());
	else if (strcasecmp(type.c_str(), "suspendbuffs") == 0)
		zone->newzone_data.SuspendBuffs = atoi(value.c_str());

	auto outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	memcpy(outapp->pBuffer, &zone->newzone_data, outapp->size);
	entity_list.QueueClients(0, outapp);
	safe_delete(outapp);
}

EQ::ItemInstance *QuestManager::CreateItem(uint32 item_id, int16 charges, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six, bool attuned) const {
	if (database.GetItem(item_id)) {
		return database.CreateItem(item_id, charges, augment_one, augment_two, augment_three, augment_four, augment_five, augment_six, attuned);
	}
	return nullptr;
}

std::string QuestManager::secondstotime(int duration) {
	int timer_length = duration;
	int hours = int(timer_length / 3600);
	timer_length %= 3600;
	int minutes = int(timer_length / 60);
	timer_length %= 60;
	int seconds = timer_length;
	std::string time_string = "Unknown";
	std::string hour_string = (hours == 1 ? "Hour" : "Hours");
	std::string minute_string = (minutes == 1 ? "Minute" : "Minutes");
	std::string second_string = (seconds == 1 ? "Second" : "Seconds");
	if (hours > 0 && minutes > 0 && seconds > 0) {
		time_string = fmt::format("{} {}, {} {}, and {} {}", hours, hour_string, minutes, minute_string, seconds, second_string);
	} else if (hours > 0 && minutes > 0 && seconds == 0) {
		time_string = fmt::format("{} {} and {} {}", hours, hour_string, minutes, minute_string);
	} else if (hours > 0 && minutes == 0 && seconds > 0) {
		time_string = fmt::format("{} {} and {} {}", hours, hour_string, seconds, second_string);
	} else if (hours > 0 && minutes == 0 && seconds == 0) {
		time_string = fmt::format("{} {}", hours, hour_string);
	} else if (hours == 0 && minutes > 0 && seconds > 0) {
		time_string = fmt::format("{} {} and {} {}", minutes, minute_string, seconds, second_string);
	} else if (hours == 0 && minutes > 0 && seconds == 0) {
		time_string = fmt::format("{} {}", minutes, minute_string);
	} else if (hours == 0 && minutes == 0 && seconds > 0) {
		time_string = fmt::format("{} {}", seconds, second_string);
	}
	return time_string;
}

std::string QuestManager::gethexcolorcode(std::string color_name) {
	static const std::map<std::string, std::string> colors = {
		{ "Black", "#000000" },
		{ "Brown", "#804000" },
		{ "Burgundy", "#800000" },
		{ "Cadet Blue", "#77BFC7" },
		{ "Cadet Blue1", "#4C787E" },
		{ "Chartreuse", "#8AFB17" },
		{ "Chartreuse1", "#7FE817" },
		{ "Chartreuse2", "#6CC417" },
		{ "Chartreuse3", "#437C17" },
		{ "Chocolate", "#C85A17" },
		{ "Coral", "#F76541" },
		{ "Coral1", "#E55B3C" },
		{ "Coral2", "#C34A2C" },
		{ "Cornflower Blue", "#151B8D" },
		{ "Cyan", "#00FFFF" },
		{ "Cyan1", "#57FEFF" },
		{ "Cyan2", "#50EBEC" },
		{ "Cyan3", "#46C7C7" },
		{ "Cyan4", "#307D7E" },
		{ "Dark Blue", "#0000A0" },
		{ "Dark Goldenrod", "#AF7817" },
		{ "Dark Goldenrod1", "#FBB117" },
		{ "Dark Goldenrod2", "#E8A317" },
		{ "Dark Goldenrod3", "#C58917" },
		{ "Dark Goldenrod4", "#7F5217" },
		{ "Dark Green", "#254117" },
		{ "Dark Grey", "#808080" },
		{ "Dark Olive Green", "#CCFB5D" },
		{ "Dark Olive Green2", "#BCE954" },
		{ "Dark Olive Green3", "#A0C544" },
		{ "Dark Olive Green4", "#667C26" },
		{ "Dark Orange", "#F88017" },
		{ "Dark Orange1", "#F87217" },
		{ "Dark Orange2", "#E56717" },
		{ "Dark Orange3", "#7E3117" },
		{ "Dark Orange3", "#C35617" },
		{ "Dark Orchid", "#7D1B7E" },
		{ "Dark Orchid1", "#B041FF" },
		{ "Dark Orchid2", "#A23BEC" },
		{ "Dark Orchid3", "#8B31C7" },
		{ "Dark Orchid4", "#571B7e" },
		{ "Dark Purple", "#800080" },
		{ "Dark Salmon", "#E18B6B" },
		{ "Dark Sea Green", "#8BB381" },
		{ "Dark Sea Green1", "#C3FDB8" },
		{ "Dark Sea Green2", "#B5EAAA" },
		{ "Dark Sea Green3", "#99C68E" },
		{ "Dark Sea Green4", "#617C58" },
		{ "Dark Slate Blue", "#2B3856" },
		{ "Dark Slate Gray", "#25383C" },
		{ "Dark Slate Gray1", "#9AFEFF" },
		{ "Dark Slate Gray2", "#8EEBEC" },
		{ "Dark Slate Gray3", "#78c7c7" },
		{ "Dark Slate Gray4", "#4C7D7E" },
		{ "Dark Turquoise", "#3B9C9C" },
		{ "Dark Violet", "#842DCE" },
		{ "Deep Pink", "#F52887" },
		{ "Deep Pink1", "#E4287C" },
		{ "Deep Pink2", "#C12267" },
		{ "Deep Pink3", "#7D053F" },
		{ "Deep Sky Blue", "#3BB9FF" },
		{ "Deep Sky Blue1", "#38ACEC" },
		{ "Deep Sky Blue2", "#3090C7" },
		{ "Deep Sky Blue3", "#25587E" },
		{ "Dim Gray", "#463E41" },
		{ "Dodger Blue", "#1589FF" },
		{ "Dodger Blue1", "#157DEC" },
		{ "Dodger Blue2", "#1569C7" },
		{ "Dodger Blue3", "#153E7E" },
		{ "Firebrick", "#800517" },
		{ "Firebrick1", "#F62817" },
		{ "Firebrick2", "#E42217" },
		{ "Firebrick3", "#C11B17" },
		{ "Forest Green", "#4E9258" },
		{ "Forest Green1", "#808000" },
		{ "Gold", "#D4A017" },
		{ "Gold1", "#FDD017" },
		{ "Gold2", "#EAC117" },
		{ "Gold3", "#C7A317" },
		{ "Gold4", "#806517" },
		{ "Goldenrod", "#EDDA74" },
		{ "Goldenrod1", "#FBB917" },
		{ "Goldenrod2", "#E9AB17" },
		{ "Goldenrod3", "#C68E17" },
		{ "Goldenrod4", "#805817" },
		{ "Grass Green", "#408080" },
		{ "Gray", "#736F6E" },
		{ "Gray1", "#150517" },
		{ "Gray2", "#250517" },
		{ "Gray3", "#2B1B17" },
		{ "Gray4", "#302217" },
		{ "Gray5", "#302226" },
		{ "Gray6", "#342826" },
		{ "Gray7", "#34282C" },
		{ "Gray8", "#382D2C" },
		{ "Gray9", "#3b3131" },
		{ "Gray10", "#3E3535" },
		{ "Gray11", "#413839" },
		{ "Gray12", "#41383C" },
		{ "Gray13", "#463E3F" },
		{ "Gray14", "#4A4344" },
		{ "Gray15", "#4C4646" },
		{ "Gray16", "#4E4848" },
		{ "Gray17", "#504A4B" },
		{ "Gray18", "#544E4F" },
		{ "Gray19", "#565051" },
		{ "Gray19", "#595454" },
		{ "Gray20", "#5C5858" },
		{ "Gray21", "#5F5A59" },
		{ "Gray22", "#625D5D" },
		{ "Gray23", "#646060" },
		{ "Gray24", "#666362" },
		{ "Gray25", "#696565" },
		{ "Gray26", "#6D6968" },
		{ "Gray27", "#6E6A6B" },
		{ "Gray28", "#726E6D" },
		{ "Gray29", "#747170" },
		{ "Green", "#00FF00" },
		{ "Green1", "#5FFB17" },
		{ "Green2", "#59E817" },
		{ "Green3", "#4CC417" },
		{ "Green4", "#347C17" },
		{ "Green Yellow", "#B1FB17" },
		{ "Hot Pink", "#F660AB" },
		{ "Hot Pink1", "#F665AB" },
		{ "Hot Pink2", "#E45E9D" },
		{ "Hot Pink3", "#C25283" },
		{ "Hot Pink4", "#7D2252" },
		{ "Indian Red", "#F75D59" },
		{ "Indian Red2", "#E55451" },
		{ "Indian Red3", "#C24641" },
		{ "Indian Red4", "#7E2217" },
		{ "Khaki", "#ADA96E" },
		{ "Khaki1", "#FFF380" },
		{ "Khaki2", "#EDE275" },
		{ "Khaki3", "#C9BE62" },
		{ "Khaki4", "#827839" },
		{ "Lavender", "#E3E4FA" },
		{ "Lavender Blush", "#FDEEF4" },
		{ "Lavender Blush1", "#EBDDE2" },
		{ "Lavender Blush2", "#C8BBBE" },
		{ "Lavender Blush3", "#817679" },
		{ "Lawn Green", "#87F717" },
		{ "Lemon Chiffon", "#FFF8C6" },
		{ "Lemon Chiffon1", "#ECE5B6" },
		{ "Lemon Chiffon2", "#C9C299" },
		{ "Lemon Chiffon3", "#827B60" },
		{ "Light Blue", "#0000FF" },
		{ "Light Blue1", "#ADDFFF" },
		{ "Light Blue2", "#BDEDFF" },
		{ "Light Blue3", "#AFDCEC" },
		{ "Light Blue4", "#95B9C7" },
		{ "Light Blue5", "#5E767E" },
		{ "Light Coral", "#E77471" },
		{ "Light Cyan", "#E0FFFF" },
		{ "Light Cyan1", "#CFECEC" },
		{ "Light Cyan2", "#AFC7C7" },
		{ "Light Cyan3", "#717D7D" },
		{ "Light Golden", "#ECD672" },
		{ "Light Goldenrod", "#ECD872" },
		{ "Light Goldenrod1", "#FFE87C" },
		{ "Light Goldenrod2", "#C8B560" },
		{ "Light Goldenrod3", "#817339" },
		{ "Light Goldenrod Yellow", "#FAF8CC" },
		{ "Light Grey", "#C0C0C0" },
		{ "Light Pink", "#FAAFBA" },
		{ "Light Pink1", "#F9A7B0" },
		{ "Light Pink2", "#E799A3" },
		{ "Light Pink3", "#C48189" },
		{ "Light Pink4", "#7F4E52" },
		{ "Light Purple", "#FF0080" },
		{ "Light Salmon", "#F9966B" },
		{ "Light Salmon1", "#E78A61" },
		{ "Light Salmon2", "#C47451" },
		{ "Light Salmon3", "#7F462C" },
		{ "Light Sea Green", "#3EA99F" },
		{ "Light Sky Blue", "#82CAFA" },
		{ "Light Sky Blue1", "#A0CFEC" },
		{ "Light Sky Blue2", "#87AFC7" },
		{ "Light Sky Blue3", "#566D7E" },
		{ "Light Slate Blue", "#736AFF" },
		{ "Light Slate Gray", "#6D7B8D" },
		{ "Light Steel Blue", "#728FCE" },
		{ "Light Steel Blue1", "#C6DEFF" },
		{ "Light Steel Blue2", "#B7CEEC" },
		{ "Light Steel Blue3", "#646D7E" },
		{ "Lime Green", "#41A317" },
		{ "Magenta", "#FF00FF" },
		{ "Magenta1", "#F433FF" },
		{ "Magenta2", "#E238EC" },
		{ "Magenta3", "#C031C7" },
		{ "Maroon", "#810541" },
		{ "Maroon1", "#F535AA" },
		{ "Maroon2", "#E3319D" },
		{ "Maroon3", "#C12283" },
		{ "Maroon4", "#7D0552" },
		{ "Medium Aquamarine", "#348781" },
		{ "Medium Forest Green", "#347235" },
		{ "Medium Orchid", "#B048B5" },
		{ "Medium Orchid1", "#D462FF" },
		{ "Medium Orchid2", "#C45AEC" },
		{ "Medium Orchid3", "#A74AC7" },
		{ "Medium Orchid4", "#6A287E" },
		{ "Medium Purple", "#8467D7" },
		{ "Medium Purple1", "#9E7BFF" },
		{ "Medium Purple2", "#9172EC" },
		{ "Medium Purple3", "#7A5DC7" },
		{ "Medium Purple4", "#4E387E" },
		{ "Medium Sea Green", "#306754" },
		{ "Medium Slate Blue", "#5E5A80" },
		{ "Medium Spring Green", "#348017" },
		{ "Medium Turquoise", "#48CCCD" },
		{ "Medium Violet Red", "#CA226B" },
		{ "Midnight Blue", "#151B54" },
		{ "Orange", "#FF8040" },
		{ "Pale Turquoise", "#92C7C7" },
		{ "Pale Turquoise1", "#5E7D7E" },
		{ "Pale Violet Red", "#D16587" },
		{ "Pale Violet Red1", "#F778A1" },
		{ "Pale Violet Red2", "#E56E94" },
		{ "Pale Violet Red3", "#C25A7C" },
		{ "Pale Violet Red4", "#7E354D" },
		{ "Pastel Green", "#00FF00" },
		{ "Pink", "#FAAFBE" },
		{ "Pink1", "#FF00FF" },
		{ "Pink2", "#E7A1B0" },
		{ "Pink3", "#C48793" },
		{ "Pink4", "#7F525D" },
		{ "Plum", "#B93B8F" },
		{ "Plum1", "#F9B7FF" },
		{ "Plum2", "#E6A9EC" },
		{ "Plum3", "#C38EC7" },
		{ "Plum4", "#7E587E" },
		{ "Purple", "#8E35EF" },
		{ "Purple1", "#893BFF" },
		{ "Purple2", "#7F38EC" },
		{ "Purple3", "#6C2DC7" },
		{ "Purple4", "#461B7E" },
		{ "Red", "#FF0000" },
		{ "Red1", "#F62217" },
		{ "Red2", "#E41B17" },
		{ "Rosy Brown", "#B38481" },
		{ "Rosy Brown1", "#FBBBB9" },
		{ "Rosy Brown2", "#E8ADAA" },
		{ "Rosy Brown3", "#C5908E" },
		{ "Rosy Brown4", "#7F5A58" },
		{ "Royal Blue", "#2B60DE" },
		{ "Royal Blue1", "#306EFF" },
		{ "Royal Blue2", "#2B65EC" },
		{ "Royal Blue3", "#2554C7" },
		{ "Royal Blue4", "#15317E" },
		{ "Salmon1", "#F88158" },
		{ "Salmon2", "#E67451" },
		{ "Salmon3", "#C36241" },
		{ "Salmon4", "#7E3817" },
		{ "Sandy Brown", "#EE9A4D" },
		{ "Sea Green", "#4E8975" },
		{ "Sea Green1", "#6AFB92" },
		{ "Sea Green2", "#64E986" },
		{ "Sea Green3", "#54C571" },
		{ "Sea Green4", "#387C44" },
		{ "Sienna", "#8A4117" },
		{ "Sienna1", "#F87431" },
		{ "Sienna2", "#E66C2C" },
		{ "Sienna3", "#C35817" },
		{ "Sienna4", "#7E3517" },
		{ "Sky Blue", "#82CAFF" },
		{ "Sky Blue1", "#6698FF" },
		{ "Sky Blue2", "#79BAEC" },
		{ "Sky Blue3", "#659EC7" },
		{ "Sky Blue4", "#41627E" },
		{ "Slate Blue", "#357EC7" },
		{ "Slate Blue1", "#737CA1" },
		{ "Slate Blue2", "#6960EC" },
		{ "Slate Blue3", "#342D7E" },
		{ "Slate Gray", "#657383" },
		{ "Slate Gray1", "#C2DFFF" },
		{ "Slate Gray2", "#B4CFEC" },
		{ "Slate Gray3", "#98AFC7" },
		{ "Slate Gray4", "#616D7E" },
		{ "Spring Green", "#4AA02C" },
		{ "Spring Green1", "#5EFB6E" },
		{ "Spring Green2", "#57E964" },
		{ "Spring Green3", "#4CC552" },
		{ "Spring Green4", "#347C2C" },
		{ "Steel Blue", "#4863A0" },
		{ "Steel Blue1", "#5CB3FF" },
		{ "Steel Blue2", "#56A5EC" },
		{ "Steel Blue3", "#488AC7" },
		{ "Steel Blue4", "#2B547E" },
		{ "Thistle", "#D2B9D3" },
		{ "Thistle1", "#FCDFFF" },
		{ "Thistle2", "#E9CFEC" },
		{ "Thistle3", "#C6AEC7" },
		{ "Thistle4", "#806D7E" },
		{ "Turquoise", "#00FFFF" },
		{ "Turquoise1", "#43C6DB" },
		{ "Turquoise2", "#52F3FF" },
		{ "Turquoise3", "#4EE2EC" },
		{ "Turquoise4", "#43BFC7" },
		{ "Violet", "#8D38C9" },
		{ "Violet Red", "#F6358A" },
		{ "Violet Red1", "#F6358A" },
		{ "Violet Red2", "#E4317F" },
		{ "Violet Red3", "#C12869" },
		{ "Violet Red4", "#7D0541" },
		{ "White", "#FFFFFF" },
		{ "Yellow", "#FFFF00" },
		{ "Yellow1", "#FFFC17" },
		{ "Yellow Green", "#52D017" }
	};
	for (auto color : colors) {
		if (!strcasecmp(color.first.c_str(), color_name.c_str())) {
			return color.second;
		}
	}

	return std::string();
}

double QuestManager::GetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id) const {
	return database.GetAAEXPModifier(character_id, zone_id);
}

double QuestManager::GetEXPModifierByCharID(uint32 character_id, uint32 zone_id) const {
	return database.GetEXPModifier(character_id, zone_id);
}

void QuestManager::SetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, double aa_modifier) {
	database.SetAAEXPModifier(character_id, zone_id, aa_modifier);
}

void QuestManager::SetEXPModifierByCharID(uint32 character_id, uint32 zone_id, double exp_modifier) {
	database.SetEXPModifier(character_id, zone_id, exp_modifier);
}

void QuestManager::CrossZoneLDoNUpdate(uint8 type, uint8 subtype, int identifier, uint32 theme_id, int points) {
	auto pack = new ServerPacket(ServerOP_CZLDoNUpdate, sizeof(CZLDoNUpdate_Struct));
	CZLDoNUpdate_Struct* CZLU = (CZLDoNUpdate_Struct*)pack->pBuffer;
	CZLU->update_type = type;
	CZLU->update_subtype = subtype;
	CZLU->update_identifier = identifier;
	CZLU->theme_id = theme_id;
	CZLU->points = points;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

std::string QuestManager::getgendername(uint32 gender_id) {
	auto gender_name = "Unknown";
	if (gender_id == MALE) {
		gender_name = "Male";
	} else if (gender_id == FEMALE) {
		gender_name = "Female";
	} else if (gender_id == NEUTER) {
		gender_name = "Neuter";
	}
	return gender_name;
}

std::string QuestManager::getdeityname(uint32 deity_id) {
	return EQ::deity::DeityName(static_cast<EQ::deity::DeityType>(deity_id));
}

std::string QuestManager::getinventoryslotname(int16 slot_id) {
	return EQ::invslot::GetInvPossessionsSlotName(slot_id);
}


