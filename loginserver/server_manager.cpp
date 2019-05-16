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
#include "server_manager.h"
#include "login_server.h"
#include "login_structures.h"
#include <stdlib.h>

#include "../common/eqemu_logsys.h"
#include "../common/eqemu_logsys_fmt.h"

extern LoginServer server;
extern bool run_server;

ServerManager::ServerManager()
{
	int listen_port = atoi(server.config->GetVariable("options", "listen_port").c_str());

	server_connection.reset(new EQ::Net::ServertalkServer());
	EQ::Net::ServertalkServerOptions opts;
	opts.port = listen_port;
	opts.ipv6 = false;
	server_connection->Listen(opts);

	server_connection->OnConnectionIdentified("World", [this](std::shared_ptr<EQ::Net::ServertalkServerConnection> c) {
		LogF(Logs::General, Logs::Login_Server, "New world server connection from {0}:{1}", c->Handle()->RemoteIP(), c->Handle()->RemotePort());

		auto iter = world_servers.begin();
		while (iter != world_servers.end()) {
			if ((*iter)->GetConnection()->Handle()->RemoteIP().compare(c->Handle()->RemoteIP()) == 0 &&
				(*iter)->GetConnection()->Handle()->RemotePort() == c->Handle()->RemotePort()) {
				LogF(Logs::General, Logs::Login_Server, "World server already existed for {0}:{1}, removing existing connection.",
					c->Handle()->RemoteIP(), c->Handle()->RemotePort());

				world_servers.erase(iter);
				break;
			}

			++iter;
		}

		world_servers.push_back(std::unique_ptr<WorldServer>(new WorldServer(c)));
	});

	server_connection->OnConnectionRemoved("World", [this](std::shared_ptr<EQ::Net::ServertalkServerConnection> c) {
		auto iter = world_servers.begin();
		while (iter != world_servers.end()) {
			if ((*iter)->GetConnection()->GetUUID() == c->GetUUID()) {
				LogF(Logs::General, Logs::World_Server, "World server {0} has been disconnected, removing.", (*iter)->GetLongName().c_str());
				world_servers.erase(iter);
				return;
			}

			++iter;
		}
	});
}

ServerManager::~ServerManager()
{

}

WorldServer* ServerManager::GetServerByAddress(const std::string &addr, int port)
{
	auto iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter)->GetConnection()->Handle()->RemoteIP() == addr && (*iter)->GetConnection()->Handle()->RemotePort()) {
			return (*iter).get();
		}
		++iter;
	}

	return nullptr;
}

