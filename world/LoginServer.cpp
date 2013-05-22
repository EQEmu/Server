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
#include "../common/debug.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <stdlib.h>
#include "../common/version.h"

#ifdef _WINDOWS
	#include <process.h>
	#include <windows.h>
	#include <winsock.h>

	#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else // Pyro: fix for linux
	#include <sys/socket.h>
#ifdef FREEBSD //Timothy Whitman - January 7, 2003
	#include <sys/types.h>
#endif
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
	#include <unistd.h>
	#include <errno.h>

	#include "../common/unix.h"

	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	extern int errno;
#endif

#define IGNORE_LS_FATAL_ERROR

#include "../common/servertalk.h"
#include "LoginServer.h"
#include "LoginServerList.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "../common/MiscFunctions.h"
#include "zoneserver.h"
#include "worlddb.h"
#include "zonelist.h"
#include "clientlist.h"
#include "WorldConfig.h"

extern ZSList zoneserver_list;
extern ClientList client_list;
extern uint32 numzones;
extern uint32 numplayers;
extern volatile bool	RunLoops;

LoginServer::LoginServer(const char* iAddress, uint16 iPort, const char* Account, const char* Password)
: statusupdate_timer(LoginServer_StatusUpdateInterval)
{
	strn0cpy(LoginServerAddress,iAddress,256);
	LoginServerPort = iPort;
	strn0cpy(LoginAccount,Account,31);
	strn0cpy(LoginPassword,Password,31);
	CanAccountUpdate = false;
	tcpc = new EmuTCPConnection(true);
	tcpc->SetPacketMode(EmuTCPConnection::packetModeLogin);
}

LoginServer::~LoginServer() {
	delete tcpc;
}

bool LoginServer::Process() {
	const WorldConfig *Config=WorldConfig::get();

	if (statusupdate_timer.Check()) {
		this->SendStatus();
	}

	/************ Get all packets from packet manager out queue and process them ************/
	ServerPacket *pack = 0;
	while((pack = tcpc->PopPacket()))
	{
		_log(WORLD__LS_TRACE,"Recevied ServerPacket from LS OpCode 0x04x",pack->opcode);
		_hex(WORLD__LS_TRACE,pack->pBuffer,pack->size);

		switch(pack->opcode) {
			case 0:
				break;
			case ServerOP_KeepAlive: {
				// ignore this
				break;
			}
			case ServerOP_UsertoWorldReq: {
				UsertoWorldRequest_Struct* utwr = (UsertoWorldRequest_Struct*) pack->pBuffer;
				uint32 id = database.GetAccountIDFromLSID(utwr->lsaccountid);
				int16 status = database.CheckStatus(id);

				ServerPacket* outpack = new ServerPacket;
				outpack->opcode = ServerOP_UsertoWorldResp;
				outpack->size = sizeof(UsertoWorldResponse_Struct);
				outpack->pBuffer = new uchar[outpack->size];
				memset(outpack->pBuffer, 0, outpack->size);
				UsertoWorldResponse_Struct* utwrs = (UsertoWorldResponse_Struct*) outpack->pBuffer;
				utwrs->lsaccountid = utwr->lsaccountid;
				utwrs->ToID = utwr->FromID;

				if(Config->Locked == true)
				{
					if((status == 0 || status < 100) && (status != -2 || status != -1))
						utwrs->response = 0;
					if(status >= 100)
						utwrs->response = 1;
				}
				else {
					utwrs->response = 1;
				}

				int32 x = Config->MaxClients;
				if( (int32)numplayers >= x && x != -1 && x != 255 && status < 80)
					utwrs->response = -3;

				if(status == -1)
					utwrs->response = -1;
				if(status == -2)
					utwrs->response = -2;

				utwrs->worldid = utwr->worldid;
				SendPacket(outpack);
				delete outpack;
				break;
			}
			case ServerOP_LSClientAuth: {
				ServerLSClientAuth* slsca = (ServerLSClientAuth*) pack->pBuffer;

				if (RuleI(World, AccountSessionLimit) >= 0) {
					// Enforce the limit on the number of characters on the same account that can be
					// online at the same time.
					client_list.EnforceSessionLimit(slsca->lsaccount_id);
				}

				client_list.CLEAdd(slsca->lsaccount_id, slsca->name, slsca->key, slsca->worldadmin, slsca->ip, slsca->local);
				break;
			}
			case ServerOP_LSFatalError: {
	#ifndef IGNORE_LS_FATAL_ERROR
				WorldConfig::DisableLoginserver();
				_log(WORLD__LS_ERR, "Login server responded with FatalError. Disabling reconnect.");
	#else
			_log(WORLD__LS_ERR, "Login server responded with FatalError.");
	#endif
				if (pack->size > 1) {
					_log(WORLD__LS_ERR, "     %s",pack->pBuffer);
				}
				break;
			}
			case ServerOP_SystemwideMessage: {
				ServerSystemwideMessage* swm = (ServerSystemwideMessage*) pack->pBuffer;
				zoneserver_list.SendEmoteMessageRaw(0, 0, 0, swm->type, swm->message);
				break;
			}
			case ServerOP_LSRemoteAddr: {
				if (!Config->WorldAddress.length()) {
					WorldConfig::SetWorldAddress((char *)pack->pBuffer);
					_log(WORLD__LS, "Loginserver provided %s as world address",pack->pBuffer);
				}
				break;
			}
			case ServerOP_LSAccountUpdate: {
				_log(WORLD__LS, "Received ServerOP_LSAccountUpdate packet from loginserver");
				CanAccountUpdate = true;
				break;
			}
			default:
			{
				_log(WORLD__LS_ERR, "Unknown LSOpCode: 0x%04x size=%d",(int)pack->opcode,pack->size);
	DumpPacket(pack->pBuffer, pack->size);
				break;
			}
		}
		delete pack;
	}

	return true;
}

