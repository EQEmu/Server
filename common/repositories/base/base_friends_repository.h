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

#ifndef EQEMU_BASE_FRIENDS_REPOSITORY_H
#define EQEMU_BASE_FRIENDS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseFriendsRepository {
public:
	struct Friends {
		int         charid;
		int         type;
		std::string name;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"type",
			"name",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("friends");
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

	static Friends NewEntity()
	{
		Friends entry{};

		entry.charid = 0;
		entry.type   = 1;
		entry.name   = "";

		return entry;
	}

	static Friends GetFriendsEntry(
		const std::vector<Friends> &friendss,
		int friends_id
	)
	{
		for (auto &friends : friendss) {
			if (friends.charid == friends_id) {
				return friends;
			}
		}

		return NewEntity();
	}

	static Friends FindOne(
		Database& db,
		int friends_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				friends_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Friends entry{};

			entry.charid = atoi(row[0]);
			entry.type   = atoi(row[1]);
			entry.name   = row[2] ? row[2] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int friends_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				friends_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Friends friends_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(friends_entry.charid));
		update_values.push_back(columns[1] + " = " + std::to_string(friends_entry.type));
		update_values.push_back(columns[2] + " = '" + EscapeString(friends_entry.name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				friends_entry.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Friends InsertOne(
		Database& db,
		Friends friends_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(friends_entry.charid));
		insert_values.push_back(std::to_string(friends_entry.type));
		insert_values.push_back("'" + EscapeString(friends_entry.name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			friends_entry.charid = results.LastInsertedID();
			return friends_entry;
		}

		friends_entry = NewEntity();

		return friends_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Friends> friends_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &friends_entry: friends_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(friends_entry.charid));
			insert_values.push_back(std::to_string(friends_entry.type));
			insert_values.push_back("'" + EscapeString(friends_entry.name) + "'");

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

	static std::vector<Friends> All(Database& db)
	{
		std::vector<Friends> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Friends entry{};

			entry.charid = atoi(row[0]);
			entry.type   = atoi(row[1]);
			entry.name   = row[2] ? row[2] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Friends> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Friends> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Friends entry{};

			entry.charid = atoi(row[0]);
			entry.type   = atoi(row[1]);
			entry.name   = row[2] ? row[2] : "";

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

#endif //EQEMU_BASE_FRIENDS_REPOSITORY_H
