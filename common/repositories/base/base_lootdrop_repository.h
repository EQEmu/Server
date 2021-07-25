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

#ifndef EQEMU_BASE_LOOTDROP_REPOSITORY_H
#define EQEMU_BASE_LOOTDROP_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLootdropRepository {
public:
	struct Lootdrop {
		int         id;
		std::string name;
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
		return std::string("lootdrop");
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

	static Lootdrop NewEntity()
	{
		Lootdrop entry{};

		entry.id                     = 0;
		entry.name                   = "";
		entry.min_expansion          = 0;
		entry.max_expansion          = 0;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Lootdrop GetLootdropEntry(
		const std::vector<Lootdrop> &lootdrops,
		int lootdrop_id
	)
	{
		for (auto &lootdrop : lootdrops) {
			if (lootdrop.id == lootdrop_id) {
				return lootdrop;
			}
		}

		return NewEntity();
	}

	static Lootdrop FindOne(
		Database& db,
		int lootdrop_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				lootdrop_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Lootdrop entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.min_expansion          = atoi(row[2]);
			entry.max_expansion          = atoi(row[3]);
			entry.content_flags          = row[4] ? row[4] : "";
			entry.content_flags_disabled = row[5] ? row[5] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int lootdrop_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				lootdrop_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Lootdrop lootdrop_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(lootdrop_entry.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(lootdrop_entry.min_expansion));
		update_values.push_back(columns[3] + " = " + std::to_string(lootdrop_entry.max_expansion));
		update_values.push_back(columns[4] + " = '" + EscapeString(lootdrop_entry.content_flags) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(lootdrop_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				lootdrop_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Lootdrop InsertOne(
		Database& db,
		Lootdrop lootdrop_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(lootdrop_entry.id));
		insert_values.push_back("'" + EscapeString(lootdrop_entry.name) + "'");
		insert_values.push_back(std::to_string(lootdrop_entry.min_expansion));
		insert_values.push_back(std::to_string(lootdrop_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(lootdrop_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(lootdrop_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			lootdrop_entry.id = results.LastInsertedID();
			return lootdrop_entry;
		}

		lootdrop_entry = NewEntity();

		return lootdrop_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Lootdrop> lootdrop_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &lootdrop_entry: lootdrop_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(lootdrop_entry.id));
			insert_values.push_back("'" + EscapeString(lootdrop_entry.name) + "'");
			insert_values.push_back(std::to_string(lootdrop_entry.min_expansion));
			insert_values.push_back(std::to_string(lootdrop_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(lootdrop_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(lootdrop_entry.content_flags_disabled) + "'");

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

	static std::vector<Lootdrop> All(Database& db)
	{
		std::vector<Lootdrop> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lootdrop entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.min_expansion          = atoi(row[2]);
			entry.max_expansion          = atoi(row[3]);
			entry.content_flags          = row[4] ? row[4] : "";
			entry.content_flags_disabled = row[5] ? row[5] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Lootdrop> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Lootdrop> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lootdrop entry{};

			entry.id                     = atoi(row[0]);
			entry.name                   = row[1] ? row[1] : "";
			entry.min_expansion          = atoi(row[2]);
			entry.max_expansion          = atoi(row[3]);
			entry.content_flags          = row[4] ? row[4] : "";
			entry.content_flags_disabled = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_LOOTDROP_REPOSITORY_H
