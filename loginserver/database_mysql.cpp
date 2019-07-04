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

#include "database_mysql.h"
#include "login_server.h"
#include "../common/eqemu_logsys.h"
#include "../common/eqemu_logsys_fmt.h"
#include "../common/string_util.h"

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

	database = mysql_init(nullptr);
	if (database) {
		char r = 1;
		mysql_options(database, MYSQL_OPT_RECONNECT, &r);
		if (!mysql_real_connect(
			database,
			host.c_str(),
			user.c_str(),
			pass.c_str(),
			name.c_str(),
			atoi(port.c_str()),
			nullptr,
			0
		)) {
			mysql_close(database);
			Log(Logs::General, Logs::Error, "Failed to connect to MySQL database. Error: %s", mysql_error(database));
			exit(1);
		}

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
	else {
		Log(Logs::General, Logs::Error, "Failed to create db object in MySQL database.");
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
		"SELECT LoginServerID, AccountPassword FROM {0} WHERE AccountName = '{1}' AND AccountLoginserver = '{2}' LIMIT 1",
		server.options.GetAccountTable(),
		EscapeString(name),
		EscapeString(loginserver)
	);

	auto results = QueryDatabase(query);

	if (results.RowCount() != 1) {
		LogF(
			Logs::Detail,
			Logs::Login_Server,
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

	LogF(
		Logs::Detail,
		Logs::Login_Server,
		"Found account for name [{0}] login [{1}]",
		name,
		loginserver
	);

	return false;
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
	if (!database) {
		return false;
	}

	MYSQL_RES *res;
	MYSQL_ROW row;
	std::stringstream query(std::stringstream::in | std::stringstream::out);
	query
		<< "SELECT tbllogintokens.Id, tbllogintokens.IpAddress, tbllogintokenclaims.Name, tbllogintokenclaims.Value FROM tbllogintokens ";
	query
		<< "JOIN tbllogintokenclaims ON tbllogintokens.Id = tbllogintokenclaims.TokenId WHERE tbllogintokens.Expires > NOW() AND tbllogintokens.Id='";
	query << EscapeString(token) << "' AND tbllogintokens.IpAddress='" << EscapeString(ip) << "'";

	if (mysql_query(database, query.str().c_str()) != 0) {
		Log(Logs::General, Logs::Error, "Mysql query failed: %s", query.str().c_str());
		return false;
	}

	res = mysql_use_result(database);

	bool found_username          = false;
	bool found_login_id          = false;
	bool found_login_server_name = false;
	if (res) {
		while ((row = mysql_fetch_row(res)) != nullptr) {
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

		mysql_free_result(res);
	}

	return found_username && found_login_id && found_login_server_name;
}

/**
 * @param loginserver
 * @return
 */
unsigned int Database::GetFreeID(const std::string &loginserver)
{
	if (!database) {
		return false;
	}

	MYSQL_RES *res;
	MYSQL_ROW row;
	std::stringstream query(std::stringstream::in | std::stringstream::out);
	query << "SELECT MAX(LoginServerID) + 1 FROM " << server.options.GetAccountTable() << " WHERE AccountLoginServer='";
	query << EscapeString(loginserver) << "'";

	if (mysql_query(database, query.str().c_str()) != 0) {
		Log(Logs::General, Logs::Error, "Mysql query failed: %s", query.str().c_str());
		return 0;
	}

	res = mysql_use_result(database);

	if (res) {
		while ((row = mysql_fetch_row(res)) != nullptr) {
			if (row[0] == nullptr) {
				mysql_free_result(res);
				return 1;
			}

			auto ret = atol(row[0]);
			mysql_free_result(res);
			return ret;
		}

		mysql_free_result(res);
	}

	return 1;
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
	return CreateLoginDataWithID(name, password, loginserver, GetFreeID(loginserver));
}

/**
 * @param name
 * @param password
 * @param loginserver
 * @param id
 * @return
 */
bool Database::CreateLoginDataWithID(
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
		"INSERT INTO {0} (LoginServerID, AccountLoginserver, AccountName, AccountPassword, AccountEmail, LastLoginDate, LastIPAddress) "
		"VALUES ({1}, '{2}', '{3}', '{4}', 'local_creation', NOW(), '127.0.0.1')",
		server.options.GetAccountTable(),
		id,
		EscapeString(loginserver),
		EscapeString(name),
		EscapeString(password)
	);

	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	return true;
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
		"SELECT AccountName FROM {0} WHERE AccountName = '{1}' AND AccountLoginserver = '{2}'",
		server.options.GetAccountTable(),
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
	LogF(
		Logs::Detail,
		Logs::Login_Server,
		"name [{0}] loginserver [{1}] hash [{2}]",
		name,
		loginserver,
		hash
	);

	auto query = fmt::format(
		"UPDATE {0} SET AccountPassword='{1}' WHERE AccountName='{2}' AND AccountLoginserver='{3}'",
		server.options.GetAccountTable(),
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

	if (!database) {
		return false;
	}

	MYSQL_RES     *res;
	MYSQL_ROW     row;
	char          escaped_short_name[101];
	unsigned long length;
	length = mysql_real_escape_string(
		database,
		escaped_short_name,
		short_name.substr(0, 100).c_str(),
		short_name.substr(0, 100).length());
	escaped_short_name[length + 1] = 0;
	std::stringstream query(std::stringstream::in | std::stringstream::out);
	query
		<< "SELECT ifnull(WSR.ServerID,999999) AS ServerID, WSR.ServerTagDescription, ifnull(WSR.ServerTrusted,0) AS ServerTrusted, ifnull(SLT.ServerListTypeID,3) AS ServerListTypeID, ";
	query << "SLT.ServerListTypeDescription, ifnull(WSR.ServerAdminID,0) AS ServerAdminID FROM "
		  << server.options.GetWorldRegistrationTable();
	query << " AS WSR JOIN " << server.options.GetWorldServerTypeTable()
		  << " AS SLT ON WSR.ServerListTypeID = SLT.ServerListTypeID";
	query << " WHERE WSR.ServerShortName = '";
	query << escaped_short_name;
	query << "'";

	if (mysql_query(database, query.str().c_str()) != 0) {
		Log(Logs::General, Logs::Error, "Mysql query failed: %s", query.str().c_str());
		return false;
	}

	res = mysql_use_result(database);
	if (res) {
		if ((row = mysql_fetch_row(res)) != nullptr) {
			id        = atoi(row[0]);
			desc      = row[1];
			trusted   = atoi(row[2]);
			list_id   = atoi(row[3]);
			list_desc = row[4];
			int db_account_id = atoi(row[5]);
			mysql_free_result(res);

			if (db_account_id > 0) {
				std::stringstream query(std::stringstream::in | std::stringstream::out);
				query << "SELECT AccountName, AccountPassword FROM " << server.options.GetWorldAdminRegistrationTable();
				query << " WHERE ServerAdminID = " << db_account_id;

				if (mysql_query(database, query.str().c_str()) != 0) {
					Log(Logs::General, Logs::Error, "Mysql query failed: %s", query.str().c_str());
					return false;
				}

				res = mysql_use_result(database);
				if (res) {
					if ((row = mysql_fetch_row(res)) != nullptr) {
						account  = row[0];
						password = row[1];
						mysql_free_result(res);
						return true;
					}
				}

				Log(Logs::General, Logs::Error, "Mysql query returned no result: %s", query.str().c_str());
				return false;
			}
			return true;
		}
	}

	Log(Logs::General, Logs::Error, "Mysql query returned no result: %s", query.str().c_str());
	return false;
}

/**
 * @param id
 * @param ip_address
 */
void Database::UpdateLSAccountData(unsigned int id, std::string ip_address)
{
	auto query = fmt::format(
		"UPDATE {0} SET LastIPAddress = '{2}', LastLoginDate = now() where LoginServerId = {3}",
		server.options.GetAccountTable(),
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
		"REPLACE {0} SET LoginServerID = {1}, AccountName = '{2}', AccountPassword = sha('{3}'), AccountCreateDate = now(), "
		"AccountEmail = '{4}', LastIPAddress = '0.0.0.0', LastLoginDate = now()",
		server.options.GetAccountTable(),
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
		"UPDATE {0} SET ServerLastLoginDate = NOW(), ServerLastIPAddr = '{1}', ServerLongName = '{2}' WHERE ServerID = {3}",
		server.options.GetWorldRegistrationTable(),
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
bool Database::CreateWorldRegistration(std::string long_name, std::string short_name, unsigned int &id)
{
	if (!database) {
		return false;
	}

	MYSQL_RES     *res;
	MYSQL_ROW     row;
	char          escaped_long_name[201];
	char          escaped_short_name[101];
	unsigned long length;
	length = mysql_real_escape_string(
		database,
		escaped_long_name,
		long_name.substr(0, 100).c_str(),
		long_name.substr(0, 100).length());
	escaped_long_name[length + 1] = 0;
	length = mysql_real_escape_string(
		database,
		escaped_short_name,
		short_name.substr(0, 100).c_str(),
		short_name.substr(0, 100).length());
	escaped_short_name[length + 1] = 0;
	std::stringstream query(std::stringstream::in | std::stringstream::out);
	query << "SELECT ifnull(max(ServerID),0) FROM " << server.options.GetWorldRegistrationTable();

	if (mysql_query(database, query.str().c_str()) != 0) {
		Log(Logs::General, Logs::Error, "Mysql query failed: %s", query.str().c_str());
		return false;
	}

	res = mysql_use_result(database);
	if (res) {
		if ((row = mysql_fetch_row(res)) != nullptr) {
			id = atoi(row[0]) + 1;
			mysql_free_result(res);

			std::stringstream query(std::stringstream::in | std::stringstream::out);
			query << "INSERT INTO " << server.options.GetWorldRegistrationTable() << " SET ServerID = " << id;
			query << ", ServerLongName = '" << escaped_long_name << "', ServerShortName = '" << escaped_short_name;
			query << "', ServerListTypeID = 3, ServerAdminID = 0, ServerTrusted = 0, ServerTagDescription = ''";

			if (mysql_query(database, query.str().c_str()) != 0) {
				Log(Logs::General, Logs::Error, "Mysql query failed: %s", query.str().c_str());
				return false;
			}
			return true;
		}
	}
	Log(Logs::General,
		Logs::Error,
		"World registration did not exist in the database for %s %s",
		long_name.c_str(),
		short_name.c_str());
	return false;
}
