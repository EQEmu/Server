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

#ifndef EQEMU_BASE_LOOTTABLE_REPOSITORY_H
#define EQEMU_BASE_LOOTTABLE_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLoottableRepository {
public:
	struct Loottable {
		int         id;
		std::string name;
		int         mincash;
		int         maxcash;
		int         avgcoin;
		int         done;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
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
			"mincash",
			"maxcash",
			"avgcoin",
			"done",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("loottable");
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

	static Loottable NewEntity()
	{
		Loottable entry{};

		entry.id                     = 0;
		entry.name                   = "";
		entry.mincash                = 0;
		entry.maxcash                = 0;
		entry.avgcoin                = 0;
		entry.done                   = 0;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Loottable GetLoottableEntry(
		const std::vector<Loottable> &loottables,
		int loottable_id
	)
	{
		for (auto &loottable : loottables) {
			if (loottable.id == loottable_id) {
				return loottable;
			}
		}

		return NewEntity();
	}

	static Loottable FindOne(
		Database& db,
		int loottable_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				loottable_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Loottable entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.mincash                = atoi(row[2]);
			entry.maxcash                = atoi(row[3]);
			entry.avgcoin                = atoi(row[4]);
			entry.done                   = atoi(row[5]);
			entry.min_expansion          = atoi(row[6]);
			entry.max_expansion          = atoi(row[7]);
			entry.content_flags          = row[8] ? row[8] : "";
			entry.content_flags_disabled = row[9] ? row[9] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int loottable_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				loottable_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Loottable loottable_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(loottable_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(loottable_entry.mincash));
		update_values.push_back(columns[3] + " = " + std::to_string(loottable_entry.maxcash));
		update_values.push_back(columns[4] + " = " + std::to_string(loottable_entry.avgcoin));
		update_values.push_back(columns[5] + " = " + std::to_string(loottable_entry.done));
		update_values.push_back(columns[6] + " = " + std::to_string(loottable_entry.min_expansion));
		update_values.push_back(columns[7] + " = " + std::to_string(loottable_entry.max_expansion));
		update_values.push_back(columns[8] + " = '" + EscapeString(loottable_entry.content_flags) + "'");
		update_values.push_back(columns[9] + " = '" + EscapeString(loottable_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				loottable_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Loottable InsertOne(
		Database& db,
		Loottable loottable_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(loottable_entry.id));
		insert_values.push_back("'" + EscapeString(loottable_entry.name) + "'");
		insert_values.push_back(std::to_string(loottable_entry.mincash));
		insert_values.push_back(std::to_string(loottable_entry.maxcash));
		insert_values.push_back(std::to_string(loottable_entry.avgcoin));
		insert_values.push_back(std::to_string(loottable_entry.done));
		insert_values.push_back(std::to_string(loottable_entry.min_expansion));
		insert_values.push_back(std::to_string(loottable_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(loottable_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(loottable_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			loottable_entry.id = results.LastInsertedID();
			return loottable_entry;
		}

		loottable_entry = NewEntity();

		return loottable_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Loottable> loottable_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &loottable_entry: loottable_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(loottable_entry.id));
			insert_values.push_back("'" + EscapeString(loottable_entry.name) + "'");
			insert_values.push_back(std::to_string(loottable_entry.mincash));
			insert_values.push_back(std::to_string(loottable_entry.maxcash));
			insert_values.push_back(std::to_string(loottable_entry.avgcoin));
			insert_values.push_back(std::to_string(loottable_entry.done));
			insert_values.push_back(std::to_string(loottable_entry.min_expansion));
			insert_values.push_back(std::to_string(loottable_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(loottable_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(loottable_entry.content_flags_disabled) + "'");

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

	static std::vector<Loottable> All(Database& db)
	{
		std::vector<Loottable> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Loottable entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.mincash                = atoi(row[2]);
			entry.maxcash                = atoi(row[3]);
			entry.avgcoin                = atoi(row[4]);
			entry.done                   = atoi(row[5]);
			entry.min_expansion          = atoi(row[6]);
			entry.max_expansion          = atoi(row[7]);
			entry.content_flags          = row[8] ? row[8] : "";
			entry.content_flags_disabled = row[9] ? row[9] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Loottable> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Loottable> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Loottable entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.mincash                = atoi(row[2]);
			entry.maxcash                = atoi(row[3]);
			entry.avgcoin                = atoi(row[4]);
			entry.done                   = atoi(row[5]);
			entry.min_expansion          = atoi(row[6]);
			entry.max_expansion          = atoi(row[7]);
			entry.content_flags          = row[8] ? row[8] : "";
			entry.content_flags_disabled = row[9] ? row[9] : "";

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

#endif //EQEMU_BASE_LOOTTABLE_REPOSITORY_H
