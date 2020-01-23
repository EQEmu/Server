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
#include "../common/string_util.h"

#include "client.h"
#include "entity.h"
#include "spawn2.h"
#include "spawngroup.h"
#include "worldserver.h"
#include "zone.h"
#include "zonedb.h"

extern EntityList entity_list;
extern Zone* zone;

extern WorldServer worldserver;

/*

CREATE TABLE spawn_conditions (
	zone VARCHAR(16) NOT NULL,
	id MEDIUMINT UNSIGNED NOT NULL DEFAULT '1',
	value MEDIUMINT NOT NULL DEFAULT '0',
	onchange TINYINT UNSIGNED NOT NULL DEFAULT '0',
	name VARCHAR(255) NOT NULL DEFAULT '',
	PRIMARY KEY(zone,id)
);

CREATE TABLE spawn_events (
	#identifiers
	id INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
	zone VARCHAR(16) NOT NULL,
	cond_id MEDIUMINT UNSIGNED NOT NULL,
	name VARCHAR(255) NOT NULL DEFAULT '',

	#timing information
	period INT UNSIGNED NOT NULL,
	next_minute TINYINT UNSIGNED NOT NULL,
	next_hour TINYINT UNSIGNED NOT NULL,
	next_day TINYINT UNSIGNED NOT NULL,
	next_month TINYINT UNSIGNED NOT NULL,
	next_year INT UNSIGNED NOT NULL,
	enabled TINYINT NOT NULL DEFAULT '1',

	#action:
	action TINYINT UNSIGNED NOT NULL DEFAULT '0',
	argument MEDIUMINT NOT NULL DEFAULT '0'
);

*/

Spawn2::Spawn2(uint32 in_spawn2_id, uint32 spawngroup_id,
	float in_x, float in_y, float in_z, float in_heading,
	uint32 respawn, uint32 variance, uint32 timeleft, uint32 grid,
	uint16 in_cond_id, int16 in_min_value, bool in_enabled, EmuAppearance anim)
: timer(100000), killcount(0)
{
	spawn2_id = in_spawn2_id;
	spawngroup_id_ = spawngroup_id;
	x = in_x;
	y = in_y;
	z = in_z;
	heading = in_heading;
	respawn_ = respawn;
	variance_ = variance;
	grid_ = grid;
	condition_id = in_cond_id;
	condition_min_value = in_min_value;
	npcthis = nullptr;
	enabled = in_enabled;
	this->anim = anim;

	if(timeleft == 0xFFFFFFFF) {
		//special disable timeleft
		timer.Disable();
	} else if(timeleft != 0){
		//we have a timeleft from the DB or something
		timer.Start(timeleft);
	} else {
		//no timeleft at all, reset to
		timer.Start(resetTimer());
		timer.Trigger();
	}
}

Spawn2::~Spawn2()
{
}

uint32 Spawn2::resetTimer()
{
	uint32 rspawn = respawn_ * 1000;

	if (variance_ != 0) {
		int var_over_2 = (variance_ * 1000) / 2;
		rspawn = zone->random.Int(rspawn - var_over_2, rspawn + var_over_2);

		//put a lower bound on it, not a lot of difference below 100, so set that as the bound.
		if(rspawn < 100)
			rspawn = 100;
	}

	return (rspawn);

}

uint32 Spawn2::despawnTimer(uint32 despawn_timer)
{
	uint32 dspawn = despawn_timer * 1000;

	if (variance_ != 0) {
		int var_over_2 = (variance_ * 1000) / 2;
		dspawn = zone->random.Int(dspawn - var_over_2, dspawn + var_over_2);

		//put a lower bound on it, not a lot of difference below 100, so set that as the bound.
		if(dspawn < 100)
			dspawn = 100;
	}

	return (dspawn);

}

