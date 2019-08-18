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
#include "world_server.h"
#include "login_server.h"
#include "login_structures.h"
#include "config.h"

#include "../common/eqemu_logsys.h"

extern LoginServer server;

WorldServer::WorldServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> c)
{
	connection = c;
	zones_booted = 0;
	players_online = 0;
	server_status = 0;
	runtime_id = 0;
	server_list_id = 0;
	server_type = 0;
	is_server_authorized = false;
	is_server_trusted = false;
	is_server_logged_in = false;

	c->OnMessage(ServerOP_NewLSInfo, std::bind(&WorldServer::ProcessNewLSInfo, this, std::placeholders::_1, std::placeholders::_2));
	c->OnMessage(ServerOP_LSStatus, std::bind(&WorldServer::ProcessLSStatus, this, std::placeholders::_1, std::placeholders::_2));
	c->OnMessage(ServerOP_UsertoWorldResp, std::bind(&WorldServer::ProcessUsertoWorldResp, this, std::placeholders::_1, std::placeholders::_2));
	c->OnMessage(ServerOP_LSAccountUpdate, std::bind(&WorldServer::ProcessLSAccountUpdate, this, std::placeholders::_1, std::placeholders::_2));
}

WorldServer::~WorldServer()
{

}

void WorldServer::Reset()
{
	zones_booted = 0;
	players_online = 0;
	server_status = 0;
	runtime_id;
	server_list_id = 0;
	server_type = 0;
	is_server_authorized = false;
	is_server_logged_in = false;
}

void WorldServer::ProcessNewLSInfo(uint16_t opcode, const EQ::Net::Packet &p)
{
	if (server.options.IsWorldTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "Application packet received from server: 0x%.4X, (size %u)", opcode, p.Length());
	}

	if (server.options.IsDumpInPacketsOn())
	{
		DumpPacket(opcode, p);
	}

	if (p.Length() < sizeof(ServerNewLSInfo_Struct))
	{
		Log(Logs::General, Logs::Error, "Received application packet from server that had opcode ServerOP_NewLSInfo, "
			"but was too small. Discarded to avoid buffer overrun.");
		return;
	}

	if (server.options.IsWorldTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "New Login Info Recieved.");
	}

	ServerNewLSInfo_Struct *info = (ServerNewLSInfo_Struct*)p.Data();
	Handle_NewLSInfo(info);
}

void WorldServer::ProcessLSStatus(uint16_t opcode, const EQ::Net::Packet &p)
{
	if (server.options.IsWorldTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "Application packet received from server: 0x%.4X, (size %u)", opcode, p.Length());
	}

	if (server.options.IsDumpInPacketsOn())
	{
		DumpPacket(opcode, p);
	}

	if (p.Length() < sizeof(ServerLSStatus_Struct))
	{
		Log(Logs::General, Logs::Error, "Recieved application packet from server that had opcode ServerOP_LSStatus, "
			"but was too small. Discarded to avoid buffer overrun.");
		return;
	}

	if (server.options.IsWorldTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "World Server Status Recieved.");
	}

	ServerLSStatus_Struct *ls_status = (ServerLSStatus_Struct*)p.Data();
	Handle_LSStatus(ls_status);
}

