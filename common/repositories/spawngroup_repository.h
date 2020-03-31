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
#ifndef EQEMU_SPAWNGROUP_REPOSITORY_H
#define EQEMU_SPAWNGROUP_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class SpawnGroupRepository {
public:
	struct SpawnGroup {
		int         id;
		std::string name;
		int8        spawn_limit;
		int         dist;
		float       max_x;
		float       min_x;
		float       max_y;
		float       min_y;
		int         delay;
		int         mindelay;
		int         despawn;
		int         despawn_timer;
		int         wp_spawns;
	};

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"spawn_limit",
			"dist",
			"max_x",
			"min_x",
			"max_y",
			"min_y",
			"delay",
			"mindelay",
			"despawn",
			"despawn_timer",
			"wp_spawns",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("spawngroup");
	}

	static std::string BaseSelect()
	{
		return std::string(
			fmt::format(
				"SELECT {} FROM {}",
				ColumnsRaw(),
				TableName()
			)
		);
	}

	static SpawnGroup NewEntity()
	{
		SpawnGroup entry;

		entry.id            = 0;
		entry.name          = "";
		entry.spawn_limit   = 0;
		entry.dist          = 0;
		entry.max_x         = 0;
		entry.min_x         = 0;
		entry.max_y         = 0;
		entry.min_y         = 0;
		entry.delay         = 0;
		entry.mindelay      = 0;
		entry.despawn       = 0;
		entry.despawn_timer = 0;
		entry.wp_spawns     = 0;

		return entry;
	}

	static std::vector<SpawnGroup> GetZoneSpawnGroups(
		const std::string &zone_short_name,
		int zone_version
	)
	{
		std::vector<SpawnGroup> spawn_groups;

		auto results = content_db.QueryDatabase(
			fmt::format(
				SQL (
					{} INNER JOIN spawn2 ON spawn2.spawngroupID = spawngroup.id
					WHERE spawn2.zone = '{}' and spawn2.version = {}
				),
				BaseSelect(),
				zone_short_name,
				zone_version
			)
		);

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnGroup entry{};

			entry.id            = atoi(row[0]);
			entry.name          = row[1];
			entry.spawn_limit   = atoi(row[2]);
			entry.dist          = atof(row[3]);
			entry.max_x         = atof(row[4]);
			entry.min_x         = atof(row[5]);
			entry.max_y         = atof(row[6]);
			entry.min_y         = atof(row[7]);
			entry.delay         = atoi(row[8]);
			entry.mindelay      = atoi(row[9]);
			entry.despawn       = atoi(row[10]);
			entry.despawn_timer = atoi(row[11]);
			entry.wp_spawns     = atoi(row[12]);

			spawn_groups.push_back(entry);
		}

		return spawn_groups;
	}

	static SpawnGroup GetGrid(
		const std::vector<SpawnGroup> &spawn_groups,
		int spawn_group_id
	)
	{
		for (auto &row : spawn_groups) {
			if (row.id == spawn_group_id) {
				return row;
			}
		}

		return NewEntity();
	}

};


#endif //EQEMU_SPAWNGROUP_REPOSITORY_H
