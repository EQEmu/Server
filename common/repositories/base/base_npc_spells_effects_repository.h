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

#ifndef EQEMU_BASE_NPC_SPELLS_EFFECTS_REPOSITORY_H
#define EQEMU_BASE_NPC_SPELLS_EFFECTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseNpcSpellsEffectsRepository {
public:
	struct NpcSpellsEffects {
		int         id;
		std::string name;
		int         parent_list;
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
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("npc_spells_effects");
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

	static NpcSpellsEffects NewEntity()
	{
		NpcSpellsEffects entry{};

		entry.id          = 0;
		entry.name        = "";
		entry.parent_list = 0;

		return entry;
	}

	static NpcSpellsEffects GetNpcSpellsEffectsEntry(
		const std::vector<NpcSpellsEffects> &npc_spells_effectss,
		int npc_spells_effects_id
	)
	{
		for (auto &npc_spells_effects : npc_spells_effectss) {
			if (npc_spells_effects.id == npc_spells_effects_id) {
				return npc_spells_effects;
			}
		}

		return NewEntity();
	}

	static NpcSpellsEffects FindOne(
		Database& db,
		int npc_spells_effects_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_spells_effects_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcSpellsEffects entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.parent_list = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_spells_effects_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_spells_effects_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcSpellsEffects npc_spells_effects_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(npc_spells_effects_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(npc_spells_effects_entry.parent_list));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				npc_spells_effects_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcSpellsEffects InsertOne(
		Database& db,
		NpcSpellsEffects npc_spells_effects_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_spells_effects_entry.id));
		insert_values.push_back("'" + EscapeString(npc_spells_effects_entry.name) + "'");
		insert_values.push_back(std::to_string(npc_spells_effects_entry.parent_list));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_spells_effects_entry.id = results.LastInsertedID();
			return npc_spells_effects_entry;
		}

		npc_spells_effects_entry = NewEntity();

		return npc_spells_effects_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcSpellsEffects> npc_spells_effects_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_spells_effects_entry: npc_spells_effects_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_spells_effects_entry.id));
			insert_values.push_back("'" + EscapeString(npc_spells_effects_entry.name) + "'");
			insert_values.push_back(std::to_string(npc_spells_effects_entry.parent_list));

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

	static std::vector<NpcSpellsEffects> All(Database& db)
	{
		std::vector<NpcSpellsEffects> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpellsEffects entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.parent_list = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<NpcSpellsEffects> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcSpellsEffects> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcSpellsEffects entry{};

			entry.id          = atoi(row[0]);
			entry.name        = row[1] ? row[1] : "";
			entry.parent_list = atoi(row[2]);

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

#endif //EQEMU_BASE_NPC_SPELLS_EFFECTS_REPOSITORY_H
