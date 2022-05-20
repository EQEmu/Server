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

#ifndef EQEMU_BASE_LOGIN_ACCOUNTS_REPOSITORY_H
#define EQEMU_BASE_LOGIN_ACCOUNTS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"
#include <ctime>

class BaseLoginAccountsRepository {
public:
	struct LoginAccounts {
		int         id;
		std::string account_name;
		std::string account_password;
		std::string account_email;
		std::string source_loginserver;
		std::string last_ip_address;
		time_t      last_login_date;
		time_t      created_at;
		time_t      updated_at;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"account_name",
			"account_password",
			"account_email",
			"source_loginserver",
			"last_ip_address",
			"last_login_date",
			"created_at",
			"updated_at",
		};
	}

	static std::vector<std::string> SelectColumns()
	{
		return {
			"id",
			"account_name",
			"account_password",
			"account_email",
			"source_loginserver",
			"last_ip_address",
			"UNIX_TIMESTAMP(last_login_date)",
			"UNIX_TIMESTAMP(created_at)",
			"UNIX_TIMESTAMP(updated_at)",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(implode(", ", SelectColumns()));
	}

	static std::string TableName()
	{
		return std::string("login_accounts");
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

	static LoginAccounts NewEntity()
	{
		LoginAccounts entry{};

		entry.id                 = 0;
		entry.account_name       = "";
		entry.account_password   = "";
		entry.account_email      = "";
		entry.source_loginserver = "";
		entry.last_ip_address    = "";
		entry.last_login_date    = 0;
		entry.created_at         = 0;
		entry.updated_at         = std::time(nullptr);

		return entry;
	}

	static LoginAccounts GetLoginAccountsEntry(
		const std::vector<LoginAccounts> &login_accountss,
		int login_accounts_id
	)
	{
		for (auto &login_accounts : login_accountss) {
			if (login_accounts.id == login_accounts_id) {
				return login_accounts;
			}
		}

		return NewEntity();
	}

	static LoginAccounts FindOne(
		Database& db,
		int login_accounts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				login_accounts_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginAccounts entry{};

			entry.id                 = atoi(row[0]);
			entry.account_name       = row[1] ? row[1] : "";
			entry.account_password   = row[2] ? row[2] : "";
			entry.account_email      = row[3] ? row[3] : "";
			entry.source_loginserver = row[4] ? row[4] : "";
			entry.last_ip_address    = row[5] ? row[5] : "";
			entry.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			entry.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			entry.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_accounts_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_accounts_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LoginAccounts login_accounts_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(login_accounts_entry.id));
		update_values.push_back(columns[1] + " = '" + EscapeString(login_accounts_entry.account_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(login_accounts_entry.account_password) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(login_accounts_entry.account_email) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(login_accounts_entry.source_loginserver) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(login_accounts_entry.last_ip_address) + "'");
		update_values.push_back(columns[6] + " = FROM_UNIXTIME(" + (login_accounts_entry.last_login_date > 0 ? std::to_string(login_accounts_entry.last_login_date) : "null") + ")");
		update_values.push_back(columns[7] + " = FROM_UNIXTIME(" + (login_accounts_entry.created_at > 0 ? std::to_string(login_accounts_entry.created_at) : "null") + ")");
		update_values.push_back(columns[8] + " = FROM_UNIXTIME(" + (login_accounts_entry.updated_at > 0 ? std::to_string(login_accounts_entry.updated_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				login_accounts_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoginAccounts InsertOne(
		Database& db,
		LoginAccounts login_accounts_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(login_accounts_entry.id));
		insert_values.push_back("'" + EscapeString(login_accounts_entry.account_name) + "'");
		insert_values.push_back("'" + EscapeString(login_accounts_entry.account_password) + "'");
		insert_values.push_back("'" + EscapeString(login_accounts_entry.account_email) + "'");
		insert_values.push_back("'" + EscapeString(login_accounts_entry.source_loginserver) + "'");
		insert_values.push_back("'" + EscapeString(login_accounts_entry.last_ip_address) + "'");
		insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_entry.last_login_date > 0 ? std::to_string(login_accounts_entry.last_login_date) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_entry.created_at > 0 ? std::to_string(login_accounts_entry.created_at) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_entry.updated_at > 0 ? std::to_string(login_accounts_entry.updated_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			login_accounts_entry.id = results.LastInsertedID();
			return login_accounts_entry;
		}

		login_accounts_entry = NewEntity();

		return login_accounts_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoginAccounts> login_accounts_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &login_accounts_entry: login_accounts_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(login_accounts_entry.id));
			insert_values.push_back("'" + EscapeString(login_accounts_entry.account_name) + "'");
			insert_values.push_back("'" + EscapeString(login_accounts_entry.account_password) + "'");
			insert_values.push_back("'" + EscapeString(login_accounts_entry.account_email) + "'");
			insert_values.push_back("'" + EscapeString(login_accounts_entry.source_loginserver) + "'");
			insert_values.push_back("'" + EscapeString(login_accounts_entry.last_ip_address) + "'");
			insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_entry.last_login_date > 0 ? std::to_string(login_accounts_entry.last_login_date) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_entry.created_at > 0 ? std::to_string(login_accounts_entry.created_at) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_entry.updated_at > 0 ? std::to_string(login_accounts_entry.updated_at) : "null") + ")");

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

	static std::vector<LoginAccounts> All(Database& db)
	{
		std::vector<LoginAccounts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginAccounts entry{};

			entry.id                 = atoi(row[0]);
			entry.account_name       = row[1] ? row[1] : "";
			entry.account_password   = row[2] ? row[2] : "";
			entry.account_email      = row[3] ? row[3] : "";
			entry.source_loginserver = row[4] ? row[4] : "";
			entry.last_ip_address    = row[5] ? row[5] : "";
			entry.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			entry.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			entry.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LoginAccounts> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LoginAccounts> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginAccounts entry{};

			entry.id                 = atoi(row[0]);
			entry.account_name       = row[1] ? row[1] : "";
			entry.account_password   = row[2] ? row[2] : "";
			entry.account_email      = row[3] ? row[3] : "";
			entry.source_loginserver = row[4] ? row[4] : "";
			entry.last_ip_address    = row[5] ? row[5] : "";
			entry.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			entry.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			entry.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

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

#endif //EQEMU_BASE_LOGIN_ACCOUNTS_REPOSITORY_H