bool LoginServer::InitLoginServer() {
	if(Connected() == false) {
		if(ConnectReady()) {
			_log(WORLD__LS, "Connecting to login server: %s:%d",LoginServerAddress,LoginServerPort);
			Connect();
		} else {
			_log(WORLD__LS_ERR, "Not connected but not ready to connect, this is bad: %s:%d",
				LoginServerAddress,LoginServerPort);
		}
	}
	return true;
}

bool LoginServer::Connect() {
	char tmp[25];
	if(database.GetVariable("loginType",tmp,sizeof(tmp)) && strcasecmp(tmp,"MinILogin") == 0){
		minilogin = true;
		_log(WORLD__LS, "Setting World to MiniLogin Server type");
	}
	else
		minilogin = false;

	if (minilogin && WorldConfig::get()->WorldAddress.length()==0) {
		_log(WORLD__LS_ERR, "**** For minilogin to work, you need to set the <address> element in the <world> section.");
		return false;
	}

	char errbuf[TCPConnection_ErrorBufferSize];
	if ((LoginServerIP = ResolveIP(LoginServerAddress, errbuf)) == 0) {
		_log(WORLD__LS_ERR, "Unable to resolve '%s' to an IP.",LoginServerAddress);
		return false;
	}

	if (LoginServerIP == 0 || LoginServerPort == 0) {
		_log(WORLD__LS_ERR, "Connect info incomplete, cannot connect: %s:%d",LoginServerAddress,LoginServerPort);
		return false;
	}

	if (tcpc->ConnectIP(LoginServerIP, LoginServerPort, errbuf)) {
		_log(WORLD__LS, "Connected to Loginserver: %s:%d",LoginServerAddress,LoginServerPort);
		if (minilogin)
			SendInfo();
		else
			SendNewInfo();
		SendStatus();
		zoneserver_list.SendLSZones();
		return true;
	}
	else {
		_log(WORLD__LS_ERR, "Could not connect to login server: %s:%d %s",LoginServerAddress,LoginServerPort,errbuf);
		return false;
	}
}
void LoginServer::SendInfo() {
	const WorldConfig *Config=WorldConfig::get();

	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_LSInfo;
	pack->size = sizeof(ServerLSInfo_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerLSInfo_Struct* lsi = (ServerLSInfo_Struct*) pack->pBuffer;
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
	uint16 port;
	const WorldConfig *Config=WorldConfig::get();

	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_NewLSInfo;
	pack->size = sizeof(ServerNewLSInfo_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerNewLSInfo_Struct* lsi = (ServerNewLSInfo_Struct*) pack->pBuffer;
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
		tcpc->GetSockName(lsi->local_address,&port);
		WorldConfig::SetLocalAddress(lsi->local_address);
	}
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendStatus() {
	statusupdate_timer.Start();
	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_LSStatus;
	pack->size = sizeof(ServerLSStatus_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerLSStatus_Struct* lss = (ServerLSStatus_Struct*) pack->pBuffer;

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

void LoginServer::SendAccountUpdate(ServerPacket* pack) {
	ServerLSAccountUpdate_Struct* s = (ServerLSAccountUpdate_Struct *) pack->pBuffer;
	if(CanUpdate()) {
		_log(WORLD__LS, "Sending ServerOP_LSAccountUpdate packet to loginserver: %s:%d",LoginServerAddress,LoginServerPort);
		strn0cpy(s->worldaccount, LoginAccount, 30);
		strn0cpy(s->worldpassword, LoginPassword, 30);
		SendPacket(pack);
	}
}

