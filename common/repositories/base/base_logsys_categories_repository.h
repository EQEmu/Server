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

#ifndef EQEMU_BASE_LOGSYS_CATEGORIES_REPOSITORY_H
#define EQEMU_BASE_LOGSYS_CATEGORIES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseLogsysCategoriesRepository {
public:
	struct LogsysCategories {
		int         log_category_id;
		std::string log_category_description;
		int         log_to_console;
		int         log_to_file;
		int         log_to_gmsay;
		int         log_to_discord;
		int         discord_webhook_id;
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
			"log_to_discord",
			"discord_webhook_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"log_category_id",
			"log_category_description",
			"log_to_console",
			"log_to_file",
			"log_to_gmsay",
			"log_to_discord",
			"discord_webhook_id",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("logsys_categories");
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

	static LogsysCategories NewEntity()
	{
		LogsysCategories entry{};

		entry.log_category_id          = 0;
		entry.log_category_description = "";
		entry.log_to_console           = 0;
		entry.log_to_file              = 0;
		entry.log_to_gmsay             = 0;
		entry.log_to_discord           = 0;
		entry.discord_webhook_id       = 0;

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
		Database& db,
		int logsys_categories_id
	)
	{
		auto results = db.QueryDatabase(
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
			entry.log_to_discord           = atoi(row[5]);
			entry.discord_webhook_id       = atoi(row[6]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int logsys_categories_id
	)
	{
		auto results = db.QueryDatabase(
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
		Database& db,
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
		update_values.push_back(columns[5] + " = " + std::to_string(logsys_categories_entry.log_to_discord));
		update_values.push_back(columns[6] + " = " + std::to_string(logsys_categories_entry.discord_webhook_id));

		auto results = db.QueryDatabase(
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
		Database& db,
		LogsysCategories logsys_categories_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(logsys_categories_entry.log_category_id));
		insert_values.push_back("'" + EscapeString(logsys_categories_entry.log_category_description) + "'");
		insert_values.push_back(std::to_string(logsys_categories_entry.log_to_console));
		insert_values.push_back(std::to_string(logsys_categories_entry.log_to_file));
		insert_values.push_back(std::to_string(logsys_categories_entry.log_to_gmsay));
		insert_values.push_back(std::to_string(logsys_categories_entry.log_to_discord));
		insert_values.push_back(std::to_string(logsys_categories_entry.discord_webhook_id));

		auto results = db.QueryDatabase(
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
		Database& db,
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
			insert_values.push_back(std::to_string(logsys_categories_entry.log_to_discord));
			insert_values.push_back(std::to_string(logsys_categories_entry.discord_webhook_id));

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

	static std::vector<LogsysCategories> All(Database& db)
	{
		std::vector<LogsysCategories> all_entries;

		auto results = db.QueryDatabase(
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
			entry.log_to_discord           = atoi(row[5]);
			entry.discord_webhook_id       = atoi(row[6]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LogsysCategories> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LogsysCategories> all_entries;

		auto results = db.QueryDatabase(
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
			entry.log_to_discord           = atoi(row[5]);
			entry.discord_webhook_id       = atoi(row[6]);

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

#endif //EQEMU_BASE_LOGSYS_CATEGORIES_REPOSITORY_H
