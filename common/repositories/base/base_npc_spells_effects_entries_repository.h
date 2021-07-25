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

#ifndef EQEMU_BASE_NPC_SPELLS_EFFECTS_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_NPC_SPELLS_EFFECTS_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcSpellsEffectsEntriesRepository {
public:
	struct NpcSpellsEffectsEntries {
		int id;
		int npc_spells_effects_id;
		int spell_effect_id;
		int minlevel;
		int maxlevel;
		int se_base;
		int se_limit;
		int se_max;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"npc_spells_effects_id",
			"spell_effect_id",
			"minlevel",
			"maxlevel",
			"se_base",
			"se_limit",
			"se_max",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("npc_spells_effects_entries");
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
			ColumnsRaw()
		);
	}

	static NpcSpellsEffectsEntries NewEntity()
	{
		NpcSpellsEffectsEntries entry{};

		entry.id                    = 0;
		entry.npc_spells_effects_id = 0;
		entry.spell_effect_id       = 0;
		entry.minlevel              = 0;
		entry.maxlevel              = 255;
		entry.se_base               = 0;
		entry.se_limit              = 0;
		entry.se_max                = 0;

		return entry;
	}

	static NpcSpellsEffectsEntries GetNpcSpellsEffectsEntriesEntry(
		const std::vector<NpcSpellsEffectsEntries> &npc_spells_effects_entriess,
		int npc_spells_effects_entries_id
	)
	{
		for (auto &npc_spells_effects_entries : npc_spells_effects_entriess) {
			if (npc_spells_effects_entries.id == npc_spells_effects_entries_id) {
				return npc_spells_effects_entries;
			}
		}

		return NewEntity();
	}

	static NpcSpellsEffectsEntries FindOne(
		Database& db,
		int npc_spells_effects_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_spells_effects_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcSpellsEffectsEntries entry{};

			entry.id                    = atoi(row[0]);
			entry.npc_spells_effects_id = atoi(row[1]);
			entry.spell_effect_id       = atoi(row[2]);
			entry.minlevel              = atoi(row[3]);
			entry.maxlevel              = atoi(row[4]);
			entry.se_base               = atoi(row[5]);
			entry.se_limit              = atoi(row[6]);
			entry.se_max                = atoi(row[7]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_spells_effects_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_spells_effects_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcSpellsEffectsEntries npc_spells_effects_entries_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(npc_spells_effects_entries_entry.npc_spells_effects_id));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_spells_effects_entries_entry.spell_effect_id));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_spells_effects_entries_entry.minlevel));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_spells_effects_entries_entry.maxlevel));
		update_values.push_back(columns[5] + " = " + std::to_string(npc_spells_effects_entries_entry.se_base));
		update_values.push_back(columns[6] + " = " + std::to_string(npc_spells_effects_entries_entry.se_limit));
		update_values.push_back(columns[7] + " = " + std::to_string(npc_spells_effects_entries_entry.se_max));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_spells_effects_entries_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcSpellsEffectsEntries InsertOne(
		Database& db,
		NpcSpellsEffectsEntries npc_spells_effects_entries_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.id));
		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.npc_spells_effects_id));
		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.spell_effect_id));
		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.minlevel));
		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.maxlevel));
		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.se_base));
		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.se_limit));
		insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.se_max));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_spells_effects_entries_entry.id = results.LastInsertedID();
			return npc_spells_effects_entries_entry;
		}

		npc_spells_effects_entries_entry = NewEntity();

		return npc_spells_effects_entries_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcSpellsEffectsEntries> npc_spells_effects_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_spells_effects_entries_entry: npc_spells_effects_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.id));
			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.npc_spells_effects_id));
			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.spell_effect_id));
			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.minlevel));
			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.maxlevel));
			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.se_base));
			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.se_limit));
			insert_values.push_back(std::to_string(npc_spells_effects_entries_entry.se_max));

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

	static std::vector<NpcSpellsEffectsEntries> All(Database& db)
	{
		std::vector<NpcSpellsEffectsEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpellsEffectsEntries entry{};

			entry.id                    = atoi(row[0]);
			entry.npc_spells_effects_id = atoi(row[1]);
			entry.spell_effect_id       = atoi(row[2]);
			entry.minlevel              = atoi(row[3]);
			entry.maxlevel              = atoi(row[4]);
			entry.se_base               = atoi(row[5]);
			entry.se_limit              = atoi(row[6]);
			entry.se_max                = atoi(row[7]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcSpellsEffectsEntries> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcSpellsEffectsEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpellsEffectsEntries entry{};

			entry.id                    = atoi(row[0]);
			entry.npc_spells_effects_id = atoi(row[1]);
			entry.spell_effect_id       = atoi(row[2]);
			entry.minlevel              = atoi(row[3]);
			entry.maxlevel              = atoi(row[4]);
			entry.se_base               = atoi(row[5]);
			entry.se_limit              = atoi(row[6]);
			entry.se_max                = atoi(row[7]);

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

#endif //EQEMU_BASE_NPC_SPELLS_EFFECTS_ENTRIES_REPOSITORY_H
