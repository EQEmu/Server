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
#include "../../string_util.h"
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
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
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
		Horses entry{};

		entry.id         = 0;
		entry.filename   = "";
		entry.race       = 216;
		entry.gender     = 0;
		entry.texture    = 0;
		entry.mountspeed = 0.75;
		entry.notes      = "Notes";

		return entry;
	}

	static Horses GetHorsesEntry(
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
			Horses entry{};

			entry.id         = atoi(row[0]);
			entry.filename   = row[1] ? row[1] : "";
			entry.race       = atoi(row[2]);
			entry.gender     = atoi(row[3]);
			entry.texture    = atoi(row[4]);
			entry.mountspeed = static_cast<float>(atof(row[5]));
			entry.notes      = row[6] ? row[6] : "";

			return entry;
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
		Horses horses_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(horses_entry.filename) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(horses_entry.race));
		update_values.push_back(columns[3] + " = " + std::to_string(horses_entry.gender));
		update_values.push_back(columns[4] + " = " + std::to_string(horses_entry.texture));
		update_values.push_back(columns[5] + " = " + std::to_string(horses_entry.mountspeed));
		update_values.push_back(columns[6] + " = '" + EscapeString(horses_entry.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				horses_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Horses InsertOne(
		Database& db,
		Horses horses_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(horses_entry.id));
		insert_values.push_back("'" + EscapeString(horses_entry.filename) + "'");
		insert_values.push_back(std::to_string(horses_entry.race));
		insert_values.push_back(std::to_string(horses_entry.gender));
		insert_values.push_back(std::to_string(horses_entry.texture));
		insert_values.push_back(std::to_string(horses_entry.mountspeed));
		insert_values.push_back("'" + EscapeString(horses_entry.notes) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			horses_entry.id = results.LastInsertedID();
			return horses_entry;
		}

		horses_entry = NewEntity();

		return horses_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Horses> horses_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &horses_entry: horses_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(horses_entry.id));
			insert_values.push_back("'" + EscapeString(horses_entry.filename) + "'");
			insert_values.push_back(std::to_string(horses_entry.race));
			insert_values.push_back(std::to_string(horses_entry.gender));
			insert_values.push_back(std::to_string(horses_entry.texture));
			insert_values.push_back(std::to_string(horses_entry.mountspeed));
			insert_values.push_back("'" + EscapeString(horses_entry.notes) + "'");

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
			Horses entry{};

			entry.id         = atoi(row[0]);
			entry.filename   = row[1] ? row[1] : "";
			entry.race       = atoi(row[2]);
			entry.gender     = atoi(row[3]);
			entry.texture    = atoi(row[4]);
			entry.mountspeed = static_cast<float>(atof(row[5]));
			entry.notes      = row[6] ? row[6] : "";

			all_entries.push_back(entry);
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
			Horses entry{};

			entry.id         = atoi(row[0]);
			entry.filename   = row[1] ? row[1] : "";
			entry.race       = atoi(row[2]);
			entry.gender     = atoi(row[3]);
			entry.texture    = atoi(row[4]);
			entry.mountspeed = static_cast<float>(atof(row[5]));
			entry.notes      = row[6] ? row[6] : "";

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

#endif //EQEMU_BASE_HORSES_REPOSITORY_H
