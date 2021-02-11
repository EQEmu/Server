/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/timer.h"
#include "../common/repositories/dynamic_zone_members_repository.h"
#include "../common/repositories/dynamic_zones_repository.h"

#include "database.h"

#include <iomanip>
#include <iostream>

// Disgrace: for windows compile
#ifdef _WINDOWS
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "unix.h"
#include "../zone/zonedb.h"
#include <netinet/in.h>
#include <sys/time.h>
#endif

/**
 * @param instance_id
 * @param char_id
 * @return
 */
bool Database::AddClientToInstance(uint16 instance_id, uint32 char_id)
{
	std::string query = StringFormat(
		"REPLACE INTO `instance_list_player` (id, charid) "
		"VALUES "
		"(%lu, %lu)",
		(unsigned long) instance_id,
		(unsigned long) char_id
	);

	auto results = QueryDatabase(query);

	return results.Success();
}

bool Database::CharacterInInstanceGroup(uint16 instance_id, uint32 char_id)
{

	std::string query = StringFormat("SELECT charid FROM instance_list_player where id=%u AND charid=%u", instance_id, char_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() != 1)
		return false;

	return true;
}

bool Database::CheckInstanceExists(uint16 instance_id) {
	std::string query = StringFormat(
		"SELECT "
		"`id`  "
		"FROM  "
		"`instance_list`  "
		"WHERE  "
		"`id` = %u",
		instance_id
		);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	return true;
}

bool Database::CheckInstanceExpired(uint16 instance_id)
{

	int32  start_time    = 0;
	int32  duration      = 0;
	uint32 never_expires = 0;

	std::string query = StringFormat(
		"SELECT start_time, duration, never_expires FROM instance_list WHERE id=%u",
		instance_id
	);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return true;
	}

	if (results.RowCount() == 0) {
		return true;
	}

	auto row = results.begin();

	start_time    = atoi(row[0]);
	duration      = atoi(row[1]);
	never_expires = atoi(row[2]);

	if (never_expires == 1) {
		return false;
	}

	timeval tv{};
	gettimeofday(&tv, nullptr);

	return (start_time + duration) <= tv.tv_sec;

}

bool Database::CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration)
{
	std::string query = StringFormat(
		"INSERT INTO instance_list (id, zone, version, start_time, duration)"
		" values (%u, %u, %u, UNIX_TIMESTAMP(), %u)",
		instance_id,
		zone_id,
		version,
		duration
	);

	auto results = QueryDatabase(query);

	return results.Success();
}

bool Database::GetUnusedInstanceID(uint16 &instance_id)
{
	uint32 max_reserved_instance_id = RuleI(Instances, ReservedInstances);
	uint32 max                      = 32000;

	std::string query = StringFormat(
		"SELECT IFNULL(MAX(id),%u)+1 FROM instance_list WHERE id > %u",
		max_reserved_instance_id,
		max_reserved_instance_id
	);

	if (RuleB(Instances, RecycleInstanceIds)) {
		query = (
			SQL(
				SELECT i.id + 1 AS next_available
				FROM instance_list i
				LEFT JOIN instance_list i2 ON i2.id = i.id + 1
				WHERE i2.id IS NULL
				ORDER BY i.id
				LIMIT 0, 1;

			)
		);
	}

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		instance_id = 0;
		return false;
	}

	if (results.RowCount() == 0) {
		instance_id = max_reserved_instance_id;
		return true;
	}

	auto row = results.begin();

	if (atoi(row[0]) <= max) {
		instance_id = atoi(row[0]);

		return true;
	}

	if (instance_id < max_reserved_instance_id) {
		instance_id = max_reserved_instance_id;
		return true;
	}

	query   = StringFormat("SELECT id FROM instance_list where id > %u ORDER BY id", max_reserved_instance_id);
	results = QueryDatabase(query);

	if (!results.Success()) {
		instance_id = 0;
		return false;
	}

	if (results.RowCount() == 0) {
		instance_id = 0;
		return false;
	}

	max_reserved_instance_id++;
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (max_reserved_instance_id < atoi(row[0])) {
			instance_id = max_reserved_instance_id;
			return true;
		}

		if (max_reserved_instance_id > max) {
			instance_id = 0;
			return false;
		}

		max_reserved_instance_id++;
	}

	instance_id = max_reserved_instance_id;

	return true;
}

