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
#include "worlddb.h"
#include "../common/repositories/expeditions_repository.h"
#include "../common/repositories/expedition_lockouts_repository.h"
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
					SELECT dynamic_zone_id, COUNT(*) member_count
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
			auto joined_expedition_ids = fmt::join(expedition_ids, ",");
			ExpeditionsRepository::DeleteWhere(database, fmt::format("id IN ({})", joined_expedition_ids));
			ExpeditionLockoutsRepository::DeleteWhere(database, fmt::format("expedition_id IN ({})", joined_expedition_ids));
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
