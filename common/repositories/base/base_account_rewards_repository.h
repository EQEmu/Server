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
#include "../../string_util.h"

class BaseAccountRewardsRepository {
public:
	struct AccountRewards {
		int account_id;
		int reward_id;
		int amount;
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

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("account_rewards");
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

	static AccountRewards NewEntity()
	{
		AccountRewards entry{};

		entry.account_id = 0;
		entry.reward_id  = 0;
		entry.amount     = 0;

		return entry;
	}

	static AccountRewards GetAccountRewardsEntry(
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
			AccountRewards entry{};

			entry.account_id = atoi(row[0]);
			entry.reward_id  = atoi(row[1]);
			entry.amount     = atoi(row[2]);

			return entry;
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
		AccountRewards account_rewards_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(account_rewards_entry.account_id));
		update_values.push_back(columns[1] + " = " + std::to_string(account_rewards_entry.reward_id));
		update_values.push_back(columns[2] + " = " + std::to_string(account_rewards_entry.amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				account_rewards_entry.account_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AccountRewards InsertOne(
		Database& db,
		AccountRewards account_rewards_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(account_rewards_entry.account_id));
		insert_values.push_back(std::to_string(account_rewards_entry.reward_id));
		insert_values.push_back(std::to_string(account_rewards_entry.amount));

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			account_rewards_entry.account_id = results.LastInsertedID();
			return account_rewards_entry;
		}

		account_rewards_entry = NewEntity();

		return account_rewards_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AccountRewards> account_rewards_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &account_rewards_entry: account_rewards_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(account_rewards_entry.account_id));
			insert_values.push_back(std::to_string(account_rewards_entry.reward_id));
			insert_values.push_back(std::to_string(account_rewards_entry.amount));

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
			AccountRewards entry{};

			entry.account_id = atoi(row[0]);
			entry.reward_id  = atoi(row[1]);
			entry.amount     = atoi(row[2]);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AccountRewards> GetWhere(Database& db, std::string where_filter)
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
			AccountRewards entry{};

			entry.account_id = atoi(row[0]);
			entry.reward_id  = atoi(row[1]);
			entry.amount     = atoi(row[2]);

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

#endif //EQEMU_BASE_ACCOUNT_REWARDS_REPOSITORY_H
