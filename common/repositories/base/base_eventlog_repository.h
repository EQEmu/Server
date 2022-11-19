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

#ifndef EQEMU_BASE_EVENTLOG_REPOSITORY_H
#define EQEMU_BASE_EVENTLOG_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseEventlogRepository {
public:
	struct Eventlog {
		uint32_t    id;
		std::string accountname;
		uint32_t    accountid;
		int32_t     status;
		std::string charname;
		std::string target;
		std::string time;
		std::string descriptiontype;
		std::string description;
		int32_t     event_nid;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"accountname",
			"accountid",
			"status",
			"charname",
			"target",
			"time",
			"descriptiontype",
			"description",
			"event_nid",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"accountname",
			"accountid",
			"status",
			"charname",
			"target",
			"time",
			"descriptiontype",
			"description",
			"event_nid",
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
		return std::string("eventlog");
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

	static Eventlog NewEntity()
	{
		Eventlog e{};

		e.id              = 0;
		e.accountname     = "";
		e.accountid       = 0;
		e.status          = 0;
		e.charname        = "";
		e.target          = "None";
		e.time            = std::time(nullptr);
		e.descriptiontype = "";
		e.description     = "";
		e.event_nid       = 0;

		return e;
	}

	static Eventlog GetEventlog(
		const std::vector<Eventlog> &eventlogs,
		int eventlog_id
	)
	{
		for (auto &eventlog : eventlogs) {
			if (eventlog.id == eventlog_id) {
				return eventlog;
			}
		}

		return NewEntity();
	}

	static Eventlog FindOne(
		Database& db,
		int eventlog_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				eventlog_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Eventlog e{};

			e.id              = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.accountname     = row[1] ? row[1] : "";
			e.accountid       = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.status          = static_cast<int32_t>(atoi(row[3]));
			e.charname        = row[4] ? row[4] : "";
			e.target          = row[5] ? row[5] : "";
			e.time            = row[6] ? row[6] : "";
			e.descriptiontype = row[7] ? row[7] : "";
			e.description     = row[8] ? row[8] : "";
			e.event_nid       = static_cast<int32_t>(atoi(row[9]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int eventlog_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				eventlog_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Eventlog &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.accountname) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.accountid));
		v.push_back(columns[3] + " = " + std::to_string(e.status));
		v.push_back(columns[4] + " = '" + Strings::Escape(e.charname) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.target) + "'");
		v.push_back(columns[6] + " = '" + Strings::Escape(e.time) + "'");
		v.push_back(columns[7] + " = '" + Strings::Escape(e.descriptiontype) + "'");
		v.push_back(columns[8] + " = '" + Strings::Escape(e.description) + "'");
		v.push_back(columns[9] + " = " + std::to_string(e.event_nid));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Eventlog InsertOne(
		Database& db,
		Eventlog e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.accountname) + "'");
		v.push_back(std::to_string(e.accountid));
		v.push_back(std::to_string(e.status));
		v.push_back("'" + Strings::Escape(e.charname) + "'");
		v.push_back("'" + Strings::Escape(e.target) + "'");
		v.push_back("'" + Strings::Escape(e.time) + "'");
		v.push_back("'" + Strings::Escape(e.descriptiontype) + "'");
		v.push_back("'" + Strings::Escape(e.description) + "'");
		v.push_back(std::to_string(e.event_nid));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Eventlog> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.accountname) + "'");
			v.push_back(std::to_string(e.accountid));
			v.push_back(std::to_string(e.status));
			v.push_back("'" + Strings::Escape(e.charname) + "'");
			v.push_back("'" + Strings::Escape(e.target) + "'");
			v.push_back("'" + Strings::Escape(e.time) + "'");
			v.push_back("'" + Strings::Escape(e.descriptiontype) + "'");
			v.push_back("'" + Strings::Escape(e.description) + "'");
			v.push_back(std::to_string(e.event_nid));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Eventlog> All(Database& db)
	{
		std::vector<Eventlog> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Eventlog e{};

			e.id              = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.accountname     = row[1] ? row[1] : "";
			e.accountid       = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.status          = static_cast<int32_t>(atoi(row[3]));
			e.charname        = row[4] ? row[4] : "";
			e.target          = row[5] ? row[5] : "";
			e.time            = row[6] ? row[6] : "";
			e.descriptiontype = row[7] ? row[7] : "";
			e.description     = row[8] ? row[8] : "";
			e.event_nid       = static_cast<int32_t>(atoi(row[9]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Eventlog> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Eventlog> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Eventlog e{};

			e.id              = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.accountname     = row[1] ? row[1] : "";
			e.accountid       = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));
			e.status          = static_cast<int32_t>(atoi(row[3]));
			e.charname        = row[4] ? row[4] : "";
			e.target          = row[5] ? row[5] : "";
			e.time            = row[6] ? row[6] : "";
			e.descriptiontype = row[7] ? row[7] : "";
			e.description     = row[8] ? row[8] : "";
			e.event_nid       = static_cast<int32_t>(atoi(row[9]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_EVENTLOG_REPOSITORY_H
