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
#include "ClientManager.h"
#include "ErrorLog.h"
#include "LoginServer.h"

extern ErrorLog *server_log;
extern LoginServer server;
extern bool run_server;

ClientManager::ClientManager()
{
	int titanium_port = atoi(server.config->GetVariable("Titanium", "port").c_str());
	titanium_stream = new EQStreamFactory(LoginStream, titanium_port);
	titanium_ops = new RegularOpcodeManager;
	if(!titanium_ops->LoadOpcodes(server.config->GetVariable("Titanium", "opcodes").c_str()))
	{
		server_log->Log(log_error, "ClientManager fatal error: couldn't load opcodes for Titanium file %s.",
			server.config->GetVariable("Titanium", "opcodes").c_str());
		run_server = false;
	}

	if(titanium_stream->Open())
	{
		server_log->Log(log_network, "ClientManager listening on Titanium stream.");
	}
	else
	{
		server_log->Log(log_error, "ClientManager fatal error: couldn't open Titanium stream.");
		run_server = false;
	}

	int sod_port = atoi(server.config->GetVariable("SoD", "port").c_str());
	sod_stream = new EQStreamFactory(LoginStream, sod_port);
	sod_ops = new RegularOpcodeManager;
	if(!sod_ops->LoadOpcodes(server.config->GetVariable("SoD", "opcodes").c_str()))
	{
		server_log->Log(log_error, "ClientManager fatal error: couldn't load opcodes for SoD file %s.",
			server.config->GetVariable("SoD", "opcodes").c_str());
		run_server = false;
	}

	if(sod_stream->Open())
	{
		server_log->Log(log_network, "ClientManager listening on SoD stream.");
	}
	else
	{
		server_log->Log(log_error, "ClientManager fatal error: couldn't open SoD stream.");
		run_server = false;
	}
}

ClientManager::~ClientManager()
{
	if(titanium_stream)
	{
		titanium_stream->Close();
		delete titanium_stream;
	}

	if(titanium_ops)
	{
		delete titanium_ops;
	}

	if(sod_stream)
	{
		sod_stream->Close();
		delete sod_stream;
	}

	if(sod_ops)
	{
		delete sod_ops;
	}
}

void ClientManager::Process()
{
	ProcessDisconnect();
	EQStream *cur = titanium_stream->Pop();
	while(cur)
	{
		struct in_addr in;
		in.s_addr = cur->GetRemoteIP();
		server_log->Log(log_network, "New Titanium client connection from %s:%d", inet_ntoa(in), ntohs(cur->GetRemotePort()));

		cur->SetOpcodeManager(&titanium_ops);
		Client *c = new Client(cur, cv_titanium);
		clients.push_back(c);
		cur = titanium_stream->Pop();
	}

	cur = sod_stream->Pop();
	while(cur)
	{
		struct in_addr in;
		in.s_addr = cur->GetRemoteIP();
		server_log->Log(log_network, "New SoD client connection from %s:%d", inet_ntoa(in), ntohs(cur->GetRemotePort()));

		cur->SetOpcodeManager(&sod_ops);
		Client *c = new Client(cur, cv_sod);
		clients.push_back(c);
		cur = sod_stream->Pop();
	}

	list<Client*>::iterator iter = clients.begin();
	while(iter != clients.end())
	{
		if((*iter)->Process() == false)
		{
			server_log->Log(log_client, "Client had a fatal error and had to be removed from the login.");
			delete (*iter);
			iter = clients.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void ClientManager::ProcessDisconnect()
{
	list<Client*>::iterator iter = clients.begin();
	while(iter != clients.end())
	{
		EQStream *c = (*iter)->GetConnection();
		if(c->CheckClosed())
		{
			server_log->Log(log_network, "Client disconnected from the server, removing client.");
			delete (*iter);
			iter = clients.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void ClientManager::UpdateServerList()
{
	list<Client*>::iterator iter = clients.begin();
	while(iter != clients.end())
	{
		(*iter)->SendServerListPacket();
		++iter;
	}
}

void ClientManager::RemoveExistingClient(unsigned int account_id)
{
	list<Client*>::iterator iter = clients.begin();
	while(iter != clients.end())
	{
		if((*iter)->GetAccountID() == account_id)
		{
			server_log->Log(log_network, "Client attempting to log in and existing client already logged in, removing existing client.");
			delete (*iter);
			iter = clients.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

Client *ClientManager::GetClient(unsigned int account_id)
{
	Client *cur = nullptr;
	int count = 0;
	list<Client*>::iterator iter = clients.begin();
	while(iter != clients.end())
	{
		if((*iter)->GetAccountID() == account_id)
		{
			cur = (*iter);
			count++;
		}
		++iter;
	}

	if(count > 1)
	{
		server_log->Log(log_client_error, "More than one client with a given account_id existed in the client list.");
	}
	return cur;
}

