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

#ifndef EQEMU_BASE_MERCHANTLIST_TEMP_REPOSITORY_H
#define EQEMU_BASE_MERCHANTLIST_TEMP_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseMerchantlistTempRepository {
public:
	struct MerchantlistTemp {
		int npcid;
		int slot;
		int itemid;
		int charges;
	};

	static std::string PrimaryKey()
	{
		return std::string("npcid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"npcid",
			"slot",
			"itemid",
			"charges",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("merchantlist_temp");
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

	static MerchantlistTemp NewEntity()
	{
		MerchantlistTemp entry{};

		entry.npcid   = 0;
		entry.slot    = 0;
		entry.itemid  = 0;
		entry.charges = 1;

		return entry;
	}

	static MerchantlistTemp GetMerchantlistTempEntry(
		const std::vector<MerchantlistTemp> &merchantlist_temps,
		int merchantlist_temp_id
	)
	{
		for (auto &merchantlist_temp : merchantlist_temps) {
			if (merchantlist_temp.npcid == merchantlist_temp_id) {
				return merchantlist_temp;
			}
		}

		return NewEntity();
	}

	static MerchantlistTemp FindOne(
		Database& db,
		int merchantlist_temp_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				merchantlist_temp_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MerchantlistTemp entry{};

			entry.npcid   = atoi(row[0]);
			entry.slot    = atoi(row[1]);
			entry.itemid  = atoi(row[2]);
			entry.charges = atoi(row[3]);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merchantlist_temp_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merchantlist_temp_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		MerchantlistTemp merchantlist_temp_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(merchantlist_temp_entry.npcid));
		update_values.push_back(columns[1] + " = " + std::to_string(merchantlist_temp_entry.slot));
		update_values.push_back(columns[2] + " = " + std::to_string(merchantlist_temp_entry.itemid));
		update_values.push_back(columns[3] + " = " + std::to_string(merchantlist_temp_entry.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				merchantlist_temp_entry.npcid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MerchantlistTemp InsertOne(
		Database& db,
		MerchantlistTemp merchantlist_temp_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(merchantlist_temp_entry.npcid));
		insert_values.push_back(std::to_string(merchantlist_temp_entry.slot));
		insert_values.push_back(std::to_string(merchantlist_temp_entry.itemid));
		insert_values.push_back(std::to_string(merchantlist_temp_entry.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			merchantlist_temp_entry.npcid = results.LastInsertedID();
			return merchantlist_temp_entry;
		}

		merchantlist_temp_entry = NewEntity();

		return merchantlist_temp_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<MerchantlistTemp> merchantlist_temp_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &merchantlist_temp_entry: merchantlist_temp_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(merchantlist_temp_entry.npcid));
			insert_values.push_back(std::to_string(merchantlist_temp_entry.slot));
			insert_values.push_back(std::to_string(merchantlist_temp_entry.itemid));
			insert_values.push_back(std::to_string(merchantlist_temp_entry.charges));

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

	static std::vector<MerchantlistTemp> All(Database& db)
	{
		std::vector<MerchantlistTemp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MerchantlistTemp entry{};

			entry.npcid   = atoi(row[0]);
			entry.slot    = atoi(row[1]);
			entry.itemid  = atoi(row[2]);
			entry.charges = atoi(row[3]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<MerchantlistTemp> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<MerchantlistTemp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MerchantlistTemp entry{};

			entry.npcid   = atoi(row[0]);
			entry.slot    = atoi(row[1]);
			entry.itemid  = atoi(row[2]);
			entry.charges = atoi(row[3]);

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

#endif //EQEMU_BASE_MERCHANTLIST_TEMP_REPOSITORY_H
