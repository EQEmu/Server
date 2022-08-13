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
		int         id;
		std::string filename;
		int         race;
		int         gender;
		int         texture;
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

	static Horses GetHorsese(
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

			e.id         = atoi(row[0]);
			e.filename   = row[1] ? row[1] : "";
			e.race       = atoi(row[2]);
			e.gender     = atoi(row[3]);
			e.texture    = atoi(row[4]);
			e.mountspeed = static_cast<float>(atof(row[5]));
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
		Horses horses_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + Strings::Escape(horses_e.filename) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(horses_e.race));
		update_values.push_back(columns[3] + " = " + std::to_string(horses_e.gender));
		update_values.push_back(columns[4] + " = " + std::to_string(horses_e.texture));
		update_values.push_back(columns[5] + " = " + std::to_string(horses_e.mountspeed));
		update_values.push_back(columns[6] + " = '" + Strings::Escape(horses_e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				horses_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Horses InsertOne(
		Database& db,
		Horses horses_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(horses_e.id));
		insert_values.push_back("'" + Strings::Escape(horses_e.filename) + "'");
		insert_values.push_back(std::to_string(horses_e.race));
		insert_values.push_back(std::to_string(horses_e.gender));
		insert_values.push_back(std::to_string(horses_e.texture));
		insert_values.push_back(std::to_string(horses_e.mountspeed));
		insert_values.push_back("'" + Strings::Escape(horses_e.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			horses_e.id = results.LastInsertedID();
			return horses_e;
		}

		horses_e = NewEntity();

		return horses_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Horses> horses_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &horses_e: horses_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(horses_e.id));
			insert_values.push_back("'" + Strings::Escape(horses_e.filename) + "'");
			insert_values.push_back(std::to_string(horses_e.race));
			insert_values.push_back(std::to_string(horses_e.gender));
			insert_values.push_back(std::to_string(horses_e.texture));
			insert_values.push_back(std::to_string(horses_e.mountspeed));
			insert_values.push_back("'" + Strings::Escape(horses_e.notes) + "'");

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

			e.id         = atoi(row[0]);
			e.filename   = row[1] ? row[1] : "";
			e.race       = atoi(row[2]);
			e.gender     = atoi(row[3]);
			e.texture    = atoi(row[4]);
			e.mountspeed = static_cast<float>(atof(row[5]));
			e.notes      = row[6] ? row[6] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Horses> GetWhere(Database& db, std::string where_filter)
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

			e.id         = atoi(row[0]);
			e.filename   = row[1] ? row[1] : "";
			e.race       = atoi(row[2]);
			e.gender     = atoi(row[3]);
			e.texture    = atoi(row[4]);
			e.mountspeed = static_cast<float>(atof(row[5]));
			e.notes      = row[6] ? row[6] : "";

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

#endif //EQEMU_BASE_HORSES_REPOSITORY_H
