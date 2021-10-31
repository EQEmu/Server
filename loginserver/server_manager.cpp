#include "server_manager.h"
#include "login_server.h"
#include "login_structures.h"
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
EQApplicationPacket *ServerManager::CreateServerListPacket(Client *client, uint32 sequence)
{
	unsigned int packet_size  = sizeof(ServerListHeader_Struct);
	unsigned int server_count = 0;
	in_addr      in{};
	in.s_addr = client->GetConnection()->GetRemoteIP();
	std::string client_ip = inet_ntoa(in);

	LogDebug("ServerManager::CreateServerListPacket via client address [{0}]", client_ip);

	auto iter = m_world_servers.begin();
	while (iter != m_world_servers.end()) {
		if (!(*iter)->IsAuthorized()) {
			LogDebug(
				"ServerManager::CreateServerListPacket | Server [{0}] via IP [{1}] is not authorized to be listed",
				(*iter)->GetServerLongName(),
				(*iter)->GetConnection()->Handle()->RemoteIP()
			);
			++iter;
			continue;
		}

		std::string world_ip = (*iter)->GetConnection()->Handle()->RemoteIP();
		if (world_ip == client_ip) {
			packet_size += (*iter)->GetServerLongName().size() + (*iter)->GetLocalIP().size() + 24;

			LogDebug(
				"CreateServerListPacket | Building list entry | Client [{0}] IP [{1}] Server Long Name [{2}] Server IP [{3}] (Local)",
				client->GetAccountName(),
				client_ip,
				(*iter)->GetServerLongName(),
				(*iter)->GetLocalIP()
			);
		}
		else if (IpUtil::IsIpInPrivateRfc1918(client_ip)) {
			packet_size += (*iter)->GetServerLongName().size() + (*iter)->GetLocalIP().size() + 24;

			LogDebug(
				"CreateServerListPacket | Building list entry | Client [{0}] IP [{1}] Server Long Name [{2}] Server IP [{3}] (Local)",
				client->GetAccountName(),
				client_ip,
				(*iter)->GetServerLongName(),
				(*iter)->GetLocalIP()
			);
		}
		else {
			packet_size += (*iter)->GetServerLongName().size() + (*iter)->GetRemoteIP().size() + 24;

			LogDebug(
				"CreateServerListPacket | Building list entry | Client [{0}] IP [{1}] Server Long Name [{2}] Server IP [{3}] (Remote)",
				client->GetAccountName(),
				client_ip,
				(*iter)->GetServerLongName(),
				(*iter)->GetRemoteIP()
			);
		}

		server_count++;
		++iter;
	}

	auto *outapp      = new EQApplicationPacket(OP_ServerListResponse, packet_size);
	auto *server_list = (ServerListHeader_Struct *) outapp->pBuffer;

	server_list->Unknown1 = sequence;
	server_list->Unknown2 = 0x00000000;
	server_list->Unknown3 = 0x01650000;

	/**
	* Not sure what this is but it should be noted setting it to
	* 0xFFFFFFFF crashes the client so: don't do that.
	*/
	server_list->Unknown4        = 0x00000000;
	server_list->NumberOfServers = server_count;

	unsigned char *data_pointer = outapp->pBuffer;
	data_pointer += sizeof(ServerListHeader_Struct);

	iter = m_world_servers.begin();
	while (iter != m_world_servers.end()) {
		if (!(*iter)->IsAuthorized()) {
			++iter;
			continue;
		}

		std::string world_ip = (*iter)->GetConnection()->Handle()->RemoteIP();
		if (world_ip == client_ip) {
			memcpy(data_pointer, (*iter)->GetLocalIP().c_str(), (*iter)->GetLocalIP().size());
			data_pointer += ((*iter)->GetLocalIP().size() + 1);
		}
		else if (IpUtil::IsIpInPrivateRfc1918(client_ip)) {
			memcpy(data_pointer, (*iter)->GetLocalIP().c_str(), (*iter)->GetLocalIP().size());
			data_pointer += ((*iter)->GetLocalIP().size() + 1);
		}
		else {
			memcpy(data_pointer, (*iter)->GetRemoteIP().c_str(), (*iter)->GetRemoteIP().size());
			data_pointer += ((*iter)->GetRemoteIP().size() + 1);
		}

		switch ((*iter)->GetServerListID()) {
			case 1: {
				*(unsigned int *) data_pointer = 0x00000030;
				break;
			}
			case 2: {
				*(unsigned int *) data_pointer = 0x00000009;
				break;
			}
			default: {
				*(unsigned int *) data_pointer = 0x00000001;
			}
		}

		data_pointer += 4;

		*(unsigned int *) data_pointer = (*iter)->GetServerId();
		data_pointer += 4;

		memcpy(data_pointer, (*iter)->GetServerLongName().c_str(), (*iter)->GetServerLongName().size());
		data_pointer += ((*iter)->GetServerLongName().size() + 1);

		memcpy(data_pointer, "EN", 2);
		data_pointer += 3;

		memcpy(data_pointer, "US", 2);
		data_pointer += 3;

		// 0 = Up, 1 = Down, 2 = Up, 3 = down, 4 = locked, 5 = locked(down)
		if ((*iter)->GetStatus() < 0) {
			if ((*iter)->GetZonesBooted() == 0) {
				*(uint32 *) data_pointer = 0x01;
			}
			else {
				*(uint32 *) data_pointer = 0x04;
			}
		}
		else {
			*(uint32 *) data_pointer = 0x02;
		}
		data_pointer += 4;

		*(uint32 *) data_pointer = (*iter)->GetPlayersOnline();
		data_pointer += 4;

		++iter;
	}

	return outapp;
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
