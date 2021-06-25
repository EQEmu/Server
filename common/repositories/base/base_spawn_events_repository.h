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

#ifndef EQEMU_BASE_SPAWN_EVENTS_REPOSITORY_H
#define EQEMU_BASE_SPAWN_EVENTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseSpawnEventsRepository {
public:
	struct SpawnEvents {
		int         id;
		std::string zone;
		int         cond_id;
		std::string name;
		int         period;
		int         next_minute;
		int         next_hour;
		int         next_day;
		int         next_month;
		int         next_year;
		int         enabled;
		int         action;
		int         argument;
		int         strict;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"cond_id",
			"name",
			"period",
			"next_minute",
			"next_hour",
			"next_day",
			"next_month",
			"next_year",
			"enabled",
			"action",
			"argument",
			"strict",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("spawn_events");
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

	static SpawnEvents NewEntity()
	{
		SpawnEvents entry{};

		entry.id          = 0;
		entry.zone        = "";
		entry.cond_id     = 0;
		entry.name        = "";
		entry.period      = 0;
		entry.next_minute = 0;
		entry.next_hour   = 0;
		entry.next_day    = 0;
		entry.next_month  = 0;
		entry.next_year   = 0;
		entry.enabled     = 1;
		entry.action      = 0;
		entry.argument    = 0;
		entry.strict      = 0;

		return entry;
	}

	static SpawnEvents GetSpawnEventsEntry(
		const std::vector<SpawnEvents> &spawn_eventss,
		int spawn_events_id
	)
	{
		for (auto &spawn_events : spawn_eventss) {
			if (spawn_events.id == spawn_events_id) {
				return spawn_events;
			}
		}

		return NewEntity();
	}

	static SpawnEvents FindOne(
		Database& db,
		int spawn_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				spawn_events_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			SpawnEvents entry{};

			entry.id          = atoi(row[0]);
			entry.zone        = row[1] ? row[1] : "";
			entry.cond_id     = atoi(row[2]);
			entry.name        = row[3] ? row[3] : "";
			entry.period      = atoi(row[4]);
			entry.next_minute = atoi(row[5]);
			entry.next_hour   = atoi(row[6]);
			entry.next_day    = atoi(row[7]);
			entry.next_month  = atoi(row[8]);
			entry.next_year   = atoi(row[9]);
			entry.enabled     = atoi(row[10]);
			entry.action      = atoi(row[11]);
			entry.argument    = atoi(row[12]);
			entry.strict      = atoi(row[13]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int spawn_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				spawn_events_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		SpawnEvents spawn_events_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(spawn_events_entry.zone) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(spawn_events_entry.cond_id));
		update_values.push_back(columns[3] + " = '" + EscapeString(spawn_events_entry.name) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(spawn_events_entry.period));
		update_values.push_back(columns[5] + " = " + std::to_string(spawn_events_entry.next_minute));
		update_values.push_back(columns[6] + " = " + std::to_string(spawn_events_entry.next_hour));
		update_values.push_back(columns[7] + " = " + std::to_string(spawn_events_entry.next_day));
		update_values.push_back(columns[8] + " = " + std::to_string(spawn_events_entry.next_month));
		update_values.push_back(columns[9] + " = " + std::to_string(spawn_events_entry.next_year));
		update_values.push_back(columns[10] + " = " + std::to_string(spawn_events_entry.enabled));
		update_values.push_back(columns[11] + " = " + std::to_string(spawn_events_entry.action));
		update_values.push_back(columns[12] + " = " + std::to_string(spawn_events_entry.argument));
		update_values.push_back(columns[13] + " = " + std::to_string(spawn_events_entry.strict));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				spawn_events_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static SpawnEvents InsertOne(
		Database& db,
		SpawnEvents spawn_events_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(spawn_events_entry.id));
		insert_values.push_back("'" + EscapeString(spawn_events_entry.zone) + "'");
		insert_values.push_back(std::to_string(spawn_events_entry.cond_id));
		insert_values.push_back("'" + EscapeString(spawn_events_entry.name) + "'");
		insert_values.push_back(std::to_string(spawn_events_entry.period));
		insert_values.push_back(std::to_string(spawn_events_entry.next_minute));
		insert_values.push_back(std::to_string(spawn_events_entry.next_hour));
		insert_values.push_back(std::to_string(spawn_events_entry.next_day));
		insert_values.push_back(std::to_string(spawn_events_entry.next_month));
		insert_values.push_back(std::to_string(spawn_events_entry.next_year));
		insert_values.push_back(std::to_string(spawn_events_entry.enabled));
		insert_values.push_back(std::to_string(spawn_events_entry.action));
		insert_values.push_back(std::to_string(spawn_events_entry.argument));
		insert_values.push_back(std::to_string(spawn_events_entry.strict));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			spawn_events_entry.id = results.LastInsertedID();
			return spawn_events_entry;
		}

		spawn_events_entry = NewEntity();

		return spawn_events_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<SpawnEvents> spawn_events_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &spawn_events_entry: spawn_events_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(spawn_events_entry.id));
			insert_values.push_back("'" + EscapeString(spawn_events_entry.zone) + "'");
			insert_values.push_back(std::to_string(spawn_events_entry.cond_id));
			insert_values.push_back("'" + EscapeString(spawn_events_entry.name) + "'");
			insert_values.push_back(std::to_string(spawn_events_entry.period));
			insert_values.push_back(std::to_string(spawn_events_entry.next_minute));
			insert_values.push_back(std::to_string(spawn_events_entry.next_hour));
			insert_values.push_back(std::to_string(spawn_events_entry.next_day));
			insert_values.push_back(std::to_string(spawn_events_entry.next_month));
			insert_values.push_back(std::to_string(spawn_events_entry.next_year));
			insert_values.push_back(std::to_string(spawn_events_entry.enabled));
			insert_values.push_back(std::to_string(spawn_events_entry.action));
			insert_values.push_back(std::to_string(spawn_events_entry.argument));
			insert_values.push_back(std::to_string(spawn_events_entry.strict));

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

	static std::vector<SpawnEvents> All(Database& db)
	{
		std::vector<SpawnEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnEvents entry{};

			entry.id          = atoi(row[0]);
			entry.zone        = row[1] ? row[1] : "";
			entry.cond_id     = atoi(row[2]);
			entry.name        = row[3] ? row[3] : "";
			entry.period      = atoi(row[4]);
			entry.next_minute = atoi(row[5]);
			entry.next_hour   = atoi(row[6]);
			entry.next_day    = atoi(row[7]);
			entry.next_month  = atoi(row[8]);
			entry.next_year   = atoi(row[9]);
			entry.enabled     = atoi(row[10]);
			entry.action      = atoi(row[11]);
			entry.argument    = atoi(row[12]);
			entry.strict      = atoi(row[13]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<SpawnEvents> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<SpawnEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			SpawnEvents entry{};

			entry.id          = atoi(row[0]);
			entry.zone        = row[1] ? row[1] : "";
			entry.cond_id     = atoi(row[2]);
			entry.name        = row[3] ? row[3] : "";
			entry.period      = atoi(row[4]);
			entry.next_minute = atoi(row[5]);
			entry.next_hour   = atoi(row[6]);
			entry.next_day    = atoi(row[7]);
			entry.next_month  = atoi(row[8]);
			entry.next_year   = atoi(row[9]);
			entry.enabled     = atoi(row[10]);
			entry.action      = atoi(row[11]);
			entry.argument    = atoi(row[12]);
			entry.strict      = atoi(row[13]);

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

#endif //EQEMU_BASE_SPAWN_EVENTS_REPOSITORY_H