bool Spawn2::Process() {
	IsDespawned = false;

	if (!Enabled())
		return true;

	//grab our spawn group
	SpawnGroup *spawn_group = zone->spawn_group_list.GetSpawnGroup(spawngroup_id_);

	if (NPCPointerValid() && (spawn_group->despawn == 0 || condition_id != 0)) {
		return true;
	}

	if (timer.Check()) {
		timer.Disable();

		LogSpawns("Spawn2 [{}]: Timer has triggered", spawn2_id);

		//first check our spawn condition, if this isnt active
		//then we reset the timer and try again next time.
		if (condition_id != SC_AlwaysEnabled
			&& !zone->spawn_conditions.Check(condition_id, condition_min_value)) {
			LogSpawns("Spawn2 [{}]: spawning prevented by spawn condition [{}]", spawn2_id, condition_id);
			Reset();
			return (true);
		}

		if (spawn_group == nullptr) {
			database.LoadSpawnGroupsByID(spawngroup_id_, &zone->spawn_group_list);
			spawn_group = zone->spawn_group_list.GetSpawnGroup(spawngroup_id_);
		}

		if (spawn_group == nullptr) {
			LogSpawns("Spawn2 [{}]: Unable to locate spawn group [{}]. Disabling", spawn2_id, spawngroup_id_);

			return false;
		}

		uint16 condition_value=1;

		if (condition_id > 0) {
			condition_value = zone->spawn_conditions.GetCondition(zone->GetShortName(), zone->GetInstanceID(), condition_id);
		}

		//have the spawn group pick an NPC for us
		uint32 npcid = spawn_group->GetNPCType(condition_value);
		if (npcid == 0) {
			LogSpawns("Spawn2 [{}]: Spawn group [{}] did not yeild an NPC! not spawning", spawn2_id, spawngroup_id_);

			Reset();    //try again later (why?)
			return (true);
		}

		//try to find our NPC type.
		const NPCType *tmp = database.LoadNPCTypesData(npcid);
		if (tmp == nullptr) {
			LogSpawns("Spawn2 [{}]: Spawn group [{}] yeilded an invalid NPC type [{}]", spawn2_id, spawngroup_id_, npcid);
			Reset();    //try again later
			return (true);
		}

		if (tmp->unique_spawn_by_name) {
			if (!entity_list.LimitCheckName(tmp->name)) {
				LogSpawns("Spawn2 [{}]: Spawn group [{}] yeilded NPC type [{}], which is unique and one already exists", spawn2_id, spawngroup_id_, npcid);
				timer.Start(5000);    //try again in five seconds.
				return (true);
			}
		}

		if (tmp->spawn_limit > 0) {
			if (!entity_list.LimitCheckType(npcid, tmp->spawn_limit)) {
				LogSpawns("Spawn2 [{}]: Spawn group [{}] yeilded NPC type [{}], which is over its spawn limit ([{}])", spawn2_id, spawngroup_id_, npcid, tmp->spawn_limit);
				timer.Start(5000);    //try again in five seconds.
				return (true);
			}
		}

		bool ignore_despawn = false;
		if (npcthis) {
			ignore_despawn = npcthis->IgnoreDespawn();
		}

		if (ignore_despawn) {
			return true;
		}

		if (spawn_group->despawn != 0 && condition_id == 0 && !ignore_despawn) {
			zone->Despawn(spawn2_id);
		}

		if (IsDespawned) {
			return true;
		}

		currentnpcid = npcid;
		NPC *npc = new NPC(tmp, this, glm::vec4(x, y, z, heading), GravityBehavior::Water);

		npc->mod_prespawn(this);

		npcthis = npc;
		npc->AddLootTable();
		if (npc->DropsGlobalLoot()) {
			npc->CheckGlobalLootTables();
		}
		npc->SetSpawnGroupId(spawngroup_id_);
		npc->SaveGuardPointAnim(anim);
		npc->SetAppearance((EmuAppearance) anim);
		entity_list.AddNPC(npc);
		//this limit add must be done after the AddNPC since we need the entity ID.
		entity_list.LimitAddNPC(npc);

		/**
		 * Roambox init
		 */
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
			LogSpawns("Spawn2 [{}]: Group [{}] spawned [{}] ([{}]) at ([{}], [{}], [{}])",
				spawn2_id,
				spawngroup_id_,
				npc->GetName(),
				npcid,
				x,
				y,
				z
			);

			LoadGrid();
		}
		else {
			LogSpawns("Spawn2 [{}]: Group [{}] spawned [{}] ([{}]) at ([{}], [{}], [{}]). Grid loading delayed",
				spawn2_id,
				spawngroup_id_,
				tmp->name,
				npcid,
				x,
				y,
				z
			);
		}
	}

	return true;
}

void Spawn2::Disable()
{
	if(npcthis)
	{
		npcthis->Depop();
	}
	enabled = false;
}

void Spawn2::LoadGrid() {
	if (!npcthis)
		return;
	if (grid_ < 1)
		return;
	if (!entity_list.IsMobInZone(npcthis))
		return;
	//dont set an NPC's grid until its loaded for them.
	npcthis->SetGrid(grid_);
	npcthis->AssignWaypoints(grid_);
	LogSpawns("Spawn2 [{}]: Loading grid [{}] for [{}]", spawn2_id, grid_, npcthis->GetName());
}

/*
	All three of these actions basically say that the mob which was
	associated with this spawn point is no longer relavent.
*/
void Spawn2::Reset() {
	timer.Start(resetTimer());
	npcthis = nullptr;
	LogSpawns("Spawn2 [{}]: Spawn reset, repop in [{}] ms", spawn2_id, timer.GetRemainingTime());
}

void Spawn2::Depop() {
	timer.Disable();
	LogSpawns("Spawn2 [{}]: Spawn reset, repop disabled", spawn2_id);
	npcthis = nullptr;
}

void Spawn2::Repop(uint32 delay) {
	if (delay == 0) {
		timer.Trigger();
		LogSpawns("Spawn2 [{}]: Spawn reset, repop immediately", spawn2_id);
	} else {
		LogSpawns("Spawn2 [{}]: Spawn reset for repop, repop in [{}] ms", spawn2_id, delay);
		timer.Start(delay);
	}
	npcthis = nullptr;
}

void Spawn2::ForceDespawn()
{
	SpawnGroup* sg = zone->spawn_group_list.GetSpawnGroup(spawngroup_id_);

	if(npcthis != nullptr)
	{
		if (npcthis->IgnoreDespawn())
			return;

		if(!npcthis->IsEngaged())
		{
			if(sg->despawn == 3 || sg->despawn == 4)
			{
				npcthis->Depop(true);
				IsDespawned = true;
				npcthis = nullptr;
				return;
			}
			else
			{
				npcthis->Depop(false);
				npcthis = nullptr;
			}
		}
	}

	uint32 cur = 100000;
	uint32 dtimer = sg->despawn_timer;

	if(sg->despawn == 1 || sg->despawn == 3)
	{
		cur = resetTimer();
	}

	if(sg->despawn == 2 || sg->despawn == 4)
	{
		cur = despawnTimer(dtimer);
	}

	LogSpawns("Spawn2 [{}]: Spawn group [{}] set despawn timer to [{}] ms", spawn2_id, spawngroup_id_, cur);
	timer.Start(cur);
}

//resets our spawn as if we just died
void Spawn2::DeathReset(bool realdeath)
{
	//get our reset based on variance etc and store it locally
	uint32 cur = resetTimer();
	//set our timer to our reset local
	timer.Start(cur);

	//zero out our NPC since he is now gone
	npcthis = nullptr;

	if(realdeath) { killcount++; }

	//if we have a valid spawn id
	if(spawn2_id)
	{
		database.UpdateRespawnTime(spawn2_id, zone->GetInstanceID(), (cur/1000));
		LogSpawns("Spawn2 [{}]: Spawn reset by death, repop in [{}] ms", spawn2_id, timer.GetRemainingTime());
		//store it to database too
	}
}

