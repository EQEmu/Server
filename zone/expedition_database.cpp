/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "expedition_database.h"
#include "expedition.h"
#include "zonedb.h"
#include "../common/database.h"
#include "../common/expedition_lockout_timer.h"
#include "../common/string_util.h"
#include <fmt/core.h>

uint32_t ExpeditionDatabase::InsertExpedition(
	const std::string& uuid, uint32_t dz_id, const std::string& expedition_name,
	uint32_t leader_id, uint32_t min_players, uint32_t max_players)
{
	LogExpeditionsDetail(
		"Inserting new expedition [{}] leader [{}] uuid [{}]", expedition_name, leader_id, uuid
	);

	std::string query = fmt::format(SQL(
		INSERT INTO expeditions
			(uuid, dynamic_zone_id, expedition_name, leader_id, min_players, max_players)
		VALUES
			('{}', {}, '{}', {}, {}, {});
	), uuid, dz_id, EscapeString(expedition_name), leader_id, min_players, max_players);

	auto results = database.QueryDatabase(query);
	if (!results.Success())
	{
		LogExpeditions("Failed to obtain an expedition id for [{}]", expedition_name);
		return 0;
	}

	return results.LastInsertedID();
}

std::vector<ExpeditionLockoutTimer> ExpeditionDatabase::LoadCharacterLockouts(uint32_t character_id)
{
	LogExpeditionsDetail("Loading character [{}] lockouts", character_id);

	std::vector<ExpeditionLockoutTimer> lockouts;

	auto query = fmt::format(SQL(
		SELECT
			from_expedition_uuid,
			expedition_name,
			event_name,
			UNIX_TIMESTAMP(expire_time),
			duration
		FROM character_expedition_lockouts
		WHERE character_id = {} AND expire_time > NOW();
	), character_id);

	auto results = database.QueryDatabase(query);
	if (results.Success())
	{
		for (auto row = results.begin(); row != results.end(); ++row)
		{
			lockouts.emplace_back(
				row[0],                                             // expedition_uuid
				row[1],                                             // expedition_name
				row[2],                                             // event_name
				strtoull(row[3], nullptr, 10),                      // expire_time
				static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) // duration
			);
		}
	}

	return lockouts;
}

std::vector<ExpeditionLockoutTimer> ExpeditionDatabase::LoadCharacterLockouts(
	uint32_t character_id, const std::string& expedition_name)
{
	LogExpeditionsDetail("Loading character [{}] lockouts for [{}]", character_id, expedition_name);

	std::vector<ExpeditionLockoutTimer> lockouts;

	auto query = fmt::format(SQL(
		SELECT
			from_expedition_uuid,
			event_name,
			UNIX_TIMESTAMP(expire_time),
			duration
		FROM character_expedition_lockouts
		WHERE
			character_id = {}
			AND expire_time > NOW()
			AND expedition_name = '{}';
	), character_id, EscapeString(expedition_name));

	auto results = database.QueryDatabase(query);
	if (results.Success())
	{
		for (auto row = results.begin(); row != results.end(); ++row)
		{
			lockouts.emplace_back(
				row[0],                                             // expedition_uuid
				expedition_name,
				row[1],                                             // event_name
				strtoull(row[2], nullptr, 10),                      // expire_time
				static_cast<uint32_t>(strtoul(row[3], nullptr, 10)) // duration
			);
		}
	}

	return lockouts;
}