void WorldServer::ProcessUsertoWorldResp(uint16_t opcode, const EQ::Net::Packet &p)
{
	if (server.options.IsWorldTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "Application packet received from server: 0x%.4X, (size %u)", opcode, p.Length());
	}

	if (server.options.IsDumpInPacketsOn())
	{
		DumpPacket(opcode, p);
	}

	if (p.Length() < sizeof(UsertoWorldResponse_Struct))
	{
		Log(Logs::General, Logs::Error, "Recieved application packet from server that had opcode ServerOP_UsertoWorldResp, "
			"but was too small. Discarded to avoid buffer overrun.");
		return;
	}

	//I don't use world trace for this and here is why:
	//Because this is a part of the client login procedure it makes tracking client errors
	//While keeping world server spam with multiple servers connected almost impossible.
	if (server.options.IsTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "User-To-World Response received.");
	}

	UsertoWorldResponse_Struct *utwr = (UsertoWorldResponse_Struct*)p.Data();
	Log(Logs::General, Logs::Debug, "Trying to find client with user id of %u.", utwr->lsaccountid);
	Client *c = server.client_manager->GetClient(utwr->lsaccountid);
	if (c)
	{
		Log(Logs::General, Logs::Debug, "Found client with user id of %u and account name of %s.", utwr->lsaccountid, c->GetAccountName().c_str());
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_PlayEverquestResponse, sizeof(PlayEverquestResponse_Struct));
		PlayEverquestResponse_Struct *per = (PlayEverquestResponse_Struct*)outapp->pBuffer;
		per->Sequence = c->GetPlaySequence();
		per->ServerNumber = c->GetPlayServerID();
		Log(Logs::General, Logs::Debug, "Found sequence and play of %u %u", c->GetPlaySequence(), c->GetPlayServerID());

		Log(Logs::General, Logs::Netcode, "[Size: %u] %s", outapp->size, DumpPacketToString(outapp).c_str());

		if (utwr->response > 0)
		{
			per->Allowed = 1;
			SendClientAuth(c->GetConnection()->GetRemoteAddr(), c->GetAccountName(), c->GetKey(), c->GetAccountID());
		}

		switch (utwr->response)
		{
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

		if (server.options.IsTraceOn())
		{
			Log(Logs::General, Logs::Netcode, "Sending play response with following data, allowed %u, sequence %u, server number %u, message %u",
				per->Allowed, per->Sequence, per->ServerNumber, per->Message);
			Log(Logs::General, Logs::Netcode, "[Size: %u] %s", outapp->size, DumpPacketToString(outapp).c_str());
		}

		if (server.options.IsDumpOutPacketsOn())
		{
			DumpPacket(outapp);
		}

		c->SendPlayResponse(outapp);
		delete outapp;
	}
	else
	{
		Log(Logs::General, Logs::Error, "Recieved User-To-World Response for %u but could not find the client referenced!.", utwr->lsaccountid);
	}
}

void WorldServer::ProcessLSAccountUpdate(uint16_t opcode, const EQ::Net::Packet &p)
{
	if (server.options.IsWorldTraceOn())
	{
		Log(Logs::General, Logs::Netcode, "Application packet received from server: 0x%.4X, (size %u)", opcode, p.Length());
	}

	if (server.options.IsDumpInPacketsOn())
	{
		DumpPacket(opcode, p);
	}

	if (p.Length() < sizeof(ServerLSAccountUpdate_Struct))
	{
		Log(Logs::General, Logs::Error, "Recieved application packet from server that had opcode ServerLSAccountUpdate_Struct, "
			"but was too small. Discarded to avoid buffer overrun.");
		return;
	}

	Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate packet received from: %s", short_name.c_str());
	ServerLSAccountUpdate_Struct *lsau = (ServerLSAccountUpdate_Struct*)p.Data();
	if (is_server_trusted)
	{
		Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate update processed for: %s", lsau->useraccount);
		std::string name;
		std::string password;
		std::string email;
		name.assign(lsau->useraccount);
		password.assign(lsau->userpassword);
		email.assign(lsau->useremail);
		server.db->UpdateLSAccountInfo(lsau->useraccountid, name, password, email);
	}
}

