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

#ifndef EQEMU_BASE_TRIBUTE_LEVELS_REPOSITORY_H
#define EQEMU_BASE_TRIBUTE_LEVELS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseTributeLevelsRepository {
public:
	struct TributeLevels {
		int tribute_id;
		int level;
		int cost;
		int item_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("tribute_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"tribute_id",
			"level",
			"cost",
			"item_id",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("tribute_levels");
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

	static TributeLevels NewEntity()
	{
		TributeLevels entry{};

		entry.tribute_id = 0;
		entry.level      = 0;
		entry.cost       = 0;
		entry.item_id    = 0;

		return entry;
	}

	static TributeLevels GetTributeLevelsEntry(
		const std::vector<TributeLevels> &tribute_levelss,
		int tribute_levels_id
	)
	{
		for (auto &tribute_levels : tribute_levelss) {
			if (tribute_levels.tribute_id == tribute_levels_id) {
				return tribute_levels;
			}
		}

		return NewEntity();
	}

	static TributeLevels FindOne(
		Database& db,
		int tribute_levels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				tribute_levels_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			TributeLevels entry{};

			entry.tribute_id = atoi(row[0]);
			entry.level      = atoi(row[1]);
			entry.cost       = atoi(row[2]);
			entry.item_id    = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int tribute_levels_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				tribute_levels_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		TributeLevels tribute_levels_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(tribute_levels_entry.tribute_id));
		update_values.push_back(columns[1] + " = " + std::to_string(tribute_levels_entry.level));
		update_values.push_back(columns[2] + " = " + std::to_string(tribute_levels_entry.cost));
		update_values.push_back(columns[3] + " = " + std::to_string(tribute_levels_entry.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				tribute_levels_entry.tribute_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static TributeLevels InsertOne(
		Database& db,
		TributeLevels tribute_levels_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(tribute_levels_entry.tribute_id));
		insert_values.push_back(std::to_string(tribute_levels_entry.level));
		insert_values.push_back(std::to_string(tribute_levels_entry.cost));
		insert_values.push_back(std::to_string(tribute_levels_entry.item_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			tribute_levels_entry.tribute_id = results.LastInsertedID();
			return tribute_levels_entry;
		}

		tribute_levels_entry = NewEntity();

		return tribute_levels_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<TributeLevels> tribute_levels_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &tribute_levels_entry: tribute_levels_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(tribute_levels_entry.tribute_id));
			insert_values.push_back(std::to_string(tribute_levels_entry.level));
			insert_values.push_back(std::to_string(tribute_levels_entry.cost));
			insert_values.push_back(std::to_string(tribute_levels_entry.item_id));

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

	static std::vector<TributeLevels> All(Database& db)
	{
		std::vector<TributeLevels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TributeLevels entry{};

			entry.tribute_id = atoi(row[0]);
			entry.level      = atoi(row[1]);
			entry.cost       = atoi(row[2]);
			entry.item_id    = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<TributeLevels> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<TributeLevels> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			TributeLevels entry{};

			entry.tribute_id = atoi(row[0]);
			entry.level      = atoi(row[1]);
			entry.cost       = atoi(row[2]);
			entry.item_id    = atoi(row[3]);

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

#endif //EQEMU_BASE_TRIBUTE_LEVELS_REPOSITORY_H
