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

#ifndef EQEMU_BASE_BUYER_REPOSITORY_H
#define EQEMU_BASE_BUYER_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseBuyerRepository {
public:
	struct Buyer {
		int32_t     charid;
		int32_t     buyslot;
		int32_t     itemid;
		std::string itemname;
		int32_t     quantity;
		int32_t     price;
	};

	static std::string PrimaryKey()
	{
		return std::string("charid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"charid",
			"buyslot",
			"itemid",
			"itemname",
			"quantity",
			"price",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"charid",
			"buyslot",
			"itemid",
			"itemname",
			"quantity",
			"price",
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
		return std::string("buyer");
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

	static Buyer NewEntity()
	{
		Buyer e{};

		e.charid   = 0;
		e.buyslot  = 0;
		e.itemid   = 0;
		e.itemname = "";
		e.quantity = 0;
		e.price    = 0;

		return e;
	}

	static Buyer GetBuyer(
		const std::vector<Buyer> &buyers,
		int buyer_id
	)
	{
		for (auto &buyer : buyers) {
			if (buyer.charid == buyer_id) {
				return buyer;
			}
		}

		return NewEntity();
	}

	static Buyer FindOne(
		Database& db,
		int buyer_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				buyer_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			Buyer e{};

			e.charid   = static_cast<int32_t>(atoi(row[0]));
			e.buyslot  = static_cast<int32_t>(atoi(row[1]));
			e.itemid   = static_cast<int32_t>(atoi(row[2]));
			e.itemname = row[3] ? row[3] : "";
			e.quantity = static_cast<int32_t>(atoi(row[4]));
			e.price    = static_cast<int32_t>(atoi(row[5]));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int buyer_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				buyer_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const Buyer &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.charid));
		v.push_back(columns[1] + " = " + std::to_string(e.buyslot));
		v.push_back(columns[2] + " = " + std::to_string(e.itemid));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.itemname) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.quantity));
		v.push_back(columns[5] + " = " + std::to_string(e.price));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Buyer InsertOne(
		Database& db,
		Buyer e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.charid));
		v.push_back(std::to_string(e.buyslot));
		v.push_back(std::to_string(e.itemid));
		v.push_back("'" + Strings::Escape(e.itemname) + "'");
		v.push_back(std::to_string(e.quantity));
		v.push_back(std::to_string(e.price));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.charid = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<Buyer> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.charid));
			v.push_back(std::to_string(e.buyslot));
			v.push_back(std::to_string(e.itemid));
			v.push_back("'" + Strings::Escape(e.itemname) + "'");
			v.push_back(std::to_string(e.quantity));
			v.push_back(std::to_string(e.price));

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

	static std::vector<Buyer> All(Database& db)
	{
		std::vector<Buyer> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Buyer e{};

			e.charid   = static_cast<int32_t>(atoi(row[0]));
			e.buyslot  = static_cast<int32_t>(atoi(row[1]));
			e.itemid   = static_cast<int32_t>(atoi(row[2]));
			e.itemname = row[3] ? row[3] : "";
			e.quantity = static_cast<int32_t>(atoi(row[4]));
			e.price    = static_cast<int32_t>(atoi(row[5]));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<Buyer> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<Buyer> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			Buyer e{};

			e.charid   = static_cast<int32_t>(atoi(row[0]));
			e.buyslot  = static_cast<int32_t>(atoi(row[1]));
			e.itemid   = static_cast<int32_t>(atoi(row[2]));
			e.itemname = row[3] ? row[3] : "";
			e.quantity = static_cast<int32_t>(atoi(row[4]));
			e.price    = static_cast<int32_t>(atoi(row[5]));

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

#endif //EQEMU_BASE_BUYER_REPOSITORY_H
