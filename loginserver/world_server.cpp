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

#include "world_server.h"
#include "login_server.h"
#include "login_structures.h"
#include "config.h"
#include "../common/eqemu_logsys.h"
#include "../common/eqemu_logsys_fmt.h"
#include "../common/ip_util.h"

extern LoginServer server;

/**
 * @param worldserver_connection
 */
WorldServer::WorldServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> worldserver_connection)
{
	connection           = worldserver_connection;
	zones_booted         = 0;
	players_online       = 0;
	server_status        = 0;
	runtime_id           = 0;
	server_list_id       = 0;
	server_type          = 0;
	is_server_authorized = false;
	is_server_trusted    = false;
	is_server_logged_in  = false;

	worldserver_connection->OnMessage(
		ServerOP_NewLSInfo,
		std::bind(&WorldServer::ProcessNewLSInfo, this, std::placeholders::_1, std::placeholders::_2)
	);

	worldserver_connection->OnMessage(
		ServerOP_LSStatus,
		std::bind(&WorldServer::ProcessLSStatus, this, std::placeholders::_1, std::placeholders::_2)
	);

	worldserver_connection->OnMessage(
		ServerOP_UsertoWorldRespLeg,
		std::bind(
			&WorldServer::ProcessUsertoWorldRespLeg,
			this,
			std::placeholders::_1,
			std::placeholders::_2
		)
	);

	worldserver_connection->OnMessage(
		ServerOP_UsertoWorldResp,
		std::bind(&WorldServer::ProcessUserToWorldResponse, this, std::placeholders::_1, std::placeholders::_2)
	);

	worldserver_connection->OnMessage(
		ServerOP_LSAccountUpdate,
		std::bind(&WorldServer::ProcessLSAccountUpdate, this, std::placeholders::_1, std::placeholders::_2)
	);
}

WorldServer::~WorldServer() = default;

void WorldServer::Reset()
{
	runtime_id;
	zones_booted         = 0;
	players_online       = 0;
	server_status        = 0;
	server_list_id       = 0;
	server_type          = 0;
	is_server_authorized = false;
	is_server_logged_in  = false;
}

/**
 * @param opcode
 * @param packet
 */
void WorldServer::ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &packet)
{
	if (server.options.IsWorldTraceOn()) {
		Log(Logs::General,
			Logs::Netcode,
			"Application packet received from server: 0x%.4X, (size %u)",
			opcode,
			packet.Length());
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(ServerNewLSInfo_Struct)) {
		Log(Logs::General, Logs::Error,
			"Received application packet from server that had opcode ServerOP_NewLSInfo, "
			"but was too small. Discarded to avoid buffer overrun.");
		return;
	}


	ServerNewLSInfo_Struct *info = (ServerNewLSInfo_Struct *) packet.Data();

	LogF(
		Logs::General,
		Logs::Login_Server,
		"Received New Login Server Info \n"
		" - name [{0}]\n"
		" - shortname [{1}]\n"
		" - remote_address [{2}]\n"
		" - local_address [{3}]\n"
		" - account [{4}]\n"
		" - password [{5}]\n"
		" - protocolversion [{6}]\n"
		" - server_version [{7}]\n"
		" - server_type [{8}]",
		info->name,
		info->shortname,
		info->remote_address,
		info->local_address,
		info->account,
		info->password,
		info->protocolversion,
		info->serverversion,
		info->servertype
	);

	Handle_NewLSInfo(info);
}

/**
 * @param opcode
 * @param packet
 */