bool ZoneDatabase::PopulateZoneSpawnListClose(uint32 zoneid, LinkedList<Spawn2*> &spawn2_list, int16 version, const glm::vec4& client_position, uint32 repop_distance)
{
	std::unordered_map<uint32, uint32> spawn_times;

	float mob_distance = 0;

	timeval tv;
	gettimeofday(&tv, nullptr);

	/* Bulk Load NPC Types Data into the cache */
	database.LoadNPCTypesData(0, true);

	std::string spawn_query = StringFormat(
		"SELECT "
		"respawn_times.id, "
		"respawn_times.`start`, "
		"respawn_times.duration "
		"FROM "
		"respawn_times "
		"WHERE instance_id = %u",
		zone->GetInstanceID()
		);
	auto results = QueryDatabase(spawn_query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		uint32 start_duration = atoi(row[1]) > 0 ? atoi(row[1]) : 0;
		uint32 end_duration = atoi(row[2]) > 0 ? atoi(row[2]) : 0;

		/* Our current time was expired */
		if ((start_duration + end_duration) <= tv.tv_sec) {
			spawn_times[atoi(row[0])] = 0;
		}
		/* We still have time left on this timer */
		else {
			spawn_times[atoi(row[0])] = ((start_duration + end_duration) - tv.tv_sec) * 1000;
		}
	}

	const char *zone_name = database.GetZoneName(zoneid);
	std::string query = StringFormat(
		"SELECT "
		"id, "
		"spawngroupID, "
		"x, "
		"y, "
		"z, "
		"heading, "
		"respawntime, "
		"variance, "
		"pathgrid, "
		"_condition, "
		"cond_value, "
		"enabled, "
		"animation "
		"FROM "
		"spawn2 "
		"WHERE zone = '%s' AND  (version = %u OR version = -1) ",
		zone_name,
		version
		);
	results = QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {

		uint32 spawn_time_left = 0;
		Spawn2* new_spawn = 0;
		bool perl_enabled = atoi(row[11]) == 1 ? true : false;

		if (spawn_times.count(atoi(row[0])) != 0)
			spawn_time_left = spawn_times[atoi(row[0])];

		glm::vec4 point;
		point.x = atof(row[2]);
		point.y = atof(row[3]);

		mob_distance = DistanceNoZ(client_position, point);

		if (mob_distance > repop_distance)
			continue;

		new_spawn = new Spawn2(							   // 
			atoi(row[0]), 								   // uint32 in_spawn2_id
			atoi(row[1]), 								   // uint32 spawngroup_id
			atof(row[2]), 								   // float in_x
			atof(row[3]), 								   // float in_y
			atof(row[4]),								   // float in_z
			atof(row[5]), 								   // float in_heading
			atoi(row[6]), 								   // uint32 respawn
			atoi(row[7]), 								   // uint32 variance
			spawn_time_left,							   // uint32 timeleft
			atoi(row[8]),								   // uint32 grid
			atoi(row[9]), 								   // uint16 in_cond_id
			atoi(row[10]), 								   // int16 in_min_value
			perl_enabled, 								   // bool in_enabled
			(EmuAppearance)atoi(row[12])				   // EmuAppearance anim
			);

		spawn2_list.Insert(new_spawn);
	}

	return true;
}

bool ZoneDatabase::PopulateZoneSpawnList(uint32 zoneid, LinkedList<Spawn2*> &spawn2_list, int16 version, uint32 repopdelay) {

	std::unordered_map<uint32, uint32> spawn_times;

	timeval tv;
	gettimeofday(&tv, nullptr);

	/* Bulk Load NPC Types Data into the cache */
	database.LoadNPCTypesData(0, true);

	std::string spawn_query = StringFormat(
		"SELECT "
		"respawn_times.id, "
		"respawn_times.`start`, "
		"respawn_times.duration "
		"FROM "
		"respawn_times "
		"WHERE instance_id = %u",
		zone->GetInstanceID()
	);
	auto results = QueryDatabase(spawn_query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		uint32 start_duration = atoi(row[1]) > 0 ? atoi(row[1]) : 0;
		uint32 end_duration = atoi(row[2]) > 0 ? atoi(row[2]) : 0;

		/* Our current time was expired */
		if ((start_duration + end_duration) <= tv.tv_sec) {
			spawn_times[atoi(row[0])] = 0;
		}
		/* We still have time left on this timer */
		else {
			spawn_times[atoi(row[0])] = ((start_duration + end_duration) - tv.tv_sec) * 1000;
		}
	}

	const char *zone_name = database.GetZoneName(zoneid);
	std::string query = StringFormat(
		"SELECT "
		"id, "
		"spawngroupID, "
		"x, "
		"y, "
		"z, "
		"heading, "
		"respawntime, "
		"variance, "
		"pathgrid, "
		"_condition, "
		"cond_value, "
		"enabled, "
		"animation "
		"FROM "
		"spawn2 "
		"WHERE zone = '%s' AND  (version = %u OR version = -1)",
		zone_name,
		version
	);
	results = QueryDatabase(query);

	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {

		uint32 spawn_time_left = 0; 
		Spawn2* new_spawn = 0; 
		bool perl_enabled = atoi(row[11]) == 1 ? true : false;

		if (spawn_times.count(atoi(row[0])) != 0)
			spawn_time_left = spawn_times[atoi(row[0])];

		new_spawn = new Spawn2(							   // 
			atoi(row[0]), 								   // uint32 in_spawn2_id
			atoi(row[1]), 								   // uint32 spawngroup_id
			atof(row[2]), 								   // float in_x
			atof(row[3]), 								   // float in_y
			atof(row[4]),								   // float in_z
			atof(row[5]), 								   // float in_heading
			atoi(row[6]), 								   // uint32 respawn
			atoi(row[7]), 								   // uint32 variance
			spawn_time_left,							   // uint32 timeleft
			atoi(row[8]),								   // uint32 grid
			atoi(row[9]), 								   // uint16 in_cond_id
			atoi(row[10]), 								   // int16 in_min_value
			perl_enabled, 								   // bool in_enabled
			(EmuAppearance)atoi(row[12])				   // EmuAppearance anim
		);

		spawn2_list.Insert(new_spawn);
	}

	NPC::SpawnZoneController();

	return true;
}


