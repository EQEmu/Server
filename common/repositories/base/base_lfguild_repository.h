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

#ifndef EQEMU_BASE_LFGUILD_REPOSITORY_H
#define EQEMU_BASE_LFGUILD_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLfguildRepository {
public:
	struct Lfguild {
		int         type;
		std::string name;
		std::string comment;
		int         fromlevel;
		int         tolevel;
		int         classes;
		int         aacount;
		int         timezone;
		int         timeposted;
	};

	static std::string PrimaryKey()
	{
		return std::string("type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"type",
			"name",
			"comment",
			"fromlevel",
			"tolevel",
			"classes",
			"aacount",
			"timezone",
			"timeposted",
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
		return std::string("lfguild");
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

	static Lfguild NewEntity()
	{
		Lfguild entry{};

		entry.type       = 0;
		entry.name       = "";
		entry.comment    = "";
		entry.fromlevel  = 0;
		entry.tolevel    = 0;
		entry.classes    = 0;
		entry.aacount    = 0;
		entry.timezone   = 0;
		entry.timeposted = 0;

		return entry;
	}

	static Lfguild GetLfguildEntry(
		const std::vector<Lfguild> &lfguilds,
		int lfguild_id
	)
	{
		for (auto &lfguild : lfguilds) {
			if (lfguild.type == lfguild_id) {
				return lfguild;
			}
		}

		return NewEntity();
	}

	static Lfguild FindOne(
		int lfguild_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				lfguild_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Lfguild entry{};

			entry.type       = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";
			entry.comment    = row[2] ? row[2] : "";
			entry.fromlevel  = atoi(row[3]);
			entry.tolevel    = atoi(row[4]);
			entry.classes    = atoi(row[5]);
			entry.aacount    = atoi(row[6]);
			entry.timezone   = atoi(row[7]);
			entry.timeposted = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int lfguild_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				lfguild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Lfguild lfguild_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(lfguild_entry.type));
		update_values.push_back(columns[1] + " = '" + EscapeString(lfguild_entry.name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(lfguild_entry.comment) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(lfguild_entry.fromlevel));
		update_values.push_back(columns[4] + " = " + std::to_string(lfguild_entry.tolevel));
		update_values.push_back(columns[5] + " = " + std::to_string(lfguild_entry.classes));
		update_values.push_back(columns[6] + " = " + std::to_string(lfguild_entry.aacount));
		update_values.push_back(columns[7] + " = " + std::to_string(lfguild_entry.timezone));
		update_values.push_back(columns[8] + " = " + std::to_string(lfguild_entry.timeposted));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				lfguild_entry.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Lfguild InsertOne(
		Lfguild lfguild_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(lfguild_entry.type));
		insert_values.push_back("'" + EscapeString(lfguild_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(lfguild_entry.comment) + "'");
		insert_values.push_back(std::to_string(lfguild_entry.fromlevel));
		insert_values.push_back(std::to_string(lfguild_entry.tolevel));
		insert_values.push_back(std::to_string(lfguild_entry.classes));
		insert_values.push_back(std::to_string(lfguild_entry.aacount));
		insert_values.push_back(std::to_string(lfguild_entry.timezone));
		insert_values.push_back(std::to_string(lfguild_entry.timeposted));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			lfguild_entry.type = results.LastInsertedID();
			return lfguild_entry;
		}

		lfguild_entry = NewEntity();

		return lfguild_entry;
	}

	static int InsertMany(
		std::vector<Lfguild> lfguild_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &lfguild_entry: lfguild_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(lfguild_entry.type));
			insert_values.push_back("'" + EscapeString(lfguild_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(lfguild_entry.comment) + "'");
			insert_values.push_back(std::to_string(lfguild_entry.fromlevel));
			insert_values.push_back(std::to_string(lfguild_entry.tolevel));
			insert_values.push_back(std::to_string(lfguild_entry.classes));
			insert_values.push_back(std::to_string(lfguild_entry.aacount));
			insert_values.push_back(std::to_string(lfguild_entry.timezone));
			insert_values.push_back(std::to_string(lfguild_entry.timeposted));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Lfguild> All()
	{
		std::vector<Lfguild> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lfguild entry{};

			entry.type       = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";
			entry.comment    = row[2] ? row[2] : "";
			entry.fromlevel  = atoi(row[3]);
			entry.tolevel    = atoi(row[4]);
			entry.classes    = atoi(row[5]);
			entry.aacount    = atoi(row[6]);
			entry.timezone   = atoi(row[7]);
			entry.timeposted = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Lfguild> GetWhere(std::string where_filter)
	{
		std::vector<Lfguild> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lfguild entry{};

			entry.type       = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";
			entry.comment    = row[2] ? row[2] : "";
			entry.fromlevel  = atoi(row[3]);
			entry.tolevel    = atoi(row[4]);
			entry.classes    = atoi(row[5]);
			entry.aacount    = atoi(row[6]);
			entry.timezone   = atoi(row[7]);
			entry.timeposted = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
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
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_LFGUILD_REPOSITORY_H
