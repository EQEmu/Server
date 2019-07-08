/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "../common/global_define.h"

#include "database.h"
#include "login_server.h"
#include "../common/eqemu_logsys.h"
#include "../common/string_util.h"
#include "../common/util/uuid.h"

extern LoginServer server;

/**
 * Initial connect
 *
 * @param user
 * @param pass
 * @param host
 * @param port
 * @param name
 */
Database::Database(
	std::string user,
	std::string pass,
	std::string host,
	std::string port,
	std::string name
)
{
	this->user = user;
	this->pass = pass;
	this->host = host;
	this->name = name;

	uint32 errnum = 0;
	char   errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(
		host.c_str(),
		user.c_str(),
		pass.c_str(),
		name.c_str(),
		atoi(port.c_str()),
		&errnum,
		errbuf
	)
		) {
		Log(Logs::General, Logs::Error, "Failed to connect to database: Error: %s", errbuf);
		exit(1);
	}
	else {
		Log(Logs::General, Logs::Status, "Using database '%s' at %s:%d", database, host, port);
	}
}

/**
 * Deconstructor
 */
Database::~Database()
{
	if (database) {
		mysql_close(database);
	}
}

/**
 * @param name
 * @param loginserver
 * @param password
 * @param id
 * @return
 */
bool Database::GetLoginDataFromAccountInfo(
	const std::string &name,
	const std::string &loginserver,
	std::string &password,
	unsigned int &id
)
{
	auto query = fmt::format(
		"SELECT id, account_password FROM login_accounts WHERE account_name = '{0}' AND source_loginserver = '{1}' LIMIT 1",
		EscapeString(name),
		EscapeString(loginserver)
	);

	auto results = QueryDatabase(query);

	if (results.RowCount() != 1) {
		LogDebug(
			"Could not find account for name [{0}] login [{1}]",
			name,
			loginserver
		);

		return false;
	}

	if (!results.Success()) {
		return false;
	}

	auto row = results.begin();

	id       = atoi(row[0]);
	password = row[1];

	LogDebug(
		"Found account for name [{0}] login [{1}]",
		name,
		loginserver
	);

	return true;
}

/**
 * @param token
 * @param ip
 * @param db_account_id
 * @param db_loginserver
 * @param user
 * @return
 */
bool Database::GetLoginTokenDataFromToken(
	const std::string &token,
	const std::string &ip,
	unsigned int &db_account_id,
	std::string &db_loginserver,
	std::string &user
)
{
	auto query = fmt::format(
		"SELECT tbllogintokens.Id, tbllogintokens.IpAddress, tbllogintokenclaims.Name, tbllogintokenclaims.Value FROM tbllogintokens "
		"JOIN tbllogintokenclaims ON tbllogintokens.Id = tbllogintokenclaims.TokenId WHERE tbllogintokens.Expires > NOW() "
		"AND tbllogintokens.Id='{0}' AND tbllogintokens.IpAddress='{1}'",
		EscapeString(token),
		EscapeString(ip)
	);

	auto results = QueryDatabase(query);
	if (results.RowCount() == 0 || !results.Success()) {
		return false;
	}

	bool      found_username          = false;
	bool      found_login_id          = false;
	bool      found_login_server_name = false;
	for (auto row                     = results.begin(); row != results.end(); ++row) {
		if (strcmp(row[2], "username") == 0) {
			user           = row[3];
			found_username = true;
			continue;
		}

		if (strcmp(row[2], "login_server_id") == 0) {
			db_account_id  = atoi(row[3]);
			found_login_id = true;
			continue;
		}

		if (strcmp(row[2], "login_server_name") == 0) {
			db_loginserver          = row[3];
			found_login_server_name = true;
			continue;
		}
	}

	return found_username && found_login_id && found_login_server_name;
}

/**
 * @param loginserver
 * @return
 */
unsigned int Database::GetFreeID(const std::string &loginserver)
{
	auto query = fmt::format(
		"SELECT IFNULL(MAX(id), 0) + 1 FROM login_accounts WHERE source_loginserver = '{0}'",
		EscapeString(loginserver)
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() != 1) {
		return 0;
	}

	auto row = results.begin();

	return std::stoi(row[0]);
}

/**
 * @param name
 * @param password
 * @param loginserver
 * @param id
 * @return
 */
bool Database::CreateLoginData(
	const std::string &name,
	const std::string &password,
	const std::string &loginserver,
	unsigned int &id
)
{
	uint32 free_id = GetFreeID(loginserver);
	id = free_id;

	return CreateLoginDataWithID(name, password, loginserver, free_id);
}

/**
 * @param in_account_name
 * @param in_account_password
 * @param loginserver
 * @param id
 * @return
 */
