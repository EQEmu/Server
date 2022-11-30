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
#include "../common/strings.h"
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
#include "../common/zone_store.h"
#include "dialogue_window.h"
#include "string_ids.h"

#include "../common/repositories/tradeskill_recipe_repository.h"

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
	const SPDat_Spell_Struct* questspell = nullptr; \
	bool depop_npc = false; \
	std::string encounter; \
	do { \
		if(!quests_running_.empty()) { \
			running_quest e = quests_running_.top(); \
			owner = e.owner; \
			initiator = e.initiator; \
			questitem = e.questitem; \
			questspell = e.questspell; \
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
				} else if (cur->mob->IsClient()) {
					//this is inheriently unsafe if we ever make it so more than npc/client start timers
					parse->EventPlayer(EVENT_TIMER, cur->mob->CastToClient(), cur->name, 0);
				}
#ifdef BOTS
				else if (cur->mob->IsBot()) {
					parse->EventBot(EVENT_TIMER, cur->mob->CastToBot(), nullptr, cur->name, 0);
				}
#endif

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

void QuestManager::StartQuest(Mob *_owner, Client *_initiator, EQ::ItemInstance* _questitem, const SPDat_Spell_Struct* _questspell, std::string encounter) {
	running_quest run;
	run.owner = _owner;
	run.initiator = _initiator;
	run.questitem = _questitem;
	run.questspell = _questspell;
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
	pFile = fopen (fmt::format("{}/{}", path.GetServerPath(), file).c_str(), "a");
	if(!pFile)
		return;
	fprintf(pFile, "%s\n", str);
	fclose (pFile);
}

Mob* QuestManager::spawn2(int npc_id, int grid, int unused, const glm::vec4& position) {
	const NPCType* t = 0;
	if (t = content_db.LoadNPCTypesData(npc_id)) {
		auto npc = new NPC(t, nullptr, position, GravityBehavior::Water);
		npc->AddLootTable();
		if (npc->DropsGlobalLoot()) {
			npc->CheckGlobalLootTables();
		}

		entity_list.AddNPC(npc, true, true);

		if (grid) {
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
			owner->SpellFinished(spell_id, tgt, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
	}
}

void QuestManager::selfcast(int spell_id) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->SpellFinished(spell_id, initiator, EQ::spells::CastingSlot::Item, 0, -1, spells[spell_id].resist_difficulty);
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
	if (initiator)
	{
		initiator->MoveZone(zone_name);
	}
}

void QuestManager::ZoneGroup(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		if (!initiator->GetGroup()) {
			initiator->MoveZone(zone_name);
		} else {
			auto client_group = initiator->GetGroup();
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->MoveZone(zone_name);
				}
			}
		}
	}
}

void QuestManager::ZoneRaid(const char *zone_name) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		if (!initiator->GetRaid()) {
			initiator->MoveZone(zone_name);
		} else {
			auto client_raid = initiator->GetRaid();
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->MoveZone(zone_name);
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

bool QuestManager::hastimer(const char *timer_name) {
	QuestManagerCurrentQuestVars();

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end)
	{
		if (cur->mob && cur->mob == owner && cur->name == timer_name)
		{
			if (cur->Timer_.Enabled())
			{
				return true;
			}
		}
		++cur;
	}
	return false;
}

uint32 QuestManager::getremainingtimeMS(const char *timer_name) {
	QuestManagerCurrentQuestVars();

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end)
	{
		if (cur->mob && cur->mob == owner && cur->name == timer_name)
		{
			if (cur->Timer_.Enabled())
			{
				return cur->Timer_.GetRemainingTime();
			}
		}
		++cur;
	}
	return 0;
}

uint32 QuestManager::gettimerdurationMS(const char *timer_name) {
	QuestManagerCurrentQuestVars();

	std::list<QuestTimer>::iterator cur = QTimerList.begin(), end;

	end = QTimerList.end();
	while (cur != end)
	{
		if (cur->mob && cur->mob == owner && cur->name == timer_name)
		{
			if (cur->Timer_.Enabled())
			{
				return cur->Timer_.GetDuration();
			}
		}
		++cur;
	}
	return 0;
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
	} else {
		worldserver.SendEmoteMessage(
			0,
			0,
			AccountStatus::Player,
			Chat::Red,
			fmt::format(
				"{} shouts, '{}'",
				owner->GetCleanName(),
				str
			).c_str()
		);
	}
}

void QuestManager::gmsay(const char *str, uint32 color, bool send_to_world, uint32 to_guilddbid, uint32 to_minstatus) {
	QuestManagerCurrentQuestVars();
	if(send_to_world) {
		worldserver.SendEmoteMessage(
			0,
			to_guilddbid,
			to_minstatus,
			color,
			str
		);
	} else {
		entity_list.MessageStatus(
			to_guilddbid,
			to_minstatus,
			color,
			str
		);
	}
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
	if (npc_type > 0) {
		entity_list.DepopAll(npc_type);
	} else {
		if (owner && owner->IsNPC() && (npc_type > 0)) {
			entity_list.DepopAll(npc_type);
		} else {
			LogQuests(
					"QuestManager::depopall called with nullptr owner, non-NPC owner, or invalid NPC Type ID. Probably syntax error in quest file.");
		}
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
		initiator->SetDeity(deity_id);
		initiator->Message(Chat::Yellow,"Your Deity has been changed/set to: %i", deity_id);
		initiator->Save(1);
		initiator->Kick("Deity change by QuestManager");
	}
}

