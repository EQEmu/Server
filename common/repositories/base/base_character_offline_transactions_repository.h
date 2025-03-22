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

#ifndef EQEMU_BASE_CHARACTER_OFFLINE_TRANSACTIONS_REPOSITORY_H
#define EQEMU_BASE_CHARACTER_OFFLINE_TRANSACTIONS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseCharacterOfflineTransactionsRepository {
public:
	struct CharacterOfflineTransactions {
		uint64_t    id;
		uint32_t    character_id;
		uint32_t    type;
		std::string item_name;
		int32_t     quantity;
		uint64_t    price;
		std::string buyer_name;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"character_id",
			"type",
			"item_name",
			"quantity",
			"price",
			"buyer_name",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"character_id",
			"type",
			"item_name",
			"quantity",
			"price",
			"buyer_name",
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
		return std::string("character_offline_transactions");
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

	static CharacterOfflineTransactions NewEntity()
	{
		CharacterOfflineTransactions e{};

		e.id           = 0;
		e.character_id = 0;
		e.type         = 0;
		e.item_name    = "";
		e.quantity     = 0;
		e.price        = 0;
		e.buyer_name   = "";

		return e;
	}

	static CharacterOfflineTransactions GetCharacterOfflineTransactions(
		const std::vector<CharacterOfflineTransactions> &character_offline_transactionss,
		int character_offline_transactions_id
	)
	{
		for (auto &character_offline_transactions : character_offline_transactionss) {
			if (character_offline_transactions.id == character_offline_transactions_id) {
				return character_offline_transactions;
			}
		}

		return NewEntity();
	}

	static CharacterOfflineTransactions FindOne(
		Database& db,
		int character_offline_transactions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				character_offline_transactions_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			CharacterOfflineTransactions e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.type         = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_name    = row[3] ? row[3] : "";
			e.quantity     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.price        = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.buyer_name   = row[6] ? row[6] : "";

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int character_offline_transactions_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				character_offline_transactions_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const CharacterOfflineTransactions &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[1] + " = " + std::to_string(e.character_id));
		v.push_back(columns[2] + " = " + std::to_string(e.type));
		v.push_back(columns[3] + " = '" + Strings::Escape(e.item_name) + "'");
		v.push_back(columns[4] + " = " + std::to_string(e.quantity));
		v.push_back(columns[5] + " = " + std::to_string(e.price));
		v.push_back(columns[6] + " = '" + Strings::Escape(e.buyer_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static CharacterOfflineTransactions InsertOne(
		Database& db,
		CharacterOfflineTransactions e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.item_name) + "'");
		v.push_back(std::to_string(e.quantity));
		v.push_back(std::to_string(e.price));
		v.push_back("'" + Strings::Escape(e.buyer_name) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<CharacterOfflineTransactions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.item_name) + "'");
			v.push_back(std::to_string(e.quantity));
			v.push_back(std::to_string(e.price));
			v.push_back("'" + Strings::Escape(e.buyer_name) + "'");

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

	static std::vector<CharacterOfflineTransactions> All(Database& db)
	{
		std::vector<CharacterOfflineTransactions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterOfflineTransactions e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.type         = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_name    = row[3] ? row[3] : "";
			e.quantity     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.price        = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.buyer_name   = row[6] ? row[6] : "";

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<CharacterOfflineTransactions> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<CharacterOfflineTransactions> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			CharacterOfflineTransactions e{};

			e.id           = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.character_id = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.type         = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.item_name    = row[3] ? row[3] : "";
			e.quantity     = row[4] ? static_cast<int32_t>(atoi(row[4])) : 0;
			e.price        = row[5] ? strtoull(row[5], nullptr, 10) : 0;
			e.buyer_name   = row[6] ? row[6] : "";

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
		const CharacterOfflineTransactions &e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.character_id));
		v.push_back(std::to_string(e.type));
		v.push_back("'" + Strings::Escape(e.item_name) + "'");
		v.push_back(std::to_string(e.quantity));
		v.push_back(std::to_string(e.price));
		v.push_back("'" + Strings::Escape(e.buyer_name) + "'");

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
		const std::vector<CharacterOfflineTransactions> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.character_id));
			v.push_back(std::to_string(e.type));
			v.push_back("'" + Strings::Escape(e.item_name) + "'");
			v.push_back(std::to_string(e.quantity));
			v.push_back(std::to_string(e.price));
			v.push_back("'" + Strings::Escape(e.buyer_name) + "'");

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

#endif //EQEMU_BASE_CHARACTER_OFFLINE_TRANSACTIONS_REPOSITORY_H
