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

void ExpeditionDatabase::PurgeExpiredExpeditions()
{
	std::string query = SQL(
		SELECT
			expeditions.id
		FROM expeditions
			LEFT JOIN dynamic_zones ON expeditions.dynamic_zone_id = dynamic_zones.id
			LEFT JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
			LEFT JOIN
				(
					SELECT expedition_id, COUNT(IF(is_current_member = TRUE, 1, NULL)) member_count
					FROM expedition_members
					GROUP BY expedition_id
				) expedition_members
				ON expedition_members.expedition_id = expeditions.id
		WHERE
			instance_list.id IS NULL
			OR expedition_members.member_count IS NULL
			OR expedition_members.member_count = 0
			OR (instance_list.start_time + instance_list.duration) <= UNIX_TIMESTAMP();
	);

	auto results = database.QueryDatabase(query);
	if (results.Success())
	{
		std::vector<uint32_t> expedition_ids;
		for (auto row = results.begin(); row != results.end(); ++row)
		{
			expedition_ids.emplace_back(static_cast<uint32_t>(strtoul(row[0], nullptr, 10)));
		}

		if (!expedition_ids.empty())
		{
			ExpeditionDatabase::MoveMembersToSafeReturn(expedition_ids);
			ExpeditionDatabase::DeleteExpeditions(expedition_ids);
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

std::vector<Expedition> ExpeditionDatabase::LoadExpeditions(uint32_t select_expedition_id)
{
	std::vector<Expedition> expeditions;

	std::string query = SQL(
		SELECT
			expeditions.id,
			expeditions.dynamic_zone_id,
			instance_list.id,
			instance_list.zone,
			instance_list.start_time,
			instance_list.duration,
			expeditions.leader_id,
			expedition_members.character_id
		FROM expeditions
			INNER JOIN dynamic_zones ON expeditions.dynamic_zone_id = dynamic_zones.id
			INNER JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
			INNER JOIN expedition_members ON expedition_members.expedition_id = expeditions.id
				AND expedition_members.is_current_member = TRUE
	);

	if (select_expedition_id != 0)
	{
		query.append(fmt::format(" WHERE expeditions.id = {};", select_expedition_id));
	}
	else
	{
		query.append(" ORDER BY expeditions.id;");
	}

	auto results = database.QueryDatabase(query);
	if (results.Success())
	{
		uint32_t last_expedition_id = 0;

		for (auto row = results.begin(); row != results.end(); ++row)
		{
			uint32_t expedition_id = strtoul(row[0], nullptr, 10);

			if (last_expedition_id != expedition_id)
			{
				expeditions.emplace_back(
					static_cast<uint32_t>(strtoul(row[0], nullptr, 10)), // expedition_id
					static_cast<uint32_t>(strtoul(row[1], nullptr, 10)), // dz_id
					static_cast<uint32_t>(strtoul(row[2], nullptr, 10)), // dz_instance_id
					static_cast<uint32_t>(strtoul(row[3], nullptr, 10)), // dz_zone_id
					static_cast<uint32_t>(strtoul(row[4], nullptr, 10)), // start_time
					static_cast<uint32_t>(strtoul(row[5], nullptr, 10)), // duration
					static_cast<uint32_t>(strtoul(row[6], nullptr, 10))  // leader_id
				);
			}

			last_expedition_id = expedition_id;

			uint32_t member_id = static_cast<uint32_t>(strtoul(row[7], nullptr, 10));
			expeditions.back().AddMember(member_id);
		}
	}

	return expeditions;
}

Expedition ExpeditionDatabase::LoadExpedition(uint32_t expedition_id)
{
	LogExpeditions("Loading expedition [{}] for world cache", expedition_id);

	Expedition expedition;

	auto expeditions = LoadExpeditions(expedition_id);
	if (!expeditions.empty())
	{
		expedition = expeditions.front();
	}

	return expedition;
}

void ExpeditionDatabase::DeleteExpeditions(const std::vector<uint32_t>& expedition_ids)
{
	LogExpeditionsDetail("Deleting [{}] expedition(s)", expedition_ids.size());

	std::string expedition_ids_query = fmt::format("{}", fmt::join(expedition_ids, ","));

	if (!expedition_ids_query.empty())
	{
		auto query = fmt::format("DELETE FROM expeditions WHERE id IN ({});", expedition_ids_query);
		database.QueryDatabase(query);

		query = fmt::format("DELETE FROM expedition_members WHERE expedition_id IN ({});", expedition_ids_query);
		database.QueryDatabase(query);

		query = fmt::format("DELETE FROM expedition_lockouts WHERE expedition_id IN ({});", expedition_ids_query);
		database.QueryDatabase(query);
	}
}

void ExpeditionDatabase::UpdateDzDuration(uint16_t instance_id, uint32_t new_duration)
{
	std::string query = fmt::format(
		"UPDATE instance_list SET duration = {} WHERE id = {};",
		new_duration, instance_id
	);

	database.QueryDatabase(query);
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
			INNER JOIN expedition_members ON character_data.id = expedition_members.character_id
			INNER JOIN expeditions ON expedition_members.expedition_id = expeditions.id
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
