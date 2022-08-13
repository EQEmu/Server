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

#ifndef EQEMU_BASE_ADVENTURE_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_ADVENTURE_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAdventureMembersRepository {
public:
	struct AdventureMembers {
		int id;
		int charid;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"charid",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"charid",
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
		return std::string("adventure_members");
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

	static AdventureMembers NewEntity()
	{
		AdventureMembers e{};

		e.id     = 0;
		e.charid = 0;

		return e;
	}

	static AdventureMembers GetAdventureMemberse(
		const std::vector<AdventureMembers> &adventure_memberss,
		int adventure_members_id
	)
	{
		for (auto &adventure_members : adventure_memberss) {
			if (adventure_members.charid == adventure_members_id) {
				return adventure_members;
			}
		}

		return NewEntity();
	}

	static AdventureMembers FindOne(
		Database& db,
		int adventure_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				adventure_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AdventureMembers e{};

			e.id     = atoi(row[0]);
			e.charid = atoi(row[1]);

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int adventure_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				adventure_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AdventureMembers adventure_members_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(adventure_members_e.id));
		update_values.push_back(columns[1] + " = " + std::to_string(adventure_members_e.charid));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				adventure_members_e.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AdventureMembers InsertOne(
		Database& db,
		AdventureMembers adventure_members_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(adventure_members_e.id));
		insert_values.push_back(std::to_string(adventure_members_e.charid));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			adventure_members_e.charid = results.LastInsertedID();
			return adventure_members_e;
		}

		adventure_members_e = NewEntity();

		return adventure_members_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<AdventureMembers> adventure_members_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &adventure_members_e: adventure_members_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(adventure_members_e.id));
			insert_values.push_back(std::to_string(adventure_members_e.charid));

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

	static std::vector<AdventureMembers> All(Database& db)
	{
		std::vector<AdventureMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureMembers e{};

			e.id     = atoi(row[0]);
			e.charid = atoi(row[1]);

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AdventureMembers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AdventureMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AdventureMembers e{};

			e.id     = atoi(row[0]);
			e.charid = atoi(row[1]);

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

#endif //EQEMU_BASE_ADVENTURE_MEMBERS_REPOSITORY_H