Spawn2* ZoneDatabase::LoadSpawn2(LinkedList<Spawn2*> &spawn2_list, uint32 spawn2id, uint32 timeleft) {

	std::string query = StringFormat("SELECT id, spawngroupID, x, y, z, heading, "
                                    "respawntime, variance, pathgrid, _condition, "
                                    "cond_value, enabled, animation FROM spawn2 "
                                    "WHERE id = %i", spawn2id);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
        return nullptr;
    }

    if (results.RowCount() != 1) {
        return nullptr;
    }

	auto row = results.begin();

    bool perl_enabled = atoi(row[11]) == 1 ? true : false;

    auto newSpawn = new Spawn2(atoi(row[0]), atoi(row[1]), atof(row[2]), atof(row[3]), atof(row[4]), atof(row[5]),
			       atoi(row[6]), atoi(row[7]), timeleft, atoi(row[8]), atoi(row[9]), atoi(row[10]),
			       perl_enabled, (EmuAppearance)atoi(row[12]));

    spawn2_list.Insert(newSpawn);

	return newSpawn;
}

bool ZoneDatabase::CreateSpawn2(Client *client, uint32 spawngroup, const char* zone, const glm::vec4& position, uint32 respawn, uint32 variance, uint16 condition, int16 cond_value)
{

	std::string query = StringFormat("INSERT INTO spawn2 (spawngroupID, zone, x, y, z, heading, "
                                    "respawntime, variance, _condition, cond_value) "
                                    "VALUES (%i, '%s', %f, %f, %f, %f, %i, %i, %u, %i)",
                                    spawngroup, zone, position.x, position.y, position.z, position.w,
                                    respawn, variance, condition, cond_value);
    auto results = QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

    if (results.RowsAffected() != 1)
        return false;

    return true;
}

uint32 Zone::CountSpawn2() {
	LinkedListIterator<Spawn2*> iterator(spawn2_list);
	uint32 count = 0;

	iterator.Reset();
	while(iterator.MoreElements())
	{
		count++;
		iterator.Advance();
	}
	return count;
}

void Zone::Despawn(uint32 spawn2ID) {
	LinkedListIterator<Spawn2*> iterator(spawn2_list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		Spawn2 *cur = iterator.GetData();
		if(spawn2ID == cur->spawn2_id)
			cur->ForceDespawn();
		iterator.Advance();
	}
}

void Spawn2::SpawnConditionChanged(const SpawnCondition &c, int16 old_value) {
	if(GetSpawnCondition() != c.condition_id)
		return;

	LogSpawns("Spawn2 [{}]: Notified that our spawn condition [{}] has changed from [{}] to [{}]. Our min value is [{}]", spawn2_id, c.condition_id, old_value, c.value, condition_min_value);

	bool old_state = (old_value >= condition_min_value);
	bool new_state = (c.value >= condition_min_value);
	if(old_state == new_state) {
		LogSpawns("Spawn2 [{}]: Our threshold for this condition was not crossed. Doing nothing", spawn2_id);
		return;	//no change
	}

	uint32 timer_remaining = 0;
	switch(c.on_change) {
	case SpawnCondition::DoNothing:
		//that was easy.
		LogSpawns("Spawn2 [{}]: Our condition is now [{}]. Taking no action on existing spawn", spawn2_id, new_state?"enabled":"disabled");
		break;
	case SpawnCondition::DoDepop:
		LogSpawns("Spawn2 [{}]: Our condition is now [{}]. Depoping our mob", spawn2_id, new_state?"enabled":"disabled");
		if(npcthis != nullptr)
			npcthis->Depop(false);	//remove the current mob
		Reset();	//reset our spawn timer
		break;
	case SpawnCondition::DoRepop:
		LogSpawns("Spawn2 [{}]: Our condition is now [{}]. Forcing a repop", spawn2_id, new_state?"enabled":"disabled");
		if(npcthis != nullptr)
			npcthis->Depop(false);	//remove the current mob
		Repop();	//repop
		break;
	case SpawnCondition::DoRepopIfReady:
		LogSpawns("Spawn2 [{}]: Our condition is now [{}]. Forcing a repop if repsawn timer is expired", spawn2_id, new_state?"enabled":"disabled");
		if(npcthis != nullptr) {
			LogSpawns("Spawn2 [{}]: Our npcthis is currently not null. The zone thinks it is [{}]. Forcing a depop", spawn2_id, npcthis->GetName());
			npcthis->Depop(false);	//remove the current mob
			npcthis = nullptr;
		}
		if(new_state) { // only get repawn timer remaining when the SpawnCondition is enabled.
			timer_remaining = database.GetSpawnTimeLeft(spawn2_id,zone->GetInstanceID());
			LogSpawns("Spawn2 [{}]: Our condition is now [{}]. The respawn timer_remaining is [{}]. Forcing a repop if it is <= 0", spawn2_id, new_state?"enabled":"disabled", timer_remaining);
			if(timer_remaining <= 0)
				Repop();
		} else {
			LogSpawns("Spawn2 [{}]: Our condition is now [{}]. Not checking respawn timer", spawn2_id, new_state?"enabled":"disabled");
		}
		break;
	default:
		if(c.on_change < SpawnCondition::DoSignalMin) {
			LogSpawns("Spawn2 [{}]: Our condition is now [{}]. Invalid on-change action [{}]", spawn2_id, new_state?"enabled":"disabled", c.on_change);
			return;	//unknown onchange action
		}
		int signal_id = c.on_change - SpawnCondition::DoSignalMin;
		LogSpawns("Spawn2 [{}]: Our condition is now [{}]. Signaling our mob with [{}]", spawn2_id, new_state?"enabled":"disabled", signal_id);
		if(npcthis != nullptr)
			npcthis->SignalNPC(signal_id);
	}
}

