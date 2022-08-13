/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://eqemu.gitbook.io/server/in-development/developer-area/repositories
 */

#ifndef EQEMU_BASE_NPC_SPELLS_REPOSITORY_H
#define EQEMU_BASE_NPC_SPELLS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("npc_spells");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			SelectColumnsRaw(),
			TableName()
		);
	}

	static std::string BaseInsert()
	{
		return fmt::format(
			"INSERT INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static NpcSpells NewEntity()
	{
		NpcSpells e{};

		e.id                       = 0;
		e.name                     = "";
		e.parent_list              = 0;
		e.attack_proc              = -1;
		e.proc_chance              = 3;
		e.range_proc               = -1;
		e.rproc_chance             = 0;
		e.defensive_proc           = -1;
		e.dproc_chance             = 0;
		e.fail_recast              = 0;
		e.engaged_no_sp_recast_min = 0;
		e.engaged_no_sp_recast_max = 0;
		e.engaged_b_self_chance    = 0;
		e.engaged_b_other_chance   = 0;
		e.engaged_d_chance         = 0;
		e.pursue_no_sp_recast_min  = 0;
		e.pursue_no_sp_recast_max  = 0;
		e.pursue_d_chance          = 0;
		e.idle_no_sp_recast_min    = 0;
		e.idle_no_sp_recast_max    = 0;
		e.idle_b_chance            = 0;

		return e;
	}

	static NpcSpells GetNpcSpellse(
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
			NpcSpells e{};

			e.id                       = atoi(row[0]);
			e.name                     = row[1] ? row[1] : "";
			e.parent_list              = atoi(row[2]);
			e.attack_proc              = atoi(row[3]);
			e.proc_chance              = atoi(row[4]);
			e.range_proc               = atoi(row[5]);
			e.rproc_chance             = atoi(row[6]);
			e.defensive_proc           = atoi(row[7]);
			e.dproc_chance             = atoi(row[8]);
			e.fail_recast              = atoi(row[9]);
			e.engaged_no_sp_recast_min = atoi(row[10]);
			e.engaged_no_sp_recast_max = atoi(row[11]);
			e.engaged_b_self_chance    = atoi(row[12]);
			e.engaged_b_other_chance   = atoi(row[13]);
			e.engaged_d_chance         = atoi(row[14]);
			e.pursue_no_sp_recast_min  = atoi(row[15]);
			e.pursue_no_sp_recast_max  = atoi(row[16]);
			e.pursue_d_chance          = atoi(row[17]);
			e.idle_no_sp_recast_min    = atoi(row[18]);
			e.idle_no_sp_recast_max    = atoi(row[19]);
			e.idle_b_chance            = atoi(row[20]);

			return e;
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
		NpcSpells npc_spells_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + Strings::Escape(npc_spells_e.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(npc_spells_e.parent_list));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_spells_e.attack_proc));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_spells_e.proc_chance));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_spells_e.range_proc));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_spells_e.rproc_chance));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_spells_e.defensive_proc));
		update_values.push_back(columns[8] + " = " + std::to_string(npc_spells_e.dproc_chance));
		update_values.push_back(columns[9] + " = " + std::to_string(npc_spells_e.fail_recast));
		update_values.push_back(columns[10] + " = " + std::to_string(npc_spells_e.engaged_no_sp_recast_min));
		update_values.push_back(columns[11] + " = " + std::to_string(npc_spells_e.engaged_no_sp_recast_max));
		update_values.push_back(columns[12] + " = " + std::to_string(npc_spells_e.engaged_b_self_chance));
		update_values.push_back(columns[13] + " = " + std::to_string(npc_spells_e.engaged_b_other_chance));
		update_values.push_back(columns[14] + " = " + std::to_string(npc_spells_e.engaged_d_chance));
		update_values.push_back(columns[15] + " = " + std::to_string(npc_spells_e.pursue_no_sp_recast_min));
		update_values.push_back(columns[16] + " = " + std::to_string(npc_spells_e.pursue_no_sp_recast_max));
		update_values.push_back(columns[17] + " = " + std::to_string(npc_spells_e.pursue_d_chance));
		update_values.push_back(columns[18] + " = " + std::to_string(npc_spells_e.idle_no_sp_recast_min));
		update_values.push_back(columns[19] + " = " + std::to_string(npc_spells_e.idle_no_sp_recast_max));
		update_values.push_back(columns[20] + " = " + std::to_string(npc_spells_e.idle_b_chance));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				npc_spells_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcSpells InsertOne(
		Database& db,
		NpcSpells npc_spells_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_spells_e.id));
		insert_values.push_back("'" + Strings::Escape(npc_spells_e.name) + "'");
		insert_values.push_back(std::to_string(npc_spells_e.parent_list));
		insert_values.push_back(std::to_string(npc_spells_e.attack_proc));
		insert_values.push_back(std::to_string(npc_spells_e.proc_chance));
		insert_values.push_back(std::to_string(npc_spells_e.range_proc));
		insert_values.push_back(std::to_string(npc_spells_e.rproc_chance));
		insert_values.push_back(std::to_string(npc_spells_e.defensive_proc));
		insert_values.push_back(std::to_string(npc_spells_e.dproc_chance));
		insert_values.push_back(std::to_string(npc_spells_e.fail_recast));
		insert_values.push_back(std::to_string(npc_spells_e.engaged_no_sp_recast_min));
		insert_values.push_back(std::to_string(npc_spells_e.engaged_no_sp_recast_max));
		insert_values.push_back(std::to_string(npc_spells_e.engaged_b_self_chance));
		insert_values.push_back(std::to_string(npc_spells_e.engaged_b_other_chance));
		insert_values.push_back(std::to_string(npc_spells_e.engaged_d_chance));
		insert_values.push_back(std::to_string(npc_spells_e.pursue_no_sp_recast_min));
		insert_values.push_back(std::to_string(npc_spells_e.pursue_no_sp_recast_max));
		insert_values.push_back(std::to_string(npc_spells_e.pursue_d_chance));
		insert_values.push_back(std::to_string(npc_spells_e.idle_no_sp_recast_min));
		insert_values.push_back(std::to_string(npc_spells_e.idle_no_sp_recast_max));
		insert_values.push_back(std::to_string(npc_spells_e.idle_b_chance));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_spells_e.id = results.LastInsertedID();
			return npc_spells_e;
		}

		npc_spells_e = NewEntity();

		return npc_spells_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcSpells> npc_spells_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_spells_e: npc_spells_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_spells_e.id));
			insert_values.push_back("'" + Strings::Escape(npc_spells_e.name) + "'");
			insert_values.push_back(std::to_string(npc_spells_e.parent_list));
			insert_values.push_back(std::to_string(npc_spells_e.attack_proc));
			insert_values.push_back(std::to_string(npc_spells_e.proc_chance));
			insert_values.push_back(std::to_string(npc_spells_e.range_proc));
			insert_values.push_back(std::to_string(npc_spells_e.rproc_chance));
			insert_values.push_back(std::to_string(npc_spells_e.defensive_proc));
			insert_values.push_back(std::to_string(npc_spells_e.dproc_chance));
			insert_values.push_back(std::to_string(npc_spells_e.fail_recast));
			insert_values.push_back(std::to_string(npc_spells_e.engaged_no_sp_recast_min));
			insert_values.push_back(std::to_string(npc_spells_e.engaged_no_sp_recast_max));
			insert_values.push_back(std::to_string(npc_spells_e.engaged_b_self_chance));
			insert_values.push_back(std::to_string(npc_spells_e.engaged_b_other_chance));
			insert_values.push_back(std::to_string(npc_spells_e.engaged_d_chance));
			insert_values.push_back(std::to_string(npc_spells_e.pursue_no_sp_recast_min));
			insert_values.push_back(std::to_string(npc_spells_e.pursue_no_sp_recast_max));
			insert_values.push_back(std::to_string(npc_spells_e.pursue_d_chance));
			insert_values.push_back(std::to_string(npc_spells_e.idle_no_sp_recast_min));
			insert_values.push_back(std::to_string(npc_spells_e.idle_no_sp_recast_max));
			insert_values.push_back(std::to_string(npc_spells_e.idle_b_chance));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			NpcSpells e{};

			e.id                       = atoi(row[0]);
			e.name                     = row[1] ? row[1] : "";
			e.parent_list              = atoi(row[2]);
			e.attack_proc              = atoi(row[3]);
			e.proc_chance              = atoi(row[4]);
			e.range_proc               = atoi(row[5]);
			e.rproc_chance             = atoi(row[6]);
			e.defensive_proc           = atoi(row[7]);
			e.dproc_chance             = atoi(row[8]);
			e.fail_recast              = atoi(row[9]);
			e.engaged_no_sp_recast_min = atoi(row[10]);
			e.engaged_no_sp_recast_max = atoi(row[11]);
			e.engaged_b_self_chance    = atoi(row[12]);
			e.engaged_b_other_chance   = atoi(row[13]);
			e.engaged_d_chance         = atoi(row[14]);
			e.pursue_no_sp_recast_min  = atoi(row[15]);
			e.pursue_no_sp_recast_max  = atoi(row[16]);
			e.pursue_d_chance          = atoi(row[17]);
			e.idle_no_sp_recast_min    = atoi(row[18]);
			e.idle_no_sp_recast_max    = atoi(row[19]);
			e.idle_b_chance            = atoi(row[20]);

			all_entries.push_back(e);
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
			NpcSpells e{};

			e.id                       = atoi(row[0]);
			e.name                     = row[1] ? row[1] : "";
			e.parent_list              = atoi(row[2]);
			e.attack_proc              = atoi(row[3]);
			e.proc_chance              = atoi(row[4]);
			e.range_proc               = atoi(row[5]);
			e.rproc_chance             = atoi(row[6]);
			e.defensive_proc           = atoi(row[7]);
			e.dproc_chance             = atoi(row[8]);
			e.fail_recast              = atoi(row[9]);
			e.engaged_no_sp_recast_min = atoi(row[10]);
			e.engaged_no_sp_recast_max = atoi(row[11]);
			e.engaged_b_self_chance    = atoi(row[12]);
			e.engaged_b_other_chance   = atoi(row[13]);
			e.engaged_d_chance         = atoi(row[14]);
			e.pursue_no_sp_recast_min  = atoi(row[15]);
			e.pursue_no_sp_recast_max  = atoi(row[16]);
			e.pursue_d_chance          = atoi(row[17]);
			e.idle_no_sp_recast_min    = atoi(row[18]);
			e.idle_no_sp_recast_max    = atoi(row[19]);
			e.idle_b_chance            = atoi(row[20]);

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_NPC_SPELLS_REPOSITORY_H
