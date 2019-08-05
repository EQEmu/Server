/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include "../common/global_define.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <stdlib.h>
#include "../common/version.h"
#include "../common/servertalk.h"
#include "../common/misc_functions.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "../common/string_util.h"
#include "../common/eqemu_logsys.h"
#include "../common/eqemu_logsys_fmt.h"
#include "login_server.h"
#include "login_server_list.h"
#include "zoneserver.h"
#include "worlddb.h"
#include "zonelist.h"
#include "clientlist.h"
#include "cliententry.h"
#include "world_config.h"

extern ZSList zoneserver_list;
extern ClientList client_list;
extern uint32 numzones;
extern uint32 numplayers;
extern volatile bool	RunLoops;

LoginServer::LoginServer(const char* iAddress, uint16 iPort, const char* Account, const char* Password, bool legacy)
{
	strn0cpy(LoginServerAddress, iAddress, 256);
	LoginServerPort = iPort;
	strn0cpy(LoginAccount, Account, 31);
	strn0cpy(LoginPassword, Password, 31);
	CanAccountUpdate = false;
	IsLegacy = legacy;
	Connect();
}

LoginServer::~LoginServer() {
}

void LoginServer::ProcessUsertoWorldReq(uint16_t opcode, EQ::Net::Packet &p) {
	const WorldConfig *Config = WorldConfig::get();
	Log(Logs::Detail, Logs::World_Server, "Recevied ServerPacket from LS OpCode 0x04x", opcode);

	UsertoWorldRequest_Struct* utwr = (UsertoWorldRequest_Struct*)p.Data();
	uint32 id = database.GetAccountIDFromLSID(utwr->lsaccountid);
	int16 status = database.CheckStatus(id);

	ServerPacket outpack;
	outpack.opcode = ServerOP_UsertoWorldResp;
	outpack.size = sizeof(UsertoWorldResponse_Struct);
	outpack.pBuffer = new uchar[outpack.size];
	memset(outpack.pBuffer, 0, outpack.size);
	UsertoWorldResponse_Struct* utwrs = (UsertoWorldResponse_Struct*)outpack.pBuffer;
	utwrs->lsaccountid = utwr->lsaccountid;
	utwrs->ToID = utwr->FromID;
	utwrs->worldid = utwr->worldid;
	utwrs->response = UserToWorldStatusSuccess;

	if (Config->Locked == true)
	{
		if (status < 100) {
			utwrs->response = UserToWorldStatusWorldUnavail;
			SendPacket(&outpack);
			return;
		}
	}

	int32 x = Config->MaxClients;
	if ((int32)numplayers >= x && x != -1 && x != 255 && status < 80) {
		utwrs->response = UserToWorldStatusWorldAtCapacity;
		SendPacket(&outpack);
		return;
	}

	if (status == -1) {
		utwrs->response = UserToWorldStatusSuspended;
		SendPacket(&outpack);
		return;
	}

	if (status == -2) {
		utwrs->response = UserToWorldStatusBanned;
		SendPacket(&outpack);
		return;
	}

	if (RuleB(World, EnforceCharacterLimitAtLogin)) {
		if (client_list.IsAccountInGame(utwr->lsaccountid)) {
			utwrs->response = UserToWorldStatusAlreadyOnline;
			SendPacket(&outpack);
			return;
		}
	}

	SendPacket(&outpack);
}

void LoginServer::ProcessLSClientAuth(uint16_t opcode, EQ::Net::Packet &p) {
	const WorldConfig *Config = WorldConfig::get();
	Log(Logs::Detail, Logs::World_Server, "Recevied ServerPacket from LS OpCode 0x04x", opcode);

	try {
		auto slsca = p.GetSerialize<ClientAuth_Struct>(0);
		client_list.CLEAdd(slsca.lsaccount_id, slsca.name, slsca.key, slsca.worldadmin, slsca.ip, slsca.local);
	}
	catch (std::exception &ex) {
		LogF(Logs::General, Logs::Error, "Error parsing LSClientAuth packet from world.\n{0}", ex.what());
	}
}

void LoginServer::ProcessLSFatalError(uint16_t opcode, EQ::Net::Packet &p) {
	const WorldConfig *Config = WorldConfig::get();
	Log(Logs::Detail, Logs::World_Server, "Recevied ServerPacket from LS OpCode 0x04x", opcode);

	Log(Logs::Detail, Logs::World_Server, "Login server responded with FatalError.");
	if (p.Length() > 1) {
		Log(Logs::Detail, Logs::World_Server, "     %s", (const char*)p.Data());
	}
}

void LoginServer::ProcessSystemwideMessage(uint16_t opcode, EQ::Net::Packet &p) {
	const WorldConfig *Config = WorldConfig::get();
	Log(Logs::Detail, Logs::World_Server, "Recevied ServerPacket from LS OpCode 0x04x", opcode);

	ServerSystemwideMessage* swm = (ServerSystemwideMessage*)p.Data();
	zoneserver_list.SendEmoteMessageRaw(0, 0, 0, swm->type, swm->message);
}

