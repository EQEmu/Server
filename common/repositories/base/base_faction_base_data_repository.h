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

#ifndef EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H
#define EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseFactionBaseDataRepository {
public:
	struct FactionBaseData {
		int client_faction_id;
		int min;
		int max;
		int unk_hero1;
		int unk_hero2;
		int unk_hero3;
	};

	static std::string PrimaryKey()
	{
		return std::string("client_faction_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"client_faction_id",
			"min",
			"max",
			"unk_hero1",
			"unk_hero2",
			"unk_hero3",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("faction_base_data");
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

	static FactionBaseData NewEntity()
	{
		FactionBaseData entry{};

		entry.client_faction_id = 0;
		entry.min               = -2000;
		entry.max               = 2000;
		entry.unk_hero1         = 0;
		entry.unk_hero2         = 0;
		entry.unk_hero3         = 0;

		return entry;
	}

	static FactionBaseData GetFactionBaseDataEntry(
		const std::vector<FactionBaseData> &faction_base_datas,
		int faction_base_data_id
	)
	{
		for (auto &faction_base_data : faction_base_datas) {
			if (faction_base_data.client_faction_id == faction_base_data_id) {
				return faction_base_data;
			}
		}

		return NewEntity();
	}

	static FactionBaseData FindOne(
		Database& db,
		int faction_base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				faction_base_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			FactionBaseData entry{};

			entry.client_faction_id = atoi(row[0]);
			entry.min               = atoi(row[1]);
			entry.max               = atoi(row[2]);
			entry.unk_hero1         = atoi(row[3]);
			entry.unk_hero2         = atoi(row[4]);
			entry.unk_hero3         = atoi(row[5]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int faction_base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				faction_base_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		FactionBaseData faction_base_data_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(faction_base_data_entry.client_faction_id));
		update_values.push_back(columns[1] + " = " + std::to_string(faction_base_data_entry.min));
		update_values.push_back(columns[2] + " = " + std::to_string(faction_base_data_entry.max));
		update_values.push_back(columns[3] + " = " + std::to_string(faction_base_data_entry.unk_hero1));
		update_values.push_back(columns[4] + " = " + std::to_string(faction_base_data_entry.unk_hero2));
		update_values.push_back(columns[5] + " = " + std::to_string(faction_base_data_entry.unk_hero3));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				faction_base_data_entry.client_faction_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static FactionBaseData InsertOne(
		Database& db,
		FactionBaseData faction_base_data_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(faction_base_data_entry.client_faction_id));
		insert_values.push_back(std::to_string(faction_base_data_entry.min));
		insert_values.push_back(std::to_string(faction_base_data_entry.max));
		insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero1));
		insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero2));
		insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero3));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			faction_base_data_entry.client_faction_id = results.LastInsertedID();
			return faction_base_data_entry;
		}

		faction_base_data_entry = NewEntity();

		return faction_base_data_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<FactionBaseData> faction_base_data_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &faction_base_data_entry: faction_base_data_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(faction_base_data_entry.client_faction_id));
			insert_values.push_back(std::to_string(faction_base_data_entry.min));
			insert_values.push_back(std::to_string(faction_base_data_entry.max));
			insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero1));
			insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero2));
			insert_values.push_back(std::to_string(faction_base_data_entry.unk_hero3));

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

	static std::vector<FactionBaseData> All(Database& db)
	{
		std::vector<FactionBaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionBaseData entry{};

			entry.client_faction_id = atoi(row[0]);
			entry.min               = atoi(row[1]);
			entry.max               = atoi(row[2]);
			entry.unk_hero1         = atoi(row[3]);
			entry.unk_hero2         = atoi(row[4]);
			entry.unk_hero3         = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<FactionBaseData> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<FactionBaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			FactionBaseData entry{};

			entry.client_faction_id = atoi(row[0]);
			entry.min               = atoi(row[1]);
			entry.max               = atoi(row[2]);
			entry.unk_hero1         = atoi(row[3]);
			entry.unk_hero2         = atoi(row[4]);
			entry.unk_hero3         = atoi(row[5]);

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

#endif //EQEMU_BASE_FACTION_BASE_DATA_REPOSITORY_H
