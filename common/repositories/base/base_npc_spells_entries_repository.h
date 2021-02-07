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

#ifndef EQEMU_BASE_NPC_SPELLS_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_NPC_SPELLS_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcSpellsEntriesRepository {
public:
	struct NpcSpellsEntries {
		int id;
		int npc_spells_id;
		int spellid;
		int type;
		int minlevel;
		int maxlevel;
		int manacost;
		int recast_delay;
		int priority;
		int resist_adjust;
		int min_hp;
		int max_hp;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"npc_spells_id",
			"spellid",
			"type",
			"minlevel",
			"maxlevel",
			"manacost",
			"recast_delay",
			"priority",
			"resist_adjust",
			"min_hp",
			"max_hp",
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
		return std::string("npc_spells_entries");
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

	static NpcSpellsEntries NewEntity()
	{
		NpcSpellsEntries entry{};

		entry.id            = 0;
		entry.npc_spells_id = 0;
		entry.spellid       = 0;
		entry.type          = 0;
		entry.minlevel      = 0;
		entry.maxlevel      = 255;
		entry.manacost      = -1;
		entry.recast_delay  = -1;
		entry.priority      = 0;
		entry.resist_adjust = 0;
		entry.min_hp        = 0;
		entry.max_hp        = 0;

		return entry;
	}

	static NpcSpellsEntries GetNpcSpellsEntriesEntry(
		const std::vector<NpcSpellsEntries> &npc_spells_entriess,
		int npc_spells_entries_id
	)
	{
		for (auto &npc_spells_entries : npc_spells_entriess) {
			if (npc_spells_entries.id == npc_spells_entries_id) {
				return npc_spells_entries;
			}
		}

		return NewEntity();
	}

	static NpcSpellsEntries FindOne(
		Database& db,
		int npc_spells_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_spells_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcSpellsEntries entry{};

			entry.id            = atoi(row[0]);
			entry.npc_spells_id = atoi(row[1]);
			entry.spellid       = atoi(row[2]);
			entry.type          = atoi(row[3]);
			entry.minlevel      = atoi(row[4]);
			entry.maxlevel      = atoi(row[5]);
			entry.manacost      = atoi(row[6]);
			entry.recast_delay  = atoi(row[7]);
			entry.priority      = atoi(row[8]);
			entry.resist_adjust = atoi(row[9]);
			entry.min_hp        = atoi(row[10]);
			entry.max_hp        = atoi(row[11]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_spells_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_spells_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcSpellsEntries npc_spells_entries_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(npc_spells_entries_entry.npc_spells_id));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_spells_entries_entry.spellid));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_spells_entries_entry.type));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_spells_entries_entry.minlevel));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_spells_entries_entry.maxlevel));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_spells_entries_entry.manacost));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_spells_entries_entry.recast_delay));
		update_values.push_back(columns[8] + " = " + std::to_string(npc_spells_entries_entry.priority));
		update_values.push_back(columns[9] + " = " + std::to_string(npc_spells_entries_entry.resist_adjust));
		update_values.push_back(columns[10] + " = " + std::to_string(npc_spells_entries_entry.min_hp));
		update_values.push_back(columns[11] + " = " + std::to_string(npc_spells_entries_entry.max_hp));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_spells_entries_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcSpellsEntries InsertOne(
		Database& db,
		NpcSpellsEntries npc_spells_entries_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_spells_entries_entry.npc_spells_id));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.spellid));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.type));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.minlevel));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.maxlevel));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.manacost));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.recast_delay));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.priority));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.resist_adjust));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.min_hp));
		insert_values.push_back(std::to_string(npc_spells_entries_entry.max_hp));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_spells_entries_entry.id = results.LastInsertedID();
			return npc_spells_entries_entry;
		}

		npc_spells_entries_entry = NewEntity();

		return npc_spells_entries_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcSpellsEntries> npc_spells_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_spells_entries_entry: npc_spells_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_spells_entries_entry.npc_spells_id));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.spellid));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.type));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.minlevel));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.maxlevel));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.manacost));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.recast_delay));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.priority));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.resist_adjust));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.min_hp));
			insert_values.push_back(std::to_string(npc_spells_entries_entry.max_hp));

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

	static std::vector<NpcSpellsEntries> All(Database& db)
	{
		std::vector<NpcSpellsEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpellsEntries entry{};

			entry.id            = atoi(row[0]);
			entry.npc_spells_id = atoi(row[1]);
			entry.spellid       = atoi(row[2]);
			entry.type          = atoi(row[3]);
			entry.minlevel      = atoi(row[4]);
			entry.maxlevel      = atoi(row[5]);
			entry.manacost      = atoi(row[6]);
			entry.recast_delay  = atoi(row[7]);
			entry.priority      = atoi(row[8]);
			entry.resist_adjust = atoi(row[9]);
			entry.min_hp        = atoi(row[10]);
			entry.max_hp        = atoi(row[11]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcSpellsEntries> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcSpellsEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpellsEntries entry{};

			entry.id            = atoi(row[0]);
			entry.npc_spells_id = atoi(row[1]);
			entry.spellid       = atoi(row[2]);
			entry.type          = atoi(row[3]);
			entry.minlevel      = atoi(row[4]);
			entry.maxlevel      = atoi(row[5]);
			entry.manacost      = atoi(row[6]);
			entry.recast_delay  = atoi(row[7]);
			entry.priority      = atoi(row[8]);
			entry.resist_adjust = atoi(row[9]);
			entry.min_hp        = atoi(row[10]);
			entry.max_hp        = atoi(row[11]);

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

#endif //EQEMU_BASE_NPC_SPELLS_ENTRIES_REPOSITORY_H
