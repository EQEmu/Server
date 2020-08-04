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

#ifndef EQEMU_BASE_GUILDS_REPOSITORY_H
#define EQEMU_BASE_GUILDS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGuildsRepository {
public:
	struct Guilds {
		int         id;
		std::string name;
		int         leader;
		int         minstatus;
		std::string motd;
		int         tribute;
		std::string motd_setter;
		std::string channel;
		std::string url;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"name",
			"leader",
			"minstatus",
			"motd",
			"tribute",
			"motd_setter",
			"channel",
			"url",
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
		return std::string("guilds");
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

	static Guilds NewEntity()
	{
		Guilds entry{};

		entry.id          = 0;
		entry.name        = "";
		entry.leader      = 0;
		entry.minstatus   = 0;
		entry.motd        = "";
		entry.tribute     = 0;
		entry.motd_setter = "";
		entry.channel     = "";
		entry.url         = "";

		return entry;
	}

	static Guilds GetGuildsEntry(
		const std::vector<Guilds> &guildss,
		int guilds_id
	)
	{
		for (auto &guilds : guildss) {
			if (guilds.id == guilds_id) {
				return guilds;
			}
		}

		return NewEntity();
	}

	static Guilds FindOne(
		int guilds_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				guilds_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Guilds entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.leader      = atoi(row[2]);
			entry.minstatus   = atoi(row[3]);
			entry.motd        = row[4] ? row[4] : "";
			entry.tribute     = atoi(row[5]);
			entry.motd_setter = row[6] ? row[6] : "";
			entry.channel     = row[7] ? row[7] : "";
			entry.url         = row[8] ? row[8] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int guilds_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guilds_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Guilds guilds_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(guilds_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(guilds_entry.leader));
		update_values.push_back(columns[3] + " = " + std::to_string(guilds_entry.minstatus));
		update_values.push_back(columns[4] + " = '" + EscapeString(guilds_entry.motd) + "'");
		update_values.push_back(columns[5] + " = " + std::to_string(guilds_entry.tribute));
		update_values.push_back(columns[6] + " = '" + EscapeString(guilds_entry.motd_setter) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(guilds_entry.channel) + "'");
		update_values.push_back(columns[8] + " = '" + EscapeString(guilds_entry.url) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				guilds_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Guilds InsertOne(
		Guilds guilds_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(guilds_entry.name) + "'");
		insert_values.push_back(std::to_string(guilds_entry.leader));
		insert_values.push_back(std::to_string(guilds_entry.minstatus));
		insert_values.push_back("'" + EscapeString(guilds_entry.motd) + "'");
		insert_values.push_back(std::to_string(guilds_entry.tribute));
		insert_values.push_back("'" + EscapeString(guilds_entry.motd_setter) + "'");
		insert_values.push_back("'" + EscapeString(guilds_entry.channel) + "'");
		insert_values.push_back("'" + EscapeString(guilds_entry.url) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			guilds_entry.id = results.LastInsertedID();
			return guilds_entry;
		}

		guilds_entry = NewEntity();

		return guilds_entry;
	}

	static int InsertMany(
		std::vector<Guilds> guilds_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &guilds_entry: guilds_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(guilds_entry.name) + "'");
			insert_values.push_back(std::to_string(guilds_entry.leader));
			insert_values.push_back(std::to_string(guilds_entry.minstatus));
			insert_values.push_back("'" + EscapeString(guilds_entry.motd) + "'");
			insert_values.push_back(std::to_string(guilds_entry.tribute));
			insert_values.push_back("'" + EscapeString(guilds_entry.motd_setter) + "'");
			insert_values.push_back("'" + EscapeString(guilds_entry.channel) + "'");
			insert_values.push_back("'" + EscapeString(guilds_entry.url) + "'");

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

	static std::vector<Guilds> All()
	{
		std::vector<Guilds> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Guilds entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.leader      = atoi(row[2]);
			entry.minstatus   = atoi(row[3]);
			entry.motd        = row[4] ? row[4] : "";
			entry.tribute     = atoi(row[5]);
			entry.motd_setter = row[6] ? row[6] : "";
			entry.channel     = row[7] ? row[7] : "";
			entry.url         = row[8] ? row[8] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Guilds> GetWhere(std::string where_filter)
	{
		std::vector<Guilds> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Guilds entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.leader      = atoi(row[2]);
			entry.minstatus   = atoi(row[3]);
			entry.motd        = row[4] ? row[4] : "";
			entry.tribute     = atoi(row[5]);
			entry.motd_setter = row[6] ? row[6] : "";
			entry.channel     = row[7] ? row[7] : "";
			entry.url         = row[8] ? row[8] : "";

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

#endif //EQEMU_BASE_GUILDS_REPOSITORY_H
