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

#ifndef EQEMU_BASE_MERCHANTLIST_REPOSITORY_H
#define EQEMU_BASE_MERCHANTLIST_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseMerchantlistRepository {
public:
	struct Merchantlist {
		int32_t     merchantid;
		int32_t     slot;
		int32_t     item;
		int16_t     faction_required;
		uint8_t     level_required;
		uint16_t    alt_currency_cost;
		int32_t     classes_required;
		int32_t     probability;
		std::string bucket_name;
		std::string bucket_value;
		uint8_t     bucket_comparison;
		int8_t      min_expansion;
		int8_t      max_expansion;
		std::string content_flags;
		std::string content_flags_disabled;
	};

	static std::string PrimaryKey()
	{
		return std::string("merchantid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"merchantid",
			"slot",
			"item",
			"faction_required",
			"level_required",
			"alt_currency_cost",
			"classes_required",
			"probability",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"merchantid",
			"slot",
			"item",
			"faction_required",
			"level_required",
			"alt_currency_cost",
			"classes_required",
			"probability",
			"bucket_name",
			"bucket_value",
			"bucket_comparison",
			"min_expansion",
			"max_expansion",
			"content_flags",
			"content_flags_disabled",
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
		return std::string("merchantlist");
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

	static Merchantlist NewEntity()
	{
		Merchantlist e{};

		e.merchantid             = 0;
		e.slot                   = 0;
		e.item                   = 0;
		e.faction_required       = -100;
		e.level_required         = 0;
		e.alt_currency_cost      = 0;
		e.classes_required       = 65535;
		e.probability            = 100;
		e.bucket_name            = "";
		e.bucket_value           = "";
		e.bucket_comparison      = 0;
		e.min_expansion          = -1;
		e.max_expansion          = -1;
		e.content_flags          = "";
		e.content_flags_disabled = "";

		return e;
	}

	static Merchantlist GetMerchantlist(
		const std::vector<Merchantlist> &merchantlists,
		int merchantlist_id
	)
	{
		for (auto &merchantlist : merchantlists) {
			if (merchantlist.merchantid == merchantlist_id) {
				return merchantlist;
			}
		}

		return NewEntity();
	}

	static Merchantlist FindOne(
		Database& db,
		int merchantlist_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				merchantlist_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Merchantlist e{};

			e.merchantid             = static_cast<int32_t>(atoi(row[0]));
			e.slot                   = static_cast<int32_t>(atoi(row[1]));
			e.item                   = static_cast<int32_t>(atoi(row[2]));
			e.faction_required       = static_cast<int16_t>(atoi(row[3]));
			e.level_required         = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.alt_currency_cost      = static_cast<uint16_t>(strtoul(row[5], nullptr, 10));
			e.classes_required       = static_cast<int32_t>(atoi(row[6]));
			e.probability            = static_cast<int32_t>(atoi(row[7]));
			e.bucket_name            = row[8] ? row[8] : "";
			e.bucket_value           = row[9] ? row[9] : "";
			e.bucket_comparison      = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.min_expansion          = static_cast<int8_t>(atoi(row[11]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[12]));
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int merchantlist_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				merchantlist_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Merchantlist &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.merchantid));
		v.push_back(columns[1] + " = " + std::to_string(e.slot));
		v.push_back(columns[2] + " = " + std::to_string(e.item));
		v.push_back(columns[3] + " = " + std::to_string(e.faction_required));
		v.push_back(columns[4] + " = " + std::to_string(e.level_required));
		v.push_back(columns[5] + " = " + std::to_string(e.alt_currency_cost));
		v.push_back(columns[6] + " = " + std::to_string(e.classes_required));
		v.push_back(columns[7] + " = " + std::to_string(e.probability));
		v.push_back(columns[8] + " = '" + Strings::Escape(e.bucket_name) + "'");
		v.push_back(columns[9] + " = '" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(columns[10] + " = " + std::to_string(e.bucket_comparison));
		v.push_back(columns[11] + " = " + std::to_string(e.min_expansion));
		v.push_back(columns[12] + " = " + std::to_string(e.max_expansion));
		v.push_back(columns[13] + " = '" + Strings::Escape(e.content_flags) + "'");
		v.push_back(columns[14] + " = '" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.merchantid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Merchantlist InsertOne(
		Database& db,
		Merchantlist e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.merchantid));
		v.push_back(std::to_string(e.slot));
		v.push_back(std::to_string(e.item));
		v.push_back(std::to_string(e.faction_required));
		v.push_back(std::to_string(e.level_required));
		v.push_back(std::to_string(e.alt_currency_cost));
		v.push_back(std::to_string(e.classes_required));
		v.push_back(std::to_string(e.probability));
		v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
		v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
		v.push_back(std::to_string(e.bucket_comparison));
		v.push_back(std::to_string(e.min_expansion));
		v.push_back(std::to_string(e.max_expansion));
		v.push_back("'" + Strings::Escape(e.content_flags) + "'");
		v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.merchantid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Merchantlist> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.merchantid));
			v.push_back(std::to_string(e.slot));
			v.push_back(std::to_string(e.item));
			v.push_back(std::to_string(e.faction_required));
			v.push_back(std::to_string(e.level_required));
			v.push_back(std::to_string(e.alt_currency_cost));
			v.push_back(std::to_string(e.classes_required));
			v.push_back(std::to_string(e.probability));
			v.push_back("'" + Strings::Escape(e.bucket_name) + "'");
			v.push_back("'" + Strings::Escape(e.bucket_value) + "'");
			v.push_back(std::to_string(e.bucket_comparison));
			v.push_back(std::to_string(e.min_expansion));
			v.push_back(std::to_string(e.max_expansion));
			v.push_back("'" + Strings::Escape(e.content_flags) + "'");
			v.push_back("'" + Strings::Escape(e.content_flags_disabled) + "'");

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

	static std::vector<Merchantlist> All(Database& db)
	{
		std::vector<Merchantlist> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Merchantlist e{};

			e.merchantid             = static_cast<int32_t>(atoi(row[0]));
			e.slot                   = static_cast<int32_t>(atoi(row[1]));
			e.item                   = static_cast<int32_t>(atoi(row[2]));
			e.faction_required       = static_cast<int16_t>(atoi(row[3]));
			e.level_required         = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.alt_currency_cost      = static_cast<uint16_t>(strtoul(row[5], nullptr, 10));
			e.classes_required       = static_cast<int32_t>(atoi(row[6]));
			e.probability            = static_cast<int32_t>(atoi(row[7]));
			e.bucket_name            = row[8] ? row[8] : "";
			e.bucket_value           = row[9] ? row[9] : "";
			e.bucket_comparison      = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.min_expansion          = static_cast<int8_t>(atoi(row[11]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[12]));
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Merchantlist> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Merchantlist> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Merchantlist e{};

			e.merchantid             = static_cast<int32_t>(atoi(row[0]));
			e.slot                   = static_cast<int32_t>(atoi(row[1]));
			e.item                   = static_cast<int32_t>(atoi(row[2]));
			e.faction_required       = static_cast<int16_t>(atoi(row[3]));
			e.level_required         = static_cast<uint8_t>(strtoul(row[4], nullptr, 10));
			e.alt_currency_cost      = static_cast<uint16_t>(strtoul(row[5], nullptr, 10));
			e.classes_required       = static_cast<int32_t>(atoi(row[6]));
			e.probability            = static_cast<int32_t>(atoi(row[7]));
			e.bucket_name            = row[8] ? row[8] : "";
			e.bucket_value           = row[9] ? row[9] : "";
			e.bucket_comparison      = static_cast<uint8_t>(strtoul(row[10], nullptr, 10));
			e.min_expansion          = static_cast<int8_t>(atoi(row[11]));
			e.max_expansion          = static_cast<int8_t>(atoi(row[12]));
			e.content_flags          = row[13] ? row[13] : "";
			e.content_flags_disabled = row[14] ? row[14] : "";

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

#endif //EQEMU_BASE_MERCHANTLIST_REPOSITORY_H
