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

#ifndef EQEMU_BASE_NPC_FACTION_REPOSITORY_H
#define EQEMU_BASE_NPC_FACTION_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcFactionRepository {
public:
	struct NpcFaction {
		int         id;
		std::string name;
		int         primaryfaction;
		int         ignore_primary_assist;
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
			"primaryfaction",
			"ignore_primary_assist",
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
		return std::string("npc_faction");
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

	static NpcFaction NewEntity()
	{
		NpcFaction entry{};

		entry.id                    = 0;
		entry.name                  = "";
		entry.primaryfaction        = 0;
		entry.ignore_primary_assist = 0;

		return entry;
	}

	static NpcFaction GetNpcFactionEntry(
		const std::vector<NpcFaction> &npc_factions,
		int npc_faction_id
	)
	{
		for (auto &npc_faction : npc_factions) {
			if (npc_faction.id == npc_faction_id) {
				return npc_faction;
			}
		}

		return NewEntity();
	}

	static NpcFaction FindOne(
		int npc_faction_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_faction_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcFaction entry{};

			entry.id                    = atoi(row[0]);
			entry.name                  = row[1] ? row[1] : "";
			entry.primaryfaction        = atoi(row[2]);
			entry.ignore_primary_assist = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int npc_faction_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		NpcFaction npc_faction_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(npc_faction_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(npc_faction_entry.primaryfaction));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_faction_entry.ignore_primary_assist));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_faction_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcFaction InsertOne(
		NpcFaction npc_faction_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(npc_faction_entry.name) + "'");
		insert_values.push_back(std::to_string(npc_faction_entry.primaryfaction));
		insert_values.push_back(std::to_string(npc_faction_entry.ignore_primary_assist));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_faction_entry.id = results.LastInsertedID();
			return npc_faction_entry;
		}

		npc_faction_entry = NewEntity();

		return npc_faction_entry;
	}

	static int InsertMany(
		std::vector<NpcFaction> npc_faction_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_faction_entry: npc_faction_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(npc_faction_entry.name) + "'");
			insert_values.push_back(std::to_string(npc_faction_entry.primaryfaction));
			insert_values.push_back(std::to_string(npc_faction_entry.ignore_primary_assist));

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

	static std::vector<NpcFaction> All()
	{
		std::vector<NpcFaction> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcFaction entry{};

			entry.id                    = atoi(row[0]);
			entry.name                  = row[1] ? row[1] : "";
			entry.primaryfaction        = atoi(row[2]);
			entry.ignore_primary_assist = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcFaction> GetWhere(std::string where_filter)
	{
		std::vector<NpcFaction> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcFaction entry{};

			entry.id                    = atoi(row[0]);
			entry.name                  = row[1] ? row[1] : "";
			entry.primaryfaction        = atoi(row[2]);
			entry.ignore_primary_assist = atoi(row[3]);

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

#endif //EQEMU_BASE_NPC_FACTION_REPOSITORY_H
