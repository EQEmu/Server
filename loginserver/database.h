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
#ifndef EQEMU_DATABASE_H
#define EQEMU_DATABASE_H

#include <string>

#define EQEMU_MYSQL_ENABLED
//#define EQEMU_POSTGRESQL_ENABLED

/**
* Base database class, intended to be extended.
*/
class Database
{
public:
	Database() : user(""), pass(""), host(""), port(""), name("") { }
	virtual ~Database() { }

	/**
	* Returns true if the database successfully connected.
	*/
	virtual bool IsConnected() { return false; }

	/**
	* Retrieves the login data (password hash and account id) from the account name provided
	* Needed for client login procedure.
	* Returns true if the record was found, false otherwise.
	*/
	virtual bool GetLoginDataFromAccountName(std::string name, std::string &password, unsigned int &id) { return false; }

	virtual bool CreateLoginData(std::string name, std::string &password, unsigned int &id) { return false; }

	/**
	* Retrieves the world registration from the long and short names provided.
	* Needed for world login procedure.
	* Returns true if the record was found, false otherwise.
	*/
	virtual bool GetWorldRegistration(std::string long_name, std::string short_name, unsigned int &id, std::string &desc, unsigned int &list_id,
		unsigned int &trusted, std::string &list_desc, std::string &account, std::string &password) { return false; }

	/**
	* Updates the ip address of the client with account id = id
	*/
	virtual void UpdateLSAccountData(unsigned int id, std::string ip_address) { }

	/**
	* Updates or creates the login server account with info from world server
	*/
	virtual void UpdateLSAccountInfo(unsigned int id, std::string name, std::string password, std::string email) { }

	/**
	* Updates the ip address of the world with account id = id
	*/
	virtual void UpdateWorldRegistration(unsigned int id, std::string long_name, std::string ip_address) { }

	/**
	* Creates new world registration for unregistered servers and returns new id
	*/
	virtual bool CreateWorldRegistration(std::string long_name, std::string short_name, unsigned int &id) { return false; }
protected:
	std::string user, pass, host, port, name;
};

#endif

