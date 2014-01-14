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
#include "spawngroup.h"
#include "entity.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "../common/types.h"
#include "zonedb.h"
#include "../common/MiscFunctions.h"
#include "../common/StringUtil.h"

extern EntityList entity_list;

SpawnEntry::SpawnEntry( uint32 in_NPCType, int in_chance, uint8 in_npc_spawn_limit )
{
	NPCType = in_NPCType;
	chance = in_chance;
	npc_spawn_limit = in_npc_spawn_limit;
}

SpawnGroup::SpawnGroup( uint32 in_id, char* name, int in_group_spawn_limit, float dist, float maxx, float minx, float maxy, float miny, int delay_in, int despawn_in, uint32 despawn_timer_in ) {
	id = in_id;
	strn0cpy( name_, name, 120);
	group_spawn_limit = in_group_spawn_limit;
	roambox[0]=maxx;
	roambox[1]=minx;
	roambox[2]=maxy;
	roambox[3]=miny;
	roamdist=dist;
	delay=delay_in;
	despawn=despawn_in;
	despawn_timer=despawn_timer_in;
}

uint32 SpawnGroup::GetNPCType() {
#if EQDEBUG >= 10
	LogFile->write(EQEMuLog::Debug, "SpawnGroup[%08x]::GetNPCType()", (uint32) this);
#endif
	int npcType = 0;
	int totalchance = 0;

	//check limits on this spawn group and npc type
	if(!entity_list.LimitCheckGroup(id, group_spawn_limit))
		return(0);

	std::list<SpawnEntry*>::iterator cur,end;
	std::list<SpawnEntry*> possible;
	cur = list_.begin();
	end = list_.end();
	for(; cur != end; ++cur) {
		SpawnEntry *se = *cur;

		//check limits on this spawn group and npc type
		if(!entity_list.LimitCheckType(se->NPCType, se->npc_spawn_limit))
			continue;

		totalchance += se->chance;
		possible.push_back(se);
	}
	if(totalchance == 0)
		return 0;


	int32 roll = 0;
	roll = MakeRandomInt(0, totalchance-1);

	cur = possible.begin();
	end = possible.end();
	for(; cur != end; ++cur) {
		SpawnEntry *se = *cur;
		if (roll < se->chance) {
			npcType = se->NPCType;
			break;
		} else {
			roll -= se->chance;
		}
	}
	//CODER implement random table
	return npcType;
}

void SpawnGroup::AddSpawnEntry( SpawnEntry* newEntry ) {
	list_.push_back( newEntry );
}

SpawnGroup::~SpawnGroup() {
	std::list<SpawnEntry*>::iterator cur,end;
	cur = list_.begin();
	end = list_.end();
	for(; cur != end; ++cur) {
		SpawnEntry* tmp = *cur;
		safe_delete(tmp);
	}
	list_.clear();
}

SpawnGroupList::~SpawnGroupList() {
	std::map<uint32, SpawnGroup*>::iterator cur,end;
	cur = groups.begin();
	end = groups.end();
	for(; cur != end; ++cur) {
		SpawnGroup* tmp = cur->second;
		safe_delete(tmp);
	}
	groups.clear();
}

void SpawnGroupList::AddSpawnGroup(SpawnGroup* newGroup) {
	if(newGroup == nullptr)
		return;
	groups[newGroup->id] = newGroup;
}

SpawnGroup* SpawnGroupList::GetSpawnGroup(uint32 in_id) {
	if(groups.count(in_id) != 1)
		return nullptr;
	return(groups[in_id]);
}

bool SpawnGroupList::RemoveSpawnGroup(uint32 in_id) {
	if(groups.count(in_id) != 1)
		return(false);

	groups.erase(in_id);
	return(true);
}

