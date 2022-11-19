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

#ifndef EQEMU_BASE_HACKERS_REPOSITORY_H
#define EQEMU_BASE_HACKERS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseHackersRepository {
public:
	struct Hackers {
		int32_t     id;
		std::string account;
		std::string name;
		std::string hacked;
		std::string zone;
		std::string date;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account",
			"name",
			"hacked",
			"zone",
			"date",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account",
			"name",
			"hacked",
			"zone",
			"date",
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
		return std::string("hackers");
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

	static Hackers NewEntity()
	{
		Hackers e{};

		e.id      = 0;
		e.account = "";
		e.name    = "";
		e.hacked  = "";
		e.zone    = "";
		e.date    = std::time(nullptr);

		return e;
	}

	static Hackers GetHackers(
		const std::vector<Hackers> &hackerss,
		int hackers_id
	)
	{
		for (auto &hackers : hackerss) {
			if (hackers.id == hackers_id) {
				return hackers;
			}
		}

		return NewEntity();
	}

	static Hackers FindOne(
		Database& db,
		int hackers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				hackers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Hackers e{};

			e.id      = static_cast<int32_t>(atoi(row[0]));
			e.account = row[1] ? row[1] : "";
			e.name    = row[2] ? row[2] : "";
			e.hacked  = row[3] ? row[3] : "";
			e.zone    = row[4] ? row[4] : "";
			e.date    = row[5] ? row[5] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int hackers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				hackers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Hackers &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.account) + "'");
		v.push_back(columns[2] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[3] + " = '" + Strings::Escape(e.hacked) + "'");
		v.push_back(columns[4] + " = '" + Strings::Escape(e.zone) + "'");
		v.push_back(columns[5] + " = '" + Strings::Escape(e.date) + "'");

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

	static Hackers InsertOne(
		Database& db,
		Hackers e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.account) + "'");
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back("'" + Strings::Escape(e.hacked) + "'");
		v.push_back("'" + Strings::Escape(e.zone) + "'");
		v.push_back("'" + Strings::Escape(e.date) + "'");

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
		const std::vector<Hackers> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.account) + "'");
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back("'" + Strings::Escape(e.hacked) + "'");
			v.push_back("'" + Strings::Escape(e.zone) + "'");
			v.push_back("'" + Strings::Escape(e.date) + "'");

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

	static std::vector<Hackers> All(Database& db)
	{
		std::vector<Hackers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Hackers e{};

			e.id      = static_cast<int32_t>(atoi(row[0]));
			e.account = row[1] ? row[1] : "";
			e.name    = row[2] ? row[2] : "";
			e.hacked  = row[3] ? row[3] : "";
			e.zone    = row[4] ? row[4] : "";
			e.date    = row[5] ? row[5] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Hackers> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Hackers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Hackers e{};

			e.id      = static_cast<int32_t>(atoi(row[0]));
			e.account = row[1] ? row[1] : "";
			e.name    = row[2] ? row[2] : "";
			e.hacked  = row[3] ? row[3] : "";
			e.zone    = row[4] ? row[4] : "";
			e.date    = row[5] ? row[5] : "";

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

#endif //EQEMU_BASE_HACKERS_REPOSITORY_H
