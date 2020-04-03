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

#ifndef EQEMU_ZONE_POINTS_REPOSITORY_H
#define EQEMU_ZONE_POINTS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class ZonePointsRepository {
public:
	struct ZonePoints {
		int         id;
		std::string zone;
		int         version;
		int16       number;
		std::string y;
		std::string x;
		std::string z;
		std::string heading;
		std::string target_y;
		std::string target_x;
		std::string target_z;
		std::string target_heading;
		int16       zoneinst;
		int         target_zone_id;
		int         target_instance;
		std::string buffer;
		int         client_version_mask;
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
			"version",
			"number",
			"y",
			"x",
			"z",
			"heading",
			"target_y",
			"target_x",
			"target_z",
			"target_heading",
			"zoneinst",
			"target_zone_id",
			"target_instance",
			"buffer",
			"client_version_mask",
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
		return std::string("zone_points");
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

	static ZonePoints NewEntity()
	{
		ZonePoints entry{};

		entry.id                  = 0;
		entry.zone                = 0;
		entry.version             = 0;
		entry.number              = 1;
		entry.y                   = 0;
		entry.x                   = 0;
		entry.z                   = 0;
		entry.heading             = 0;
		entry.target_y            = 0;
		entry.target_x            = 0;
		entry.target_z            = 0;
		entry.target_heading      = 0;
		entry.zoneinst            = 0;
		entry.target_zone_id      = 0;
		entry.target_instance     = 0;
		entry.buffer              = 0;
		entry.client_version_mask = 4294967295;

		return entry;
	}

	static ZonePoints GetZonePointsEntry(
		const std::vector<ZonePoints> &zone_pointss,
		int zone_points_id
	)
	{
		for (auto &zone_points : zone_pointss) {
			if (zone_points.id == zone_points_id) {
				return zone_points;
			}
		}

		return NewEntity();
	}

	static ZonePoints FindOne(
		int zone_points_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				zone_points_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ZonePoints entry{};

			entry.id                  = atoi(row[0]);
			entry.zone                = row[1];
			entry.version             = atoi(row[2]);
			entry.number              = atoi(row[3]);
			entry.y                   = atof(row[4]);
			entry.x                   = atof(row[5]);
			entry.z                   = atof(row[6]);
			entry.heading             = atof(row[7]);
			entry.target_y            = atof(row[8]);
			entry.target_x            = atof(row[9]);
			entry.target_z            = atof(row[10]);
			entry.target_heading      = atof(row[11]);
			entry.zoneinst            = atoi(row[12]);
			entry.target_zone_id      = atoi(row[13]);
			entry.target_instance     = atoi(row[14]);
			entry.buffer              = atof(row[15]);
			entry.client_version_mask = atoi(row[16]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int zone_points_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				zone_points_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		ZonePoints zone_points_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(zone_points_entry.zone) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(zone_points_entry.version));
		update_values.push_back(columns[3] + " = " + std::to_string(zone_points_entry.number));
		update_values.push_back(columns[4] + " = '" + EscapeString(zone_points_entry.y) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(zone_points_entry.x) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(zone_points_entry.z) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(zone_points_entry.heading) + "'");
		update_values.push_back(columns[8] + " = '" + EscapeString(zone_points_entry.target_y) + "'");
		update_values.push_back(columns[9] + " = '" + EscapeString(zone_points_entry.target_x) + "'");
		update_values.push_back(columns[10] + " = '" + EscapeString(zone_points_entry.target_z) + "'");
		update_values.push_back(columns[11] + " = '" + EscapeString(zone_points_entry.target_heading) + "'");
		update_values.push_back(columns[12] + " = " + std::to_string(zone_points_entry.zoneinst));
		update_values.push_back(columns[13] + " = " + std::to_string(zone_points_entry.target_zone_id));
		update_values.push_back(columns[14] + " = " + std::to_string(zone_points_entry.target_instance));
		update_values.push_back(columns[15] + " = '" + EscapeString(zone_points_entry.buffer) + "'");
		update_values.push_back(columns[16] + " = " + std::to_string(zone_points_entry.client_version_mask));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				zone_points_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ZonePoints InsertOne(
		ZonePoints zone_points_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(zone_points_entry.zone) + "'");
		insert_values.push_back(std::to_string(zone_points_entry.version));
		insert_values.push_back(std::to_string(zone_points_entry.number));
		insert_values.push_back("'" + EscapeString(zone_points_entry.y) + "'");
		insert_values.push_back("'" + EscapeString(zone_points_entry.x) + "'");
		insert_values.push_back("'" + EscapeString(zone_points_entry.z) + "'");
		insert_values.push_back("'" + EscapeString(zone_points_entry.heading) + "'");
		insert_values.push_back("'" + EscapeString(zone_points_entry.target_y) + "'");
		insert_values.push_back("'" + EscapeString(zone_points_entry.target_x) + "'");
		insert_values.push_back("'" + EscapeString(zone_points_entry.target_z) + "'");
		insert_values.push_back("'" + EscapeString(zone_points_entry.target_heading) + "'");
		insert_values.push_back(std::to_string(zone_points_entry.zoneinst));
		insert_values.push_back(std::to_string(zone_points_entry.target_zone_id));
		insert_values.push_back(std::to_string(zone_points_entry.target_instance));
		insert_values.push_back("'" + EscapeString(zone_points_entry.buffer) + "'");
		insert_values.push_back(std::to_string(zone_points_entry.client_version_mask));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			zone_points_entry.id = results.LastInsertedID();
			return zone_points_entry;
		}

		zone_points_entry = InstanceListRepository::NewEntity();

		return zone_points_entry;
	}

	static int InsertMany(
		std::vector<ZonePoints> zone_points_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &zone_points_entry: zone_points_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(zone_points_entry.zone) + "'");
			insert_values.push_back(std::to_string(zone_points_entry.version));
			insert_values.push_back(std::to_string(zone_points_entry.number));
			insert_values.push_back("'" + EscapeString(zone_points_entry.y) + "'");
			insert_values.push_back("'" + EscapeString(zone_points_entry.x) + "'");
			insert_values.push_back("'" + EscapeString(zone_points_entry.z) + "'");
			insert_values.push_back("'" + EscapeString(zone_points_entry.heading) + "'");
			insert_values.push_back("'" + EscapeString(zone_points_entry.target_y) + "'");
			insert_values.push_back("'" + EscapeString(zone_points_entry.target_x) + "'");
			insert_values.push_back("'" + EscapeString(zone_points_entry.target_z) + "'");
			insert_values.push_back("'" + EscapeString(zone_points_entry.target_heading) + "'");
			insert_values.push_back(std::to_string(zone_points_entry.zoneinst));
			insert_values.push_back(std::to_string(zone_points_entry.target_zone_id));
			insert_values.push_back(std::to_string(zone_points_entry.target_instance));
			insert_values.push_back("'" + EscapeString(zone_points_entry.buffer) + "'");
			insert_values.push_back(std::to_string(zone_points_entry.client_version_mask));

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

	static std::vector<ZonePoints> All()
	{
		std::vector<ZonePoints> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ZonePoints entry{};

			entry.id                  = atoi(row[0]);
			entry.zone                = row[1];
			entry.version             = atoi(row[2]);
			entry.number              = atoi(row[3]);
			entry.y                   = atof(row[4]);
			entry.x                   = atof(row[5]);
			entry.z                   = atof(row[6]);
			entry.heading             = atof(row[7]);
			entry.target_y            = atof(row[8]);
			entry.target_x            = atof(row[9]);
			entry.target_z            = atof(row[10]);
			entry.target_heading      = atof(row[11]);
			entry.zoneinst            = atoi(row[12]);
			entry.target_zone_id      = atoi(row[13]);
			entry.target_instance     = atoi(row[14]);
			entry.buffer              = atof(row[15]);
			entry.client_version_mask = atoi(row[16]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

};

#endif //EQEMU_ZONE_POINTS_REPOSITORY_H