bool ZoneDatabase::LoadSpawnGroups(const char* zone_name, uint16 version, SpawnGroupList* spawn_group_list) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	// CODER new spawn code
	query = 0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT DISTINCT(spawngroupID), spawngroup.name, spawngroup.spawn_limit, spawngroup.dist, spawngroup.max_x, spawngroup.min_x, spawngroup.max_y, spawngroup.min_y, spawngroup.delay, spawngroup.despawn, spawngroup.despawn_timer FROM spawn2,spawngroup WHERE spawn2.spawngroupID=spawngroup.ID and spawn2.version=%u and zone='%s'", version, zone_name), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result))) {
			SpawnGroup* newSpawnGroup = new SpawnGroup( atoi(row[0]), row[1], atoi(row[2]), atof(row[3]), atof(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atoi(row[8]), atoi(row[9]), atoi(row[10]));
			spawn_group_list->AddSpawnGroup(newSpawnGroup);
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error2 in PopulateZoneLists query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	query = 0;
	if (RunQuery(query, MakeAnyLenString(&query,
		"SELECT DISTINCT spawnentry.spawngroupID, npcid, chance, "
		"npc_types.spawn_limit AS sl "
		"FROM spawnentry, spawn2, npc_types "
		"WHERE spawnentry.npcID=npc_types.id AND spawnentry.spawngroupID=spawn2.spawngroupID "
		"AND zone='%s'", zone_name), errbuf, &result)) {
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)))
		{
			SpawnEntry* newSpawnEntry = new SpawnEntry( atoi(row[1]), atoi(row[2]), row[3]?atoi(row[3]):0);
			SpawnGroup *sg = spawn_group_list->GetSpawnGroup(atoi(row[0]));
			if (sg)
				sg->AddSpawnEntry(newSpawnEntry);
			else
				std::cout << "Error in SpawngroupID: " << row[0] << std::endl;
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error3 in PopulateZoneLists query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	// CODER end new spawn code
	return true;
}

bool ZoneDatabase::LoadSpawnGroupsByID(int spawngroupid, SpawnGroupList* spawn_group_list) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;


	// CODER new spawn code
	query = 0;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT DISTINCT spawngroup.id, spawngroup.name, spawngroup.spawn_limit, spawngroup.dist, spawngroup.max_x, spawngroup.min_x, spawngroup.max_y, spawngroup.min_y, spawngroup.delay, spawngroup.despawn, spawngroup.despawn_timer FROM spawngroup WHERE spawngroup.ID='%i'", spawngroupid), errbuf, &result))
	{
		safe_delete_array(query);
		while((row = mysql_fetch_row(result))) {
			SpawnGroup* newSpawnGroup = new SpawnGroup( atoi(row[0]), row[1], atoi(row[2]), atof(row[3]), atof(row[4]), atof(row[5]), atof(row[6]), atof(row[7]), atoi(row[8]), atoi(row[9]), atoi(row[10]));
			spawn_group_list->AddSpawnGroup(newSpawnGroup);
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error2 in PopulateZoneLists query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	query = 0;
	if (RunQuery(query, MakeAnyLenString(&query,
		"SELECT DISTINCT spawnentry.spawngroupID, spawnentry.npcid, spawnentry.chance, spawngroup.spawn_limit FROM spawnentry,spawngroup WHERE spawnentry.spawngroupID='%i' AND spawngroup.spawn_limit='0' ORDER by chance", spawngroupid), errbuf, &result)) {
		safe_delete_array(query);
		while((row = mysql_fetch_row(result)))
		{
			SpawnEntry* newSpawnEntry = new SpawnEntry( atoi(row[1]), atoi(row[2]), row[3]?atoi(row[3]):0);
			SpawnGroup *sg = spawn_group_list->GetSpawnGroup(atoi(row[0]));
			if (sg)
				sg->AddSpawnEntry(newSpawnEntry);
			else
				std::cout << "Error in SpawngroupID: " << row[0] << std::endl;
		}
		mysql_free_result(result);
	}
	else
	{
		std::cerr << "Error3 in PopulateZoneLists query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return false;
	}

	// CODER end new spawn code
	return true;
}
