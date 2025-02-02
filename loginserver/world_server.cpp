#include "world_server.h"
#include "login_server.h"
#include "login_types.h"
#include "../common/ip_util.h"
#include "../common/strings.h"

extern LoginServer server;

WorldServer::WorldServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> worldserver_connection)
{
	m_connection                   = worldserver_connection;
	m_zones_booted                 = 0;
	m_players_online               = 0;
	m_server_status                = 0;
	m_server_id                    = 0;
	m_server_list_type_id          = 0;
	m_server_process_type          = 0;
	m_is_server_authorized_to_list = false;
	m_is_server_trusted            = false;
	m_is_server_logged_in          = false;

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
}

WorldServer::~WorldServer() = default;

void WorldServer::Reset()
{
	m_server_id                    = 0;
	m_zones_booted                 = 0;
	m_players_online               = 0;
	m_server_status                = 0;
	m_server_list_type_id          = 0;
	m_server_process_type          = 0;
	m_is_server_authorized_to_list = false;
	m_is_server_logged_in          = false;
}

void WorldServer::ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(LoginserverNewWorldRequest)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_NewLSInfo, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto *info = (LoginserverNewWorldRequest *) packet.Data();

	// if for whatever reason the world server is not sending an address, use the local address it sends
	std::string remote_ip_addr = info->remote_ip_address;
	std::string local_ip_addr  = info->local_ip_address;
	if (remote_ip_addr.empty() && !local_ip_addr.empty() && local_ip_addr != "127.0.0.1") {
		strcpy(info->remote_ip_address, local_ip_addr.c_str());
	}

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

	HandleNewWorldserver(info);
}

void WorldServer::ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(LoginserverWorldStatusUpdate)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_LSStatus, but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto *ls_status = (LoginserverWorldStatusUpdate *) packet.Data();

	LogDebug(
		"World Server Status Update Received | Server [{0}] Status [{1}] Players [{2}] Zones [{3}]",
		m_server_long_name,
		ls_status->status,
		ls_status->num_players,
		ls_status->num_zones
	);

	HandleWorldserverStatusUpdate(ls_status);
}

void WorldServer::ProcessUserToWorldResponseLegacy(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(UsertoWorldResponseLegacy_Struct)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto *r = (UsertoWorldResponseLegacy_Struct *) packet.Data();

	LogDebug("Trying to find client with user id of [{0}]", r->lsaccountid);
	Client *client = server.client_manager->GetClient(r->lsaccountid, "eqemu");
	if (client) {
		LogDebug(
			"Found client with user id of [{0}] and account name of [{1}]",
			r->lsaccountid,
			client->GetAccountName()
		);

		auto *outapp = new EQApplicationPacket(
			OP_PlayEverquestResponse,
			sizeof(PlayEverquestResponse_Struct)
		);

		auto *per = (PlayEverquestResponse_Struct *) outapp->pBuffer;
		per->base_header.sequence = client->GetCurrentPlaySequence();
		per->server_number        = client->GetSelectedPlayServerID();

		if (r->response > 0) {
			per->base_reply.success = true;
			SendClientAuthToWorld(
				client->GetConnection()->GetRemoteAddr(),
				client->GetAccountName(),
				client->GetLoginKey(),
				client->GetAccountID(),
				client->GetLoginServerName()
			);
		}

		switch (r->response) {
			case UserToWorldStatusSuccess:
				per->base_reply.error_str_id = LS::ErrStr::ERROR_NONE;
				break;
			case UserToWorldStatusWorldUnavail:
				per->base_reply.error_str_id = LS::ErrStr::ERROR_SERVER_UNAVAILABLE;
				break;
			case UserToWorldStatusSuspended:
				per->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_SUSPENDED;
				break;
			case UserToWorldStatusBanned:
				per->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_BANNED;
				break;
			case UserToWorldStatusWorldAtCapacity:
				per->base_reply.error_str_id = LS::ErrStr::ERROR_WORLD_MAX_CAPACITY;
				break;
			case UserToWorldStatusAlreadyOnline:
				per->base_reply.error_str_id = LS::ErrStr::ERROR_ACTIVE_CHARACTER;
				break;
			default:
				per->base_reply.error_str_id = LS::ErrStr::ERROR_UNKNOWN;
				break;
		}

		LogDebug(
			"Sending play response: allowed [{0}] sequence [{1}] server number [{2}] message [{3}]",
			per->base_reply.success,
			per->base_header.sequence,
			per->server_number,
			per->base_reply.error_str_id
		);

		client->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		LogError(
			"Received User-To-World Response for [{0}] but could not find the client referenced!",
			r->lsaccountid
		);
	}
}

