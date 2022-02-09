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
#include "../../string_util.h"

class BaseBuyerRepository {
public:
	struct Buyer {
		int         charid;
		int         buyslot;
		int         itemid;
		std::string itemname;
		int         quantity;
		int         price;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("buyer");
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

	static Buyer NewEntity()
	{
		Buyer entry{};

		entry.charid   = 0;
		entry.buyslot  = 0;
		entry.itemid   = 0;
		entry.itemname = "";
		entry.quantity = 0;
		entry.price    = 0;

		return entry;
	}

	static Buyer GetBuyerEntry(
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
			Buyer entry{};

			entry.charid   = atoi(row[0]);
			entry.buyslot  = atoi(row[1]);
			entry.itemid   = atoi(row[2]);
			entry.itemname = row[3] ? row[3] : "";
			entry.quantity = atoi(row[4]);
			entry.price    = atoi(row[5]);

			return entry;
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
		Buyer buyer_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(buyer_entry.charid));
		update_values.push_back(columns[1] + " = " + std::to_string(buyer_entry.buyslot));
		update_values.push_back(columns[2] + " = " + std::to_string(buyer_entry.itemid));
		update_values.push_back(columns[3] + " = '" + EscapeString(buyer_entry.itemname) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(buyer_entry.quantity));
		update_values.push_back(columns[5] + " = " + std::to_string(buyer_entry.price));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				buyer_entry.charid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static Buyer InsertOne(
		Database& db,
		Buyer buyer_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(buyer_entry.charid));
		insert_values.push_back(std::to_string(buyer_entry.buyslot));
		insert_values.push_back(std::to_string(buyer_entry.itemid));
		insert_values.push_back("'" + EscapeString(buyer_entry.itemname) + "'");
		insert_values.push_back(std::to_string(buyer_entry.quantity));
		insert_values.push_back(std::to_string(buyer_entry.price));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			buyer_entry.charid = results.LastInsertedID();
			return buyer_entry;
		}

		buyer_entry = NewEntity();

		return buyer_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<Buyer> buyer_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &buyer_entry: buyer_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(buyer_entry.charid));
			insert_values.push_back(std::to_string(buyer_entry.buyslot));
			insert_values.push_back(std::to_string(buyer_entry.itemid));
			insert_values.push_back("'" + EscapeString(buyer_entry.itemname) + "'");
			insert_values.push_back(std::to_string(buyer_entry.quantity));
			insert_values.push_back(std::to_string(buyer_entry.price));

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
			Buyer entry{};

			entry.charid   = atoi(row[0]);
			entry.buyslot  = atoi(row[1]);
			entry.itemid   = atoi(row[2]);
			entry.itemname = row[3] ? row[3] : "";
			entry.quantity = atoi(row[4]);
			entry.price    = atoi(row[5]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<Buyer> GetWhere(Database& db, std::string where_filter)
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
			Buyer entry{};

			entry.charid   = atoi(row[0]);
			entry.buyslot  = atoi(row[1]);
			entry.itemid   = atoi(row[2]);
			entry.itemname = row[3] ? row[3] : "";
			entry.quantity = atoi(row[4]);
			entry.price    = atoi(row[5]);

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

#endif //EQEMU_BASE_BUYER_REPOSITORY_H
