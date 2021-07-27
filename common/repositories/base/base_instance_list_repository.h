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

#ifndef EQEMU_BASE_INSTANCE_LIST_REPOSITORY_H
#define EQEMU_BASE_INSTANCE_LIST_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseInstanceListRepository {
public:
	struct InstanceList {
		int id;
		int zone;
		int version;
		int is_global;
		int start_time;
		int duration;
		int never_expires;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zone",
			"version",
			"is_global",
			"start_time",
			"duration",
			"never_expires",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("instance_list");
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

	static InstanceList NewEntity()
	{
		InstanceList entry{};

		entry.id            = 0;
		entry.zone          = 0;
		entry.version       = 0;
		entry.is_global     = 0;
		entry.start_time    = 0;
		entry.duration      = 0;
		entry.never_expires = 0;

		return entry;
	}

	static InstanceList GetInstanceListEntry(
		const std::vector<InstanceList> &instance_lists,
		int instance_list_id
	)
	{
		for (auto &instance_list : instance_lists) {
			if (instance_list.id == instance_list_id) {
				return instance_list;
			}
		}

		return NewEntity();
	}

	static InstanceList FindOne(
		Database& db,
		int instance_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				instance_list_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			InstanceList entry{};

			entry.id            = atoi(row[0]);
			entry.zone          = atoi(row[1]);
			entry.version       = atoi(row[2]);
			entry.is_global     = atoi(row[3]);
			entry.start_time    = atoi(row[4]);
			entry.duration      = atoi(row[5]);
			entry.never_expires = atoi(row[6]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int instance_list_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				instance_list_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		InstanceList instance_list_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(instance_list_entry.zone));
		update_values.push_back(columns[2] + " = " + std::to_string(instance_list_entry.version));
		update_values.push_back(columns[3] + " = " + std::to_string(instance_list_entry.is_global));
		update_values.push_back(columns[4] + " = " + std::to_string(instance_list_entry.start_time));
		update_values.push_back(columns[5] + " = " + std::to_string(instance_list_entry.duration));
		update_values.push_back(columns[6] + " = " + std::to_string(instance_list_entry.never_expires));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				instance_list_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static InstanceList InsertOne(
		Database& db,
		InstanceList instance_list_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(instance_list_entry.id));
		insert_values.push_back(std::to_string(instance_list_entry.zone));
		insert_values.push_back(std::to_string(instance_list_entry.version));
		insert_values.push_back(std::to_string(instance_list_entry.is_global));
		insert_values.push_back(std::to_string(instance_list_entry.start_time));
		insert_values.push_back(std::to_string(instance_list_entry.duration));
		insert_values.push_back(std::to_string(instance_list_entry.never_expires));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			instance_list_entry.id = results.LastInsertedID();
			return instance_list_entry;
		}

		instance_list_entry = NewEntity();

		return instance_list_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<InstanceList> instance_list_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &instance_list_entry: instance_list_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(instance_list_entry.id));
			insert_values.push_back(std::to_string(instance_list_entry.zone));
			insert_values.push_back(std::to_string(instance_list_entry.version));
			insert_values.push_back(std::to_string(instance_list_entry.is_global));
			insert_values.push_back(std::to_string(instance_list_entry.start_time));
			insert_values.push_back(std::to_string(instance_list_entry.duration));
			insert_values.push_back(std::to_string(instance_list_entry.never_expires));

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

	static std::vector<InstanceList> All(Database& db)
	{
		std::vector<InstanceList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceList entry{};

			entry.id            = atoi(row[0]);
			entry.zone          = atoi(row[1]);
			entry.version       = atoi(row[2]);
			entry.is_global     = atoi(row[3]);
			entry.start_time    = atoi(row[4]);
			entry.duration      = atoi(row[5]);
			entry.never_expires = atoi(row[6]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<InstanceList> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<InstanceList> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceList entry{};

			entry.id            = atoi(row[0]);
			entry.zone          = atoi(row[1]);
			entry.version       = atoi(row[2]);
			entry.is_global     = atoi(row[3]);
			entry.start_time    = atoi(row[4]);
			entry.duration      = atoi(row[5]);
			entry.never_expires = atoi(row[6]);

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

#endif //EQEMU_BASE_INSTANCE_LIST_REPOSITORY_H
