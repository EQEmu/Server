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
#include "../../strings.h"
#include <ctime>

class BaseTradeskillRecipeRepository {
public:
	struct TradeskillRecipe {
		int32_t     id;
		std::string name;
		int16_t     tradeskill;
		int16_t     skillneeded;
		int16_t     trivial;
		int8_t      nofail;
		int8_t      replace_container;
		std::string notes;
		int8_t      must_learn;
		int8_t      quest;
		int8_t      enabled;
		int8_t      min_expansion;
		int8_t      max_expansion;
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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("tradeskill_recipe");
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

	static TradeskillRecipe NewEntity()
	{
		TradeskillRecipe e{};

		e.id                     = 0;
		e.name                   = "";
		e.tradeskill             = 0;
		e.skillneeded            = 0;
		e.trivial                = 0;
		e.nofail                 = 0;
		e.replace_container      = 0;
		e.notes                  = "";
		e.must_learn             = 0;
		e.quest                  = 0;
		e.enabled                = 1;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static TradeskillRecipe GetTradeskillRecipe(
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
			TradeskillRecipe e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.name                   = row[1] ? row[1] : "";
			e.tradeskill             = static_cast<int16_t>(atoi(row[2]));
			e.skillneeded            = static_cast<int16_t>(atoi(row[3]));
			e.trivial                = static_cast<int16_t>(atoi(row[4]));
			e.nofail                 = static_cast<int8_t>(atoi(row[5]));
			e.replace_container      = static_cast<int8_t>(atoi(row[6]));
			e.notes                  = row[7] ? row[7] : "";
			e.must_learn             = static_cast<int8_t>(atoi(row[8]));
			e.quest                  = static_cast<int8_t>(atoi(row[9]));
			e.enabled                = static_cast<int8_t>(atoi(row[10]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[11]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[12]));
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			return e;
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
		const TradeskillRecipe &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.tradeskill));
		v.push_back(columns[3] + " = " + std::to_string(e.skillneeded));
		v.push_back(columns[4] + " = " + std::to_string(e.trivial));
		v.push_back(columns[5] + " = " + std::to_string(e.nofail));
		v.push_back(columns[6] + " = " + std::to_string(e.replace_container));
		v.push_back(columns[7] + " = '" + Strings::Escape(e.notes) + "'");
		v.push_back(columns[8] + " = " + std::to_string(e.must_learn));
		v.push_back(columns[9] + " = " + std::to_string(e.quest));
		v.push_back(columns[10] + " = " + std::to_string(e.enabled));
		v.push_back(columns[11] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[12] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[13] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[14] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static TradeskillRecipe InsertOne(
		Database& db,
		TradeskillRecipe e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.tradeskill));
		v.push_back(std::to_string(e.skillneeded));
		v.push_back(std::to_string(e.trivial));
		v.push_back(std::to_string(e.nofail));
		v.push_back(std::to_string(e.replace_container));
		v.push_back("'" + Strings::Escape(e.notes) + "'");
		v.push_back(std::to_string(e.must_learn));
		v.push_back(std::to_string(e.quest));
		v.push_back(std::to_string(e.enabled));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
		const std::vector<TradeskillRecipe> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.tradeskill));
			v.push_back(std::to_string(e.skillneeded));
			v.push_back(std::to_string(e.trivial));
			v.push_back(std::to_string(e.nofail));
			v.push_back(std::to_string(e.replace_container));
			v.push_back("'" + Strings::Escape(e.notes) + "'");
			v.push_back(std::to_string(e.must_learn));
			v.push_back(std::to_string(e.quest));
			v.push_back(std::to_string(e.enabled));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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
			TradeskillRecipe e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.name                   = row[1] ? row[1] : "";
			e.tradeskill             = static_cast<int16_t>(atoi(row[2]));
			e.skillneeded            = static_cast<int16_t>(atoi(row[3]));
			e.trivial                = static_cast<int16_t>(atoi(row[4]));
			e.nofail                 = static_cast<int8_t>(atoi(row[5]));
			e.replace_container      = static_cast<int8_t>(atoi(row[6]));
			e.notes                  = row[7] ? row[7] : "";
			e.must_learn             = static_cast<int8_t>(atoi(row[8]));
			e.quest                  = static_cast<int8_t>(atoi(row[9]));
			e.enabled                = static_cast<int8_t>(atoi(row[10]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[11]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[12]));
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<TradeskillRecipe> GetWhere(Database& db, const std::string &where_filter)
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
			TradeskillRecipe e{};

			e.id                     = static_cast<int32_t>(atoi(row[0]));
			e.name                   = row[1] ? row[1] : "";
			e.tradeskill             = static_cast<int16_t>(atoi(row[2]));
			e.skillneeded            = static_cast<int16_t>(atoi(row[3]));
			e.trivial                = static_cast<int16_t>(atoi(row[4]));
			e.nofail                 = static_cast<int8_t>(atoi(row[5]));
			e.replace_container      = static_cast<int8_t>(atoi(row[6]));
			e.notes                  = row[7] ? row[7] : "";
			e.must_learn             = static_cast<int8_t>(atoi(row[8]));
			e.quest                  = static_cast<int8_t>(atoi(row[9]));
			e.enabled                = static_cast<int8_t>(atoi(row[10]));
			e.min_expansion          = static_cast<int8_t>(atoi(row[11]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[12]));
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

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

#endif //EQEMU_BASE_TRADESKILL_RECIPE_REPOSITORY_H
