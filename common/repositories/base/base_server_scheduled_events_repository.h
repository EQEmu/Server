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
#include "../../strings.h"
#include <ctime>

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
		time_t      created_at;
		time_t      deleted_at;
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

	static std::vector<std::string> SelectColumns()
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
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(deleted_at)",
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
		return std::string("server_scheduled_events");
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

	static ServerScheduledEvents NewEntity()
	{
		ServerScheduledEvents e{};

		e.id              = 0;
		e.description     = "";
		e.event_type      = "";
		e.event_data      = "";
		e.minute_start    = 0;
		e.hour_start      = 0;
		e.day_start       = 0;
		e.month_start     = 0;
		e.year_start      = 0;
		e.minute_end      = 0;
		e.hour_end        = 0;
		e.day_end         = 0;
		e.month_end       = 0;
		e.year_end        = 0;
		e.cron_expression = "";
		e.created_at      = 0;
		e.deleted_at      = 0;

		return e;
	}

	static ServerScheduledEvents GetServerScheduledEventse(
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
			ServerScheduledEvents e{};

			e.id              = atoi(row[0]);
			e.description     = row[1] ? row[1] : "";
			e.event_type      = row[2] ? row[2] : "";
			e.event_data      = row[3] ? row[3] : "";
			e.minute_start    = atoi(row[4]);
			e.hour_start      = atoi(row[5]);
			e.day_start       = atoi(row[6]);
			e.month_start     = atoi(row[7]);
			e.year_start      = atoi(row[8]);
			e.minute_end      = atoi(row[9]);
			e.hour_end        = atoi(row[10]);
			e.day_end         = atoi(row[11]);
			e.month_end       = atoi(row[12]);
			e.year_end        = atoi(row[13]);
			e.cron_expression = row[14] ? row[14] : "";
			e.created_at      = strtoll(row[15] ? row[15] : "-1", nullptr, 10);
			e.deleted_at      = strtoll(row[16] ? row[16] : "-1", nullptr, 10);

			return e;
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
		ServerScheduledEvents server_scheduled_events_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + Strings::Escape(server_scheduled_events_e.description) + "'");
		update_values.push_back(columns[2] + " = '" + Strings::Escape(server_scheduled_events_e.event_type) + "'");
		update_values.push_back(columns[3] + " = '" + Strings::Escape(server_scheduled_events_e.event_data) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(server_scheduled_events_e.minute_start));
		update_values.push_back(columns[5] + " = " + std::to_string(server_scheduled_events_e.hour_start));
		update_values.push_back(columns[6] + " = " + std::to_string(server_scheduled_events_e.day_start));
		update_values.push_back(columns[7] + " = " + std::to_string(server_scheduled_events_e.month_start));
		update_values.push_back(columns[8] + " = " + std::to_string(server_scheduled_events_e.year_start));
		update_values.push_back(columns[9] + " = " + std::to_string(server_scheduled_events_e.minute_end));
		update_values.push_back(columns[10] + " = " + std::to_string(server_scheduled_events_e.hour_end));
		update_values.push_back(columns[11] + " = " + std::to_string(server_scheduled_events_e.day_end));
		update_values.push_back(columns[12] + " = " + std::to_string(server_scheduled_events_e.month_end));
		update_values.push_back(columns[13] + " = " + std::to_string(server_scheduled_events_e.year_end));
		update_values.push_back(columns[14] + " = '" + Strings::Escape(server_scheduled_events_e.cron_expression) + "'");
		update_values.push_back(columns[15] + " = FROM_UNIXTIME(" + (server_scheduled_events_e.created_at > 0 ? std::to_string(server_scheduled_events_e.created_at) : "null") + ")");
		update_values.push_back(columns[16] + " = FROM_UNIXTIME(" + (server_scheduled_events_e.deleted_at > 0 ? std::to_string(server_scheduled_events_e.deleted_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				server_scheduled_events_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ServerScheduledEvents InsertOne(
		Database& db,
		ServerScheduledEvents server_scheduled_events_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(server_scheduled_events_e.id));
		insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.description) + "'");
		insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.event_type) + "'");
		insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.event_data) + "'");
		insert_values.push_back(std::to_string(server_scheduled_events_e.minute_start));
		insert_values.push_back(std::to_string(server_scheduled_events_e.hour_start));
		insert_values.push_back(std::to_string(server_scheduled_events_e.day_start));
		insert_values.push_back(std::to_string(server_scheduled_events_e.month_start));
		insert_values.push_back(std::to_string(server_scheduled_events_e.year_start));
		insert_values.push_back(std::to_string(server_scheduled_events_e.minute_end));
		insert_values.push_back(std::to_string(server_scheduled_events_e.hour_end));
		insert_values.push_back(std::to_string(server_scheduled_events_e.day_end));
		insert_values.push_back(std::to_string(server_scheduled_events_e.month_end));
		insert_values.push_back(std::to_string(server_scheduled_events_e.year_end));
		insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.cron_expression) + "'");
		insert_values.push_back("FROM_UNIXTIME(" + (server_scheduled_events_e.created_at > 0 ? std::to_string(server_scheduled_events_e.created_at) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (server_scheduled_events_e.deleted_at > 0 ? std::to_string(server_scheduled_events_e.deleted_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			server_scheduled_events_e.id = results.LastInsertedID();
			return server_scheduled_events_e;
		}

		server_scheduled_events_e = NewEntity();

		return server_scheduled_events_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<ServerScheduledEvents> server_scheduled_events_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &server_scheduled_events_e: server_scheduled_events_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(server_scheduled_events_e.id));
			insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.description) + "'");
			insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.event_type) + "'");
			insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.event_data) + "'");
			insert_values.push_back(std::to_string(server_scheduled_events_e.minute_start));
			insert_values.push_back(std::to_string(server_scheduled_events_e.hour_start));
			insert_values.push_back(std::to_string(server_scheduled_events_e.day_start));
			insert_values.push_back(std::to_string(server_scheduled_events_e.month_start));
			insert_values.push_back(std::to_string(server_scheduled_events_e.year_start));
			insert_values.push_back(std::to_string(server_scheduled_events_e.minute_end));
			insert_values.push_back(std::to_string(server_scheduled_events_e.hour_end));
			insert_values.push_back(std::to_string(server_scheduled_events_e.day_end));
			insert_values.push_back(std::to_string(server_scheduled_events_e.month_end));
			insert_values.push_back(std::to_string(server_scheduled_events_e.year_end));
			insert_values.push_back("'" + Strings::Escape(server_scheduled_events_e.cron_expression) + "'");
			insert_values.push_back("FROM_UNIXTIME(" + (server_scheduled_events_e.created_at > 0 ? std::to_string(server_scheduled_events_e.created_at) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (server_scheduled_events_e.deleted_at > 0 ? std::to_string(server_scheduled_events_e.deleted_at) : "null") + ")");

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
			ServerScheduledEvents e{};

			e.id              = atoi(row[0]);
			e.description     = row[1] ? row[1] : "";
			e.event_type      = row[2] ? row[2] : "";
			e.event_data      = row[3] ? row[3] : "";
			e.minute_start    = atoi(row[4]);
			e.hour_start      = atoi(row[5]);
			e.day_start       = atoi(row[6]);
			e.month_start     = atoi(row[7]);
			e.year_start      = atoi(row[8]);
			e.minute_end      = atoi(row[9]);
			e.hour_end        = atoi(row[10]);
			e.day_end         = atoi(row[11]);
			e.month_end       = atoi(row[12]);
			e.year_end        = atoi(row[13]);
			e.cron_expression = row[14] ? row[14] : "";
			e.created_at      = strtoll(row[15] ? row[15] : "-1", nullptr, 10);
			e.deleted_at      = strtoll(row[16] ? row[16] : "-1", nullptr, 10);

			all_entries.push_back(e);
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
			ServerScheduledEvents e{};

			e.id              = atoi(row[0]);
			e.description     = row[1] ? row[1] : "";
			e.event_type      = row[2] ? row[2] : "";
			e.event_data      = row[3] ? row[3] : "";
			e.minute_start    = atoi(row[4]);
			e.hour_start      = atoi(row[5]);
			e.day_start       = atoi(row[6]);
			e.month_start     = atoi(row[7]);
			e.year_start      = atoi(row[8]);
			e.minute_end      = atoi(row[9]);
			e.hour_end        = atoi(row[10]);
			e.day_end         = atoi(row[11]);
			e.month_end       = atoi(row[12]);
			e.year_end        = atoi(row[13]);
			e.cron_expression = row[14] ? row[14] : "";
			e.created_at      = strtoll(row[15] ? row[15] : "-1", nullptr, 10);
			e.deleted_at      = strtoll(row[16] ? row[16] : "-1", nullptr, 10);

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

#endif //EQEMU_BASE_SERVER_SCHEDULED_EVENTS_REPOSITORY_H
