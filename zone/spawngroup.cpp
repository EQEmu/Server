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

#include <fmt/format.h>
#include "../common/global_define.h"
#include "../common/types.h"

#include "entity.h"
#include "spawngroup.h"
#include "zone.h"
#include "zonedb.h"

extern EntityList entity_list;
extern Zone       *zone;

SpawnEntry::SpawnEntry(uint32 in_NPCType, int in_chance, uint8 in_npc_spawn_limit)
{
	NPCType         = in_NPCType;
	chance          = in_chance;
	npc_spawn_limit = in_npc_spawn_limit;
}

SpawnGroup::SpawnGroup(
	uint32 in_id,
	char *name,
	int in_group_spawn_limit,
	float dist,
	float maxx,
	float minx,
	float maxy,
	float miny,
	int delay_in,
	int despawn_in,
	uint32 despawn_timer_in,
	int min_delay_in
)
{
	id = in_id;
	strn0cpy(name_, name, 120);
	group_spawn_limit = in_group_spawn_limit;
	roambox[0] = maxx;
	roambox[1] = minx;
	roambox[2] = maxy;
	roambox[3] = miny;
	roamdist      = dist;
	min_delay     = min_delay_in;
	delay         = delay_in;
	despawn       = despawn_in;
	despawn_timer = despawn_timer_in;
}

uint32 SpawnGroup::GetNPCType()
{
#if EQDEBUG >= 10
	Log(Logs::General, Logs::None, "SpawnGroup[%08x]::GetNPCType()", (uint32) this);
#endif
	int npcType     = 0;
	int totalchance = 0;

	if (!entity_list.LimitCheckGroup(id, group_spawn_limit)) {
		return (0);
	}

	std::list<SpawnEntry *>::iterator cur, end;
	std::list<SpawnEntry *>           possible;
	cur = list_.begin();
	end = list_.end();
	for (; cur != end; ++cur) {
		SpawnEntry *se = *cur;

		if (!entity_list.LimitCheckType(se->NPCType, se->npc_spawn_limit)) {
			continue;
		}

		totalchance += se->chance;
		possible.push_back(se);
	}
	if (totalchance == 0) {
		return 0;
	}


	int32 roll = 0;
	roll = zone->random.Int(0, totalchance - 1);

	cur = possible.begin();
	end = possible.end();
	for (; cur != end; ++cur) {
		SpawnEntry *se = *cur;
		if (roll < se->chance) {
			npcType = se->NPCType;
			break;
		}
		else {
			roll -= se->chance;
		}
	}
	return npcType;
}

void SpawnGroup::AddSpawnEntry(SpawnEntry *newEntry)
{
	list_.push_back(newEntry);
}

SpawnGroup::~SpawnGroup()
{
	std::list<SpawnEntry *>::iterator cur, end;
	cur = list_.begin();
	end = list_.end();
	for (; cur != end; ++cur) {
		SpawnEntry *tmp = *cur;
		safe_delete(tmp);
	}
	list_.clear();
}

SpawnGroupList::~SpawnGroupList()
{
	std::map<uint32, SpawnGroup *>::iterator cur, end;
	cur = m_spawn_groups.begin();
	end = m_spawn_groups.end();
	for (; cur != end; ++cur) {
		SpawnGroup *tmp = cur->second;
		safe_delete(tmp);
	}
	m_spawn_groups.clear();
}

void SpawnGroupList::AddSpawnGroup(SpawnGroup *new_group)
{
	if (new_group == nullptr) {
		return;
	}

	m_spawn_groups[new_group->id] = new_group;
}

SpawnGroup *SpawnGroupList::GetSpawnGroup(uint32 in_id)
{
	if (m_spawn_groups.count(in_id) != 1) {
		return nullptr;
	}

	return (m_spawn_groups[in_id]);
}

bool SpawnGroupList::RemoveSpawnGroup(uint32 in_id)
{
	if (m_spawn_groups.count(in_id) != 1) {
		return (false);
	}

	m_spawn_groups.erase(in_id);

	return (true);
}

void SpawnGroupList::ReloadSpawnGroups()
{
	ClearSpawnGroups();
	database.LoadSpawnGroups(zone->GetShortName(), zone->GetInstanceVersion(), &zone->spawn_group_list);
}

void SpawnGroupList::ClearSpawnGroups()
{
	m_spawn_groups.clear();
}

