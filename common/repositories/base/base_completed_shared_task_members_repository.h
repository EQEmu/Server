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

#ifndef EQEMU_BASE_COMPLETED_SHARED_TASK_MEMBERS_REPOSITORY_H
#define EQEMU_BASE_COMPLETED_SHARED_TASK_MEMBERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseCompletedSharedTaskMembersRepository {
public:
	struct CompletedSharedTaskMembers {
		int64 shared_task_id;
		int64 character_id;
		int   is_leader;
	};

	static std::string PrimaryKey()
	{
		return std::string("shared_task_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"shared_task_id",
			"character_id",
			"is_leader",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"shared_task_id",
			"character_id",
			"is_leader",
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
		return std::string("completed_shared_task_members");
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

	static CompletedSharedTaskMembers NewEntity()
	{
		CompletedSharedTaskMembers entry{};

		entry.shared_task_id = 0;
		entry.character_id   = 0;
		entry.is_leader      = 0;

		return entry;
	}

	static CompletedSharedTaskMembers GetCompletedSharedTaskMembersEntry(
		const std::vector<CompletedSharedTaskMembers> &completed_shared_task_memberss,
		int completed_shared_task_members_id
	)
	{
		for (auto &completed_shared_task_members : completed_shared_task_memberss) {
			if (completed_shared_task_members.shared_task_id == completed_shared_task_members_id) {
				return completed_shared_task_members;
			}
		}

		return NewEntity();
	}

	static CompletedSharedTaskMembers FindOne(
		Database& db,
		int completed_shared_task_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				completed_shared_task_members_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CompletedSharedTaskMembers entry{};

			entry.shared_task_id = strtoll(row[0], nullptr, 10);
			entry.character_id   = strtoll(row[1], nullptr, 10);
			entry.is_leader      = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int completed_shared_task_members_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				completed_shared_task_members_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CompletedSharedTaskMembers completed_shared_task_members_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(completed_shared_task_members_entry.shared_task_id));
		update_values.push_back(columns[1] + " = " + std::to_string(completed_shared_task_members_entry.character_id));
		update_values.push_back(columns[2] + " = " + std::to_string(completed_shared_task_members_entry.is_leader));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				completed_shared_task_members_entry.shared_task_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CompletedSharedTaskMembers InsertOne(
		Database& db,
		CompletedSharedTaskMembers completed_shared_task_members_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(completed_shared_task_members_entry.shared_task_id));
		insert_values.push_back(std::to_string(completed_shared_task_members_entry.character_id));
		insert_values.push_back(std::to_string(completed_shared_task_members_entry.is_leader));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			completed_shared_task_members_entry.shared_task_id = results.LastInsertedID();
			return completed_shared_task_members_entry;
		}

		completed_shared_task_members_entry = NewEntity();

		return completed_shared_task_members_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CompletedSharedTaskMembers> completed_shared_task_members_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &completed_shared_task_members_entry: completed_shared_task_members_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(completed_shared_task_members_entry.shared_task_id));
			insert_values.push_back(std::to_string(completed_shared_task_members_entry.character_id));
			insert_values.push_back(std::to_string(completed_shared_task_members_entry.is_leader));

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

	static std::vector<CompletedSharedTaskMembers> All(Database& db)
	{
		std::vector<CompletedSharedTaskMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTaskMembers entry{};

			entry.shared_task_id = strtoll(row[0], nullptr, 10);
			entry.character_id   = strtoll(row[1], nullptr, 10);
			entry.is_leader      = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CompletedSharedTaskMembers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CompletedSharedTaskMembers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CompletedSharedTaskMembers entry{};

			entry.shared_task_id = strtoll(row[0], nullptr, 10);
			entry.character_id   = strtoll(row[1], nullptr, 10);
			entry.is_leader      = atoi(row[2]);

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

#endif //EQEMU_BASE_COMPLETED_SHARED_TASK_MEMBERS_REPOSITORY_H
