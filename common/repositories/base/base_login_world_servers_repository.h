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

#ifndef EQEMU_BASE_LOGIN_WORLD_SERVERS_REPOSITORY_H
#define EQEMU_BASE_LOGIN_WORLD_SERVERS_REPOSITORY_H

#include "../../database.h"
#include "../../string_util.h"

class BaseLoginWorldServersRepository {
public:
	struct LoginWorldServers {
		int         id;
		std::string long_name;
		std::string short_name;
		std::string tag_description;
		int         login_server_list_type_id;
		std::string last_login_date;
		std::string last_ip_address;
		int         login_server_admin_id;
		int         is_server_trusted;
		std::string note;
	};

	static std::string PrimaryKey()
	{
		return std::string("id");
	}

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"long_name",
			"short_name",
			"tag_description",
			"login_server_list_type_id",
			"last_login_date",
			"last_ip_address",
			"login_server_admin_id",
			"is_server_trusted",
			"note",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("login_world_servers");
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

	static LoginWorldServers NewEntity()
	{
		LoginWorldServers entry{};

		entry.id                        = 0;
		entry.long_name                 = "";
		entry.short_name                = "";
		entry.tag_description           = "";
		entry.login_server_list_type_id = 0;
		entry.last_login_date           = 0;
		entry.last_ip_address           = "";
		entry.login_server_admin_id     = 0;
		entry.is_server_trusted         = 0;
		entry.note                      = "";

		return entry;
	}

	static LoginWorldServers GetLoginWorldServersEntry(
		const std::vector<LoginWorldServers> &login_world_serverss,
		int login_world_servers_id
	)
	{
		for (auto &login_world_servers : login_world_serverss) {
			if (login_world_servers.id == login_world_servers_id) {
				return login_world_servers;
			}
		}

		return NewEntity();
	}

