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
	WorldServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> worldserver_connection);

	/**
	 * Destructor, frees our connection if it exists
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
	void SetConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> c) { connection = c; }
	unsigned int GetRuntimeID() const { return runtime_id; }
	void SetRuntimeID(unsigned int id) { runtime_id = id; }
	std::string GetLongName() const { return long_name; }
	std::string GetShortName() const { return short_name; }

	/**
	 * Gets whether the server is authorized to show up on the server list or not
	 * @return
	 */
	bool IsAuthorized() const { return is_server_authorized; }
	std::string GetLocalIP() const { return local_ip; }
	std::string GetRemoteIP() const { return remote_ip; }

	/**
	 * Gets what kind of server this server is (legends, preferred, normal)
	 *
	 * @return
	 */
	unsigned int GetServerListID() const { return server_list_id; }
	int GetStatus() const { return server_status; }
	unsigned int GetZonesBooted() const { return zones_booted; }
	unsigned int GetPlayersOnline() const { return players_online; }

	/**
	 * Takes the info struct we received from world and processes it
	 *
	 * @param new_worldserver_info_packet
	 */
	void Handle_NewLSInfo(ServerNewLSInfo_Struct* new_worldserver_info_packet);

	/**
	 * Takes the status struct we received from world and processes it
	 *
	 * @param server_login_status
	 */
	void Handle_LSStatus(ServerLSStatus_Struct *server_login_status);

	/**
	 * Informs world that there is a client incoming with the following data.
	 *
	 * @param ip
	 * @param account
	 * @param key
	 * @param account_id
	 * @param loginserver_name
	 */
	void SendClientAuth(std::string ip, std::string account, std::string key, unsigned int account_id, const std::string &loginserver_name);

private:

	/**
	 * Packet processing functions
	 *
	 * @param opcode
	 * @param packet
	 */
	void ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessUsertoWorldRespLeg(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessUserToWorldResponse(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &packet);

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

