/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2010 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef EQEMU_DATABASEMYSQL_H
#define EQEMU_DATABASEMYSQL_H

#include "database.h"
#ifdef EQEMU_MYSQL_ENABLED

#include <string>
#include <sstream>
#include <stdlib.h>
#include <mysql.h>

/**
* Mysql Database class
*/
class DatabaseMySQL : public Database
{
public:
	/**
	* Constructor, sets our database to null.
	*/
	DatabaseMySQL() { database = nullptr; }

	/**
	* Constructor, tries to set our database to connect to the supplied options.
	*/
	DatabaseMySQL(std::string user, std::string pass, std::string host, std::string port, std::string name);

	/**
	* Destructor, frees our database if needed.
	*/
	virtual ~DatabaseMySQL();

	/**
	* @return Returns true if the database successfully connected.
	*/
	virtual bool IsConnected() { return (database != nullptr); }

	/**
	* Retrieves the login data (password hash and account id) from the account name provided
	* Needed for client login procedure.
	* Returns true if the record was found, false otherwise.
	*/
	virtual bool GetLoginDataFromAccountName(std::string name, std::string &password, unsigned int &id);

	virtual bool CreateLoginData(std::string name, std::string &password, unsigned int &id);

	/**
	* Retrieves the world registration from the long and short names provided.
	* Needed for world login procedure.
	* Returns true if the record was found, false otherwise.
	*/
	virtual bool GetWorldRegistration(std::string long_name, std::string short_name, unsigned int &id, std::string &desc, unsigned int &list_id,
		unsigned int &trusted, std::string &list_desc, std::string &account, std::string &password);

	/**
	* Updates the ip address of the client with account id = id
	*/
	virtual void UpdateLSAccountData(unsigned int id, std::string ip_address);

	/**
	* Updates or creates the login server account with info from world server
	*/
	virtual void UpdateLSAccountInfo(unsigned int id, std::string name, std::string password, std::string email);

	/**
	* Updates the ip address of the world with account id = id
	*/
	virtual void UpdateWorldRegistration(unsigned int id, std::string long_name, std::string ip_address);

	/**
	* Creates new world registration for unregistered servers and returns new id
	*/
	virtual bool CreateWorldRegistration(std::string long_name, std::string short_name, unsigned int &id);
protected:
	std::string user, pass, host, port, name;
	MYSQL *database;
};

#endif
#endif