EQApplicationPacket *ServerManager::CreateServerListPacket(Client *c, uint32 seq)
{
	unsigned int packet_size = sizeof(ServerListHeader_Struct);
	unsigned int server_count = 0;
	in_addr in;
	in.s_addr = c->GetConnection()->GetRemoteIP();
	std::string client_ip = inet_ntoa(in);

	auto iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter)->IsAuthorized() == false) {
			++iter;
			continue;
		}

		std::string world_ip = (*iter)->GetConnection()->Handle()->RemoteIP();

		if (world_ip.compare(client_ip) == 0) {
			packet_size += (*iter)->GetLongName().size() + (*iter)->GetLocalIP().size() + 24;
		}
		else if (client_ip.find(server.options.GetLocalNetwork()) != std::string::npos) {
			packet_size += (*iter)->GetLongName().size() + (*iter)->GetLocalIP().size() + 24;
		}
		else {
			packet_size += (*iter)->GetLongName().size() + (*iter)->GetRemoteIP().size() + 24;
		}

		server_count++;
		++iter;
	}

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ServerListResponse, packet_size);
	ServerListHeader_Struct *server_list = (ServerListHeader_Struct*)outapp->pBuffer;
	server_list->Unknown1 = seq;
	server_list->Unknown2 = 0x00000000;
	server_list->Unknown3 = 0x01650000;

	/**
	* Not sure what this is but it should be noted setting it to
	* 0xFFFFFFFF crashes the client so: don't do that.
	*/
	server_list->Unknown4 = 0x00000000;
	server_list->NumberOfServers = server_count;

	unsigned char *data_pointer = outapp->pBuffer;
	data_pointer += sizeof(ServerListHeader_Struct);

	iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter)->IsAuthorized() == false) {
			++iter;
			continue;
		}

		std::string world_ip = (*iter)->GetConnection()->Handle()->RemoteIP();
		if (world_ip.compare(client_ip) == 0) {
			memcpy(data_pointer, (*iter)->GetLocalIP().c_str(), (*iter)->GetLocalIP().size());
			data_pointer += ((*iter)->GetLocalIP().size() + 1);
		}
		else if (client_ip.find(server.options.GetLocalNetwork()) != std::string::npos) {
			memcpy(data_pointer, (*iter)->GetLocalIP().c_str(), (*iter)->GetLocalIP().size());
			data_pointer += ((*iter)->GetLocalIP().size() + 1);
		}
		else {
			memcpy(data_pointer, (*iter)->GetRemoteIP().c_str(), (*iter)->GetRemoteIP().size());
			data_pointer += ((*iter)->GetRemoteIP().size() + 1);
		}

		switch ((*iter)->GetServerListID()) {
		case 1: {
			*(unsigned int*)data_pointer = 0x00000030;
			break;
		}
		case 2: {
			*(unsigned int*)data_pointer = 0x00000009;
			break;
		}
		default: {
			*(unsigned int*)data_pointer = 0x00000001;
		}
		}

		data_pointer += 4;

		*(unsigned int*)data_pointer = (*iter)->GetRuntimeID();
		data_pointer += 4;

		memcpy(data_pointer, (*iter)->GetLongName().c_str(), (*iter)->GetLongName().size());
		data_pointer += ((*iter)->GetLongName().size() + 1);

		memcpy(data_pointer, "EN", 2);
		data_pointer += 3;

		memcpy(data_pointer, "US", 2);
		data_pointer += 3;

		// 0 = Up, 1 = Down, 2 = Up, 3 = down, 4 = locked, 5 = locked(down)
		if ((*iter)->GetStatus() < 0) {
			if ((*iter)->GetZonesBooted() == 0) {
				*(uint32*)data_pointer = 0x01;
			}
			else {
				*(uint32*)data_pointer = 0x04;
			}
		}
		else {
			*(uint32*)data_pointer = 0x02;
		}
		data_pointer += 4;

		*(uint32*)data_pointer = (*iter)->GetPlayersOnline();
		data_pointer += 4;

		++iter;
	}

	return outapp;
}

void ServerManager::SendUserToWorldRequest(unsigned int server_id, unsigned int client_account_id)
{
	auto iter = world_servers.begin();
	bool found = false;
	while (iter != world_servers.end()) {
		if ((*iter)->GetRuntimeID() == server_id) {
			EQ::Net::DynamicPacket outapp;
			outapp.Resize(sizeof(UsertoWorldRequest_Struct));
			UsertoWorldRequest_Struct *utwr = (UsertoWorldRequest_Struct*)outapp.Data();
			utwr->worldid = server_id;
			utwr->lsaccountid = client_account_id;
			(*iter)->GetConnection()->Send(ServerOP_UsertoWorldReq, outapp);
			found = true;

			if (server.options.IsDumpInPacketsOn()) {
				LogF(Logs::General, Logs::Login_Server, "{0}", outapp.ToString());
			}
		}
		++iter;
	}

	if (!found && server.options.IsTraceOn()) {
		Log(Logs::General, Logs::Error, "Client requested a user to world but supplied an invalid id of %u.", server_id);
	}
}

bool ServerManager::ServerExists(std::string l_name, std::string s_name, WorldServer *ignore)
{
	auto iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter).get() == ignore) {
			++iter;
			continue;
		}

		if ((*iter)->GetLongName().compare(l_name) == 0 && (*iter)->GetShortName().compare(s_name) == 0) {
			return true;
		}

		++iter;
	}
	return false;
}

void ServerManager::DestroyServerByName(std::string l_name, std::string s_name, WorldServer *ignore)
{
	auto iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter).get() == ignore) {
			++iter;
			continue;
		}

		if ((*iter)->GetLongName().compare(l_name) == 0 && (*iter)->GetShortName().compare(s_name) == 0) {
			(*iter)->GetConnection()->Handle()->Disconnect();
			iter = world_servers.erase(iter);
			continue;
		}

		++iter;
	}
}
