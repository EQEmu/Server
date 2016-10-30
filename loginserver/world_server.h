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
#ifndef EQEMU_WORLDSERVER_H
#define EQEMU_WORLDSERVER_H

#include "../common/global_define.h"
#include "../common/net/servertalk_server_connection.h"
#include "../common/servertalk.h"
#include "../common/packet_dump.h"
#include <string>
#include <memory>

/**
 * World server class, controls the connected server processing.
 */
class WorldServer
{
public:
	/**
	* Constructor, sets our connection to c.
	*/
	WorldServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> c);

	/**
	* Destructor, frees our connection if it exists.
	*/
	~WorldServer();

	/**
	* Resets the basic stats of this server.
	*/
	void Reset();

	/**
	* Accesses connection, it is intentional that this is not const (trust me).
	*/
	std::shared_ptr<EQ::Net::ServertalkServerConnection> GetConnection() { return connection; }

	/**
	* Sets the connection to c.
	*/
	void SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> c) { connection = c; }

	/**
	* Gets the runtime id of this server.
	*/
	unsigned int GetRuntimeID() const { return runtime_id; }

	/**
	* Sets the runtime id of this server.
	*/
	void SetRuntimeID(unsigned int id) { runtime_id = id; }

	/**
	* Gets the long name of the server.
	*/
	std::string GetLongName() const { return long_name; }

	/**
	* Gets the short name of the server.
	*/
	std::string GetShortName() const { return short_name; }

	/**
	* Gets whether the server is authorized to show up on the server list or not.
	*/
	bool IsAuthorized() const { return is_server_authorized; }

	/**
	* Gets the local ip of the server.
	*/
	std::string GetLocalIP() const { return local_ip; }

	/**
	* Gets the remote ip of the server.
	*/
	std::string GetRemoteIP() const { return remote_ip; }

	/**
	* Gets what kind of server this server is (legends, preferred, normal)
	*/
	unsigned int GetServerListID() const { return server_list_id; }

	/**
	* Gets the status of the server.
	*/
	int GetStatus() const { return server_status; }

	/**
	* Gets the number of zones online on the server.
	*/
	unsigned int GetZonesBooted() const { return zones_booted; }

	/**
	* Gets the number of players on the server.
	*/
	unsigned int GetPlayersOnline() const { return players_online; }

	/**
	* Takes the info struct we received from world and processes it.
	*/
	void Handle_NewLSInfo(ServerNewLSInfo_Struct* i);

	/**
	* Takes the status struct we received from world and processes it.
	*/
	void Handle_LSStatus(ServerLSStatus_Struct *s);

	/**
	* Informs world that there is a client incoming with the following data.
	*/
	void SendClientAuth(std::string ip, std::string account, std::string key, unsigned int account_id);

private:

	/**
	* Packet processing functions:
	*/
	void ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &p);
	void ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &p);
	void ProcessUsertoWorldResp(uint16_t opcode, const EQ::Net::Packet &p);
	void ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &p);

	std::shared_ptr<EQ::Net::ServertalkServerConnection> connection;
	unsigned int zones_booted;
	unsigned int players_online;
	int server_status;
	unsigned int runtime_id;
	unsigned int server_list_id;
	unsigned int server_type;
	std::string desc;
	std::string long_name;
	std::string short_name;
	std::string account_name;
	std::string account_password;
	std::string remote_ip;
	std::string local_ip;
	std::string protocol;
	std::string version;
	bool is_server_authorized;
	bool is_server_logged_in;
	bool is_server_trusted;
};

#endif