void WorldServer::ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &packet)
{
	Log(
		Logs::Detail,
		Logs::Netcode,
		"Application packet received from server: 0x%.4X, (size %u)",
		opcode,
		packet.Length()
	);

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(ServerLSStatus_Struct)) {
		Error(
			"Received application packet from server that had opcode ServerOP_LSStatus, but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto *ls_status = (ServerLSStatus_Struct *) packet.Data();

	LogLoginserverDetail(
		"World Server Status Update Received | Server [{0}] Status [{1}] Players [{2}] Zones [{3}]",
		this->GetLongName(),
		ls_status->status,
		ls_status->num_players,
		ls_status->num_zones
	);

	Handle_LSStatus(ls_status);
}

/**
 * @param opcode
 * @param packet
 */
void WorldServer::ProcessUsertoWorldRespLeg(uint16_t opcode, const EQ::Net::Packet &packet)
{
	if (server.options.IsWorldTraceOn()) {
		Log(Logs::General,
			Logs::Netcode,
			"Application packet received from server: 0x%.4X, (size %u)",
			opcode,
			packet.Length());
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(UsertoWorldResponseLegacy_Struct)) {
		Error(
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun"
		);
		return;
	}

	//I don't use world trace for this and here is why:
	//Because this is a part of the client login procedure it makes tracking client errors
	//While keeping world server spam with multiple servers connected almost impossible.
	if (server.options.IsTraceOn()) {
		Log(Logs::General, Logs::Netcode, "User-To-World Response received.");
	}

	auto *user_to_world_response = (UsertoWorldResponseLegacy_Struct *) packet.Data();
	Log(Logs::General, Logs::Debug, "Trying to find client with user id of %u.", user_to_world_response->lsaccountid);
	Client *c = server.client_manager->GetClient(user_to_world_response->lsaccountid, "eqemu");
	if (c) {

		Log(Logs::General,
			Logs::Debug,
			"Found client with user id of %u and account name of %s.",
			user_to_world_response->lsaccountid,
			c->GetAccountName().c_str()
		);

		auto *outapp = new EQApplicationPacket(
			OP_PlayEverquestResponse,
			sizeof(PlayEverquestResponse_Struct)
		);

		auto *per = (PlayEverquestResponse_Struct *) outapp->pBuffer;
		per->Sequence     = c->GetPlaySequence();
		per->ServerNumber = c->GetPlayServerID();

		if (user_to_world_response->response > 0) {
			per->Allowed = 1;
			SendClientAuth(
				c->GetConnection()->GetRemoteAddr(),
				c->GetAccountName(),
				c->GetKey(),
				c->GetAccountID(),
				c->GetLoginServerName()
			);
		}

		switch (user_to_world_response->response) {
			case 1:
				per->Message = 101;
				break;
			case 0:
				per->Message = 326;
				break;
			case -1:
				per->Message = 337;
				break;
			case -2:
				per->Message = 338;
				break;
			case -3:
				per->Message = 303;
				break;
		}

		LogF(Logs::General,
			 Logs::Netcode,
			 "Sending play response: allowed [{0}] sequence [{1}] server number [{2}] message [{3}]",
			 per->Allowed,
			 per->Sequence,
			 per->ServerNumber,
			 per->Message
		);

		Log(Logs::General, Logs::Netcode, "[Size: %u] %s", outapp->size, DumpPacketToString(outapp).c_str());

		if (server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		c->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		Error(
			"Received User-To-World Response for {0} but could not find the client referenced!",
			user_to_world_response->lsaccountid
		);
	}
}

/**
 * @param opcode
 * @param packet
 */
void WorldServer::ProcessUserToWorldResponse(uint16_t opcode, const EQ::Net::Packet &packet)
{
	if (server.options.IsWorldTraceOn()) {
		Log(Logs::General,
			Logs::Netcode,
			"Application packet received from server: 0x%.4X, (size %u)",
			opcode,
			packet.Length());
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(UsertoWorldResponse_Struct)) {
		Log(Logs::General,
			Logs::Error,
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun.");
		return;
	}

	//I don't use world trace for this and here is why:
	//Because this is a part of the client login procedure it makes tracking client errors
	//While keeping world server spam with multiple servers connected almost impossible.
	if (server.options.IsTraceOn()) {
		Log(Logs::General, Logs::Netcode, "User-To-World Response received.");
	}

	auto user_to_world_response = (UsertoWorldResponse_Struct *) packet.Data();
	Log(Logs::General, Logs::Debug, "Trying to find client with user id of %u.", user_to_world_response->lsaccountid);
	Client *client = server.client_manager->GetClient(
		user_to_world_response->lsaccountid,
		user_to_world_response->login
	);
	if (client) {
		Log(Logs::General,
			Logs::Debug,
			"Found client with user id of %u and account name of %s.",
			user_to_world_response->lsaccountid,
			client->GetAccountName().c_str()
		);

		auto *outapp = new EQApplicationPacket(
			OP_PlayEverquestResponse,
			sizeof(PlayEverquestResponse_Struct)
		);

		auto *per = (PlayEverquestResponse_Struct *) outapp->pBuffer;
		per->Sequence     = client->GetPlaySequence();
		per->ServerNumber = client->GetPlayServerID();
		Log(Logs::General,
			Logs::Debug,
			"Found sequence and play of %u %u",
			client->GetPlaySequence(),
			client->GetPlayServerID()
		);

		Log(Logs::General, Logs::Netcode, "[Size: %u] %s", outapp->size, DumpPacketToString(outapp).c_str());

		if (user_to_world_response->response > 0) {
			per->Allowed = 1;
			SendClientAuth(
				client->GetConnection()->GetRemoteAddr(),
				client->GetAccountName(),
				client->GetKey(),
				client->GetAccountID(),
				client->GetLoginServerName()
			);
		}

		switch (user_to_world_response->response) {
			case 1:
				per->Message = 101;
				break;
			case 0:
				per->Message = 326;
				break;
			case -1:
				per->Message = 337;
				break;
			case -2:
				per->Message = 338;
				break;
			case -3:
				per->Message = 303;
				break;
		}

		if (server.options.IsTraceOn()) {
			Log(Logs::General,
				Logs::Netcode,
				"Sending play response with following data, allowed %u, sequence %u, server number %u, message %u",
				per->Allowed,
				per->Sequence,
				per->ServerNumber,
				per->Message);
			Log(Logs::General, Logs::Netcode, "[Size: %u] %s", outapp->size, DumpPacketToString(outapp).c_str());
		}

		if (server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		client->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		Error("Received User-To-World Response for {0} but could not find the client referenced!.",
			  user_to_world_response->lsaccountid);
	}
}

/**
 * @param opcode
 * @param packet
 */
void WorldServer::ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &packet)
{
	if (server.options.IsWorldTraceOn()) {
		Log(Logs::General,
			Logs::Netcode,
			"Application packet received from server: 0x%.4X, (size %u)",
			opcode,
			packet.Length());
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(ServerLSAccountUpdate_Struct)) {
		Error(
			"Received application packet from server that had opcode ServerLSAccountUpdate_Struct, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate packet received from: %s", short_name.c_str());

	auto *loginserver_update = (ServerLSAccountUpdate_Struct *) packet.Data();
	if (is_server_trusted) {
		Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate update processed for: %s", loginserver_update->useraccount);
		std::string name     = "";
		std::string password = "";
		std::string email    = "";
		name.assign(loginserver_update->useraccount);
		password.assign(loginserver_update->userpassword);

		if (loginserver_update->useremail) {
			email.assign(loginserver_update->useremail);
		}

		server.db->UpdateLSAccountInfo(loginserver_update->useraccountid, name, password, email);
	}
}

/**
 *
 * @param new_world_server_info_packet
 */
void WorldServer::Handle_NewLSInfo(ServerNewLSInfo_Struct *new_world_server_info_packet)
{
	if (is_server_logged_in) {
		Error("WorldServer::Handle_NewLSInfo called but the login server was already marked as logged in, aborting.");
		return;
	}

	if (strlen(new_world_server_info_packet->account) <= 30) {
		account_name = new_world_server_info_packet->account;
	}
	else {
		Error("Handle_NewLSInfo error, account name was too long.");
		return;
	}

	if (strlen(new_world_server_info_packet->password) <= 30) {
		account_password = new_world_server_info_packet->password;
	}
	else {
		Error("Handle_NewLSInfo error, account password was too long.");
		return;
	}

	if (strlen(new_world_server_info_packet->name) <= 200) {
		long_name = new_world_server_info_packet->name;
	}
	else {
		Error("Handle_NewLSInfo error, long name was too long.");
		return;
	}

	if (strlen(new_world_server_info_packet->shortname) <= 50) {
		short_name = new_world_server_info_packet->shortname;
	}
	else {
		Error("Handle_NewLSInfo error, short name was too long.");
		return;
	}

	if (strlen(new_world_server_info_packet->local_address) <= 125) {
		if (strlen(new_world_server_info_packet->local_address) == 0) {
			Error("Handle_NewLSInfo error, local address was null, defaulting to localhost");
			local_ip = "127.0.0.1";
		}
		else {
			local_ip = new_world_server_info_packet->local_address;
		}
	}
	else {
		Error("Handle_NewLSInfo error, local address was too long.");

		return;
	}

	if (strlen(new_world_server_info_packet->remote_address) <= 125) {
		if (strlen(new_world_server_info_packet->remote_address) == 0) {
			remote_ip = GetConnection()->Handle()->RemoteIP();
			Error(
				"Remote address was null, defaulting to stream address %s.",
				remote_ip.c_str()
			);
		}
		else {
			remote_ip = new_world_server_info_packet->remote_address;
		}
	}
	else {
		remote_ip = GetConnection()->Handle()->RemoteIP();

		Log(
			Logs::General,
			Logs::Error,
			"Handle_NewLSInfo error, remote address was too long, defaulting to stream address %s.",
			remote_ip.c_str()
		);
	}

	if (strlen(new_world_server_info_packet->serverversion) <= 64) {
		version = new_world_server_info_packet->serverversion;
	}
	else {
		Error("Handle_NewLSInfo error, server version was too long.");
		return;
	}

	if (strlen(new_world_server_info_packet->protocolversion) <= 25) {
		protocol = new_world_server_info_packet->protocolversion;
	}
	else {
		Error("Handle_NewLSInfo error, protocol version was too long.");
		return;
	}

	server_type         = new_world_server_info_packet->servertype;
	is_server_logged_in = true;

	if (server.options.IsRejectingDuplicateServers()) {
		if (server.server_manager->ServerExists(long_name, short_name, this)) {
			Error("World tried to login but there already exists a server that has that name");
			return;
		}
	}
	else {
		if (server.server_manager->ServerExists(long_name, short_name, this)) {
			Error("World tried to login but there already exists a server that has that name");
			server.server_manager->DestroyServerByName(long_name, short_name, this);
		}
	}

	if (!server.options.IsUnregisteredAllowed()) {
		if (account_name.size() > 0 && account_password.size() > 0) {
			unsigned int s_id        = 0;
			unsigned int s_list_type = 0;
			unsigned int s_trusted   = 0;
			std::string  s_desc;
			std::string  s_list_desc;
			std::string  s_acct_name;
			std::string  s_acct_pass;
			if (server.db->GetWorldRegistration(
				long_name,
				short_name,
				s_id,
				s_desc,
				s_list_type,
				s_trusted,
				s_list_desc,
				s_acct_name,
				s_acct_pass
			)) {
				if (s_acct_name.size() == 0 || s_acct_pass.size() == 0) {
					Log(Logs::General,
						Logs::World_Server,
						"Server %s(%s) successfully logged into account that had no user/password requirement.",
						long_name.c_str(),
						short_name.c_str());
					is_server_authorized = true;
					SetRuntimeID(s_id);
					server_list_id = s_list_type;
					desc           = s_desc;
				}
				else if (s_acct_name.compare(account_name) == 0 && s_acct_pass.compare(account_password) == 0) {
					Log(Logs::General, Logs::World_Server, "Server %s(%s) successfully logged in.",
						long_name.c_str(), short_name.c_str());
					is_server_authorized = true;
					SetRuntimeID(s_id);
					server_list_id = s_list_type;
					desc           = s_desc;
					if (s_trusted) {
						Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate sent to world");
						is_server_trusted = true;

						EQ::Net::DynamicPacket outapp;
						connection->Send(ServerOP_LSAccountUpdate, outapp);
					}
				}
				else {
					Log(Logs::General,
						Logs::World_Server,
						"Server %s(%s) attempted to log in but account and password did not match the entry in the database, and only"
						" registered servers are allowed.",
						long_name.c_str(),
						short_name.c_str());
					return;
				}
			}
			else {
				Log(Logs::General,
					Logs::World_Server,
					"Server %s(%s) attempted to log in but database couldn't find an entry and only registered servers are allowed.",
					long_name.c_str(),
					short_name.c_str());
				return;
			}
		}
		else {
			Log(Logs::General,
				Logs::World_Server,
				"Server %s(%s) did not attempt to log in but only registered servers are allowed.",
				long_name.c_str(),
				short_name.c_str());
			return;
		}
	}
	else {
		unsigned int server_id         = 0;
		unsigned int server_list_type  = 0;
		unsigned int is_server_trusted = 0;
		std::string  server_description;
		std::string  server_list_description;
		std::string  server_account_name;
		std::string  server_account_password;

		if (server.db->GetWorldRegistration(
			long_name,
			short_name,
			server_id,
			server_description,
			server_list_type,
			is_server_trusted,
			server_list_description,
			server_account_name,
			server_account_password
		)) {

			if (account_name.size() > 0 && account_password.size() > 0) {
				if (server_account_name.compare(account_name) == 0 &&
					server_account_password.compare(account_password) == 0) {
					Log(Logs::General, Logs::World_Server, "Server %s(%s) successfully logged in.",
						long_name.c_str(), short_name.c_str());
					is_server_authorized = true;
					SetRuntimeID(server_id);
					server_list_id = server_list_type;
					desc           = server_description;

					if (is_server_trusted) {
						Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate sent to world");
						is_server_trusted = true;
						EQ::Net::DynamicPacket outapp;
						connection->Send(ServerOP_LSAccountUpdate, outapp);
					}
				}

					/**
					 * this is the first of two cases where we should deny access even if unregistered is allowed
					 */
				else {
					Log(Logs::General,
						Logs::World_Server,
						"Server %s(%s) attempted to log in but account and password did not match the entry in the database.",
						long_name.c_str(),
						short_name.c_str());
				}
			}
			else {

				/**
				 * this is the second of two cases where we should deny access even if unregistered is allowed
				 */
				if (server_account_name.size() > 0 || server_account_password.size() > 0) {
					Log(Logs::General,
						Logs::World_Server,
						"Server %s(%s) did not attempt to log in but this server requires a password.",
						long_name.c_str(),
						short_name.c_str());
				}
				else {
					Log(Logs::General,
						Logs::World_Server,
						"Server %s(%s) did not attempt to log in but unregistered servers are allowed.",
						long_name.c_str(),
						short_name.c_str());

					is_server_authorized = true;
					SetRuntimeID(server_id);
					server_list_id = 3;
				}
			}
		}
		else {
			LogF(Logs::General,
				 Logs::World_Server,
				 "Server [{0}] ({1}) is not registered but unregistered servers are allowed",
				 long_name,
				 short_name
			);

			if (server.db->CreateWorldRegistration(long_name, short_name, server_id)) {
				is_server_authorized = true;
				SetRuntimeID(server_id);
				server_list_id = 3;
			}
		}
	}

	server.db->UpdateWorldRegistration(GetRuntimeID(), long_name, GetConnection()->Handle()->RemoteIP());
}

/**
 * @param server_login_status
 */
void WorldServer::Handle_LSStatus(ServerLSStatus_Struct *server_login_status)
{
	players_online = server_login_status->num_players;
	zones_booted   = server_login_status->num_zones;
	server_status  = server_login_status->status;
}

/**
 * @param ip
 * @param account
 * @param key
 * @param account_id
 * @param loginserver_name
 */
void WorldServer::SendClientAuth(
	std::string ip,
	std::string account,
	std::string key,
	unsigned int account_id,
	const std::string &loginserver_name
)
{
	EQ::Net::DynamicPacket outapp;
	ClientAuth_Struct      client_auth{};

	client_auth.lsaccount_id = account_id;

	strncpy(client_auth.name, account.c_str(), 30);
	strncpy(client_auth.key, key.c_str(), 30);

	client_auth.lsadmin    = 0;
	client_auth.worldadmin = 0;
	client_auth.ip         = inet_addr(ip.c_str());
	strncpy(client_auth.lsname, &loginserver_name[0], 64);

	const std::string &client_address(ip);
	std::string       world_address(connection->Handle()->RemoteIP());

	if (client_address.compare(world_address) == 0) {
		client_auth.local = 1;
	}
	else if (IpUtil::IsIpInPrivateRfc1918(client_address)) {
		LogLoginserver("Client is authenticating from a local address [{0}]", client_address);
		client_auth.local = 1;
	}
	else {
		client_auth.local = 0;
	}

	struct in_addr ip_addr{};
	ip_addr.s_addr = client_auth.ip;

	LogLoginserver(
		"Client authentication response: world_address [{0}] client_address [{1}]",
		world_address,
		client_address
	);

	LogLoginserver(
		"Sending Client Authentication Response ls_account_id [{0}] ls_name [{1}] name [{2}] key [{3}] ls_admin [{4}] "
		" world_admin [{5}] ip [{6}] local [{7}]",
		client_auth.lsaccount_id,
		client_auth.lsname,
		client_auth.name,
		client_auth.key,
		client_auth.lsadmin,
		client_auth.worldadmin,
		inet_ntoa(ip_addr),
		client_auth.local
	);

	outapp.PutSerialize(0, client_auth);
	connection->Send(ServerOP_LSClientAuth, outapp);

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(ServerOP_LSClientAuth, outapp);
	}
}
