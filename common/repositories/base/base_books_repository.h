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

#ifndef EQEMU_BASE_BOOKS_REPOSITORY_H
#define EQEMU_BASE_BOOKS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseBooksRepository {
public:
	struct Books {
		int         id;
		std::string name;
		std::string txtfile;
		int         language;
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
			"txtfile",
			"language",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"txtfile",
			"language",
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
		return std::string("books");
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

	static Books NewEntity()
	{
		Books entry{};

		entry.id       = 0;
		entry.name     = "";
		entry.txtfile  = "";
		entry.language = 0;

		return entry;
	}

	static Books GetBooksEntry(
		const std::vector<Books> &bookss,
		int books_id
	)
	{
		for (auto &books : bookss) {
			if (books.id == books_id) {
				return books;
			}
		}

		return NewEntity();
	}

	static Books FindOne(
		Database& db,
		int books_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				books_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Books entry{};

			entry.id       = atoi(row[0]);
			entry.name     = row[1] ? row[1] : "";
			entry.txtfile  = row[2] ? row[2] : "";
			entry.language = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int books_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				books_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Books books_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(books_entry.name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(books_entry.txtfile) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(books_entry.language));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				books_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Books InsertOne(
		Database& db,
		Books books_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(books_entry.id));
		insert_values.push_back("'" + EscapeString(books_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(books_entry.txtfile) + "'");
		insert_values.push_back(std::to_string(books_entry.language));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			books_entry.id = results.LastInsertedID();
			return books_entry;
		}

		books_entry = NewEntity();

		return books_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Books> books_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &books_entry: books_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(books_entry.id));
			insert_values.push_back("'" + EscapeString(books_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(books_entry.txtfile) + "'");
			insert_values.push_back(std::to_string(books_entry.language));

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

	static std::vector<Books> All(Database& db)
	{
		std::vector<Books> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Books entry{};

			entry.id       = atoi(row[0]);
			entry.name     = row[1] ? row[1] : "";
			entry.txtfile  = row[2] ? row[2] : "";
			entry.language = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Books> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Books> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Books entry{};

			entry.id       = atoi(row[0]);
			entry.name     = row[1] ? row[1] : "";
			entry.txtfile  = row[2] ? row[2] : "";
			entry.language = atoi(row[3]);

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

#endif //EQEMU_BASE_BOOKS_REPOSITORY_H