void Zone::SpawnConditionChanged(const SpawnCondition &c, int16 old_value) {
	LogSpawns("Zone notified that spawn condition [{}] has changed from [{}] to [{}]. Notifying all spawn points", c.condition_id, old_value, c.value);

	LinkedListIterator<Spawn2*> iterator(spawn2_list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		Spawn2 *cur = iterator.GetData();
		if(cur->GetSpawnCondition() == c.condition_id)
			cur->SpawnConditionChanged(c, old_value);
		iterator.Advance();
	}
}

SpawnCondition::SpawnCondition() {
	condition_id = 0;
	value = 0;
	on_change = DoNothing;
}

SpawnEvent::SpawnEvent() {
	id = 0;
	condition_id = 0;
	enabled = false;
	action = ActionSet;
	argument = 0;
	period = 0xFFFFFFFF;
	strict = false;
	memset(&next, 0, sizeof(next));
}

SpawnConditionManager::SpawnConditionManager()
 : minute_timer(3000)	//1 eq minute
{
	memset(&next_event, 0, sizeof(next_event));
}

void SpawnConditionManager::Process() {
	if(spawn_events.empty())
		return;

	if(minute_timer.Check()) {
		//check each spawn event.

		//get our current time
		TimeOfDay_Struct tod;
		zone->zone_time.GetCurrentEQTimeOfDay(&tod);

		//see if time is past our nearest event.
		if(EQTime::IsTimeBefore(&next_event, &tod))
			return;

		//at least one event should get triggered,
		std::vector<SpawnEvent>::iterator cur,end;
		cur = spawn_events.begin();
		end = spawn_events.end();
		for(; cur != end; ++cur) {
			SpawnEvent &cevent = *cur;

			if(cevent.enabled)
			{
				if(EQTime::IsTimeBefore(&tod, &cevent.next)) {
					//this event has been triggered.
					//execute the event
					if(!cevent.strict || (cevent.strict && cevent.next.hour == tod.hour && cevent.next.day == tod.day && cevent.next.month == tod.month && cevent.next.year == tod.year))
						ExecEvent(cevent, true);

					//add the period of the event to the trigger time
					EQTime::AddMinutes(cevent.period, &cevent.next);
					std::string t;
					EQTime::ToString(&cevent.next, t);
					LogSpawns("Event [{}]: Will trigger again in [{}] EQ minutes at [{}]", cevent.id, cevent.period, t.c_str());
					//save the next event time in the DB
					UpdateDBEvent(cevent);
					//find the next closest event timer.
					FindNearestEvent();
					//minor optimization, if theres no more possible events,
					//then stop trying... I dunno how worth while this is.
					if(EQTime::IsTimeBefore(&next_event, &tod))
						return;
				}
			}
		}
	}
}

void SpawnConditionManager::ExecEvent(SpawnEvent &event, bool send_update) {
	std::map<uint16, SpawnCondition>::iterator condi;
	condi = spawn_conditions.find(event.condition_id);
	if(condi == spawn_conditions.end()) {
		LogSpawns("Event [{}]: Unable to find condition [{}] to execute on", event.id, event.condition_id);
		return;	//unable to find the spawn condition to operate on
	}

	TimeOfDay_Struct tod;
	zone->zone_time.GetCurrentEQTimeOfDay(&tod);
	if(event.strict && (event.next.hour != tod.hour || event.next.day != tod.day || event.next.month != tod.month || event.next.year != tod.year))
	{
		LogSpawns("Event [{}]: Unable to execute. Condition is strict, and event time has already passed", event.id);
		return;
	}

	SpawnCondition &cond = condi->second;

	int16 new_value = cond.value;

	//we have our event and our condition, do our stuff.
	switch(event.action) {
	case SpawnEvent::ActionSet:
		new_value = event.argument;
		LogSpawns("Event [{}]: Executing. Setting condition [{}] to [{}]", event.id, event.condition_id, event.argument);
		break;
	case SpawnEvent::ActionAdd:
		new_value += event.argument;
		LogSpawns("Event [{}]: Executing. Adding [{}] to condition [{}], yielding [{}]", event.id, event.argument, event.condition_id, new_value);
		break;
	case SpawnEvent::ActionSubtract:
		new_value -= event.argument;
		LogSpawns("Event [{}]: Executing. Subtracting [{}] from condition [{}], yielding [{}]", event.id, event.argument, event.condition_id, new_value);
		break;
	case SpawnEvent::ActionMultiply:
		new_value *= event.argument;
		LogSpawns("Event [{}]: Executing. Multiplying condition [{}] by [{}], yielding [{}]", event.id, event.condition_id, event.argument, new_value);
		break;
	case SpawnEvent::ActionDivide:
		new_value /= event.argument;
		LogSpawns("Event [{}]: Executing. Dividing condition [{}] by [{}], yielding [{}]", event.id, event.condition_id, event.argument, new_value);
		break;
	default:
		LogSpawns("Event [{}]: Invalid event action type [{}]", event.id, event.action);
		return;
	}

	//now set the condition to the new value
	if(send_update)	//full blown update
		SetCondition(zone->GetShortName(), zone->GetInstanceID(), cond.condition_id, new_value);
	else	//minor update done while loading
		cond.value = new_value;
}

