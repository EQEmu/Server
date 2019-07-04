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
		const std::string &name,
		const std::string &password,
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

	/**
	 * Retrieves the world registration from the long and short names provided
	 * Needed for world login procedure
	 * Returns true if the record was found, false otherwise.
	 *
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
	bool GetWorldRegistration(
		std::string long_name,
		std::string short_name,
		unsigned int &id,
		std::string &desc,
		unsigned int &list_id,
		unsigned int &trusted,
		std::string &list_desc,
		std::string &account,
		std::string &password
	);

	void UpdateLSAccountData(unsigned int id, std::string ip_address);
	void UpdateLSAccountInfo(unsigned int id, std::string name, std::string password, std::string email);
	void UpdateWorldRegistration(unsigned int id, std::string long_name, std::string ip_address);
	bool CreateWorldRegistration(std::string long_name, std::string short_name, unsigned int &id);
protected:
	std::string user, pass, host, port, name;
	MYSQL       *database;
};

#endif

