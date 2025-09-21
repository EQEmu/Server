/**
 * DO NOT MODIFY THIS FILE
 *
 * This repository was automatically generated and is NOT to be modified directly.
 * Any repository modifications are meant to be made to the repository extending the base.
 * Any modifications to base repositories are to be made by the generator only
 *
 * @generator ./utils/scripts/generators/repository-generator.pl
 * @docs https://docs.eqemu.io/developer/repositories
 */

#ifndef EQEMU_BASE_INVENTORY_REPOSITORY_H
#define EQEMU_BASE_INVENTORY_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseInventoryRepository {
public:
	struct Inventory {
		uint32_t    character_id;
		uint32_t    slot_id;
		uint32_t    item_id;
		uint16_t    charges;
		uint32_t    color;
		uint32_t    augment_one;
		uint32_t    augment_two;
		uint32_t    augment_three;
		uint32_t    augment_four;
		uint32_t    augment_five;
		uint32_t    augment_six;
		uint8_t     instnodrop;
		std::string custom_data;
		uint32_t    ornament_icon;
		uint32_t    ornament_idfile;
		int32_t     ornament_hero_model;
		std::string item_unique_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("character_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"character_id",
			"slot_id",
			"item_id",
			"charges",
			"color",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"instnodrop",
			"custom_data",
			"ornament_icon",
			"ornament_idfile",
			"ornament_hero_model",
			"item_unique_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"character_id",
			"slot_id",
			"item_id",
			"charges",
			"color",
			"augment_one",
			"augment_two",
			"augment_three",
			"augment_four",
			"augment_five",
			"augment_six",
			"instnodrop",
			"custom_data",
			"ornament_icon",
			"ornament_idfile",
			"ornament_hero_model",
			"item_unique_id",
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
		return std::string("inventory");
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

	static Inventory NewEntity()
	{
		Inventory e{};

		e.character_id        = 0;
		e.slot_id             = 0;
		e.item_id             = 0;
		e.charges             = 0;
		e.color               = 0;
		e.augment_one         = 0;
		e.augment_two         = 0;
		e.augment_three       = 0;
		e.augment_four        = 0;
		e.augment_five        = 0;
		e.augment_six         = 0;
		e.instnodrop          = 0;
		e.custom_data         = "";
		e.ornament_icon       = 0;
		e.ornament_idfile     = 0;
		e.ornament_hero_model = 0;
		e.item_unique_id      = "";

		return e;
	}

	static Inventory GetInventory(
		const std::vector<Inventory> &inventorys,
		int inventory_id
	)
	{
		for (auto &inventory : inventorys) {
			if (inventory.character_id == inventory_id) {
				return inventory;
			}
		}

		return NewEntity();
	}

	static Inventory FindOne(
		Database& db,
		int inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				inventory_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Inventory e{};

			e.character_id        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot_id             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges             = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.color               = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.augment_one         = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_two         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_three       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_four        = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_five        = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_six         = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.instnodrop          = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.custom_data         = row[12] ? row[12] : "";
			e.ornament_icon       = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.ornament_idfile     = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.ornament_hero_model = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.item_unique_id      = row[16] ? row[16] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int inventory_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				inventory_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Inventory &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.character_id));
		v.push_back(columns[1] + " = " + std::to_string(e.slot_id));
		v.push_back(columns[2] + " = " + std::to_string(e.item_id));
		v.push_back(columns[3] + " = " + std::to_string(e.charges));
		v.push_back(columns[4] + " = " + std::to_string(e.color));
		v.push_back(columns[5] + " = " + std::to_string(e.augment_one));
		v.push_back(columns[6] + " = " + std::to_string(e.augment_two));
		v.push_back(columns[7] + " = " + std::to_string(e.augment_three));
		v.push_back(columns[8] + " = " + std::to_string(e.augment_four));
		v.push_back(columns[9] + " = " + std::to_string(e.augment_five));
		v.push_back(columns[10] + " = " + std::to_string(e.augment_six));
		v.push_back(columns[11] + " = " + std::to_string(e.instnodrop));
		v.push_back(columns[12] + " = '" + Strings::Escape(e.custom_data) + "'");
		v.push_back(columns[13] + " = " + std::to_string(e.ornament_icon));
		v.push_back(columns[14] + " = " + std::to_string(e.ornament_idfile));
		v.push_back(columns[15] + " = " + std::to_string(e.ornament_hero_model));
		v.push_back(columns[16] + " = '" + Strings::Escape(e.item_unique_id) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.character_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Inventory InsertOne(
		Database& db,
		Inventory e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.color));
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.instnodrop));
		v.push_back("'" + Strings::Escape(e.custom_data) + "'");
		v.push_back(std::to_string(e.ornament_icon));
		v.push_back(std::to_string(e.ornament_idfile));
		v.push_back(std::to_string(e.ornament_hero_model));
		v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.character_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Inventory> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.color));
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.instnodrop));
			v.push_back("'" + Strings::Escape(e.custom_data) + "'");
			v.push_back(std::to_string(e.ornament_icon));
			v.push_back(std::to_string(e.ornament_idfile));
			v.push_back(std::to_string(e.ornament_hero_model));
			v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseInsert(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<Inventory> All(Database& db)
	{
		std::vector<Inventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Inventory e{};

			e.character_id        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot_id             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges             = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.color               = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.augment_one         = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_two         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_three       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_four        = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_five        = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_six         = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.instnodrop          = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.custom_data         = row[12] ? row[12] : "";
			e.ornament_icon       = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.ornament_idfile     = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.ornament_hero_model = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.item_unique_id      = row[16] ? row[16] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Inventory> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Inventory> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Inventory e{};

			e.character_id        = row[0] ? static_cast<uint32_t>(strtoul(row[0], nullptr, 10)) : 0;
			e.slot_id             = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.item_id             = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.charges             = row[3] ? static_cast<uint16_t>(strtoul(row[3], nullptr, 10)) : 0;
			e.color               = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.augment_one         = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.augment_two         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.augment_three       = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.augment_four        = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.augment_five        = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.augment_six         = row[10] ? static_cast<uint32_t>(strtoul(row[10], nullptr, 10)) : 0;
			e.instnodrop          = row[11] ? static_cast<uint8_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.custom_data         = row[12] ? row[12] : "";
			e.ornament_icon       = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.ornament_idfile     = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.ornament_hero_model = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.item_unique_id      = row[16] ? row[16] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int DeleteWhere(Database& db, const std::string &where_filter)
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
				"SELECT COALESCE(MAX({}), 0) FROM {}",
				PrimaryKey(),
				TableName()
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static int64 Count(Database& db, const std::string &where_filter = "")
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT COUNT(*) FROM {} {}",
				TableName(),
				(where_filter.empty() ? "" : "WHERE " + where_filter)
			)
		);

		return (results.Success() && results.begin()[0] ? strtoll(results.begin()[0], nullptr, 10) : 0);
	}

	static std::string BaseReplace()
	{
		return fmt::format(
			"REPLACE INTO {} ({}) ",
			TableName(),
			ColumnsRaw()
		);
	}

	static int ReplaceOne(
		Database& db,
		const Inventory &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.slot_id));
		v.push_back(std::to_string(e.item_id));
		v.push_back(std::to_string(e.charges));
		v.push_back(std::to_string(e.color));
		v.push_back(std::to_string(e.augment_one));
		v.push_back(std::to_string(e.augment_two));
		v.push_back(std::to_string(e.augment_three));
		v.push_back(std::to_string(e.augment_four));
		v.push_back(std::to_string(e.augment_five));
		v.push_back(std::to_string(e.augment_six));
		v.push_back(std::to_string(e.instnodrop));
		v.push_back("'" + Strings::Escape(e.custom_data) + "'");
		v.push_back(std::to_string(e.ornament_icon));
		v.push_back(std::to_string(e.ornament_idfile));
		v.push_back(std::to_string(e.ornament_hero_model));
		v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseReplace(),
				Strings::Implode(",", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<Inventory> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.slot_id));
			v.push_back(std::to_string(e.item_id));
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.color));
			v.push_back(std::to_string(e.augment_one));
			v.push_back(std::to_string(e.augment_two));
			v.push_back(std::to_string(e.augment_three));
			v.push_back(std::to_string(e.augment_four));
			v.push_back(std::to_string(e.augment_five));
			v.push_back(std::to_string(e.augment_six));
			v.push_back(std::to_string(e.instnodrop));
			v.push_back("'" + Strings::Escape(e.custom_data) + "'");
			v.push_back(std::to_string(e.ornament_icon));
			v.push_back(std::to_string(e.ornament_idfile));
			v.push_back(std::to_string(e.ornament_hero_model));
			v.push_back("'" + Strings::Escape(e.item_unique_id) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES {}",
				BaseReplace(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_BASE_INVENTORY_REPOSITORY_H