void SpawnConditionManager::UpdateDBEvent(SpawnEvent &event) {

	std::string query = StringFormat("UPDATE spawn_events SET "
                                    "next_minute = %d, next_hour = %d, "
                                    "next_day = %d, next_month = %d, "
                                    "next_year = %d, enabled = %d, "
                                    "strict = %d WHERE id = %d",
                                    event.next.minute, event.next.hour,
                                    event.next.day, event.next.month,
                                    event.next.year, event.enabled? 1: 0,
                                    event.strict? 1: 0, event.id);
	database.QueryDatabase(query);
}

void SpawnConditionManager::UpdateDBCondition(const char* zone_name, uint32 instance_id, uint16 cond_id, int16 value) {

	std::string query = StringFormat("REPLACE INTO spawn_condition_values "
                                    "(id, value, zone, instance_id) "
                                    "VALUES( %u, %u, '%s', %u)",
                                    cond_id, value, zone_name, instance_id);
    database.QueryDatabase(query);
}

bool SpawnConditionManager::LoadDBEvent(uint32 event_id, SpawnEvent &event, std::string &zone_name) {

    std::string query = StringFormat("SELECT id, cond_id, period, "
                                    "next_minute, next_hour, next_day, "
                                    "next_month, next_year, enabled, "
                                    "action, argument, strict, zone "
                                    "FROM spawn_events WHERE id = %d", event_id);
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
		return false;
	}

    if (results.RowCount() == 0)
        return false;

	auto row = results.begin();

    event.id = atoi(row[0]);
    event.condition_id = atoi(row[1]);
    event.period = atoi(row[2]);

    event.next.minute = atoi(row[3]);
    event.next.hour = atoi(row[4]);
    event.next.day = atoi(row[5]);
    event.next.month = atoi(row[6]);
    event.next.year = atoi(row[7]);

    event.enabled = atoi(row[8]) != 0;
    event.action = (SpawnEvent::Action) atoi(row[9]);
    event.argument = atoi(row[10]);
    event.strict = atoi(row[11]) != 0;
    zone_name = row[12];

    std::string timeAsString;
    EQTime::ToString(&event.next, timeAsString);

  LogSpawns("(LoadDBEvent) Loaded [{}] spawn event [{}] on condition [{}] with period [{}], action [{}], argument [{}], strict [{}]. Will trigger at [{}]", event.enabled? "enabled": "disabled", event.id, event.condition_id, event.period, event.action, event.argument, event.strict, timeAsString.c_str());

	return true;
}

bool SpawnConditionManager::LoadSpawnConditions(const char* zone_name, uint32 instance_id)
{
	//clear out old stuff..
	spawn_conditions.clear();

	std::string query = StringFormat("SELECT id, onchange, value "
                                    "FROM spawn_conditions "
                                    "WHERE zone = '%s'", zone_name);
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

    for (auto row = results.begin(); row != results.end(); ++row) {
        //load spawn conditions
        SpawnCondition cond;

        cond.condition_id = atoi(row[0]);
        cond.value = atoi(row[2]);
        cond.on_change = (SpawnCondition::OnChange) atoi(row[1]);
        spawn_conditions[cond.condition_id] = cond;

    LogSpawns("Loaded spawn condition [{}] with value [{}] and on_change [{}]", cond.condition_id, cond.value, cond.on_change);
    }

	//load values
	query = StringFormat("SELECT id, value FROM spawn_condition_values "
                        "WHERE zone = '%s' AND instance_id = %u",
                        zone_name, instance_id);
    results = database.QueryDatabase(query);
    if (!results.Success()) {
		spawn_conditions.clear();
		return false;
    }

    for (auto row = results.begin(); row != results.end(); ++row) {
        auto iter = spawn_conditions.find(atoi(row[0]));

        if(iter != spawn_conditions.end())
            iter->second.value = atoi(row[1]);
    }

	//load spawn events
    query = StringFormat("SELECT id, cond_id, period, next_minute, next_hour, "
                        "next_day, next_month, next_year, enabled, action, argument, strict "
                        "FROM spawn_events WHERE zone = '%s'", zone_name);
    results = database.QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

	for (auto row = results.begin(); row != results.end(); ++row) {
		SpawnEvent event;

		event.id           = atoi(row[0]);
		event.condition_id = atoi(row[1]);
		event.period       = atoi(row[2]);

		if (event.period == 0) {
			LogError("Refusing to load spawn event #[{}] because it has a period of 0\n", event.id);
			continue;
		}

		event.next.minute = atoi(row[3]);
		event.next.hour   = atoi(row[4]);
		event.next.day    = atoi(row[5]);
		event.next.month  = atoi(row[6]);
		event.next.year   = atoi(row[7]);

		event.enabled  = atoi(row[8]) == 0 ? false : true;
		event.action   = (SpawnEvent::Action) atoi(row[9]);
		event.argument = atoi(row[10]);
		event.strict   = atoi(row[11]) == 0 ? false : true;

		spawn_events.push_back(event);

		LogSpawns(
			"(LoadSpawnConditions) Loaded [{}] spawn event [{}] on condition [{}] with period [{}], action [{}], argument [{}], strict [{}]",
			event.enabled ? "enabled" : "disabled",
			event.id,
			event.condition_id,
			event.period,
			event.action,
			event.argument,
			event.strict
		);
	}

	//now we need to catch up on events that happened while we were away
	//and use them to alter just the condition variables.

	//each spawn2 will then use its correct condition value when
	//it decides what to do. This essentially forces a 'depop' action
	//on spawn points which are turned off, and a 'repop' action on
	//spawn points which get turned on. Im too lazy to figure out a
	//better solution, and I just dont care thats much.
	//get our current time
	TimeOfDay_Struct tod;
	zone->zone_time.GetCurrentEQTimeOfDay(&tod);

	for(auto cur = spawn_events.begin(); cur != spawn_events.end(); ++cur) {
		SpawnEvent &cevent = *cur;

		bool StrictCheck = false;
		if(cevent.strict &&
			cevent.next.hour == tod.hour &&
			cevent.next.day == tod.day &&
			cevent.next.month == tod.month &&
			cevent.next.year == tod.year)
			StrictCheck = true;

		//If event is disabled, or we failed the strict check, set initial spawn_condition to 0.
		if(!cevent.enabled || !StrictCheck)
			SetCondition(zone->GetShortName(), zone->GetInstanceID(),cevent.condition_id,0);

		if(!cevent.enabled)
            continue;

		//watch for special case of all 0s, which means to reset next to now
		if (cevent.next.year == 0 && cevent.next.month == 0 && cevent.next.day == 0 && cevent.next.hour == 0 &&
			cevent.next.minute == 0) {
			LogSpawns("Initial next trigger time set for spawn event [{}]", cevent.id);
			memcpy(&cevent.next, &tod, sizeof(cevent.next));
			//add one period
			EQTime::AddMinutes(cevent.period, &cevent.next);
			//save it in the db.
			UpdateDBEvent(cevent);
			continue;    //were done with this event.
		}

		bool ran = false;
		while (EQTime::IsTimeBefore(&tod, &cevent.next)) {
			LogSpawns("Catch up triggering on event [{}]", cevent.id);
			//this event has been triggered.
			//execute the event
			if (!cevent.strict || StrictCheck) {
				ExecEvent(cevent, false);
			}

			//add the period of the event to the trigger time
			EQTime::AddMinutes(cevent.period, &cevent.next);
			ran = true;
		}

		//only write it out if the event actually ran
        if(ran)
            UpdateDBEvent(cevent); //save the event in the DB
	}

	//now our event timers are all up to date, find our closest event.
	FindNearestEvent();

	return true;
}