bool Database::CreateLoginDataWithID(
	const std::string &in_account_name,
	const std::string &in_account_password,
	const std::string &loginserver,
	unsigned int id
)
{
	if (id == 0) {
		return false;
	}

	auto query = fmt::format(
		"INSERT INTO login_accounts (id, source_loginserver, account_name, account_password, account_email, last_login_date, last_ip_address, created_at) "
		"VALUES ({0}, '{1}', '{2}', '{3}', 'local_creation', NOW(), '127.0.0.1', NOW())",
		id,
		EscapeString(loginserver),
		EscapeString(in_account_name),
		EscapeString(in_account_password)
	);

	auto results = QueryDatabase(query);

	return results.Success();
}

/**
 * @param name
 * @param password
 * @param loginserver
 * @param id
 * @return
 */
bool Database::DoesLoginServerAccountExist(
	const std::string &name,
	const std::string &password,
	const std::string &loginserver,
	unsigned int id
)
{
	if (id == 0) {
		return false;
	}

	auto query = fmt::format(
		"SELECT account_name FROM login_accounts WHERE account_name = '{0}' AND source_loginserver = '{1}'",
		EscapeString(name),
		EscapeString(loginserver)
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() != 1) {
		return false;
	}

	return true;
}

/**
 * @param name
 * @param loginserver
 * @param hash
 */
void Database::UpdateLoginHash(
	const std::string &name,
	const std::string &loginserver,
	const std::string &hash
)
{
	LogDebug(
		"name [{0}] loginserver [{1}] hash [{2}]",
		name,
		loginserver,
		hash
	);

	auto query = fmt::format(
		"UPDATE login_accounts SET account_password = '{0}' WHERE account_name = '{1}' AND source_loginserver = '{2}'",
		hash,
		EscapeString(name),
		EscapeString(loginserver)
	);

	QueryDatabase(query);
}

/**
 * @param long_name
 * @param short_name
 * @param id
 * @param desc
 * @param list_id
 * @param trusted
 * @param list_desc
 * @param account
 * @param password
 * @return
 */
bool Database::GetWorldRegistration(
	std::string long_name,
	std::string short_name,
	unsigned int &id,
	std::string &desc,
	unsigned int &list_id,
	unsigned int &trusted,
	std::string &list_desc,
	std::string &account,
	std::string &password
)
{
	auto query = fmt::format(
		"SELECT\n"
		"  ifnull(WSR.id, 999999) AS server_id,\n"
		"  WSR.tag_description,\n"
		"  ifnull(WSR.is_server_trusted, 0) AS is_server_trusted,\n"
		"  ifnull(SLT.id, 3) AS login_server_list_type_id,\n"
		"  SLT.description,\n"
		"  ifnull(WSR.login_server_admin_id, 0) AS login_server_admin_id\n"
		"FROM\n"
		"  login_world_servers AS WSR\n"
		"  JOIN login_server_list_types AS SLT ON WSR.login_server_list_type_id = SLT.id\n"
		"WHERE\n"
		"  WSR.short_name = '{0}' LIMIT 1",
		EscapeString(short_name)
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() != 1) {
		return false;
	}

	auto row = results.begin();

	id        = atoi(row[0]);
	desc      = row[1];
	trusted   = atoi(row[2]);
	list_id   = atoi(row[3]);
	list_desc = row[4];

	int db_account_id = atoi(row[5]);
	if (db_account_id > 0) {

		auto world_registration_query = fmt::format(
			"SELECT account_name, account_password FROM login_server_admins WHERE id = {0} LIMIT 1",
			db_account_id
		);

		auto world_registration_results = QueryDatabase(world_registration_query);
		if (!world_registration_results.Success() || world_registration_results.RowCount() != 1) {
			return false;
		}

		auto world_registration_row = world_registration_results.begin();

		account  = world_registration_row[0];
		password = world_registration_row[1];
	}

	return true;
}

/**
 * @param id
 * @param ip_address
 */
void Database::UpdateLSAccountData(unsigned int id, std::string ip_address)
{
	auto query = fmt::format(
		"UPDATE login_accounts SET last_ip_address = '{0}', last_login_date = NOW() where id = {1}",
		ip_address,
		id
	);

	QueryDatabase(query);
}

/**
 * @param id
 * @param name
 * @param password
 * @param email
 */
void Database::UpdateLSAccountInfo(
	unsigned int id,
	std::string name,
	std::string password,
	std::string email
)
{
	auto query = fmt::format(
		"REPLACE login_accounts SET id = {0}, account_name = '{1}', account_password = sha('{2}'), "
		"account_email = '{3}', last_ip_address = '0.0.0.0', last_login_date = now()",
		id,
		EscapeString(name),
		EscapeString(password),
		EscapeString(email)
	);

	QueryDatabase(query);
}

/**
 * @param id
 * @param long_name
 * @param ip_address
 */
void Database::UpdateWorldRegistration(unsigned int id, std::string long_name, std::string ip_address)
{
	auto query = fmt::format(
		"UPDATE login_world_servers SET last_login_date = NOW(), last_ip_address = '{0}', long_name = '{1}' WHERE id = {2}",
		ip_address,
		EscapeString(long_name),
		id
	);

	QueryDatabase(query);
}

