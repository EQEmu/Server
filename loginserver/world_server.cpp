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
#include "../common/eqemu_logsys.h"
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
	server_id            = 0;
	server_list_type_id  = 0;
	server_process_type  = 0;
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
			&WorldServer::ProcessUserToWorldResponseLegacy,
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

	m_keepalive.reset(new EQ::Timer(5000, true, std::bind(&WorldServer::OnKeepAlive, this, std::placeholders::_1)));
}

WorldServer::~WorldServer() = default;

void WorldServer::Reset()
{
	server_id;
	zones_booted         = 0;
	players_online       = 0;
	server_status        = 0;
	server_list_type_id  = 0;
	server_process_type  = 0;
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
		LogDebug(
			"Application packet received from server: [{0}], (size {1})",
			opcode,
			packet.Length()
		);
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(ServerNewLSInfo_Struct)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_NewLSInfo, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}


	auto *info = (ServerNewLSInfo_Struct *) packet.Data();

	LogInfo(
		"New World Server Info | name [{0}] shortname [{1}] remote_address [{2}] local_address [{3}] account [{4}] password [{5}] server_type [{6}]",
		info->server_long_name,
		info->server_short_name,
		info->remote_ip_address,
		info->local_ip_address,
		info->account_name,
		info->account_password,
		info->server_process_type
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
		LogError(
			"Received application packet from server that had opcode ServerOP_LSStatus, but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto *ls_status = (ServerLSStatus_Struct *) packet.Data();

	if (server.options.IsWorldTraceOn()) {
		LogDebug(
			"World Server Status Update Received | Server [{0}] Status [{1}] Players [{2}] Zones [{3}]",
			GetServerLongName(),
			ls_status->status,
			ls_status->num_players,
			ls_status->num_zones
		);
	}

	Handle_LSStatus(ls_status);
}

/**
 * @param opcode
 * @param packet
 */
void WorldServer::ProcessUserToWorldResponseLegacy(uint16_t opcode, const EQ::Net::Packet &packet)
{
	if (server.options.IsWorldTraceOn()) {
		LogDebug(
			"Application packet received from server: [{0}], (size {1})",
			opcode,
			packet.Length()
		);
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(UsertoWorldResponseLegacy_Struct)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	//I don't use world trace for this and here is why:
	//Because this is a part of the client login procedure it makes tracking client errors
	//While keeping world server spam with multiple servers connected almost impossible.
	if (server.options.IsTraceOn()) {
		LogDebug("User-To-World Response received");
	}

	auto *user_to_world_response = (UsertoWorldResponseLegacy_Struct *) packet.Data();

	LogDebug("Trying to find client with user id of [{0}]", user_to_world_response->lsaccountid);
	Client *client = server.client_manager->GetClient(user_to_world_response->lsaccountid, "eqemu");
	if (client) {

		LogDebug(
			"Found client with user id of [{0}] and account name of [{1}]",
			user_to_world_response->lsaccountid,
			client->GetAccountName()
		);

		auto *outapp = new EQApplicationPacket(
			OP_PlayEverquestResponse,
			sizeof(PlayEverquestResponse_Struct)
		);

		auto *per = (PlayEverquestResponse_Struct *) outapp->pBuffer;
		per->Sequence     = client->GetPlaySequence();
		per->ServerNumber = client->GetPlayServerID();

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
			case UserToWorldStatusSuccess:
				per->Message = 101;
				break;
			case UserToWorldStatusWorldUnavail:
				per->Message = 326;
				break;
			case UserToWorldStatusSuspended:
				per->Message = 337;
				break;
			case UserToWorldStatusBanned:
				per->Message = 338;
				break;
			case UserToWorldStatusWorldAtCapacity:
				per->Message = 339;
				break;
			case UserToWorldStatusAlreadyOnline:
				per->Message = 111;
				break;
			default:
				per->Message = 102;
		}

		if (server.options.IsWorldTraceOn()) {
			LogDebug(
				"Sending play response: allowed [{0}] sequence [{1}] server number [{2}] message [{3}]",
				per->Allowed,
				per->Sequence,
				per->ServerNumber,
				per->Message
			);

			LogDebug("[Size: [{0}]] {1}", outapp->size, DumpPacketToString(outapp));
		}

		if (server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		client->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		LogError(
			"Received User-To-World Response for [{0}] but could not find the client referenced!",
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
		LogDebug(
			"Application packet received from server: 0x%.4X, (size %u)",
			opcode,
			packet.Length()
		);
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(UsertoWorldResponse_Struct)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	//I don't use world trace for this and here is why:
	//Because this is a part of the client login procedure it makes tracking client errors
	//While keeping world server spam with multiple servers connected almost impossible.
	if (server.options.IsTraceOn()) {
		LogDebug("User-To-World Response received");
	}

	auto user_to_world_response = (UsertoWorldResponse_Struct *) packet.Data();
	LogDebug("Trying to find client with user id of [{0}]", user_to_world_response->lsaccountid);

	Client *client = server.client_manager->GetClient(
		user_to_world_response->lsaccountid,
		user_to_world_response->login
	);

	if (client) {
		LogDebug("Found client with user id of [{0}] and account name of {1}",
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

		LogDebug(
			"Found sequence and play of [{0}] [{1}]",
			client->GetPlaySequence(),
			client->GetPlayServerID()
		);

		LogDebug("[Size: [{0}]] {1}", outapp->size, DumpPacketToString(outapp));

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
			case UserToWorldStatusSuccess:
				per->Message = 101;
				break;
			case UserToWorldStatusWorldUnavail:
				per->Message = 326;
				break;
			case UserToWorldStatusSuspended:
				per->Message = 337;
				break;
			case UserToWorldStatusBanned:
				per->Message = 338;
				break;
			case UserToWorldStatusWorldAtCapacity:
				per->Message = 339;
				break;
			case UserToWorldStatusAlreadyOnline:
				per->Message = 111;
				break;
			default:
				per->Message = 102;
		}

		if (server.options.IsTraceOn()) {
			LogDebug(
				"Sending play response with following data, allowed [{0}], sequence {1}, server number {2}, message {3}",
				per->Allowed,
				per->Sequence,
				per->ServerNumber,
				per->Message
			);
			LogDebug("[Size: [{0}]] {1}", outapp->size, DumpPacketToString(outapp));
		}

		if (server.options.IsDumpOutPacketsOn()) {
			DumpPacket(outapp);
		}

		client->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		LogError(
			"Received User-To-World Response for [{0}] but could not find the client referenced!.",
			user_to_world_response->lsaccountid
		);
	}
}

/**
 * @param opcode
 * @param packet
 */
void WorldServer::ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &packet)
{
	if (server.options.IsWorldTraceOn()) {
		LogDebug(
			"Application packet received from server: [{0}], (size {1})",
			opcode,
			packet.Length()
		);
	}

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(opcode, packet);
	}

	if (packet.Length() < sizeof(ServerLSAccountUpdate_Struct)) {
		LogError(
			"Received application packet from server that had opcode ServerLSAccountUpdate_Struct, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	if (server.options.IsWorldTraceOn()) {
		LogDebug("ServerOP_LSAccountUpdate packet received from [{0}]", short_name);
	}

	auto *loginserver_update = (ServerLSAccountUpdate_Struct *) packet.Data();
	if (IsServerTrusted()) {
		LogDebug("ServerOP_LSAccountUpdate update processed for: [{0}]", loginserver_update->useraccount);
		std::string name;
		std::string password;
		std::string email;

		name.assign(loginserver_update->useraccount);
		password.assign(loginserver_update->userpassword);

		if (loginserver_update->user_email) {
			email.assign(loginserver_update->user_email);
		}

		server.db->UpdateLSAccountInfo(
			loginserver_update->useraccountid,
			name,
			password,
			email
		);
	}
}

/**
 * When a worldserver first messages the loginserver telling them who they are
 *
 * @param new_world_server_info_packet
 */
void WorldServer::Handle_NewLSInfo(ServerNewLSInfo_Struct *new_world_server_info_packet)
{
	if (IsServerLoggedIn()) {
		LogError("WorldServer::Handle_NewLSInfo called but the login server was already marked as logged in, aborting");
		return;
	}

	if (!HandleNewLoginserverInfoValidation(new_world_server_info_packet)) {
		LogError("WorldServer::Handle_NewLSInfo failed validation rules");
		return;
	}

	SetAccountPassword(new_world_server_info_packet->account_password)
		->SetLongName(new_world_server_info_packet->server_long_name)
		->SetShortName(new_world_server_info_packet->server_short_name)
		->SetLocalIp(new_world_server_info_packet->local_ip_address)
		->SetRemoteIp(new_world_server_info_packet->remote_ip_address)
		->SetVersion(new_world_server_info_packet->server_version)
		->SetProtocol(new_world_server_info_packet->protocol_version)
		->SetServerProcessType(new_world_server_info_packet->server_process_type)
		->SetIsServerLoggedIn(true)
		->SetAccountName(new_world_server_info_packet->account_name);

	if (server.options.IsRejectingDuplicateServers()) {
		if (server.server_manager->ServerExists(GetServerLongName(), GetServerShortName(), this)) {
			LogError("World tried to login but there already exists a server that has that name");
			return;
		}
	}
	else {
		if (server.server_manager->ServerExists(GetServerLongName(), GetServerShortName(), this)) {
			LogInfo("World tried to login but there already exists a server that has that name, destroying [{}]",
					long_name);
			server.server_manager->DestroyServerByName(long_name, short_name, this);
		}
	}

	uint32 world_server_admin_id = 0;

	/**
	 * If our world is trying to authenticate, let's try and pull the owner first to try associating
	 * with a world short_name
	 */
	if (!GetAccountName().empty() && !GetAccountPassword().empty()) {
		Database::DbLoginServerAdmin
			login_server_admin = server.db->GetLoginServerAdmin(GetAccountName());

		if (login_server_admin.loaded) {
			LogDebug(
				"WorldServer::Handle_NewLSInfo | Attempting to authenticate world admin... [{0}] ({1}) against worldserver [{2}]",
				GetAccountName(),
				login_server_admin.id,
				GetServerShortName()
			);

			if (WorldServer::ValidateWorldServerAdminLogin(
				login_server_admin.id,
				GetAccountName(),
				GetAccountPassword(),
				login_server_admin.account_password
			)) {
				LogDebug(
					"WorldServer::Handle_NewLSInfo | Authenticating world admin... [{0}] ({1}) success! World ({2})",
					GetAccountName(),
					login_server_admin.id,
					GetServerShortName()
				);
				world_server_admin_id = login_server_admin.id;

				SetIsServerAuthorized(true);
			}
		}
	}

	Database::DbWorldRegistration
		world_registration = server.db->GetWorldRegistration(
		GetServerShortName(),
		world_server_admin_id
	);

	if (!server.options.IsUnregisteredAllowed()) {
		if (!HandleNewLoginserverRegisteredOnly(world_registration)) {
			LogError(
				"WorldServer::HandleNewLoginserverRegisteredOnly checks failed with server [{0}]",
				GetServerLongName()
			);
			return;
		}
	}
	else {
		if (!HandleNewLoginserverInfoUnregisteredAllowed(world_registration)) {
			LogError(
				"WorldServer::HandleNewLoginserverInfoUnregisteredAllowed checks failed with server [{0}]",
				GetServerLongName()
			);
			return;
		}
	}

	server.db->UpdateWorldRegistration(
		GetServerId(),
		GetServerLongName(),
		GetRemoteIp()
	);
}

/**
 * @param server_login_status
 */
void WorldServer::Handle_LSStatus(ServerLSStatus_Struct *server_login_status)
{
	SetPlayersOnline(server_login_status->num_players);
	SetZonesBooted(server_login_status->num_zones);
	SetServerStatus(server_login_status->status);
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

	client_auth.loginserver_account_id = account_id;

	strncpy(client_auth.account_name, account.c_str(), 30);
	strncpy(client_auth.key, key.c_str(), 30);

	client_auth.lsadmin        = 0;
	client_auth.is_world_admin = 0;
	client_auth.ip             = inet_addr(ip.c_str());
	strncpy(client_auth.loginserver_name, &loginserver_name[0], 64);

	const std::string &client_address(ip);
	std::string       world_address(connection->Handle()->RemoteIP());

	if (client_address == world_address) {
		client_auth.is_client_from_local_network = 1;
	}
	else if (IpUtil::IsIpInPrivateRfc1918(client_address)) {
		LogInfo("Client is authenticating from a local address [{0}]", client_address);
		client_auth.is_client_from_local_network = 1;
	}
	else {
		client_auth.is_client_from_local_network = 0;
	}

	struct in_addr ip_addr{};
	ip_addr.s_addr = client_auth.ip;

	LogInfo(
		"Client authentication response: world_address [{0}] client_address [{1}]",
		world_address,
		client_address
	);

	LogInfo(
		"Sending Client Authentication Response ls_account_id [{0}] ls_name [{1}] name [{2}] key [{3}] ls_admin [{4}] "
		"world_admin [{5}] ip [{6}] local [{7}]",
		client_auth.loginserver_account_id,
		client_auth.loginserver_name,
		client_auth.account_name,
		client_auth.key,
		client_auth.lsadmin,
		client_auth.is_world_admin,
		inet_ntoa(ip_addr),
		client_auth.is_client_from_local_network
	);

	outapp.PutSerialize(0, client_auth);
	connection->Send(ServerOP_LSClientAuth, outapp);

	if (server.options.IsDumpInPacketsOn()) {
		DumpPacket(ServerOP_LSClientAuth, outapp);
	}
}

/**
 * @param new_world_server_info_packet
 * @return
 */
bool WorldServer::HandleNewLoginserverInfoValidation(
	ServerNewLSInfo_Struct *new_world_server_info_packet
)
{
	const int max_account_name_length          = 30;
	const int max_account_password_length      = 30;
	const int max_server_long_name_length      = 200;
	const int max_server_short_name_length     = 50;
	const int max_server_local_address_length  = 125;
	const int max_server_remote_address_length = 125;
	const int max_server_version_length        = 64;
	const int max_server_protocol_version      = 25;

	if (strlen(new_world_server_info_packet->account_name) >= max_account_name_length) {
		LogError("Handle_NewLSInfo error [account_name] was too long | max [{0}]", max_account_name_length);
		return false;
	}
	else if (strlen(new_world_server_info_packet->account_password) >= max_account_password_length) {
		LogError("Handle_NewLSInfo error [account_password] was too long | max [{0}]", max_account_password_length);
		return false;
	}
	else if (strlen(new_world_server_info_packet->server_long_name) >= max_server_long_name_length) {
		LogError("Handle_NewLSInfo error [server_long_name] was too long | max [{0}]", max_server_long_name_length);
		return false;
	}
	else if (strlen(new_world_server_info_packet->server_short_name) >= max_server_short_name_length) {
		LogError("Handle_NewLSInfo error [server_short_name] was too long | max [{0}]", max_server_short_name_length);
		return false;
	}
	else if (strlen(new_world_server_info_packet->server_version) >= max_server_short_name_length) {
		LogError("Handle_NewLSInfo error [server_version] was too long | max [{0}]", max_server_version_length);
		return false;
	}
	else if (strlen(new_world_server_info_packet->protocol_version) >= max_server_protocol_version) {
		LogError("Handle_NewLSInfo error [protocol_version] was too long | max [{0}]", max_server_protocol_version);
		return false;
	}

	if (strlen(new_world_server_info_packet->local_ip_address) <= max_server_local_address_length) {
		if (strlen(new_world_server_info_packet->local_ip_address) == 0) {
			LogError("Handle_NewLSInfo error, local address was null, defaulting to localhost");
			SetLocalIp("127.0.0.1");
		}
		else {
			SetLocalIp(new_world_server_info_packet->local_ip_address);
		}
	}
	else {
		LogError("Handle_NewLSInfo error, local address was too long | max [{0}]", max_server_local_address_length);
		return false;
	}

	if (strlen(new_world_server_info_packet->remote_ip_address) <= max_server_remote_address_length) {
		if (strlen(new_world_server_info_packet->remote_ip_address) == 0) {
			SetRemoteIp(GetConnection()->Handle()->RemoteIP());

			LogWarning(
				"Remote address was null, defaulting to stream address [{0}]",
				remote_ip_address
			);
		}
		else {
			SetRemoteIp(new_world_server_info_packet->remote_ip_address);
		}
	}
	else {
		SetRemoteIp(GetConnection()->Handle()->RemoteIP());

		LogWarning(
			"Handle_NewLSInfo remote address was too long, defaulting to stream address [{0}]",
			remote_ip_address
		);
	}

	return true;
}

/**
 * @param world_registration
 * @return
 */
bool WorldServer::HandleNewLoginserverRegisteredOnly(
	Database::DbWorldRegistration &world_registration
)
{
	if (!GetAccountName().empty() && !GetAccountPassword().empty()) {
		if (world_registration.loaded) {
			bool does_world_server_not_require_authentication = (
				world_registration.server_admin_account_name.empty() ||
				world_registration.server_admin_account_password.empty()
			);

			bool does_world_server_pass_authentication_check = (
				world_registration.server_admin_account_name == GetAccountName() &&
				WorldServer::ValidateWorldServerAdminLogin(
					world_registration.server_admin_id,
					GetAccountName(),
					GetAccountPassword(),
					world_registration.server_admin_account_password
				)
			);

			SetServerDescription(world_registration.server_description)
				->SetServerId(world_registration.server_id)
				->SetIsServerTrusted(world_registration.is_server_trusted)
				->SetServerListTypeId(world_registration.server_list_type);

			if (does_world_server_not_require_authentication) {

				SetIsServerAuthorized(true);

				LogInfo(
					"Server long_name [{0}] short_name [{1}] successfully logged into account that had no user/password requirement",
					GetServerLongName(),
					GetServerShortName()
				);
			}
			else if (does_world_server_pass_authentication_check) {

				SetIsServerAuthorized(true);

				LogInfo(
					"Server long_name [{0}] short_name [{1}] successfully logged in",
					GetServerLongName(),
					GetServerShortName()
				);

				if (IsServerTrusted()) {
					LogDebug("WorldServer::HandleNewLoginserverRegisteredOnly | ServerOP_LSAccountUpdate sent to world");
					EQ::Net::DynamicPacket outapp;
					connection->Send(ServerOP_LSAccountUpdate, outapp);
				}
			}
			else {
				LogInfo(
					"Server long_name [{0}] short_name [{1}] attempted to log in but account and password did not "
					"match the entry in the database, and only registered servers are allowed",
					GetServerLongName(),
					GetServerShortName()
				);

				return false;
			}
		}
		else {
			LogInfo(
				"Server long_name [{0}] short_name [{1}] attempted to log in but database couldn't find an entry and only registered servers are allowed",
				GetServerLongName(),
				GetServerShortName()
			);

			return false;
		}
	}
	else {
		LogInfo(
			"Server long_name [{0}] short_name [{1}] did not attempt to log in but only registered servers are allowed",
			GetServerLongName(),
			GetServerShortName()
		);

		return false;
	}

	return true;
}

/**
 * @param world_registration
 * @return
 */
bool WorldServer::HandleNewLoginserverInfoUnregisteredAllowed(
	Database::DbWorldRegistration &world_registration
)
{
	if (world_registration.loaded) {
		SetServerDescription(world_registration.server_description)
			->SetServerId(world_registration.server_id)
			->SetIsServerTrusted(world_registration.is_server_trusted)
			->SetServerListTypeId(world_registration.server_list_type);

		bool does_world_server_pass_authentication_check = (
			world_registration.server_admin_account_name == GetAccountName() &&
			WorldServer::ValidateWorldServerAdminLogin(
				world_registration.server_admin_id,
				GetAccountName(),
				GetAccountPassword(),
				world_registration.server_admin_account_password
			)
		);

		bool does_world_server_have_non_empty_credentials = (
			!GetAccountName().empty() &&
			!GetAccountPassword().empty()
		);

		if (does_world_server_have_non_empty_credentials) {
			if (does_world_server_pass_authentication_check) {
				SetIsServerAuthorized(true);

				LogInfo(
					"Server long_name [{0}] short_name [{1}] successfully logged in",
					GetServerLongName(),
					GetServerShortName()
				);

				if (IsServerTrusted()) {
					LogDebug("WorldServer::HandleNewLoginserverRegisteredOnly | ServerOP_LSAccountUpdate sent to world");
					EQ::Net::DynamicPacket outapp;
					connection->Send(ServerOP_LSAccountUpdate, outapp);
				}
			}
			else {

				/**
				 * this is the first of two cases where we should deny access even if unregistered is allowed
				 */
				LogInfo(
					"Server long_name [{0}] short_name [{1}] attempted to log in but account and password did not match the entry in the database.",
					GetServerLongName(),
					GetServerShortName()
				);
			}
		}
		else {

			/**
			 * this is the second of two cases where we should deny access even if unregistered is allowed
			 */
			if (!GetAccountName().empty() || !GetAccountPassword().empty()) {
				LogInfo(
					"Server [{0}] [{1}] did not login but this server required a password to login",
					GetServerLongName(),
					GetServerShortName()
				);
			}
			else {
				SetIsServerAuthorized(true);
				LogInfo(
					"Server [{0}] [{1}] did not login but unregistered servers are allowed",
					GetServerLongName(),
					GetServerShortName()
				);
			}
		}
	}
	else {
		LogInfo(
			"Server [{0}] ({1}) is not registered but unregistered servers are allowed",
			GetServerLongName(),
			GetServerShortName()
		);

		SetIsServerAuthorized(true);

		if (world_registration.loaded) {
			return true;
		}

		Database::DbLoginServerAdmin login_server_admin = server.db->GetLoginServerAdmin(GetAccountName());

		uint32 server_admin_id = 0;
		if (login_server_admin.loaded) {
			if (WorldServer::ValidateWorldServerAdminLogin(
				login_server_admin.id,
				GetAccountName(),
				GetAccountPassword(),
				login_server_admin.account_password
			)) {
				server_admin_id = login_server_admin.id;
			}
		}

		/**
		 * Auto create a registration
		 */
		if (!server.db->CreateWorldRegistration(
			GetServerLongName(),
			GetServerShortName(),
			GetRemoteIp(),
			server_id,
			server_admin_id
		)) {
			return false;
		}
	}

	return true;
}

/**
 * @param world_admin_id
 * @param world_admin_username
 * @param world_admin_password
 * @param world_admin_password_hash
 * @return
 */
bool WorldServer::ValidateWorldServerAdminLogin(
	int world_admin_id,
	const std::string &world_admin_username,
	const std::string &world_admin_password,
	const std::string &world_admin_password_hash
)
{
	auto encryption_mode = server.options.GetEncryptionMode();
	if (eqcrypt_verify_hash(world_admin_username, world_admin_password, world_admin_password_hash, encryption_mode)) {
		return true;
	}
	else {
		if (server.options.IsUpdatingInsecurePasswords()) {
			if (encryption_mode < EncryptionModeArgon2) {
				encryption_mode = EncryptionModeArgon2;
			}

			uint32 insecure_source_encryption_mode = 0;
			if (world_admin_password_hash.length() == CryptoHash::md5_hash_length) {
				for (int i = EncryptionModeMD5; i <= EncryptionModeMD5Triple; ++i) {
					if (i != encryption_mode &&
						eqcrypt_verify_hash(world_admin_username, world_admin_password, world_admin_password_hash, i)) {
						LogDebug("[{}] Checking for [{}] world admin", __func__, GetEncryptionByModeId(i));
						insecure_source_encryption_mode = i;
					}
				}
			}
			else if (world_admin_password_hash.length() == CryptoHash::sha1_hash_length &&
					 insecure_source_encryption_mode == 0) {
				for (int i = EncryptionModeSHA; i <= EncryptionModeSHATriple; ++i) {
					if (i != encryption_mode &&
						eqcrypt_verify_hash(world_admin_username, world_admin_password, world_admin_password_hash, i)) {
						LogDebug("[{}] Checking for [{}] world admin", __func__, GetEncryptionByModeId(i));
						insecure_source_encryption_mode = i;
					}
				}
			}
			else if (world_admin_password_hash.length() == CryptoHash::sha512_hash_length &&
					 insecure_source_encryption_mode == 0) {
				for (int i = EncryptionModeSHA512; i <= EncryptionModeSHA512Triple; ++i) {
					if (i != encryption_mode &&
						eqcrypt_verify_hash(world_admin_username, world_admin_password, world_admin_password_hash, i)) {
						LogDebug("[{}] Checking for [{}] world admin", __func__, GetEncryptionByModeId(i));
						insecure_source_encryption_mode = i;
					}
				}
			}

			if (insecure_source_encryption_mode > 0) {
				LogInfo(
					"[{}] Updated insecure world_admin_username [{}] from mode [{}] ({}) to mode [{}] ({})",
					__func__,
					world_admin_username,
					GetEncryptionByModeId(insecure_source_encryption_mode),
					insecure_source_encryption_mode,
					GetEncryptionByModeId(encryption_mode),
					encryption_mode
				);

				std::string new_password_hash = eqcrypt_hash(
					world_admin_username,
					world_admin_password,
					encryption_mode
				);

				server.db->UpdateLoginWorldAdminAccountPassword(world_admin_id, new_password_hash);

				return true;
			}
		}
	}

	return false;
}

/**
 * @param in_server_list_id
 * @return
 */
WorldServer *WorldServer::SetServerListTypeId(unsigned int in_server_list_id)
{
	server_list_type_id = in_server_list_id;

	return this;
}

/**
 * @return
 */
const std::string &WorldServer::GetServerDescription() const
{
	return server_description;
}

/**
 * @param in_server_description
 */
WorldServer *WorldServer::SetServerDescription(const std::string &in_server_description)
{
	WorldServer::server_description = in_server_description;

	return this;
}

/**
 * @return
 */
bool WorldServer::IsServerAuthorized() const
{
	return is_server_authorized;
}

/**
 * @param in_is_server_authorized
 */
WorldServer *WorldServer::SetIsServerAuthorized(bool in_is_server_authorized)
{
	WorldServer::is_server_authorized = in_is_server_authorized;

	return this;
}

/**
 * @return
 */
bool WorldServer::IsServerLoggedIn() const
{
	return is_server_logged_in;
}

/**
 * @param in_is_server_logged_in
 */
WorldServer *WorldServer::SetIsServerLoggedIn(bool in_is_server_logged_in)
{
	WorldServer::is_server_logged_in = in_is_server_logged_in;

	return this;
}

/**
 * @return
 */
bool WorldServer::IsServerTrusted() const
{
	return is_server_trusted;
}

/**
 * @param in_is_server_trusted
 */
WorldServer *WorldServer::SetIsServerTrusted(bool in_is_server_trusted)
{
	WorldServer::is_server_trusted = in_is_server_trusted;

	return this;
}

/**
 * @param in_zones_booted
 */
WorldServer *WorldServer::SetZonesBooted(unsigned int in_zones_booted)
{
	WorldServer::zones_booted = in_zones_booted;

	return this;
}

/**
 * @param in_players_online
 */
WorldServer *WorldServer::SetPlayersOnline(unsigned int in_players_online)
{
	WorldServer::players_online = in_players_online;

	return this;
}

/**
 * @param in_server_status
 */
WorldServer *WorldServer::SetServerStatus(int in_server_status)
{
	WorldServer::server_status = in_server_status;

	return this;
}

/**
 * @param in_server_process_type
 */
WorldServer *WorldServer::SetServerProcessType(unsigned int in_server_process_type)
{
	WorldServer::server_process_type = in_server_process_type;

	return this;
}

/**
 * @param in_long_name
 */
WorldServer *WorldServer::SetLongName(const std::string &in_long_name)
{
	WorldServer::long_name = in_long_name;

	return this;
}

/**
 * @param in_short_name
 */
WorldServer *WorldServer::SetShortName(const std::string &in_short_name)
{
	WorldServer::short_name = in_short_name;

	return this;
}

/**
 * @param in_account_name
 */
WorldServer *WorldServer::SetAccountName(const std::string &in_account_name)
{
	WorldServer::account_name = in_account_name;

	return this;
}

/**
 * @param in_account_password
 */
WorldServer *WorldServer::SetAccountPassword(const std::string &in_account_password)
{
	WorldServer::account_password = in_account_password;

	return this;
}

/**
 * @param in_remote_ip
 */
WorldServer *WorldServer::SetRemoteIp(const std::string &in_remote_ip)
{
	WorldServer::remote_ip_address = in_remote_ip;

	return this;
}

/**
 * @param in_local_ip
 */
WorldServer *WorldServer::SetLocalIp(const std::string &in_local_ip)
{
	WorldServer::local_ip = in_local_ip;

	return this;
}

/**
 * @param in_protocol
 */
WorldServer *WorldServer::SetProtocol(const std::string &in_protocol)
{
	WorldServer::protocol = in_protocol;

	return this;
}

/**
 * @param in_version
 */
WorldServer *WorldServer::SetVersion(const std::string &in_version)
{
	WorldServer::version = in_version;

	return this;
}

/**
 * @return
 */
int WorldServer::GetServerStatus() const
{
	return server_status;
}

/**
 * @return
 */
unsigned int WorldServer::GetServerListTypeId() const
{
	return server_list_type_id;
}

/**
 * @return
 */
unsigned int WorldServer::GetServerProcessType() const
{
	return server_process_type;
}

/**
 * @return
 */
const std::string &WorldServer::GetAccountName() const
{
	return account_name;
}

/**
 * @return
 */
const std::string &WorldServer::GetAccountPassword() const
{
	return account_password;
}

/**
 * @return
 */
const std::string &WorldServer::GetRemoteIp() const
{
	return remote_ip_address;
}

/**
 * @return
 */
const std::string &WorldServer::GetLocalIp() const
{
	return local_ip;
}

/**
 * @return
 */
const std::string &WorldServer::GetProtocol() const
{
	return protocol;
}

/**
 * @return
 */
const std::string &WorldServer::GetVersion() const
{
	return version;
}

void WorldServer::OnKeepAlive(EQ::Timer *t)
{
	ServerPacket pack(ServerOP_KeepAlive, 0);
	connection->SendPacket(&pack);
}