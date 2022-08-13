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

#ifndef EQEMU_BASE_LOOTTABLE_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_LOOTTABLE_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseLoottableEntriesRepository {
public:
	struct LoottableEntries {
		int   loottable_id;
		int   lootdrop_id;
		int   multiplier;
		int   droplimit;
		int   mindrop;
		float probability;
	};

	static std::string PrimaryKey()
	{
		return std::string("loottable_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"loottable_id",
			"lootdrop_id",
			"multiplier",
			"droplimit",
			"mindrop",
			"probability",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"loottable_id",
			"lootdrop_id",
			"multiplier",
			"droplimit",
			"mindrop",
			"probability",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("loottable_entries");
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

	static LoottableEntries NewEntity()
	{
		LoottableEntries e{};

		e.loottable_id = 0;
		e.lootdrop_id  = 0;
		e.multiplier   = 1;
		e.droplimit    = 0;
		e.mindrop      = 0;
		e.probability  = 100;

		return e;
	}

	static LoottableEntries GetLoottableEntriese(
		const std::vector<LoottableEntries> &loottable_entriess,
		int loottable_entries_id
	)
	{
		for (auto &loottable_entries : loottable_entriess) {
			if (loottable_entries.loottable_id == loottable_entries_id) {
				return loottable_entries;
			}
		}

		return NewEntity();
	}

	static LoottableEntries FindOne(
		Database& db,
		int loottable_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				loottable_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoottableEntries e{};

			e.loottable_id = atoi(row[0]);
			e.lootdrop_id  = atoi(row[1]);
			e.multiplier   = atoi(row[2]);
			e.droplimit    = atoi(row[3]);
			e.mindrop      = atoi(row[4]);
			e.probability  = static_cast<float>(atof(row[5]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int loottable_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				loottable_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LoottableEntries loottable_entries_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(loottable_entries_e.loottable_id));
		update_values.push_back(columns[1] + " = " + std::to_string(loottable_entries_e.lootdrop_id));
		update_values.push_back(columns[2] + " = " + std::to_string(loottable_entries_e.multiplier));
		update_values.push_back(columns[3] + " = " + std::to_string(loottable_entries_e.droplimit));
		update_values.push_back(columns[4] + " = " + std::to_string(loottable_entries_e.mindrop));
		update_values.push_back(columns[5] + " = " + std::to_string(loottable_entries_e.probability));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				loottable_entries_e.loottable_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoottableEntries InsertOne(
		Database& db,
		LoottableEntries loottable_entries_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(loottable_entries_e.loottable_id));
		insert_values.push_back(std::to_string(loottable_entries_e.lootdrop_id));
		insert_values.push_back(std::to_string(loottable_entries_e.multiplier));
		insert_values.push_back(std::to_string(loottable_entries_e.droplimit));
		insert_values.push_back(std::to_string(loottable_entries_e.mindrop));
		insert_values.push_back(std::to_string(loottable_entries_e.probability));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			loottable_entries_e.loottable_id = results.LastInsertedID();
			return loottable_entries_e;
		}

		loottable_entries_e = NewEntity();

		return loottable_entries_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoottableEntries> loottable_entries_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &loottable_entries_e: loottable_entries_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(loottable_entries_e.loottable_id));
			insert_values.push_back(std::to_string(loottable_entries_e.lootdrop_id));
			insert_values.push_back(std::to_string(loottable_entries_e.multiplier));
			insert_values.push_back(std::to_string(loottable_entries_e.droplimit));
			insert_values.push_back(std::to_string(loottable_entries_e.mindrop));
			insert_values.push_back(std::to_string(loottable_entries_e.probability));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<LoottableEntries> All(Database& db)
	{
		std::vector<LoottableEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoottableEntries e{};

			e.loottable_id = atoi(row[0]);
			e.lootdrop_id  = atoi(row[1]);
			e.multiplier   = atoi(row[2]);
			e.droplimit    = atoi(row[3]);
			e.mindrop      = atoi(row[4]);
			e.probability  = static_cast<float>(atof(row[5]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<LoottableEntries> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LoottableEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoottableEntries e{};

			e.loottable_id = atoi(row[0]);
			e.lootdrop_id  = atoi(row[1]);
			e.multiplier   = atoi(row[2]);
			e.droplimit    = atoi(row[3]);
			e.mindrop      = atoi(row[4]);
			e.probability  = static_cast<float>(atof(row[5]));

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_LOOTTABLE_ENTRIES_REPOSITORY_H
