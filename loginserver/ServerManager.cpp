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
#include "ServerManager.h"
#include "LoginServer.h"
#include "ErrorLog.h"
#include "LoginStructures.h"
#include <stdlib.h>

extern ErrorLog *server_log;
extern LoginServer server;
extern bool run_server;

ServerManager::ServerManager()
{
	char error_buffer[TCPConnection_ErrorBufferSize];

	int listen_port = atoi(server.config->GetVariable("options", "listen_port").c_str());
	tcps = new EmuTCPServer(listen_port, true);
	if(tcps->Open(listen_port, error_buffer))
	{
		server_log->Log(log_network, "ServerManager listening on port %u", listen_port);
	}
	else
	{
		server_log->Log(log_error, "ServerManager fatal error opening port on %u: %s", listen_port, error_buffer);
		run_server = false;
	}
}

ServerManager::~ServerManager()
{
	if(tcps)
	{
		tcps->Close();
		delete tcps;
	}
}

void ServerManager::Process()
{
	ProcessDisconnect();
	EmuTCPConnection *tcp_c = nullptr;
	while(tcp_c = tcps->NewQueuePop())
	{
		in_addr tmp;
		tmp.s_addr = tcp_c->GetrIP();
		server_log->Log(log_network, "New world server connection from %s:%d", inet_ntoa(tmp), tcp_c->GetrPort());

		WorldServer *cur = GetServerByAddress(tcp_c->GetrIP());
		if(cur)
		{
			server_log->Log(log_network, "World server already existed for %s, removing existing connection and updating current.", inet_ntoa(tmp));
			cur->GetConnection()->Free();
			cur->SetConnection(tcp_c);
			cur->Reset();
		}
		else
		{
			WorldServer *w = new WorldServer(tcp_c);
			world_servers.push_back(w);
		}
	}

	list<WorldServer*>::iterator iter = world_servers.begin();
	while(iter != world_servers.end())
	{
		if((*iter)->Process() == false)
		{
			server_log->Log(log_world, "World server %s had a fatal error and had to be removed from the login.", (*iter)->GetLongName().c_str());
			delete (*iter);
			iter = world_servers.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void ServerManager::ProcessDisconnect()
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while(iter != world_servers.end())
	{
		EmuTCPConnection *c = (*iter)->GetConnection();
		if(!c->Connected())
		{
			in_addr tmp;
			tmp.s_addr = c->GetrIP();
			server_log->Log(log_network, "World server disconnected from the server, removing server and freeing connection.");
			c->Free();
			delete (*iter);
			iter = world_servers.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

WorldServer* ServerManager::GetServerByAddress(unsigned int address)
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while(iter != world_servers.end())
	{
		if((*iter)->GetConnection()->GetrIP() == address)
		{
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
	while(iter != world_servers.end())
	{
		if((*iter)->IsAuthorized() == false)
		{
			++iter;
			continue;
		}

		in.s_addr = (*iter)->GetConnection()->GetrIP();
		string world_ip = inet_ntoa(in);

		if(world_ip.compare(client_ip) == 0)
		{
			packet_size += (*iter)->GetLongName().size() + (*iter)->GetLocalIP().size() + 24;
		}
		else if(client_ip.find(server.options.GetLocalNetwork()) != string::npos)
		{
			packet_size += (*iter)->GetLongName().size() + (*iter)->GetLocalIP().size() + 24;
		}
		else
		{
			packet_size += (*iter)->GetLongName().size() + (*iter)->GetRemoteIP().size() + 24;
		}

		server_count++;
		++iter;
	}

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ServerListResponse, packet_size);
	ServerListHeader_Struct *sl = (ServerListHeader_Struct*)outapp->pBuffer;
	sl->Unknown1 = 0x00000004;
	sl->Unknown2 = 0x00000000;
	sl->Unknown3 = 0x01650000;
	/**
	* Not sure what this is but it should be noted setting it to
	* 0xFFFFFFFF crashes the client so: don't do that.
	*/
	sl->Unknown4 = 0x00000000;
	sl->NumberOfServers = server_count;

	unsigned char *data_ptr = outapp->pBuffer;
	data_ptr += sizeof(ServerListHeader_Struct);

	iter = world_servers.begin();
	while(iter != world_servers.end())
	{
		if((*iter)->IsAuthorized() == false)
		{
			++iter;
			continue;
		}

		in.s_addr = (*iter)->GetConnection()->GetrIP();
		string world_ip = inet_ntoa(in);
		if(world_ip.compare(client_ip) == 0)
		{
			memcpy(data_ptr, (*iter)->GetLocalIP().c_str(), (*iter)->GetLocalIP().size());
			data_ptr += ((*iter)->GetLocalIP().size() + 1);
		}
		else if(client_ip.find(server.options.GetLocalNetwork()) != string::npos)
		{
			memcpy(data_ptr, (*iter)->GetLocalIP().c_str(), (*iter)->GetLocalIP().size());
			data_ptr += ((*iter)->GetLocalIP().size() + 1);
		}
		else
		{
			memcpy(data_ptr, (*iter)->GetRemoteIP().c_str(), (*iter)->GetRemoteIP().size());
			data_ptr += ((*iter)->GetRemoteIP().size() + 1);
		}

		switch((*iter)->GetServerListID())
		{
		case 1:
			{
				*(unsigned int*)data_ptr = 0x00000030;
				break;
			}
		case 2:
			{
				*(unsigned int*)data_ptr = 0x00000009;
				break;
			}
		default:
			{
				*(unsigned int*)data_ptr = 0x00000001;
			}
		}
		data_ptr += 4;

		*(unsigned int*)data_ptr = (*iter)->GetRuntimeID();
		data_ptr += 4;

		memcpy(data_ptr, (*iter)->GetLongName().c_str(), (*iter)->GetLongName().size());
		data_ptr += ((*iter)->GetLongName().size() + 1);

		memcpy(data_ptr, "EN", 2);
		data_ptr += 3;

		memcpy(data_ptr, "US", 2);
		data_ptr += 3;

		// 0 = Up, 1 = Down, 2 = Up, 3 = down, 4 = locked, 5 = locked(down)
		if((*iter)->GetStatus() < 0)
		{
			if((*iter)->GetZonesBooted() == 0)
			{
				*(uint32*)data_ptr = 0x01;
			}
			else
			{
				*(uint32*)data_ptr = 0x04;
			}
		}
		else
		{
			*(uint32*)data_ptr = 0x02;
		}
		data_ptr += 4;

		*(uint32*)data_ptr = (*iter)->GetPlayersOnline();
		data_ptr += 4;

		++iter;
	}

	return outapp;
}

void ServerManager::SendUserToWorldRequest(unsigned int server_id, unsigned int client_account_id)
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	bool found = false;
	while(iter != world_servers.end())
	{
		if((*iter)->GetRuntimeID() == server_id)
		{
			ServerPacket *outapp = new ServerPacket(ServerOP_UsertoWorldReq, sizeof(UsertoWorldRequest_Struct));
			UsertoWorldRequest_Struct *utwr = (UsertoWorldRequest_Struct*)outapp->pBuffer;
			utwr->worldid = server_id;
			utwr->lsaccountid = client_account_id;
			(*iter)->GetConnection()->SendPacket(outapp);
			found = true;

			if(server.options.IsDumpInPacketsOn())
			{
				DumpPacket(outapp);
			}
			delete outapp;
		}
		++iter;
	}

	if(!found && server.options.IsTraceOn())
	{
		server_log->Log(log_client_error, "Client requested a user to world but supplied an invalid id of %u.", server_id);
	}
}

bool ServerManager::ServerExists(string l_name, string s_name, WorldServer *ignore)
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while(iter != world_servers.end())
	{
		if((*iter) == ignore)
		{
			++iter;
			continue;
		}

		if((*iter)->GetLongName().compare(l_name) == 0 && (*iter)->GetShortName().compare(s_name) == 0)
		{
			return true;
		}

		++iter;
	}
	return false;
}

void ServerManager::DestroyServerByName(string l_name, string s_name, WorldServer *ignore)
{
	list<WorldServer*>::iterator iter = world_servers.begin();
	while(iter != world_servers.end())
	{
		if((*iter) == ignore)
		{
			++iter;
		}

		if((*iter)->GetLongName().compare(l_name) == 0 && (*iter)->GetShortName().compare(s_name) == 0)
		{
			EmuTCPConnection *c = (*iter)->GetConnection();
			if(c->Connected())
			{
				c->Disconnect();
			}
			c->Free();
			delete (*iter);
			iter = world_servers.erase(iter);
		}

		++iter;
	}
}

