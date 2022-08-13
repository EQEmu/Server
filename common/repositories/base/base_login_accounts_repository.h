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
#include "../../strings.h"
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
		return std::string(Strings::Implode(", ", Columns()));
	}

	static std::string SelectColumnsRaw()
	{
		return std::string(Strings::Implode(", ", SelectColumns()));
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
		LoginAccounts e{};

		e.id                 = 0;
		e.account_name       = "";
		e.account_password   = "";
		e.account_email      = "";
		e.source_loginserver = "";
		e.last_ip_address    = "";
		e.last_login_date    = 0;
		e.created_at         = 0;
		e.updated_at         = std::time(nullptr);

		return e;
	}

	static LoginAccounts GetLoginAccountse(
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
			LoginAccounts e{};

			e.id                 = atoi(row[0]);
			e.account_name       = row[1] ? row[1] : "";
			e.account_password   = row[2] ? row[2] : "";
			e.account_email      = row[3] ? row[3] : "";
			e.source_loginserver = row[4] ? row[4] : "";
			e.last_ip_address    = row[5] ? row[5] : "";
			e.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			e.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			return e;
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
		LoginAccounts login_accounts_e
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[0] + " = " + std::to_string(login_accounts_e.id));
		update_values.push_back(columns[1] + " = '" + Strings::Escape(login_accounts_e.account_name) + "'");
		update_values.push_back(columns[2] + " = '" + Strings::Escape(login_accounts_e.account_password) + "'");
		update_values.push_back(columns[3] + " = '" + Strings::Escape(login_accounts_e.account_email) + "'");
		update_values.push_back(columns[4] + " = '" + Strings::Escape(login_accounts_e.source_loginserver) + "'");
		update_values.push_back(columns[5] + " = '" + Strings::Escape(login_accounts_e.last_ip_address) + "'");
		update_values.push_back(columns[6] + " = FROM_UNIXTIME(" + (login_accounts_e.last_login_date > 0 ? std::to_string(login_accounts_e.last_login_date) : "null") + ")");
		update_values.push_back(columns[7] + " = FROM_UNIXTIME(" + (login_accounts_e.created_at > 0 ? std::to_string(login_accounts_e.created_at) : "null") + ")");
		update_values.push_back(columns[8] + " = FROM_UNIXTIME(" + (login_accounts_e.updated_at > 0 ? std::to_string(login_accounts_e.updated_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				Strings::Implode(", ", update_values),
				PrimaryKey(),
				login_accounts_e.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoginAccounts InsertOne(
		Database& db,
		LoginAccounts login_accounts_e
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(login_accounts_e.id));
		insert_values.push_back("'" + Strings::Escape(login_accounts_e.account_name) + "'");
		insert_values.push_back("'" + Strings::Escape(login_accounts_e.account_password) + "'");
		insert_values.push_back("'" + Strings::Escape(login_accounts_e.account_email) + "'");
		insert_values.push_back("'" + Strings::Escape(login_accounts_e.source_loginserver) + "'");
		insert_values.push_back("'" + Strings::Escape(login_accounts_e.last_ip_address) + "'");
		insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_e.last_login_date > 0 ? std::to_string(login_accounts_e.last_login_date) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_e.created_at > 0 ? std::to_string(login_accounts_e.created_at) : "null") + ")");
		insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_e.updated_at > 0 ? std::to_string(login_accounts_e.updated_at) : "null") + ")");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				Strings::Implode(",", insert_values)
			)
		);

		if (results.Success()) {
			login_accounts_e.id = results.LastInsertedID();
			return login_accounts_e;
		}

		login_accounts_e = NewEntity();

		return login_accounts_e;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoginAccounts> login_accounts_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &login_accounts_e: login_accounts_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(login_accounts_e.id));
			insert_values.push_back("'" + Strings::Escape(login_accounts_e.account_name) + "'");
			insert_values.push_back("'" + Strings::Escape(login_accounts_e.account_password) + "'");
			insert_values.push_back("'" + Strings::Escape(login_accounts_e.account_email) + "'");
			insert_values.push_back("'" + Strings::Escape(login_accounts_e.source_loginserver) + "'");
			insert_values.push_back("'" + Strings::Escape(login_accounts_e.last_ip_address) + "'");
			insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_e.last_login_date > 0 ? std::to_string(login_accounts_e.last_login_date) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_e.created_at > 0 ? std::to_string(login_accounts_e.created_at) : "null") + ")");
			insert_values.push_back("FROM_UNIXTIME(" + (login_accounts_e.updated_at > 0 ? std::to_string(login_accounts_e.updated_at) : "null") + ")");

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
			LoginAccounts e{};

			e.id                 = atoi(row[0]);
			e.account_name       = row[1] ? row[1] : "";
			e.account_password   = row[2] ? row[2] : "";
			e.account_email      = row[3] ? row[3] : "";
			e.source_loginserver = row[4] ? row[4] : "";
			e.last_ip_address    = row[5] ? row[5] : "";
			e.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			e.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

			all_entries.push_back(e);
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
			LoginAccounts e{};

			e.id                 = atoi(row[0]);
			e.account_name       = row[1] ? row[1] : "";
			e.account_password   = row[2] ? row[2] : "";
			e.account_email      = row[3] ? row[3] : "";
			e.source_loginserver = row[4] ? row[4] : "";
			e.last_ip_address    = row[5] ? row[5] : "";
			e.last_login_date    = strtoll(row[6] ? row[6] : "-1", nullptr, 10);
			e.created_at         = strtoll(row[7] ? row[7] : "-1", nullptr, 10);
			e.updated_at         = strtoll(row[8] ? row[8] : "-1", nullptr, 10);

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

#endif //EQEMU_BASE_LOGIN_ACCOUNTS_REPOSITORY_H