void WorldServer::Handle_NewLSInfo(ServerNewLSInfo_Struct* i)
{
	if (is_server_logged_in)
	{
		Log(Logs::General, Logs::Error, "WorldServer::Handle_NewLSInfo called but the login server was already marked as logged in, aborting.");
		return;
	}

	if (strlen(i->account) <= 30)
	{
		account_name = i->account;
	}
	else
	{
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, account name was too long.");
		return;
	}

	if (strlen(i->password) <= 30)
	{
		account_password = i->password;
	}
	else
	{
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, account password was too long.");
		return;
	}

	if (strlen(i->name) <= 200)
	{
		long_name = i->name;
	}
	else
	{
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, long name was too long.");
		return;
	}

	if (strlen(i->shortname) <= 50)
	{
		short_name = i->shortname;
	}
	else
	{
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, short name was too long.");
		return;
	}

	if (strlen(i->local_address) <= 125)
	{
		if (strlen(i->local_address) == 0)
		{
			Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, local address was null, defaulting to localhost");
			local_ip = "127.0.0.1";
		}
		else
		{
			local_ip = i->local_address;
		}
	}
	else
	{
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, local address was too long.");
		return;
	}

	if (strlen(i->remote_address) <= 125)
	{
		if (strlen(i->remote_address) == 0)
		{
			remote_ip = GetConnection()->Handle()->RemoteIP();
			Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, remote address was null, defaulting to stream address %s.", remote_ip.c_str());
		}
		else
		{
			remote_ip = i->remote_address;
		}
	}
	else
	{
		remote_ip = GetConnection()->Handle()->RemoteIP();
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, remote address was too long, defaulting to stream address %s.", remote_ip.c_str());
	}

	if (strlen(i->serverversion) <= 64)
	{
		version = i->serverversion;
	}
	else
	{
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, server version was too long.");
		return;
	}

	if (strlen(i->protocolversion) <= 25)
	{
		protocol = i->protocolversion;
	}
	else
	{
		Log(Logs::General, Logs::Error, "Handle_NewLSInfo error, protocol version was too long.");
		return;
	}

	server_type = i->servertype;
	is_server_logged_in = true;

	if (server.options.IsRejectingDuplicateServers())
	{
		if (server.server_manager->ServerExists(long_name, short_name, this))
		{
			Log(Logs::General, Logs::Error, "World tried to login but there already exists a server that has that name.");
			return;
		}
	}
	else
	{
		if (server.server_manager->ServerExists(long_name, short_name, this))
		{
			Log(Logs::General, Logs::Error, "World tried to login but there already exists a server that has that name.");
			server.server_manager->DestroyServerByName(long_name, short_name, this);
		}
	}

	if (!server.options.IsUnregisteredAllowed())
	{
		if (account_name.size() > 0 && account_password.size() > 0)
		{
			unsigned int s_id = 0;
			unsigned int s_list_type = 0;
			unsigned int s_trusted = 0;
			std::string s_desc;
			std::string s_list_desc;
			std::string s_acct_name;
			std::string s_acct_pass;
			if (server.db->GetWorldRegistration(long_name, short_name, s_id, s_desc, s_list_type, s_trusted, s_list_desc, s_acct_name, s_acct_pass))
			{
				if (s_acct_name.size() == 0 || s_acct_pass.size() == 0)
				{
					Log(Logs::General, Logs::World_Server, "Server %s(%s) successfully logged into account that had no user/password requirement.",
						long_name.c_str(), short_name.c_str());
					is_server_authorized = true;
					SetRuntimeID(s_id);
					server_list_id = s_list_type;
					desc = s_desc;
				}
				else if (s_acct_name.compare(account_name) == 0 && s_acct_pass.compare(account_password) == 0)
				{
					Log(Logs::General, Logs::World_Server, "Server %s(%s) successfully logged in.",
						long_name.c_str(), short_name.c_str());
					is_server_authorized = true;
					SetRuntimeID(s_id);
					server_list_id = s_list_type;
					desc = s_desc;
					if (s_trusted) {
						Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate sent to world");
						is_server_trusted = true;

						EQ::Net::DynamicPacket outapp;
						connection->Send(ServerOP_LSAccountUpdate, outapp);
					}
				}
				else {
					Log(Logs::General, Logs::World_Server, "Server %s(%s) attempted to log in but account and password did not match the entry in the database, and only"
						" registered servers are allowed.", long_name.c_str(), short_name.c_str());
					return;
				}
			}
			else {
				Log(Logs::General, Logs::World_Server, "Server %s(%s) attempted to log in but database couldn't find an entry and only registered servers are allowed.",
					long_name.c_str(), short_name.c_str());
				return;
			}
		}
		else {
			Log(Logs::General, Logs::World_Server, "Server %s(%s) did not attempt to log in but only registered servers are allowed.",
				long_name.c_str(), short_name.c_str());
			return;
		}
	}
	else {
		unsigned int server_id = 0;
		unsigned int server_list_type = 0;
		unsigned int is_server_trusted = 0;
		std::string server_description;
		std::string server_list_description;
		std::string server_account_name;
		std::string server_account_password;


		if (server.db->GetWorldRegistration(
			long_name,
			short_name,
			server_id,
			server_description,
			server_list_type,
			is_server_trusted,
			server_list_description,
			server_account_name,
			server_account_password))
		{

			if (account_name.size() > 0 && account_password.size() > 0) {
				if (server_account_name.compare(account_name) == 0 && server_account_password.compare(account_password) == 0) {
					Log(Logs::General, Logs::World_Server, "Server %s(%s) successfully logged in.",
						long_name.c_str(), short_name.c_str());
					is_server_authorized = true;
					SetRuntimeID(server_id);
					server_list_id = server_list_type;
					desc = server_description;

					if (is_server_trusted) {
						Log(Logs::General, Logs::Netcode, "ServerOP_LSAccountUpdate sent to world");
						is_server_trusted = true;
						EQ::Net::DynamicPacket outapp;
						connection->Send(ServerOP_LSAccountUpdate, outapp);
					}
				}
				else {
					// this is the first of two cases where we should deny access even if unregistered is allowed
					Log(Logs::General, Logs::World_Server, "Server %s(%s) attempted to log in but account and password did not match the entry in the database.",
						long_name.c_str(), short_name.c_str());
				}
			}
			else {
				if (server_account_name.size() > 0 || server_account_password.size() > 0) {
					// this is the second of two cases where we should deny access even if unregistered is allowed
					Log(Logs::General, Logs::World_Server, "Server %s(%s) did not attempt to log in but this server requires a password.",
						long_name.c_str(), short_name.c_str());
				}
				else {
					Log(Logs::General, Logs::World_Server, "Server %s(%s) did not attempt to log in but unregistered servers are allowed.",
						long_name.c_str(), short_name.c_str());
					is_server_authorized = true;
					SetRuntimeID(server_id);
					server_list_id = 3;
				}
			}
		}
		else
		{
			Log(Logs::General, Logs::World_Server, "Server %s(%s) attempted to log in but database couldn't find an entry but unregistered servers are allowed.",
				long_name.c_str(), short_name.c_str());
			if (server.db->CreateWorldRegistration(long_name, short_name, server_id)) {
				is_server_authorized = true;
				SetRuntimeID(server_id);
				server_list_id = 3;
			}
		}
	}

	server.db->UpdateWorldRegistration(GetRuntimeID(), long_name, GetConnection()->Handle()->RemoteIP());
}

