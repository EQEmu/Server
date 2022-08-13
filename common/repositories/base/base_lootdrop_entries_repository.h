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

#ifndef EQEMU_BASE_LOOTDROP_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_LOOTDROP_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseLootdropEntriesRepository {
public:
	struct LootdropEntries {
		int   lootdrop_id;
		int   item_id;
		int   item_charges;
		int   equip_item;
		float chance;
		float disabled_chance;
		int   trivial_min_level;
		int   trivial_max_level;
		int   multiplier;
		int   npc_min_level;
		int   npc_max_level;
	};

	static std::string PrimaryKey()
	{
		return std::string("lootdrop_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"lootdrop_id",
			"item_id",
			"item_charges",
			"equip_item",
			"chance",
			"disabled_chance",
			"trivial_min_level",
			"trivial_max_level",
			"multiplier",
			"npc_min_level",
			"npc_max_level",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"lootdrop_id",
			"item_id",
			"item_charges",
			"equip_item",
			"chance",
			"disabled_chance",
			"trivial_min_level",
			"trivial_max_level",
			"multiplier",
			"npc_min_level",
			"npc_max_level",
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
		return std::string("lootdrop_entries");
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

	static LootdropEntries NewEntity()
	{
		LootdropEntries e{};

		e.lootdrop_id       = 0;
		e.item_id           = 0;
		e.item_charges      = 1;
		e.equip_item        = 0;
		e.chance            = 1;
		e.disabled_chance   = 0;
		e.trivial_min_level = 0;
		e.trivial_max_level = 0;
		e.multiplier        = 1;
		e.npc_min_level     = 0;
		e.npc_max_level     = 0;

		return e;
	}

	static LootdropEntries GetLootdropEntries(
		const std::vector<LootdropEntries> &lootdrop_entriess,
		int lootdrop_entries_id
	)
	{
		for (auto &lootdrop_entries : lootdrop_entriess) {
			if (lootdrop_entries.lootdrop_id == lootdrop_entries_id) {
				return lootdrop_entries;
			}
		}

		return NewEntity();
	}

	static LootdropEntries FindOne(
		Database& db,
		int lootdrop_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				lootdrop_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LootdropEntries e{};

			e.lootdrop_id       = atoi(row[0]);
			e.item_id           = atoi(row[1]);
			e.item_charges      = atoi(row[2]);
			e.equip_item        = atoi(row[3]);
			e.chance            = static_cast<float>(atof(row[4]));
			e.disabled_chance   = static_cast<float>(atof(row[5]));
			e.trivial_min_level = atoi(row[6]);
			e.trivial_max_level = atoi(row[7]);
			e.multiplier        = atoi(row[8]);
			e.npc_min_level     = atoi(row[9]);
			e.npc_max_level     = atoi(row[10]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int lootdrop_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				lootdrop_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const LootdropEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.lootdrop_id));
		v.push_back(columns[1] + " = " + std::to_string(e.item_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_charges));
		v.push_back(columns[3] + " = " + std::to_string(e.equip_item));
		v.push_back(columns[4] + " = " + std::to_string(e.chance));
		v.push_back(columns[5] + " = " + std::to_string(e.disabled_chance));
		v.push_back(columns[6] + " = " + std::to_string(e.trivial_min_level));
		v.push_back(columns[7] + " = " + std::to_string(e.trivial_max_level));
		v.push_back(columns[8] + " = " + std::to_string(e.multiplier));
		v.push_back(columns[9] + " = " + std::to_string(e.npc_min_level));
		v.push_back(columns[10] + " = " + std::to_string(e.npc_max_level));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.lootdrop_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LootdropEntries InsertOne(
		Database& db,
		LootdropEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.lootdrop_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.item_charges));
		v.push_back(std::to_string(e.equip_item));
		v.push_back(std::to_string(e.chance));
		v.push_back(std::to_string(e.disabled_chance));
		v.push_back(std::to_string(e.trivial_min_level));
		v.push_back(std::to_string(e.trivial_max_level));
		v.push_back(std::to_string(e.multiplier));
		v.push_back(std::to_string(e.npc_min_level));
		v.push_back(std::to_string(e.npc_max_level));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.lootdrop_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<LootdropEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.lootdrop_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.item_charges));
			v.push_back(std::to_string(e.equip_item));
			v.push_back(std::to_string(e.chance));
			v.push_back(std::to_string(e.disabled_chance));
			v.push_back(std::to_string(e.trivial_min_level));
			v.push_back(std::to_string(e.trivial_max_level));
			v.push_back(std::to_string(e.multiplier));
			v.push_back(std::to_string(e.npc_min_level));
			v.push_back(std::to_string(e.npc_max_level));

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

	static std::vector<LootdropEntries> All(Database& db)
	{
		std::vector<LootdropEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LootdropEntries e{};

			e.lootdrop_id       = atoi(row[0]);
			e.item_id           = atoi(row[1]);
			e.item_charges      = atoi(row[2]);
			e.equip_item        = atoi(row[3]);
			e.chance            = static_cast<float>(atof(row[4]));
			e.disabled_chance   = static_cast<float>(atof(row[5]));
			e.trivial_min_level = atoi(row[6]);
			e.trivial_max_level = atoi(row[7]);
			e.multiplier        = atoi(row[8]);
			e.npc_min_level     = atoi(row[9]);
			e.npc_max_level     = atoi(row[10]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LootdropEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<LootdropEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LootdropEntries e{};

			e.lootdrop_id       = atoi(row[0]);
			e.item_id           = atoi(row[1]);
			e.item_charges      = atoi(row[2]);
			e.equip_item        = atoi(row[3]);
			e.chance            = static_cast<float>(atof(row[4]));
			e.disabled_chance   = static_cast<float>(atof(row[5]));
			e.trivial_min_level = atoi(row[6]);
			e.trivial_max_level = atoi(row[7]);
			e.multiplier        = atoi(row[8]);
			e.npc_min_level     = atoi(row[9]);
			e.npc_max_level     = atoi(row[10]);

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

#endif //EQEMU_BASE_LOOTDROP_ENTRIES_REPOSITORY_H