/**
 * @param long_name
 * @param short_name
 * @param id
 * @return
 */
bool Database::CreateWorldRegistration(
	std::string long_name,
	std::string short_name,
	unsigned int &id
)
{
	auto query = fmt::format(
		"SELECT ifnull(max(id),0) + 1 FROM login_world_servers"
	);

	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() != 1) {
		return false;
	}

	auto row = results.begin();

	id = atoi(row[0]);

	auto insert_query = fmt::format(
		"INSERT INTO login_world_servers SET id = {0}, long_name = '{1}', short_name = '{2}', \n"
		"login_server_list_type_id = 3, login_server_admin_id = 0, is_server_trusted = 0, tag_description = ''",
		id,
		long_name,
		short_name
	);

	auto insert_results = QueryDatabase(insert_query);
	if (!insert_results.Success()) {
		LogF(
			Logs::General,
			Logs::Error,
			"World registration did not exist in the database for {0} - {1}",
			long_name,
			short_name
		);

		return false;
	}

	return true;
}

/**
 * @param long_name
 * @param short_name
 * @param id
 * @return
 */
std::string Database::CreateLoginserverApiToken(
	bool write_mode,
	bool read_mode
)
{
	std::string token = EQ::Util::UUID::Generate().ToString();
	auto        query = fmt::format(
		"INSERT INTO loginserver_api_tokens (token, can_write, can_read, created_at) VALUES ('{0}', {1}, {2}, NOW())",
		token,
		(write_mode ? "1" : "0"),
		(read_mode ? "1" : "0")
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return "";
	}

	return token;
}

/**
 * @param long_name
 * @param short_name
 * @param id
 * @return
 */
MySQLRequestResult Database::GetLoginserverApiTokens()
{
	return QueryDatabase("SELECT token, can_write, can_read FROM loginserver_api_tokens");
}

/**
 * @param log_settings
 */
void Database::LoadLogSettings(EQEmuLogSys::LogSettings *log_settings)
{
	std::string query =
					"SELECT "
					"log_category_id, "
					"log_category_description, "
					"log_to_console, "
					"log_to_file, "
					"log_to_gmsay "
					"FROM "
					"logsys_categories "
					"ORDER BY log_category_id";

	auto results         = QueryDatabase(query);
	int  log_category_id = 0;

	int *categories_in_database = new int[1000];

	for (auto row = results.begin(); row != results.end(); ++row) {
		log_category_id = atoi(row[0]);
		if (log_category_id <= Logs::None || log_category_id >= Logs::MaxCategoryID) {
			continue;
		}

		log_settings[log_category_id].log_to_console = static_cast<uint8>(atoi(row[2]));
		log_settings[log_category_id].log_to_file    = static_cast<uint8>(atoi(row[3]));
		log_settings[log_category_id].log_to_gmsay   = static_cast<uint8>(atoi(row[4]));

		/**
		 * Determine if any output method is enabled for the category
		 * and set it to 1 so it can used to check if category is enabled
		 */
		const bool log_to_console      = log_settings[log_category_id].log_to_console > 0;
		const bool log_to_file         = log_settings[log_category_id].log_to_file > 0;
		const bool log_to_gmsay        = log_settings[log_category_id].log_to_gmsay > 0;
		const bool is_category_enabled = log_to_console || log_to_file || log_to_gmsay;

		if (is_category_enabled) {
			log_settings[log_category_id].is_category_enabled = 1;
		}

		/**
		 * This determines whether or not the process needs to actually file log anything.
		 * If we go through this whole loop and nothing is set to any debug level, there is no point to create a file or keep anything open
		 */
		if (log_settings[log_category_id].log_to_file > 0) {
			LogSys.file_logs_enabled = true;
		}

		categories_in_database[log_category_id] = 1;
	}

	/**
	 * Auto inject categories that don't exist in the database...
	 */
	for (int log_index = Logs::AA; log_index != Logs::MaxCategoryID; log_index++) {
		if (categories_in_database[log_index] != 1) {

			LogInfo(
				"New Log Category [{0}] doesn't exist... Automatically adding to [logsys_categories] table...",
				Logs::LogCategoryName[log_index]
			);

			auto inject_query = fmt::format(
				"INSERT INTO logsys_categories "
				"(log_category_id, "
				"log_category_description, "
				"log_to_console, "
				"log_to_file, "
				"log_to_gmsay) "
				"VALUES "
				"({0}, '{1}', {2}, {3}, {4})",
				log_index,
				EscapeString(Logs::LogCategoryName[log_index]),
				std::to_string(log_settings[log_index].log_to_console),
				std::to_string(log_settings[log_index].log_to_file),
				std::to_string(log_settings[log_index].log_to_gmsay)
			);

			QueryDatabase(inject_query);
		}
	}

	delete[] categories_in_database;
}