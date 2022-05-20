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

#ifndef EQEMU_BASE_SPAWNENTRY_REPOSITORY_H
#define EQEMU_BASE_SPAWNENTRY_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseSpawnentryRepository {
public:
	struct Spawnentry {
		int         spawngroupID;
		int         npcID;
		int         chance;
		int         condition_value_filter;
		int         min_expansion;
		int         max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("spawngroupID");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"spawngroupID",
			"npcID",
			"chance",
			"condition_value_filter",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"spawngroupID",
			"npcID",
			"chance",
			"condition_value_filter",
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

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("spawnentry");
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

	static Spawnentry NewEntity()
	{
		Spawnentry entry{};

		entry.spawngroupID           = 0;
		entry.npcID                  = 0;
		entry.chance                 = 0;
		entry.condition_value_filter = 1;
		entry.min_expansion          = -1;
		entry.max_expansion          = -1;
		entry.content_flags          = "";
		entry.content_flags_disabled = "";

		return entry;
	}

	static Spawnentry GetSpawnentryEntry(
		const std::vector<Spawnentry> &spawnentrys,
		int spawnentry_id
	)
	{
		for (auto &spawnentry : spawnentrys) {
			if (spawnentry.spawngroupID == spawnentry_id) {
				return spawnentry;
			}
		}

		return NewEntity();
	}

	static Spawnentry FindOne(
		Database& db,
		int spawnentry_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawnentry_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawnentry entry{};

			entry.spawngroupID           = atoi(row[0]);
			entry.npcID                  = atoi(row[1]);
			entry.chance                 = atoi(row[2]);
			entry.condition_value_filter = atoi(row[3]);
			entry.min_expansion          = atoi(row[4]);
			entry.max_expansion          = atoi(row[5]);
			entry.content_flags          = row[6] ? row[6] : "";
			entry.content_flags_disabled = row[7] ? row[7] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawnentry_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawnentry_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Spawnentry spawnentry_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(spawnentry_entry.spawngroupID));
		update_values.push_back(columns[1] + " = " + std::to_string(spawnentry_entry.npcID));
		update_values.push_back(columns[2] + " = " + std::to_string(spawnentry_entry.chance));
		update_values.push_back(columns[3] + " = " + std::to_string(spawnentry_entry.condition_value_filter));
		update_values.push_back(columns[4] + " = " + std::to_string(spawnentry_entry.min_expansion));
		update_values.push_back(columns[5] + " = " + std::to_string(spawnentry_entry.max_expansion));
		update_values.push_back(columns[6] + " = '" + EscapeString(spawnentry_entry.content_flags) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(spawnentry_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spawnentry_entry.spawngroupID
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Spawnentry InsertOne(
		Database& db,
		Spawnentry spawnentry_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spawnentry_entry.spawngroupID));
		insert_values.push_back(std::to_string(spawnentry_entry.npcID));
		insert_values.push_back(std::to_string(spawnentry_entry.chance));
		insert_values.push_back(std::to_string(spawnentry_entry.condition_value_filter));
		insert_values.push_back(std::to_string(spawnentry_entry.min_expansion));
		insert_values.push_back(std::to_string(spawnentry_entry.max_expansion));
		insert_values.push_back("'" + EscapeString(spawnentry_entry.content_flags) + "'");
		insert_values.push_back("'" + EscapeString(spawnentry_entry.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spawnentry_entry.spawngroupID = results.LastInsertedID();
			return spawnentry_entry;
		}

		spawnentry_entry = NewEntity();

		return spawnentry_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Spawnentry> spawnentry_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spawnentry_entry: spawnentry_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spawnentry_entry.spawngroupID));
			insert_values.push_back(std::to_string(spawnentry_entry.npcID));
			insert_values.push_back(std::to_string(spawnentry_entry.chance));
			insert_values.push_back(std::to_string(spawnentry_entry.condition_value_filter));
			insert_values.push_back(std::to_string(spawnentry_entry.min_expansion));
			insert_values.push_back(std::to_string(spawnentry_entry.max_expansion));
			insert_values.push_back("'" + EscapeString(spawnentry_entry.content_flags) + "'");
			insert_values.push_back("'" + EscapeString(spawnentry_entry.content_flags_disabled) + "'");

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

	static std::vector<Spawnentry> All(Database& db)
	{
		std::vector<Spawnentry> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawnentry entry{};

			entry.spawngroupID           = atoi(row[0]);
			entry.npcID                  = atoi(row[1]);
			entry.chance                 = atoi(row[2]);
			entry.condition_value_filter = atoi(row[3]);
			entry.min_expansion          = atoi(row[4]);
			entry.max_expansion          = atoi(row[5]);
			entry.content_flags          = row[6] ? row[6] : "";
			entry.content_flags_disabled = row[7] ? row[7] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Spawnentry> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Spawnentry> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawnentry entry{};

			entry.spawngroupID           = atoi(row[0]);
			entry.npcID                  = atoi(row[1]);
			entry.chance                 = atoi(row[2]);
			entry.condition_value_filter = atoi(row[3]);
			entry.min_expansion          = atoi(row[4]);
			entry.max_expansion          = atoi(row[5]);
			entry.content_flags          = row[6] ? row[6] : "";
			entry.content_flags_disabled = row[7] ? row[7] : "";

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

#endif //EQEMU_BASE_SPAWNENTRY_REPOSITORY_H
