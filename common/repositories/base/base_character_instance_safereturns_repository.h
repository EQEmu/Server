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

#ifndef EQEMU_BASE_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseCharacterInstanceSafereturnsRepository {
public:
	struct CharacterInstanceSafereturns {
		int   id;
		int   character_id;
		int   instance_zone_id;
		int   instance_id;
		int   safe_zone_id;
		float safe_x;
		float safe_y;
		float safe_z;
		float safe_heading;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"instance_zone_id",
			"instance_id",
			"safe_zone_id",
			"safe_x",
			"safe_y",
			"safe_z",
			"safe_heading",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("character_instance_safereturns");
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

	static CharacterInstanceSafereturns NewEntity()
	{
		CharacterInstanceSafereturns entry{};

		entry.id               = 0;
		entry.character_id     = 0;
		entry.instance_zone_id = 0;
		entry.instance_id      = 0;
		entry.safe_zone_id     = 0;
		entry.safe_x           = 0;
		entry.safe_y           = 0;
		entry.safe_z           = 0;
		entry.safe_heading     = 0;

		return entry;
	}

	static CharacterInstanceSafereturns GetCharacterInstanceSafereturnsEntry(
		const std::vector<CharacterInstanceSafereturns> &character_instance_safereturnss,
		int character_instance_safereturns_id
	)
	{
		for (auto &character_instance_safereturns : character_instance_safereturnss) {
			if (character_instance_safereturns.id == character_instance_safereturns_id) {
				return character_instance_safereturns;
			}
		}

		return NewEntity();
	}

	static CharacterInstanceSafereturns FindOne(
		Database& db,
		int character_instance_safereturns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				character_instance_safereturns_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterInstanceSafereturns entry{};

			entry.id               = atoi(row[0]);
			entry.character_id     = atoi(row[1]);
			entry.instance_zone_id = atoi(row[2]);
			entry.instance_id      = atoi(row[3]);
			entry.safe_zone_id     = atoi(row[4]);
			entry.safe_x           = static_cast<float>(atof(row[5]));
			entry.safe_y           = static_cast<float>(atof(row[6]));
			entry.safe_z           = static_cast<float>(atof(row[7]));
			entry.safe_heading     = static_cast<float>(atof(row[8]));

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_instance_safereturns_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_instance_safereturns_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		CharacterInstanceSafereturns character_instance_safereturns_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = " + std::to_string(character_instance_safereturns_entry.character_id));
		update_values.push_back(columns[2] + " = " + std::to_string(character_instance_safereturns_entry.instance_zone_id));
		update_values.push_back(columns[3] + " = " + std::to_string(character_instance_safereturns_entry.instance_id));
		update_values.push_back(columns[4] + " = " + std::to_string(character_instance_safereturns_entry.safe_zone_id));
		update_values.push_back(columns[5] + " = " + std::to_string(character_instance_safereturns_entry.safe_x));
		update_values.push_back(columns[6] + " = " + std::to_string(character_instance_safereturns_entry.safe_y));
		update_values.push_back(columns[7] + " = " + std::to_string(character_instance_safereturns_entry.safe_z));
		update_values.push_back(columns[8] + " = " + std::to_string(character_instance_safereturns_entry.safe_heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				character_instance_safereturns_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterInstanceSafereturns InsertOne(
		Database& db,
		CharacterInstanceSafereturns character_instance_safereturns_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_instance_safereturns_entry.id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.character_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.instance_zone_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.instance_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_zone_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_x));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_y));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_z));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_heading));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			character_instance_safereturns_entry.id = results.LastInsertedID();
			return character_instance_safereturns_entry;
		}

		character_instance_safereturns_entry = NewEntity();

		return character_instance_safereturns_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<CharacterInstanceSafereturns> character_instance_safereturns_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &character_instance_safereturns_entry: character_instance_safereturns_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(character_instance_safereturns_entry.id));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.character_id));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.instance_zone_id));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.instance_id));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_zone_id));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_x));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_y));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_z));
			insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_heading));

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

	static std::vector<CharacterInstanceSafereturns> All(Database& db)
	{
		std::vector<CharacterInstanceSafereturns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterInstanceSafereturns entry{};

			entry.id               = atoi(row[0]);
			entry.character_id     = atoi(row[1]);
			entry.instance_zone_id = atoi(row[2]);
			entry.instance_id      = atoi(row[3]);
			entry.safe_zone_id     = atoi(row[4]);
			entry.safe_x           = static_cast<float>(atof(row[5]));
			entry.safe_y           = static_cast<float>(atof(row[6]));
			entry.safe_z           = static_cast<float>(atof(row[7]));
			entry.safe_heading     = static_cast<float>(atof(row[8]));

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<CharacterInstanceSafereturns> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<CharacterInstanceSafereturns> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterInstanceSafereturns entry{};

			entry.id               = atoi(row[0]);
			entry.character_id     = atoi(row[1]);
			entry.instance_zone_id = atoi(row[2]);
			entry.instance_id      = atoi(row[3]);
			entry.safe_zone_id     = atoi(row[4]);
			entry.safe_x           = static_cast<float>(atof(row[5]));
			entry.safe_y           = static_cast<float>(atof(row[6]));
			entry.safe_z           = static_cast<float>(atof(row[7]));
			entry.safe_heading     = static_cast<float>(atof(row[8]));

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

#endif //EQEMU_BASE_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H