void QuestManager::exp(int amt) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->AddEXP(amt);
}

void QuestManager::level(int newlevel) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->SetLevel(newlevel, true);
}

void QuestManager::traindisc(uint32 discipline_tome_item_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		initiator->TrainDiscipline(discipline_tome_item_id);
	}
}

bool QuestManager::isdisctome(uint32 item_id) {
	const auto &item = database.GetItem(item_id);
	if (!item) {
		return false;
	}

	if (!item->IsClassCommon() || item->ItemType != EQ::item::ItemTypeSpell) {
		return false;
	}

	//Need a way to determine the difference between a spell and a tome
	//so they cant turn in a spell and get it as a discipline
	//this is kinda a hack:

	const std::string item_name = item->Name;

	if (
		strcmp(item_name.substr(0, 5).c_str(), "Tome ") &&
		strcmp(item_name.substr(0, 7).c_str(), "Skill: ")
	) {
		return false;
	}

	//we know for sure none of the int casters get disciplines
	uint32 class_bit = 0;
	class_bit |= 1 << (WIZARD - 1);
	class_bit |= 1 << (ENCHANTER - 1);
	class_bit |= 1 << (MAGICIAN - 1);
	class_bit |= 1 << (NECROMANCER - 1);
	if (item->Classes & class_bit) {
		return false;
	}

	const auto& spell_id = static_cast<uint32>(item->Scroll.Effect);
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	//we know for sure none of the int casters get disciplines
	const auto& spell = spells[spell_id];
	if(
		spell.classes[WIZARD - 1] != 255 &&
		spell.classes[ENCHANTER - 1] != 255 &&
		spell.classes[MAGICIAN - 1] != 255 &&
		spell.classes[NECROMANCER - 1] != 255
	) {
		return false;
	}

	return true;
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
	return EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(skill_id));
}

std::string QuestManager::getldonthemename(uint32 theme_id) {
	return EQ::constants::GetLDoNThemeName(theme_id);
}

std::string QuestManager::getfactionname(int faction_id) {
	return content_db.GetFactionName(faction_id);
}

std::string QuestManager::getlanguagename(int language_id) {
	return EQ::constants::GetLanguageName(language_id);
}

std::string QuestManager::getbodytypename(uint32 bodytype_id) {
	return EQ::constants::GetBodyTypeName(static_cast<bodyType>(bodytype_id));
}

std::string QuestManager::getconsiderlevelname(uint8 consider_level) {
	return EQ::constants::GetConsiderLevelName(consider_level);
}

