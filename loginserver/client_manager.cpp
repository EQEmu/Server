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
#include "client_manager.h"
#include "login_server.h"
#include <eqemu_logsys.h>

extern LoginServer server;
extern bool run_server;

ClientManager::ClientManager()
{
	EQ::Net::EQStreamManagerOptions titanium_opts;
	titanium_opts.daybreak_options.port = atoi(server.config->GetVariable("Titanium", "port").c_str());
	titanium_stream.reset(new EQ::Net::EQStreamManager(titanium_opts));
	titanium_patch.reset(new EQ::Patches::LoginTitaniumPatch());
	titanium_stream->RegisterPotentialPatch(titanium_patch.get());

	titanium_stream->OnNewConnection(std::bind(&ClientManager::HandleNewConnectionTitanium, this, std::placeholders::_1));
	titanium_stream->OnConnectionStateChange(std::bind(&ClientManager::HandleConnectionChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	titanium_stream->OnPacketRecv(std::bind(&ClientManager::HandlePacket, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	
	EQ::Net::EQStreamManagerOptions sod_opts;
	sod_opts.daybreak_options.port = atoi(server.config->GetVariable("SoD", "port").c_str());
	sod_stream.reset(new EQ::Net::EQStreamManager(sod_opts));
	sod_patch.reset(new EQ::Patches::LoginSoDPatch());
	sod_stream->RegisterPotentialPatch(sod_patch.get());

	sod_stream->OnNewConnection(std::bind(&ClientManager::HandleNewConnectionSod, this, std::placeholders::_1));
	sod_stream->OnConnectionStateChange(std::bind(&ClientManager::HandleConnectionChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	sod_stream->OnPacketRecv(std::bind(&ClientManager::HandlePacket, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

ClientManager::~ClientManager()
{

}

void ClientManager::HandleNewConnectionTitanium(std::shared_ptr<EQ::Net::EQStream> connection)
{
	Log.OutF(Logs::General, Logs::Login_Server, "New Titanium client from {0}:{1}", connection->RemoteEndpoint(), connection->RemotePort());
	Client *c = new Client(connection, cv_titanium);
	clients.push_back(std::unique_ptr<Client>(c));
}

void ClientManager::HandleNewConnectionSod(std::shared_ptr<EQ::Net::EQStream> connection)
{
	Log.OutF(Logs::General, Logs::Login_Server, "New SoD client from {0}:{1}", connection->RemoteEndpoint(), connection->RemotePort());
	Client *c = new Client(connection, cv_sod);
	clients.push_back(std::unique_ptr<Client>(c));
}

void ClientManager::HandleConnectionChange(std::shared_ptr<EQ::Net::EQStream> connection, EQ::Net::DbProtocolStatus old_status, EQ::Net::DbProtocolStatus new_status)
{
	if (new_status == EQ::Net::DbProtocolStatus::StatusDisconnected) {
		Log.OutF(Logs::General, Logs::Login_Server, "Client has been disconnected, removing {0}:{1}", connection->RemoteEndpoint(), connection->RemotePort());
		auto iter = clients.begin();
		while (iter != clients.end()) {
			if ((*iter)->GetConnection() == connection) {
				clients.erase(iter);
				break;
			}
			++iter;
		}
	}
}

void ClientManager::HandlePacket(std::shared_ptr<EQ::Net::EQStream> connection, EmuOpcode opcode, EQ::Net::Packet &p)
{
	auto iter = clients.begin();
	while (iter != clients.end()) {
		if ((*iter)->GetConnection() == connection) {
			EQApplicationPacket app(opcode, (unsigned char*)p.Data(), (uint32)p.Length());
			(*iter)->Process(&app);
			return;
		}

		++iter;
	}
}

void ClientManager::UpdateServerList()
{
	auto iter = clients.begin();
	while(iter != clients.end())
	{
		(*iter)->SendServerListPacket();
		++iter;
	}
}

void ClientManager::RemoveExistingClient(unsigned int account_id)
{
	auto iter = clients.begin();
	while(iter != clients.end())
	{
		if((*iter)->GetAccountID() == account_id)
		{
			Log.Out(Logs::General, Logs::Login_Server, "Client attempting to log in and existing client already logged in, removing existing client.");
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
	auto iter = clients.begin();
	while(iter != clients.end())
	{
		if((*iter)->GetAccountID() == account_id)
		{
			cur = (*iter).get();
			count++;
		}
		++iter;
	}

	if(count > 1)
	{
		Log.Out(Logs::General, Logs::Error, "More than one client with a given account_id existed in the client list.");
	}
	return cur;
}

