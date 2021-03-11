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
#include "../../string_util.h"

class BaseGroupIdRepository {
public:
	struct GroupId {
		int         groupid;
		int         charid;
		std::string name;
		int         ismerc;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("group_id");
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

	static GroupId NewEntity()
	{
		GroupId entry{};

		entry.groupid = 0;
		entry.charid  = 0;
		entry.name    = "";
		entry.ismerc  = 0;

		return entry;
	}

	static GroupId GetGroupIdEntry(
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
			GroupId entry{};

			entry.groupid = atoi(row[0]);
			entry.charid  = atoi(row[1]);
			entry.name    = row[2] ? row[2] : "";
			entry.ismerc  = atoi(row[3]);

			return entry;
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
		GroupId group_id_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(group_id_entry.groupid));
		update_values.push_back(columns[1] + " = " + std::to_string(group_id_entry.charid));
		update_values.push_back(columns[2] + " = '" + EscapeString(group_id_entry.name) + "'");
		update_values.push_back(columns[3] + " = " + std::to_string(group_id_entry.ismerc));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				group_id_entry.groupid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GroupId InsertOne(
		Database& db,
		GroupId group_id_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(group_id_entry.groupid));
		insert_values.push_back(std::to_string(group_id_entry.charid));
		insert_values.push_back("'" + EscapeString(group_id_entry.name) + "'");
		insert_values.push_back(std::to_string(group_id_entry.ismerc));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			group_id_entry.groupid = results.LastInsertedID();
			return group_id_entry;
		}

		group_id_entry = NewEntity();

		return group_id_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GroupId> group_id_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &group_id_entry: group_id_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(group_id_entry.groupid));
			insert_values.push_back(std::to_string(group_id_entry.charid));
			insert_values.push_back("'" + EscapeString(group_id_entry.name) + "'");
			insert_values.push_back(std::to_string(group_id_entry.ismerc));

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
			GroupId entry{};

			entry.groupid = atoi(row[0]);
			entry.charid  = atoi(row[1]);
			entry.name    = row[2] ? row[2] : "";
			entry.ismerc  = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GroupId> GetWhere(Database& db, std::string where_filter)
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
			GroupId entry{};

			entry.groupid = atoi(row[0]);
			entry.charid  = atoi(row[1]);
			entry.name    = row[2] ? row[2] : "";
			entry.ismerc  = atoi(row[3]);

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

#endif //EQEMU_BASE_GROUP_ID_REPOSITORY_H
