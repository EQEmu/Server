#include "server_manager.h"
#include "login_server.h"
#include "login_types.h"
#include <stdlib.h>

#include "../common/eqemu_logsys.h"
#include "../common/ip_util.h"

extern LoginServer server;
extern bool        run_server;

ServerManager::ServerManager()
{
	int listen_port = server.config.GetVariableInt("general", "listen_port", 5998);

	m_server_connection = std::make_unique<EQ::Net::ServertalkServer>();
	EQ::Net::ServertalkServerOptions opts;
	opts.port = listen_port;
	opts.ipv6           = false;
	m_server_connection->Listen(opts);

	LogInfo("Loginserver now listening on port [{0}]", listen_port);

	m_server_connection->OnConnectionIdentified(
		"World", [this](std::shared_ptr<EQ::Net::ServertalkServerConnection> world_connection) {
			LogInfo(
				"New World Server connection from {0}:{1}",
				world_connection->Handle()->RemoteIP(),
				world_connection->Handle()->RemotePort()
			);

			auto iter = m_world_servers.begin();
			while (iter != m_world_servers.end()) {
				if ((*iter)->GetConnection()->Handle()->RemoteIP().compare(world_connection->Handle()->RemoteIP()) ==
					0 &&
					(*iter)->GetConnection()->Handle()->RemotePort() == world_connection->Handle()->RemotePort()) {

					LogInfo(
						"World server already existed for {0}:{1}, removing existing connection.",
						world_connection->Handle()->RemoteIP(),
						world_connection->Handle()->RemotePort()
					);

					m_world_servers.erase(iter);
					break;
				}

				++iter;
			}

			m_world_servers.push_back(std::make_unique<WorldServer>(world_connection));
		}
	);

	m_server_connection->OnConnectionRemoved(
		"World", [this](std::shared_ptr<EQ::Net::ServertalkServerConnection> c) {
			auto iter = m_world_servers.begin();
			while (iter != m_world_servers.end()) {
				if ((*iter)->GetConnection()->GetUUID() == c->GetUUID()) {
					LogInfo(
						"World server {0} has been disconnected, removing.",
						(*iter)->GetServerLongName()
					);
					m_world_servers.erase(iter);
					return;
				}

				++iter;
			}
		}
	);
}

ServerManager::~ServerManager() = default;

/**
 * @param ip_address
 * @param port
 * @return
 */
WorldServer *ServerManager::GetServerByAddress(const std::string &ip_address, int port)
{
	auto iter = m_world_servers.begin();
	while (iter != m_world_servers.end()) {
		if ((*iter)->GetConnection()->Handle()->RemoteIP() == ip_address &&
			(*iter)->GetConnection()->Handle()->RemotePort()) {
			return (*iter).get();
		}
		++iter;
	}

	return nullptr;
}

/**
 * @param client
 * @param sequence
 * @return
 */
std::unique_ptr<EQApplicationPacket> ServerManager::CreateServerListPacket(Client *client, uint32 sequence)
{
	unsigned int server_count = 0;
	in_addr      in{};
	in.s_addr = client->GetConnection()->GetRemoteIP();
	std::string client_ip = inet_ntoa(in);

	LogDebug("ServerManager::CreateServerListPacket via client address [{0}]", client_ip);

	for (const auto& world_server : m_world_servers)
	{
		if (world_server->IsAuthorized()) {
			++server_count;
		}
	}

	SerializeBuffer buf;

	// LoginBaseMessage_Struct header
	buf.WriteInt32(sequence);
	buf.WriteInt8(0);
	buf.WriteInt8(0);
	buf.WriteInt32(0);

	// LoginBaseReplyMessage_Struct
	buf.WriteInt8(true);  // success (no error)
	buf.WriteInt32(0x65); // 101 "No Error" eqlsstr
	buf.WriteString("");

	// ServerListReply_Struct
	buf.WriteInt32(server_count);

	for (const auto& world_server : m_world_servers)
	{
		if (!world_server->IsAuthorized()) {
			LogDebug(
				"ServerManager::CreateServerListPacket | Server [{}] via IP [{}] is not authorized to be listed",
				world_server->GetServerLongName(),
				world_server->GetConnection()->Handle()->RemoteIP()
			);
			continue;
		}

		bool use_local_ip = false;

		std::string world_ip = world_server->GetConnection()->Handle()->RemoteIP();
		if (world_ip == client_ip || IpUtil::IsIpInPrivateRfc1918(client_ip)) {
			use_local_ip = true;
		}

		LogDebug(
			"CreateServerListPacket | Building list entry | Client [{}] IP [{}] Server Long Name [{}] Server IP [{}] ({})",
			client->GetAccountName(),
			client_ip,
			world_server->GetServerLongName(),
			use_local_ip ? world_server->GetLocalIP() : world_server->GetRemoteIP(),
			use_local_ip ? "Local" : "Remote"
		);

		world_server->SerializeForClientServerList(buf, use_local_ip);
	}

	return std::make_unique<EQApplicationPacket>(OP_ServerListResponse, buf);
}

/**
 * @param server_id
 * @param client_account_id
 * @param client_loginserver
 */
void ServerManager::SendUserToWorldRequest(
	unsigned int server_id,
	unsigned int client_account_id,
	const std::string &client_loginserver
)
{
	auto iter  = m_world_servers.begin();
	bool found = false;
	while (iter != m_world_servers.end()) {
		if ((*iter)->GetServerId() == server_id) {
			EQ::Net::DynamicPacket outapp;
			outapp.Resize(sizeof(UsertoWorldRequest_Struct));

			auto *user_to_world_request = (UsertoWorldRequest_Struct *) outapp.Data();
			user_to_world_request->worldid     = server_id;
			user_to_world_request->lsaccountid = client_account_id;
			strncpy(user_to_world_request->login, &client_loginserver[0], 64);
			(*iter)->GetConnection()->Send(ServerOP_UsertoWorldReq, outapp);
			found = true;

			if (server.options.IsDumpInPacketsOn()) {
				LogInfo("{0}", outapp.ToString());
			}
		}
		++iter;
	}

	if (!found && server.options.IsTraceOn()) {
		LogError("Client requested a user to world but supplied an invalid id of {0}", server_id);
	}
}

/**
 * @param server_long_name
 * @param server_short_name
 * @param ignore
 * @return
 */
bool ServerManager::ServerExists(
	std::string server_long_name,
	std::string server_short_name,
	WorldServer *ignore
)
{
	auto iter = m_world_servers.begin();
	while (iter != m_world_servers.end()) {
		if ((*iter).get() == ignore) {
			++iter;
			continue;
		}

		if ((*iter)->GetServerLongName() == server_long_name && (*iter)->GetServerShortName() == server_short_name) {
			return true;
		}

		++iter;
	}
	return false;
}

/**
 * @param server_long_name
 * @param server_short_name
 * @param ignore
 */
void ServerManager::DestroyServerByName(
	std::string server_long_name,
	std::string server_short_name,
	WorldServer *ignore
)
{
	auto iter = m_world_servers.begin();
	while (iter != m_world_servers.end()) {
		if ((*iter).get() == ignore) {
			++iter;
			continue;
		}

		if ((*iter)->GetServerLongName().compare(server_long_name) == 0 &&
			(*iter)->GetServerShortName().compare(server_short_name) == 0) {
			(*iter)->GetConnection()->Handle()->Disconnect();
			iter = m_world_servers.erase(iter);
			continue;
		}

		++iter;
	}
}

/**
 * @return
 */
const std::list<std::unique_ptr<WorldServer>> &ServerManager::getWorldServers() const
{
	return m_world_servers;
}
