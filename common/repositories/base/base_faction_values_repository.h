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

#ifndef EQEMU_BASE_FACTION_VALUES_REPOSITORY_H
#define EQEMU_BASE_FACTION_VALUES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseFactionValuesRepository {
public:
	struct FactionValues {
		int char_id;
		int faction_id;
		int current_value;
		int temp;
	};

	static std::string PrimaryKey()
	{
		return std::string("char_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"char_id",
			"faction_id",
			"current_value",
			"temp",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("faction_values");
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

	static FactionValues NewEntity()
	{
		FactionValues entry{};

		entry.char_id       = 0;
		entry.faction_id    = 0;
		entry.current_value = 0;
		entry.temp          = 0;

		return entry;
	}

	static FactionValues GetFactionValuesEntry(
		const std::vector<FactionValues> &faction_valuess,
		int faction_values_id
	)
	{
		for (auto &faction_values : faction_valuess) {
			if (faction_values.char_id == faction_values_id) {
				return faction_values;
			}
		}

		return NewEntity();
	}

	static FactionValues FindOne(
		Database& db,
		int faction_values_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				faction_values_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FactionValues entry{};

			entry.char_id       = atoi(row[0]);
			entry.faction_id    = atoi(row[1]);
			entry.current_value = atoi(row[2]);
			entry.temp          = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int faction_values_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				faction_values_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		FactionValues faction_values_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(faction_values_entry.char_id));
		update_values.push_back(columns[1] + " = " + std::to_string(faction_values_entry.faction_id));
		update_values.push_back(columns[2] + " = " + std::to_string(faction_values_entry.current_value));
		update_values.push_back(columns[3] + " = " + std::to_string(faction_values_entry.temp));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				faction_values_entry.char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static FactionValues InsertOne(
		Database& db,
		FactionValues faction_values_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(faction_values_entry.char_id));
		insert_values.push_back(std::to_string(faction_values_entry.faction_id));
		insert_values.push_back(std::to_string(faction_values_entry.current_value));
		insert_values.push_back(std::to_string(faction_values_entry.temp));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			faction_values_entry.char_id = results.LastInsertedID();
			return faction_values_entry;
		}

		faction_values_entry = NewEntity();

		return faction_values_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<FactionValues> faction_values_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &faction_values_entry: faction_values_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(faction_values_entry.char_id));
			insert_values.push_back(std::to_string(faction_values_entry.faction_id));
			insert_values.push_back(std::to_string(faction_values_entry.current_value));
			insert_values.push_back(std::to_string(faction_values_entry.temp));

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

	static std::vector<FactionValues> All(Database& db)
	{
		std::vector<FactionValues> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionValues entry{};

			entry.char_id       = atoi(row[0]);
			entry.faction_id    = atoi(row[1]);
			entry.current_value = atoi(row[2]);
			entry.temp          = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<FactionValues> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<FactionValues> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionValues entry{};

			entry.char_id       = atoi(row[0]);
			entry.faction_id    = atoi(row[1]);
			entry.current_value = atoi(row[2]);
			entry.temp          = atoi(row[3]);

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

#endif //EQEMU_BASE_FACTION_VALUES_REPOSITORY_H
