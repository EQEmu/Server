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

#ifndef EQEMU_BASE_LFGUILD_REPOSITORY_H
#define EQEMU_BASE_LFGUILD_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseLfguildRepository {
public:
	struct Lfguild {
		int         type;
		std::string name;
		std::string comment;
		int         fromlevel;
		int         tolevel;
		int         classes;
		int         aacount;
		int         timezone;
		int         timeposted;
	};

	static std::string PrimaryKey()
	{
		return std::string("type");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"type",
			"name",
			"comment",
			"fromlevel",
			"tolevel",
			"classes",
			"aacount",
			"timezone",
			"timeposted",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"type",
			"name",
			"comment",
			"fromlevel",
			"tolevel",
			"classes",
			"aacount",
			"timezone",
			"timeposted",
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
		return std::string("lfguild");
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

	static Lfguild NewEntity()
	{
		Lfguild e{};

		e.type       = 0;
		e.name       = "";
		e.comment    = "";
		e.fromlevel  = 0;
		e.tolevel    = 0;
		e.classes    = 0;
		e.aacount    = 0;
		e.timezone   = 0;
		e.timeposted = 0;

		return e;
	}

	static Lfguild GetLfguilde(
		const std::vector<Lfguild> &lfguilds,
		int lfguild_id
	)
	{
		for (auto &lfguild : lfguilds) {
			if (lfguild.type == lfguild_id) {
				return lfguild;
			}
		}

		return NewEntity();
	}

	static Lfguild FindOne(
		Database& db,
		int lfguild_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				lfguild_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Lfguild e{};

			e.type       = atoi(row[0]);
			e.name       = row[1] ? row[1] : "";
			e.comment    = row[2] ? row[2] : "";
			e.fromlevel  = atoi(row[3]);
			e.tolevel    = atoi(row[4]);
			e.classes    = atoi(row[5]);
			e.aacount    = atoi(row[6]);
			e.timezone   = atoi(row[7]);
			e.timeposted = atoi(row[8]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int lfguild_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				lfguild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		Lfguild lfguild_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(lfguild_e.type));
		update_values.push_back(columns[1] + " = '" + Strings::Escape(lfguild_e.name) + "'");
		update_values.push_back(columns[2] + " = '" + Strings::Escape(lfguild_e.comment) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(lfguild_e.fromlevel));
		update_values.push_back(columns[4] + " = " + std::to_string(lfguild_e.tolevel));
		update_values.push_back(columns[5] + " = " + std::to_string(lfguild_e.classes));
		update_values.push_back(columns[6] + " = " + std::to_string(lfguild_e.aacount));
		update_values.push_back(columns[7] + " = " + std::to_string(lfguild_e.timezone));
		update_values.push_back(columns[8] + " = " + std::to_string(lfguild_e.timeposted));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				lfguild_e.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Lfguild InsertOne(
		Database& db,
		Lfguild lfguild_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(lfguild_e.type));
		insert_values.push_back("'" + Strings::Escape(lfguild_e.name) + "'");
		insert_values.push_back("'" + Strings::Escape(lfguild_e.comment) + "'");
		insert_values.push_back(std::to_string(lfguild_e.fromlevel));
		insert_values.push_back(std::to_string(lfguild_e.tolevel));
		insert_values.push_back(std::to_string(lfguild_e.classes));
		insert_values.push_back(std::to_string(lfguild_e.aacount));
		insert_values.push_back(std::to_string(lfguild_e.timezone));
		insert_values.push_back(std::to_string(lfguild_e.timeposted));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			lfguild_e.type = results.LastInsertedID();
			return lfguild_e;
		}

		lfguild_e = NewEntity();

		return lfguild_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<Lfguild> lfguild_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &lfguild_e: lfguild_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(lfguild_e.type));
			insert_values.push_back("'" + Strings::Escape(lfguild_e.name) + "'");
			insert_values.push_back("'" + Strings::Escape(lfguild_e.comment) + "'");
			insert_values.push_back(std::to_string(lfguild_e.fromlevel));
			insert_values.push_back(std::to_string(lfguild_e.tolevel));
			insert_values.push_back(std::to_string(lfguild_e.classes));
			insert_values.push_back(std::to_string(lfguild_e.aacount));
			insert_values.push_back(std::to_string(lfguild_e.timezone));
			insert_values.push_back(std::to_string(lfguild_e.timeposted));

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

	static std::vector<Lfguild> All(Database& db)
	{
		std::vector<Lfguild> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lfguild e{};

			e.type       = atoi(row[0]);
			e.name       = row[1] ? row[1] : "";
			e.comment    = row[2] ? row[2] : "";
			e.fromlevel  = atoi(row[3]);
			e.tolevel    = atoi(row[4]);
			e.classes    = atoi(row[5]);
			e.aacount    = atoi(row[6]);
			e.timezone   = atoi(row[7]);
			e.timeposted = atoi(row[8]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Lfguild> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<Lfguild> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Lfguild e{};

			e.type       = atoi(row[0]);
			e.name       = row[1] ? row[1] : "";
			e.comment    = row[2] ? row[2] : "";
			e.fromlevel  = atoi(row[3]);
			e.tolevel    = atoi(row[4]);
			e.classes    = atoi(row[5]);
			e.aacount    = atoi(row[6]);
			e.timezone   = atoi(row[7]);
			e.timeposted = atoi(row[8]);

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

#endif //EQEMU_BASE_LFGUILD_REPOSITORY_H
