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

#ifndef EQEMU_BASE_HACKERS_REPOSITORY_H
#define EQEMU_BASE_HACKERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseHackersRepository {
public:
	struct Hackers {
		int         id;
		std::string account;
		std::string name;
		std::string hacked;
		std::string zone;
		std::string date;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account",
			"name",
			"hacked",
			"zone",
			"date",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("hackers");
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

	static Hackers NewEntity()
	{
		Hackers entry{};

		entry.id      = 0;
		entry.account = "";
		entry.name    = "";
		entry.hacked  = "";
		entry.zone    = "";
		entry.date    = current_timestamp();

		return entry;
	}

	static Hackers GetHackersEntry(
		const std::vector<Hackers> &hackerss,
		int hackers_id
	)
	{
		for (auto &hackers : hackerss) {
			if (hackers.id == hackers_id) {
				return hackers;
			}
		}

		return NewEntity();
	}

	static Hackers FindOne(
		Database& db,
		int hackers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				hackers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Hackers entry{};

			entry.id      = atoi(row[0]);
			entry.account = row[1] ? row[1] : "";
			entry.name    = row[2] ? row[2] : "";
			entry.hacked  = row[3] ? row[3] : "";
			entry.zone    = row[4] ? row[4] : "";
			entry.date    = row[5] ? row[5] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int hackers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				hackers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Hackers hackers_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(hackers_entry.account) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(hackers_entry.name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(hackers_entry.hacked) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(hackers_entry.zone) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(hackers_entry.date) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				hackers_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Hackers InsertOne(
		Database& db,
		Hackers hackers_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(hackers_entry.id));
		insert_values.push_back("'" + EscapeString(hackers_entry.account) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.hacked) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.zone) + "'");
		insert_values.push_back("'" + EscapeString(hackers_entry.date) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			hackers_entry.id = results.LastInsertedID();
			return hackers_entry;
		}

		hackers_entry = NewEntity();

		return hackers_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Hackers> hackers_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &hackers_entry: hackers_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(hackers_entry.id));
			insert_values.push_back("'" + EscapeString(hackers_entry.account) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.hacked) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.zone) + "'");
			insert_values.push_back("'" + EscapeString(hackers_entry.date) + "'");

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

	static std::vector<Hackers> All(Database& db)
	{
		std::vector<Hackers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Hackers entry{};

			entry.id      = atoi(row[0]);
			entry.account = row[1] ? row[1] : "";
			entry.name    = row[2] ? row[2] : "";
			entry.hacked  = row[3] ? row[3] : "";
			entry.zone    = row[4] ? row[4] : "";
			entry.date    = row[5] ? row[5] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Hackers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Hackers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Hackers entry{};

			entry.id      = atoi(row[0]);
			entry.account = row[1] ? row[1] : "";
			entry.name    = row[2] ? row[2] : "";
			entry.hacked  = row[3] ? row[3] : "";
			entry.zone    = row[4] ? row[4] : "";
			entry.date    = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_HACKERS_REPOSITORY_H
