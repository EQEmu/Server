/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_RULE_SETS_REPOSITORY_H
#define EQEMU_BASE_RULE_SETS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseRuleSetsRepository {
public:
	struct RuleSets {
		int32_t     ruleset_id;
		std::string name;
		std::string zone_ids;
		std::string instance_versions;
		std::string content_flags;
		std::string content_flags_disabled;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string notes;
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
			"zone_ids",
			"instance_versions",
			"content_flags",
			"content_flags_disabled",
			"min_expansion",
			"max_expansion",
			"notes",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"ruleset_id",
			"name",
			"zone_ids",
			"instance_versions",
			"content_flags",
			"content_flags_disabled",
			"min_expansion",
			"max_expansion",
			"notes",
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
		return std::string("rule_sets");
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

	static RuleSets NewEntity()
	{
		RuleSets e{};

		e.ruleset_id             = 0;
		e.name                   = "";
		e.zone_ids               = "";
		e.instance_versions      = "";
		e.content_flags          = "";
		e.content_flags_disabled = "";
		e.min_expansion          = -2;
		e.max_expansion          = -2;
		e.notes                  = "";

		return e;
	}

	static RuleSets GetRuleSets(
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
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				rule_sets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RuleSets e{};

			e.ruleset_id             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                   = row[1] ? row[1] : "";
			e.zone_ids               = row[2] ? row[2] : "";
			e.instance_versions      = row[3] ? row[3] : "";
			e.content_flags          = row[4] ? row[4] : "";
			e.content_flags_disabled = row[5] ? row[5] : "";
			e.min_expansion          = row[6] ? static_cast<int8_t>(atoi(row[6])) : -2;
			e.max_expansion          = row[7] ? static_cast<int8_t>(atoi(row[7])) : -2;
			e.notes                  = row[8] ? row[8] : "";

			return e;
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
		const RuleSets &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.zone_ids) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.instance_versions) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(columns[6] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[7] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.ruleset_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RuleSets InsertOne(
		Database& db,
		RuleSets e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.ruleset_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.zone_ids) + "'");
		v.push_back("'" + Strings::Escape(e.instance_versions) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.ruleset_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<RuleSets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.ruleset_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.zone_ids) + "'");
			v.push_back("'" + Strings::Escape(e.instance_versions) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.notes) + "'");

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
			RuleSets e{};

			e.ruleset_id             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                   = row[1] ? row[1] : "";
			e.zone_ids               = row[2] ? row[2] : "";
			e.instance_versions      = row[3] ? row[3] : "";
			e.content_flags          = row[4] ? row[4] : "";
			e.content_flags_disabled = row[5] ? row[5] : "";
			e.min_expansion          = row[6] ? static_cast<int8_t>(atoi(row[6])) : -2;
			e.max_expansion          = row[7] ? static_cast<int8_t>(atoi(row[7])) : -2;
			e.notes                  = row[8] ? row[8] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<RuleSets> GetWhere(Database& db, const std::string &where_filter)
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
			RuleSets e{};

			e.ruleset_id             = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
			e.name                   = row[1] ? row[1] : "";
			e.zone_ids               = row[2] ? row[2] : "";
			e.instance_versions      = row[3] ? row[3] : "";
			e.content_flags          = row[4] ? row[4] : "";
			e.content_flags_disabled = row[5] ? row[5] : "";
			e.min_expansion          = row[6] ? static_cast<int8_t>(atoi(row[6])) : -2;
			e.max_expansion          = row[7] ? static_cast<int8_t>(atoi(row[7])) : -2;
			e.notes                  = row[8] ? row[8] : "";

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

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const RuleSets &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.ruleset_id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.zone_ids) + "'");
		v.push_back("'" + Strings::Escape(e.instance_versions) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<RuleSets> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.ruleset_id));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.zone_ids) + "'");
			v.push_back("'" + Strings::Escape(e.instance_versions) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.notes) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_RULE_SETS_REPOSITORY_H
