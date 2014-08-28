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
#ifndef EQEMU_DATABASEPOSTGRESQL_H
#define EQEMU_DATABASEPOSTGRESQL_H

#include "database.h"
#ifdef EQEMU_POSTGRESQL_ENABLED

#include <string>
#include <sstream>
#include <stdlib.h>
#include <libpq-fe.h>

using namespace std;

/**
 * PostgreSQL Database class
 */
class DatabasePostgreSQL : public Database
{
public:
	/**
	* Constructor, sets our database to null.
	*/
	DatabasePostgreSQL() { db = nullptr; }

	/**
	* Constructor, tries to set our database to connect to the supplied options.
	*/
	DatabasePostgreSQL(string user, string pass, string host, string port, string name);

	/**
	* Destructor, frees our database if needed.
	*/
	virtual ~DatabasePostgreSQL();

	/**
	* Returns true if the database successfully connected.
	*/
	virtual bool IsConnected() { return (db != nullptr); }

	/**
	* Retrieves the login data (password hash and account id) from the account name provided
	* Needed for client login procedure.
	* Returns true if the record was found, false otherwise.
	*/
	virtual bool GetLoginDataFromAccountName(string name, string &password, unsigned int &id);

	/**
	* Retrieves the world registration from the long and short names provided.
	* Needed for world login procedure.
	* Returns true if the record was found, false otherwise.
	*/
	virtual bool GetWorldRegistration(string long_name, string short_name, unsigned int &id, string &desc, unsigned int &list_id,
		unsigned int &trusted, string &list_desc, string &account, string &password);

	/**
	* Updates the ip address of the client with account id = id
	*/
	virtual void UpdateLSAccountData(unsigned int id, string ip_address);

	/**
	* Updates the ip address of the world with account id = id
	*/
	virtual void UpdateWorldRegistration(unsigned int id, string long_name, string ip_address);

	/**
	* Creates new world registration for unregistered servers and returns new id
	*/
	virtual bool CreateWorldRegistration(string long_name, string short_name, unsigned int &id);
protected:
	string user, pass, host, port, name;
	PGconn *db;
};

#endif
#endif

