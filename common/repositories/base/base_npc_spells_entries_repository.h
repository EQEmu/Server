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

#ifndef EQEMU_BASE_NPC_SPELLS_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_NPC_SPELLS_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseNpcSpellsEntriesRepository {
public:
	struct NpcSpellsEntries {
		uint32_t id;
		int32_t  npc_spells_id;
		uint16_t spellid;
		uint32_t type;
		uint8_t  minlevel;
		uint8_t  maxlevel;
		int16_t  manacost;
		int32_t  recast_delay;
		int16_t  priority;
		int32_t  resist_adjust;
		int16_t  min_hp;
		int16_t  max_hp;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("npc_spells_entries");
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

	static NpcSpellsEntries NewEntity()
	{
		NpcSpellsEntries e{};

		e.id            = 0;
		e.npc_spells_id = 0;
		e.spellid       = 0;
		e.type          = 0;
		e.minlevel      = 0;
		e.maxlevel      = 255;
		e.manacost      = -1;
		e.recast_delay  = -1;
		e.priority      = 0;
		e.resist_adjust = 0;
		e.min_hp        = 0;
		e.max_hp        = 0;

		return e;
	}

	static NpcSpellsEntries GetNpcSpellsEntries(
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
			NpcSpellsEntries e{};

			e.id            = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.npc_spells_id = static_cast<int32_t>(atoi(row[1]));
			e.spellid       = static_cast<uint16_t>(strtoul(row[2], nullptr, 10));
			e.type          = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.minlevel      = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.maxlevel      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.manacost      = static_cast<int16_t>(atoi(row[6]));
			e.recast_delay  = static_cast<int32_t>(atoi(row[7]));
			e.priority      = static_cast<int16_t>(atoi(row[8]));
			e.resist_adjust = static_cast<int32_t>(atoi(row[9]));
			e.min_hp        = static_cast<int16_t>(atoi(row[10]));
			e.max_hp        = static_cast<int16_t>(atoi(row[11]));

			return e;
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
		const NpcSpellsEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.npc_spells_id));
		v.push_back(columns[2] + " = " + std::to_string(e.spellid));
		v.push_back(columns[3] + " = " + std::to_string(e.type));
		v.push_back(columns[4] + " = " + std::to_string(e.minlevel));
		v.push_back(columns[5] + " = " + std::to_string(e.maxlevel));
		v.push_back(columns[6] + " = " + std::to_string(e.manacost));
		v.push_back(columns[7] + " = " + std::to_string(e.recast_delay));
		v.push_back(columns[8] + " = " + std::to_string(e.priority));
		v.push_back(columns[9] + " = " + std::to_string(e.resist_adjust));
		v.push_back(columns[10] + " = " + std::to_string(e.min_hp));
		v.push_back(columns[11] + " = " + std::to_string(e.max_hp));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static NpcSpellsEntries InsertOne(
		Database& db,
		NpcSpellsEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.npc_spells_id));
		v.push_back(std::to_string(e.spellid));
		v.push_back(std::to_string(e.type));
		v.push_back(std::to_string(e.minlevel));
		v.push_back(std::to_string(e.maxlevel));
		v.push_back(std::to_string(e.manacost));
		v.push_back(std::to_string(e.recast_delay));
		v.push_back(std::to_string(e.priority));
		v.push_back(std::to_string(e.resist_adjust));
		v.push_back(std::to_string(e.min_hp));
		v.push_back(std::to_string(e.max_hp));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<NpcSpellsEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.npc_spells_id));
			v.push_back(std::to_string(e.spellid));
			v.push_back(std::to_string(e.type));
			v.push_back(std::to_string(e.minlevel));
			v.push_back(std::to_string(e.maxlevel));
			v.push_back(std::to_string(e.manacost));
			v.push_back(std::to_string(e.recast_delay));
			v.push_back(std::to_string(e.priority));
			v.push_back(std::to_string(e.resist_adjust));
			v.push_back(std::to_string(e.min_hp));
			v.push_back(std::to_string(e.max_hp));

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
			NpcSpellsEntries e{};

			e.id            = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.npc_spells_id = static_cast<int32_t>(atoi(row[1]));
			e.spellid       = static_cast<uint16_t>(strtoul(row[2], nullptr, 10));
			e.type          = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.minlevel      = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.maxlevel      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.manacost      = static_cast<int16_t>(atoi(row[6]));
			e.recast_delay  = static_cast<int32_t>(atoi(row[7]));
			e.priority      = static_cast<int16_t>(atoi(row[8]));
			e.resist_adjust = static_cast<int32_t>(atoi(row[9]));
			e.min_hp        = static_cast<int16_t>(atoi(row[10]));
			e.max_hp        = static_cast<int16_t>(atoi(row[11]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<NpcSpellsEntries> GetWhere(Database& db, const std::string &where_filter)
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
			NpcSpellsEntries e{};

			e.id            = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.npc_spells_id = static_cast<int32_t>(atoi(row[1]));
			e.spellid       = static_cast<uint16_t>(strtoul(row[2], nullptr, 10));
			e.type          = static_cast<uint32_t>(strtoul(row[3], nullptr, 10));
			e.minlevel      = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.maxlevel      = static_cast<uint8_t>(strtoul(row[5], nullptr, 10));
			e.manacost      = static_cast<int16_t>(atoi(row[6]));
			e.recast_delay  = static_cast<int32_t>(atoi(row[7]));
			e.priority      = static_cast<int16_t>(atoi(row[8]));
			e.resist_adjust = static_cast<int32_t>(atoi(row[9]));
			e.min_hp        = static_cast<int16_t>(atoi(row[10]));
			e.max_hp        = static_cast<int16_t>(atoi(row[11]));

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

#endif //EQEMU_BASE_NPC_SPELLS_ENTRIES_REPOSITORY_H
