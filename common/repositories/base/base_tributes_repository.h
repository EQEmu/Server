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

#ifndef EQEMU_BASE_TRIBUTES_REPOSITORY_H
#define EQEMU_BASE_TRIBUTES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTributesRepository {
public:
	struct Tributes {
		int         id;
		int         unknown;
		std::string name;
		std::string descr;
		int         isguild;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"unknown",
			"name",
			"descr",
			"isguild",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("tributes");
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

	static Tributes NewEntity()
	{
		Tributes entry{};

		entry.id      = 0;
		entry.unknown = 0;
		entry.name    = "";
		entry.descr   = "";
		entry.isguild = 0;

		return entry;
	}

	static Tributes GetTributesEntry(
		const std::vector<Tributes> &tributess,
		int tributes_id
	)
	{
		for (auto &tributes : tributess) {
			if (tributes.id == tributes_id) {
				return tributes;
			}
		}

		return NewEntity();
	}

	static Tributes FindOne(
		Database& db,
		int tributes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tributes_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Tributes entry{};

			entry.id      = atoi(row[0]);
			entry.unknown = atoi(row[1]);
			entry.name    = row[2] ? row[2] : "";
			entry.descr   = row[3] ? row[3] : "";
			entry.isguild = atoi(row[4]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tributes_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tributes_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Tributes tributes_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(tributes_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(tributes_entry.unknown));
		update_values.push_back(columns[2] + " = '" + EscapeString(tributes_entry.name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(tributes_entry.descr) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(tributes_entry.isguild));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				tributes_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Tributes InsertOne(
		Database& db,
		Tributes tributes_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(tributes_entry.id));
		insert_values.push_back(std::to_string(tributes_entry.unknown));
		insert_values.push_back("'" + EscapeString(tributes_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(tributes_entry.descr) + "'");
		insert_values.push_back(std::to_string(tributes_entry.isguild));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tributes_entry.id = results.LastInsertedID();
			return tributes_entry;
		}

		tributes_entry = NewEntity();

		return tributes_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Tributes> tributes_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tributes_entry: tributes_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(tributes_entry.id));
			insert_values.push_back(std::to_string(tributes_entry.unknown));
			insert_values.push_back("'" + EscapeString(tributes_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(tributes_entry.descr) + "'");
			insert_values.push_back(std::to_string(tributes_entry.isguild));

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

	static std::vector<Tributes> All(Database& db)
	{
		std::vector<Tributes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tributes entry{};

			entry.id      = atoi(row[0]);
			entry.unknown = atoi(row[1]);
			entry.name    = row[2] ? row[2] : "";
			entry.descr   = row[3] ? row[3] : "";
			entry.isguild = atoi(row[4]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Tributes> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Tributes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Tributes entry{};

			entry.id      = atoi(row[0]);
			entry.unknown = atoi(row[1]);
			entry.name    = row[2] ? row[2] : "";
			entry.descr   = row[3] ? row[3] : "";
			entry.isguild = atoi(row[4]);

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

#endif //EQEMU_BASE_TRIBUTES_REPOSITORY_H
