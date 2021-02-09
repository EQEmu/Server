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

#ifndef EQEMU_BASE_CHAR_CREATE_COMBINATIONS_REPOSITORY_H
#define EQEMU_BASE_CHAR_CREATE_COMBINATIONS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharCreateCombinationsRepository {
public:
	struct CharCreateCombinations {
		int allocation_id;
		int race;
		int class;
		int deity;
		int start_zone;
		int expansions_req;
	};

	static std::string PrimaryKey()
	{
		return std::string("race");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"allocation_id",
			"race",
			"class",
			"deity",
			"start_zone",
			"expansions_req",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("char_create_combinations");
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

	static CharCreateCombinations NewEntity()
	{
		CharCreateCombinations entry{};

		entry.allocation_id  = 0;
		entry.race           = 0;
		entry.class          = 0;
		entry.deity          = 0;
		entry.start_zone     = 0;
		entry.expansions_req = 0;

		return entry;
	}

	static CharCreateCombinations GetCharCreateCombinationsEntry(
		const std::vector<CharCreateCombinations> &char_create_combinationss,
		int char_create_combinations_id
	)
	{
		for (auto &char_create_combinations : char_create_combinationss) {
			if (char_create_combinations.race == char_create_combinations_id) {
				return char_create_combinations;
			}
		}

		return NewEntity();
	}

	static CharCreateCombinations FindOne(
		Database& db,
		int char_create_combinations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				char_create_combinations_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharCreateCombinations entry{};

			entry.allocation_id  = atoi(row[0]);
			entry.race           = atoi(row[1]);
			entry.class          = atoi(row[2]);
			entry.deity          = atoi(row[3]);
			entry.start_zone     = atoi(row[4]);
			entry.expansions_req = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int char_create_combinations_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				char_create_combinations_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharCreateCombinations char_create_combinations_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(char_create_combinations_entry.allocation_id));
		update_values.push_back(columns[1] + " = " + std::to_string(char_create_combinations_entry.race));
		update_values.push_back(columns[2] + " = " + std::to_string(char_create_combinations_entry.class));
		update_values.push_back(columns[3] + " = " + std::to_string(char_create_combinations_entry.deity));
		update_values.push_back(columns[4] + " = " + std::to_string(char_create_combinations_entry.start_zone));
		update_values.push_back(columns[5] + " = " + std::to_string(char_create_combinations_entry.expansions_req));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				char_create_combinations_entry.race
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharCreateCombinations InsertOne(
		Database& db,
		CharCreateCombinations char_create_combinations_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(char_create_combinations_entry.allocation_id));
		insert_values.push_back(std::to_string(char_create_combinations_entry.race));
		insert_values.push_back(std::to_string(char_create_combinations_entry.class));
		insert_values.push_back(std::to_string(char_create_combinations_entry.deity));
		insert_values.push_back(std::to_string(char_create_combinations_entry.start_zone));
		insert_values.push_back(std::to_string(char_create_combinations_entry.expansions_req));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			char_create_combinations_entry.race = results.LastInsertedID();
			return char_create_combinations_entry;
		}

		char_create_combinations_entry = NewEntity();

		return char_create_combinations_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharCreateCombinations> char_create_combinations_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &char_create_combinations_entry: char_create_combinations_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(char_create_combinations_entry.allocation_id));
			insert_values.push_back(std::to_string(char_create_combinations_entry.race));
			insert_values.push_back(std::to_string(char_create_combinations_entry.class));
			insert_values.push_back(std::to_string(char_create_combinations_entry.deity));
			insert_values.push_back(std::to_string(char_create_combinations_entry.start_zone));
			insert_values.push_back(std::to_string(char_create_combinations_entry.expansions_req));

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

	static std::vector<CharCreateCombinations> All(Database& db)
	{
		std::vector<CharCreateCombinations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreateCombinations entry{};

			entry.allocation_id  = atoi(row[0]);
			entry.race           = atoi(row[1]);
			entry.class          = atoi(row[2]);
			entry.deity          = atoi(row[3]);
			entry.start_zone     = atoi(row[4]);
			entry.expansions_req = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharCreateCombinations> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharCreateCombinations> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharCreateCombinations entry{};

			entry.allocation_id  = atoi(row[0]);
			entry.race           = atoi(row[1]);
			entry.class          = atoi(row[2]);
			entry.deity          = atoi(row[3]);
			entry.start_zone     = atoi(row[4]);
			entry.expansions_req = atoi(row[5]);

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

#endif //EQEMU_BASE_CHAR_CREATE_COMBINATIONS_REPOSITORY_H
