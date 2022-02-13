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

#ifndef EQEMU_BASE_DB_STR_REPOSITORY_H
#define EQEMU_BASE_DB_STR_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseDbStrRepository {
public:
	struct DbStr {
		int         id;
		int         type;
		std::string value;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"type",
			"value",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("db_str");
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

	static DbStr NewEntity()
	{
		DbStr entry{};

		entry.id    = 0;
		entry.type  = 0;
		entry.value = "";

		return entry;
	}

	static DbStr GetDbStrEntry(
		const std::vector<DbStr> &db_strs,
		int db_str_id
	)
	{
		for (auto &db_str : db_strs) {
			if (db_str.id == db_str_id) {
				return db_str;
			}
		}

		return NewEntity();
	}

	static DbStr FindOne(
		Database& db,
		int db_str_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				db_str_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			DbStr entry{};

			entry.id    = atoi(row[0]);
			entry.type  = atoi(row[1]);
			entry.value = row[2] ? row[2] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int db_str_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				db_str_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		DbStr db_str_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(db_str_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(db_str_entry.type));
		update_values.push_back(columns[2] + " = '" + EscapeString(db_str_entry.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				db_str_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static DbStr InsertOne(
		Database& db,
		DbStr db_str_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(db_str_entry.id));
		insert_values.push_back(std::to_string(db_str_entry.type));
		insert_values.push_back("'" + EscapeString(db_str_entry.value) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			db_str_entry.id = results.LastInsertedID();
			return db_str_entry;
		}

		db_str_entry = NewEntity();

		return db_str_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<DbStr> db_str_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &db_str_entry: db_str_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(db_str_entry.id));
			insert_values.push_back(std::to_string(db_str_entry.type));
			insert_values.push_back("'" + EscapeString(db_str_entry.value) + "'");

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

	static std::vector<DbStr> All(Database& db)
	{
		std::vector<DbStr> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DbStr entry{};

			entry.id    = atoi(row[0]);
			entry.type  = atoi(row[1]);
			entry.value = row[2] ? row[2] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<DbStr> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<DbStr> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			DbStr entry{};

			entry.id    = atoi(row[0]);
			entry.type  = atoi(row[1]);
			entry.value = row[2] ? row[2] : "";

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

#endif //EQEMU_BASE_DB_STR_REPOSITORY_H
