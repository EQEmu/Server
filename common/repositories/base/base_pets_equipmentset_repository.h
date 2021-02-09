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

#ifndef EQEMU_BASE_PETS_EQUIPMENTSET_REPOSITORY_H
#define EQEMU_BASE_PETS_EQUIPMENTSET_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BasePetsEquipmentsetRepository {
public:
	struct PetsEquipmentset {
		int         set_id;
		std::string setname;
		int         nested_set;
	};

	static std::string PrimaryKey()
	{
		return std::string("set_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"set_id",
			"setname",
			"nested_set",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("pets_equipmentset");
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

	static PetsEquipmentset NewEntity()
	{
		PetsEquipmentset entry{};

		entry.set_id     = 0;
		entry.setname    = "";
		entry.nested_set = -1;

		return entry;
	}

	static PetsEquipmentset GetPetsEquipmentsetEntry(
		const std::vector<PetsEquipmentset> &pets_equipmentsets,
		int pets_equipmentset_id
	)
	{
		for (auto &pets_equipmentset : pets_equipmentsets) {
			if (pets_equipmentset.set_id == pets_equipmentset_id) {
				return pets_equipmentset;
			}
		}

		return NewEntity();
	}

	static PetsEquipmentset FindOne(
		Database& db,
		int pets_equipmentset_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				pets_equipmentset_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			PetsEquipmentset entry{};

			entry.set_id     = atoi(row[0]);
			entry.setname    = row[1] ? row[1] : "";
			entry.nested_set = atoi(row[2]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int pets_equipmentset_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				pets_equipmentset_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		PetsEquipmentset pets_equipmentset_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(pets_equipmentset_entry.set_id));
		update_values.push_back(columns[1] + " = '" + EscapeString(pets_equipmentset_entry.setname) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(pets_equipmentset_entry.nested_set));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				pets_equipmentset_entry.set_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static PetsEquipmentset InsertOne(
		Database& db,
		PetsEquipmentset pets_equipmentset_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(pets_equipmentset_entry.set_id));
		insert_values.push_back("'" + EscapeString(pets_equipmentset_entry.setname) + "'");
		insert_values.push_back(std::to_string(pets_equipmentset_entry.nested_set));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			pets_equipmentset_entry.set_id = results.LastInsertedID();
			return pets_equipmentset_entry;
		}

		pets_equipmentset_entry = NewEntity();

		return pets_equipmentset_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<PetsEquipmentset> pets_equipmentset_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &pets_equipmentset_entry: pets_equipmentset_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(pets_equipmentset_entry.set_id));
			insert_values.push_back("'" + EscapeString(pets_equipmentset_entry.setname) + "'");
			insert_values.push_back(std::to_string(pets_equipmentset_entry.nested_set));

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

	static std::vector<PetsEquipmentset> All(Database& db)
	{
		std::vector<PetsEquipmentset> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsEquipmentset entry{};

			entry.set_id     = atoi(row[0]);
			entry.setname    = row[1] ? row[1] : "";
			entry.nested_set = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<PetsEquipmentset> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<PetsEquipmentset> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			PetsEquipmentset entry{};

			entry.set_id     = atoi(row[0]);
			entry.setname    = row[1] ? row[1] : "";
			entry.nested_set = atoi(row[2]);

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

#endif //EQEMU_BASE_PETS_EQUIPMENTSET_REPOSITORY_H
