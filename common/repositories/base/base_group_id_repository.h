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

#ifndef EQEMU_BASE_GROUP_ID_REPOSITORY_H
#define EQEMU_BASE_GROUP_ID_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseGroupIdRepository {
public:
	struct GroupId {
		int32_t     groupid;
		int32_t     charid;
		std::string name;
		int8_t      ismerc;
	};

	static std::string PrimaryKey()
	{
		return std::string("groupid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"groupid",
			"charid",
			"name",
			"ismerc",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"groupid",
			"charid",
			"name",
			"ismerc",
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
		return std::string("group_id");
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

	static GroupId NewEntity()
	{
		GroupId e{};

		e.groupid = 0;
		e.charid  = 0;
		e.name    = "";
		e.ismerc  = 0;

		return e;
	}

	static GroupId GetGroupId(
		const std::vector<GroupId> &group_ids,
		int group_id_id
	)
	{
		for (auto &group_id : group_ids) {
			if (group_id.groupid == group_id_id) {
				return group_id;
			}
		}

		return NewEntity();
	}

	static GroupId FindOne(
		Database& db,
		int group_id_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				group_id_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GroupId e{};

			e.groupid = static_cast<int32_t>(atoi(row[0]));
			e.charid  = static_cast<int32_t>(atoi(row[1]));
			e.name    = row[2] ? row[2] : "";
			e.ismerc  = static_cast<int8_t>(atoi(row[3]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int group_id_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				group_id_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const GroupId &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.groupid));
		v.push_back(columns[1] + " = " + std::to_string(e.charid));
		v.push_back(columns[2] + " = '" + Strings::Escape(e.name) + "'");
		v.push_back(columns[3] + " = " + std::to_string(e.ismerc));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.groupid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GroupId InsertOne(
		Database& db,
		GroupId e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.groupid));
		v.push_back(std::to_string(e.charid));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.ismerc));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.groupid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<GroupId> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.groupid));
			v.push_back(std::to_string(e.charid));
			v.push_back("'" + Strings::Escape(e.name) + "'");
			v.push_back(std::to_string(e.ismerc));

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

	static std::vector<GroupId> All(Database& db)
	{
		std::vector<GroupId> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroupId e{};

			e.groupid = static_cast<int32_t>(atoi(row[0]));
			e.charid  = static_cast<int32_t>(atoi(row[1]));
			e.name    = row[2] ? row[2] : "";
			e.ismerc  = static_cast<int8_t>(atoi(row[3]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<GroupId> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<GroupId> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroupId e{};

			e.groupid = static_cast<int32_t>(atoi(row[0]));
			e.charid  = static_cast<int32_t>(atoi(row[1]));
			e.name    = row[2] ? row[2] : "";
			e.ismerc  = static_cast<int8_t>(atoi(row[3]));

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

#endif //EQEMU_BASE_GROUP_ID_REPOSITORY_H
