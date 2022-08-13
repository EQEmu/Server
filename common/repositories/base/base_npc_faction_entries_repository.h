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

#ifndef EQEMU_BASE_NPC_FACTION_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_NPC_FACTION_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseNpcFactionEntriesRepository {
public:
	struct NpcFactionEntries {
		int npc_faction_id;
		int faction_id;
		int value;
		int npc_value;
		int temp;
	};

	static std::string PrimaryKey()
	{
		return std::string("npc_faction_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"npc_faction_id",
			"faction_id",
			"value",
			"npc_value",
			"temp",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"npc_faction_id",
			"faction_id",
			"value",
			"npc_value",
			"temp",
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
		return std::string("npc_faction_entries");
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

	static NpcFactionEntries NewEntity()
	{
		NpcFactionEntries e{};

		e.npc_faction_id = 0;
		e.faction_id     = 0;
		e.value          = 0;
		e.npc_value      = 0;
		e.temp           = 0;

		return e;
	}

	static NpcFactionEntries GetNpcFactionEntriese(
		const std::vector<NpcFactionEntries> &npc_faction_entriess,
		int npc_faction_entries_id
	)
	{
		for (auto &npc_faction_entries : npc_faction_entriess) {
			if (npc_faction_entries.npc_faction_id == npc_faction_entries_id) {
				return npc_faction_entries;
			}
		}

		return NewEntity();
	}

	static NpcFactionEntries FindOne(
		Database& db,
		int npc_faction_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				npc_faction_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			NpcFactionEntries e{};

			e.npc_faction_id = atoi(row[0]);
			e.faction_id     = atoi(row[1]);
			e.value          = atoi(row[2]);
			e.npc_value      = atoi(row[3]);
			e.temp           = atoi(row[4]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int npc_faction_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				npc_faction_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		NpcFactionEntries npc_faction_entries_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(npc_faction_entries_e.npc_faction_id));
		update_values.push_back(columns[1] + " = " + std::to_string(npc_faction_entries_e.faction_id));
		update_values.push_back(columns[2] + " = " + std::to_string(npc_faction_entries_e.value));
		update_values.push_back(columns[3] + " = " + std::to_string(npc_faction_entries_e.npc_value));
		update_values.push_back(columns[4] + " = " + std::to_string(npc_faction_entries_e.temp));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				npc_faction_entries_e.npc_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcFactionEntries InsertOne(
		Database& db,
		NpcFactionEntries npc_faction_entries_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(npc_faction_entries_e.npc_faction_id));
		insert_values.push_back(std::to_string(npc_faction_entries_e.faction_id));
		insert_values.push_back(std::to_string(npc_faction_entries_e.value));
		insert_values.push_back(std::to_string(npc_faction_entries_e.npc_value));
		insert_values.push_back(std::to_string(npc_faction_entries_e.temp));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			npc_faction_entries_e.npc_faction_id = results.LastInsertedID();
			return npc_faction_entries_e;
		}

		npc_faction_entries_e = NewEntity();

		return npc_faction_entries_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<NpcFactionEntries> npc_faction_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &npc_faction_entries_e: npc_faction_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(npc_faction_entries_e.npc_faction_id));
			insert_values.push_back(std::to_string(npc_faction_entries_e.faction_id));
			insert_values.push_back(std::to_string(npc_faction_entries_e.value));
			insert_values.push_back(std::to_string(npc_faction_entries_e.npc_value));
			insert_values.push_back(std::to_string(npc_faction_entries_e.temp));

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

	static std::vector<NpcFactionEntries> All(Database& db)
	{
		std::vector<NpcFactionEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcFactionEntries e{};

			e.npc_faction_id = atoi(row[0]);
			e.faction_id     = atoi(row[1]);
			e.value          = atoi(row[2]);
			e.npc_value      = atoi(row[3]);
			e.temp           = atoi(row[4]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcFactionEntries> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<NpcFactionEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			NpcFactionEntries e{};

			e.npc_faction_id = atoi(row[0]);
			e.faction_id     = atoi(row[1]);
			e.value          = atoi(row[2]);
			e.npc_value      = atoi(row[3]);
			e.temp           = atoi(row[4]);

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

#endif //EQEMU_BASE_NPC_FACTION_ENTRIES_REPOSITORY_H
