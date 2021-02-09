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

#ifndef EQEMU_BASE_LOGIN_SERVER_ADMINS_REPOSITORY_H
#define EQEMU_BASE_LOGIN_SERVER_ADMINS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLoginServerAdminsRepository {
public:
	struct LoginServerAdmins {
		int         id;
		std::string account_name;
		std::string account_password;
		std::string first_name;
		std::string last_name;
		std::string email;
		std::string registration_date;
		std::string registration_ip_address;
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
			"first_name",
			"last_name",
			"email",
			"registration_date",
			"registration_ip_address",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("login_server_admins");
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

	static LoginServerAdmins NewEntity()
	{
		LoginServerAdmins entry{};

		entry.id                      = 0;
		entry.account_name            = "";
		entry.account_password        = "";
		entry.first_name              = "";
		entry.last_name               = "";
		entry.email                   = "";
		entry.registration_date       = 0;
		entry.registration_ip_address = "";

		return entry;
	}

	static LoginServerAdmins GetLoginServerAdminsEntry(
		const std::vector<LoginServerAdmins> &login_server_adminss,
		int login_server_admins_id
	)
	{
		for (auto &login_server_admins : login_server_adminss) {
			if (login_server_admins.id == login_server_admins_id) {
				return login_server_admins;
			}
		}

		return NewEntity();
	}

	static LoginServerAdmins FindOne(
		Database& db,
		int login_server_admins_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				login_server_admins_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginServerAdmins entry{};

			entry.id                      = atoi(row[0]);
			entry.account_name            = row[1] ? row[1] : "";
			entry.account_password        = row[2] ? row[2] : "";
			entry.first_name              = row[3] ? row[3] : "";
			entry.last_name               = row[4] ? row[4] : "";
			entry.email                   = row[5] ? row[5] : "";
			entry.registration_date       = row[6] ? row[6] : "";
			entry.registration_ip_address = row[7] ? row[7] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_server_admins_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_server_admins_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LoginServerAdmins login_server_admins_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(login_server_admins_entry.account_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(login_server_admins_entry.account_password) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(login_server_admins_entry.first_name) + "'");
		update_values.push_back(columns[4] + " = '" + EscapeString(login_server_admins_entry.last_name) + "'");
		update_values.push_back(columns[5] + " = '" + EscapeString(login_server_admins_entry.email) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(login_server_admins_entry.registration_date) + "'");
		update_values.push_back(columns[7] + " = '" + EscapeString(login_server_admins_entry.registration_ip_address) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				login_server_admins_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoginServerAdmins InsertOne(
		Database& db,
		LoginServerAdmins login_server_admins_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(login_server_admins_entry.id));
		insert_values.push_back("'" + EscapeString(login_server_admins_entry.account_name) + "'");
		insert_values.push_back("'" + EscapeString(login_server_admins_entry.account_password) + "'");
		insert_values.push_back("'" + EscapeString(login_server_admins_entry.first_name) + "'");
		insert_values.push_back("'" + EscapeString(login_server_admins_entry.last_name) + "'");
		insert_values.push_back("'" + EscapeString(login_server_admins_entry.email) + "'");
		insert_values.push_back("'" + EscapeString(login_server_admins_entry.registration_date) + "'");
		insert_values.push_back("'" + EscapeString(login_server_admins_entry.registration_ip_address) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			login_server_admins_entry.id = results.LastInsertedID();
			return login_server_admins_entry;
		}

		login_server_admins_entry = NewEntity();

		return login_server_admins_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoginServerAdmins> login_server_admins_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &login_server_admins_entry: login_server_admins_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(login_server_admins_entry.id));
			insert_values.push_back("'" + EscapeString(login_server_admins_entry.account_name) + "'");
			insert_values.push_back("'" + EscapeString(login_server_admins_entry.account_password) + "'");
			insert_values.push_back("'" + EscapeString(login_server_admins_entry.first_name) + "'");
			insert_values.push_back("'" + EscapeString(login_server_admins_entry.last_name) + "'");
			insert_values.push_back("'" + EscapeString(login_server_admins_entry.email) + "'");
			insert_values.push_back("'" + EscapeString(login_server_admins_entry.registration_date) + "'");
			insert_values.push_back("'" + EscapeString(login_server_admins_entry.registration_ip_address) + "'");

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

	static std::vector<LoginServerAdmins> All(Database& db)
	{
		std::vector<LoginServerAdmins> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginServerAdmins entry{};

			entry.id                      = atoi(row[0]);
			entry.account_name            = row[1] ? row[1] : "";
			entry.account_password        = row[2] ? row[2] : "";
			entry.first_name              = row[3] ? row[3] : "";
			entry.last_name               = row[4] ? row[4] : "";
			entry.email                   = row[5] ? row[5] : "";
			entry.registration_date       = row[6] ? row[6] : "";
			entry.registration_ip_address = row[7] ? row[7] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LoginServerAdmins> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LoginServerAdmins> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginServerAdmins entry{};

			entry.id                      = atoi(row[0]);
			entry.account_name            = row[1] ? row[1] : "";
			entry.account_password        = row[2] ? row[2] : "";
			entry.first_name              = row[3] ? row[3] : "";
			entry.last_name               = row[4] ? row[4] : "";
			entry.email                   = row[5] ? row[5] : "";
			entry.registration_date       = row[6] ? row[6] : "";
			entry.registration_ip_address = row[7] ? row[7] : "";

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

#endif //EQEMU_BASE_LOGIN_SERVER_ADMINS_REPOSITORY_H
