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

#ifndef EQEMU_BASE_TRADESKILL_RECIPE_REPOSITORY_H
#define EQEMU_BASE_TRADESKILL_RECIPE_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTradeskillRecipeRepository {
public:
	struct TradeskillRecipe {
		int         id;
		std::string name;
		int         tradeskill;
		int         skillneeded;
		int         trivial;
		int         nofail;
		int         replace_container;
		std::string notes;
		int         must_learn;
		int         quest;
		int         enabled;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
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
			"tradeskill",
			"skillneeded",
			"trivial",
			"nofail",
			"replace_container",
			"notes",
			"must_learn",
			"quest",
			"enabled",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("tradeskill_recipe");
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

	static TradeskillRecipe NewEntity()
	{
		TradeskillRecipe entry{};

		entry.id                     = 0;
		entry.name                   = "";
		entry.tradeskill             = 0;
		entry.skillneeded            = 0;
		entry.trivial                = 0;
		entry.nofail                 = 0;
		entry.replace_container      = 0;
		entry.notes                  = "";
		entry.must_learn             = 0;
		entry.quest                  = 0;
		entry.enabled                = 1;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static TradeskillRecipe GetTradeskillRecipeEntry(
		const std::vector<TradeskillRecipe> &tradeskill_recipes,
		int tradeskill_recipe_id
	)
	{
		for (auto &tradeskill_recipe : tradeskill_recipes) {
			if (tradeskill_recipe.id == tradeskill_recipe_id) {
				return tradeskill_recipe;
			}
		}

		return NewEntity();
	}

	static TradeskillRecipe FindOne(
		Database& db,
		int tradeskill_recipe_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tradeskill_recipe_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TradeskillRecipe entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.tradeskill             = atoi(row[2]);
			entry.skillneeded            = atoi(row[3]);
			entry.trivial                = atoi(row[4]);
			entry.nofail                 = atoi(row[5]);
			entry.replace_container      = atoi(row[6]);
			entry.notes                  = row[7] ? row[7] : "";
			entry.must_learn             = atoi(row[8]);
			entry.quest                  = atoi(row[9]);
			entry.enabled                = atoi(row[10]);
			entry.min_expansion          = atoi(row[11]);
			entry.max_expansion          = atoi(row[12]);
			entry.content_flags          = row[13] ? row[13] : "";
			entry.content_flags_disabled = row[14] ? row[14] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tradeskill_recipe_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tradeskill_recipe_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		TradeskillRecipe tradeskill_recipe_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(tradeskill_recipe_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(tradeskill_recipe_entry.tradeskill));
		update_values.push_back(columns[3] + " = " + std::to_string(tradeskill_recipe_entry.skillneeded));
		update_values.push_back(columns[4] + " = " + std::to_string(tradeskill_recipe_entry.trivial));
		update_values.push_back(columns[5] + " = " + std::to_string(tradeskill_recipe_entry.nofail));
		update_values.push_back(columns[6] + " = " + std::to_string(tradeskill_recipe_entry.replace_container));
		update_values.push_back(columns[7] + " = '" + EscapeString(tradeskill_recipe_entry.notes) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(tradeskill_recipe_entry.must_learn));
		update_values.push_back(columns[9] + " = " + std::to_string(tradeskill_recipe_entry.quest));
		update_values.push_back(columns[10] + " = " + std::to_string(tradeskill_recipe_entry.enabled));
		update_values.push_back(columns[11] + " = " + std::to_string(tradeskill_recipe_entry.min_expansion));
		update_values.push_back(columns[12] + " = " + std::to_string(tradeskill_recipe_entry.max_expansion));
		update_values.push_back(columns[13] + " = '" + EscapeString(tradeskill_recipe_entry.content_flags) + "'");
		update_values.push_back(columns[14] + " = '" + EscapeString(tradeskill_recipe_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				tradeskill_recipe_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TradeskillRecipe InsertOne(
		Database& db,
		TradeskillRecipe tradeskill_recipe_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(tradeskill_recipe_entry.id));
		insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.name) + "'");
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.tradeskill));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.skillneeded));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.trivial));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.nofail));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.replace_container));
		insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.notes) + "'");
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.must_learn));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.quest));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.enabled));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.min_expansion));
		insert_values.push_back(std::to_string(tradeskill_recipe_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tradeskill_recipe_entry.id = results.LastInsertedID();
			return tradeskill_recipe_entry;
		}

		tradeskill_recipe_entry = NewEntity();

		return tradeskill_recipe_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<TradeskillRecipe> tradeskill_recipe_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tradeskill_recipe_entry: tradeskill_recipe_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(tradeskill_recipe_entry.id));
			insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.name) + "'");
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.tradeskill));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.skillneeded));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.trivial));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.nofail));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.replace_container));
			insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.notes) + "'");
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.must_learn));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.quest));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.enabled));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.min_expansion));
			insert_values.push_back(std::to_string(tradeskill_recipe_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(tradeskill_recipe_entry.content_flags_disabled) + "'");

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

	static std::vector<TradeskillRecipe> All(Database& db)
	{
		std::vector<TradeskillRecipe> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TradeskillRecipe entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.tradeskill             = atoi(row[2]);
			entry.skillneeded            = atoi(row[3]);
			entry.trivial                = atoi(row[4]);
			entry.nofail                 = atoi(row[5]);
			entry.replace_container      = atoi(row[6]);
			entry.notes                  = row[7] ? row[7] : "";
			entry.must_learn             = atoi(row[8]);
			entry.quest                  = atoi(row[9]);
			entry.enabled                = atoi(row[10]);
			entry.min_expansion          = atoi(row[11]);
			entry.max_expansion          = atoi(row[12]);
			entry.content_flags          = row[13] ? row[13] : "";
			entry.content_flags_disabled = row[14] ? row[14] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<TradeskillRecipe> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<TradeskillRecipe> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TradeskillRecipe entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.tradeskill             = atoi(row[2]);
			entry.skillneeded            = atoi(row[3]);
			entry.trivial                = atoi(row[4]);
			entry.nofail                 = atoi(row[5]);
			entry.replace_container      = atoi(row[6]);
			entry.notes                  = row[7] ? row[7] : "";
			entry.must_learn             = atoi(row[8]);
			entry.quest                  = atoi(row[9]);
			entry.enabled                = atoi(row[10]);
			entry.min_expansion          = atoi(row[11]);
			entry.max_expansion          = atoi(row[12]);
			entry.content_flags          = row[13] ? row[13] : "";
			entry.content_flags_disabled = row[14] ? row[14] : "";

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

#endif //EQEMU_BASE_TRADESKILL_RECIPE_REPOSITORY_H
