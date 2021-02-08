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

#ifndef EQEMU_BASE_EXPEDITION_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_EXPEDITION_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseExpeditionMembersRepository {
public:
	struct ExpeditionMembers {
		int id;
		int expedition_id;
		int character_id;
		int is_current_member;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"expedition_id",
			"character_id",
			"is_current_member",
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
		return std::string("expedition_members");
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

	static ExpeditionMembers NewEntity()
	{
		ExpeditionMembers entry{};

		entry.id                = 0;
		entry.expedition_id     = 0;
		entry.character_id      = 0;
		entry.is_current_member = 1;

		return entry;
	}

	static ExpeditionMembers GetExpeditionMembersEntry(
		const std::vector<ExpeditionMembers> &expedition_memberss,
		int expedition_members_id
	)
	{
		for (auto &expedition_members : expedition_memberss) {
			if (expedition_members.id == expedition_members_id) {
				return expedition_members;
			}
		}

		return NewEntity();
	}

	static ExpeditionMembers FindOne(
		Database& db,
		int expedition_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				expedition_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ExpeditionMembers entry{};

			entry.id                = atoi(row[0]);
			entry.expedition_id     = atoi(row[1]);
			entry.character_id      = atoi(row[2]);
			entry.is_current_member = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int expedition_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				expedition_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		ExpeditionMembers expedition_members_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(expedition_members_entry.expedition_id));
		update_values.push_back(columns[2] + " = " + std::to_string(expedition_members_entry.character_id));
		update_values.push_back(columns[3] + " = " + std::to_string(expedition_members_entry.is_current_member));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				expedition_members_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ExpeditionMembers InsertOne(
		Database& db,
		ExpeditionMembers expedition_members_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(expedition_members_entry.expedition_id));
		insert_values.push_back(std::to_string(expedition_members_entry.character_id));
		insert_values.push_back(std::to_string(expedition_members_entry.is_current_member));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			expedition_members_entry.id = results.LastInsertedID();
			return expedition_members_entry;
		}

		expedition_members_entry = NewEntity();

		return expedition_members_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<ExpeditionMembers> expedition_members_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &expedition_members_entry: expedition_members_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(expedition_members_entry.expedition_id));
			insert_values.push_back(std::to_string(expedition_members_entry.character_id));
			insert_values.push_back(std::to_string(expedition_members_entry.is_current_member));

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

	static std::vector<ExpeditionMembers> All(Database& db)
	{
		std::vector<ExpeditionMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ExpeditionMembers entry{};

			entry.id                = atoi(row[0]);
			entry.expedition_id     = atoi(row[1]);
			entry.character_id      = atoi(row[2]);
			entry.is_current_member = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<ExpeditionMembers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<ExpeditionMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ExpeditionMembers entry{};

			entry.id                = atoi(row[0]);
			entry.expedition_id     = atoi(row[1]);
			entry.character_id      = atoi(row[2]);
			entry.is_current_member = atoi(row[3]);

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

#endif //EQEMU_BASE_EXPEDITION_MEMBERS_REPOSITORY_H
