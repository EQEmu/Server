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

#ifndef EQEMU_BASE_INSTANCE_LIST_PLAYER_REPOSITORY_H
#define EQEMU_BASE_INSTANCE_LIST_PLAYER_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseInstanceListPlayerRepository {
public:
	struct InstanceListPlayer {
		int id;
		int charid;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"charid",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("instance_list_player");
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

	static InstanceListPlayer NewEntity()
	{
		InstanceListPlayer entry{};

		entry.id     = 0;
		entry.charid = 0;

		return entry;
	}

	static InstanceListPlayer GetInstanceListPlayerEntry(
		const std::vector<InstanceListPlayer> &instance_list_players,
		int instance_list_player_id
	)
	{
		for (auto &instance_list_player : instance_list_players) {
			if (instance_list_player.id == instance_list_player_id) {
				return instance_list_player;
			}
		}

		return NewEntity();
	}

	static InstanceListPlayer FindOne(
		Database& db,
		int instance_list_player_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				instance_list_player_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			InstanceListPlayer entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int instance_list_player_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				instance_list_player_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		InstanceListPlayer instance_list_player_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(instance_list_player_entry.id));
		update_values.push_back(columns[1] + " = " + std::to_string(instance_list_player_entry.charid));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				instance_list_player_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static InstanceListPlayer InsertOne(
		Database& db,
		InstanceListPlayer instance_list_player_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(instance_list_player_entry.id));
		insert_values.push_back(std::to_string(instance_list_player_entry.charid));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			instance_list_player_entry.id = results.LastInsertedID();
			return instance_list_player_entry;
		}

		instance_list_player_entry = NewEntity();

		return instance_list_player_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<InstanceListPlayer> instance_list_player_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &instance_list_player_entry: instance_list_player_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(instance_list_player_entry.id));
			insert_values.push_back(std::to_string(instance_list_player_entry.charid));

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

	static std::vector<InstanceListPlayer> All(Database& db)
	{
		std::vector<InstanceListPlayer> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceListPlayer entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<InstanceListPlayer> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<InstanceListPlayer> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstanceListPlayer entry{};

			entry.id     = atoi(row[0]);
			entry.charid = atoi(row[1]);

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

#endif //EQEMU_BASE_INSTANCE_LIST_PLAYER_REPOSITORY_H
