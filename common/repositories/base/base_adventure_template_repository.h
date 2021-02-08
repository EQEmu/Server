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

#ifndef EQEMU_BASE_ADVENTURE_TEMPLATE_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_TEMPLATE_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAdventureTemplateRepository {
public:
	struct AdventureTemplate {
		int         id;
		std::string zone;
		int         zone_version;
		int         is_hard;
		int         is_raid;
		int         min_level;
		int         max_level;
		int         type;
		int         type_data;
		int         type_count;
		float       assa_x;
		float       assa_y;
		float       assa_z;
		float       assa_h;
		std::string text;
		int         duration;
		int         zone_in_time;
		int         win_points;
		int         lose_points;
		int         theme;
		int         zone_in_zone_id;
		float       zone_in_x;
		float       zone_in_y;
		int         zone_in_object_id;
		float       dest_x;
		float       dest_y;
		float       dest_z;
		float       dest_h;
		int         graveyard_zone_id;
		float       graveyard_x;
		float       graveyard_y;
		float       graveyard_z;
		float       graveyard_radius;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"zone_version",
			"is_hard",
			"is_raid",
			"min_level",
			"max_level",
			"type",
			"type_data",
			"type_count",
			"assa_x",
			"assa_y",
			"assa_z",
			"assa_h",
			"text",
			"duration",
			"zone_in_time",
			"win_points",
			"lose_points",
			"theme",
			"zone_in_zone_id",
			"zone_in_x",
			"zone_in_y",
			"zone_in_object_id",
			"dest_x",
			"dest_y",
			"dest_z",
			"dest_h",
			"graveyard_zone_id",
			"graveyard_x",
			"graveyard_y",
			"graveyard_z",
			"graveyard_radius",
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
		return std::string("adventure_template");
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

	static AdventureTemplate NewEntity()
	{
		AdventureTemplate entry{};

		entry.id                = 0;
		entry.zone              = "";
		entry.zone_version      = 0;
		entry.is_hard           = 0;
		entry.is_raid           = 0;
		entry.min_level         = 1;
		entry.max_level         = 65;
		entry.type              = 0;
		entry.type_data         = 0;
		entry.type_count        = 0;
		entry.assa_x            = 0;
		entry.assa_y            = 0;
		entry.assa_z            = 0;
		entry.assa_h            = 0;
		entry.text              = "";
		entry.duration          = 7200;
		entry.zone_in_time      = 1800;
		entry.win_points        = 0;
		entry.lose_points       = 0;
		entry.theme             = 1;
		entry.zone_in_zone_id   = 0;
		entry.zone_in_x         = 0;
		entry.zone_in_y         = 0;
		entry.zone_in_object_id = 0;
		entry.dest_x            = 0;
		entry.dest_y            = 0;
		entry.dest_z            = 0;
		entry.dest_h            = 0;
		entry.graveyard_zone_id = 0;
		entry.graveyard_x       = 0;
		entry.graveyard_y       = 0;
		entry.graveyard_z       = 0;
		entry.graveyard_radius  = 0;

		return entry;
	}

	static AdventureTemplate GetAdventureTemplateEntry(
		const std::vector<AdventureTemplate> &adventure_templates,
		int adventure_template_id
	)
	{
		for (auto &adventure_template : adventure_templates) {
			if (adventure_template.id == adventure_template_id) {
				return adventure_template;
			}
		}

		return NewEntity();
	}

	static AdventureTemplate FindOne(
		Database& db,
		int adventure_template_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_template_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureTemplate entry{};

			entry.id                = atoi(row[0]);
			entry.zone              = row[1] ? row[1] : "";
			entry.zone_version      = atoi(row[2]);
			entry.is_hard           = atoi(row[3]);
			entry.is_raid           = atoi(row[4]);
			entry.min_level         = atoi(row[5]);
			entry.max_level         = atoi(row[6]);
			entry.type              = atoi(row[7]);
			entry.type_data         = atoi(row[8]);
			entry.type_count        = atoi(row[9]);
			entry.assa_x            = static_cast<float>(atof(row[10]));
			entry.assa_y            = static_cast<float>(atof(row[11]));
			entry.assa_z            = static_cast<float>(atof(row[12]));
			entry.assa_h            = static_cast<float>(atof(row[13]));
			entry.text              = row[14] ? row[14] : "";
			entry.duration          = atoi(row[15]);
			entry.zone_in_time      = atoi(row[16]);
			entry.win_points        = atoi(row[17]);
			entry.lose_points       = atoi(row[18]);
			entry.theme             = atoi(row[19]);
			entry.zone_in_zone_id   = atoi(row[20]);
			entry.zone_in_x         = static_cast<float>(atof(row[21]));
			entry.zone_in_y         = static_cast<float>(atof(row[22]));
			entry.zone_in_object_id = atoi(row[23]);
			entry.dest_x            = static_cast<float>(atof(row[24]));
			entry.dest_y            = static_cast<float>(atof(row[25]));
			entry.dest_z            = static_cast<float>(atof(row[26]));
			entry.dest_h            = static_cast<float>(atof(row[27]));
			entry.graveyard_zone_id = atoi(row[28]);
			entry.graveyard_x       = static_cast<float>(atof(row[29]));
			entry.graveyard_y       = static_cast<float>(atof(row[30]));
			entry.graveyard_z       = static_cast<float>(atof(row[31]));
			entry.graveyard_radius  = static_cast<float>(atof(row[32]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_template_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_template_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AdventureTemplate adventure_template_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(adventure_template_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(adventure_template_entry.zone) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(adventure_template_entry.zone_version));
		update_values.push_back(columns[3] + " = " + std::to_string(adventure_template_entry.is_hard));
		update_values.push_back(columns[4] + " = " + std::to_string(adventure_template_entry.is_raid));
		update_values.push_back(columns[5] + " = " + std::to_string(adventure_template_entry.min_level));
		update_values.push_back(columns[6] + " = " + std::to_string(adventure_template_entry.max_level));
		update_values.push_back(columns[7] + " = " + std::to_string(adventure_template_entry.type));
		update_values.push_back(columns[8] + " = " + std::to_string(adventure_template_entry.type_data));
		update_values.push_back(columns[9] + " = " + std::to_string(adventure_template_entry.type_count));
		update_values.push_back(columns[10] + " = " + std::to_string(adventure_template_entry.assa_x));
		update_values.push_back(columns[11] + " = " + std::to_string(adventure_template_entry.assa_y));
		update_values.push_back(columns[12] + " = " + std::to_string(adventure_template_entry.assa_z));
		update_values.push_back(columns[13] + " = " + std::to_string(adventure_template_entry.assa_h));
		update_values.push_back(columns[14] + " = '" + EscapeString(adventure_template_entry.text) + "'");
		update_values.push_back(columns[15] + " = " + std::to_string(adventure_template_entry.duration));
		update_values.push_back(columns[16] + " = " + std::to_string(adventure_template_entry.zone_in_time));
		update_values.push_back(columns[17] + " = " + std::to_string(adventure_template_entry.win_points));
		update_values.push_back(columns[18] + " = " + std::to_string(adventure_template_entry.lose_points));
		update_values.push_back(columns[19] + " = " + std::to_string(adventure_template_entry.theme));
		update_values.push_back(columns[20] + " = " + std::to_string(adventure_template_entry.zone_in_zone_id));
		update_values.push_back(columns[21] + " = " + std::to_string(adventure_template_entry.zone_in_x));
		update_values.push_back(columns[22] + " = " + std::to_string(adventure_template_entry.zone_in_y));
		update_values.push_back(columns[23] + " = " + std::to_string(adventure_template_entry.zone_in_object_id));
		update_values.push_back(columns[24] + " = " + std::to_string(adventure_template_entry.dest_x));
		update_values.push_back(columns[25] + " = " + std::to_string(adventure_template_entry.dest_y));
		update_values.push_back(columns[26] + " = " + std::to_string(adventure_template_entry.dest_z));
		update_values.push_back(columns[27] + " = " + std::to_string(adventure_template_entry.dest_h));
		update_values.push_back(columns[28] + " = " + std::to_string(adventure_template_entry.graveyard_zone_id));
		update_values.push_back(columns[29] + " = " + std::to_string(adventure_template_entry.graveyard_x));
		update_values.push_back(columns[30] + " = " + std::to_string(adventure_template_entry.graveyard_y));
		update_values.push_back(columns[31] + " = " + std::to_string(adventure_template_entry.graveyard_z));
		update_values.push_back(columns[32] + " = " + std::to_string(adventure_template_entry.graveyard_radius));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				adventure_template_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureTemplate InsertOne(
		Database& db,
		AdventureTemplate adventure_template_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(adventure_template_entry.id));
		insert_values.push_back("'" + EscapeString(adventure_template_entry.zone) + "'");
		insert_values.push_back(std::to_string(adventure_template_entry.zone_version));
		insert_values.push_back(std::to_string(adventure_template_entry.is_hard));
		insert_values.push_back(std::to_string(adventure_template_entry.is_raid));
		insert_values.push_back(std::to_string(adventure_template_entry.min_level));
		insert_values.push_back(std::to_string(adventure_template_entry.max_level));
		insert_values.push_back(std::to_string(adventure_template_entry.type));
		insert_values.push_back(std::to_string(adventure_template_entry.type_data));
		insert_values.push_back(std::to_string(adventure_template_entry.type_count));
		insert_values.push_back(std::to_string(adventure_template_entry.assa_x));
		insert_values.push_back(std::to_string(adventure_template_entry.assa_y));
		insert_values.push_back(std::to_string(adventure_template_entry.assa_z));
		insert_values.push_back(std::to_string(adventure_template_entry.assa_h));
		insert_values.push_back("'" + EscapeString(adventure_template_entry.text) + "'");
		insert_values.push_back(std::to_string(adventure_template_entry.duration));
		insert_values.push_back(std::to_string(adventure_template_entry.zone_in_time));
		insert_values.push_back(std::to_string(adventure_template_entry.win_points));
		insert_values.push_back(std::to_string(adventure_template_entry.lose_points));
		insert_values.push_back(std::to_string(adventure_template_entry.theme));
		insert_values.push_back(std::to_string(adventure_template_entry.zone_in_zone_id));
		insert_values.push_back(std::to_string(adventure_template_entry.zone_in_x));
		insert_values.push_back(std::to_string(adventure_template_entry.zone_in_y));
		insert_values.push_back(std::to_string(adventure_template_entry.zone_in_object_id));
		insert_values.push_back(std::to_string(adventure_template_entry.dest_x));
		insert_values.push_back(std::to_string(adventure_template_entry.dest_y));
		insert_values.push_back(std::to_string(adventure_template_entry.dest_z));
		insert_values.push_back(std::to_string(adventure_template_entry.dest_h));
		insert_values.push_back(std::to_string(adventure_template_entry.graveyard_zone_id));
		insert_values.push_back(std::to_string(adventure_template_entry.graveyard_x));
		insert_values.push_back(std::to_string(adventure_template_entry.graveyard_y));
		insert_values.push_back(std::to_string(adventure_template_entry.graveyard_z));
		insert_values.push_back(std::to_string(adventure_template_entry.graveyard_radius));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_template_entry.id = results.LastInsertedID();
			return adventure_template_entry;
		}

		adventure_template_entry = NewEntity();

		return adventure_template_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AdventureTemplate> adventure_template_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_template_entry: adventure_template_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(adventure_template_entry.id));
			insert_values.push_back("'" + EscapeString(adventure_template_entry.zone) + "'");
			insert_values.push_back(std::to_string(adventure_template_entry.zone_version));
			insert_values.push_back(std::to_string(adventure_template_entry.is_hard));
			insert_values.push_back(std::to_string(adventure_template_entry.is_raid));
			insert_values.push_back(std::to_string(adventure_template_entry.min_level));
			insert_values.push_back(std::to_string(adventure_template_entry.max_level));
			insert_values.push_back(std::to_string(adventure_template_entry.type));
			insert_values.push_back(std::to_string(adventure_template_entry.type_data));
			insert_values.push_back(std::to_string(adventure_template_entry.type_count));
			insert_values.push_back(std::to_string(adventure_template_entry.assa_x));
			insert_values.push_back(std::to_string(adventure_template_entry.assa_y));
			insert_values.push_back(std::to_string(adventure_template_entry.assa_z));
			insert_values.push_back(std::to_string(adventure_template_entry.assa_h));
			insert_values.push_back("'" + EscapeString(adventure_template_entry.text) + "'");
			insert_values.push_back(std::to_string(adventure_template_entry.duration));
			insert_values.push_back(std::to_string(adventure_template_entry.zone_in_time));
			insert_values.push_back(std::to_string(adventure_template_entry.win_points));
			insert_values.push_back(std::to_string(adventure_template_entry.lose_points));
			insert_values.push_back(std::to_string(adventure_template_entry.theme));
			insert_values.push_back(std::to_string(adventure_template_entry.zone_in_zone_id));
			insert_values.push_back(std::to_string(adventure_template_entry.zone_in_x));
			insert_values.push_back(std::to_string(adventure_template_entry.zone_in_y));
			insert_values.push_back(std::to_string(adventure_template_entry.zone_in_object_id));
			insert_values.push_back(std::to_string(adventure_template_entry.dest_x));
			insert_values.push_back(std::to_string(adventure_template_entry.dest_y));
			insert_values.push_back(std::to_string(adventure_template_entry.dest_z));
			insert_values.push_back(std::to_string(adventure_template_entry.dest_h));
			insert_values.push_back(std::to_string(adventure_template_entry.graveyard_zone_id));
			insert_values.push_back(std::to_string(adventure_template_entry.graveyard_x));
			insert_values.push_back(std::to_string(adventure_template_entry.graveyard_y));
			insert_values.push_back(std::to_string(adventure_template_entry.graveyard_z));
			insert_values.push_back(std::to_string(adventure_template_entry.graveyard_radius));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<AdventureTemplate> All(Database& db)
	{
		std::vector<AdventureTemplate> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureTemplate entry{};

			entry.id                = atoi(row[0]);
			entry.zone              = row[1] ? row[1] : "";
			entry.zone_version      = atoi(row[2]);
			entry.is_hard           = atoi(row[3]);
			entry.is_raid           = atoi(row[4]);
			entry.min_level         = atoi(row[5]);
			entry.max_level         = atoi(row[6]);
			entry.type              = atoi(row[7]);
			entry.type_data         = atoi(row[8]);
			entry.type_count        = atoi(row[9]);
			entry.assa_x            = static_cast<float>(atof(row[10]));
			entry.assa_y            = static_cast<float>(atof(row[11]));
			entry.assa_z            = static_cast<float>(atof(row[12]));
			entry.assa_h            = static_cast<float>(atof(row[13]));
			entry.text              = row[14] ? row[14] : "";
			entry.duration          = atoi(row[15]);
			entry.zone_in_time      = atoi(row[16]);
			entry.win_points        = atoi(row[17]);
			entry.lose_points       = atoi(row[18]);
			entry.theme             = atoi(row[19]);
			entry.zone_in_zone_id   = atoi(row[20]);
			entry.zone_in_x         = static_cast<float>(atof(row[21]));
			entry.zone_in_y         = static_cast<float>(atof(row[22]));
			entry.zone_in_object_id = atoi(row[23]);
			entry.dest_x            = static_cast<float>(atof(row[24]));
			entry.dest_y            = static_cast<float>(atof(row[25]));
			entry.dest_z            = static_cast<float>(atof(row[26]));
			entry.dest_h            = static_cast<float>(atof(row[27]));
			entry.graveyard_zone_id = atoi(row[28]);
			entry.graveyard_x       = static_cast<float>(atof(row[29]));
			entry.graveyard_y       = static_cast<float>(atof(row[30]));
			entry.graveyard_z       = static_cast<float>(atof(row[31]));
			entry.graveyard_radius  = static_cast<float>(atof(row[32]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AdventureTemplate> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AdventureTemplate> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureTemplate entry{};

			entry.id                = atoi(row[0]);
			entry.zone              = row[1] ? row[1] : "";
			entry.zone_version      = atoi(row[2]);
			entry.is_hard           = atoi(row[3]);
			entry.is_raid           = atoi(row[4]);
			entry.min_level         = atoi(row[5]);
			entry.max_level         = atoi(row[6]);
			entry.type              = atoi(row[7]);
			entry.type_data         = atoi(row[8]);
			entry.type_count        = atoi(row[9]);
			entry.assa_x            = static_cast<float>(atof(row[10]));
			entry.assa_y            = static_cast<float>(atof(row[11]));
			entry.assa_z            = static_cast<float>(atof(row[12]));
			entry.assa_h            = static_cast<float>(atof(row[13]));
			entry.text              = row[14] ? row[14] : "";
			entry.duration          = atoi(row[15]);
			entry.zone_in_time      = atoi(row[16]);
			entry.win_points        = atoi(row[17]);
			entry.lose_points       = atoi(row[18]);
			entry.theme             = atoi(row[19]);
			entry.zone_in_zone_id   = atoi(row[20]);
			entry.zone_in_x         = static_cast<float>(atof(row[21]));
			entry.zone_in_y         = static_cast<float>(atof(row[22]));
			entry.zone_in_object_id = atoi(row[23]);
			entry.dest_x            = static_cast<float>(atof(row[24]));
			entry.dest_y            = static_cast<float>(atof(row[25]));
			entry.dest_z            = static_cast<float>(atof(row[26]));
			entry.dest_h            = static_cast<float>(atof(row[27]));
			entry.graveyard_zone_id = atoi(row[28]);
			entry.graveyard_x       = static_cast<float>(atof(row[29]));
			entry.graveyard_y       = static_cast<float>(atof(row[30]));
			entry.graveyard_z       = static_cast<float>(atof(row[31]));
			entry.graveyard_radius  = static_cast<float>(atof(row[32]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, std::string where_filter)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_ADVENTURE_TEMPLATE_REPOSITORY_H