void QuestManager::safemove() {
	QuestManagerCurrentQuestVars();
	if (initiator)
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
	if (initiator) {
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

void QuestManager::surname(std::string last_name) {
	QuestManagerCurrentQuestVars();
	//Changes the last name.
	if (initiator) {
		initiator->ChangeLastName(last_name);
		initiator->Message(
			Chat::White,
			fmt::format(
				"Your last name has been set to \"{}\".",
				last_name
			).c_str()
		);
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
	return initiator->ScribeSpells(min_level, max_level);
}

uint16 QuestManager::traindiscs(uint8 max_level, uint8 min_level) {
	QuestManagerCurrentQuestVars();
	return initiator->LearnDisciplines(min_level, max_level);
}

void QuestManager::unscribespells() {
	QuestManagerCurrentQuestVars();
	initiator->UnscribeSpellAll();
}

void QuestManager::untraindiscs() {
	QuestManagerCurrentQuestVars();
	initiator->UntrainDiscAll();
}

void QuestManager::givecash(uint32 copper, uint32 silver, uint32 gold, uint32 platinum) {
	QuestManagerCurrentQuestVars();
	if (
		initiator &&
		initiator->IsClient() &&
		(
			copper ||
			silver ||
			gold ||
			platinum
		)
	) {
		initiator->CashReward(
			copper,
			silver,
			gold,
			platinum
		);
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
	if (initiator)
		initiator->MovePC(zone_id, x, y, z, heading);
}

void QuestManager::gmmove(float x, float y, float z) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->GMMove(x, y, z);
}

void QuestManager::movegrp(int zoneid, float x, float y, float z) {
	QuestManagerCurrentQuestVars();
	if (initiator)
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
	if (initiator)
		initiator->AddSkill((EQ::skills::SkillType) skill_id, value);
}

void QuestManager::setlanguage(int skill_id, int value) {
	QuestManagerCurrentQuestVars();
	if (initiator)
		initiator->SetLanguageSkill(skill_id, value);
}

void QuestManager::setskill(int skill_id, int value) {
	QuestManagerCurrentQuestVars();
	if (skill_id < 0 || skill_id > EQ::skills::HIGHEST_SKILL)
		return;
	if (initiator)
		initiator->SetSkill((EQ::skills::SkillType) skill_id, value);
}

void QuestManager::setallskill(int value) {
	QuestManagerCurrentQuestVars();
	if (!initiator)
		return;
	if (initiator) {
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
	if (initiator)
		initiator->Save();
}

void QuestManager::faction(int faction_id, int faction_value, int temp) {
	QuestManagerCurrentQuestVars();
	running_quest run = quests_running_.top();
	if(run.owner->IsCharmed() == false && initiator) {
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

void QuestManager::rewardfaction(int faction_id, int faction_value) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		if (faction_id != 0 && faction_value != 0) {
			initiator->RewardFaction(faction_id, faction_value);
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
	if (initiator) {
		guild_mgr.SetGuild(initiator->CharacterID(), new_guild_id, new_rank);
	}
}

void QuestManager::CreateGuild(const char *guild_name, const char *leader) {
	QuestManagerCurrentQuestVars();
	uint32 character_id = database.GetCharacterID(leader);
	if (character_id == 0) {
		worldserver.SendEmoteMessage(
			0,
			0,
			AccountStatus::QuestTroupe,
			Chat::Yellow,
			"Guild Error | Guild leader not found."
		);
		return;
	}

	uint32 tmp = guild_mgr.FindGuildByLeader(character_id);
	if (tmp != GUILD_NONE) {
		worldserver.SendEmoteMessage(
			0,
			0,
			AccountStatus::QuestTroupe,
			Chat::Yellow,
			fmt::format(
				"Guild Error | {} already is the leader of {} ({}).",
				leader,
				guild_mgr.GetGuildName(tmp),
				tmp
			).c_str()
		);
	} else {
		uint32 gid = guild_mgr.CreateGuild(guild_name, character_id);
		if (gid == GUILD_NONE) {
			worldserver.SendEmoteMessage(
				0,
				0,
				AccountStatus::QuestTroupe,
				Chat::Yellow,
				"Guild Error | Guild creation failed."
			);
		} else {
			worldserver.SendEmoteMessage(
				0,
				0,
				AccountStatus::QuestTroupe,
				Chat::Yellow,
				fmt::format(
					"Guild Created | Leader: {} ({}) ID: {}",
					leader,
					character_id,
					gid
				).c_str()
			);
			if (!guild_mgr.SetGuild(character_id, gid, GUILD_LEADER)) {
				worldserver.SendEmoteMessage(
					0,
					0,
					AccountStatus::QuestTroupe,
					Chat::Yellow,
					"Unable to set guild leader's guild in the database. Use #guild set."
				);
			}
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

	if (initiator){ // some events like waypoint and spawn don't have a player involved
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
	if (RuleB(QueryServ, PlayerLogQGlobalUpdate) && qgCharid && qgCharid > 0 && initiator){
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

	if (initiator) // some events like waypoint and spawn don't have a player involved
		qgCharid=initiator->CharacterID();
	else
		qgCharid=-qgNpcid;		// make char id negative npc id as a fudge

	/* QS: PlayerLogQGlobalUpdate */
	if (RuleB(QueryServ, PlayerLogQGlobalUpdate) && qgCharid && qgCharid > 0 && initiator){
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
	if (initiator)
		initiator->SendSound();

}

void QuestManager::rebind(int zone_id, const glm::vec3& location) {
	QuestManagerCurrentQuestVars();
	if(initiator) {
		initiator->SetBindPoint(0, zone_id, 0, location);
	}
}

void QuestManager::rebind(int zone_id, const glm::vec4& location) {
	QuestManagerCurrentQuestVars();
	if(initiator) {
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
	if(initiator) {
		initiator->UpdateLDoNWinLoss(theme_id);
	}
}

void QuestManager::addldonwin(uint32 theme_id) {
	QuestManagerCurrentQuestVars();
	if(initiator) {
		initiator->UpdateLDoNWinLoss(theme_id, true);
	}
}

void QuestManager::removeldonloss(uint32 theme_id) {
	QuestManagerCurrentQuestVars();
	if(initiator) {
		initiator->UpdateLDoNWinLoss(theme_id, false, true);
	}
}

void QuestManager::removeldonwin(uint32 theme_id) {
	QuestManagerCurrentQuestVars();
	if(initiator) {
		initiator->UpdateLDoNWinLoss(theme_id, true, true);
	}
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

void QuestManager::set_proximity_range(float x_range, float y_range, float z_range, bool enable_say)
{
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		return;
	}

	auto n = owner->CastToNPC();

	entity_list.AddProximity(n);

	n->proximity->min_x         = n->GetX() - x_range;
	n->proximity->max_x         = n->GetX() + x_range;
	n->proximity->min_y         = n->GetY() - y_range;
	n->proximity->max_y         = n->GetY() + y_range;
	n->proximity->min_z         = n->GetZ() - z_range;
	n->proximity->max_z         = n->GetZ() + z_range;
	n->proximity->say           = enable_say;
	n->proximity->proximity_set = true;
}

void QuestManager::set_proximity(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z, bool enable_say)
{
	QuestManagerCurrentQuestVars();
	if (!owner || !owner->IsNPC()) {
		return;
	}

	auto n = owner->CastToNPC();

	entity_list.AddProximity(n);

	n->proximity->min_x         = min_x;
	n->proximity->max_x         = max_x;
	n->proximity->min_y         = min_y;
	n->proximity->max_y         = max_y;
	n->proximity->min_z         = min_z;
	n->proximity->max_z         = max_z;
	n->proximity->say           = enable_say;
	n->proximity->proximity_set = true;
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

void QuestManager::sethp(int64 hpperc) {
	QuestManagerCurrentQuestVars();
	int64 newhp = (owner->GetMaxHP() * (100 - hpperc)) / 100;
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

int QuestManager::createbotcount(uint8 class_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		return initiator->GetBotCreationLimit(class_id);
	}

	return RuleI(Bots, CreationLimit);
}

int QuestManager::spawnbotcount(uint8 class_id) {
	QuestManagerCurrentQuestVars();
	if (initiator) {
		return initiator->GetBotSpawnLimit(class_id);
	}

	return RuleI(Bots, SpawnLimit);
}

bool QuestManager::botquest()
{
	return RuleB(Bots, QuestableSpawnLimit);
}

bool QuestManager::createBot(const char *name, const char *lastname, uint8 level, uint16 race, uint8 botclass, uint8 gender)
{
	QuestManagerCurrentQuestVars();

	if (initiator) {
		auto bot_creation_limit = initiator->GetBotCreationLimit();
		auto bot_creation_limit_class = initiator->GetBotCreationLimit(botclass);
		auto bot_spawn_limit = initiator->GetBotSpawnLimit();
		auto bot_spawn_limit_class = initiator->GetBotSpawnLimit(botclass);

		uint32 bot_count = 0;
		uint32 bot_class_count = 0;
		if (!database.botdb.QueryBotCount(initiator->CharacterID(), botclass, bot_count, bot_class_count)) {
			initiator->Message(Chat::White, "Failed to query bot count.");
			return false;
		}

		if (bot_creation_limit >= 0 && bot_count >= bot_creation_limit) {
			std::string message;

			if (bot_creation_limit) {
				message = fmt::format(
					"You cannot create anymore than {} bot{}.",
					bot_creation_limit,
					bot_creation_limit != 1 ? "s" : ""
				);
			} else {
				message = "You cannot create any bots.";
			}

			initiator->Message(Chat::White, message.c_str());
			return false;
		}

		if (bot_creation_limit_class >= 0 && bot_class_count >= bot_creation_limit_class) {
			std::string message;

			if (bot_creation_limit_class) {
				message = fmt::format(
					"You cannot create anymore than {} {} bot{}.",
					bot_creation_limit_class,
					GetClassIDName(botclass),
					bot_creation_limit_class != 1 ? "s" : ""
				);
			} else {
				message = fmt::format(
					"You cannot create any {} bots.",
					GetClassIDName(botclass)
				);
			}

			initiator->Message(Chat::White, message.c_str());
			return false;
		}

		auto spawned_bot_count = Bot::SpawnedBotCount(initiator->CharacterID());

		if (
			bot_spawn_limit >= 0 &&
			spawned_bot_count >= bot_spawn_limit &&
			!initiator->GetGM()
		) {
			std::string message;
			if (bot_spawn_limit) {
				message = fmt::format(
					"You cannot have more than {} spawned bot{}.",
					bot_spawn_limit,
					bot_spawn_limit != 1 ? "s" : ""
				);
			} else {
				message = "You are not currently allowed to spawn any bots.";
			}

			initiator->Message(Chat::White, message.c_str());
			return false;
		}

		auto spawned_bot_count_class = Bot::SpawnedBotCount(initiator->CharacterID(), botclass);

		if (
			bot_spawn_limit_class >= 0 &&
			spawned_bot_count_class >= bot_spawn_limit_class &&
			!initiator->GetGM()
		) {
			std::string message;
			if (bot_spawn_limit_class) {
				message = fmt::format(
					"You cannot have more than {} spawned {} bot{}.",
					bot_spawn_limit_class,
					GetClassIDName(botclass),
					bot_spawn_limit_class != 1 ? "s" : ""
				);
			} else {
				message = fmt::format(
					"You are not currently allowed to spawn any {} bots.",
					GetClassIDName(botclass)
				);
			}

			initiator->Message(Chat::White, message.c_str());
			return false;
		}

		std::string test_name = name;
		bool available_flag = false;
		if (!database.botdb.QueryNameAvailablity(test_name, available_flag)) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"Failed to query name availability for '{}'.",
					test_name
				).c_str()
			);
			return false;
		}

		if (!available_flag) {
			initiator->Message(
				Chat::White,
				fmt::format(
					"The name {} is already being used or is invalid. Please choose a different name.",
					test_name
				).c_str()
			);
			return false;
		}

		Bot* new_bot = new Bot(Bot::CreateDefaultNPCTypeStructForBot(name, lastname, level, race, botclass, gender), initiator);

		if (new_bot) {
			if (!new_bot->IsValidRaceClassCombo()) {
				initiator->Message(Chat::White, "That Race/Class combination cannot be created.");
				return false;
			}

			if (!new_bot->IsValidName()) {
				initiator->Message(
					Chat::White,
					fmt::format(
						"{} has invalid characters. You can use only the A-Z, a-z and _ characters in a bot name.",
						new_bot->GetCleanName()
					).c_str()
				);
				return false;
			}

			// Now that all validation is complete, we can save our newly created bot
			if (!new_bot->Save()) {
				initiator->Message(
					Chat::White,
					fmt::format(
						"Unable to save {} as a bot.",
						new_bot->GetCleanName()
					).c_str()
				);
			} else {
				initiator->Message(
					Chat::White,
					fmt::format(
						"{} saved as bot ID {}.",
						new_bot->GetCleanName(),
						new_bot->GetBotID()
					).c_str()
				);
				return true;
			}
		}
	}
	return false;
}

#endif //BOTS

void QuestManager::taskselector(const std::vector<int>& tasks, bool ignore_cooldown) {
	QuestManagerCurrentQuestVars();
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && task_manager)
		initiator->TaskQuestSetSelector(owner, tasks, ignore_cooldown);
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

void QuestManager::tasksetselector(int tasksetid, bool ignore_cooldown) {
	QuestManagerCurrentQuestVars();
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSetSelector called for task set %i", tasksetid);
	if(RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && task_manager)
		initiator->TaskSetSelector(owner, tasksetid, ignore_cooldown);
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

	if (RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && owner->IsNPC())
		return initiator->ActiveSpeakTask(owner->CastToNPC());
	return 0;
}

int QuestManager::activespeakactivity(int taskid) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && initiator && owner && owner->IsNPC())
		return initiator->ActiveSpeakActivity(owner->CastToNPC(), taskid);

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

int QuestManager::GetCurrentDzTaskID() {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && zone && task_manager) {
		return task_manager->GetCurrentDzTaskID();
	}

	return 0;
}

void QuestManager::EndCurrentDzTask(bool send_fail) {
	QuestManagerCurrentQuestVars();

	if (RuleB(TaskSystem, EnableTaskSystem) && zone && task_manager) {
		task_manager->EndCurrentDzTask(send_fail);
	}
}

void QuestManager::clearspawntimers() {
	if (!zone) {
        return;
	}

	zone->ClearSpawnTimers();
}

void QuestManager::ze(int type, const char *str) {
	entity_list.Message(0, type, str);
}

void QuestManager::we(int type, const char *str) {
	worldserver.SendEmoteMessage(
		0,
		0,
		type,
		str
	);
}

void QuestManager::message(uint32 type, const char *message) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->Message(type, message);
}

void QuestManager::whisper(const char *message) {
	QuestManagerCurrentQuestVars();
	if (!initiator || !owner) {
		return;
	}

	initiator->Message(
		Chat::EchoChat1,
		fmt::format(
			"{} whispers, '{}'",
			owner->GetCleanName(),
			message
		).c_str()
	);
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
	else if(type == 4)
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

void QuestManager::ModifyNPCStat(std::string stat, std::string value)
{
	QuestManagerCurrentQuestVars();
	if (owner && owner->IsNPC()) {
		owner->CastToNPC()->ModifyNPCStat(stat, value);
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

uint16 QuestManager::CreateInstance(const char *zone_short_name, int16 instance_version, uint32 duration)
{
	QuestManagerCurrentQuestVars();

	uint32 zone_id = ZoneID(zone_short_name);
	if (!zone_id) {
		return 0;
	}

	uint16 instance_id = 0;
	if (!database.GetUnusedInstanceID(instance_id)) {
		if (initiator) {
			initiator->Message(Chat::Red, "Server was unable to find a free instance id.");
		}

		return 0;
	}

	if (!database.CreateInstance(instance_id, zone_id, instance_version, duration)) {
		if (initiator) {
			initiator->Message(Chat::Red, "Server was unable to create a new instance.");
		}

		return 0;
	}

	return instance_id;
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

std::vector<uint16> QuestManager::GetInstanceIDs(std::string zone_name, uint32 character_id)
{
	if (!character_id) {
		QuestManagerCurrentQuestVars();
		if (initiator) {
			return database.GetInstanceIDs(ZoneID(zone_name), initiator->CharacterID());
		}

		return { };
	}

	return database.GetInstanceIDs(ZoneID(zone_name), character_id);
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
	return database.CheckInstanceByCharID(instance_id, char_id);
}

void QuestManager::RemoveAllFromInstance(uint16 instance_id)
{
	QuestManagerCurrentQuestVars();
	if (initiator)
	{
		std::list<uint32> character_ids;

		if (database.RemoveClientsFromInstance(instance_id))
			initiator->Message(Chat::Say, "Removed all players from instance.");
		else
		{
			database.GetCharactersInInstance(instance_id, character_ids);
			initiator->Message(Chat::Say, "Failed to remove %i player(s) from instance.", character_ids.size()); // once the expedition system is in, this message it not relevant
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

	return Saylink::Create(saylink_text, silent, link_name);
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

uint32 QuestManager::getcurrencyid(uint32 item_id) {
	return zone->GetCurrencyID(item_id);
}

uint32 QuestManager::getcurrencyitemid(uint32 currency_id) {
	return zone->GetCurrencyItemID(currency_id);
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
			case FACTION_THREATENINGLY:
				newfac = 2;
				break;
			case FACTION_DUBIOUSLY:
				newfac = 3;
				break;
			case FACTION_APPREHENSIVELY:
				newfac = 4;
				break;
			case FACTION_INDIFFERENTLY:
				newfac = 5;
				break;
			case FACTION_AMIABLY:
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
	return ZoneLongName(ZoneID(zone_short_name), true);
}

std::string QuestManager::GetZoneLongNameByID(uint32 zone_id) {
	return ZoneLongName(zone_id, true);
}

std::string QuestManager::GetZoneShortName(uint32 zone_id) {
	return ZoneName(zone_id, true);
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

#ifdef BOTS
Bot *QuestManager::GetBot() const {
	if (!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return (e.owner && e.owner->IsBot()) ? e.owner->CastToBot() : nullptr;
	}

	return nullptr;
}
#endif

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

const SPDat_Spell_Struct *QuestManager::GetQuestSpell() {
	if(!quests_running_.empty()) {
		running_quest e = quests_running_.top();
		return e.questspell;
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
	} else if (strcasecmp(type.c_str(), "gravity") == 0) {
		zone->newzone_data.gravity = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "time_type") == 0) {
		zone->newzone_data.time_type = atoi(value.c_str());
	} else if (strcasecmp(type.c_str(), "rain_chance") == 0) {
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
	} else if (strcasecmp(type.c_str(), "sky") == 0) {
		zone->newzone_data.sky = atoi(value.c_str());
	} else if (strcasecmp(type.c_str(), "safe_x") == 0) {
		zone->newzone_data.safe_x = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "safe_y") == 0) {
		zone->newzone_data.safe_y = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "safe_z") == 0) {
		zone->newzone_data.safe_z = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "max_z") == 0) {
		zone->newzone_data.max_z = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "underworld") == 0) {
		zone->newzone_data.underworld = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "minclip") == 0) {
		zone->newzone_data.minclip = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "maxclip") == 0) {
		zone->newzone_data.maxclip = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "fog_density") == 0) {
		zone->newzone_data.fog_density = atof(value.c_str());
	} else if (strcasecmp(type.c_str(), "suspendbuffs") == 0) {
		zone->newzone_data.suspend_buffs = atoi(value.c_str());
	} else if (strcasecmp(type.c_str(), "lavadamage") == 0) {
		zone->newzone_data.lava_damage = atoi(value.c_str());
	} else if (strcasecmp(type.c_str(), "minlavadamage") == 0) {
		zone->newzone_data.min_lava_damage = atoi(value.c_str());
	}

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

std::string QuestManager::gethexcolorcode(std::string color_name) {

	for (auto color : html_colors) {
		if (!strcasecmp(color.first.c_str(), color_name.c_str())) {
			return color.second;
		}
	}

	return std::string();
}

double QuestManager::GetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version) const {
	return database.GetAAEXPModifier(character_id, zone_id, instance_version);
}

double QuestManager::GetEXPModifierByCharID(uint32 character_id, uint32 zone_id, int16 instance_version) const {
	return database.GetEXPModifier(character_id, zone_id, instance_version);
}

void QuestManager::SetAAEXPModifierByCharID(uint32 character_id, uint32 zone_id, double aa_modifier, int16 instance_version) {
	database.SetAAEXPModifier(character_id, zone_id, aa_modifier, instance_version);
}

void QuestManager::SetEXPModifierByCharID(uint32 character_id, uint32 zone_id, double exp_modifier, int16 instance_version) {
	database.SetEXPModifier(character_id, zone_id, exp_modifier, instance_version);
}

std::string QuestManager::getgendername(uint32 gender_id) {
	std::string gender_name = GetGenderName(gender_id);
	return gender_name;
}

std::string QuestManager::getdeityname(uint32 deity_id) {
	return EQ::deity::DeityName(static_cast<EQ::deity::DeityType>(deity_id));
}

std::string QuestManager::getinventoryslotname(int16 slot_id) {
	return EQ::invslot::GetInvPossessionsSlotName(slot_id);
}

int QuestManager::getitemstat(uint32 item_id, std::string stat_identifier) {
	QuestManagerCurrentQuestVars();
	return EQ::InventoryProfile::GetItemStatValue(item_id, stat_identifier.c_str());
}

int QuestManager::getspellstat(uint32 spell_id, std::string stat_identifier, uint8 slot) {
	QuestManagerCurrentQuestVars();
	return GetSpellStatValue(spell_id, stat_identifier.c_str(), slot);
}

void QuestManager::CrossZoneDialogueWindow(uint8 update_type, int update_identifier, const char* message, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZDialogueWindow, sizeof(CZDialogueWindow_Struct));
	CZDialogueWindow_Struct* CZDW = (CZDialogueWindow_Struct*)pack->pBuffer;
	CZDW->update_type = update_type;
	CZDW->update_identifier = update_identifier;
	strn0cpy(CZDW->message, message, 4096);
	strn0cpy(CZDW->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneLDoNUpdate(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 theme_id, int points, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZLDoNUpdate, sizeof(CZLDoNUpdate_Struct));
	CZLDoNUpdate_Struct* CZLU = (CZLDoNUpdate_Struct*)pack->pBuffer;
	CZLU->update_type = update_type;
	CZLU->update_subtype = update_subtype;
	CZLU->update_identifier = update_identifier;
	CZLU->theme_id = theme_id;
	CZLU->points = points;
	strn0cpy(CZLU->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMarquee(uint8 update_type, int update_identifier, uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZMarquee, sizeof(CZMarquee_Struct));
	CZMarquee_Struct* CZM = (CZMarquee_Struct*)pack->pBuffer;
	CZM->update_type = update_type;
	CZM->update_identifier = update_identifier;
	CZM->type = type;
	CZM->priority = priority;
	CZM->fade_in = fade_in;
	CZM->fade_out = fade_out;
	CZM->duration = duration;
	strn0cpy(CZM->message, message, 512);
	strn0cpy(CZM->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMessage(uint8 update_type, int update_identifier, uint32 type, const char* message, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZMessage, sizeof(CZMarquee_Struct));
	CZMessage_Struct* CZM = (CZMessage_Struct*)pack->pBuffer;
	CZM->update_type = update_type;
	CZM->update_identifier = update_identifier;
	CZM->type = type;
	strn0cpy(CZM->message, message, 512);
	strn0cpy(CZM->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneMove(uint8 update_type, uint8 update_subtype, int update_identifier, const char* zone_short_name, uint16 instance_id, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZMove, sizeof(CZMove_Struct));
	CZMove_Struct* CZM = (CZMove_Struct*)pack->pBuffer;
	CZM->update_type = update_type;
	CZM->update_subtype = update_subtype;
	CZM->update_identifier = update_identifier;
	strn0cpy(CZM->zone_short_name, zone_short_name, 32);
	CZM->instance_id = instance_id;
	strn0cpy(CZM->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSetEntityVariable(uint8 update_type, int update_identifier, const char* variable_name, const char* variable_value, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZSetEntityVariable, sizeof(CZSetEntityVariable_Struct));
	CZSetEntityVariable_Struct* CZM = (CZSetEntityVariable_Struct*)pack->pBuffer;
	CZM->update_type = update_type;
	CZM->update_identifier = update_identifier;
	strn0cpy(CZM->variable_name, variable_name, 256);
	strn0cpy(CZM->variable_value, variable_value, 256);
	strn0cpy(CZM->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSignal(uint8 update_type, int update_identifier, int signal, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZSignal, sizeof(CZSignal_Struct));
	CZSignal_Struct* CZS = (CZSignal_Struct*)pack->pBuffer;
	CZS->update_type = update_type;
	CZS->update_identifier = update_identifier;
	CZS->signal = signal;
	strn0cpy(CZS->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneSpell(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 spell_id, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZSpell, sizeof(CZSpell_Struct));
	CZSpell_Struct* CZS = (CZSpell_Struct*)pack->pBuffer;
	CZS->update_type = update_type;
	CZS->update_subtype = update_subtype;
	CZS->update_identifier = update_identifier;
	CZS->spell_id = spell_id;
	strn0cpy(CZS->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::CrossZoneTaskUpdate(uint8 update_type, uint8 update_subtype, int update_identifier, uint32 task_identifier, int task_subidentifier, int update_count, bool enforce_level_requirement, const char* client_name) {
	auto pack = new ServerPacket(ServerOP_CZTaskUpdate, sizeof(CZTaskUpdate_Struct));
	CZTaskUpdate_Struct* CZTU = (CZTaskUpdate_Struct*)pack->pBuffer;
	CZTU->update_type = update_type;
	CZTU->update_subtype = update_subtype;
	CZTU->update_identifier = update_identifier;
	CZTU->task_identifier = task_identifier;
	CZTU->task_subidentifier = task_subidentifier;
	CZTU->update_count = update_count;
	CZTU->enforce_level_requirement = enforce_level_requirement;
	strn0cpy(CZTU->client_name, client_name, 64);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideDialogueWindow(const char* message, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWDialogueWindow, sizeof(WWDialogueWindow_Struct));
	WWDialogueWindow_Struct* WWDW = (WWDialogueWindow_Struct*)pack->pBuffer;
	strn0cpy(WWDW->message, message, 4096);
	WWDW->min_status = min_status;
	WWDW->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideLDoNUpdate(uint8 update_type, uint32 theme_id, int points, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWLDoNUpdate, sizeof(WWLDoNUpdate_Struct));
	WWLDoNUpdate_Struct* WWLU = (WWLDoNUpdate_Struct*)pack->pBuffer;
	WWLU->update_type = update_type;
	WWLU->theme_id = theme_id;
	WWLU->points = points;
	WWLU->min_status = min_status;
	WWLU->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMarquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, const char* message, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWMarquee, sizeof(WWMarquee_Struct));
	WWMarquee_Struct* WWM = (WWMarquee_Struct*)pack->pBuffer;
	WWM->type = type;
	WWM->priority = priority;
	WWM->fade_in = fade_in;
	WWM->fade_out = fade_out;
	WWM->duration = duration;
	strn0cpy(WWM->message, message, 512);
	WWM->min_status = min_status;
	WWM->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMessage(uint32 type, const char* message, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWMessage, sizeof(WWMarquee_Struct));
	WWMessage_Struct* WWM = (WWMessage_Struct*)pack->pBuffer;
	WWM->type = type;
	strn0cpy(WWM->message, message, 512);
	WWM->min_status = min_status;
	WWM->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideMove(uint8 update_type, const char* zone_short_name, uint16 instance_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWMove, sizeof(WWMove_Struct));
	WWMove_Struct* WWM = (WWMove_Struct*)pack->pBuffer;
	WWM->update_type = update_type;
	strn0cpy(WWM->zone_short_name, zone_short_name, 32);
	WWM->instance_id = instance_id;
	WWM->min_status = min_status;
	WWM->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSetEntityVariable(uint8 update_type, const char* variable_name, const char* variable_value, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWSetEntityVariable, sizeof(WWSetEntityVariable_Struct));
	WWSetEntityVariable_Struct* WWSEV = (WWSetEntityVariable_Struct*)pack->pBuffer;
	WWSEV->update_type = update_type;
	strn0cpy(WWSEV->variable_name, variable_name, 256);
	strn0cpy(WWSEV->variable_value, variable_value, 256);
	WWSEV->min_status = min_status;
	WWSEV->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSignal(uint8 update_type, int signal, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWSignal, sizeof(WWSignal_Struct));
	WWSignal_Struct* WWS = (WWSignal_Struct*)pack->pBuffer;
	WWS->update_type = update_type;
	WWS->signal = signal;
	WWS->min_status = min_status;
	WWS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideSpell(uint8 update_type, uint32 spell_id, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWSpell, sizeof(WWSpell_Struct));
	WWSpell_Struct* WWS = (WWSpell_Struct*)pack->pBuffer;
	WWS->update_type = update_type;
	WWS->spell_id = spell_id;
	WWS->min_status = min_status;
	WWS->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QuestManager::WorldWideTaskUpdate(uint8 update_type, uint32 task_identifier, int task_subidentifier, int update_count, bool enforce_level_requirement, uint8 min_status, uint8 max_status) {
	auto pack = new ServerPacket(ServerOP_WWTaskUpdate, sizeof(WWTaskUpdate_Struct));
	WWTaskUpdate_Struct* WWTU = (WWTaskUpdate_Struct*)pack->pBuffer;
	WWTU->update_type = update_type;
	WWTU->task_identifier = task_identifier;
	WWTU->task_subidentifier = task_subidentifier;
	WWTU->update_count = update_count;
	WWTU->enforce_level_requirement = enforce_level_requirement;
	WWTU->min_status = min_status;
	WWTU->max_status = max_status;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

const SPDat_Spell_Struct* QuestManager::getspell(uint32 spell_id) {
    if (spells[spell_id].id) {
        return &spells[spell_id];
    }
    return nullptr;
}

std::string QuestManager::getenvironmentaldamagename(uint8 damage_type) {
	std::string environmental_damage_name = EQ::constants::GetEnvironmentalDamageName(damage_type);
	return environmental_damage_name;
}

void QuestManager::TrackNPC(uint32 entity_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->SetTrackingID(entity_id);
}

int QuestManager::GetRecipeMadeCount(uint32 recipe_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return 0;
	}

	return initiator->GetRecipeMadeCount(recipe_id);
}

std::string QuestManager::GetRecipeName(uint32 recipe_id) {
	auto r = TradeskillRecipeRepository::GetWhere(
		database,
		fmt::format("id = {}", recipe_id)
	);

	if (!r.empty() && r[0].id) {
		return r[0].name;
	}

	return std::string();
}

bool QuestManager::HasRecipeLearned(uint32 recipe_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return false;
	}

	return initiator->HasRecipeLearned(recipe_id);
}

void QuestManager::LearnRecipe(uint32 recipe_id) {
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->LearnRecipe(recipe_id);
}

void QuestManager::marquee(uint32 type, std::string message, uint32 duration)
{
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->SendMarqueeMessage(type, message, duration);
}

void QuestManager::marquee(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string message)
{
	QuestManagerCurrentQuestVars();
	if (!initiator) {
		return;
	}

	initiator->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
}
