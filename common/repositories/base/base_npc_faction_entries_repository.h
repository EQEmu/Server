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
		uint32_t npc_faction_id;
		uint32_t faction_id;
		int32_t  value;
		int8_t   npc_value;
		int8_t   temp;
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

	static NpcFactionEntries GetNpcFactionEntries(
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

			e.npc_faction_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.faction_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.value          = static_cast<int32_t>(atoi(row[2]));
			e.npc_value      = static_cast<int8_t>(atoi(row[3]));
			e.temp           = static_cast<int8_t>(atoi(row[4]));

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
		const NpcFactionEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.npc_faction_id));
		v.push_back(columns[1] + " = " + std::to_string(e.faction_id));
		v.push_back(columns[2] + " = " + std::to_string(e.value));
		v.push_back(columns[3] + " = " + std::to_string(e.npc_value));
		v.push_back(columns[4] + " = " + std::to_string(e.temp));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.npc_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcFactionEntries InsertOne(
		Database& db,
		NpcFactionEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.npc_faction_id));
		v.push_back(std::to_string(e.faction_id));
		v.push_back(std::to_string(e.value));
		v.push_back(std::to_string(e.npc_value));
		v.push_back(std::to_string(e.temp));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.npc_faction_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<NpcFactionEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.npc_faction_id));
			v.push_back(std::to_string(e.faction_id));
			v.push_back(std::to_string(e.value));
			v.push_back(std::to_string(e.npc_value));
			v.push_back(std::to_string(e.temp));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

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

			e.npc_faction_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.faction_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.value          = static_cast<int32_t>(atoi(row[2]));
			e.npc_value      = static_cast<int8_t>(atoi(row[3]));
			e.temp           = static_cast<int8_t>(atoi(row[4]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcFactionEntries> GetWhere(Database& db, const std::string &where_filter)
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

			e.npc_faction_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.faction_id     = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.value          = static_cast<int32_t>(atoi(row[2]));
			e.npc_value      = static_cast<int8_t>(atoi(row[3]));
			e.temp           = static_cast<int8_t>(atoi(row[4]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_NPC_FACTION_ENTRIES_REPOSITORY_H
