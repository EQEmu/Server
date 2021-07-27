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

#ifndef EQEMU_BASE_RESPAWN_TIMES_REPOSITORY_H
#define EQEMU_BASE_RESPAWN_TIMES_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseRespawnTimesRepository {
public:
	struct RespawnTimes {
		int id;
		int start;
		int duration;
		int instance_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"start",
			"duration",
			"instance_id",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("respawn_times");
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

	static RespawnTimes NewEntity()
	{
		RespawnTimes entry{};

		entry.id          = 0;
		entry.start       = 0;
		entry.duration    = 0;
		entry.instance_id = 0;

		return entry;
	}

	static RespawnTimes GetRespawnTimesEntry(
		const std::vector<RespawnTimes> &respawn_timess,
		int respawn_times_id
	)
	{
		for (auto &respawn_times : respawn_timess) {
			if (respawn_times.id == respawn_times_id) {
				return respawn_times;
			}
		}

		return NewEntity();
	}

	static RespawnTimes FindOne(
		Database& db,
		int respawn_times_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				respawn_times_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			RespawnTimes entry{};

			entry.id          = atoi(row[0]);
			entry.start       = atoi(row[1]);
			entry.duration    = atoi(row[2]);
			entry.instance_id = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int respawn_times_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				respawn_times_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		RespawnTimes respawn_times_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(respawn_times_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(respawn_times_entry.start));
		update_values.push_back(columns[2] + " = " + std::to_string(respawn_times_entry.duration));
		update_values.push_back(columns[3] + " = " + std::to_string(respawn_times_entry.instance_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				respawn_times_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static RespawnTimes InsertOne(
		Database& db,
		RespawnTimes respawn_times_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(respawn_times_entry.id));
		insert_values.push_back(std::to_string(respawn_times_entry.start));
		insert_values.push_back(std::to_string(respawn_times_entry.duration));
		insert_values.push_back(std::to_string(respawn_times_entry.instance_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			respawn_times_entry.id = results.LastInsertedID();
			return respawn_times_entry;
		}

		respawn_times_entry = NewEntity();

		return respawn_times_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<RespawnTimes> respawn_times_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &respawn_times_entry: respawn_times_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(respawn_times_entry.id));
			insert_values.push_back(std::to_string(respawn_times_entry.start));
			insert_values.push_back(std::to_string(respawn_times_entry.duration));
			insert_values.push_back(std::to_string(respawn_times_entry.instance_id));

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

	static std::vector<RespawnTimes> All(Database& db)
	{
		std::vector<RespawnTimes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RespawnTimes entry{};

			entry.id          = atoi(row[0]);
			entry.start       = atoi(row[1]);
			entry.duration    = atoi(row[2]);
			entry.instance_id = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<RespawnTimes> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<RespawnTimes> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			RespawnTimes entry{};

			entry.id          = atoi(row[0]);
			entry.start       = atoi(row[1]);
			entry.duration    = atoi(row[2]);
			entry.instance_id = atoi(row[3]);

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

#endif //EQEMU_BASE_RESPAWN_TIMES_REPOSITORY_H
