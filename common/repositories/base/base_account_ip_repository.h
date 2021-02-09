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

#ifndef EQEMU_BASE_ACCOUNT_IP_REPOSITORY_H
#define EQEMU_BASE_ACCOUNT_IP_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseAccountIpRepository {
public:
	struct AccountIp {
		int         accid;
		std::string ip;
		int         count;
		std::string lastused;
	};

	static std::string PrimaryKey()
	{
		return std::string("accid");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"accid",
			"ip",
			"count",
			"lastused",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("account_ip");
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

	static AccountIp NewEntity()
	{
		AccountIp entry{};

		entry.accid    = 0;
		entry.ip       = "";
		entry.count    = 1;
		entry.lastused = current_timestamp();

		return entry;
	}

	static AccountIp GetAccountIpEntry(
		const std::vector<AccountIp> &account_ips,
		int account_ip_id
	)
	{
		for (auto &account_ip : account_ips) {
			if (account_ip.accid == account_ip_id) {
				return account_ip;
			}
		}

		return NewEntity();
	}

	static AccountIp FindOne(
		Database& db,
		int account_ip_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				account_ip_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			AccountIp entry{};

			entry.accid    = atoi(row[0]);
			entry.ip       = row[1] ? row[1] : "";
			entry.count    = atoi(row[2]);
			entry.lastused = row[3] ? row[3] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int account_ip_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				account_ip_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		AccountIp account_ip_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(account_ip_entry.accid));
		update_values.push_back(columns[1] + " = '" + EscapeString(account_ip_entry.ip) + "'");
		update_values.push_back(columns[2] + " = " + std::to_string(account_ip_entry.count));
		update_values.push_back(columns[3] + " = '" + EscapeString(account_ip_entry.lastused) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				account_ip_entry.accid
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static AccountIp InsertOne(
		Database& db,
		AccountIp account_ip_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(account_ip_entry.accid));
		insert_values.push_back("'" + EscapeString(account_ip_entry.ip) + "'");
		insert_values.push_back(std::to_string(account_ip_entry.count));
		insert_values.push_back("'" + EscapeString(account_ip_entry.lastused) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			account_ip_entry.accid = results.LastInsertedID();
			return account_ip_entry;
		}

		account_ip_entry = NewEntity();

		return account_ip_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<AccountIp> account_ip_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &account_ip_entry: account_ip_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(account_ip_entry.accid));
			insert_values.push_back("'" + EscapeString(account_ip_entry.ip) + "'");
			insert_values.push_back(std::to_string(account_ip_entry.count));
			insert_values.push_back("'" + EscapeString(account_ip_entry.lastused) + "'");

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

	static std::vector<AccountIp> All(Database& db)
	{
		std::vector<AccountIp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountIp entry{};

			entry.accid    = atoi(row[0]);
			entry.ip       = row[1] ? row[1] : "";
			entry.count    = atoi(row[2]);
			entry.lastused = row[3] ? row[3] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<AccountIp> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<AccountIp> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			AccountIp entry{};

			entry.accid    = atoi(row[0]);
			entry.ip       = row[1] ? row[1] : "";
			entry.count    = atoi(row[2]);
			entry.lastused = row[3] ? row[3] : "";

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

#endif //EQEMU_BASE_ACCOUNT_IP_REPOSITORY_H
