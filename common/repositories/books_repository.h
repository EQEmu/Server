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
 */

#ifndef EQEMU_BOOKS_REPOSITORY_H
#define EQEMU_BOOKS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class BooksRepository {
public:
	struct Books {
		std::string name;
		std::string txtfile;
		int         language;
	};

	static std::string PrimaryKey()
	{
		return std::string("name");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"name",
			"txtfile",
			"language",
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
		return std::string("books");
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

	static Books NewEntity()
	{
		Books entry{};

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
			if (books.name == books_id) {
				return books;
			}
		}

		return NewEntity();
	}

	static Books FindOne(
		int books_id
	)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				books_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Books entry{};

			entry.name     = row[0];
			entry.txtfile  = row[1];
			entry.language = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		int books_id
	)
	{
		auto results = content_db.QueryDatabase(
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
		Books books_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(books_entry.txtfile) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(books_entry.language));

		auto results = content_db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				books_entry.name
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Books InsertOne(
		Books books_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back("'" + EscapeString(books_entry.txtfile) + "'");
		insert_values.push_back(std::to_string(books_entry.language));

		auto results = content_db.QueryDatabase(
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

		books_entry = BooksRepository::NewEntity();

		return books_entry;
	}

	static int InsertMany(
		std::vector<Books> books_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &books_entry: books_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back("'" + EscapeString(books_entry.txtfile) + "'");
			insert_values.push_back(std::to_string(books_entry.language));

			insert_chunks.push_back("(" + implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Books> All()
	{
		std::vector<Books> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Books entry{};

			entry.name     = row[0];
			entry.txtfile  = row[1];
			entry.language = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Books> GetWhere(std::string where_filter)
	{
		std::vector<Books> all_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Books entry{};

			entry.name     = row[0];
			entry.txtfile  = row[1];
			entry.language = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static int DeleteWhere(std::string where_filter)
	{
		auto results = content_db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {}",
				TableName(),
				PrimaryKey(),
				where_filter
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_BOOKS_REPOSITORY_H