void WorldServer::ProcessUserToWorldResponse(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(UsertoWorldResponse_Struct)) {
		LogError(
			"Received application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	auto user_to_world_response = (UsertoWorldResponse_Struct *) packet.Data();
	LogDebug("Trying to find client with user id of [{0}]", user_to_world_response->lsaccountid);

	Client *c = server.client_manager->GetClient(
		user_to_world_response->lsaccountid,
		user_to_world_response->login
	);

	if (c) {
		LogDebug("Found client with user id of [{0}] and account name of {1}",
				 user_to_world_response->lsaccountid,
				 c->GetAccountName().c_str()
		);

		auto *outapp = new EQApplicationPacket(
			OP_PlayEverquestResponse,
			sizeof(PlayEverquestResponse_Struct)
		);

		auto *r = (PlayEverquestResponse_Struct *) outapp->pBuffer;
		r->base_header.sequence = c->GetCurrentPlaySequence();
		r->server_number        = c->GetSelectedPlayServerID();

		LogDebug(
			"Found sequence and play of [{0}] [{1}]",
			c->GetCurrentPlaySequence(),
			c->GetSelectedPlayServerID()
		);

		LogDebug("[Size: [{0}]] {1}", outapp->size, DumpPacketToString(outapp));

		if (user_to_world_response->response > 0) {
			r->base_reply.success = true;
			SendClientAuthToWorld(
				c->GetConnection()->GetRemoteAddr(),
				c->GetAccountName(),
				c->GetLoginKey(),
				c->GetAccountID(),
				c->GetLoginServerName()
			);
		}

		switch (user_to_world_response->response) {
			case UserToWorldStatusSuccess:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_NONE;
				break;
			case UserToWorldStatusWorldUnavail:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_SERVER_UNAVAILABLE;
				break;
			case UserToWorldStatusSuspended:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_SUSPENDED;
				break;
			case UserToWorldStatusBanned:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_ACCOUNT_BANNED;
				break;
			case UserToWorldStatusWorldAtCapacity:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_WORLD_MAX_CAPACITY;
				break;
			case UserToWorldStatusAlreadyOnline:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_ACTIVE_CHARACTER;
				break;
			default:
				r->base_reply.error_str_id = LS::ErrStr::ERROR_UNKNOWN;
				break;
		}

		LogDebug(
			"Sending play response with following data, allowed [{0}], sequence {1}, server number {2}, message {3}",
			r->base_reply.success,
			r->base_header.sequence,
			r->server_number,
			r->base_reply.error_str_id
		);

		c->SendPlayResponse(outapp);
		delete outapp;
	}
	else {
		LogError(
			"Received User-To-World Response for [{0}] but could not find the client referenced!.",
			user_to_world_response->lsaccountid
		);
	}
}

void WorldServer::ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &packet)
{
	LogNetcode(
		"Application packet received from server [{:#04x}] [Size: {}]\n{}",
		opcode,
		packet.Length(),
		packet.ToString()
	);

	if (packet.Length() < sizeof(ServerLSAccountUpdate_Struct)) {
		LogError(
			"Received application packet from server that had opcode ServerLSAccountUpdate_Struct, "
			"but was too small. Discarded to avoid buffer overrun"
		);

		return;
	}

	LogDebug("ServerOP_LSAccountUpdate packet received from [{0}]", m_server_short_name);

	auto *loginserver_update = (ServerLSAccountUpdate_Struct *) packet.Data();
	if (m_is_server_trusted) {
		LogDebug("ServerOP_LSAccountUpdate update processed for: [{0}]", loginserver_update->useraccount);
		std::string name;
		std::string password;
		std::string email;

		name.assign(loginserver_update->useraccount);
		password.assign(loginserver_update->userpassword);

		if (loginserver_update->user_email[0] != '\0') {
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

void WorldServer::HandleNewWorldserver(LoginserverNewWorldRequest *r)
{
	if (m_is_server_logged_in) {
		LogError(
			"WorldServer::HandleNewWorldserver called but the login server was already marked as logged in, aborting");
		return;
	}

	if (!HandleNewWorldserverValidation(r)) {
		LogError("WorldServer::HandleNewWorldserver failed validation rules");
		return;
	}

	SanitizeWorldServerName(r->server_long_name);

	m_server_long_name    = r->server_long_name;
	m_server_short_name   = r->server_short_name;
	m_account_password    = r->account_password;
	m_account_name        = r->account_name;
	m_local_ip            = r->local_ip_address;
	m_remote_ip_address   = r->remote_ip_address;
	m_server_version      = r->server_version;
	m_protocol            = r->protocol_version;
	m_server_process_type = r->server_process_type;
	m_is_server_logged_in = true;

	if (server.options.IsRejectingDuplicateServers()) {
		if (server.server_manager->DoesServerExist(m_server_long_name, m_server_short_name, this)) {
			LogError("World tried to login but there already exists a server that has that name");
			return;
		}
	}
	else {
		if (server.server_manager->DoesServerExist(m_server_long_name, m_server_short_name, this)) {
			LogInfo("World tried to login but there already exists a server that has that name, destroying [{}]",
					m_server_long_name);
			server.server_manager->DestroyServerByName(m_server_long_name, m_server_short_name, this);
		}
	}

	uint32 world_server_admin_id = 0;

	/**
	 * If our world is trying to authenticate, let's try and pull the owner first to try associating
	 * with a world short_name
	 */
	if (!m_account_name.empty() && !m_account_password.empty()) {
		Database::DbLoginServerAdmin
			login_server_admin = server.db->GetLoginServerAdmin(m_account_name);

		if (login_server_admin.loaded) {
			LogDebug(
				"Attempting to authenticate world admin... [{0}] ({1}) against worldserver [{2}]",
				m_account_name,
				login_server_admin.id,
				m_server_short_name
			);

			if (WorldServer::ValidateWorldServerAdminLogin(
				login_server_admin.id,
				m_account_name,
				m_account_password,
				login_server_admin.account_password
			)) {
				LogDebug(
					"Authenticating world admin... [{0}] ({1}) success! World ({2})",
					m_account_name,
					login_server_admin.id,
					m_server_short_name
				);
				world_server_admin_id = login_server_admin.id;

				m_is_server_authorized_to_list = true;
			}
		}
	}

	Database::DbWorldRegistration
		world_registration = server.db->GetWorldRegistration(
		m_server_short_name,
		m_server_long_name,
		world_server_admin_id
	);

	if (!server.options.IsUnregisteredAllowed()) {
		if (!HandleNewLoginserverRegisteredOnly(world_registration)) {
			LogError(
				"WorldServer::HandleNewLoginserverRegisteredOnly checks failed with server [{0}]",
				m_server_long_name
			);
			return;
		}
	}
	else {
		if (!HandleNewLoginserverInfoUnregisteredAllowed(world_registration)) {
			LogError(
				"WorldServer::HandleNewLoginserverInfoUnregisteredAllowed checks failed with server [{0}]",
				m_server_long_name
			);
			return;
		}
	}

	LogInfo(
		"World registration id [{}] for server [{}] ip_address [{}]",
		m_server_id,
		m_server_long_name,
		m_remote_ip_address
	);

	server.db->UpdateWorldRegistration(
		m_server_id,
		m_server_long_name,
		m_remote_ip_address
	);

	WorldServer::FormatWorldServerName(
		r->server_long_name,
		world_registration.server_list_type
	);

	m_server_long_name = r->server_long_name;
}

void WorldServer::HandleWorldserverStatusUpdate(LoginserverWorldStatusUpdate *u)
{
	m_players_online = u->num_players;
	m_zones_booted   = u->num_zones;
	m_server_status  = u->status;
}

void WorldServer::SendClientAuthToWorld(
	std::string ip,
	std::string account,
	std::string key,
	unsigned int account_id,
	const std::string &loginserver_name
)
{
	EQ::Net::DynamicPacket outapp;
	ClientAuth_Struct      a{};

	a.loginserver_account_id = account_id;

	strncpy(a.account_name, account.c_str(), 30);
	strncpy(a.key, key.c_str(), 30);

	a.lsadmin        = 0;
	a.is_world_admin = 0;
	a.ip             = inet_addr(ip.c_str());
	strncpy(a.loginserver_name, &loginserver_name[0], 64);

	const std::string &client_address(ip);
	std::string       world_address(m_connection->Handle()->RemoteIP());

	if (client_address == world_address) {
		a.is_client_from_local_network = 1;
	}
	else if (IpUtil::IsIpInPrivateRfc1918(client_address)) {
		LogInfo("Client is authenticating from a local address [{0}]", client_address);
		a.is_client_from_local_network = 1;
	}
	else {
		a.is_client_from_local_network = 0;
	}

	struct in_addr ip_addr{};
	ip_addr.s_addr = a.ip;

	LogInfo(
		"Client authentication response: world_address [{0}] client_address [{1}]",
		world_address,
		client_address
	);

	LogInfo(
		"Sending Client Authentication Response ls_account_id [{0}] ls_name [{1}] name [{2}] key [{3}] ls_admin [{4}] "
		"world_admin [{5}] ip [{6}] local [{7}]",
		a.loginserver_account_id,
		a.loginserver_name,
		a.account_name,
		a.key,
		a.lsadmin,
		a.is_world_admin,
		inet_ntoa(ip_addr),
		a.is_client_from_local_network
	);

	outapp.PutSerialize(0, a);
	m_connection->Send(ServerOP_LSClientAuth, outapp);

	LogNetcode(
		"Sending [{:#04x}] [Size: {}]\n{}",
		ServerOP_LSClientAuth,
		outapp.Length(),
		outapp.ToString()
	);
}

constexpr static int MAX_ACCOUNT_NAME_LENGTH          = 30;
constexpr static int MAX_ACCOUNT_PASSWORD_LENGTH      = 30;
constexpr static int MAX_SERVER_LONG_NAME_LENGTH      = 200;
constexpr static int MAX_SERVER_SHORT_NAME_LENGTH     = 50;
constexpr static int MAX_SERVER_LOCAL_ADDRESS_LENGTH  = 125;
constexpr static int MAX_SERVER_REMOTE_ADDRESS_LENGTH = 125;
constexpr static int MAX_SERVER_VERSION_LENGTH        = 64;
constexpr static int MAX_SERVER_PROTOCOL_VERSION      = 25;

bool WorldServer::HandleNewWorldserverValidation(
	LoginserverNewWorldRequest *r
)
{
	if (strlen(r->account_name) >= MAX_ACCOUNT_NAME_LENGTH) {
		LogError("HandleNewWorldserver error [account_name] was too long | max [{0}]", MAX_ACCOUNT_NAME_LENGTH);
		return false;
	}
	else if (strlen(r->account_password) >= MAX_ACCOUNT_PASSWORD_LENGTH) {
		LogError("HandleNewWorldserver error [account_password] was too long | max [{0}]", MAX_ACCOUNT_PASSWORD_LENGTH);
		return false;
	}
	else if (strlen(r->server_long_name) >= MAX_SERVER_LONG_NAME_LENGTH) {
		LogError("HandleNewWorldserver error [server_long_name] was too long | max [{0}]", MAX_SERVER_LONG_NAME_LENGTH);
		return false;
	}
	else if (strlen(r->server_short_name) >= MAX_SERVER_SHORT_NAME_LENGTH) {
		LogError("HandleNewWorldserver error [server_short_name] was too long | max [{0}]",
				 MAX_SERVER_SHORT_NAME_LENGTH);
		return false;
	}
	else if (strlen(r->server_version) >= MAX_SERVER_VERSION_LENGTH) {
		LogError("HandleNewWorldserver error [server_version] was too long | max [{0}]", MAX_SERVER_VERSION_LENGTH);
		return false;
	}
	else if (strlen(r->protocol_version) >= MAX_SERVER_PROTOCOL_VERSION) {
		LogError("HandleNewWorldserver error [protocol_version] was too long | max [{0}]", MAX_SERVER_PROTOCOL_VERSION);
		return false;
	}

	if (strlen(r->local_ip_address) <= MAX_SERVER_LOCAL_ADDRESS_LENGTH) {
		if (strlen(r->local_ip_address) == 0) {
			LogError("HandleNewWorldserver error, local address was null, defaulting to localhost");
			m_local_ip = "127.0.0.1";
		}
		else {
			m_local_ip = r->local_ip_address;
		}
	}
	else {
		LogError("HandleNewWorldserver error, local address was too long | max [{0}]", MAX_SERVER_LOCAL_ADDRESS_LENGTH);
		return false;
	}

	if (strlen(r->remote_ip_address) <= MAX_SERVER_REMOTE_ADDRESS_LENGTH) {
		if (strlen(r->remote_ip_address) == 0) {
			m_remote_ip_address = GetConnection()->Handle()->RemoteIP();

			LogWarning(
				"Remote address was null, defaulting to stream address [{0}]",
				m_remote_ip_address
			);
		}
		else {
			m_remote_ip_address = r->remote_ip_address;
		}
	}
	else {
		m_remote_ip_address = GetConnection()->Handle()->RemoteIP();

		LogWarning(
			"HandleNewWorldserver remote address was too long, defaulting to stream address [{0}]",
			m_remote_ip_address
		);
	}

	return true;
}

bool WorldServer::HandleNewLoginserverRegisteredOnly(
	Database::DbWorldRegistration &r
)
{
	if (!m_account_name.empty() && !m_account_password.empty()) {
		if (r.loaded) {
			bool does_world_server_not_require_authentication = (
				r.server_admin_account_name.empty() ||
				r.server_admin_account_password.empty()
			);

			bool does_world_server_pass_authentication_check = (
				r.server_admin_account_name == m_account_name &&
				WorldServer::ValidateWorldServerAdminLogin(
					r.server_admin_id,
					m_account_name,
					m_account_password,
					r.server_admin_account_password
				)
			);

			m_server_description  = r.server_description;
			m_server_id           = r.server_id;
			m_is_server_trusted   = r.is_server_trusted;
			m_server_list_type_id = r.server_list_type;

			if (does_world_server_not_require_authentication) {
				m_is_server_authorized_to_list = true;

				LogInfo(
					"Server long_name [{0}] short_name [{1}] successfully logged into account that had no user/password requirement",
					m_server_long_name,
					m_server_short_name
				);
			}
			else if (does_world_server_pass_authentication_check) {
				m_is_server_authorized_to_list = true;

				LogInfo(
					"Server long_name [{0}] short_name [{1}] successfully logged in",
					m_server_long_name,
					m_server_short_name
				);

				if (m_is_server_trusted) {
					LogDebug("WorldServer::HandleNewLoginserverRegisteredOnly | ServerOP_LSAccountUpdate sent to world");
					EQ::Net::DynamicPacket outapp;
					m_connection->Send(ServerOP_LSAccountUpdate, outapp);
				}
			}
			else {
				LogInfo(
					"Server long_name [{0}] short_name [{1}] attempted to log in but account and password did not "
					"match the entry in the database, and only registered servers are allowed",
					m_server_long_name,
					m_server_short_name
				);

				return false;
			}
		}
		else {
			LogInfo(
				"Server long_name [{0}] short_name [{1}] attempted to log in but database couldn't find an entry and only registered servers are allowed",
				m_server_long_name,
				m_server_short_name
			);

			return false;
		}
	}
	else {
		LogInfo(
			"Server long_name [{0}] short_name [{1}] did not attempt to log in but only registered servers are allowed",
			m_server_long_name,
			m_server_short_name
		);

		return false;
	}

	return true;
}

bool WorldServer::HandleNewLoginserverInfoUnregisteredAllowed(
	Database::DbWorldRegistration &r
)
{
	if (r.loaded) {
		m_server_description           = r.server_description;
		m_server_id                    = r.server_id;
		m_is_server_trusted            = r.is_server_trusted;
		m_server_list_type_id          = r.server_list_type;
		m_is_server_authorized_to_list = true;

		bool does_world_server_pass_authentication_check = (
			r.server_admin_account_name == m_account_name &&
			WorldServer::ValidateWorldServerAdminLogin(
				r.server_admin_id,
				m_account_name,
				m_account_password,
				r.server_admin_account_password
			)
		);

		bool does_world_server_have_non_empty_credentials = (
			!m_account_name.empty() &&
			!m_account_password.empty()
		);

		if (does_world_server_have_non_empty_credentials) {
			if (does_world_server_pass_authentication_check) {
				m_is_server_logged_in = true;

				LogInfo(
					"Server [{}] long_name [{}] short_name [{}] successfully logged in",
					r.server_id,
					m_server_long_name,
					m_server_short_name
				);

				if (m_is_server_trusted) {
					LogDebug("WorldServer::HandleNewLoginserverRegisteredOnly | ServerOP_LSAccountUpdate sent to world");
					EQ::Net::DynamicPacket outapp;
					m_connection->Send(ServerOP_LSAccountUpdate, outapp);
				}
			}
			else {
				// server is authorized to be on the loginserver list but didn't pass login check
				LogInfo(
					"Server long_name [{0}] short_name [{1}] attempted to log in but account and password did not match the entry in the database. Unregistered still allowed",
					m_server_long_name,
					m_server_short_name
				);
			}
		}
		else {

			// server is authorized to be on the loginserver list but didn't pass login check
			if (!m_account_name.empty() || !m_account_password.empty()) {
				LogInfo(
					"Server [{0}] [{1}] did not login but this server required a password to login",
					m_server_long_name,
					m_server_short_name
				);
			}
			else {
				LogInfo(
					"Server [{0}] [{1}] did not login but unregistered servers are allowed",
					m_server_long_name,
					m_server_short_name
				);
			}
		}
	}
	else {
		LogInfo(
			"Server ID [{}] [{}] ({}) is not registered but unregistered servers are allowed",
			r.server_id,
			m_server_long_name,
			m_server_short_name
		);

		m_is_server_authorized_to_list = true;

		if (r.loaded) {
			return true;
		}

		Database::DbLoginServerAdmin login_server_admin = server.db->GetLoginServerAdmin(m_account_name);

		uint32 server_admin_id = 0;
		if (login_server_admin.loaded) {
			if (WorldServer::ValidateWorldServerAdminLogin(
				login_server_admin.id,
				m_account_name,
				m_account_password,
				login_server_admin.account_password
			)) {
				server_admin_id = login_server_admin.id;
			}
		}

		// Auto create a registration
		if (!server.db->CreateWorldRegistration(
			m_server_long_name,
			m_server_short_name,
			m_remote_ip_address,
			m_server_id,
			server_admin_id
		)) {
			return false;
		}
	}

	return true;
}

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

void WorldServer::SerializeForClientServerList(SerializeBuffer &out, bool use_local_ip, LSClientVersion version) const
{
	// see LoginClientServerData_Struct
	if (use_local_ip) {
		out.WriteString(GetLocalIP());
	}
	else {
		out.WriteString(m_remote_ip_address);
	}

	if (version == cv_larion) {
		out.WriteUInt32(9000);
	}

	switch (GetServerListID()) {
		case LS::ServerType::Legends:
			out.WriteInt32(LS::ServerTypeFlags::Legends);
			break;
		case LS::ServerType::Preferred:
			out.WriteInt32(LS::ServerTypeFlags::Preferred);
			break;
		default:
			out.WriteInt32(LS::ServerTypeFlags::Standard);
			break;
	}
	if (version == cv_larion) {
		auto server_id = m_server_id;
		//if this is 0, the client will not show the server in the list
		out.WriteUInt32(1);
		out.WriteUInt32(server_id);
	}
	else {
		out.WriteUInt32(m_server_id);
	}

	out.WriteString(m_server_long_name);
	out.WriteString("us"); // country code
	out.WriteString("en"); // language code

	// 0 = Up, 1 = Down, 2 = Up, 3 = down, 4 = locked, 5 = locked(down)
	if (GetStatus() < 0) {
		if (GetZonesBooted() == 0) {
			out.WriteInt32(LS::ServerStatusFlags::Down);
		}
		else {
			out.WriteInt32(LS::ServerStatusFlags::Locked);
		}
	}
	else {
		out.WriteInt32(LS::ServerStatusFlags::Up);
	}

	out.WriteUInt32(GetPlayersOnline());
}

void WorldServer::FormatWorldServerName(char *name, int8 server_list_type)
{
	std::string server_long_name = name;
	server_long_name = Strings::Trim(server_long_name);

	bool name_set_to_bottom = false;
	if (server_list_type == LS::ServerType::Standard) {
		if (server.options.IsWorldDevTestServersListBottom()) {
			std::string s = Strings::ToLower(server_long_name);
			if (s.find("dev") != std::string::npos) {
				server_long_name   = fmt::format("|D| {}", server_long_name);
				name_set_to_bottom = true;
			}
			else if (s.find("test") != std::string::npos) {
				server_long_name   = fmt::format("|T| {}", server_long_name);
				name_set_to_bottom = true;
			}
			else if (s.find("installer") != std::string::npos) {
				server_long_name   = fmt::format("|I| {}", server_long_name);
				name_set_to_bottom = true;
			}
		}
		if (server.options.IsWorldSpecialCharacterStartListBottom() && !name_set_to_bottom) {
			auto first_char = server_long_name.c_str()[0];
			if (IsAllowedWorldServerCharacterList(first_char) && first_char != '|') {
				server_long_name   = fmt::format("|*| {}", server_long_name);
				name_set_to_bottom = true;
			}
		}
	}

	strn0cpy(name, server_long_name.c_str(), 201);
}
