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

#ifndef EQEMU_BASE_GOALLISTS_REPOSITORY_H
#define EQEMU_BASE_GOALLISTS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseGoallistsRepository {
public:
	struct Goallists {
		uint32_t listid;
		uint32_t entry;
	};

	static std::string PrimaryKey()
	{
		return std::string("listid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"listid",
			"entry",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"listid",
			"entry",
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
		return std::string("goallists");
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

	static Goallists NewEntity()
	{
		Goallists e{};

		e.listid = 0;
		e.entry  = 0;

		return e;
	}

	static Goallists GetGoallists(
		const std::vector<Goallists> &goallistss,
		int goallists_id
	)
	{
		for (auto &goallists : goallistss) {
			if (goallists.listid == goallists_id) {
				return goallists;
			}
		}

		return NewEntity();
	}

	static Goallists FindOne(
		Database& db,
		int goallists_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				goallists_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Goallists e{};

			e.listid = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.entry  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int goallists_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				goallists_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Goallists &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.listid));
		v.push_back(columns[1] + " = " + std::to_string(e.entry));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.listid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Goallists InsertOne(
		Database& db,
		Goallists e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.listid));
		v.push_back(std::to_string(e.entry));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.listid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Goallists> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.listid));
			v.push_back(std::to_string(e.entry));

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

	static std::vector<Goallists> All(Database& db)
	{
		std::vector<Goallists> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Goallists e{};

			e.listid = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.entry  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Goallists> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Goallists> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Goallists e{};

			e.listid = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.entry  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));

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

#endif //EQEMU_BASE_GOALLISTS_REPOSITORY_H
