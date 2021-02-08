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

#ifndef EQEMU_BASE_GUILD_RELATIONS_REPOSITORY_H
#define EQEMU_BASE_GUILD_RELATIONS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGuildRelationsRepository {
public:
	struct GuildRelations {
		int guild1;
		int guild2;
		int relation;
	};

	static std::string PrimaryKey()
	{
		return std::string("guild1");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"guild1",
			"guild2",
			"relation",
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
		return std::string("guild_relations");
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

	static GuildRelations NewEntity()
	{
		GuildRelations entry{};

		entry.guild1   = 0;
		entry.guild2   = 0;
		entry.relation = 0;

		return entry;
	}

	static GuildRelations GetGuildRelationsEntry(
		const std::vector<GuildRelations> &guild_relationss,
		int guild_relations_id
	)
	{
		for (auto &guild_relations : guild_relationss) {
			if (guild_relations.guild1 == guild_relations_id) {
				return guild_relations;
			}
		}

		return NewEntity();
	}

	static GuildRelations FindOne(
		Database& db,
		int guild_relations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				guild_relations_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GuildRelations entry{};

			entry.guild1   = atoi(row[0]);
			entry.guild2   = atoi(row[1]);
			entry.relation = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int guild_relations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				guild_relations_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		GuildRelations guild_relations_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(guild_relations_entry.guild1));
		update_values.push_back(columns[1] + " = " + std::to_string(guild_relations_entry.guild2));
		update_values.push_back(columns[2] + " = " + std::to_string(guild_relations_entry.relation));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				guild_relations_entry.guild1
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GuildRelations InsertOne(
		Database& db,
		GuildRelations guild_relations_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(guild_relations_entry.guild1));
		insert_values.push_back(std::to_string(guild_relations_entry.guild2));
		insert_values.push_back(std::to_string(guild_relations_entry.relation));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			guild_relations_entry.guild1 = results.LastInsertedID();
			return guild_relations_entry;
		}

		guild_relations_entry = NewEntity();

		return guild_relations_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GuildRelations> guild_relations_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &guild_relations_entry: guild_relations_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(guild_relations_entry.guild1));
			insert_values.push_back(std::to_string(guild_relations_entry.guild2));
			insert_values.push_back(std::to_string(guild_relations_entry.relation));

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

	static std::vector<GuildRelations> All(Database& db)
	{
		std::vector<GuildRelations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildRelations entry{};

			entry.guild1   = atoi(row[0]);
			entry.guild2   = atoi(row[1]);
			entry.relation = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GuildRelations> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<GuildRelations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildRelations entry{};

			entry.guild1   = atoi(row[0]);
			entry.guild2   = atoi(row[1]);
			entry.relation = atoi(row[2]);

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

#endif //EQEMU_BASE_GUILD_RELATIONS_REPOSITORY_H
