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

#ifndef EQEMU_BASE_GROUP_LEADERS_REPOSITORY_H
#define EQEMU_BASE_GROUP_LEADERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseGroupLeadersRepository {
public:
	struct GroupLeaders {
		int         gid;
		std::string leadername;
		std::string marknpc;
		std::string leadershipaa;
		std::string maintank;
		std::string assist;
		std::string puller;
		std::string mentoree;
		int         mentor_percent;
	};

	static std::string PrimaryKey()
	{
		return std::string("gid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"gid",
			"leadername",
			"marknpc",
			"leadershipaa",
			"maintank",
			"assist",
			"puller",
			"mentoree",
			"mentor_percent",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("group_leaders");
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

	static GroupLeaders NewEntity()
	{
		GroupLeaders entry{};

		entry.gid            = 0;
		entry.leadername     = "";
		entry.marknpc        = "";
		entry.leadershipaa   = 0;
		entry.maintank       = "";
		entry.assist         = "";
		entry.puller         = "";
		entry.mentoree       = "";
		entry.mentor_percent = 0;

		return entry;
	}

	static GroupLeaders GetGroupLeadersEntry(
		const std::vector<GroupLeaders> &group_leaderss,
		int group_leaders_id
	)
	{
		for (auto &group_leaders : group_leaderss) {
			if (group_leaders.gid == group_leaders_id) {
				return group_leaders;
			}
		}

		return NewEntity();
	}

	static GroupLeaders FindOne(
		Database& db,
		int group_leaders_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				group_leaders_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			GroupLeaders entry{};

			entry.gid            = atoi(row[0]);
			entry.leadername     = row[1] ? row[1] : "";
			entry.marknpc        = row[2] ? row[2] : "";
			entry.leadershipaa   = row[3] ? row[3] : "";
			entry.maintank       = row[4] ? row[4] : "";
			entry.assist         = row[5] ? row[5] : "";
			entry.puller         = row[6] ? row[6] : "";
			entry.mentoree       = row[7] ? row[7] : "";
			entry.mentor_percent = atoi(row[8]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int group_leaders_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				group_leaders_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		GroupLeaders group_leaders_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(group_leaders_entry.gid));
		update_values.push_back(columns[1] + " = '" + EscapeString(group_leaders_entry.leadername) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(group_leaders_entry.marknpc) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(group_leaders_entry.leadershipaa) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(group_leaders_entry.maintank) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(group_leaders_entry.assist) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(group_leaders_entry.puller) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(group_leaders_entry.mentoree) + "'");
		update_values.push_back(columns[8] + " = " + std::to_string(group_leaders_entry.mentor_percent));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				group_leaders_entry.gid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static GroupLeaders InsertOne(
		Database& db,
		GroupLeaders group_leaders_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(group_leaders_entry.gid));
		insert_values.push_back("'" + EscapeString(group_leaders_entry.leadername) + "'");
		insert_values.push_back("'" + EscapeString(group_leaders_entry.marknpc) + "'");
		insert_values.push_back("'" + EscapeString(group_leaders_entry.leadershipaa) + "'");
		insert_values.push_back("'" + EscapeString(group_leaders_entry.maintank) + "'");
		insert_values.push_back("'" + EscapeString(group_leaders_entry.assist) + "'");
		insert_values.push_back("'" + EscapeString(group_leaders_entry.puller) + "'");
		insert_values.push_back("'" + EscapeString(group_leaders_entry.mentoree) + "'");
		insert_values.push_back(std::to_string(group_leaders_entry.mentor_percent));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			group_leaders_entry.gid = results.LastInsertedID();
			return group_leaders_entry;
		}

		group_leaders_entry = NewEntity();

		return group_leaders_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<GroupLeaders> group_leaders_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &group_leaders_entry: group_leaders_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(group_leaders_entry.gid));
			insert_values.push_back("'" + EscapeString(group_leaders_entry.leadername) + "'");
			insert_values.push_back("'" + EscapeString(group_leaders_entry.marknpc) + "'");
			insert_values.push_back("'" + EscapeString(group_leaders_entry.leadershipaa) + "'");
			insert_values.push_back("'" + EscapeString(group_leaders_entry.maintank) + "'");
			insert_values.push_back("'" + EscapeString(group_leaders_entry.assist) + "'");
			insert_values.push_back("'" + EscapeString(group_leaders_entry.puller) + "'");
			insert_values.push_back("'" + EscapeString(group_leaders_entry.mentoree) + "'");
			insert_values.push_back(std::to_string(group_leaders_entry.mentor_percent));

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

	static std::vector<GroupLeaders> All(Database& db)
	{
		std::vector<GroupLeaders> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroupLeaders entry{};

			entry.gid            = atoi(row[0]);
			entry.leadername     = row[1] ? row[1] : "";
			entry.marknpc        = row[2] ? row[2] : "";
			entry.leadershipaa   = row[3] ? row[3] : "";
			entry.maintank       = row[4] ? row[4] : "";
			entry.assist         = row[5] ? row[5] : "";
			entry.puller         = row[6] ? row[6] : "";
			entry.mentoree       = row[7] ? row[7] : "";
			entry.mentor_percent = atoi(row[8]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<GroupLeaders> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<GroupLeaders> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GroupLeaders entry{};

			entry.gid            = atoi(row[0]);
			entry.leadername     = row[1] ? row[1] : "";
			entry.marknpc        = row[2] ? row[2] : "";
			entry.leadershipaa   = row[3] ? row[3] : "";
			entry.maintank       = row[4] ? row[4] : "";
			entry.assist         = row[5] ? row[5] : "";
			entry.puller         = row[6] ? row[6] : "";
			entry.mentoree       = row[7] ? row[7] : "";
			entry.mentor_percent = atoi(row[8]);

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

#endif //EQEMU_BASE_GROUP_LEADERS_REPOSITORY_H
