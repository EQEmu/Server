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

#ifndef EQEMU_BASE_TRADESKILL_RECIPE_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_TRADESKILL_RECIPE_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseTradeskillRecipeEntriesRepository {
public:
	struct TradeskillRecipeEntries {
		int32_t id;
		int32_t recipe_id;
		int32_t item_id;
		int8_t  successcount;
		int8_t  failcount;
		int8_t  componentcount;
		int8_t  salvagecount;
		int8_t  iscontainer;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"recipe_id",
			"item_id",
			"successcount",
			"failcount",
			"componentcount",
			"salvagecount",
			"iscontainer",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"recipe_id",
			"item_id",
			"successcount",
			"failcount",
			"componentcount",
			"salvagecount",
			"iscontainer",
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
		return std::string("tradeskill_recipe_entries");
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

	static TradeskillRecipeEntries NewEntity()
	{
		TradeskillRecipeEntries e{};

		e.id             = 0;
		e.recipe_id      = 0;
		e.item_id        = 0;
		e.successcount   = 0;
		e.failcount      = 0;
		e.componentcount = 1;
		e.salvagecount   = 0;
		e.iscontainer    = 0;

		return e;
	}

	static TradeskillRecipeEntries GetTradeskillRecipeEntries(
		const std::vector<TradeskillRecipeEntries> &tradeskill_recipe_entriess,
		int tradeskill_recipe_entries_id
	)
	{
		for (auto &tradeskill_recipe_entries : tradeskill_recipe_entriess) {
			if (tradeskill_recipe_entries.id == tradeskill_recipe_entries_id) {
				return tradeskill_recipe_entries;
			}
		}

		return NewEntity();
	}

	static TradeskillRecipeEntries FindOne(
		Database& db,
		int tradeskill_recipe_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tradeskill_recipe_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TradeskillRecipeEntries e{};

			e.id             = static_cast<int32_t>(atoi(row[0]));
			e.recipe_id      = static_cast<int32_t>(atoi(row[1]));
			e.item_id        = static_cast<int32_t>(atoi(row[2]));
			e.successcount   = static_cast<int8_t>(atoi(row[3]));
			e.failcount      = static_cast<int8_t>(atoi(row[4]));
			e.componentcount = static_cast<int8_t>(atoi(row[5]));
			e.salvagecount   = static_cast<int8_t>(atoi(row[6]));
			e.iscontainer    = static_cast<int8_t>(atoi(row[7]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tradeskill_recipe_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tradeskill_recipe_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const TradeskillRecipeEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.recipe_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.successcount));
		v.push_back(columns[4] + " = " + std::to_string(e.failcount));
		v.push_back(columns[5] + " = " + std::to_string(e.componentcount));
		v.push_back(columns[6] + " = " + std::to_string(e.salvagecount));
		v.push_back(columns[7] + " = " + std::to_string(e.iscontainer));

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

	static TradeskillRecipeEntries InsertOne(
		Database& db,
		TradeskillRecipeEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.recipe_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.successcount));
		v.push_back(std::to_string(e.failcount));
		v.push_back(std::to_string(e.componentcount));
		v.push_back(std::to_string(e.salvagecount));
		v.push_back(std::to_string(e.iscontainer));

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
		const std::vector<TradeskillRecipeEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.recipe_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.successcount));
			v.push_back(std::to_string(e.failcount));
			v.push_back(std::to_string(e.componentcount));
			v.push_back(std::to_string(e.salvagecount));
			v.push_back(std::to_string(e.iscontainer));

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

	static std::vector<TradeskillRecipeEntries> All(Database& db)
	{
		std::vector<TradeskillRecipeEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TradeskillRecipeEntries e{};

			e.id             = static_cast<int32_t>(atoi(row[0]));
			e.recipe_id      = static_cast<int32_t>(atoi(row[1]));
			e.item_id        = static_cast<int32_t>(atoi(row[2]));
			e.successcount   = static_cast<int8_t>(atoi(row[3]));
			e.failcount      = static_cast<int8_t>(atoi(row[4]));
			e.componentcount = static_cast<int8_t>(atoi(row[5]));
			e.salvagecount   = static_cast<int8_t>(atoi(row[6]));
			e.iscontainer    = static_cast<int8_t>(atoi(row[7]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<TradeskillRecipeEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<TradeskillRecipeEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TradeskillRecipeEntries e{};

			e.id             = static_cast<int32_t>(atoi(row[0]));
			e.recipe_id      = static_cast<int32_t>(atoi(row[1]));
			e.item_id        = static_cast<int32_t>(atoi(row[2]));
			e.successcount   = static_cast<int8_t>(atoi(row[3]));
			e.failcount      = static_cast<int8_t>(atoi(row[4]));
			e.componentcount = static_cast<int8_t>(atoi(row[5]));
			e.salvagecount   = static_cast<int8_t>(atoi(row[6]));
			e.iscontainer    = static_cast<int8_t>(atoi(row[7]));

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

#endif //EQEMU_BASE_TRADESKILL_RECIPE_ENTRIES_REPOSITORY_H
