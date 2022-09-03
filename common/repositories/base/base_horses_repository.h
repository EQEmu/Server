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

#ifndef EQEMU_BASE_HORSES_REPOSITORY_H
#define EQEMU_BASE_HORSES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseHorsesRepository {
public:
	struct Horses {
		int32_t     id;
		std::string filename;
		int16_t     race;
		int8_t      gender;
		int8_t      texture;
		float       mountspeed;
		std::string notes;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"filename",
			"race",
			"gender",
			"texture",
			"mountspeed",
			"notes",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"filename",
			"race",
			"gender",
			"texture",
			"mountspeed",
			"notes",
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
		return std::string("horses");
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

	static Horses NewEntity()
	{
		Horses e{};

		e.id         = 0;
		e.filename   = "";
		e.race       = 216;
		e.gender     = 0;
		e.texture    = 0;
		e.mountspeed = 0.75;
		e.notes      = "Notes";

		return e;
	}

	static Horses GetHorses(
		const std::vector<Horses> &horsess,
		int horses_id
	)
	{
		for (auto &horses : horsess) {
			if (horses.id == horses_id) {
				return horses;
			}
		}

		return NewEntity();
	}

	static Horses FindOne(
		Database& db,
		int horses_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				horses_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Horses e{};

			e.id         = static_cast<int32_t>(atoi(row[0]));
			e.filename   = row[1] ? row[1] : "";
			e.race       = static_cast<int16_t>(atoi(row[2]));
			e.gender     = static_cast<int8_t>(atoi(row[3]));
			e.texture    = static_cast<int8_t>(atoi(row[4]));
			e.mountspeed = strtof(row[5], nullptr);
			e.notes      = row[6] ? row[6] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int horses_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				horses_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Horses &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = '" + Strings::Escape(e.filename) + "'");
		v.push_back(columns[2] + " = " + std::to_string(e.race));
		v.push_back(columns[3] + " = " + std::to_string(e.gender));
		v.push_back(columns[4] + " = " + std::to_string(e.texture));
		v.push_back(columns[5] + " = " + std::to_string(e.mountspeed));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.notes) + "'");

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

	static Horses InsertOne(
		Database& db,
		Horses e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.filename) + "'");
		v.push_back(std::to_string(e.race));
		v.push_back(std::to_string(e.gender));
		v.push_back(std::to_string(e.texture));
		v.push_back(std::to_string(e.mountspeed));
		v.push_back("'" + Strings::Escape(e.notes) + "'");

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
		const std::vector<Horses> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back("'" + Strings::Escape(e.filename) + "'");
			v.push_back(std::to_string(e.race));
			v.push_back(std::to_string(e.gender));
			v.push_back(std::to_string(e.texture));
			v.push_back(std::to_string(e.mountspeed));
			v.push_back("'" + Strings::Escape(e.notes) + "'");

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

	static std::vector<Horses> All(Database& db)
	{
		std::vector<Horses> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Horses e{};

			e.id         = static_cast<int32_t>(atoi(row[0]));
			e.filename   = row[1] ? row[1] : "";
			e.race       = static_cast<int16_t>(atoi(row[2]));
			e.gender     = static_cast<int8_t>(atoi(row[3]));
			e.texture    = static_cast<int8_t>(atoi(row[4]));
			e.mountspeed = strtof(row[5], nullptr);
			e.notes      = row[6] ? row[6] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Horses> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Horses> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Horses e{};

			e.id         = static_cast<int32_t>(atoi(row[0]));
			e.filename   = row[1] ? row[1] : "";
			e.race       = static_cast<int16_t>(atoi(row[2]));
			e.gender     = static_cast<int8_t>(atoi(row[3]));
			e.texture    = static_cast<int8_t>(atoi(row[4]));
			e.mountspeed = strtof(row[5], nullptr);
			e.notes      = row[6] ? row[6] : "";

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

#endif //EQEMU_BASE_HORSES_REPOSITORY_H
