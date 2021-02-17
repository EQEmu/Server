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

#ifndef EQEMU_BASE_SERVER_SCHEDULED_EVENTS_REPOSITORY_H
#define EQEMU_BASE_SERVER_SCHEDULED_EVENTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseServerScheduledEventsRepository {
public:
	struct ServerScheduledEvents {
		int         id;
		std::string description;
		std::string event_type;
		std::string event_data;
		int         minute_start;
		int         hour_start;
		int         day_start;
		int         month_start;
		int         year_start;
		int         minute_end;
		int         hour_end;
		int         day_end;
		int         month_end;
		int         year_end;
		std::string cron_expression;
		std::string created_at;
		std::string deleted_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"description",
			"event_type",
			"event_data",
			"minute_start",
			"hour_start",
			"day_start",
			"month_start",
			"year_start",
			"minute_end",
			"hour_end",
			"day_end",
			"month_end",
			"year_end",
			"cron_expression",
			"created_at",
			"deleted_at",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("server_scheduled_events");
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

	static ServerScheduledEvents NewEntity()
	{
		ServerScheduledEvents entry{};

		entry.id              = 0;
		entry.description     = "";
		entry.event_type      = "";
		entry.event_data      = "";
		entry.minute_start    = 0;
		entry.hour_start      = 0;
		entry.day_start       = 0;
		entry.month_start     = 0;
		entry.year_start      = 0;
		entry.minute_end      = 0;
		entry.hour_end        = 0;
		entry.day_end         = 0;
		entry.month_end       = 0;
		entry.year_end        = 0;
		entry.cron_expression = "";
		entry.created_at      = "";
		entry.deleted_at      = "";

		return entry;
	}

	static ServerScheduledEvents GetServerScheduledEventsEntry(
		const std::vector<ServerScheduledEvents> &server_scheduled_eventss,
		int server_scheduled_events_id
	)
	{
		for (auto &server_scheduled_events : server_scheduled_eventss) {
			if (server_scheduled_events.id == server_scheduled_events_id) {
				return server_scheduled_events;
			}
		}

		return NewEntity();
	}

	static ServerScheduledEvents FindOne(
		Database& db,
		int server_scheduled_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				server_scheduled_events_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ServerScheduledEvents entry{};

			entry.id              = atoi(row[0]);
			entry.description     = row[1] ? row[1] : "";
			entry.event_type      = row[2] ? row[2] : "";
			entry.event_data      = row[3] ? row[3] : "";
			entry.minute_start    = atoi(row[4]);
			entry.hour_start      = atoi(row[5]);
			entry.day_start       = atoi(row[6]);
			entry.month_start     = atoi(row[7]);
			entry.year_start      = atoi(row[8]);
			entry.minute_end      = atoi(row[9]);
			entry.hour_end        = atoi(row[10]);
			entry.day_end         = atoi(row[11]);
			entry.month_end       = atoi(row[12]);
			entry.year_end        = atoi(row[13]);
			entry.cron_expression = row[14] ? row[14] : "";
			entry.created_at      = row[15] ? row[15] : "";
			entry.deleted_at      = row[16] ? row[16] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int server_scheduled_events_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				server_scheduled_events_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		ServerScheduledEvents server_scheduled_events_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(server_scheduled_events_entry.description) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(server_scheduled_events_entry.event_type) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(server_scheduled_events_entry.event_data) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(server_scheduled_events_entry.minute_start));
		update_values.push_back(columns[5] + " = " + std::to_string(server_scheduled_events_entry.hour_start));
		update_values.push_back(columns[6] + " = " + std::to_string(server_scheduled_events_entry.day_start));
		update_values.push_back(columns[7] + " = " + std::to_string(server_scheduled_events_entry.month_start));
		update_values.push_back(columns[8] + " = " + std::to_string(server_scheduled_events_entry.year_start));
		update_values.push_back(columns[9] + " = " + std::to_string(server_scheduled_events_entry.minute_end));
		update_values.push_back(columns[10] + " = " + std::to_string(server_scheduled_events_entry.hour_end));
		update_values.push_back(columns[11] + " = " + std::to_string(server_scheduled_events_entry.day_end));
		update_values.push_back(columns[12] + " = " + std::to_string(server_scheduled_events_entry.month_end));
		update_values.push_back(columns[13] + " = " + std::to_string(server_scheduled_events_entry.year_end));
		update_values.push_back(columns[14] + " = '" + EscapeString(server_scheduled_events_entry.cron_expression) + "'");
		update_values.push_back(columns[15] + " = '" + EscapeString(server_scheduled_events_entry.created_at) + "'");
		update_values.push_back(columns[16] + " = '" + EscapeString(server_scheduled_events_entry.deleted_at) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				server_scheduled_events_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ServerScheduledEvents InsertOne(
		Database& db,
		ServerScheduledEvents server_scheduled_events_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(server_scheduled_events_entry.id));
		insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.description) + "'");
		insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.event_type) + "'");
		insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.event_data) + "'");
		insert_values.push_back(std::to_string(server_scheduled_events_entry.minute_start));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.hour_start));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.day_start));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.month_start));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.year_start));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.minute_end));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.hour_end));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.day_end));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.month_end));
		insert_values.push_back(std::to_string(server_scheduled_events_entry.year_end));
		insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.cron_expression) + "'");
		insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.created_at) + "'");
		insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.deleted_at) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			server_scheduled_events_entry.id = results.LastInsertedID();
			return server_scheduled_events_entry;
		}

		server_scheduled_events_entry = NewEntity();

		return server_scheduled_events_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<ServerScheduledEvents> server_scheduled_events_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &server_scheduled_events_entry: server_scheduled_events_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(server_scheduled_events_entry.id));
			insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.description) + "'");
			insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.event_type) + "'");
			insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.event_data) + "'");
			insert_values.push_back(std::to_string(server_scheduled_events_entry.minute_start));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.hour_start));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.day_start));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.month_start));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.year_start));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.minute_end));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.hour_end));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.day_end));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.month_end));
			insert_values.push_back(std::to_string(server_scheduled_events_entry.year_end));
			insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.cron_expression) + "'");
			insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.created_at) + "'");
			insert_values.push_back("'" + EscapeString(server_scheduled_events_entry.deleted_at) + "'");

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

	static std::vector<ServerScheduledEvents> All(Database& db)
	{
		std::vector<ServerScheduledEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ServerScheduledEvents entry{};

			entry.id              = atoi(row[0]);
			entry.description     = row[1] ? row[1] : "";
			entry.event_type      = row[2] ? row[2] : "";
			entry.event_data      = row[3] ? row[3] : "";
			entry.minute_start    = atoi(row[4]);
			entry.hour_start      = atoi(row[5]);
			entry.day_start       = atoi(row[6]);
			entry.month_start     = atoi(row[7]);
			entry.year_start      = atoi(row[8]);
			entry.minute_end      = atoi(row[9]);
			entry.hour_end        = atoi(row[10]);
			entry.day_end         = atoi(row[11]);
			entry.month_end       = atoi(row[12]);
			entry.year_end        = atoi(row[13]);
			entry.cron_expression = row[14] ? row[14] : "";
			entry.created_at      = row[15] ? row[15] : "";
			entry.deleted_at      = row[16] ? row[16] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<ServerScheduledEvents> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<ServerScheduledEvents> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ServerScheduledEvents entry{};

			entry.id              = atoi(row[0]);
			entry.description     = row[1] ? row[1] : "";
			entry.event_type      = row[2] ? row[2] : "";
			entry.event_data      = row[3] ? row[3] : "";
			entry.minute_start    = atoi(row[4]);
			entry.hour_start      = atoi(row[5]);
			entry.day_start       = atoi(row[6]);
			entry.month_start     = atoi(row[7]);
			entry.year_start      = atoi(row[8]);
			entry.minute_end      = atoi(row[9]);
			entry.hour_end        = atoi(row[10]);
			entry.day_end         = atoi(row[11]);
			entry.month_end       = atoi(row[12]);
			entry.year_end        = atoi(row[13]);
			entry.cron_expression = row[14] ? row[14] : "";
			entry.created_at      = row[15] ? row[15] : "";
			entry.deleted_at      = row[16] ? row[16] : "";

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

#endif //EQEMU_BASE_SERVER_SCHEDULED_EVENTS_REPOSITORY_H