void WorldServer::Handle_LSStatus(ServerLSStatus_Struct *s)
{
	players_online = s->num_players;
	zones_booted = s->num_zones;
	server_status = s->status;
}

void WorldServer::SendClientAuth(std::string ip, std::string account, std::string key, unsigned int account_id)
{
	EQ::Net::DynamicPacket outapp;
	ClientAuth_Struct client_auth;
	client_auth.lsaccount_id = account_id;
	strncpy(client_auth.name, account.c_str(), 30);
	strncpy(client_auth.key, key.c_str(), 30);
	client_auth.lsadmin = 0;
	client_auth.worldadmin = 0;
	client_auth.ip = inet_addr(ip.c_str());

	std::string client_address(ip);
	std::string world_address(connection->Handle()->RemoteIP());

	if (client_address.compare(world_address) == 0) {
		client_auth.local = 1;
	}
	else if (client_address.find(server.options.GetLocalNetwork()) != std::string::npos) {
		client_auth.local = 1;
	}
	else {
		client_auth.local = 0;
	}

	outapp.PutSerialize(0, client_auth);
	connection->Send(ServerOP_LSClientAuth, outapp);

	if (server.options.IsDumpInPacketsOn())
	{
		DumpPacket(ServerOP_LSClientAuth, outapp);
	}
}
