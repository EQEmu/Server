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
#include "worlddb.h"
#include "../common/repositories/dynamic_zone_members_repository.h"

void ExpeditionDatabase::PurgeExpiredExpeditions()
{
	std::string query = SQL(
		SELECT
			expeditions.id,
			expeditions.dynamic_zone_id
		FROM expeditions
			LEFT JOIN dynamic_zones ON expeditions.dynamic_zone_id = dynamic_zones.id
			LEFT JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
			LEFT JOIN
				(
					SELECT dynamic_zone_id, COUNT(IF(is_current_member = TRUE, 1, NULL)) member_count
					FROM dynamic_zone_members
					GROUP BY dynamic_zone_id
				) dynamic_zone_members
				ON dynamic_zone_members.dynamic_zone_id = expeditions.dynamic_zone_id
		WHERE
			instance_list.id IS NULL
			OR dynamic_zone_members.member_count IS NULL
			OR dynamic_zone_members.member_count = 0
			OR (instance_list.start_time + instance_list.duration) <= UNIX_TIMESTAMP();
	);

	auto results = database.QueryDatabase(query);
	if (results.Success())
	{
		std::vector<uint32_t> expedition_ids;
		std::vector<uint32_t> dynamic_zone_ids;
		for (auto row = results.begin(); row != results.end(); ++row)
		{
			expedition_ids.emplace_back(static_cast<uint32_t>(strtoul(row[0], nullptr, 10)));
			dynamic_zone_ids.emplace_back(static_cast<uint32_t>(strtoul(row[1], nullptr, 10)));
		}

		if (!expedition_ids.empty())
		{
			ExpeditionDatabase::MoveMembersToSafeReturn(expedition_ids);
			ExpeditionDatabase::DeleteExpeditions(expedition_ids);
			DynamicZoneMembersRepository::RemoveAllMembers(database, dynamic_zone_ids);
		}
	}
}

void ExpeditionDatabase::PurgeExpiredCharacterLockouts()
{
	std::string query = SQL(
		DELETE FROM character_expedition_lockouts
		WHERE expire_time <= NOW();
	);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::DeleteExpeditions(const std::vector<uint32_t>& expedition_ids)
{
	LogExpeditionsDetail("Deleting [{}] expedition(s)", expedition_ids.size());

	std::string expedition_ids_query = fmt::format("{}", fmt::join(expedition_ids, ","));

	if (!expedition_ids_query.empty())
	{
		auto query = fmt::format("DELETE FROM expeditions WHERE id IN ({});", expedition_ids_query);
		database.QueryDatabase(query);

		query = fmt::format("DELETE FROM expedition_lockouts WHERE expedition_id IN ({});", expedition_ids_query);
		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::UpdateLeaderID(uint32_t expedition_id, uint32_t leader_id)
{
	LogExpeditionsDetail("Updating leader [{}] for expedition [{}]", leader_id, expedition_id);

	auto query = fmt::format(SQL(
		UPDATE expeditions SET leader_id = {} WHERE id = {};
	), leader_id, expedition_id);

	database.QueryDatabase(query);
}

void ExpeditionDatabase::MoveMembersToSafeReturn(const std::vector<uint32_t>& expedition_ids)
{
	LogExpeditionsDetail("Moving members from [{}] expedition(s) to safereturn", expedition_ids.size());

	// only offline members still in expired dz zones should be updated here
	std::string query = fmt::format(SQL(
		UPDATE character_data
			INNER JOIN dynamic_zone_members ON character_data.id = dynamic_zone_members.character_id
			INNER JOIN expeditions ON dynamic_zone_members.dynamic_zone_id = expeditions.dynamic_zone_id
			INNER JOIN dynamic_zones ON expeditions.dynamic_zone_id = dynamic_zones.id
			INNER JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
				AND character_data.zone_instance = instance_list.id
				AND character_data.zone_id = instance_list.zone
		SET
			zone_id       = IF(safe_return_zone_id > 0, safe_return_zone_id, zone_id),
			zone_instance = IF(safe_return_zone_id > 0, 0, zone_instance),
			x             = IF(safe_return_zone_id > 0, safe_return_x, x),
			y             = IF(safe_return_zone_id > 0, safe_return_y, y),
			z             = IF(safe_return_zone_id > 0, safe_return_z, z),
			heading       = IF(safe_return_zone_id > 0, safe_return_heading, heading)
		WHERE expeditions.id IN ({});
	), fmt::join(expedition_ids, ","));

	database.QueryDatabase(query);
}
