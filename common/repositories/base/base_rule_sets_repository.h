/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 */

/**
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to
 * the repository extending the base. Any modifications to base repositories are to
 * be made by the generator only
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

	static std::string InsertColumnsRaw()
	{
		std::vector<std::string> insert_columns;

		for (auto &column : Columns()) {
			if (column == PrimaryKey()) {
				continue;
			}

			insert_columns.push_back(column);
		}

		return std::string(implode(", ", insert_columns));
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
			InsertColumnsRaw()
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
		int rule_sets_id
	)
	{
		auto results = database.QueryDatabase(
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
		int rule_sets_id
	)
	{
		auto results = database.QueryDatabase(
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
		RuleSets rule_sets_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(rule_sets_entry.name) + "'");

		auto results = database.QueryDatabase(
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
		RuleSets rule_sets_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(rule_sets_entry.name) + "'");

		auto results = database.QueryDatabase(
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
		std::vector<RuleSets> rule_sets_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &rule_sets_entry: rule_sets_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(rule_sets_entry.name) + "'");

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<RuleSets> All()
	{
		std::vector<RuleSets> all_entries;

		auto results = database.QueryDatabase(
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

	static std::vector<RuleSets> GetWhere(std::string where_filter)
	{
		std::vector<RuleSets> all_entries;

		auto results = database.QueryDatabase(
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

	static int DeleteWhere(std::string where_filter)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int Truncate()
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"TRUNCATE TABLE {}",
				TableName()
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BASE_RULE_SETS_REPOSITORY_H