	static LoginWorldServers FindOne(
		Database& db,
		int login_world_servers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE id = {} LIMIT 1",
				BaseSelect(),
				login_world_servers_id
			)
		);

		auto row = results.begin();
		if (results.RowCount() == 1) {
			LoginWorldServers entry{};

			entry.id                        = atoi(row[0]);
			entry.long_name                 = row[1] ? row[1] : "";
			entry.short_name                = row[2] ? row[2] : "";
			entry.tag_description           = row[3] ? row[3] : "";
			entry.login_server_list_type_id = atoi(row[4]);
			entry.last_login_date           = row[5] ? row[5] : "";
			entry.last_ip_address           = row[6] ? row[6] : "";
			entry.login_server_admin_id     = atoi(row[7]);
			entry.is_server_trusted         = atoi(row[8]);
			entry.note                      = row[9] ? row[9] : "";

			return entry;
		}

		return NewEntity();
	}

	static int DeleteOne(
		Database& db,
		int login_world_servers_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"DELETE FROM {} WHERE {} = {}",
				TableName(),
				PrimaryKey(),
				login_world_servers_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOne(
		Database& db,
		LoginWorldServers login_world_servers_entry
	)
	{
		std::vector<std::string> update_values;

		auto columns = Columns();

		update_values.push_back(columns[1] + " = '" + EscapeString(login_world_servers_entry.long_name) + "'");
		update_values.push_back(columns[2] + " = '" + EscapeString(login_world_servers_entry.short_name) + "'");
		update_values.push_back(columns[3] + " = '" + EscapeString(login_world_servers_entry.tag_description) + "'");
		update_values.push_back(columns[4] + " = " + std::to_string(login_world_servers_entry.login_server_list_type_id));
		update_values.push_back(columns[5] + " = '" + EscapeString(login_world_servers_entry.last_login_date) + "'");
		update_values.push_back(columns[6] + " = '" + EscapeString(login_world_servers_entry.last_ip_address) + "'");
		update_values.push_back(columns[7] + " = " + std::to_string(login_world_servers_entry.login_server_admin_id));
		update_values.push_back(columns[8] + " = " + std::to_string(login_world_servers_entry.is_server_trusted));
		update_values.push_back(columns[9] + " = '" + EscapeString(login_world_servers_entry.note) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET {} WHERE {} = {}",
				TableName(),
				implode(", ", update_values),
				PrimaryKey(),
				login_world_servers_entry.id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static LoginWorldServers InsertOne(
		Database& db,
		LoginWorldServers login_world_servers_entry
	)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(login_world_servers_entry.id));
		insert_values.push_back("'" + EscapeString(login_world_servers_entry.long_name) + "'");
		insert_values.push_back("'" + EscapeString(login_world_servers_entry.short_name) + "'");
		insert_values.push_back("'" + EscapeString(login_world_servers_entry.tag_description) + "'");
		insert_values.push_back(std::to_string(login_world_servers_entry.login_server_list_type_id));
		insert_values.push_back("'" + EscapeString(login_world_servers_entry.last_login_date) + "'");
		insert_values.push_back("'" + EscapeString(login_world_servers_entry.last_ip_address) + "'");
		insert_values.push_back(std::to_string(login_world_servers_entry.login_server_admin_id));
		insert_values.push_back(std::to_string(login_world_servers_entry.is_server_trusted));
		insert_values.push_back("'" + EscapeString(login_world_servers_entry.note) + "'");

		auto results = db.QueryDatabase(
			fmt::format(
				"{} VALUES ({})",
				BaseInsert(),
				implode(",", insert_values)
			)
		);

		if (results.Success()) {
			login_world_servers_entry.id = results.LastInsertedID();
			return login_world_servers_entry;
		}

		login_world_servers_entry = NewEntity();

		return login_world_servers_entry;
	}

	static int InsertMany(
		Database& db,
		std::vector<LoginWorldServers> login_world_servers_entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto &login_world_servers_entry: login_world_servers_entries) {
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(login_world_servers_entry.id));
			insert_values.push_back("'" + EscapeString(login_world_servers_entry.long_name) + "'");
			insert_values.push_back("'" + EscapeString(login_world_servers_entry.short_name) + "'");
			insert_values.push_back("'" + EscapeString(login_world_servers_entry.tag_description) + "'");
			insert_values.push_back(std::to_string(login_world_servers_entry.login_server_list_type_id));
			insert_values.push_back("'" + EscapeString(login_world_servers_entry.last_login_date) + "'");
			insert_values.push_back("'" + EscapeString(login_world_servers_entry.last_ip_address) + "'");
			insert_values.push_back(std::to_string(login_world_servers_entry.login_server_admin_id));
			insert_values.push_back(std::to_string(login_world_servers_entry.is_server_trusted));
			insert_values.push_back("'" + EscapeString(login_world_servers_entry.note) + "'");

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

	static std::vector<LoginWorldServers> All(Database& db)
	{
		std::vector<LoginWorldServers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{}",
				BaseSelect()
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginWorldServers entry{};

			entry.id                        = atoi(row[0]);
			entry.long_name                 = row[1] ? row[1] : "";
			entry.short_name                = row[2] ? row[2] : "";
			entry.tag_description           = row[3] ? row[3] : "";
			entry.login_server_list_type_id = atoi(row[4]);
			entry.last_login_date           = row[5] ? row[5] : "";
			entry.last_ip_address           = row[6] ? row[6] : "";
			entry.login_server_admin_id     = atoi(row[7]);
			entry.is_server_trusted         = atoi(row[8]);
			entry.note                      = row[9] ? row[9] : "";

			all_entries.push_back(entry);
		}

		return all_entries;
	}

	static std::vector<LoginWorldServers> GetWhere(Database& db, std::string where_filter)
	{
		std::vector<LoginWorldServers> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {}",
				BaseSelect(),
				where_filter
			)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			LoginWorldServers entry{};

			entry.id                        = atoi(row[0]);
			entry.long_name                 = row[1] ? row[1] : "";
			entry.short_name                = row[2] ? row[2] : "";
			entry.tag_description           = row[3] ? row[3] : "";
			entry.login_server_list_type_id = atoi(row[4]);
			entry.last_login_date           = row[5] ? row[5] : "";
			entry.last_ip_address           = row[6] ? row[6] : "";
			entry.login_server_admin_id     = atoi(row[7]);
			entry.is_server_trusted         = atoi(row[8]);
			entry.note                      = row[9] ? row[9] : "";

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

#endif //EQEMU_BASE_LOGIN_WORLD_SERVERS_REPOSITORY_H
