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
#include "../../strings.h"
#include <ctime>

class BaseMerchantlistTempRepository {
public:
	struct MerchantlistTemp {
		int npcid;
		int slot;
		int zone_id;
		int instance_id;
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
			"zone_id",
			"instance_id",
			"itemid",
			"charges",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"npcid",
			"slot",
			"zone_id",
			"instance_id",
			"itemid",
			"charges",
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
		return std::string("merchantlist_temp");
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

	static MerchantlistTemp NewEntity()
	{
		MerchantlistTemp e{};

		e.npcid       = 0;
		e.slot        = 0;
		e.zone_id     = 0;
		e.instance_id = 0;
		e.itemid      = 0;
		e.charges     = 1;

		return e;
	}

	static MerchantlistTemp GetMerchantlistTempe(
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
			MerchantlistTemp e{};

			e.npcid       = atoi(row[0]);
			e.slot        = atoi(row[1]);
			e.zone_id     = atoi(row[2]);
			e.instance_id = atoi(row[3]);
			e.itemid      = atoi(row[4]);
			e.charges     = atoi(row[5]);

			return e;
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
		MerchantlistTemp merchantlist_temp_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(merchantlist_temp_e.npcid));
		update_values.push_back(columns[1] + " = " + std::to_string(merchantlist_temp_e.slot));
		update_values.push_back(columns[2] + " = " + std::to_string(merchantlist_temp_e.zone_id));
		update_values.push_back(columns[3] + " = " + std::to_string(merchantlist_temp_e.instance_id));
		update_values.push_back(columns[4] + " = " + std::to_string(merchantlist_temp_e.itemid));
		update_values.push_back(columns[5] + " = " + std::to_string(merchantlist_temp_e.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				merchantlist_temp_e.npcid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MerchantlistTemp InsertOne(
		Database& db,
		MerchantlistTemp merchantlist_temp_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(merchantlist_temp_e.npcid));
		insert_values.push_back(std::to_string(merchantlist_temp_e.slot));
		insert_values.push_back(std::to_string(merchantlist_temp_e.zone_id));
		insert_values.push_back(std::to_string(merchantlist_temp_e.instance_id));
		insert_values.push_back(std::to_string(merchantlist_temp_e.itemid));
		insert_values.push_back(std::to_string(merchantlist_temp_e.charges));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			merchantlist_temp_e.npcid = results.LastInsertedID();
			return merchantlist_temp_e;
		}

		merchantlist_temp_e = NewEntity();

		return merchantlist_temp_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<MerchantlistTemp> merchantlist_temp_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &merchantlist_temp_e: merchantlist_temp_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(merchantlist_temp_e.npcid));
			insert_values.push_back(std::to_string(merchantlist_temp_e.slot));
			insert_values.push_back(std::to_string(merchantlist_temp_e.zone_id));
			insert_values.push_back(std::to_string(merchantlist_temp_e.instance_id));
			insert_values.push_back(std::to_string(merchantlist_temp_e.itemid));
			insert_values.push_back(std::to_string(merchantlist_temp_e.charges));

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
			MerchantlistTemp e{};

			e.npcid       = atoi(row[0]);
			e.slot        = atoi(row[1]);
			e.zone_id     = atoi(row[2]);
			e.instance_id = atoi(row[3]);
			e.itemid      = atoi(row[4]);
			e.charges     = atoi(row[5]);

			all_entries.push_back(e);
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
			MerchantlistTemp e{};

			e.npcid       = atoi(row[0]);
			e.slot        = atoi(row[1]);
			e.zone_id     = atoi(row[2]);
			e.instance_id = atoi(row[3]);
			e.itemid      = atoi(row[4]);
			e.charges     = atoi(row[5]);

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

#endif //EQEMU_BASE_MERCHANTLIST_TEMP_REPOSITORY_H
