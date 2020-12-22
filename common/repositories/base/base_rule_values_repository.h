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

#ifndef EQEMU_BASE_RULE_VALUES_REPOSITORY_H
#define EQEMU_BASE_RULE_VALUES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseRuleValuesRepository {
public:
	struct RuleValues {
		int         ruleset_id;
		std::string rule_name;
		std::string rule_value;
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
			"rule_name",
			"rule_value",
			"notes",
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
		return std::string("rule_values");
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

	static RuleValues NewEntity()
	{
		RuleValues entry{};

		entry.ruleset_id = 0;
		entry.rule_name  = "";
		entry.rule_value = "";
		entry.notes      = "";

		return entry;
	}

	static RuleValues GetRuleValuesEntry(
		const std::vector<RuleValues> &rule_valuess,
		int rule_values_id
	)
	{
		for (auto &rule_values : rule_valuess) {
			if (rule_values.ruleset_id == rule_values_id) {
				return rule_values;
			}
		}

		return NewEntity();
	}

	static RuleValues FindOne(
		int rule_values_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				rule_values_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RuleValues entry{};

			entry.ruleset_id = atoi(row[0]);
			entry.rule_name  = row[1] ? row[1] : "";
			entry.rule_value = row[2] ? row[2] : "";
			entry.notes      = row[3] ? row[3] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int rule_values_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				rule_values_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		RuleValues rule_values_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(rule_values_entry.ruleset_id));
		update_values.push_back(columns[1] + " = '" + EscapeString(rule_values_entry.rule_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(rule_values_entry.rule_value) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(rule_values_entry.notes) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				rule_values_entry.ruleset_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RuleValues InsertOne(
		RuleValues rule_values_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(rule_values_entry.ruleset_id));
		insert_values.push_back("'" + EscapeString(rule_values_entry.rule_name) + "'");
		insert_values.push_back("'" + EscapeString(rule_values_entry.rule_value) + "'");
		insert_values.push_back("'" + EscapeString(rule_values_entry.notes) + "'");

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			rule_values_entry.ruleset_id = results.LastInsertedID();
			return rule_values_entry;
		}

		rule_values_entry = NewEntity();

		return rule_values_entry;
	}

	static int InsertMany(
		std::vector<RuleValues> rule_values_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &rule_values_entry: rule_values_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(rule_values_entry.ruleset_id));
			insert_values.push_back("'" + EscapeString(rule_values_entry.rule_name) + "'");
			insert_values.push_back("'" + EscapeString(rule_values_entry.rule_value) + "'");
			insert_values.push_back("'" + EscapeString(rule_values_entry.notes) + "'");

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

	static std::vector<RuleValues> All()
	{
		std::vector<RuleValues> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RuleValues entry{};

			entry.ruleset_id = atoi(row[0]);
			entry.rule_name  = row[1] ? row[1] : "";
			entry.rule_value = row[2] ? row[2] : "";
			entry.notes      = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<RuleValues> GetWhere(std::string where_filter)
	{
		std::vector<RuleValues> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RuleValues entry{};

			entry.ruleset_id = atoi(row[0]);
			entry.rule_name  = row[1] ? row[1] : "";
			entry.rule_value = row[2] ? row[2] : "";
			entry.notes      = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_RULE_VALUES_REPOSITORY_H
