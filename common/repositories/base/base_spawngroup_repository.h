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

#ifndef EQEMU_BASE_SPAWNGROUP_REPOSITORY_H
#define EQEMU_BASE_SPAWNGROUP_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseSpawngroupRepository {
public:
	struct Spawngroup {
		int         id;
		std::string name;
		int         spawn_limit;
		float       dist;
		float       max_x;
		float       min_x;
		float       max_y;
		float       min_y;
		int         delay;
		int         mindelay;
		int         despawn;
		int         despawn_timer;
		int         wp_spawns;
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
			"spawn_limit",
			"dist",
			"max_x",
			"min_x",
			"max_y",
			"min_y",
			"delay",
			"mindelay",
			"despawn",
			"despawn_timer",
			"wp_spawns",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"name",
			"spawn_limit",
			"dist",
			"max_x",
			"min_x",
			"max_y",
			"min_y",
			"delay",
			"mindelay",
			"despawn",
			"despawn_timer",
			"wp_spawns",
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
		return std::string("spawngroup");
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

	static Spawngroup NewEntity()
	{
		Spawngroup e{};

		e.id            = 0;
		e.name          = "";
		e.spawn_limit   = 0;
		e.dist          = 0;
		e.max_x         = 0;
		e.min_x         = 0;
		e.max_y         = 0;
		e.min_y         = 0;
		e.delay         = 45000;
		e.mindelay      = 15000;
		e.despawn       = 0;
		e.despawn_timer = 100;
		e.wp_spawns     = 0;

		return e;
	}

	static Spawngroup GetSpawngroupe(
		const std::vector<Spawngroup> &spawngroups,
		int spawngroup_id
	)
	{
		for (auto &spawngroup : spawngroups) {
			if (spawngroup.id == spawngroup_id) {
				return spawngroup;
			}
		}

		return NewEntity();
	}

	static Spawngroup FindOne(
		Database& db,
		int spawngroup_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawngroup_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Spawngroup e{};

			e.id            = atoi(row[0]);
			e.name          = row[1] ? row[1] : "";
			e.spawn_limit   = atoi(row[2]);
			e.dist          = static_cast<float>(atof(row[3]));
			e.max_x         = static_cast<float>(atof(row[4]));
			e.min_x         = static_cast<float>(atof(row[5]));
			e.max_y         = static_cast<float>(atof(row[6]));
			e.min_y         = static_cast<float>(atof(row[7]));
			e.delay         = atoi(row[8]);
			e.mindelay      = atoi(row[9]);
			e.despawn       = atoi(row[10]);
			e.despawn_timer = atoi(row[11]);
			e.wp_spawns     = atoi(row[12]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawngroup_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawngroup_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Spawngroup spawngroup_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + Strings::Escape(spawngroup_e.name) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(spawngroup_e.spawn_limit));
		update_values.push_back(columns[3] + " = " + std::to_string(spawngroup_e.dist));
		update_values.push_back(columns[4] + " = " + std::to_string(spawngroup_e.max_x));
		update_values.push_back(columns[5] + " = " + std::to_string(spawngroup_e.min_x));
		update_values.push_back(columns[6] + " = " + std::to_string(spawngroup_e.max_y));
		update_values.push_back(columns[7] + " = " + std::to_string(spawngroup_e.min_y));
		update_values.push_back(columns[8] + " = " + std::to_string(spawngroup_e.delay));
		update_values.push_back(columns[9] + " = " + std::to_string(spawngroup_e.mindelay));
		update_values.push_back(columns[10] + " = " + std::to_string(spawngroup_e.despawn));
		update_values.push_back(columns[11] + " = " + std::to_string(spawngroup_e.despawn_timer));
		update_values.push_back(columns[12] + " = " + std::to_string(spawngroup_e.wp_spawns));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				spawngroup_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Spawngroup InsertOne(
		Database& db,
		Spawngroup spawngroup_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spawngroup_e.id));
		insert_values.push_back("'" + Strings::Escape(spawngroup_e.name) + "'");
		insert_values.push_back(std::to_string(spawngroup_e.spawn_limit));
		insert_values.push_back(std::to_string(spawngroup_e.dist));
		insert_values.push_back(std::to_string(spawngroup_e.max_x));
		insert_values.push_back(std::to_string(spawngroup_e.min_x));
		insert_values.push_back(std::to_string(spawngroup_e.max_y));
		insert_values.push_back(std::to_string(spawngroup_e.min_y));
		insert_values.push_back(std::to_string(spawngroup_e.delay));
		insert_values.push_back(std::to_string(spawngroup_e.mindelay));
		insert_values.push_back(std::to_string(spawngroup_e.despawn));
		insert_values.push_back(std::to_string(spawngroup_e.despawn_timer));
		insert_values.push_back(std::to_string(spawngroup_e.wp_spawns));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spawngroup_e.id = results.LastInsertedID();
			return spawngroup_e;
		}

		spawngroup_e = NewEntity();

		return spawngroup_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Spawngroup> spawngroup_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spawngroup_e: spawngroup_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spawngroup_e.id));
			insert_values.push_back("'" + Strings::Escape(spawngroup_e.name) + "'");
			insert_values.push_back(std::to_string(spawngroup_e.spawn_limit));
			insert_values.push_back(std::to_string(spawngroup_e.dist));
			insert_values.push_back(std::to_string(spawngroup_e.max_x));
			insert_values.push_back(std::to_string(spawngroup_e.min_x));
			insert_values.push_back(std::to_string(spawngroup_e.max_y));
			insert_values.push_back(std::to_string(spawngroup_e.min_y));
			insert_values.push_back(std::to_string(spawngroup_e.delay));
			insert_values.push_back(std::to_string(spawngroup_e.mindelay));
			insert_values.push_back(std::to_string(spawngroup_e.despawn));
			insert_values.push_back(std::to_string(spawngroup_e.despawn_timer));
			insert_values.push_back(std::to_string(spawngroup_e.wp_spawns));

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

	static std::vector<Spawngroup> All(Database& db)
	{
		std::vector<Spawngroup> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawngroup e{};

			e.id            = atoi(row[0]);
			e.name          = row[1] ? row[1] : "";
			e.spawn_limit   = atoi(row[2]);
			e.dist          = static_cast<float>(atof(row[3]));
			e.max_x         = static_cast<float>(atof(row[4]));
			e.min_x         = static_cast<float>(atof(row[5]));
			e.max_y         = static_cast<float>(atof(row[6]));
			e.min_y         = static_cast<float>(atof(row[7]));
			e.delay         = atoi(row[8]);
			e.mindelay      = atoi(row[9]);
			e.despawn       = atoi(row[10]);
			e.despawn_timer = atoi(row[11]);
			e.wp_spawns     = atoi(row[12]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Spawngroup> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Spawngroup> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Spawngroup e{};

			e.id            = atoi(row[0]);
			e.name          = row[1] ? row[1] : "";
			e.spawn_limit   = atoi(row[2]);
			e.dist          = static_cast<float>(atof(row[3]));
			e.max_x         = static_cast<float>(atof(row[4]));
			e.min_x         = static_cast<float>(atof(row[5]));
			e.max_y         = static_cast<float>(atof(row[6]));
			e.min_y         = static_cast<float>(atof(row[7]));
			e.delay         = atoi(row[8]);
			e.mindelay      = atoi(row[9]);
			e.despawn       = atoi(row[10]);
			e.despawn_timer = atoi(row[11]);
			e.wp_spawns     = atoi(row[12]);

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

#endif //EQEMU_BASE_SPAWNGROUP_REPOSITORY_H
