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

#ifndef EQEMU_BASE_LOGSYS_CATEGORIES_REPOSITORY_H
#define EQEMU_BASE_LOGSYS_CATEGORIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLogsysCategoriesRepository {
public:
	struct LogsysCategories {
		int         log_category_id;
		std::string log_category_description;
		int         log_to_console;
		int         log_to_file;
		int         log_to_gmsay;
	};

	static std::string PrimaryKey()
	{
		return std::string("log_category_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"log_category_id",
			"log_category_description",
			"log_to_console",
			"log_to_file",
			"log_to_gmsay",
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
		return std::string("logsys_categories");
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

	static LogsysCategories NewEntity()
	{
		LogsysCategories entry{};

		entry.log_category_id          = 0;
		entry.log_category_description = "";
		entry.log_to_console           = 0;
		entry.log_to_file              = 0;
		entry.log_to_gmsay             = 0;

		return entry;
	}

	static LogsysCategories GetLogsysCategoriesEntry(
		const std::vector<LogsysCategories> &logsys_categoriess,
		int logsys_categories_id
	)
	{
		for (auto &logsys_categories : logsys_categoriess) {
			if (logsys_categories.log_category_id == logsys_categories_id) {
				return logsys_categories;
			}
		}

		return NewEntity();
	}

	static LogsysCategories FindOne(
		int logsys_categories_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				logsys_categories_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LogsysCategories entry{};

			entry.log_category_id          = atoi(row[0]);
			entry.log_category_description = row[1] ? row[1] : "";
			entry.log_to_console           = atoi(row[2]);
			entry.log_to_file              = atoi(row[3]);
			entry.log_to_gmsay             = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int logsys_categories_id
	)
	{
		auto results = database.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				logsys_categories_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		LogsysCategories logsys_categories_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(logsys_categories_entry.log_category_id));
		update_values.push_back(columns[1] + " = '" + EscapeString(logsys_categories_entry.log_category_description) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(logsys_categories_entry.log_to_console));
		update_values.push_back(columns[3] + " = " + std::to_string(logsys_categories_entry.log_to_file));
		update_values.push_back(columns[4] + " = " + std::to_string(logsys_categories_entry.log_to_gmsay));

		auto results = database.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				logsys_categories_entry.log_category_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LogsysCategories InsertOne(
		LogsysCategories logsys_categories_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(logsys_categories_entry.log_category_id));
		insert_values.push_back("'" + EscapeString(logsys_categories_entry.log_category_description) + "'");
		insert_values.push_back(std::to_string(logsys_categories_entry.log_to_console));
		insert_values.push_back(std::to_string(logsys_categories_entry.log_to_file));
		insert_values.push_back(std::to_string(logsys_categories_entry.log_to_gmsay));

		auto results = database.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			logsys_categories_entry.log_category_id = results.LastInsertedID();
			return logsys_categories_entry;
		}

		logsys_categories_entry = NewEntity();

		return logsys_categories_entry;
	}

	static int InsertMany(
		std::vector<LogsysCategories> logsys_categories_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &logsys_categories_entry: logsys_categories_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(logsys_categories_entry.log_category_id));
			insert_values.push_back("'" + EscapeString(logsys_categories_entry.log_category_description) + "'");
			insert_values.push_back(std::to_string(logsys_categories_entry.log_to_console));
			insert_values.push_back(std::to_string(logsys_categories_entry.log_to_file));
			insert_values.push_back(std::to_string(logsys_categories_entry.log_to_gmsay));

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

	static std::vector<LogsysCategories> All()
	{
		std::vector<LogsysCategories> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LogsysCategories entry{};

			entry.log_category_id          = atoi(row[0]);
			entry.log_category_description = row[1] ? row[1] : "";
			entry.log_to_console           = atoi(row[2]);
			entry.log_to_file              = atoi(row[3]);
			entry.log_to_gmsay             = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LogsysCategories> GetWhere(std::string where_filter)
	{
		std::vector<LogsysCategories> all_entries;

		auto results = database.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LogsysCategories entry{};

			entry.log_category_id          = atoi(row[0]);
			entry.log_category_description = row[1] ? row[1] : "";
			entry.log_to_console           = atoi(row[2]);
			entry.log_to_file              = atoi(row[3]);
			entry.log_to_gmsay             = atoi(row[4]);

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

#endif //EQEMU_BASE_LOGSYS_CATEGORIES_REPOSITORY_H
