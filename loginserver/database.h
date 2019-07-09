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

#ifndef EQEMU_DATABASEMYSQL_H
#define EQEMU_DATABASEMYSQL_H

#include "../common/dbcore.h"
#include "../common/eqemu_logsys.h"

#include <string>
#include <sstream>
#include <stdlib.h>
#include <mysql.h>

class Database : public DBcore {
public:

	Database() { database = nullptr; }

	/**
	 * Constructor, tries to set our database to connect to the supplied options.
	 *
	 * @param user
	 * @param pass
	 * @param host
	 * @param port
	 * @param name
	 */
	Database(std::string user, std::string pass, std::string host, std::string port, std::string name);

	/**
	 * Destructor, frees our database if needed.
	 */
	~Database();
	bool IsConnected() { return (database != nullptr); }

	/**
	 * Retrieves the login data (password hash and account id) from the account name provided needed for client login procedure.
	 * @param name
	 * @param loginserver
	 * @param password
	 * @param id
	 * @return
	 */
	bool GetLoginDataFromAccountInfo(
		const std::string &name,
		const std::string &loginserver,
		std::string &password,
		unsigned int &id
	);
	bool GetLoginTokenDataFromToken(
		const std::string &token,
		const std::string &ip,
		unsigned int &db_account_id,
		std::string &db_loginserver,
		std::string &user
	);
	unsigned int GetFreeID(const std::string &loginserver);
	bool CreateLoginData(
		const std::string &name,
		const std::string &password,
		const std::string &loginserver,
		unsigned int &id
	);
	bool CreateLoginDataWithID(
		const std::string &in_account_name,
		const std::string &in_account_password,
		const std::string &loginserver,
		unsigned int id
	);

	void UpdateLoginHash(const std::string &name, const std::string &loginserver, const std::string &hash);

	bool DoesLoginServerAccountExist(
		const std::string &name,
		const std::string &password,
		const std::string &loginserver,
		unsigned int id
	);

	struct DbWorldRegistration {
		bool        loaded            = false;
		int32       server_id         = 0;
		int8        server_list_type  = 3;
		bool        is_server_trusted = false;
		std::string server_description;
		std::string server_list_description;
		std::string server_admin_account_name;
		std::string server_admin_account_password;
	};

	/**
	 * Retrieves the world registration from the long and short names provided
	 * Needed for world login procedure
	 * Returns true if the record was found, false otherwise
	 *
	 * @param short_name
	 * @param remote_ip
	 * @param local_ip
	 * @return
	 */
	Database::DbWorldRegistration GetWorldRegistration(
		const std::string& short_name,
		const std::string& remote_ip,
		const std::string& local_ip
	);

	void UpdateLSAccountData(unsigned int id, std::string ip_address);
	void UpdateLSAccountInfo(unsigned int id, std::string name, std::string password, std::string email);
	void UpdateWorldRegistration(unsigned int id, std::string long_name, std::string ip_address);
	bool CreateWorldRegistration(std::string long_name, std::string short_name, unsigned int &id);
	void LoadLogSettings(EQEmuLogSys::LogSettings *log_settings);

	/**
	 * @param write_mode
	 * @param read_mode
	 * @return
	 */
	std::string CreateLoginserverApiToken(bool write_mode, bool read_mode);
	MySQLRequestResult GetLoginserverApiTokens();

protected:
	std::string user, pass, host, port, name;
	MYSQL       *database;
};

#endif

