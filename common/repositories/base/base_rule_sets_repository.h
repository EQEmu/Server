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

#ifndef EQEMU_BASE_RULE_SETS_REPOSITORY_H
#define EQEMU_BASE_RULE_SETS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseRuleSetsRepository {
public:
	struct RuleSets {
		int         ruleset_id;
		std::string name;
	};

	static std::string PrimaryKey()
	{
		return std::string("ruleset_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"ruleset_id",
			"name",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("rule_sets");
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

	static RuleSets NewEntity()
	{
		RuleSets entry{};

		entry.ruleset_id = 0;
		entry.name       = "";

		return entry;
	}

	static RuleSets GetRuleSetsEntry(
		const std::vector<RuleSets> &rule_setss,
		int rule_sets_id
	)
	{
		for (auto &rule_sets : rule_setss) {
			if (rule_sets.ruleset_id == rule_sets_id) {
				return rule_sets;
			}
		}

		return NewEntity();
	}

	static RuleSets FindOne(
		Database& db,
		int rule_sets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				rule_sets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RuleSets entry{};

			entry.ruleset_id = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int rule_sets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				rule_sets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		RuleSets rule_sets_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(rule_sets_entry.name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				rule_sets_entry.ruleset_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RuleSets InsertOne(
		Database& db,
		RuleSets rule_sets_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(rule_sets_entry.ruleset_id));
		insert_values.push_back("'" + EscapeString(rule_sets_entry.name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			rule_sets_entry.ruleset_id = results.LastInsertedID();
			return rule_sets_entry;
		}

		rule_sets_entry = NewEntity();

		return rule_sets_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<RuleSets> rule_sets_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &rule_sets_entry: rule_sets_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(rule_sets_entry.ruleset_id));
			insert_values.push_back("'" + EscapeString(rule_sets_entry.name) + "'");

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

	static std::vector<RuleSets> All(Database& db)
	{
		std::vector<RuleSets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RuleSets entry{};

			entry.ruleset_id = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<RuleSets> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<RuleSets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RuleSets entry{};

			entry.ruleset_id = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";

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

#endif //EQEMU_BASE_RULE_SETS_REPOSITORY_H
