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
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
 */

#ifndef EQEMU_BASE_START_ZONES_REPOSITORY_H
#define EQEMU_BASE_START_ZONES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseStartZonesRepository {
public:
	struct StartZones {
		float       x;
		float       y;
		float       z;
		float       heading;
		int         zone_id;
		int         bind_id;
		int         player_choice;
		int         player_class;
		int         player_deity;
		int         player_race;
		int         start_zone;
		float       bind_x;
		float       bind_y;
		float       bind_z;
		int         select_rank;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
	};

	static std::string PrimaryKey()
	{
		return std::string("player_choice");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"x",
			"y",
			"z",
			"heading",
			"zone_id",
			"bind_id",
			"player_choice",
			"player_class",
			"player_deity",
			"player_race",
			"start_zone",
			"bind_x",
			"bind_y",
			"bind_z",
			"select_rank",
			"min_expansion",
			"max_expansion",
			"content_flags",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
	}

	static std::string TableName()
	{
		return std::string("start_zones");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			ColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			InsertColumnsRaw()
		);
	}

	static StartZones NewEntity()
	{
		StartZones entry{};

		entry.x             = 0;
		entry.y             = 0;
		entry.z             = 0;
		entry.heading       = 0;
		entry.zone_id       = 0;
		entry.bind_id       = 0;
		entry.player_choice = 0;
		entry.player_class  = 0;
		entry.player_deity  = 0;
		entry.player_race   = 0;
		entry.start_zone    = 0;
		entry.bind_x        = 0;
		entry.bind_y        = 0;
		entry.bind_z        = 0;
		entry.select_rank   = 50;
		entry.min_expansion = 0;
		entry.max_expansion = 0;
		entry.content_flags = "";

		return entry;
	}

	static StartZones GetStartZonesEntry(
		const std::vector<StartZones> &start_zoness,
		int start_zones_id
	)
	{
		for (auto &start_zones : start_zoness) {
			if (start_zones.player_choice == start_zones_id) {
				return start_zones;
			}
		}

		return NewEntity();
	}

	static StartZones FindOne(
		int start_zones_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				start_zones_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			StartZones entry{};

			entry.x             = static_cast<float>(atof(row[0]));
			entry.y             = static_cast<float>(atof(row[1]));
			entry.z             = static_cast<float>(atof(row[2]));
			entry.heading       = static_cast<float>(atof(row[3]));
			entry.zone_id       = atoi(row[4]);
			entry.bind_id       = atoi(row[5]);
			entry.player_choice = atoi(row[6]);
			entry.player_class  = atoi(row[7]);
			entry.player_deity  = atoi(row[8]);
			entry.player_race   = atoi(row[9]);
			entry.start_zone    = atoi(row[10]);
			entry.bind_x        = static_cast<float>(atof(row[11]));
			entry.bind_y        = static_cast<float>(atof(row[12]));
			entry.bind_z        = static_cast<float>(atof(row[13]));
			entry.select_rank   = atoi(row[14]);
			entry.min_expansion = atoi(row[15]);
			entry.max_expansion = atoi(row[16]);
			entry.content_flags = row[17] ? row[17] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int start_zones_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				start_zones_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		StartZones start_zones_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(start_zones_entry.x));
		update_values.push_back(columns[1] + " = " + std::to_string(start_zones_entry.y));
		update_values.push_back(columns[2] + " = " + std::to_string(start_zones_entry.z));
		update_values.push_back(columns[3] + " = " + std::to_string(start_zones_entry.heading));
		update_values.push_back(columns[4] + " = " + std::to_string(start_zones_entry.zone_id));
		update_values.push_back(columns[5] + " = " + std::to_string(start_zones_entry.bind_id));
		update_values.push_back(columns[6] + " = " + std::to_string(start_zones_entry.player_choice));
		update_values.push_back(columns[7] + " = " + std::to_string(start_zones_entry.player_class));
		update_values.push_back(columns[8] + " = " + std::to_string(start_zones_entry.player_deity));
		update_values.push_back(columns[9] + " = " + std::to_string(start_zones_entry.player_race));
		update_values.push_back(columns[10] + " = " + std::to_string(start_zones_entry.start_zone));
		update_values.push_back(columns[11] + " = " + std::to_string(start_zones_entry.bind_x));
		update_values.push_back(columns[12] + " = " + std::to_string(start_zones_entry.bind_y));
		update_values.push_back(columns[13] + " = " + std::to_string(start_zones_entry.bind_z));
		update_values.push_back(columns[14] + " = " + std::to_string(start_zones_entry.select_rank));
		update_values.push_back(columns[15] + " = " + std::to_string(start_zones_entry.min_expansion));
		update_values.push_back(columns[16] + " = " + std::to_string(start_zones_entry.max_expansion));
		update_values.push_back(columns[17] + " = '" + EscapeString(start_zones_entry.content_flags) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				start_zones_entry.player_choice
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static StartZones InsertOne(
		StartZones start_zones_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(start_zones_entry.x));
		insert_values.push_back(std::to_string(start_zones_entry.y));
		insert_values.push_back(std::to_string(start_zones_entry.z));
		insert_values.push_back(std::to_string(start_zones_entry.heading));
		insert_values.push_back(std::to_string(start_zones_entry.zone_id));
		insert_values.push_back(std::to_string(start_zones_entry.bind_id));
		insert_values.push_back(std::to_string(start_zones_entry.player_choice));
		insert_values.push_back(std::to_string(start_zones_entry.player_class));
		insert_values.push_back(std::to_string(start_zones_entry.player_deity));
		insert_values.push_back(std::to_string(start_zones_entry.player_race));
		insert_values.push_back(std::to_string(start_zones_entry.start_zone));
		insert_values.push_back(std::to_string(start_zones_entry.bind_x));
		insert_values.push_back(std::to_string(start_zones_entry.bind_y));
		insert_values.push_back(std::to_string(start_zones_entry.bind_z));
		insert_values.push_back(std::to_string(start_zones_entry.select_rank));
		insert_values.push_back(std::to_string(start_zones_entry.min_expansion));
		insert_values.push_back(std::to_string(start_zones_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(start_zones_entry.content_flags) + "'");

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			start_zones_entry.player_choice = results.LastInsertedID();
			return start_zones_entry;
		}

		start_zones_entry = NewEntity();

		return start_zones_entry;
	}

	static int InsertMany(
		std::vector<StartZones> start_zones_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &start_zones_entry: start_zones_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(start_zones_entry.x));
			insert_values.push_back(std::to_string(start_zones_entry.y));
			insert_values.push_back(std::to_string(start_zones_entry.z));
			insert_values.push_back(std::to_string(start_zones_entry.heading));
			insert_values.push_back(std::to_string(start_zones_entry.zone_id));
			insert_values.push_back(std::to_string(start_zones_entry.bind_id));
			insert_values.push_back(std::to_string(start_zones_entry.player_choice));
			insert_values.push_back(std::to_string(start_zones_entry.player_class));
			insert_values.push_back(std::to_string(start_zones_entry.player_deity));
			insert_values.push_back(std::to_string(start_zones_entry.player_race));
			insert_values.push_back(std::to_string(start_zones_entry.start_zone));
			insert_values.push_back(std::to_string(start_zones_entry.bind_x));
			insert_values.push_back(std::to_string(start_zones_entry.bind_y));
			insert_values.push_back(std::to_string(start_zones_entry.bind_z));
			insert_values.push_back(std::to_string(start_zones_entry.select_rank));
			insert_values.push_back(std::to_string(start_zones_entry.min_expansion));
			insert_values.push_back(std::to_string(start_zones_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(start_zones_entry.content_flags) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<StartZones> All()
	{
		std::vector<StartZones> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartZones entry{};

			entry.x             = static_cast<float>(atof(row[0]));
			entry.y             = static_cast<float>(atof(row[1]));
			entry.z             = static_cast<float>(atof(row[2]));
			entry.heading       = static_cast<float>(atof(row[3]));
			entry.zone_id       = atoi(row[4]);
			entry.bind_id       = atoi(row[5]);
			entry.player_choice = atoi(row[6]);
			entry.player_class  = atoi(row[7]);
			entry.player_deity  = atoi(row[8]);
			entry.player_race   = atoi(row[9]);
			entry.start_zone    = atoi(row[10]);
			entry.bind_x        = static_cast<float>(atof(row[11]));
			entry.bind_y        = static_cast<float>(atof(row[12]));
			entry.bind_z        = static_cast<float>(atof(row[13]));
			entry.select_rank   = atoi(row[14]);
			entry.min_expansion = atoi(row[15]);
			entry.max_expansion = atoi(row[16]);
			entry.content_flags = row[17] ? row[17] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<StartZones> GetWhere(std::string where_filter)
	{
		std::vector<StartZones> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			StartZones entry{};

			entry.x             = static_cast<float>(atof(row[0]));
			entry.y             = static_cast<float>(atof(row[1]));
			entry.z             = static_cast<float>(atof(row[2]));
			entry.heading       = static_cast<float>(atof(row[3]));
			entry.zone_id       = atoi(row[4]);
			entry.bind_id       = atoi(row[5]);
			entry.player_choice = atoi(row[6]);
			entry.player_class  = atoi(row[7]);
			entry.player_deity  = atoi(row[8]);
			entry.player_race   = atoi(row[9]);
			entry.start_zone    = atoi(row[10]);
			entry.bind_x        = static_cast<float>(atof(row[11]));
			entry.bind_y        = static_cast<float>(atof(row[12]));
			entry.bind_z        = static_cast<float>(atof(row[13]));
			entry.select_rank   = atoi(row[14]);
			entry.min_expansion = atoi(row[15]);
			entry.max_expansion = atoi(row[16]);
			entry.content_flags = row[17] ? row[17] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_START_ZONES_REPOSITORY_H
