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

#ifndef EQEMU_BASE_SAYLINK_REPOSITORY_H
#define EQEMU_BASE_SAYLINK_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSaylinkRepository {
public:
	struct Saylink {
		int         id;
		std::string phrase;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"phrase",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("saylink");
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

	static Saylink NewEntity()
	{
		Saylink entry{};

		entry.id     = 0;
		entry.phrase = "";

		return entry;
	}

	static Saylink GetSaylinkEntry(
		const std::vector<Saylink> &saylinks,
		int saylink_id
	)
	{
		for (auto &saylink : saylinks) {
			if (saylink.id == saylink_id) {
				return saylink;
			}
		}

		return NewEntity();
	}

	static Saylink FindOne(
		Database& db,
		int saylink_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				saylink_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Saylink entry{};

			entry.id     = atoi(row[0]);
			entry.phrase = row[1] ? row[1] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int saylink_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				saylink_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Saylink saylink_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(saylink_entry.phrase) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				saylink_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Saylink InsertOne(
		Database& db,
		Saylink saylink_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(saylink_entry.id));
		insert_values.push_back("'" + EscapeString(saylink_entry.phrase) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			saylink_entry.id = results.LastInsertedID();
			return saylink_entry;
		}

		saylink_entry = NewEntity();

		return saylink_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Saylink> saylink_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &saylink_entry: saylink_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(saylink_entry.id));
			insert_values.push_back("'" + EscapeString(saylink_entry.phrase) + "'");

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

	static std::vector<Saylink> All(Database& db)
	{
		std::vector<Saylink> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Saylink entry{};

			entry.id     = atoi(row[0]);
			entry.phrase = row[1] ? row[1] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Saylink> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Saylink> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Saylink entry{};

			entry.id     = atoi(row[0]);
			entry.phrase = row[1] ? row[1] : "";

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

#endif //EQEMU_BASE_SAYLINK_REPOSITORY_H
