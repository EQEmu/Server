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

#ifndef EQEMU_BASE_EXPEDITION_LOCKOUTS_REPOSITORY_H
#define EQEMU_BASE_EXPEDITION_LOCKOUTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseExpeditionLockoutsRepository {
public:
	struct ExpeditionLockouts {
		int         id;
		int         expedition_id;
		std::string event_name;
		time_t      expire_time;
		int         duration;
		std::string from_expedition_uuid;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"expedition_id",
			"event_name",
			"expire_time",
			"duration",
			"from_expedition_uuid",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"expedition_id",
			"event_name",
			"UNIX_TIMESTAMP(expire_time)",
			"duration",
			"from_expedition_uuid",
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
		return std::string("expedition_lockouts");
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

	static ExpeditionLockouts NewEntity()
	{
		ExpeditionLockouts e{};

		e.id                   = 0;
		e.expedition_id        = 0;
		e.event_name           = "";
		e.expire_time          = std::time(nullptr);
		e.duration             = 0;
		e.from_expedition_uuid = "";

		return e;
	}

	static ExpeditionLockouts GetExpeditionLockoutse(
		const std::vector<ExpeditionLockouts> &expedition_lockoutss,
		int expedition_lockouts_id
	)
	{
		for (auto &expedition_lockouts : expedition_lockoutss) {
			if (expedition_lockouts.id == expedition_lockouts_id) {
				return expedition_lockouts;
			}
		}

		return NewEntity();
	}

	static ExpeditionLockouts FindOne(
		Database& db,
		int expedition_lockouts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				expedition_lockouts_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			ExpeditionLockouts e{};

			e.id                   = atoi(row[0]);
			e.expedition_id        = atoi(row[1]);
			e.event_name           = row[2] ? row[2] : "";
			e.expire_time          = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.duration             = atoi(row[4]);
			e.from_expedition_uuid = row[5] ? row[5] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int expedition_lockouts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				expedition_lockouts_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		ExpeditionLockouts expedition_lockouts_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(expedition_lockouts_e.expedition_id));
		update_values.push_back(columns[2] + " = '" + Strings::Escape(expedition_lockouts_e.event_name) + "'");
		update_values.push_back(columns[3] + " = FROM_UNIXTIME(" + (expedition_lockouts_e.expire_time > 0 ? std::to_string(expedition_lockouts_e.expire_time) : "null") + ")");
		update_values.push_back(columns[4] + " = " + std::to_string(expedition_lockouts_e.duration));
		update_values.push_back(columns[5] + " = '" + Strings::Escape(expedition_lockouts_e.from_expedition_uuid) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				expedition_lockouts_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static ExpeditionLockouts InsertOne(
		Database& db,
		ExpeditionLockouts expedition_lockouts_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(expedition_lockouts_e.id));
		insert_values.push_back(std::to_string(expedition_lockouts_e.expedition_id));
		insert_values.push_back("'" + Strings::Escape(expedition_lockouts_e.event_name) + "'");
		insert_values.push_back("FROM_UNIXTIME(" + (expedition_lockouts_e.expire_time > 0 ? std::to_string(expedition_lockouts_e.expire_time) : "null") + ")");
		insert_values.push_back(std::to_string(expedition_lockouts_e.duration));
		insert_values.push_back("'" + Strings::Escape(expedition_lockouts_e.from_expedition_uuid) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			expedition_lockouts_e.id = results.LastInsertedID();
			return expedition_lockouts_e;
		}

		expedition_lockouts_e = NewEntity();

		return expedition_lockouts_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<ExpeditionLockouts> expedition_lockouts_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &expedition_lockouts_e: expedition_lockouts_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(expedition_lockouts_e.id));
			insert_values.push_back(std::to_string(expedition_lockouts_e.expedition_id));
			insert_values.push_back("'" + Strings::Escape(expedition_lockouts_e.event_name) + "'");
			insert_values.push_back("FROM_UNIXTIME(" + (expedition_lockouts_e.expire_time > 0 ? std::to_string(expedition_lockouts_e.expire_time) : "null") + ")");
			insert_values.push_back(std::to_string(expedition_lockouts_e.duration));
			insert_values.push_back("'" + Strings::Escape(expedition_lockouts_e.from_expedition_uuid) + "'");

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

	static std::vector<ExpeditionLockouts> All(Database& db)
	{
		std::vector<ExpeditionLockouts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ExpeditionLockouts e{};

			e.id                   = atoi(row[0]);
			e.expedition_id        = atoi(row[1]);
			e.event_name           = row[2] ? row[2] : "";
			e.expire_time          = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.duration             = atoi(row[4]);
			e.from_expedition_uuid = row[5] ? row[5] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<ExpeditionLockouts> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<ExpeditionLockouts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			ExpeditionLockouts e{};

			e.id                   = atoi(row[0]);
			e.expedition_id        = atoi(row[1]);
			e.event_name           = row[2] ? row[2] : "";
			e.expire_time          = strtoll(row[3] ? row[3] : "-1", nullptr, 10);
			e.duration             = atoi(row[4]);
			e.from_expedition_uuid = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_EXPEDITION_LOCKOUTS_REPOSITORY_H
