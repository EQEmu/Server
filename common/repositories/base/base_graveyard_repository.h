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

#ifndef EQEMU_BASE_GRAVEYARD_REPOSITORY_H
#define EQEMU_BASE_GRAVEYARD_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseGraveyardRepository {
public:
	struct Graveyard {
		int   id;
		int   zone_id;
		float x;
		float y;
		float z;
		float heading;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone_id",
			"x",
			"y",
			"z",
			"heading",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"zone_id",
			"x",
			"y",
			"z",
			"heading",
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
		return std::string("graveyard");
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

	static Graveyard NewEntity()
	{
		Graveyard e{};

		e.id      = 0;
		e.zone_id = 0;
		e.x       = 0;
		e.y       = 0;
		e.z       = 0;
		e.heading = 0;

		return e;
	}

	static Graveyard GetGraveyarde(
		const std::vector<Graveyard> &graveyards,
		int graveyard_id
	)
	{
		for (auto &graveyard : graveyards) {
			if (graveyard.id == graveyard_id) {
				return graveyard;
			}
		}

		return NewEntity();
	}

	static Graveyard FindOne(
		Database& db,
		int graveyard_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				graveyard_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Graveyard e{};

			e.id      = atoi(row[0]);
			e.zone_id = atoi(row[1]);
			e.x       = static_cast<float>(atof(row[2]));
			e.y       = static_cast<float>(atof(row[3]));
			e.z       = static_cast<float>(atof(row[4]));
			e.heading = static_cast<float>(atof(row[5]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int graveyard_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				graveyard_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Graveyard graveyard_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(graveyard_e.zone_id));
		update_values.push_back(columns[2] + " = " + std::to_string(graveyard_e.x));
		update_values.push_back(columns[3] + " = " + std::to_string(graveyard_e.y));
		update_values.push_back(columns[4] + " = " + std::to_string(graveyard_e.z));
		update_values.push_back(columns[5] + " = " + std::to_string(graveyard_e.heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				graveyard_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Graveyard InsertOne(
		Database& db,
		Graveyard graveyard_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(graveyard_e.id));
		insert_values.push_back(std::to_string(graveyard_e.zone_id));
		insert_values.push_back(std::to_string(graveyard_e.x));
		insert_values.push_back(std::to_string(graveyard_e.y));
		insert_values.push_back(std::to_string(graveyard_e.z));
		insert_values.push_back(std::to_string(graveyard_e.heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			graveyard_e.id = results.LastInsertedID();
			return graveyard_e;
		}

		graveyard_e = NewEntity();

		return graveyard_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Graveyard> graveyard_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &graveyard_e: graveyard_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(graveyard_e.id));
			insert_values.push_back(std::to_string(graveyard_e.zone_id));
			insert_values.push_back(std::to_string(graveyard_e.x));
			insert_values.push_back(std::to_string(graveyard_e.y));
			insert_values.push_back(std::to_string(graveyard_e.z));
			insert_values.push_back(std::to_string(graveyard_e.heading));

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

	static std::vector<Graveyard> All(Database& db)
	{
		std::vector<Graveyard> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Graveyard e{};

			e.id      = atoi(row[0]);
			e.zone_id = atoi(row[1]);
			e.x       = static_cast<float>(atof(row[2]));
			e.y       = static_cast<float>(atof(row[3]));
			e.z       = static_cast<float>(atof(row[4]));
			e.heading = static_cast<float>(atof(row[5]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Graveyard> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Graveyard> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Graveyard e{};

			e.id      = atoi(row[0]);
			e.zone_id = atoi(row[1]);
			e.x       = static_cast<float>(atof(row[2]));
			e.y       = static_cast<float>(atof(row[3]));
			e.z       = static_cast<float>(atof(row[4]));
			e.heading = static_cast<float>(atof(row[5]));

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

#endif //EQEMU_BASE_GRAVEYARD_REPOSITORY_H
