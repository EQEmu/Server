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

#ifndef EQEMU_BASE_NPC_SPELLS_REPOSITORY_H
#define EQEMU_BASE_NPC_SPELLS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcSpellsRepository {
public:
	struct NpcSpells {
		int         id;
		std::string name;
		int         parent_list;
		int         attack_proc;
		int         proc_chance;
		int         range_proc;
		int         rproc_chance;
		int         defensive_proc;
		int         dproc_chance;
		int         fail_recast;
		int         engaged_no_sp_recast_min;
		int         engaged_no_sp_recast_max;
		int         engaged_b_self_chance;
		int         engaged_b_other_chance;
		int         engaged_d_chance;
		int         pursue_no_sp_recast_min;
		int         pursue_no_sp_recast_max;
		int         pursue_d_chance;
		int         idle_no_sp_recast_min;
		int         idle_no_sp_recast_max;
		int         idle_b_chance;
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
			"parent_list",
			"attack_proc",
			"proc_chance",
			"range_proc",
			"rproc_chance",
			"defensive_proc",
			"dproc_chance",
			"fail_recast",
			"engaged_no_sp_recast_min",
			"engaged_no_sp_recast_max",
			"engaged_b_self_chance",
			"engaged_b_other_chance",
			"engaged_d_chance",
			"pursue_no_sp_recast_min",
			"pursue_no_sp_recast_max",
			"pursue_d_chance",
			"idle_no_sp_recast_min",
			"idle_no_sp_recast_max",
			"idle_b_chance",
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
		return std::string("npc_spells");
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

	static NpcSpells NewEntity()
	{
		NpcSpells entry{};

		entry.id                       = 0;
		entry.name                     = "";
		entry.parent_list              = 0;
		entry.attack_proc              = -1;
		entry.proc_chance              = 3;
		entry.range_proc               = -1;
		entry.rproc_chance             = 0;
		entry.defensive_proc           = -1;
		entry.dproc_chance             = 0;
		entry.fail_recast              = 0;
		entry.engaged_no_sp_recast_min = 0;
		entry.engaged_no_sp_recast_max = 0;
		entry.engaged_b_self_chance    = 0;
		entry.engaged_b_other_chance   = 0;
		entry.engaged_d_chance         = 0;
		entry.pursue_no_sp_recast_min  = 0;
		entry.pursue_no_sp_recast_max  = 0;
		entry.pursue_d_chance          = 0;
		entry.idle_no_sp_recast_min    = 0;
		entry.idle_no_sp_recast_max    = 0;
		entry.idle_b_chance            = 0;

		return entry;
	}

	static NpcSpells GetNpcSpellsEntry(
		const std::vector<NpcSpells> &npc_spellss,
		int npc_spells_id
	)
	{
		for (auto &npc_spells : npc_spellss) {
			if (npc_spells.id == npc_spells_id) {
				return npc_spells;
			}
		}

		return NewEntity();
	}

	static NpcSpells FindOne(
		Database& db,
		int npc_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_spells_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcSpells entry{};

			entry.id                       = atoi(row[0]);
			entry.name                     = row[1] ? row[1] : "";
			entry.parent_list              = atoi(row[2]);
			entry.attack_proc              = atoi(row[3]);
			entry.proc_chance              = atoi(row[4]);
			entry.range_proc               = atoi(row[5]);
			entry.rproc_chance             = atoi(row[6]);
			entry.defensive_proc           = atoi(row[7]);
			entry.dproc_chance             = atoi(row[8]);
			entry.fail_recast              = atoi(row[9]);
			entry.engaged_no_sp_recast_min = atoi(row[10]);
			entry.engaged_no_sp_recast_max = atoi(row[11]);
			entry.engaged_b_self_chance    = atoi(row[12]);
			entry.engaged_b_other_chance   = atoi(row[13]);
			entry.engaged_d_chance         = atoi(row[14]);
			entry.pursue_no_sp_recast_min  = atoi(row[15]);
			entry.pursue_no_sp_recast_max  = atoi(row[16]);
			entry.pursue_d_chance          = atoi(row[17]);
			entry.idle_no_sp_recast_min    = atoi(row[18]);
			entry.idle_no_sp_recast_max    = atoi(row[19]);
			entry.idle_b_chance            = atoi(row[20]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_spells_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_spells_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcSpells npc_spells_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(npc_spells_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(npc_spells_entry.parent_list));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_spells_entry.attack_proc));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_spells_entry.proc_chance));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_spells_entry.range_proc));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_spells_entry.rproc_chance));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_spells_entry.defensive_proc));
		update_values.push_back(columns[8] + " = " + std::to_string(npc_spells_entry.dproc_chance));
		update_values.push_back(columns[9] + " = " + std::to_string(npc_spells_entry.fail_recast));
		update_values.push_back(columns[10] + " = " + std::to_string(npc_spells_entry.engaged_no_sp_recast_min));
		update_values.push_back(columns[11] + " = " + std::to_string(npc_spells_entry.engaged_no_sp_recast_max));
		update_values.push_back(columns[12] + " = " + std::to_string(npc_spells_entry.engaged_b_self_chance));
		update_values.push_back(columns[13] + " = " + std::to_string(npc_spells_entry.engaged_b_other_chance));
		update_values.push_back(columns[14] + " = " + std::to_string(npc_spells_entry.engaged_d_chance));
		update_values.push_back(columns[15] + " = " + std::to_string(npc_spells_entry.pursue_no_sp_recast_min));
		update_values.push_back(columns[16] + " = " + std::to_string(npc_spells_entry.pursue_no_sp_recast_max));
		update_values.push_back(columns[17] + " = " + std::to_string(npc_spells_entry.pursue_d_chance));
		update_values.push_back(columns[18] + " = " + std::to_string(npc_spells_entry.idle_no_sp_recast_min));
		update_values.push_back(columns[19] + " = " + std::to_string(npc_spells_entry.idle_no_sp_recast_max));
		update_values.push_back(columns[20] + " = " + std::to_string(npc_spells_entry.idle_b_chance));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_spells_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcSpells InsertOne(
		Database& db,
		NpcSpells npc_spells_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(npc_spells_entry.name) + "'");
		insert_values.push_back(std::to_string(npc_spells_entry.parent_list));
		insert_values.push_back(std::to_string(npc_spells_entry.attack_proc));
		insert_values.push_back(std::to_string(npc_spells_entry.proc_chance));
		insert_values.push_back(std::to_string(npc_spells_entry.range_proc));
		insert_values.push_back(std::to_string(npc_spells_entry.rproc_chance));
		insert_values.push_back(std::to_string(npc_spells_entry.defensive_proc));
		insert_values.push_back(std::to_string(npc_spells_entry.dproc_chance));
		insert_values.push_back(std::to_string(npc_spells_entry.fail_recast));
		insert_values.push_back(std::to_string(npc_spells_entry.engaged_no_sp_recast_min));
		insert_values.push_back(std::to_string(npc_spells_entry.engaged_no_sp_recast_max));
		insert_values.push_back(std::to_string(npc_spells_entry.engaged_b_self_chance));
		insert_values.push_back(std::to_string(npc_spells_entry.engaged_b_other_chance));
		insert_values.push_back(std::to_string(npc_spells_entry.engaged_d_chance));
		insert_values.push_back(std::to_string(npc_spells_entry.pursue_no_sp_recast_min));
		insert_values.push_back(std::to_string(npc_spells_entry.pursue_no_sp_recast_max));
		insert_values.push_back(std::to_string(npc_spells_entry.pursue_d_chance));
		insert_values.push_back(std::to_string(npc_spells_entry.idle_no_sp_recast_min));
		insert_values.push_back(std::to_string(npc_spells_entry.idle_no_sp_recast_max));
		insert_values.push_back(std::to_string(npc_spells_entry.idle_b_chance));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_spells_entry.id = results.LastInsertedID();
			return npc_spells_entry;
		}

		npc_spells_entry = NewEntity();

		return npc_spells_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcSpells> npc_spells_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_spells_entry: npc_spells_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(npc_spells_entry.name) + "'");
			insert_values.push_back(std::to_string(npc_spells_entry.parent_list));
			insert_values.push_back(std::to_string(npc_spells_entry.attack_proc));
			insert_values.push_back(std::to_string(npc_spells_entry.proc_chance));
			insert_values.push_back(std::to_string(npc_spells_entry.range_proc));
			insert_values.push_back(std::to_string(npc_spells_entry.rproc_chance));
			insert_values.push_back(std::to_string(npc_spells_entry.defensive_proc));
			insert_values.push_back(std::to_string(npc_spells_entry.dproc_chance));
			insert_values.push_back(std::to_string(npc_spells_entry.fail_recast));
			insert_values.push_back(std::to_string(npc_spells_entry.engaged_no_sp_recast_min));
			insert_values.push_back(std::to_string(npc_spells_entry.engaged_no_sp_recast_max));
			insert_values.push_back(std::to_string(npc_spells_entry.engaged_b_self_chance));
			insert_values.push_back(std::to_string(npc_spells_entry.engaged_b_other_chance));
			insert_values.push_back(std::to_string(npc_spells_entry.engaged_d_chance));
			insert_values.push_back(std::to_string(npc_spells_entry.pursue_no_sp_recast_min));
			insert_values.push_back(std::to_string(npc_spells_entry.pursue_no_sp_recast_max));
			insert_values.push_back(std::to_string(npc_spells_entry.pursue_d_chance));
			insert_values.push_back(std::to_string(npc_spells_entry.idle_no_sp_recast_min));
			insert_values.push_back(std::to_string(npc_spells_entry.idle_no_sp_recast_max));
			insert_values.push_back(std::to_string(npc_spells_entry.idle_b_chance));

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

	static std::vector<NpcSpells> All(Database& db)
	{
		std::vector<NpcSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpells entry{};

			entry.id                       = atoi(row[0]);
			entry.name                     = row[1] ? row[1] : "";
			entry.parent_list              = atoi(row[2]);
			entry.attack_proc              = atoi(row[3]);
			entry.proc_chance              = atoi(row[4]);
			entry.range_proc               = atoi(row[5]);
			entry.rproc_chance             = atoi(row[6]);
			entry.defensive_proc           = atoi(row[7]);
			entry.dproc_chance             = atoi(row[8]);
			entry.fail_recast              = atoi(row[9]);
			entry.engaged_no_sp_recast_min = atoi(row[10]);
			entry.engaged_no_sp_recast_max = atoi(row[11]);
			entry.engaged_b_self_chance    = atoi(row[12]);
			entry.engaged_b_other_chance   = atoi(row[13]);
			entry.engaged_d_chance         = atoi(row[14]);
			entry.pursue_no_sp_recast_min  = atoi(row[15]);
			entry.pursue_no_sp_recast_max  = atoi(row[16]);
			entry.pursue_d_chance          = atoi(row[17]);
			entry.idle_no_sp_recast_min    = atoi(row[18]);
			entry.idle_no_sp_recast_max    = atoi(row[19]);
			entry.idle_b_chance            = atoi(row[20]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcSpells> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcSpells> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpells entry{};

			entry.id                       = atoi(row[0]);
			entry.name                     = row[1] ? row[1] : "";
			entry.parent_list              = atoi(row[2]);
			entry.attack_proc              = atoi(row[3]);
			entry.proc_chance              = atoi(row[4]);
			entry.range_proc               = atoi(row[5]);
			entry.rproc_chance             = atoi(row[6]);
			entry.defensive_proc           = atoi(row[7]);
			entry.dproc_chance             = atoi(row[8]);
			entry.fail_recast              = atoi(row[9]);
			entry.engaged_no_sp_recast_min = atoi(row[10]);
			entry.engaged_no_sp_recast_max = atoi(row[11]);
			entry.engaged_b_self_chance    = atoi(row[12]);
			entry.engaged_b_other_chance   = atoi(row[13]);
			entry.engaged_d_chance         = atoi(row[14]);
			entry.pursue_no_sp_recast_min  = atoi(row[15]);
			entry.pursue_no_sp_recast_max  = atoi(row[16]);
			entry.pursue_d_chance          = atoi(row[17]);
			entry.idle_no_sp_recast_min    = atoi(row[18]);
			entry.idle_no_sp_recast_max    = atoi(row[19]);
			entry.idle_b_chance            = atoi(row[20]);

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

#endif //EQEMU_BASE_NPC_SPELLS_REPOSITORY_H