bool ZoneDatabase::LoadSpawnGroups(const char *zone_name, uint16 version, SpawnGroupList *spawn_group_list)
{
	std::string query = fmt::format(
		SQL(
			SELECT
			DISTINCT(spawngroupID),
			spawngroup.name,
			spawngroup.spawn_limit,
			spawngroup.dist,
			spawngroup.max_x,
			spawngroup.min_x,
			spawngroup.max_y,
			spawngroup.min_y,
			spawngroup.delay,
			spawngroup.despawn,
			spawngroup.despawn_timer,
			spawngroup.mindelay
				FROM
				spawn2,
			spawngroup
				WHERE
				spawn2.spawngroupID = spawngroup.ID
				AND
				spawn2.version = {} and zone = '{}'
		),
		version,
		zone_name
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto new_spawn_group = new SpawnGroup(
			atoi(row[0]),
			row[1],
			atoi(row[2]),
			atof(row[3]),
			atof(row[4]),
			atof(row[5]),
			atof(row[6]),
			atof(row[7]),
			atoi(row[8]),
			atoi(row[9]),
			atoi(row[10]),
			atoi(row[11])
		);

		spawn_group_list->AddSpawnGroup(new_spawn_group);
	}

	query = fmt::format(
		SQL(
			SELECT
				DISTINCT
			spawnentry.spawngroupID,
			npcid,
			chance,
			npc_types.spawn_limit
				AS sl
				FROM
				spawnentry,
			spawn2,
			npc_types
				WHERE
				spawnentry.npcID = npc_types.id
				AND
				spawnentry.spawngroupID = spawn2.spawngroupID
				AND
				zone = '{}'),
		zone_name
	);

	results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto new_spawn_entry = new SpawnEntry(
			atoi(row[1]),
			atoi(row[2]),
			(row[3] ? atoi(row[3]) : 0)
		);

		SpawnGroup *spawn_group = spawn_group_list->GetSpawnGroup(atoi(row[0]));

		if (!spawn_group) {
			safe_delete(new_spawn_entry);
			continue;
		}

		spawn_group->AddSpawnEntry(new_spawn_entry);
	}

	return true;
}

/**
 * @param spawn_group_id
 * @param spawn_group_list
 * @return
 */
bool ZoneDatabase::LoadSpawnGroupsByID(int spawn_group_id, SpawnGroupList *spawn_group_list)
{
	std::string query = fmt::format(
		SQL(
			SELECT DISTINCT
			(spawngroup.id),
			spawngroup.name,
			spawngroup.spawn_limit,
			spawngroup.dist,
			spawngroup.max_x,
			spawngroup.min_x,
			spawngroup.max_y,
			spawngroup.min_y,
			spawngroup.delay,
			spawngroup.despawn,
			spawngroup.despawn_timer,
			spawngroup.mindelay
				FROM
					spawngroup
				WHERE
				spawngroup.ID = '{}'
		),
		spawn_group_id
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto new_spawn_group = new SpawnGroup(
			atoi(row[0]),
			row[1],
			atoi(row[2]),
			atof(row[3]),
			atof(row[4]),
			atof(row[5]),
			atof(row[6]),
			atof(row[7]),
			atoi(row[8]),
			atoi(row[9]),
			atoi(row[10]),
			atoi(row[11])
		);

		spawn_group_list->AddSpawnGroup(new_spawn_group);
	}

	query   = fmt::format(
		SQL(
			SELECT DISTINCT
			(spawnentry.spawngroupID),
			spawnentry.npcid,
			spawnentry.chance,
			spawngroup.spawn_limit
				FROM
				spawnentry,
			spawngroup
				WHERE
				spawnentry.spawngroupID = '{}'
				AND spawngroup.spawn_limit = '0'
				ORDER BY chance),
		spawn_group_id
	);

	results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto new_spawn_entry = new SpawnEntry(
			atoi(row[1]),
			atoi(row[2]),
			(row[3] ? atoi(row[3]) : 0)
		);

		SpawnGroup *spawn_group = spawn_group_list->GetSpawnGroup(atoi(row[0]));
		if (!spawn_group) {
			safe_delete(new_spawn_entry);
			continue;
		}

		spawn_group->AddSpawnEntry(new_spawn_entry);
	}

	return true;
}
