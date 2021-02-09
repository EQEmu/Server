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

#ifndef EQEMU_BASE_LEVEL_EXP_MODS_REPOSITORY_H
#define EQEMU_BASE_LEVEL_EXP_MODS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLevelExpModsRepository {
public:
	struct LevelExpMods {
		int   level;
		float exp_mod;
		float aa_exp_mod;
	};

	static std::string PrimaryKey()
	{
		return std::string("level");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"level",
			"exp_mod",
			"aa_exp_mod",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("level_exp_mods");
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

	static LevelExpMods NewEntity()
	{
		LevelExpMods entry{};

		entry.level      = 0;
		entry.exp_mod    = 0;
		entry.aa_exp_mod = 0;

		return entry;
	}

	static LevelExpMods GetLevelExpModsEntry(
		const std::vector<LevelExpMods> &level_exp_modss,
		int level_exp_mods_id
	)
	{
		for (auto &level_exp_mods : level_exp_modss) {
			if (level_exp_mods.level == level_exp_mods_id) {
				return level_exp_mods;
			}
		}

		return NewEntity();
	}

	static LevelExpMods FindOne(
		Database& db,
		int level_exp_mods_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				level_exp_mods_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LevelExpMods entry{};

			entry.level      = atoi(row[0]);
			entry.exp_mod    = static_cast<float>(atof(row[1]));
			entry.aa_exp_mod = static_cast<float>(atof(row[2]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int level_exp_mods_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				level_exp_mods_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LevelExpMods level_exp_mods_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(level_exp_mods_entry.level));
		update_values.push_back(columns[1] + " = " + std::to_string(level_exp_mods_entry.exp_mod));
		update_values.push_back(columns[2] + " = " + std::to_string(level_exp_mods_entry.aa_exp_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				level_exp_mods_entry.level
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LevelExpMods InsertOne(
		Database& db,
		LevelExpMods level_exp_mods_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(level_exp_mods_entry.level));
		insert_values.push_back(std::to_string(level_exp_mods_entry.exp_mod));
		insert_values.push_back(std::to_string(level_exp_mods_entry.aa_exp_mod));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			level_exp_mods_entry.level = results.LastInsertedID();
			return level_exp_mods_entry;
		}

		level_exp_mods_entry = NewEntity();

		return level_exp_mods_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LevelExpMods> level_exp_mods_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &level_exp_mods_entry: level_exp_mods_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(level_exp_mods_entry.level));
			insert_values.push_back(std::to_string(level_exp_mods_entry.exp_mod));
			insert_values.push_back(std::to_string(level_exp_mods_entry.aa_exp_mod));

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

	static std::vector<LevelExpMods> All(Database& db)
	{
		std::vector<LevelExpMods> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LevelExpMods entry{};

			entry.level      = atoi(row[0]);
			entry.exp_mod    = static_cast<float>(atof(row[1]));
			entry.aa_exp_mod = static_cast<float>(atof(row[2]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LevelExpMods> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LevelExpMods> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LevelExpMods entry{};

			entry.level      = atoi(row[0]);
			entry.exp_mod    = static_cast<float>(atof(row[1]));
			entry.aa_exp_mod = static_cast<float>(atof(row[2]));

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

#endif //EQEMU_BASE_LEVEL_EXP_MODS_REPOSITORY_H
