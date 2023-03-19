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

#ifndef EQEMU_BASE_MERC_MERCHANT_TEMPLATE_ENTRIES_REPOSITORY_H
#define EQEMU_BASE_MERC_MERCHANT_TEMPLATE_ENTRIES_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>


class BaseMercMerchantTemplateEntriesRepository {
public:
	struct MercMerchantTemplateEntries {
		uint32_t merc_merchant_template_entry_id;
		uint32_t merc_merchant_template_id;
		uint32_t merc_template_id;
	};

	static std::string PrimaryKey()
	{
		return std::string("merc_merchant_template_entry_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merc_merchant_template_entry_id",
			"merc_merchant_template_id",
			"merc_template_id",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merc_merchant_template_entry_id",
			"merc_merchant_template_id",
			"merc_template_id",
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
		return std::string("merc_merchant_template_entries");
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

	static MercMerchantTemplateEntries NewEntity()
	{
		MercMerchantTemplateEntries e{};

		e.merc_merchant_template_entry_id = 0;
		e.merc_merchant_template_id       = 0;
		e.merc_template_id                = 0;

		return e;
	}

	static MercMerchantTemplateEntries GetMercMerchantTemplateEntries(
		const std::vector<MercMerchantTemplateEntries> &merc_merchant_template_entriess,
		int merc_merchant_template_entries_id
	)
	{
		for (auto &merc_merchant_template_entries : merc_merchant_template_entriess) {
			if (merc_merchant_template_entries.merc_merchant_template_entry_id == merc_merchant_template_entries_id) {
				return merc_merchant_template_entries;
			}
		}

		return NewEntity();
	}

	static MercMerchantTemplateEntries FindOne(
		Database& db,
		int merc_merchant_template_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				merc_merchant_template_entries_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			MercMerchantTemplateEntries e{};

			e.merc_merchant_template_entry_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_merchant_template_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.merc_template_id                = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merc_merchant_template_entries_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merc_merchant_template_entries_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const MercMerchantTemplateEntries &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.merc_merchant_template_id));
		v.push_back(columns[2] + " = " + std::to_string(e.merc_template_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merc_merchant_template_entry_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static MercMerchantTemplateEntries InsertOne(
		Database& db,
		MercMerchantTemplateEntries e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merc_merchant_template_entry_id));
		v.push_back(std::to_string(e.merc_merchant_template_id));
		v.push_back(std::to_string(e.merc_template_id));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merc_merchant_template_entry_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<MercMerchantTemplateEntries> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merc_merchant_template_entry_id));
			v.push_back(std::to_string(e.merc_merchant_template_id));
			v.push_back(std::to_string(e.merc_template_id));

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

	static std::vector<MercMerchantTemplateEntries> All(Database& db)
	{
		std::vector<MercMerchantTemplateEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercMerchantTemplateEntries e{};

			e.merc_merchant_template_entry_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_merchant_template_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.merc_template_id                = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<MercMerchantTemplateEntries> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<MercMerchantTemplateEntries> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			MercMerchantTemplateEntries e{};

			e.merc_merchant_template_entry_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.merc_merchant_template_id       = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.merc_template_id                = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

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

};

#endif //EQEMU_BASE_MERC_MERCHANT_TEMPLATE_ENTRIES_REPOSITORY_H
