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

#ifndef EQEMU_CHATCHANNELS_REPOSITORY_H
#define EQEMU_CHATCHANNELS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class ChatchannelsRepository {
public:
	struct Chatchannels {
		std::string name;
		std::string owner;
		std::string password;
		int         minstatus;
	};

	static std::string PrimaryKey()
	{
		return std::string("name");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"name",
			"owner",
			"password",
			"minstatus",
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
		return std::string("chatchannels");
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

	static Chatchannels NewEntity()
	{
		Chatchannels entry{};

		entry.name      = "";
		entry.owner     = "";
		entry.password  = "";
		entry.minstatus = 0;

		return entry;
	}

	static Chatchannels GetChatchannelsEntry(
		const std::vector<Chatchannels> &chatchannelss,
		int chatchannels_id
	)
	{
		for (auto &chatchannels : chatchannelss) {
			if (chatchannels.name == chatchannels_id) {
				return chatchannels;
			}
		}

		return NewEntity();
	}

	static Chatchannels FindOne(
		int chatchannels_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				chatchannels_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Chatchannels entry{};

			entry.name      = row[0] ? row[0] : "";
			entry.owner     = row[1] ? row[1] : "";
			entry.password  = row[2] ? row[2] : "";
			entry.minstatus = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int chatchannels_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				chatchannels_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Chatchannels chatchannels_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(chatchannels_entry.owner) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(chatchannels_entry.password) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(chatchannels_entry.minstatus));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				chatchannels_entry.name
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Chatchannels InsertOne(
		Chatchannels chatchannels_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(chatchannels_entry.owner) + "'");
		insert_values.push_back("'" + EscapeString(chatchannels_entry.password) + "'");
		insert_values.push_back(std::to_string(chatchannels_entry.minstatus));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			chatchannels_entry.id = results.LastInsertedID();
			return chatchannels_entry;
		}

		chatchannels_entry = ChatchannelsRepository::NewEntity();

		return chatchannels_entry;
	}

	static int InsertMany(
		std::vector<Chatchannels> chatchannels_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &chatchannels_entry: chatchannels_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(chatchannels_entry.owner) + "'");
			insert_values.push_back("'" + EscapeString(chatchannels_entry.password) + "'");
			insert_values.push_back(std::to_string(chatchannels_entry.minstatus));

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

	static std::vector<Chatchannels> All()
	{
		std::vector<Chatchannels> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Chatchannels entry{};

			entry.name      = row[0] ? row[0] : "";
			entry.owner     = row[1] ? row[1] : "";
			entry.password  = row[2] ? row[2] : "";
			entry.minstatus = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Chatchannels> GetWhere(std::string where_filter)
	{
		std::vector<Chatchannels> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Chatchannels entry{};

			entry.name      = row[0] ? row[0] : "";
			entry.owner     = row[1] ? row[1] : "";
			entry.password  = row[2] ? row[2] : "";
			entry.minstatus = atoi(row[3]);

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
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_CHATCHANNELS_REPOSITORY_H