void SpawnConditionManager::FindNearestEvent() {
	//set a huge year which should never get reached normally
	next_event.year = 0xFFFFFF;

	std::vector<SpawnEvent>::iterator cur,end;
	cur = spawn_events.begin();
	end = spawn_events.end();
	int next_id = -1;
	for(; cur != end; ++cur) {
		SpawnEvent &cevent = *cur;
		if(cevent.enabled)
		{
			//see if this event is before our last nearest
			if(EQTime::IsTimeBefore(&next_event, &cevent.next))
			{
				memcpy(&next_event, &cevent.next, sizeof(next_event));
				next_id = cevent.id;
			}
		}
	}
	if (next_id == -1) {
		LogSpawns("No spawn events enabled. Disabling next event");
	}
	else {
		LogSpawns("Next event determined to be event [{}]", next_id);
	}
}

void SpawnConditionManager::SetCondition(const char *zone_short, uint32 instance_id, uint16 condition_id, int16 new_value, bool world_update)
{
	if(world_update) {
		//this is an update coming from another zone, they
		//have allready updated the DB, just need to update our
		//memory, and check for condition changes
		std::map<uint16, SpawnCondition>::iterator condi;
		condi = spawn_conditions.find(condition_id);
		if(condi == spawn_conditions.end()) {
			LogSpawns("Condition update received from world for [{}], but we do not have that conditon", condition_id);
			return;	//unable to find the spawn condition
		}

		SpawnCondition &cond = condi->second;

		if(cond.value == new_value) {
			LogSpawns("Condition update received from world for [{}] with value [{}], which is what we already have", condition_id, new_value);
			return;
		}

		int16 old_value = cond.value;

		//set our local value
		cond.value = new_value;

		LogSpawns("Condition update received from world for [{}] with value [{}]", condition_id, new_value);

		//now we have to test each spawn point to see if it changed.
		zone->SpawnConditionChanged(cond, old_value);
	} else if(!strcasecmp(zone_short, zone->GetShortName()) && instance_id == zone->GetInstanceID())
	{
		//this is a local spawn condition, we need to update the DB,
		//our memory, then notify spawn points of the change.
		std::map<uint16, SpawnCondition>::iterator condi;
		condi = spawn_conditions.find(condition_id);
		if(condi == spawn_conditions.end()) {
			LogSpawns("Local Condition update requested for [{}], but we do not have that conditon", condition_id);
			return;	//unable to find the spawn condition
		}

		SpawnCondition &cond = condi->second;

		if(cond.value == new_value) {
			LogSpawns("Local Condition update requested for [{}] with value [{}], which is what we already have", condition_id, new_value);
			return;
		}

		int16 old_value = cond.value;

		//set our local value
		cond.value = new_value;
		//save it in the DB too
		UpdateDBCondition(zone_short, instance_id, condition_id, new_value);

		LogSpawns("Local Condition update requested for [{}] with value [{}]", condition_id, new_value);

		//now we have to test each spawn point to see if it changed.
		zone->SpawnConditionChanged(cond, old_value);
	}
	else
	{
		//this is a remote spawn condition, update the DB and send
		//an update packet to the zone if its up

		LogSpawns("Remote spawn condition [{}] set to [{}]. Updating DB and notifying world", condition_id, new_value);

		UpdateDBCondition(zone_short, instance_id, condition_id, new_value);

		auto pack = new ServerPacket(ServerOP_SpawnCondition, sizeof(ServerSpawnCondition_Struct));
		ServerSpawnCondition_Struct* ssc = (ServerSpawnCondition_Struct*)pack->pBuffer;

		ssc->zoneID = database.GetZoneID(zone_short);
		ssc->instanceID = instance_id;
		ssc->condition_id = condition_id;
		ssc->value = new_value;

		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void SpawnConditionManager::ReloadEvent(uint32 event_id) {
	std::string zone_short_name;

	LogSpawns("Requested to reload event [{}] from the database", event_id);

	//first look for the event in our local event list
	std::vector<SpawnEvent>::iterator cur,end;
	cur = spawn_events.begin();
	end = spawn_events.end();
	for(; cur != end; ++cur) {
		SpawnEvent &cevent = *cur;

		if(cevent.id == event_id) {
			//load the event into the old event slot
			if(!LoadDBEvent(event_id, cevent, zone_short_name)) {
				//unable to find the event in the database...
				LogSpawns("Failed to reload event [{}] from the database", event_id);
				return;
			}
			//sync up our nearest event
			FindNearestEvent();
			return;
		}
	}

	//if we get here, it is a new event...
	SpawnEvent e;
	if(!LoadDBEvent(event_id, e, zone_short_name)) {
		//unable to find the event in the database...
		LogSpawns("Failed to reload event [{}] from the database", event_id);
		return;
	}

	//we might want to check the next timer like we do on
	//regular load events, but we are assuming this is a new event
	//and anyways, this will get handled (albeit not optimally)
	//naturally by the event handling code.

	spawn_events.push_back(e);

	//sync up our nearest event
	FindNearestEvent();
}

void SpawnConditionManager::ToggleEvent(uint32 event_id, bool enabled, bool strict, bool reset_base) {

	LogSpawns("Request to [{}] spawn event [{}] [{}]resetting trigger time", enabled?"enable":"disable", event_id, reset_base?"":"without ");

	//first look for the event in our local event list
	std::vector<SpawnEvent>::iterator cur,end;
	cur = spawn_events.begin();
	end = spawn_events.end();
	for(; cur != end; ++cur) {
		SpawnEvent &cevent = *cur;

		if(cevent.id == event_id) {
			//make sure were actually changing something
			if(cevent.enabled != enabled || reset_base || cevent.strict != strict) {
				cevent.enabled = enabled;
				cevent.strict = strict;
				if(reset_base) {
					LogSpawns("Spawn event [{}] located in this zone. State set. Trigger time reset (period [{}])", event_id, cevent.period);
					//start with the time now
					zone->zone_time.GetCurrentEQTimeOfDay(&cevent.next);
					//advance the next time by our period
					EQTime::AddMinutes(cevent.period, &cevent.next);
				} else {
					LogSpawns("Spawn event [{}] located in this zone. State changed", event_id);
				}

				//save the event in the DB
				UpdateDBEvent(cevent);

				//sync up our nearest event
				FindNearestEvent();
			} else {
				LogSpawns("Spawn event [{}] located in this zone but no change was needed", event_id);
			}
			//even if we dont change anything, we still found it
			return;
		}
	}

	//if we get here, the condition must be in another zone.
	//first figure out what zone it applies to.
	//update the DB and send and send an update packet to
	//the zone if its up

	//we need to load the event from the DB and then update
	//the values in the DB, because we do not know if the zone
	//is up or not. The message to the zone will just tell it to
	//update its in-memory event list
	SpawnEvent e;
	std::string zone_short_name;
	if(!LoadDBEvent(event_id, e, zone_short_name)) {
		LogSpawns("Unable to find spawn event [{}] in the database", event_id);
		//unable to find the event in the database...
		return;
	}
	if(e.enabled == enabled && !reset_base) {
		LogSpawns("Spawn event [{}] is not located in this zone but no change was needed", event_id);
		return;	//no changes.
	}

	e.enabled = enabled;
	if(reset_base) {
		LogSpawns("Spawn event [{}] is in zone [{}]. State set. Trigger time reset (period [{}]). Notifying world", event_id, zone_short_name.c_str(), e.period);
		//start with the time now
		zone->zone_time.GetCurrentEQTimeOfDay(&e.next);
		//advance the next time by our period
		EQTime::AddMinutes(e.period, &e.next);
	} else {
		LogSpawns("Spawn event [{}] is in zone [{}]. State changed. Notifying world", event_id, zone_short_name.c_str(), e.period);
	}
	//save the event in the DB
	UpdateDBEvent(e);


	//now notify the zone
	auto pack = new ServerPacket(ServerOP_SpawnEvent, sizeof(ServerSpawnEvent_Struct));
	ServerSpawnEvent_Struct* sse = (ServerSpawnEvent_Struct*)pack->pBuffer;

	sse->zoneID = database.GetZoneID(zone_short_name.c_str());
	sse->event_id = event_id;

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

int16 SpawnConditionManager::GetCondition(const char *zone_short, uint32 instance_id, uint16 condition_id) {
	if(!strcasecmp(zone_short, zone->GetShortName()) && instance_id == zone->GetInstanceID())
	{
		//this is a local spawn condition
		std::map<uint16, SpawnCondition>::iterator condi;
		condi = spawn_conditions.find(condition_id);
		if(condi == spawn_conditions.end())
		{
			LogSpawns("Unable to find local condition [{}] in Get request", condition_id);
			return(0);	//unable to find the spawn condition
		}

		SpawnCondition &cond = condi->second;
		return cond.value;
	}

	//this is a remote spawn condition, grab it from the DB
	//load spawn conditions
	std::string query   = StringFormat(
		"SELECT value FROM spawn_condition_values "
		"WHERE zone = '%s' AND instance_id = %u AND id = %d",
		zone_short, instance_id, condition_id
	);
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogSpawns("Unable to query remote condition [{}] from zone [{}] in Get request", condition_id, zone_short);
		return 0;    //dunno a better thing to do...
	}

	if (results.RowCount() == 0) {
		LogSpawns("Unable to load remote condition [{}] from zone [{}] in Get request", condition_id, zone_short);
		return 0;    //dunno a better thing to do...
	}

    auto row = results.begin();

    return atoi(row[0]);
}

bool SpawnConditionManager::Check(uint16 condition, int16 min_value) {
	std::map<uint16, SpawnCondition>::iterator condi;
	condi = spawn_conditions.find(condition);
	if(condi == spawn_conditions.end())
		return(false);	//unable to find the spawn condition

	SpawnCondition &cond = condi->second;

	return(cond.value >= min_value);
}

