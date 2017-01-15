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
#ifndef EQEMU_SERVERMANAGER_H
#define EQEMU_SERVERMANAGER_H

#include "../common/global_define.h"
#include "../common/servertalk.h"
#include "../common/packet_dump.h"
#include "../common/net/servertalk_server.h"
#include "world_server.h"
#include "client.h"
#include <list>

/**
* Server manager class, deals with management of the world servers.
*/
class ServerManager
{
public:
	/**
	* Constructor, sets up the TCP server and starts listening.
	*/
	ServerManager();

	/**
	* Destructor, shuts down the TCP server.
	*/
	~ServerManager();

	/**
	* Sends a request to world to see if the client is banned or suspended.
	*/
	void SendUserToWorldRequest(unsigned int server_id, unsigned int client_account_id);

	/**
	* Creates a server list packet for the client.
	*/
	EQApplicationPacket *CreateServerListPacket(Client *c, uint32 seq);

	/**
	* Checks to see if there is a server exists with this name, ignoring option.
	*/
	bool ServerExists(std::string l_name, std::string s_name, WorldServer *ignore = nullptr);

	/**
	* Destroys a server with this name, ignoring option.
	*/
	void DestroyServerByName(std::string l_name, std::string s_name, WorldServer *ignore = nullptr);

private:
	/**
	* Retrieves a server(if exists) by ip address
	* Useful utility for the reconnect process.
	*/
	WorldServer* GetServerByAddress(const std::string &address, int port);

	std::unique_ptr<EQ::Net::ServertalkServer> server_connection;
	std::list<std::unique_ptr<WorldServer>> world_servers;
};

#endif

