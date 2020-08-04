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

#ifndef EQEMU_BASE_PROXIMITIES_REPOSITORY_H
#define EQEMU_BASE_PROXIMITIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseProximitiesRepository {
public:
	struct Proximities {
		int   zoneid;
		int   exploreid;
		float minx;
		float maxx;
		float miny;
		float maxy;
		float minz;
		float maxz;
	};

	static std::string PrimaryKey()
	{
		return std::string("zoneid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"zoneid",
			"exploreid",
			"minx",
			"maxx",
			"miny",
			"maxy",
			"minz",
			"maxz",
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
		return std::string("proximities");
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

	static Proximities NewEntity()
	{
		Proximities entry{};

		entry.zoneid    = 0;
		entry.exploreid = 0;
		entry.minx      = 0.000000;
		entry.maxx      = 0.000000;
		entry.miny      = 0.000000;
		entry.maxy      = 0.000000;
		entry.minz      = 0.000000;
		entry.maxz      = 0.000000;

		return entry;
	}

	static Proximities GetProximitiesEntry(
		const std::vector<Proximities> &proximitiess,
		int proximities_id
	)
	{
		for (auto &proximities : proximitiess) {
			if (proximities.zoneid == proximities_id) {
				return proximities;
			}
		}

		return NewEntity();
	}

	static Proximities FindOne(
		int proximities_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				proximities_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Proximities entry{};

			entry.zoneid    = atoi(row[0]);
			entry.exploreid = atoi(row[1]);
			entry.minx      = static_cast<float>(atof(row[2]));
			entry.maxx      = static_cast<float>(atof(row[3]));
			entry.miny      = static_cast<float>(atof(row[4]));
			entry.maxy      = static_cast<float>(atof(row[5]));
			entry.minz      = static_cast<float>(atof(row[6]));
			entry.maxz      = static_cast<float>(atof(row[7]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int proximities_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				proximities_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Proximities proximities_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(proximities_entry.zoneid));
		update_values.push_back(columns[1] + " = " + std::to_string(proximities_entry.exploreid));
		update_values.push_back(columns[2] + " = " + std::to_string(proximities_entry.minx));
		update_values.push_back(columns[3] + " = " + std::to_string(proximities_entry.maxx));
		update_values.push_back(columns[4] + " = " + std::to_string(proximities_entry.miny));
		update_values.push_back(columns[5] + " = " + std::to_string(proximities_entry.maxy));
		update_values.push_back(columns[6] + " = " + std::to_string(proximities_entry.minz));
		update_values.push_back(columns[7] + " = " + std::to_string(proximities_entry.maxz));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				proximities_entry.zoneid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Proximities InsertOne(
		Proximities proximities_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(proximities_entry.zoneid));
		insert_values.push_back(std::to_string(proximities_entry.exploreid));
		insert_values.push_back(std::to_string(proximities_entry.minx));
		insert_values.push_back(std::to_string(proximities_entry.maxx));
		insert_values.push_back(std::to_string(proximities_entry.miny));
		insert_values.push_back(std::to_string(proximities_entry.maxy));
		insert_values.push_back(std::to_string(proximities_entry.minz));
		insert_values.push_back(std::to_string(proximities_entry.maxz));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			proximities_entry.zoneid = results.LastInsertedID();
			return proximities_entry;
		}

		proximities_entry = NewEntity();

		return proximities_entry;
	}

	static int InsertMany(
		std::vector<Proximities> proximities_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &proximities_entry: proximities_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(proximities_entry.zoneid));
			insert_values.push_back(std::to_string(proximities_entry.exploreid));
			insert_values.push_back(std::to_string(proximities_entry.minx));
			insert_values.push_back(std::to_string(proximities_entry.maxx));
			insert_values.push_back(std::to_string(proximities_entry.miny));
			insert_values.push_back(std::to_string(proximities_entry.maxy));
			insert_values.push_back(std::to_string(proximities_entry.minz));
			insert_values.push_back(std::to_string(proximities_entry.maxz));

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

	static std::vector<Proximities> All()
	{
		std::vector<Proximities> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Proximities entry{};

			entry.zoneid    = atoi(row[0]);
			entry.exploreid = atoi(row[1]);
			entry.minx      = static_cast<float>(atof(row[2]));
			entry.maxx      = static_cast<float>(atof(row[3]));
			entry.miny      = static_cast<float>(atof(row[4]));
			entry.maxy      = static_cast<float>(atof(row[5]));
			entry.minz      = static_cast<float>(atof(row[6]));
			entry.maxz      = static_cast<float>(atof(row[7]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Proximities> GetWhere(std::string where_filter)
	{
		std::vector<Proximities> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Proximities entry{};

			entry.zoneid    = atoi(row[0]);
			entry.exploreid = atoi(row[1]);
			entry.minx      = static_cast<float>(atof(row[2]));
			entry.maxx      = static_cast<float>(atof(row[3]));
			entry.miny      = static_cast<float>(atof(row[4]));
			entry.maxy      = static_cast<float>(atof(row[5]));
			entry.minz      = static_cast<float>(atof(row[6]));
			entry.maxz      = static_cast<float>(atof(row[7]));

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

#endif //EQEMU_BASE_PROXIMITIES_REPOSITORY_H
