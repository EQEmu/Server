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

#ifndef EQEMU_BASE_PLAYER_TITLESETS_REPOSITORY_H
#define EQEMU_BASE_PLAYER_TITLESETS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BasePlayerTitlesetsRepository {
public:
	struct PlayerTitlesets {
		int id;
		int char_id;
		int title_set;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"char_id",
			"title_set",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("player_titlesets");
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

	static PlayerTitlesets NewEntity()
	{
		PlayerTitlesets entry{};

		entry.id        = 0;
		entry.char_id   = 0;
		entry.title_set = 0;

		return entry;
	}

	static PlayerTitlesets GetPlayerTitlesetsEntry(
		const std::vector<PlayerTitlesets> &player_titlesetss,
		int player_titlesets_id
	)
	{
		for (auto &player_titlesets : player_titlesetss) {
			if (player_titlesets.id == player_titlesets_id) {
				return player_titlesets;
			}
		}

		return NewEntity();
	}

	static PlayerTitlesets FindOne(
		Database& db,
		int player_titlesets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				player_titlesets_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PlayerTitlesets entry{};

			entry.id        = atoi(row[0]);
			entry.char_id   = atoi(row[1]);
			entry.title_set = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int player_titlesets_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				player_titlesets_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		PlayerTitlesets player_titlesets_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(player_titlesets_entry.char_id));
		update_values.push_back(columns[2] + " = " + std::to_string(player_titlesets_entry.title_set));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				player_titlesets_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PlayerTitlesets InsertOne(
		Database& db,
		PlayerTitlesets player_titlesets_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(player_titlesets_entry.id));
		insert_values.push_back(std::to_string(player_titlesets_entry.char_id));
		insert_values.push_back(std::to_string(player_titlesets_entry.title_set));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			player_titlesets_entry.id = results.LastInsertedID();
			return player_titlesets_entry;
		}

		player_titlesets_entry = NewEntity();

		return player_titlesets_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<PlayerTitlesets> player_titlesets_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &player_titlesets_entry: player_titlesets_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(player_titlesets_entry.id));
			insert_values.push_back(std::to_string(player_titlesets_entry.char_id));
			insert_values.push_back(std::to_string(player_titlesets_entry.title_set));

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

	static std::vector<PlayerTitlesets> All(Database& db)
	{
		std::vector<PlayerTitlesets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerTitlesets entry{};

			entry.id        = atoi(row[0]);
			entry.char_id   = atoi(row[1]);
			entry.title_set = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<PlayerTitlesets> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<PlayerTitlesets> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PlayerTitlesets entry{};

			entry.id        = atoi(row[0]);
			entry.char_id   = atoi(row[1]);
			entry.title_set = atoi(row[2]);

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

#endif //EQEMU_BASE_PLAYER_TITLESETS_REPOSITORY_H
