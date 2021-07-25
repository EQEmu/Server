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
			ColumnsRaw()
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
		Database& db,
		int npc_faction_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
		int npc_faction_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
		NpcFaction npc_faction_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(npc_faction_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(npc_faction_entry.primaryfaction));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_faction_entry.ignore_primary_assist));

		auto results = db.QueryDatabase(
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
		Database& db,
		NpcFaction npc_faction_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_faction_entry.id));
		insert_values.push_back("'" + EscapeString(npc_faction_entry.name) + "'");
		insert_values.push_back(std::to_string(npc_faction_entry.primaryfaction));
		insert_values.push_back(std::to_string(npc_faction_entry.ignore_primary_assist));

		auto results = db.QueryDatabase(
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
		Database& db,
		std::vector<NpcFaction> npc_faction_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_faction_entry: npc_faction_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_faction_entry.id));
			insert_values.push_back("'" + EscapeString(npc_faction_entry.name) + "'");
			insert_values.push_back(std::to_string(npc_faction_entry.primaryfaction));
			insert_values.push_back(std::to_string(npc_faction_entry.ignore_primary_assist));

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

	static std::vector<NpcFaction> All(Database& db)
	{
		std::vector<NpcFaction> all_entries;

		auto results = db.QueryDatabase(
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

	static std::vector<NpcFaction> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcFaction> all_entries;

		auto results = db.QueryDatabase(
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

#endif //EQEMU_BASE_NPC_FACTION_REPOSITORY_H