bool Database::GlobalInstance(uint16 instance_id)
{
	std::string query = StringFormat(
		"SELECT "
		"is_global "
		"FROM "
		"instance_list "
		"WHERE "
		"id = %u "
		"LIMIT 1 ",
		instance_id
		);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	return (atoi(row[0]) == 1) ? true : false;
}

bool Database::RemoveClientFromInstance(uint16 instance_id, uint32 char_id)
{
	std::string query = StringFormat("DELETE FROM instance_list_player WHERE id=%lu AND charid=%lu",
		(unsigned long)instance_id, (unsigned long)char_id);
	auto results = QueryDatabase(query);

	return results.Success();
}


bool Database::RemoveClientsFromInstance(uint16 instance_id)
{
	std::string query = StringFormat("DELETE FROM instance_list_player WHERE id=%lu", (unsigned long)instance_id);
	auto results = QueryDatabase(query);

	return results.Success();
}

bool Database::VerifyInstanceAlive(uint16 instance_id, uint32 char_id)
{
	//we are not saved to this instance so set our instance to 0
	if (!GlobalInstance(instance_id) && !CharacterInInstanceGroup(instance_id, char_id))
		return false;

	if (CheckInstanceExpired(instance_id))
	{
		DeleteInstance(instance_id);
		return false;
	}

	return true;
}