void ExpeditionDatabase::DeleteAllCharacterLockouts(uint32_t character_id)
{
	LogExpeditionsDetail("Deleting all character [{}] lockouts", character_id);

	if (character_id != 0)
	{
		std::string query = fmt::format(SQL(
			DELETE FROM character_expedition_lockouts
			WHERE character_id = {};
		), character_id);

		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::DeleteAllCharacterLockouts(
	uint32_t character_id, const std::string& expedition_name)
{
	LogExpeditionsDetail("Deleting all character [{}] lockouts for [{}]", character_id, expedition_name);

	if (character_id != 0 && !expedition_name.empty())
	{
		std::string query = fmt::format(SQL(
			DELETE FROM character_expedition_lockouts
			WHERE character_id = {} AND expedition_name = '{}';
		), character_id, EscapeString(expedition_name));

		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::DeleteCharacterLockout(
	uint32_t character_id, const std::string& expedition_name, const std::string& event_name)
{
	LogExpeditionsDetail(
		"Deleting character [{}] lockout: [{}]:[{}]", character_id, expedition_name, event_name
	);

	auto query = fmt::format(SQL(
		DELETE FROM character_expedition_lockouts
		WHERE
			character_id = {}
			AND expedition_name = '{}'
			AND event_name = '{}';
	), character_id, EscapeString(expedition_name), EscapeString(event_name));

	database.QueryDatabase(query);
}

void ExpeditionDatabase::DeleteMembersLockout(
	const std::vector<DynamicZoneMember>& members,
	const std::string& expedition_name, const std::string& event_name)
{
	LogExpeditionsDetail("Deleting members lockout: [{}]:[{}]", expedition_name, event_name);

	std::string query_character_ids;
	for (const auto& member : members)
	{
		fmt::format_to(std::back_inserter(query_character_ids), "{},", member.id);
	}

	if (!query_character_ids.empty())
	{
		query_character_ids.pop_back(); // trailing comma

		auto query = fmt::format(SQL(
			DELETE FROM character_expedition_lockouts
			WHERE character_id
				IN ({})
				AND expedition_name = '{}'
				AND event_name = '{}';
		), query_character_ids, EscapeString(expedition_name), EscapeString(event_name));

		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::DeleteLockout(uint32_t expedition_id, const std::string& event_name)
{
	LogExpeditionsDetail("Deleting expedition [{}] lockout event [{}]", expedition_id, event_name);

	auto query = fmt::format(SQL(
		DELETE FROM expedition_lockouts
		WHERE expedition_id = {} AND event_name = '{}';
	), expedition_id, EscapeString(event_name));

	database.QueryDatabase(query);
}

uint32_t ExpeditionDatabase::GetExpeditionIDFromCharacterID(uint32_t character_id)
{
	LogExpeditionsDetail("Getting expedition id for character [{}]", character_id);

	uint32_t expedition_id = 0;
	auto query = fmt::format(SQL(
		SELECT expedition_id FROM expedition_members
		WHERE character_id = {} AND is_current_member = TRUE;
	), character_id);

	auto results = database.QueryDatabase(query);
	if (results.Success() && results.RowCount() > 0)
	{
		auto row = results.begin();
		expedition_id = strtoul(row[0], nullptr, 10);
	}
	return expedition_id;
}

uint32_t ExpeditionDatabase::GetMemberCount(uint32_t expedition_id)
{
	LogExpeditionsDetail("Getting expedition [{}] member count from db", expedition_id);

	uint32_t member_count = 0;
	if (expedition_id != 0)
	{
		auto query = fmt::format(SQL(
			SELECT COUNT(*)
			FROM expedition_members
			WHERE expedition_id = {} AND is_current_member = TRUE;
		), expedition_id);

		auto results = database.QueryDatabase(query);
		if (results.Success() && results.RowCount() > 0)
		{
			auto row = results.begin();
			member_count = strtoul(row[0], nullptr, 10);
		}
	}
	return member_count;
}

bool ExpeditionDatabase::HasMember(uint32_t expedition_id, uint32_t character_id)
{
	LogExpeditionsDetail("Checking db expedition [{}] for character [{}]", expedition_id, character_id);

	if (expedition_id == 0 || character_id == 0)
	{
		return false;
	}

	auto query = fmt::format(SQL(
		SELECT id
		FROM expedition_members
		WHERE expedition_id = {} AND character_id = {} AND is_current_member = TRUE;
	), expedition_id, character_id);

	auto results = database.QueryDatabase(query);
	return (results.Success() && results.RowCount() > 0);
}

void ExpeditionDatabase::InsertCharacterLockouts(uint32_t character_id,
	const std::vector<ExpeditionLockoutTimer>& lockouts)
{
	LogExpeditionsDetail("Inserting [{}] lockouts for character [{}]", lockouts.size(), character_id);

	std::string insert_values;
	for (const auto& lockout : lockouts)
	{
		fmt::format_to(std::back_inserter(insert_values),
			"({}, FROM_UNIXTIME({}), {}, '{}', '{}', '{}'),",
			character_id,
			lockout.GetExpireTime(),
			lockout.GetDuration(),
			lockout.GetExpeditionUUID(),
			EscapeString(lockout.GetExpeditionName()),
			EscapeString(lockout.GetEventName())
		);
	}

	if (!insert_values.empty())
	{
		insert_values.pop_back(); // trailing comma

		auto query = fmt::format(SQL(
			INSERT INTO character_expedition_lockouts
				(character_id, expire_time, duration, from_expedition_uuid, expedition_name, event_name)
			VALUES {}
			ON DUPLICATE KEY UPDATE
				from_expedition_uuid = VALUES(from_expedition_uuid),
				expire_time = VALUES(expire_time),
				duration = VALUES(duration);
		), insert_values);

		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::InsertMembersLockout(
	const std::vector<DynamicZoneMember>& members, const ExpeditionLockoutTimer& lockout)
{
	LogExpeditionsDetail(
		"Inserting members lockout [{}]:[{}] with expire time [{}]",
		lockout.GetExpeditionName(), lockout.GetEventName(), lockout.GetExpireTime()
	);

	std::string insert_values;
	for (const auto& member : members)
	{
		fmt::format_to(std::back_inserter(insert_values),
			"({}, FROM_UNIXTIME({}), {}, '{}', '{}', '{}'),",
			member.id,
			lockout.GetExpireTime(),
			lockout.GetDuration(),
			lockout.GetExpeditionUUID(),
			EscapeString(lockout.GetExpeditionName()),
			EscapeString(lockout.GetEventName())
		);
	}

	if (!insert_values.empty())
	{
		insert_values.pop_back(); // trailing comma

		auto query = fmt::format(SQL(
			INSERT INTO character_expedition_lockouts
				(character_id, expire_time, duration, from_expedition_uuid, expedition_name, event_name)
			VALUES {}
			ON DUPLICATE KEY UPDATE
				from_expedition_uuid = VALUES(from_expedition_uuid),
				expire_time = VALUES(expire_time),
				duration = VALUES(duration);
		), insert_values);

		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::InsertLockout(
	uint32_t expedition_id, const ExpeditionLockoutTimer& lockout)
{
	LogExpeditionsDetail(
		"Inserting expedition [{}] lockout: [{}]:[{}] expire time: [{}]",
		expedition_id, lockout.GetExpeditionName(), lockout.GetEventName(), lockout.GetExpireTime()
	);

	auto query = fmt::format(SQL(
		INSERT INTO expedition_lockouts
			(expedition_id, from_expedition_uuid, event_name, expire_time, duration)
		VALUES
			({}, '{}', '{}', FROM_UNIXTIME({}), {})
		ON DUPLICATE KEY UPDATE
			from_expedition_uuid = VALUES(from_expedition_uuid),
			expire_time = VALUES(expire_time),
			duration = VALUES(duration);
	),
		expedition_id,
		lockout.GetExpeditionUUID(),
		EscapeString(lockout.GetEventName()),
		lockout.GetExpireTime(),
		lockout.GetDuration()
	);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::InsertLockouts(
	uint32_t expedition_id, const std::unordered_map<std::string, ExpeditionLockoutTimer>& lockouts)
{
	LogExpeditionsDetail("Inserting expedition [{}] lockouts", expedition_id);

	std::string insert_values;
	for (const auto& lockout : lockouts)
	{
		fmt::format_to(std::back_inserter(insert_values),
			"({}, '{}', '{}', FROM_UNIXTIME({}), {}),",
			expedition_id,
			lockout.second.GetExpeditionUUID(),
			EscapeString(lockout.second.GetEventName()),
			lockout.second.GetExpireTime(),
			lockout.second.GetDuration()
		);
	}

	if (!insert_values.empty())
	{
		insert_values.pop_back(); // trailing comma

		auto query = fmt::format(SQL(
			INSERT INTO expedition_lockouts
				(expedition_id, from_expedition_uuid, event_name, expire_time, duration)
			VALUES {}
			ON DUPLICATE KEY UPDATE
				from_expedition_uuid = VALUES(from_expedition_uuid),
				expire_time = VALUES(expire_time),
				duration = VALUES(duration);
		), insert_values);

		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::InsertMember(uint32_t expedition_id, uint32_t character_id)
{
	LogExpeditionsDetail("Inserting character [{}] into expedition [{}]", character_id, expedition_id);

	auto query = fmt::format(SQL(
		INSERT INTO expedition_members
			(expedition_id, character_id)
		VALUES
			({}, {})
		ON DUPLICATE KEY UPDATE is_current_member = TRUE;
	), expedition_id, character_id);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::InsertMembers(
	uint32_t expedition_id, const std::vector<DynamicZoneMember>& members)
{
	LogExpeditionsDetail("Inserting characters into expedition [{}]", expedition_id);

	std::string insert_values;
	for (const auto& member : members)
	{
		fmt::format_to(std::back_inserter(insert_values),
			"({}, {}),",
			expedition_id, member.id
		);
	}

	if (!insert_values.empty())
	{
		insert_values.pop_back(); // trailing comma

		auto query = fmt::format(SQL(
			INSERT INTO expedition_members
				(expedition_id, character_id)
			VALUES {}
			ON DUPLICATE KEY UPDATE is_current_member = TRUE;
		), insert_values);

		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::UpdateLockState(uint32_t expedition_id, bool is_locked)
{
	LogExpeditionsDetail("Updating lock state [{}] for expedition [{}]", is_locked, expedition_id);

	auto query = fmt::format(SQL(
		UPDATE expeditions SET is_locked = {} WHERE id = {};
	), is_locked, expedition_id);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::DeleteMember(uint32_t expedition_id, uint32_t character_id)
{
	LogExpeditionsDetail("Removing member [{}] from expedition [{}]", character_id, expedition_id);

	auto query = fmt::format(SQL(
		UPDATE expedition_members SET is_current_member = FALSE
		WHERE expedition_id = {} AND character_id = {};
	), expedition_id, character_id);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::DeleteAllMembers(uint32_t expedition_id)
{
	LogExpeditionsDetail("Removing all members of expedition [{}]", expedition_id);

	auto query = fmt::format(SQL(
		UPDATE expedition_members SET is_current_member = FALSE WHERE expedition_id = {};
	), expedition_id);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::UpdateReplayLockoutOnJoin(uint32_t expedition_id, bool add_on_join)
{
	LogExpeditionsDetail("Updating replay lockout on join [{}] for expedition [{}]", add_on_join, expedition_id);

	auto query = fmt::format(SQL(
		UPDATE expeditions SET add_replay_on_join = {} WHERE id = {};
	), add_on_join, expedition_id);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::AddLockoutDuration(const std::vector<DynamicZoneMember>& members,
	const ExpeditionLockoutTimer& lockout, int seconds)
{
	LogExpeditionsDetail(
		"Adding duration [{}] seconds to members lockouts [{}]:[{}]",
		seconds, lockout.GetExpeditionName(), lockout.GetEventName());

	std::string insert_values;
	for (const auto& member : members)
	{
		fmt::format_to(std::back_inserter(insert_values),
			"({}, FROM_UNIXTIME({}), {}, '{}', '{}', '{}'),",
			member.id,
			lockout.GetExpireTime(),
			lockout.GetDuration(),
			lockout.GetExpeditionUUID(),
			EscapeString(lockout.GetExpeditionName()),
			EscapeString(lockout.GetEventName())
		);
	}

	if (!insert_values.empty())
	{
		insert_values.pop_back(); // trailing comma

		auto query = fmt::format(SQL(
			INSERT INTO character_expedition_lockouts
				(character_id, expire_time, duration, from_expedition_uuid, expedition_name, event_name)
			VALUES {}
			ON DUPLICATE KEY UPDATE
				from_expedition_uuid = VALUES(from_expedition_uuid),
				expire_time = DATE_ADD(expire_time, INTERVAL {} SECOND),
				duration = GREATEST(0, CAST(duration AS SIGNED) + {});
		), insert_values, seconds, seconds);

		database.QueryDatabase(query);
	}
}
