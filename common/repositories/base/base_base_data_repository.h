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

#ifndef EQEMU_BASE_BASE_DATA_REPOSITORY_H
#define EQEMU_BASE_BASE_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseBaseDataRepository {
public:
	struct BaseData {
		int   level;
		int   class;
		float hp;
		float mana;
		float end;
		float unk1;
		float unk2;
		float hp_fac;
		float mana_fac;
		float end_fac;
	};

	static std::string PrimaryKey()
	{
		return std::string("level");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"level",
			"class",
			"hp",
			"mana",
			"end",
			"unk1",
			"unk2",
			"hp_fac",
			"mana_fac",
			"end_fac",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("base_data");
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

	static BaseData NewEntity()
	{
		BaseData entry{};

		entry.level    = 0;
		entry.class    = 0;
		entry.hp       = 0;
		entry.mana     = 0;
		entry.end      = 0;
		entry.unk1     = 0;
		entry.unk2     = 0;
		entry.hp_fac   = 0;
		entry.mana_fac = 0;
		entry.end_fac  = 0;

		return entry;
	}

	static BaseData GetBaseDataEntry(
		const std::vector<BaseData> &base_datas,
		int base_data_id
	)
	{
		for (auto &base_data : base_datas) {
			if (base_data.level == base_data_id) {
				return base_data;
			}
		}

		return NewEntity();
	}

	static BaseData FindOne(
		Database& db,
		int base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				base_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			BaseData entry{};

			entry.level    = atoi(row[0]);
			entry.class    = atoi(row[1]);
			entry.hp       = static_cast<float>(atof(row[2]));
			entry.mana     = static_cast<float>(atof(row[3]));
			entry.end      = static_cast<float>(atof(row[4]));
			entry.unk1     = static_cast<float>(atof(row[5]));
			entry.unk2     = static_cast<float>(atof(row[6]));
			entry.hp_fac   = static_cast<float>(atof(row[7]));
			entry.mana_fac = static_cast<float>(atof(row[8]));
			entry.end_fac  = static_cast<float>(atof(row[9]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int base_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				base_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		BaseData base_data_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(base_data_entry.level));
		update_values.push_back(columns[1] + " = " + std::to_string(base_data_entry.class));
		update_values.push_back(columns[2] + " = " + std::to_string(base_data_entry.hp));
		update_values.push_back(columns[3] + " = " + std::to_string(base_data_entry.mana));
		update_values.push_back(columns[4] + " = " + std::to_string(base_data_entry.end));
		update_values.push_back(columns[5] + " = " + std::to_string(base_data_entry.unk1));
		update_values.push_back(columns[6] + " = " + std::to_string(base_data_entry.unk2));
		update_values.push_back(columns[7] + " = " + std::to_string(base_data_entry.hp_fac));
		update_values.push_back(columns[8] + " = " + std::to_string(base_data_entry.mana_fac));
		update_values.push_back(columns[9] + " = " + std::to_string(base_data_entry.end_fac));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				base_data_entry.level
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static BaseData InsertOne(
		Database& db,
		BaseData base_data_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(base_data_entry.level));
		insert_values.push_back(std::to_string(base_data_entry.class));
		insert_values.push_back(std::to_string(base_data_entry.hp));
		insert_values.push_back(std::to_string(base_data_entry.mana));
		insert_values.push_back(std::to_string(base_data_entry.end));
		insert_values.push_back(std::to_string(base_data_entry.unk1));
		insert_values.push_back(std::to_string(base_data_entry.unk2));
		insert_values.push_back(std::to_string(base_data_entry.hp_fac));
		insert_values.push_back(std::to_string(base_data_entry.mana_fac));
		insert_values.push_back(std::to_string(base_data_entry.end_fac));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			base_data_entry.level = results.LastInsertedID();
			return base_data_entry;
		}

		base_data_entry = NewEntity();

		return base_data_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<BaseData> base_data_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &base_data_entry: base_data_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(base_data_entry.level));
			insert_values.push_back(std::to_string(base_data_entry.class));
			insert_values.push_back(std::to_string(base_data_entry.hp));
			insert_values.push_back(std::to_string(base_data_entry.mana));
			insert_values.push_back(std::to_string(base_data_entry.end));
			insert_values.push_back(std::to_string(base_data_entry.unk1));
			insert_values.push_back(std::to_string(base_data_entry.unk2));
			insert_values.push_back(std::to_string(base_data_entry.hp_fac));
			insert_values.push_back(std::to_string(base_data_entry.mana_fac));
			insert_values.push_back(std::to_string(base_data_entry.end_fac));

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

	static std::vector<BaseData> All(Database& db)
	{
		std::vector<BaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BaseData entry{};

			entry.level    = atoi(row[0]);
			entry.class    = atoi(row[1]);
			entry.hp       = static_cast<float>(atof(row[2]));
			entry.mana     = static_cast<float>(atof(row[3]));
			entry.end      = static_cast<float>(atof(row[4]));
			entry.unk1     = static_cast<float>(atof(row[5]));
			entry.unk2     = static_cast<float>(atof(row[6]));
			entry.hp_fac   = static_cast<float>(atof(row[7]));
			entry.mana_fac = static_cast<float>(atof(row[8]));
			entry.end_fac  = static_cast<float>(atof(row[9]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<BaseData> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<BaseData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			BaseData entry{};

			entry.level    = atoi(row[0]);
			entry.class    = atoi(row[1]);
			entry.hp       = static_cast<float>(atof(row[2]));
			entry.mana     = static_cast<float>(atof(row[3]));
			entry.end      = static_cast<float>(atof(row[4]));
			entry.unk1     = static_cast<float>(atof(row[5]));
			entry.unk2     = static_cast<float>(atof(row[6]));
			entry.hp_fac   = static_cast<float>(atof(row[7]));
			entry.mana_fac = static_cast<float>(atof(row[8]));
			entry.end_fac  = static_cast<float>(atof(row[9]));

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

#endif //EQEMU_BASE_BASE_DATA_REPOSITORY_H
