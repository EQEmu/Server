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

#ifndef EQEMU_BASE_BUGS_REPOSITORY_H
#define EQEMU_BASE_BUGS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseBugsRepository {
public:
	struct Bugs {
		int         id;
		std::string zone;
		std::string name;
		std::string ui;
		float       x;
		float       y;
		float       z;
		std::string type;
		int         flag;
		std::string target;
		std::string bug;
		std::string date;
		int         status;
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
			"name",
			"ui",
			"x",
			"y",
			"z",
			"type",
			"flag",
			"target",
			"bug",
			"date",
			"status",
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
		return std::string("bugs");
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

	static Bugs NewEntity()
	{
		Bugs entry{};

		entry.id     = 0;
		entry.zone   = "";
		entry.name   = "";
		entry.ui     = "";
		entry.x      = 0;
		entry.y      = 0;
		entry.z      = 0;
		entry.type   = "";
		entry.flag   = 0;
		entry.target = "";
		entry.bug    = "";
		entry.date   = 0;
		entry.status = 0;

		return entry;
	}

	static Bugs GetBugsEntry(
		const std::vector<Bugs> &bugss,
		int bugs_id
	)
	{
		for (auto &bugs : bugss) {
			if (bugs.id == bugs_id) {
				return bugs;
			}
		}

		return NewEntity();
	}

	static Bugs FindOne(
		Database& db,
		int bugs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				bugs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Bugs entry{};

			entry.id     = atoi(row[0]);
			entry.zone   = row[1] ? row[1] : "";
			entry.name   = row[2] ? row[2] : "";
			entry.ui     = row[3] ? row[3] : "";
			entry.x      = static_cast<float>(atof(row[4]));
			entry.y      = static_cast<float>(atof(row[5]));
			entry.z      = static_cast<float>(atof(row[6]));
			entry.type   = row[7] ? row[7] : "";
			entry.flag   = atoi(row[8]);
			entry.target = row[9] ? row[9] : "";
			entry.bug    = row[10] ? row[10] : "";
			entry.date   = row[11] ? row[11] : "";
			entry.status = atoi(row[12]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bugs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bugs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Bugs bugs_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(bugs_entry.zone) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(bugs_entry.name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(bugs_entry.ui) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(bugs_entry.x));
		update_values.push_back(columns[5] + " = " + std::to_string(bugs_entry.y));
		update_values.push_back(columns[6] + " = " + std::to_string(bugs_entry.z));
		update_values.push_back(columns[7] + " = '" + EscapeString(bugs_entry.type) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(bugs_entry.flag));
		update_values.push_back(columns[9] + " = '" + EscapeString(bugs_entry.target) + "'");
		update_values.push_back(columns[10] + " = '" + EscapeString(bugs_entry.bug) + "'");
		update_values.push_back(columns[11] + " = '" + EscapeString(bugs_entry.date) + "'");
		update_values.push_back(columns[12] + " = " + std::to_string(bugs_entry.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				bugs_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Bugs InsertOne(
		Database& db,
		Bugs bugs_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(bugs_entry.zone) + "'");
		insert_values.push_back("'" + EscapeString(bugs_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(bugs_entry.ui) + "'");
		insert_values.push_back(std::to_string(bugs_entry.x));
		insert_values.push_back(std::to_string(bugs_entry.y));
		insert_values.push_back(std::to_string(bugs_entry.z));
		insert_values.push_back("'" + EscapeString(bugs_entry.type) + "'");
		insert_values.push_back(std::to_string(bugs_entry.flag));
		insert_values.push_back("'" + EscapeString(bugs_entry.target) + "'");
		insert_values.push_back("'" + EscapeString(bugs_entry.bug) + "'");
		insert_values.push_back("'" + EscapeString(bugs_entry.date) + "'");
		insert_values.push_back(std::to_string(bugs_entry.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			bugs_entry.id = results.LastInsertedID();
			return bugs_entry;
		}

		bugs_entry = NewEntity();

		return bugs_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Bugs> bugs_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &bugs_entry: bugs_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(bugs_entry.zone) + "'");
			insert_values.push_back("'" + EscapeString(bugs_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(bugs_entry.ui) + "'");
			insert_values.push_back(std::to_string(bugs_entry.x));
			insert_values.push_back(std::to_string(bugs_entry.y));
			insert_values.push_back(std::to_string(bugs_entry.z));
			insert_values.push_back("'" + EscapeString(bugs_entry.type) + "'");
			insert_values.push_back(std::to_string(bugs_entry.flag));
			insert_values.push_back("'" + EscapeString(bugs_entry.target) + "'");
			insert_values.push_back("'" + EscapeString(bugs_entry.bug) + "'");
			insert_values.push_back("'" + EscapeString(bugs_entry.date) + "'");
			insert_values.push_back(std::to_string(bugs_entry.status));

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

	static std::vector<Bugs> All(Database& db)
	{
		std::vector<Bugs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Bugs entry{};

			entry.id     = atoi(row[0]);
			entry.zone   = row[1] ? row[1] : "";
			entry.name   = row[2] ? row[2] : "";
			entry.ui     = row[3] ? row[3] : "";
			entry.x      = static_cast<float>(atof(row[4]));
			entry.y      = static_cast<float>(atof(row[5]));
			entry.z      = static_cast<float>(atof(row[6]));
			entry.type   = row[7] ? row[7] : "";
			entry.flag   = atoi(row[8]);
			entry.target = row[9] ? row[9] : "";
			entry.bug    = row[10] ? row[10] : "";
			entry.date   = row[11] ? row[11] : "";
			entry.status = atoi(row[12]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Bugs> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Bugs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Bugs entry{};

			entry.id     = atoi(row[0]);
			entry.zone   = row[1] ? row[1] : "";
			entry.name   = row[2] ? row[2] : "";
			entry.ui     = row[3] ? row[3] : "";
			entry.x      = static_cast<float>(atof(row[4]));
			entry.y      = static_cast<float>(atof(row[5]));
			entry.z      = static_cast<float>(atof(row[6]));
			entry.type   = row[7] ? row[7] : "";
			entry.flag   = atoi(row[8]);
			entry.target = row[9] ? row[9] : "";
			entry.bug    = row[10] ? row[10] : "";
			entry.date   = row[11] ? row[11] : "";
			entry.status = atoi(row[12]);

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

#endif //EQEMU_BASE_BUGS_REPOSITORY_H
