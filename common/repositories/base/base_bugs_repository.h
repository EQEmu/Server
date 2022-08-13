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

#ifndef EQEMU_BASE_BUGS_REPOSITORY_H
#define EQEMU_BASE_BUGS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseBugsRepository {
public:
	struct Bugs {
		int         id;
		std::string zone;
		std::string name;
		std::string ui;
		float       x;
		float       y;
		float       z;
		std::string type;
		int         flag;
		std::string target;
		std::string bug;
		std::string date;
		int         status;
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
			"name",
			"ui",
			"x",
			"y",
			"z",
			"type",
			"flag",
			"target",
			"bug",
			"date",
			"status",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone",
			"name",
			"ui",
			"x",
			"y",
			"z",
			"type",
			"flag",
			"target",
			"bug",
			"date",
			"status",
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
		return std::string("bugs");
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

	static Bugs NewEntity()
	{
		Bugs e{};

		e.id     = 0;
		e.zone   = "";
		e.name   = "";
		e.ui     = "";
		e.x      = 0;
		e.y      = 0;
		e.z      = 0;
		e.type   = "";
		e.flag   = 0;
		e.target = "";
		e.bug    = "";
		e.date   = 0;
		e.status = 0;

		return e;
	}

	static Bugs GetBugse(
		const std::vector<Bugs> &bugss,
		int bugs_id
	)
	{
		for (auto &bugs : bugss) {
			if (bugs.id == bugs_id) {
				return bugs;
			}
		}

		return NewEntity();
	}

	static Bugs FindOne(
		Database& db,
		int bugs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				bugs_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Bugs e{};

			e.id     = atoi(row[0]);
			e.zone   = row[1] ? row[1] : "";
			e.name   = row[2] ? row[2] : "";
			e.ui     = row[3] ? row[3] : "";
			e.x      = static_cast<float>(atof(row[4]));
			e.y      = static_cast<float>(atof(row[5]));
			e.z      = static_cast<float>(atof(row[6]));
			e.type   = row[7] ? row[7] : "";
			e.flag   = atoi(row[8]);
			e.target = row[9] ? row[9] : "";
			e.bug    = row[10] ? row[10] : "";
			e.date   = row[11] ? row[11] : "";
			e.status = atoi(row[12]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int bugs_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				bugs_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Bugs bugs_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + Strings::Escape(bugs_e.zone) + "'");
		update_values.push_back(columns[2] + " = '" + Strings::Escape(bugs_e.name) + "'");
		update_values.push_back(columns[3] + " = '" + Strings::Escape(bugs_e.ui) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(bugs_e.x));
		update_values.push_back(columns[5] + " = " + std::to_string(bugs_e.y));
		update_values.push_back(columns[6] + " = " + std::to_string(bugs_e.z));
		update_values.push_back(columns[7] + " = '" + Strings::Escape(bugs_e.type) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(bugs_e.flag));
		update_values.push_back(columns[9] + " = '" + Strings::Escape(bugs_e.target) + "'");
		update_values.push_back(columns[10] + " = '" + Strings::Escape(bugs_e.bug) + "'");
		update_values.push_back(columns[11] + " = '" + Strings::Escape(bugs_e.date) + "'");
		update_values.push_back(columns[12] + " = " + std::to_string(bugs_e.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				bugs_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Bugs InsertOne(
		Database& db,
		Bugs bugs_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(bugs_e.id));
		insert_values.push_back("'" + Strings::Escape(bugs_e.zone) + "'");
		insert_values.push_back("'" + Strings::Escape(bugs_e.name) + "'");
		insert_values.push_back("'" + Strings::Escape(bugs_e.ui) + "'");
		insert_values.push_back(std::to_string(bugs_e.x));
		insert_values.push_back(std::to_string(bugs_e.y));
		insert_values.push_back(std::to_string(bugs_e.z));
		insert_values.push_back("'" + Strings::Escape(bugs_e.type) + "'");
		insert_values.push_back(std::to_string(bugs_e.flag));
		insert_values.push_back("'" + Strings::Escape(bugs_e.target) + "'");
		insert_values.push_back("'" + Strings::Escape(bugs_e.bug) + "'");
		insert_values.push_back("'" + Strings::Escape(bugs_e.date) + "'");
		insert_values.push_back(std::to_string(bugs_e.status));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			bugs_e.id = results.LastInsertedID();
			return bugs_e;
		}

		bugs_e = NewEntity();

		return bugs_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Bugs> bugs_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &bugs_e: bugs_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(bugs_e.id));
			insert_values.push_back("'" + Strings::Escape(bugs_e.zone) + "'");
			insert_values.push_back("'" + Strings::Escape(bugs_e.name) + "'");
			insert_values.push_back("'" + Strings::Escape(bugs_e.ui) + "'");
			insert_values.push_back(std::to_string(bugs_e.x));
			insert_values.push_back(std::to_string(bugs_e.y));
			insert_values.push_back(std::to_string(bugs_e.z));
			insert_values.push_back("'" + Strings::Escape(bugs_e.type) + "'");
			insert_values.push_back(std::to_string(bugs_e.flag));
			insert_values.push_back("'" + Strings::Escape(bugs_e.target) + "'");
			insert_values.push_back("'" + Strings::Escape(bugs_e.bug) + "'");
			insert_values.push_back("'" + Strings::Escape(bugs_e.date) + "'");
			insert_values.push_back(std::to_string(bugs_e.status));

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

	static std::vector<Bugs> All(Database& db)
	{
		std::vector<Bugs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Bugs e{};

			e.id     = atoi(row[0]);
			e.zone   = row[1] ? row[1] : "";
			e.name   = row[2] ? row[2] : "";
			e.ui     = row[3] ? row[3] : "";
			e.x      = static_cast<float>(atof(row[4]));
			e.y      = static_cast<float>(atof(row[5]));
			e.z      = static_cast<float>(atof(row[6]));
			e.type   = row[7] ? row[7] : "";
			e.flag   = atoi(row[8]);
			e.target = row[9] ? row[9] : "";
			e.bug    = row[10] ? row[10] : "";
			e.date   = row[11] ? row[11] : "";
			e.status = atoi(row[12]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Bugs> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Bugs> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Bugs e{};

			e.id     = atoi(row[0]);
			e.zone   = row[1] ? row[1] : "";
			e.name   = row[2] ? row[2] : "";
			e.ui     = row[3] ? row[3] : "";
			e.x      = static_cast<float>(atof(row[4]));
			e.y      = static_cast<float>(atof(row[5]));
			e.z      = static_cast<float>(atof(row[6]));
			e.type   = row[7] ? row[7] : "";
			e.flag   = atoi(row[8]);
			e.target = row[9] ? row[9] : "";
			e.bug    = row[10] ? row[10] : "";
			e.date   = row[11] ? row[11] : "";
			e.status = atoi(row[12]);

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

#endif //EQEMU_BASE_BUGS_REPOSITORY_H
