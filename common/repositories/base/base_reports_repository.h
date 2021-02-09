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

#ifndef EQEMU_BASE_REPORTS_REPOSITORY_H
#define EQEMU_BASE_REPORTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseReportsRepository {
public:
	struct Reports {
		int         id;
		std::string name;
		std::string reported;
		std::string reported_text;
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
			"reported",
			"reported_text",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("reports");
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

	static Reports NewEntity()
	{
		Reports entry{};

		entry.id            = 0;
		entry.name          = "";
		entry.reported      = "";
		entry.reported_text = "";

		return entry;
	}

	static Reports GetReportsEntry(
		const std::vector<Reports> &reportss,
		int reports_id
	)
	{
		for (auto &reports : reportss) {
			if (reports.id == reports_id) {
				return reports;
			}
		}

		return NewEntity();
	}

	static Reports FindOne(
		Database& db,
		int reports_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				reports_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Reports entry{};

			entry.id            = atoi(row[0]);
			entry.name          = row[1] ? row[1] : "";
			entry.reported      = row[2] ? row[2] : "";
			entry.reported_text = row[3] ? row[3] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int reports_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				reports_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Reports reports_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(reports_entry.name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(reports_entry.reported) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(reports_entry.reported_text) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				reports_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Reports InsertOne(
		Database& db,
		Reports reports_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(reports_entry.id));
		insert_values.push_back("'" + EscapeString(reports_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(reports_entry.reported) + "'");
		insert_values.push_back("'" + EscapeString(reports_entry.reported_text) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			reports_entry.id = results.LastInsertedID();
			return reports_entry;
		}

		reports_entry = NewEntity();

		return reports_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Reports> reports_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &reports_entry: reports_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(reports_entry.id));
			insert_values.push_back("'" + EscapeString(reports_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(reports_entry.reported) + "'");
			insert_values.push_back("'" + EscapeString(reports_entry.reported_text) + "'");

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

	static std::vector<Reports> All(Database& db)
	{
		std::vector<Reports> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Reports entry{};

			entry.id            = atoi(row[0]);
			entry.name          = row[1] ? row[1] : "";
			entry.reported      = row[2] ? row[2] : "";
			entry.reported_text = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Reports> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Reports> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Reports entry{};

			entry.id            = atoi(row[0]);
			entry.name          = row[1] ? row[1] : "";
			entry.reported      = row[2] ? row[2] : "";
			entry.reported_text = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_REPORTS_REPOSITORY_H
