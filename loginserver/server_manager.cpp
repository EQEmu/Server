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

extern EQEmuLogSys Log;
extern LoginServer server;
extern bool run_server;

ServerManager::ServerManager()
{
	char error_buffer[TCPConnection_ErrorBufferSize];

	int listen_port = atoi(server.config->GetVariable("options", "listen_port").c_str());
	tcps = new EmuTCPServer(listen_port, true);
	if(tcps->Open(listen_port, error_buffer)) {
		Log.Out(Logs::General, Logs::Login_Server, "ServerManager listening on port %u", listen_port);
	}
	else {
		Log.Out(Logs::General, Logs::Error, "ServerManager fatal error opening port on %u: %s", listen_port, error_buffer);
		run_server = false;
	}
}

ServerManager::~ServerManager()
{
	if (tcps) {
		tcps->Close();
		delete tcps;
	}
}

void ServerManager::Process()
{
	ProcessDisconnect();
	EmuTCPConnection *tcp_c = nullptr;
	while (tcp_c = tcps->NewQueuePop()) {
		in_addr tmp;
		tmp.s_addr = tcp_c->GetrIP();
		Log.Out(Logs::General, Logs::Login_Server, "New world server connection from %s:%d", inet_ntoa(tmp), tcp_c->GetrPort());

		WorldServer *server_entity = GetServerByAddress(tcp_c->GetrIP());
		if (server_entity) {
			Log.Out(Logs::General, Logs::Login_Server, "World server already existed for %s, removing existing connection and updating current.", inet_ntoa(tmp));
			server_entity->GetConnection()->Free();
			server_entity->SetConnection(tcp_c);
			server_entity->Reset();
		}
		else {
			WorldServer *w = new WorldServer(tcp_c);
			world_servers.push_back(w);
		}
	}

	list<WorldServer*>::iterator iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter)->Process() == false) {
			Log.Out(Logs::General, Logs::World_Server, "World server %s had a fatal error and had to be removed from the login.", (*iter)->GetLongName().c_str());
			delete (*iter);
			iter = world_servers.erase(iter);
		}
		else {
			++iter;
		}
	}
}

void ServerManager::ProcessDisconnect()
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while (iter != world_servers.end()) {
		EmuTCPConnection *connection = (*iter)->GetConnection();
		if (!connection->Connected()) {
			in_addr tmp;
			tmp.s_addr = connection->GetrIP();
			Log.Out(Logs::General, Logs::Login_Server, "World server disconnected from the server, removing server and freeing connection.");
			connection->Free();
			delete (*iter);
			iter = world_servers.erase(iter);
		}
		else {
			++iter;
		}
	}
}

WorldServer* ServerManager::GetServerByAddress(unsigned int address)
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter)->GetConnection()->GetrIP() == address) {
			return (*iter);
		}
		++iter;
	}

	return nullptr;
}

EQApplicationPacket *ServerManager::CreateServerListPacket(Client *c)
{
	unsigned int packet_size = sizeof(ServerListHeader_Struct);
	unsigned int server_count = 0;
	in_addr in;
	in.s_addr = c->GetConnection()->GetRemoteIP();
	string client_ip = inet_ntoa(in);

	list<WorldServer*>::iterator iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter)->IsAuthorized() == false) {
			++iter;
			continue;
		}

		in.s_addr = (*iter)->GetConnection()->GetrIP();
		string world_ip = inet_ntoa(in);

		if (world_ip.compare(client_ip) == 0) {
			packet_size += (*iter)->GetLongName().size() + (*iter)->GetLocalIP().size() + 24;
		}
		else if (client_ip.find(server.options.GetLocalNetwork()) != string::npos) {
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
	server_list->Unknown1 = 0x00000004;
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

		in.s_addr = (*iter)->GetConnection()->GetrIP();
		string world_ip = inet_ntoa(in);
		if (world_ip.compare(client_ip) == 0) {
			memcpy(data_pointer, (*iter)->GetLocalIP().c_str(), (*iter)->GetLocalIP().size());
			data_pointer += ((*iter)->GetLocalIP().size() + 1);
		}
		else if (client_ip.find(server.options.GetLocalNetwork()) != string::npos) {
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
	list<WorldServer*>::iterator iter = world_servers.begin();
	bool found = false;
	while (iter != world_servers.end()) {
		if ((*iter)->GetRuntimeID() == server_id) {
			ServerPacket *outapp = new ServerPacket(ServerOP_UsertoWorldReq, sizeof(UsertoWorldRequest_Struct));
			UsertoWorldRequest_Struct *utwr = (UsertoWorldRequest_Struct*)outapp->pBuffer;
			utwr->worldid = server_id;
			utwr->lsaccountid = client_account_id;
			(*iter)->GetConnection()->SendPacket(outapp);
			found = true;

			if (server.options.IsDumpInPacketsOn()) {
				DumpPacket(outapp);
			}
			delete outapp;
		}
		++iter;
	}

	if (!found && server.options.IsTraceOn()) {
		Log.Out(Logs::General, Logs::Error, "Client requested a user to world but supplied an invalid id of %u.", server_id);
	}
}

bool ServerManager::ServerExists(string l_name, string s_name, WorldServer *ignore)
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter) == ignore) {
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

void ServerManager::DestroyServerByName(string l_name, string s_name, WorldServer *ignore)
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while (iter != world_servers.end()) {
		if ((*iter) == ignore) {
			++iter;
		}

		if ((*iter)->GetLongName().compare(l_name) == 0 && (*iter)->GetShortName().compare(s_name) == 0) {
			EmuTCPConnection *c = (*iter)->GetConnection();
			if (c->Connected()) {
				c->Disconnect();
			}
			c->Free();
			delete (*iter);
			iter = world_servers.erase(iter);
		}

		++iter;
	}
}