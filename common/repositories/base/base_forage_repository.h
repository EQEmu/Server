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

#ifndef EQEMU_BASE_FORAGE_REPOSITORY_H
#define EQEMU_BASE_FORAGE_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseForageRepository {
public:
	struct Forage {
		int         id;
		int         zoneid;
		int         Itemid;
		int         level;
		int         chance;
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
			"zoneid",
			"Itemid",
			"level",
			"chance",
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
		return std::string("forage");
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

	static Forage NewEntity()
	{
		Forage entry{};

		entry.id                     = 0;
		entry.zoneid                 = 0;
		entry.Itemid                 = 0;
		entry.level                  = 0;
		entry.chance                 = 0;
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Forage GetForageEntry(
		const std::vector<Forage> &forages,
		int forage_id
	)
	{
		for (auto &forage : forages) {
			if (forage.id == forage_id) {
				return forage;
			}
		}

		return NewEntity();
	}

	static Forage FindOne(
		Database& db,
		int forage_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				forage_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Forage entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.Itemid                 = atoi(row[2]);
			entry.level                  = atoi(row[3]);
			entry.chance                 = atoi(row[4]);
			entry.min_expansion          = atoi(row[5]);
			entry.max_expansion          = atoi(row[6]);
			entry.content_flags          = row[7] ? row[7] : "";
			entry.content_flags_disabled = row[8] ? row[8] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int forage_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				forage_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Forage forage_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(forage_entry.zoneid));
		update_values.push_back(columns[2] + " = " + std::to_string(forage_entry.Itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(forage_entry.level));
		update_values.push_back(columns[4] + " = " + std::to_string(forage_entry.chance));
		update_values.push_back(columns[5] + " = " + std::to_string(forage_entry.min_expansion));
		update_values.push_back(columns[6] + " = " + std::to_string(forage_entry.max_expansion));
		update_values.push_back(columns[7] + " = '" + EscapeString(forage_entry.content_flags) + "'");
		update_values.push_back(columns[8] + " = '" + EscapeString(forage_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				forage_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Forage InsertOne(
		Database& db,
		Forage forage_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(forage_entry.id));
		insert_values.push_back(std::to_string(forage_entry.zoneid));
		insert_values.push_back(std::to_string(forage_entry.Itemid));
		insert_values.push_back(std::to_string(forage_entry.level));
		insert_values.push_back(std::to_string(forage_entry.chance));
		insert_values.push_back(std::to_string(forage_entry.min_expansion));
		insert_values.push_back(std::to_string(forage_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(forage_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(forage_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			forage_entry.id = results.LastInsertedID();
			return forage_entry;
		}

		forage_entry = NewEntity();

		return forage_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Forage> forage_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &forage_entry: forage_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(forage_entry.id));
			insert_values.push_back(std::to_string(forage_entry.zoneid));
			insert_values.push_back(std::to_string(forage_entry.Itemid));
			insert_values.push_back(std::to_string(forage_entry.level));
			insert_values.push_back(std::to_string(forage_entry.chance));
			insert_values.push_back(std::to_string(forage_entry.min_expansion));
			insert_values.push_back(std::to_string(forage_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(forage_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(forage_entry.content_flags_disabled) + "'");

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

	static std::vector<Forage> All(Database& db)
	{
		std::vector<Forage> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Forage entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.Itemid                 = atoi(row[2]);
			entry.level                  = atoi(row[3]);
			entry.chance                 = atoi(row[4]);
			entry.min_expansion          = atoi(row[5]);
			entry.max_expansion          = atoi(row[6]);
			entry.content_flags          = row[7] ? row[7] : "";
			entry.content_flags_disabled = row[8] ? row[8] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Forage> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Forage> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Forage entry{};

			entry.id                     = atoi(row[0]);
			entry.zoneid                 = atoi(row[1]);
			entry.Itemid                 = atoi(row[2]);
			entry.level                  = atoi(row[3]);
			entry.chance                 = atoi(row[4]);
			entry.min_expansion          = atoi(row[5]);
			entry.max_expansion          = atoi(row[6]);
			entry.content_flags          = row[7] ? row[7] : "";
			entry.content_flags_disabled = row[8] ? row[8] : "";

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

#endif //EQEMU_BASE_FORAGE_REPOSITORY_H
