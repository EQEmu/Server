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
#include "database.h"
#include "../common/event/timer.h"
#include <string>
#include <memory>

/**
 * World server class, controls the connected server processing.
 */
class WorldServer {
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

	/**
	 * @return
	 */
	unsigned int GetServerId() const { return server_id; }
	WorldServer *SetServerId(unsigned int id)
	{
		server_id = id;
		return this;
	}

	/**
	 * @return
	 */
	std::string GetServerLongName() const { return long_name; }
	std::string GetServerShortName() const { return short_name; }

	/**
	 * Gets whether the server is authorized to show up on the server list or not
	 * @return
	 */
	bool IsAuthorized() const { return is_server_authorized; }
	std::string GetLocalIP() const { return local_ip; }
	std::string GetRemoteIP() const { return remote_ip_address; }

	/**
	 * Gets what kind of server this server is (legends, preferred, normal)
	 *
	 * @return
	 */
	unsigned int GetServerListID() const { return server_list_type_id; }
	WorldServer *SetServerListTypeId(unsigned int in_server_list_id);

	int GetStatus() const { return server_status; }
	unsigned int GetZonesBooted() const { return zones_booted; }
	unsigned int GetPlayersOnline() const { return players_online; }

	/**
	 * Takes the info struct we received from world and processes it
	 *
	 * @param new_world_server_info_packet
	 */
	void Handle_NewLSInfo(ServerNewLSInfo_Struct *new_world_server_info_packet);

	/**
	 * Takes the status struct we received from world and processes it
	 *
	 * @param server_login_status
	 */
	void Handle_LSStatus(ServerLSStatus_Struct *server_login_status);

	bool HandleNewLoginserverInfoValidation(ServerNewLSInfo_Struct *new_world_server_info_packet);

	/**
	 * Informs world that there is a client incoming with the following data.
	 *
	 * @param ip
	 * @param account
	 * @param key
	 * @param account_id
	 * @param loginserver_name
	 */
	void SendClientAuth(
		std::string ip,
		std::string account,
		std::string key,
		unsigned int account_id,
		const std::string &loginserver_name
	);

	/**
	 * @param world_admin_id
	 * @param world_admin_username
	 * @param world_admin_password
	 * @param world_admin_password_hash
	 * @return
	 */
	static bool ValidateWorldServerAdminLogin(
		int world_admin_id,
		const std::string &world_admin_username,
		const std::string &world_admin_password,
		const std::string &world_admin_password_hash
	);

	WorldServer *SetZonesBooted(unsigned int in_zones_booted);
	WorldServer *SetPlayersOnline(unsigned int in_players_online);
	WorldServer *SetServerStatus(int in_server_status);
	WorldServer *SetServerProcessType(unsigned int in_server_process_type);
	WorldServer *SetLongName(const std::string &in_long_name);
	WorldServer *SetShortName(const std::string &in_short_name);
	WorldServer *SetAccountName(const std::string &in_account_name);
	WorldServer *SetAccountPassword(const std::string &in_account_password);
	WorldServer *SetRemoteIp(const std::string &in_remote_ip);
	WorldServer *SetLocalIp(const std::string &in_local_ip);
	WorldServer *SetProtocol(const std::string &in_protocol);
	WorldServer *SetVersion(const std::string &in_version);
	WorldServer *SetServerDescription(const std::string &in_server_description);
	WorldServer *SetIsServerAuthorized(bool in_is_server_authorized);
	WorldServer *SetIsServerLoggedIn(bool in_is_server_logged_in);
	WorldServer *SetIsServerTrusted(bool in_is_server_trusted);

	bool IsServerAuthorized() const;
	bool IsServerLoggedIn() const;
	bool IsServerTrusted() const;
	const std::string &GetAccountName() const;
	const std::string &GetAccountPassword() const;
	const std::string &GetLocalIp() const;
	const std::string &GetProtocol() const;
	const std::string &GetRemoteIp() const;
	const std::string &GetServerDescription() const;
	const std::string &GetVersion() const;
	int GetServerStatus() const;
	unsigned int GetServerListTypeId() const;
	unsigned int GetServerProcessType() const;

	bool HandleNewLoginserverRegisteredOnly(Database::DbWorldRegistration &world_registration);
	bool HandleNewLoginserverInfoUnregisteredAllowed(Database::DbWorldRegistration &world_registration);

private:

	/**
	 * Packet processing functions
	 *
	 * @param opcode
	 * @param packet
	 */
	void ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessUserToWorldResponseLegacy(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessUserToWorldResponse(uint16_t opcode, const EQ::Net::Packet &packet);
	void ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &packet);

	std::shared_ptr<EQ::Net::ServertalkServerConnection> connection;

	unsigned int zones_booted;
	unsigned int players_online;
	int          server_status;
	unsigned int server_id;
	unsigned int server_list_type_id;
	unsigned int server_process_type;
	std::string  server_description;
	std::string  long_name;
	std::string  short_name;
	std::string  account_name;
	std::string  account_password;
	std::string  remote_ip_address;
	std::string  local_ip;
	std::string  protocol;
	std::string  version;

	bool                       is_server_authorized;
	bool                       is_server_logged_in;
	bool                       is_server_trusted;

	/**
	 * Keepalive
	 * @param t
	 */
	void OnKeepAlive(EQ::Timer *t);
	std::unique_ptr<EQ::Timer> m_keepalive;

};

#endif