bool Database::VerifyZoneInstance(uint32 zone_id, uint16 instance_id)
{

	std::string query = StringFormat("SELECT id FROM instance_list where id=%u AND zone=%u", instance_id, zone_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	return true;
}

uint16 Database::GetInstanceID(uint32 zone, uint32 character_id, int16 version)
{
	if (!zone)
		return 0;

	std::string query = StringFormat(
		"SELECT "
		"instance_list.id "
		"FROM "
		"instance_list, "
		"instance_list_player "
		"WHERE "
		"instance_list.zone = %u "
		"AND instance_list.version = %u "
		"AND instance_list.id = instance_list_player.id "
		"AND instance_list_player.charid = %u "
		"LIMIT 1; ",
		zone,
		version,
		character_id
		);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

uint16 Database::GetInstanceVersion(uint16 instance_id) {
	if (instance_id == 0)
		return 0;

	std::string query = StringFormat("SELECT version FROM instance_list where id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();
	return atoi(row[0]);
}

uint32 Database::GetTimeRemainingInstance(uint16 instance_id, bool &is_perma)
{
	uint32 start_time = 0;
	uint32 duration = 0;
	uint32 never_expires = 0;

	std::string query = StringFormat("SELECT start_time, duration, never_expires FROM instance_list WHERE id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
	{
		is_perma = false;
		return 0;
	}

	if (results.RowCount() == 0)
	{
		is_perma = false;
		return 0;
	}

	auto row = results.begin();

	start_time = atoi(row[0]);
	duration = atoi(row[1]);
	never_expires = atoi(row[2]);

	if (never_expires == 1)
	{
		is_perma = true;
		return 0;
	}

	is_perma = false;

	timeval tv;
	gettimeofday(&tv, nullptr);
	return ((start_time + duration) - tv.tv_sec);
}

uint32 Database::VersionFromInstanceID(uint16 instance_id)
{

	std::string query = StringFormat("SELECT version FROM instance_list where id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

uint32 Database::ZoneIDFromInstanceID(uint16 instance_id)
{

	std::string query = StringFormat("SELECT zone FROM instance_list where id=%u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return 0;

	if (results.RowCount() == 0)
		return 0;

	auto row = results.begin();

	return atoi(row[0]);
}

void Database::AssignGroupToInstance(uint32 group_id, uint32 instance_id)
{

	uint32 zone_id = ZoneIDFromInstanceID(instance_id);
	uint16 version = VersionFromInstanceID(instance_id);

	std::string query = StringFormat("SELECT `charid` FROM `group_id` WHERE `groupid` = %u", group_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row = results.begin(); row != results.end(); ++row)
	{
		uint32 charid = atoi(row[0]);
		if (GetInstanceID(zone_id, charid, version) == 0)
			AddClientToInstance(instance_id, charid);
	}
}

void Database::AssignRaidToInstance(uint32 raid_id, uint32 instance_id)
{

	uint32 zone_id = ZoneIDFromInstanceID(instance_id);
	uint16 version = VersionFromInstanceID(instance_id);

	std::string query = StringFormat("SELECT `charid` FROM `raid_members` WHERE `raidid` = %u", raid_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row = results.begin(); row != results.end(); ++row)
	{
		uint32 charid = atoi(row[0]);
		if (GetInstanceID(zone_id, charid, version) == 0)
			AddClientToInstance(instance_id, charid);
	}
}

void Database::BuryCorpsesInInstance(uint16 instance_id) {
	QueryDatabase(
		fmt::format(
			"UPDATE character_corpses SET is_buried = 1, instance_id = 0 WHERE instance_id = {}",
			instance_id
		)
	);
}

void Database::DeleteInstance(uint16 instance_id)
{
	std::string query;
	
	query = StringFormat("DELETE FROM instance_list_player WHERE id=%u", instance_id);
	QueryDatabase(query);

	query = StringFormat("DELETE FROM respawn_times WHERE instance_id=%u", instance_id);
	QueryDatabase(query);

	query = StringFormat("DELETE FROM spawn_condition_values WHERE instance_id=%u", instance_id);
	QueryDatabase(query);

	DynamicZoneMembersRepository::DeleteByInstance(*this, instance_id);
	DynamicZonesRepository::DeleteWhere(*this, fmt::format("instance_id = {}", instance_id));

	BuryCorpsesInInstance(instance_id);
}

void Database::FlagInstanceByGroupLeader(uint32 zone, int16 version, uint32 charid, uint32 gid)
{
	uint16 id = GetInstanceID(zone, charid, version);
	if (id != 0)
		return;

	char ln[128];
	memset(ln, 0, 128);
	GetGroupLeadershipInfo(gid, ln);
	uint32 l_charid = GetCharacterID((const char*)ln);
	uint16 l_id = GetInstanceID(zone, l_charid, version);

	if (l_id == 0)
		return;

	AddClientToInstance(l_id, charid);
}

void Database::FlagInstanceByRaidLeader(uint32 zone, int16 version, uint32 charid, uint32 rid)
{
	uint16 id = GetInstanceID(zone, charid, version);
	if (id != 0)
		return;

	uint32 l_charid = GetCharacterID(GetRaidLeaderName(rid));
	uint16 l_id = GetInstanceID(zone, l_charid, version);

	if (l_id == 0)
		return;

	AddClientToInstance(l_id, charid);
}

void Database::GetCharactersInInstance(uint16 instance_id, std::list<uint32> &charid_list) {

	std::string query = StringFormat("SELECT `charid` FROM `instance_list_player` WHERE `id` = %u", instance_id);
	auto results = QueryDatabase(query);

	if (!results.Success())
		return;

	for (auto row = results.begin(); row != results.end(); ++row)
		charid_list.push_back(atoi(row[0]));
}

void Database::PurgeExpiredInstances()
{

	/**
	 * Delay purging by a day so that we can continue using adjacent free instance id's
	 * from the table without risking the chance we immediately re-allocate a zone that freshly expired but
	 * has not been fully de-allocated
	 */
	std::string query =
		SQL(
			SELECT
				id
			FROM
				instance_list
			where
			(start_time + duration) <= (UNIX_TIMESTAMP() - 86400)
			and never_expires = 0
		);

	auto results = QueryDatabase(query);

	if (!results.Success()) {
		return;
	}

	if (results.RowCount() == 0) {
		return;
	}

	std::vector<std::string> instance_ids;
	for (auto row = results.begin(); row != results.end(); ++row) {
		instance_ids.emplace_back(row[0]);
	}

	std::string imploded_instance_ids = implode(",", instance_ids);

	QueryDatabase(fmt::format("DELETE FROM instance_list WHERE id IN ({})", imploded_instance_ids));
	QueryDatabase(fmt::format("DELETE FROM instance_list_player WHERE id IN ({})", imploded_instance_ids));
	QueryDatabase(fmt::format("DELETE FROM respawn_times WHERE instance_id IN ({})", imploded_instance_ids));
	QueryDatabase(fmt::format("DELETE FROM spawn_condition_values WHERE instance_id IN ({})", imploded_instance_ids));
	QueryDatabase(fmt::format("UPDATE character_corpses SET is_buried = 1, instance_id = 0 WHERE instance_id IN ({})", imploded_instance_ids));
	DynamicZoneMembersRepository::DeleteByManyInstances(*this, imploded_instance_ids);
	DynamicZonesRepository::DeleteWhere(*this, fmt::format("instance_id IN ({})", imploded_instance_ids));
}

void Database::SetInstanceDuration(uint16 instance_id, uint32 new_duration)
{
	std::string query = StringFormat("UPDATE `instance_list` SET start_time=UNIX_TIMESTAMP(), "
		"duration=%u WHERE id=%u", new_duration, instance_id);
	auto results = QueryDatabase(query);
}
