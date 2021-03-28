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
#include "../../string_util.h"

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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("lfguild");
	}

	static std::string BaseSelect()
	{
		return fmt::format(
			"SELECT {} FROM {}",
			ColumnsRaw(),
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
		Lfguild entry{};

		entry.type       = 0;
		entry.name       = "";
		entry.comment    = "";
		entry.fromlevel  = 0;
		entry.tolevel    = 0;
		entry.classes    = 0;
		entry.aacount    = 0;
		entry.timezone   = 0;
		entry.timeposted = 0;

		return entry;
	}

	static Lfguild GetLfguildEntry(
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
			Lfguild entry{};

			entry.type       = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";
			entry.comment    = row[2] ? row[2] : "";
			entry.fromlevel  = atoi(row[3]);
			entry.tolevel    = atoi(row[4]);
			entry.classes    = atoi(row[5]);
			entry.aacount    = atoi(row[6]);
			entry.timezone   = atoi(row[7]);
			entry.timeposted = atoi(row[8]);

			return entry;
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
		Lfguild lfguild_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(lfguild_entry.type));
		update_values.push_back(columns[1] + " = '" + EscapeString(lfguild_entry.name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(lfguild_entry.comment) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(lfguild_entry.fromlevel));
		update_values.push_back(columns[4] + " = " + std::to_string(lfguild_entry.tolevel));
		update_values.push_back(columns[5] + " = " + std::to_string(lfguild_entry.classes));
		update_values.push_back(columns[6] + " = " + std::to_string(lfguild_entry.aacount));
		update_values.push_back(columns[7] + " = " + std::to_string(lfguild_entry.timezone));
		update_values.push_back(columns[8] + " = " + std::to_string(lfguild_entry.timeposted));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				lfguild_entry.type
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Lfguild InsertOne(
		Database& db,
		Lfguild lfguild_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(lfguild_entry.type));
		insert_values.push_back("'" + EscapeString(lfguild_entry.name) + "'");
		insert_values.push_back("'" + EscapeString(lfguild_entry.comment) + "'");
		insert_values.push_back(std::to_string(lfguild_entry.fromlevel));
		insert_values.push_back(std::to_string(lfguild_entry.tolevel));
		insert_values.push_back(std::to_string(lfguild_entry.classes));
		insert_values.push_back(std::to_string(lfguild_entry.aacount));
		insert_values.push_back(std::to_string(lfguild_entry.timezone));
		insert_values.push_back(std::to_string(lfguild_entry.timeposted));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			lfguild_entry.type = results.LastInsertedID();
			return lfguild_entry;
		}

		lfguild_entry = NewEntity();

		return lfguild_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Lfguild> lfguild_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &lfguild_entry: lfguild_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(lfguild_entry.type));
			insert_values.push_back("'" + EscapeString(lfguild_entry.name) + "'");
			insert_values.push_back("'" + EscapeString(lfguild_entry.comment) + "'");
			insert_values.push_back(std::to_string(lfguild_entry.fromlevel));
			insert_values.push_back(std::to_string(lfguild_entry.tolevel));
			insert_values.push_back(std::to_string(lfguild_entry.classes));
			insert_values.push_back(std::to_string(lfguild_entry.aacount));
			insert_values.push_back(std::to_string(lfguild_entry.timezone));
			insert_values.push_back(std::to_string(lfguild_entry.timeposted));

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
			Lfguild entry{};

			entry.type       = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";
			entry.comment    = row[2] ? row[2] : "";
			entry.fromlevel  = atoi(row[3]);
			entry.tolevel    = atoi(row[4]);
			entry.classes    = atoi(row[5]);
			entry.aacount    = atoi(row[6]);
			entry.timezone   = atoi(row[7]);
			entry.timeposted = atoi(row[8]);

			all_entries.push_back(entry);
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
			Lfguild entry{};

			entry.type       = atoi(row[0]);
			entry.name       = row[1] ? row[1] : "";
			entry.comment    = row[2] ? row[2] : "";
			entry.fromlevel  = atoi(row[3]);
			entry.tolevel    = atoi(row[4]);
			entry.classes    = atoi(row[5]);
			entry.aacount    = atoi(row[6]);
			entry.timezone   = atoi(row[7]);
			entry.timeposted = atoi(row[8]);

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

#endif //EQEMU_BASE_LFGUILD_REPOSITORY_H