void LoginServer::ProcessLSRemoteAddr(uint16_t opcode, EQ::Net::Packet &p) {
	const WorldConfig *Config = WorldConfig::get();
	Log(Logs::Detail, Logs::World_Server, "Recevied ServerPacket from LS OpCode 0x04x", opcode);

	if (!Config->WorldAddress.length()) {
		WorldConfig::SetWorldAddress((char *)p.Data());
		Log(Logs::Detail, Logs::World_Server, "Loginserver provided %s as world address", (const char*)p.Data());
	}
}

void LoginServer::ProcessLSAccountUpdate(uint16_t opcode, EQ::Net::Packet &p) {
	const WorldConfig *Config = WorldConfig::get();
	Log(Logs::Detail, Logs::World_Server, "Recevied ServerPacket from LS OpCode 0x04x", opcode);

	Log(Logs::Detail, Logs::World_Server, "Received ServerOP_LSAccountUpdate packet from loginserver");
	CanAccountUpdate = true;
}

bool LoginServer::Connect() {
	std::string tmp;
	if (database.GetVariable("loginType", tmp) && strcasecmp(tmp.c_str(), "MinILogin") == 0) {
		minilogin = true;
		Log(Logs::Detail, Logs::World_Server, "Setting World to MiniLogin Server type");
	}
	else
		minilogin = false;

	if (minilogin && WorldConfig::get()->WorldAddress.length() == 0) {
		Log(Logs::Detail, Logs::World_Server, "**** For minilogin to work, you need to set the <address> element in the <world> section.");
		return false;
	}

	char errbuf[1024];
	if ((LoginServerIP = ResolveIP(LoginServerAddress, errbuf)) == 0) {
		Log(Logs::Detail, Logs::World_Server, "Unable to resolve '%s' to an IP.", LoginServerAddress);
		return false;
	}

	if (LoginServerIP == 0 || LoginServerPort == 0) {
		Log(Logs::Detail, Logs::World_Server, "Connect info incomplete, cannot connect: %s:%d", LoginServerAddress, LoginServerPort);
		return false;
	}

	if (IsLegacy) {
		legacy_client.reset(new EQ::Net::ServertalkLegacyClient(LoginServerAddress, LoginServerPort, false));
		legacy_client->OnConnect([this](EQ::Net::ServertalkLegacyClient *client) {
			if (client) {
				Log(Logs::Detail, Logs::World_Server, "Connected to Legacy Loginserver: %s:%d", LoginServerAddress, LoginServerPort);
				if (minilogin)
					SendInfo();
				else
					SendNewInfo();
				SendStatus();
				zoneserver_list.SendLSZones();

				statusupdate_timer.reset(new EQ::Timer(LoginServer_StatusUpdateInterval, true, [this](EQ::Timer *t) {
					SendStatus();
				}));
			}
			else {
				Log(Logs::Detail, Logs::World_Server, "Could not connect to Legacy Loginserver: %s:%d", LoginServerAddress, LoginServerPort);
			}
		});

		legacy_client->OnMessage(ServerOP_UsertoWorldReq, std::bind(&LoginServer::ProcessUsertoWorldReq, this, std::placeholders::_1, std::placeholders::_2));
		legacy_client->OnMessage(ServerOP_LSClientAuth, std::bind(&LoginServer::ProcessLSClientAuth, this, std::placeholders::_1, std::placeholders::_2));
		legacy_client->OnMessage(ServerOP_LSFatalError, std::bind(&LoginServer::ProcessLSFatalError, this, std::placeholders::_1, std::placeholders::_2));
		legacy_client->OnMessage(ServerOP_SystemwideMessage, std::bind(&LoginServer::ProcessSystemwideMessage, this, std::placeholders::_1, std::placeholders::_2));
		legacy_client->OnMessage(ServerOP_LSRemoteAddr, std::bind(&LoginServer::ProcessLSRemoteAddr, this, std::placeholders::_1, std::placeholders::_2));
		legacy_client->OnMessage(ServerOP_LSAccountUpdate, std::bind(&LoginServer::ProcessLSAccountUpdate, this, std::placeholders::_1, std::placeholders::_2));

	}
	else {
		client.reset(new EQ::Net::ServertalkClient(LoginServerAddress, LoginServerPort, false, "World", ""));
		client->OnConnect([this](EQ::Net::ServertalkClient *client) {
			if (client) {
				Log(Logs::Detail, Logs::World_Server, "Connected to Loginserver: %s:%d", LoginServerAddress, LoginServerPort);
				if (minilogin)
					SendInfo();
				else
					SendNewInfo();
				SendStatus();
				zoneserver_list.SendLSZones();

				statusupdate_timer.reset(new EQ::Timer(LoginServer_StatusUpdateInterval, true, [this](EQ::Timer *t) {
					SendStatus();
				}));
			}
			else {
				Log(Logs::Detail, Logs::World_Server, "Could not connect to Loginserver: %s:%d", LoginServerAddress, LoginServerPort);
			}
		});

		client->OnMessage(ServerOP_UsertoWorldReq, std::bind(&LoginServer::ProcessUsertoWorldReq, this, std::placeholders::_1, std::placeholders::_2));
		client->OnMessage(ServerOP_LSClientAuth, std::bind(&LoginServer::ProcessLSClientAuth, this, std::placeholders::_1, std::placeholders::_2));
		client->OnMessage(ServerOP_LSFatalError, std::bind(&LoginServer::ProcessLSFatalError, this, std::placeholders::_1, std::placeholders::_2));
		client->OnMessage(ServerOP_SystemwideMessage, std::bind(&LoginServer::ProcessSystemwideMessage, this, std::placeholders::_1, std::placeholders::_2));
		client->OnMessage(ServerOP_LSRemoteAddr, std::bind(&LoginServer::ProcessLSRemoteAddr, this, std::placeholders::_1, std::placeholders::_2));
		client->OnMessage(ServerOP_LSAccountUpdate, std::bind(&LoginServer::ProcessLSAccountUpdate, this, std::placeholders::_1, std::placeholders::_2));
	}

	return true;
}
void LoginServer::SendInfo() {
	const WorldConfig *Config = WorldConfig::get();

	auto pack = new ServerPacket;
	pack->opcode = ServerOP_LSInfo;
	pack->size = sizeof(ServerLSInfo_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerLSInfo_Struct* lsi = (ServerLSInfo_Struct*)pack->pBuffer;
	strcpy(lsi->protocolversion, EQEMU_PROTOCOL_VERSION);
	strcpy(lsi->serverversion, LOGIN_VERSION);
	strcpy(lsi->name, Config->LongName.c_str());
	strcpy(lsi->account, LoginAccount);
	strcpy(lsi->password, LoginPassword);
	strcpy(lsi->address, Config->WorldAddress.c_str());
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendNewInfo() {
	const WorldConfig *Config = WorldConfig::get();

	auto pack = new ServerPacket;
	pack->opcode = ServerOP_NewLSInfo;
	pack->size = sizeof(ServerNewLSInfo_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerNewLSInfo_Struct* lsi = (ServerNewLSInfo_Struct*)pack->pBuffer;
	strcpy(lsi->protocolversion, EQEMU_PROTOCOL_VERSION);
	strcpy(lsi->serverversion, LOGIN_VERSION);
	strcpy(lsi->name, Config->LongName.c_str());
	strcpy(lsi->shortname, Config->ShortName.c_str());
	strcpy(lsi->account, LoginAccount);
	strcpy(lsi->password, LoginPassword);
	if (Config->WorldAddress.length())
		strcpy(lsi->remote_address, Config->WorldAddress.c_str());
	if (Config->LocalAddress.length())
		strcpy(lsi->local_address, Config->LocalAddress.c_str());
	else {
		auto local_addr = IsLegacy ? legacy_client->Handle()->LocalIP() : client->Handle()->LocalIP();
		strcpy(lsi->local_address, local_addr.c_str());
		WorldConfig::SetLocalAddress(lsi->local_address);
	}
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendStatus() {
	auto pack = new ServerPacket;
	pack->opcode = ServerOP_LSStatus;
	pack->size = sizeof(ServerLSStatus_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerLSStatus_Struct* lss = (ServerLSStatus_Struct*)pack->pBuffer;

	if (WorldConfig::get()->Locked)
		lss->status = -2;
	else if (numzones <= 0)
		lss->status = -2;
	else
		lss->status = numplayers;

	lss->num_zones = numzones;
	lss->num_players = numplayers;
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendPacket(ServerPacket *pack)
{
	if (IsLegacy) {
		if (legacy_client) {
			legacy_client->SendPacket(pack);
		}
	}
	else {
		if (client) {
			client->SendPacket(pack);
		}
	}
}

void LoginServer::SendAccountUpdate(ServerPacket* pack) {
	ServerLSAccountUpdate_Struct* s = (ServerLSAccountUpdate_Struct *)pack->pBuffer;
	if (CanUpdate()) {
		Log(Logs::Detail, Logs::World_Server, "Sending ServerOP_LSAccountUpdate packet to loginserver: %s:%d", LoginServerAddress, LoginServerPort);
		strn0cpy(s->worldaccount, LoginAccount, 30);
		strn0cpy(s->worldpassword, LoginPassword, 30);
		SendPacket(pack);
	}
}

bool LoginServer::Connected()
{
	if (IsLegacy) {
		if (legacy_client) {
			return legacy_client->Connected();
		}
	}
	else {
		if (client) {
			return client->Connected();
		}
	}

	return false;
}
