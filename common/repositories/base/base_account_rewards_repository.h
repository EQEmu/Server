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

#ifndef EQEMU_BASE_ACCOUNT_REWARDS_REPOSITORY_H
#define EQEMU_BASE_ACCOUNT_REWARDS_REPOSITORY_H

#include "../../database.h"
#include "../../strings.h"
#include <ctime>

class BaseAccountRewardsRepository {
public:
	struct AccountRewards {
		uint32_t account_id;
		uint32_t reward_id;
		uint32_t amount;
	};

	static std::string PrimaryKey()
	{
		return std::string("account_id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"account_id",
			"reward_id",
			"amount",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"account_id",
			"reward_id",
			"amount",
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
		return std::string("account_rewards");
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

	static AccountRewards NewEntity()
	{
		AccountRewards e{};

		e.account_id = 0;
		e.reward_id  = 0;
		e.amount     = 0;

		return e;
	}

	static AccountRewards GetAccountRewards(
		const std::vector<AccountRewards> &account_rewardss,
		int account_rewards_id
	)
	{
		for (auto &account_rewards : account_rewardss) {
			if (account_rewards.account_id == account_rewards_id) {
				return account_rewards;
			}
		}

		return NewEntity();
	}

	static AccountRewards FindOne(
		Database& db,
		int account_rewards_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				account_rewards_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AccountRewards e{};

			e.account_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.reward_id  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.amount     = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			return e;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int account_rewards_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				account_rewards_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		const AccountRewards &e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(columns[0] + " = " + std::to_string(e.account_id));
		v.push_back(columns[1] + " = " + std::to_string(e.reward_id));
		v.push_back(columns[2] + " = " + std::to_string(e.amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", v),
				PrimaryKey(),
				e.account_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AccountRewards InsertOne(
		Database& db,
		AccountRewards e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.account_id));
		v.push_back(std::to_string(e.reward_id));
		v.push_back(std::to_string(e.amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", v)
			)
		);

		if (results.Success()) {
			e.account_id = results.LastInsertedID();
			return e;
		}

		e = NewEntity();

		return e;
	}

	static int InsertMany(
		Database& db,
		const std::vector<AccountRewards> &entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &e: entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.account_id));
			v.push_back(std::to_string(e.reward_id));
			v.push_back(std::to_string(e.amount));

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

	static std::vector<AccountRewards> All(Database& db)
	{
		std::vector<AccountRewards> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountRewards e{};

			e.account_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.reward_id  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.amount     = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static std::vector<AccountRewards> GetWhere(Database& db, const std::string &where_filter)
	{
		std::vector<AccountRewards> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountRewards e{};

			e.account_id = static_cast<uint32_t>(strtoul(row[0], nullptr, 10));
			e.reward_id  = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.amount     = static_cast<uint32_t>(strtoul(row[2], nullptr, 10));

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

#endif //EQEMU_BASE_ACCOUNT_REWARDS_REPOSITORY_H
