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

#ifndef EQEMU_BASE_GUILD_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_GUILD_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGuildMembersRepository {
public:
	struct GuildMembers {
		int         char_id;
		int         guild_id;
		int         rank;
		int         tribute_enable;
		int         total_tribute;
		int         last_tribute;
		int         banker;
		std::string public_note;
		int         alt;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"guild_id",
			"rank",
			"tribute_enable",
			"total_tribute",
			"last_tribute",
			"banker",
			"public_note",
			"alt",
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
		return std::string("guild_members");
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

	static GuildMembers NewEntity()
	{
		GuildMembers entry{};

		entry.char_id        = 0;
		entry.guild_id       = 0;
		entry.rank           = 0;
		entry.tribute_enable = 0;
		entry.total_tribute  = 0;
		entry.last_tribute   = 0;
		entry.banker         = 0;
		entry.public_note    = "";
		entry.alt            = 0;

		return entry;
	}

	static GuildMembers GetGuildMembersEntry(
		const std::vector<GuildMembers> &guild_memberss,
		int guild_members_id
	)
	{
		for (auto &guild_members : guild_memberss) {
			if (guild_members.char_id == guild_members_id) {
				return guild_members;
			}
		}

		return NewEntity();
	}

	static GuildMembers FindOne(
		Database& db,
		int guild_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				guild_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildMembers entry{};

			entry.char_id        = atoi(row[0]);
			entry.guild_id       = atoi(row[1]);
			entry.rank           = atoi(row[2]);
			entry.tribute_enable = atoi(row[3]);
			entry.total_tribute  = atoi(row[4]);
			entry.last_tribute   = atoi(row[5]);
			entry.banker         = atoi(row[6]);
			entry.public_note    = row[7] ? row[7] : "";
			entry.alt            = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guild_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		GuildMembers guild_members_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(guild_members_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(guild_members_entry.guild_id));
		update_values.push_back(columns[2] + " = " + std::to_string(guild_members_entry.rank));
		update_values.push_back(columns[3] + " = " + std::to_string(guild_members_entry.tribute_enable));
		update_values.push_back(columns[4] + " = " + std::to_string(guild_members_entry.total_tribute));
		update_values.push_back(columns[5] + " = " + std::to_string(guild_members_entry.last_tribute));
		update_values.push_back(columns[6] + " = " + std::to_string(guild_members_entry.banker));
		update_values.push_back(columns[7] + " = '" + EscapeString(guild_members_entry.public_note) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(guild_members_entry.alt));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				guild_members_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildMembers InsertOne(
		Database& db,
		GuildMembers guild_members_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(guild_members_entry.char_id));
		insert_values.push_back(std::to_string(guild_members_entry.guild_id));
		insert_values.push_back(std::to_string(guild_members_entry.rank));
		insert_values.push_back(std::to_string(guild_members_entry.tribute_enable));
		insert_values.push_back(std::to_string(guild_members_entry.total_tribute));
		insert_values.push_back(std::to_string(guild_members_entry.last_tribute));
		insert_values.push_back(std::to_string(guild_members_entry.banker));
		insert_values.push_back("'" + EscapeString(guild_members_entry.public_note) + "'");
		insert_values.push_back(std::to_string(guild_members_entry.alt));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			guild_members_entry.char_id = results.LastInsertedID();
			return guild_members_entry;
		}

		guild_members_entry = NewEntity();

		return guild_members_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GuildMembers> guild_members_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &guild_members_entry: guild_members_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(guild_members_entry.char_id));
			insert_values.push_back(std::to_string(guild_members_entry.guild_id));
			insert_values.push_back(std::to_string(guild_members_entry.rank));
			insert_values.push_back(std::to_string(guild_members_entry.tribute_enable));
			insert_values.push_back(std::to_string(guild_members_entry.total_tribute));
			insert_values.push_back(std::to_string(guild_members_entry.last_tribute));
			insert_values.push_back(std::to_string(guild_members_entry.banker));
			insert_values.push_back("'" + EscapeString(guild_members_entry.public_note) + "'");
			insert_values.push_back(std::to_string(guild_members_entry.alt));

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

	static std::vector<GuildMembers> All(Database& db)
	{
		std::vector<GuildMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildMembers entry{};

			entry.char_id        = atoi(row[0]);
			entry.guild_id       = atoi(row[1]);
			entry.rank           = atoi(row[2]);
			entry.tribute_enable = atoi(row[3]);
			entry.total_tribute  = atoi(row[4]);
			entry.last_tribute   = atoi(row[5]);
			entry.banker         = atoi(row[6]);
			entry.public_note    = row[7] ? row[7] : "";
			entry.alt            = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GuildMembers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<GuildMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildMembers entry{};

			entry.char_id        = atoi(row[0]);
			entry.guild_id       = atoi(row[1]);
			entry.rank           = atoi(row[2]);
			entry.tribute_enable = atoi(row[3]);
			entry.total_tribute  = atoi(row[4]);
			entry.last_tribute   = atoi(row[5]);
			entry.banker         = atoi(row[6]);
			entry.public_note    = row[7] ? row[7] : "";
			entry.alt            = atoi(row[8]);

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

#endif //EQEMU_BASE_GUILD_MEMBERS_REPOSITORY_H
