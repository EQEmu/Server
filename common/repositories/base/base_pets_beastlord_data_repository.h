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

#ifndef EQEMU_BASE_PETS_BEASTLORD_DATA_REPOSITORY_H
#define EQEMU_BASE_PETS_BEASTLORD_DATA_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BasePetsBeastlordDataRepository {
public:
	struct PetsBeastlordData {
		int   player_race;
		int   pet_race;
		int   texture;
		int   helm_texture;
		int   gender;
		float size_modifier;
		int   face;
	};

	static std::string PrimaryKey()
	{
		return std::string("player_race");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"player_race",
			"pet_race",
			"texture",
			"helm_texture",
			"gender",
			"size_modifier",
			"face",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("pets_beastlord_data");
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

	static PetsBeastlordData NewEntity()
	{
		PetsBeastlordData entry{};

		entry.player_race   = 1;
		entry.pet_race      = 42;
		entry.texture       = 0;
		entry.helm_texture  = 0;
		entry.gender        = 2;
		entry.size_modifier = 1;
		entry.face          = 0;

		return entry;
	}

	static PetsBeastlordData GetPetsBeastlordDataEntry(
		const std::vector<PetsBeastlordData> &pets_beastlord_datas,
		int pets_beastlord_data_id
	)
	{
		for (auto &pets_beastlord_data : pets_beastlord_datas) {
			if (pets_beastlord_data.player_race == pets_beastlord_data_id) {
				return pets_beastlord_data;
			}
		}

		return NewEntity();
	}

	static PetsBeastlordData FindOne(
		Database& db,
		int pets_beastlord_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				pets_beastlord_data_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PetsBeastlordData entry{};

			entry.player_race   = atoi(row[0]);
			entry.pet_race      = atoi(row[1]);
			entry.texture       = atoi(row[2]);
			entry.helm_texture  = atoi(row[3]);
			entry.gender        = atoi(row[4]);
			entry.size_modifier = static_cast<float>(atof(row[5]));
			entry.face          = atoi(row[6]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int pets_beastlord_data_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				pets_beastlord_data_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		PetsBeastlordData pets_beastlord_data_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(pets_beastlord_data_entry.player_race));
		update_values.push_back(columns[1] + " = " + std::to_string(pets_beastlord_data_entry.pet_race));
		update_values.push_back(columns[2] + " = " + std::to_string(pets_beastlord_data_entry.texture));
		update_values.push_back(columns[3] + " = " + std::to_string(pets_beastlord_data_entry.helm_texture));
		update_values.push_back(columns[4] + " = " + std::to_string(pets_beastlord_data_entry.gender));
		update_values.push_back(columns[5] + " = " + std::to_string(pets_beastlord_data_entry.size_modifier));
		update_values.push_back(columns[6] + " = " + std::to_string(pets_beastlord_data_entry.face));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				pets_beastlord_data_entry.player_race
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PetsBeastlordData InsertOne(
		Database& db,
		PetsBeastlordData pets_beastlord_data_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(pets_beastlord_data_entry.player_race));
		insert_values.push_back(std::to_string(pets_beastlord_data_entry.pet_race));
		insert_values.push_back(std::to_string(pets_beastlord_data_entry.texture));
		insert_values.push_back(std::to_string(pets_beastlord_data_entry.helm_texture));
		insert_values.push_back(std::to_string(pets_beastlord_data_entry.gender));
		insert_values.push_back(std::to_string(pets_beastlord_data_entry.size_modifier));
		insert_values.push_back(std::to_string(pets_beastlord_data_entry.face));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			pets_beastlord_data_entry.player_race = results.LastInsertedID();
			return pets_beastlord_data_entry;
		}

		pets_beastlord_data_entry = NewEntity();

		return pets_beastlord_data_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<PetsBeastlordData> pets_beastlord_data_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &pets_beastlord_data_entry: pets_beastlord_data_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(pets_beastlord_data_entry.player_race));
			insert_values.push_back(std::to_string(pets_beastlord_data_entry.pet_race));
			insert_values.push_back(std::to_string(pets_beastlord_data_entry.texture));
			insert_values.push_back(std::to_string(pets_beastlord_data_entry.helm_texture));
			insert_values.push_back(std::to_string(pets_beastlord_data_entry.gender));
			insert_values.push_back(std::to_string(pets_beastlord_data_entry.size_modifier));
			insert_values.push_back(std::to_string(pets_beastlord_data_entry.face));

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

	static std::vector<PetsBeastlordData> All(Database& db)
	{
		std::vector<PetsBeastlordData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsBeastlordData entry{};

			entry.player_race   = atoi(row[0]);
			entry.pet_race      = atoi(row[1]);
			entry.texture       = atoi(row[2]);
			entry.helm_texture  = atoi(row[3]);
			entry.gender        = atoi(row[4]);
			entry.size_modifier = static_cast<float>(atof(row[5]));
			entry.face          = atoi(row[6]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<PetsBeastlordData> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<PetsBeastlordData> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsBeastlordData entry{};

			entry.player_race   = atoi(row[0]);
			entry.pet_race      = atoi(row[1]);
			entry.texture       = atoi(row[2]);
			entry.helm_texture  = atoi(row[3]);
			entry.gender        = atoi(row[4]);
			entry.size_modifier = static_cast<float>(atof(row[5]));
			entry.face          = atoi(row[6]);

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

#endif //EQEMU_BASE_PETS_BEASTLORD_DATA_REPOSITORY_H
