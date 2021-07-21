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

#ifndef EQEMU_DYNAMIC_ZONES_REPOSITORY_H
#define EQEMU_DYNAMIC_ZONES_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"
#include "base/base_dynamic_zones_repository.h"

class DynamicZonesRepository: public BaseDynamicZonesRepository {
public:

    /**
     * This file was auto generated and can be modified and extended upon
     *
     * Base repository methods are automatically
     * generated in the "base" version of this repository. The base repository
     * is immutable and to be left untouched, while methods in this class
     * are used as extension methods for more specific persistence-layer
     * accessors or mutators.
     *
     * Base Methods (Subject to be expanded upon in time)
     *
     * Note: Not all tables are designed appropriately to fit functionality with all base methods
     *
     * InsertOne
     * UpdateOne
     * DeleteOne
     * FindOne
     * GetWhere(std::string where_filter)
     * DeleteWhere(std::string where_filter)
     * InsertMany
     * All
     *
     * Example custom methods in a repository
     *
     * DynamicZonesRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * DynamicZonesRepository::GetWhereNeverExpires()
     * DynamicZonesRepository::GetWhereXAndY()
     * DynamicZonesRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	struct DynamicZoneInstance
	{
		uint32_t id;
		std::string uuid;
		std::string name;
		int      leader_id;
		int      min_players;
		int      max_players;
		int      instance_id;
		int      type;
		int      compass_zone_id;
		float    compass_x;
		float    compass_y;
		float    compass_z;
		int      safe_return_zone_id;
		float    safe_return_x;
		float    safe_return_y;
		float    safe_return_z;
		float    safe_return_heading;
		float    zone_in_x;
		float    zone_in_y;
		float    zone_in_z;
		float    zone_in_heading;
		int      has_zone_in;
		int      zone;
		int      version;
		int      is_global;
		uint32_t start_time;
		int      duration;
		int      never_expires;
	};

	static std::string SelectDynamicZoneJoinInstance()
	{
		return std::string(SQL(
			SELECT
				dynamic_zones.id,
				dynamic_zones.uuid,
				dynamic_zones.name,
				dynamic_zones.leader_id,
				dynamic_zones.min_players,
				dynamic_zones.max_players,
				dynamic_zones.instance_id,
				dynamic_zones.type,
				dynamic_zones.compass_zone_id,
				dynamic_zones.compass_x,
				dynamic_zones.compass_y,
				dynamic_zones.compass_z,
				dynamic_zones.safe_return_zone_id,
				dynamic_zones.safe_return_x,
				dynamic_zones.safe_return_y,
				dynamic_zones.safe_return_z,
				dynamic_zones.safe_return_heading,
				dynamic_zones.zone_in_x,
				dynamic_zones.zone_in_y,
				dynamic_zones.zone_in_z,
				dynamic_zones.zone_in_heading,
				dynamic_zones.has_zone_in,
				instance_list.zone,
				instance_list.version,
				instance_list.is_global,
				instance_list.start_time,
				instance_list.duration,
				instance_list.never_expires
			FROM dynamic_zones
				INNER JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
		));
	}

	static DynamicZoneInstance FillWithInstanceFromRow(MySQLRequestRow& row)
	{
		DynamicZoneInstance entry{};

		int col = 0;
		entry.id                  = strtoul(row[col++], nullptr, 10);
		entry.uuid                = row[col++];
		entry.name                = row[col++];
		entry.leader_id           = strtol(row[col++], nullptr, 10);
		entry.min_players         = strtol(row[col++], nullptr, 10);
		entry.max_players         = strtol(row[col++], nullptr, 10);
		entry.instance_id         = strtol(row[col++], nullptr, 10);
		entry.type                = strtol(row[col++], nullptr, 10);
		entry.compass_zone_id     = strtol(row[col++], nullptr, 10);
		entry.compass_x           = strtof(row[col++], nullptr);
		entry.compass_y           = strtof(row[col++], nullptr);
		entry.compass_z           = strtof(row[col++], nullptr);
		entry.safe_return_zone_id = strtol(row[col++], nullptr, 10);
		entry.safe_return_x       = strtof(row[col++], nullptr);
		entry.safe_return_y       = strtof(row[col++], nullptr);
		entry.safe_return_z       = strtof(row[col++], nullptr);
		entry.safe_return_heading = strtof(row[col++], nullptr);
		entry.zone_in_x           = strtof(row[col++], nullptr);
		entry.zone_in_y           = strtof(row[col++], nullptr);
		entry.zone_in_z           = strtof(row[col++], nullptr);
		entry.zone_in_heading     = strtof(row[col++], nullptr);
		entry.has_zone_in         = strtol(row[col++], nullptr, 10) != 0;
		// from instance_list
		entry.zone                = strtol(row[col++], nullptr, 10);
		entry.version             = strtol(row[col++], nullptr, 10);
		entry.is_global           = strtol(row[col++], nullptr, 10);
		entry.start_time          = strtoul(row[col++], nullptr, 10);
		entry.duration            = strtol(row[col++], nullptr, 10);
		entry.never_expires       = strtol(row[col++], nullptr, 10);

		return entry;
	}

	static std::vector<DynamicZoneInstance> AllWithInstanceNotExpired(Database& db)
	{
		std::vector<DynamicZoneInstance> all_entries;

		auto results = db.QueryDatabase(fmt::format(SQL(
			{} WHERE
				(instance_list.start_time + instance_list.duration) > UNIX_TIMESTAMP()
				AND instance_list.never_expires = 0
		), SelectDynamicZoneJoinInstance()));

		if (results.Success())
		{
			all_entries.reserve(results.RowCount());

			for (auto row = results.begin(); row != results.end(); ++row)
			{
				DynamicZoneInstance entry = FillWithInstanceFromRow(row);
				all_entries.emplace_back(std::move(entry));
			}
		}

		return all_entries;
	}

	static void UpdateCompass(Database& db, uint32_t dz_id, int zone_id, float x, float y, float z)
	{
		if (dz_id != 0)
		{
			std::string query = fmt::format(SQL(
				UPDATE {} SET
					compass_zone_id = {},
					compass_x = {},
					compass_y = {},
					compass_z = {}
				WHERE {} = {};
			), TableName(), zone_id, x, y, z, PrimaryKey(), dz_id);

			db.QueryDatabase(query);
		}
	}

	static void UpdateSafeReturn(Database& db, uint32_t dz_id, int zone_id, float x, float y, float z, float heading)
	{
		if (dz_id != 0)
		{
			std::string query = fmt::format(SQL(
				UPDATE {} SET
					safe_return_zone_id = {},
					safe_return_x = {},
					safe_return_y = {},
					safe_return_z = {},
					safe_return_heading = {}
				WHERE {} = {};
			), TableName(), zone_id, x, y, z, heading, PrimaryKey(), dz_id);

			db.QueryDatabase(query);
		}
	}

	static void UpdateZoneIn(Database& db, uint32_t dz_id, uint32_t zone_id, float x, float y, float z, float heading, bool has_zone_in)
	{
		if (dz_id != 0)
		{
			std::string query = fmt::format(SQL(
				UPDATE {} SET
					zone_in_x = {},
					zone_in_y = {},
					zone_in_z = {},
					zone_in_heading = {},
					has_zone_in = {}
				WHERE {} = {};
			), TableName(), x, y, z, heading, has_zone_in, PrimaryKey(), dz_id);

			db.QueryDatabase(query);
		}
	}

	static void UpdateLeaderID(Database& db, uint32_t dz_id, uint32_t leader_id)
	{
		if (dz_id != 0)
		{
			std::string query = fmt::format(SQL(
				UPDATE {} SET leader_id = {} WHERE {} = {};
			), TableName(), leader_id, PrimaryKey(), dz_id);

			db.QueryDatabase(query);
		}
	}

	struct DynamicZoneInstancePlayerCount
	{
		uint32_t id;
		int      type;
		int      instance;
		int      zone;
		int      version;
		uint32_t start_time;
		int      duration;
		int      member_count;
	};

	static std::string SelectDynamicZoneInstancePlayerCount()
	{
		return std::string(SQL(
			SELECT
				dynamic_zones.id,
				dynamic_zones.type,
				instance_list.id,
				instance_list.zone,
				instance_list.version,
				instance_list.start_time,
				instance_list.duration,
				COUNT(dynamic_zone_members.character_id) member_count
			FROM dynamic_zones
				INNER JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
				LEFT JOIN dynamic_zone_members ON dynamic_zones.id = dynamic_zone_members.dynamic_zone_id
			GROUP BY instance_list.id
			ORDER BY dynamic_zones.id;
		));
	};

	static std::vector<DynamicZoneInstancePlayerCount> AllDzInstancePlayerCounts(Database& db)
	{
		std::vector<DynamicZoneInstancePlayerCount> all_entries;

		auto results = db.QueryDatabase(SelectDynamicZoneInstancePlayerCount());
		if (results.Success())
		{
			all_entries.reserve(results.RowCount());

			for (auto row = results.begin(); row != results.end(); ++row)
			{
				DynamicZoneInstancePlayerCount entry{};

				int col = 0;
				entry.id           = strtoul(row[col++], nullptr, 10);
				entry.type         = strtol(row[col++], nullptr, 10);
				entry.instance     = strtol(row[col++], nullptr, 10);
				entry.zone         = strtol(row[col++], nullptr, 10);
				entry.version      = strtol(row[col++], nullptr, 10);
				entry.start_time   = strtoul(row[col++], nullptr, 10);
				entry.duration     = strtol(row[col++], nullptr, 10);
				entry.member_count = strtol(row[col++], nullptr, 10);

				all_entries.emplace_back(std::move(entry));
			}
		}
		return all_entries;
	}

	static std::vector<uint32_t> GetStaleIDs(Database& db)
	{
		std::vector<uint32_t> all_entries;

		// dzs with no members, missing instance, or expired instance
		auto results = db.QueryDatabase(SQL(
			SELECT
				dynamic_zones.id
			FROM dynamic_zones
				LEFT JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
				LEFT JOIN
					(
						SELECT dynamic_zone_id, COUNT(*) member_count
						FROM dynamic_zone_members
						GROUP BY dynamic_zone_id
					) dynamic_zone_members
					ON dynamic_zone_members.dynamic_zone_id = dynamic_zones.id
			WHERE
				instance_list.id IS NULL
				OR dynamic_zone_members.member_count IS NULL
				OR dynamic_zone_members.member_count = 0
				OR ((instance_list.start_time + instance_list.duration) <= UNIX_TIMESTAMP()
					AND instance_list.never_expires = 0);
		));

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row)
		{
			all_entries.push_back(atoi(row[0]));
		}

		return all_entries;
	}
};

#endif //EQEMU_DYNAMIC_ZONES_REPOSITORY_H
