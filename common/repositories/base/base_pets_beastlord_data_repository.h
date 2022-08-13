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
#include "../../strings.h"
#include <ctime>

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

	static std::vector<std::string> SelectColumns()
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("pets_beastlord_data");
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

	static PetsBeastlordData NewEntity()
	{
		PetsBeastlordData e{};

		e.player_race   = 1;
		e.pet_race      = 42;
		e.texture       = 0;
		e.helm_texture  = 0;
		e.gender        = 2;
		e.size_modifier = 1;
		e.face          = 0;

		return e;
	}

	static PetsBeastlordData GetPetsBeastlordDatae(
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
			PetsBeastlordData e{};

			e.player_race   = atoi(row[0]);
			e.pet_race      = atoi(row[1]);
			e.texture       = atoi(row[2]);
			e.helm_texture  = atoi(row[3]);
			e.gender        = atoi(row[4]);
			e.size_modifier = static_cast<float>(atof(row[5]));
			e.face          = atoi(row[6]);

			return e;
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
		PetsBeastlordData pets_beastlord_data_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(pets_beastlord_data_e.player_race));
		update_values.push_back(columns[1] + " = " + std::to_string(pets_beastlord_data_e.pet_race));
		update_values.push_back(columns[2] + " = " + std::to_string(pets_beastlord_data_e.texture));
		update_values.push_back(columns[3] + " = " + std::to_string(pets_beastlord_data_e.helm_texture));
		update_values.push_back(columns[4] + " = " + std::to_string(pets_beastlord_data_e.gender));
		update_values.push_back(columns[5] + " = " + std::to_string(pets_beastlord_data_e.size_modifier));
		update_values.push_back(columns[6] + " = " + std::to_string(pets_beastlord_data_e.face));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				pets_beastlord_data_e.player_race
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PetsBeastlordData InsertOne(
		Database& db,
		PetsBeastlordData pets_beastlord_data_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(pets_beastlord_data_e.player_race));
		insert_values.push_back(std::to_string(pets_beastlord_data_e.pet_race));
		insert_values.push_back(std::to_string(pets_beastlord_data_e.texture));
		insert_values.push_back(std::to_string(pets_beastlord_data_e.helm_texture));
		insert_values.push_back(std::to_string(pets_beastlord_data_e.gender));
		insert_values.push_back(std::to_string(pets_beastlord_data_e.size_modifier));
		insert_values.push_back(std::to_string(pets_beastlord_data_e.face));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			pets_beastlord_data_e.player_race = results.LastInsertedID();
			return pets_beastlord_data_e;
		}

		pets_beastlord_data_e = NewEntity();

		return pets_beastlord_data_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<PetsBeastlordData> pets_beastlord_data_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &pets_beastlord_data_e: pets_beastlord_data_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(pets_beastlord_data_e.player_race));
			insert_values.push_back(std::to_string(pets_beastlord_data_e.pet_race));
			insert_values.push_back(std::to_string(pets_beastlord_data_e.texture));
			insert_values.push_back(std::to_string(pets_beastlord_data_e.helm_texture));
			insert_values.push_back(std::to_string(pets_beastlord_data_e.gender));
			insert_values.push_back(std::to_string(pets_beastlord_data_e.size_modifier));
			insert_values.push_back(std::to_string(pets_beastlord_data_e.face));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
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
			PetsBeastlordData e{};

			e.player_race   = atoi(row[0]);
			e.pet_race      = atoi(row[1]);
			e.texture       = atoi(row[2]);
			e.helm_texture  = atoi(row[3]);
			e.gender        = atoi(row[4]);
			e.size_modifier = static_cast<float>(atof(row[5]));
			e.face          = atoi(row[6]);

			all_entries.push_back(e);
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
			PetsBeastlordData e{};

			e.player_race   = atoi(row[0]);
			e.pet_race      = atoi(row[1]);
			e.texture       = atoi(row[2]);
			e.helm_texture  = atoi(row[3]);
			e.gender        = atoi(row[4]);
			e.size_modifier = static_cast<float>(atof(row[5]));
			e.face          = atoi(row[6]);

			all_entries.push_back(e);
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

	static int64 GetMaxId(Database& db)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT MAX({}) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string& where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

};

#endif //EQEMU_BASE_PETS_BEASTLORD_DATA_REPOSITORY_H
