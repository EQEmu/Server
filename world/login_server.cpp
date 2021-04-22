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
#include "login_server.h"
#include "login_server_list.h"
#include "zoneserver.h"
#include "worlddb.h"
#include "zonelist.h"
#include "clientlist.h"
#include "cliententry.h"
#include "world_config.h"


extern ZSList        zoneserver_list;
extern ClientList    client_list;
extern uint32        numzones;
extern uint32        numplayers;
extern volatile bool RunLoops;

LoginServer::LoginServer(const char *iAddress, uint16 iPort, const char *Account, const char *Password, bool legacy)
{
	strn0cpy(LoginServerAddress, iAddress, 256);
	LoginServerPort  = iPort;
	LoginAccount     = Account;
	LoginPassword    = Password;
	CanAccountUpdate = false;
	IsLegacy         = legacy;
	Connect();
}

LoginServer::~LoginServer()
{
}

void LoginServer::ProcessUsertoWorldReqLeg(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("[ProcessUsertoWorldReqLeg] Received ServerPacket from LS OpCode {:#04x}", opcode);

	UsertoWorldRequestLegacy_Struct *utwr  = (UsertoWorldRequestLegacy_Struct *) p.Data();
	uint32                          id     = database.GetAccountIDFromLSID("eqemu", utwr->lsaccountid);
	int16                           status = database.CheckStatus(id);

	LogDebug(
		"[ProcessUsertoWorldReqLeg] id [{}] status [{}] account_id [{}] world_id [{}] from_id [{}] to_id [{}] ip [{}]",
		id,
		status,
		utwr->lsaccountid,
		utwr->worldid,
		utwr->FromID,
		utwr->ToID,
		utwr->IPAddr
	);

	ServerPacket outpack;
	outpack.opcode  = ServerOP_UsertoWorldRespLeg;
	outpack.size    = sizeof(UsertoWorldResponseLegacy_Struct);
	outpack.pBuffer = new uchar[outpack.size];
	memset(outpack.pBuffer, 0, outpack.size);

	UsertoWorldResponseLegacy_Struct *utwrs = (UsertoWorldResponseLegacy_Struct *) outpack.pBuffer;
	utwrs->lsaccountid = utwr->lsaccountid;
	utwrs->ToID        = utwr->FromID;
	utwrs->worldid     = utwr->worldid;
	utwrs->response    = UserToWorldStatusSuccess;

	if (Config->Locked) {
		if (status < (RuleI(GM, MinStatusToBypassLockedServer))) {
			LogDebug("[ProcessUsertoWorldReqLeg] Server locked and status is not high enough for account_id [{0}]", utwr->lsaccountid);
			utwrs->response = UserToWorldStatusWorldUnavail;
			SendPacket(&outpack);
			return;
		}
	}

	int32 x = Config->MaxClients;
	if ((int32) numplayers >= x && x != -1 && x != 255 && status < (RuleI(GM, MinStatusToBypassLockedServer))) {
		LogDebug("[ProcessUsertoWorldReqLeg] World at capacity account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusWorldAtCapacity;
		SendPacket(&outpack);
		return;
	}

	if (status == -1) {
		LogDebug("[ProcessUsertoWorldReqLeg] User suspended account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusSuspended;
		SendPacket(&outpack);
		return;
	}

	if (status == -2) {
		LogDebug("[ProcessUsertoWorldReqLeg] User banned account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusBanned;
		SendPacket(&outpack);
		return;
	}

	if (RuleB(World, EnforceCharacterLimitAtLogin)) {
		if (client_list.IsAccountInGame(utwr->lsaccountid)) {
			LogDebug("[ProcessUsertoWorldReqLeg] User already online account_id [{0}]", utwr->lsaccountid);
			utwrs->response = UserToWorldStatusAlreadyOnline;
			SendPacket(&outpack);
			return;
		}
	}

	LogDebug("[ProcessUsertoWorldReqLeg] Sent response to account_id [{0}]", utwr->lsaccountid);

	SendPacket(&outpack);
}

void LoginServer::ProcessUsertoWorldReq(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("[ProcessUsertoWorldReq] Received ServerPacket from LS OpCode {:#04x}", opcode);

	UsertoWorldRequest_Struct *utwr  = (UsertoWorldRequest_Struct *) p.Data();
	uint32                    id     = database.GetAccountIDFromLSID(utwr->login, utwr->lsaccountid);
	int16                     status = database.CheckStatus(id);

	LogDebug(
		"[ProcessUsertoWorldReq] id [{}] status [{}] account_id [{}] world_id [{}] from_id [{}] to_id [{}] ip [{}]",
		id,
		status,
		utwr->lsaccountid,
		utwr->worldid,
		utwr->FromID,
		utwr->ToID,
		utwr->IPAddr
	);

	ServerPacket outpack;
	outpack.opcode  = ServerOP_UsertoWorldResp;
	outpack.size    = sizeof(UsertoWorldResponse_Struct);
	outpack.pBuffer = new uchar[outpack.size];
	memset(outpack.pBuffer, 0, outpack.size);

	UsertoWorldResponse_Struct *utwrs = (UsertoWorldResponse_Struct *) outpack.pBuffer;
	utwrs->lsaccountid = utwr->lsaccountid;
	utwrs->ToID        = utwr->FromID;
	strn0cpy(utwrs->login, utwr->login, 64);
	utwrs->worldid  = utwr->worldid;
	utwrs->response = UserToWorldStatusSuccess;

	if (Config->Locked == true) {
		if (status < (RuleI(GM, MinStatusToBypassLockedServer))) {
			LogDebug("[ProcessUsertoWorldReq] Server locked and status is not high enough for account_id [{0}]", utwr->lsaccountid);
			utwrs->response = UserToWorldStatusWorldUnavail;
			SendPacket(&outpack);
			return;
		}
	}

	int32 x = Config->MaxClients;
	if ((int32) numplayers >= x && x != -1 && x != 255 && status < (RuleI(GM, MinStatusToBypassLockedServer))) {
		LogDebug("[ProcessUsertoWorldReq] World at capacity account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusWorldAtCapacity;
		SendPacket(&outpack);
		return;
	}

	if (status == -1) {
		LogDebug("[ProcessUsertoWorldReq] User suspended account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusSuspended;
		SendPacket(&outpack);
		return;
	}

	if (status == -2) {
		LogDebug("[ProcessUsertoWorldReq] User banned account_id [{0}]", utwr->lsaccountid);
		utwrs->response = UserToWorldStatusBanned;
		SendPacket(&outpack);
		return;
	}

	if (RuleB(World, EnforceCharacterLimitAtLogin)) {
		if (client_list.IsAccountInGame(utwr->lsaccountid)) {
			LogDebug("[ProcessUsertoWorldReq] User already online account_id [{0}]", utwr->lsaccountid);
			utwrs->response = UserToWorldStatusAlreadyOnline;
			SendPacket(&outpack);
			return;
		}
	}

	LogDebug("[ProcessUsertoWorldReq] Sent response to account_id [{0}]", utwr->lsaccountid);

	SendPacket(&outpack);
}

void LoginServer::ProcessLSClientAuthLegacy(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	try {
		auto client_authentication_request = p.GetSerialize<ClientAuthLegacy_Struct>(0);

		LogDebug(
			"Processing Loginserver Auth Legacy | account_id [{0}] account_name [{1}] key [{2}] admin [{3}] ip [{4}] "
			"local_network [{5}]",
			client_authentication_request.loginserver_account_id,
			client_authentication_request.loginserver_account_name,
			client_authentication_request.key,
			client_authentication_request.is_world_admin,
			client_authentication_request.ip,
			client_authentication_request.is_client_from_local_network
		);

		client_list.CLEAdd(
			client_authentication_request.loginserver_account_id,
			"eqemu",
			client_authentication_request.loginserver_account_name,
			client_authentication_request.key,
			client_authentication_request.is_world_admin,
			client_authentication_request.ip,
			client_authentication_request.is_client_from_local_network
		);
	}
	catch (std::exception &ex) {
		LogError("Error parsing ClientAuthLegacy packet from world\nReason [{0}]", ex.what());
	}
}

void LoginServer::ProcessLSClientAuth(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	try {
		auto client_authentication_request = p.GetSerialize<ClientAuth_Struct>(0);

		LogDebug(
			"Processing Loginserver Auth | account_id [{0}] account_name [{1}] loginserver_name [{2}] key [{3}] "
			"admin [{4}] ip [{5}] local_network [{6}]",
			client_authentication_request.loginserver_account_id,
			client_authentication_request.account_name,
			client_authentication_request.loginserver_name,
			client_authentication_request.key,
			client_authentication_request.is_world_admin,
			client_authentication_request.ip,
			client_authentication_request.is_client_from_local_network
		);

		client_list.CLEAdd(
			client_authentication_request.loginserver_account_id,
			client_authentication_request.loginserver_name,
			client_authentication_request.account_name,
			client_authentication_request.key,
			client_authentication_request.is_world_admin,
			client_authentication_request.ip,
			client_authentication_request.is_client_from_local_network
		);
	}
	catch (std::exception &ex) {
		LogError("Error parsing ClientAuth packet from world\nReason [{0}]", ex.what());
	}
}

void LoginServer::ProcessLSFatalError(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	LogInfo("Login server responded with FatalError");
	if (p.Length() > 1) {
		LogError("Error [{}]", (const char *) p.Data());
	}
}

void LoginServer::ProcessSystemwideMessage(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	ServerSystemwideMessage *swm = (ServerSystemwideMessage *) p.Data();
	zoneserver_list.SendEmoteMessageRaw(0, 0, 0, swm->type, swm->message);
}

void LoginServer::ProcessLSRemoteAddr(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	if (!Config->WorldAddress.length()) {
		WorldConfig::SetWorldAddress((char *) p.Data());
		LogInfo("Loginserver provided [{}] as world address", (const char *) p.Data());
	}
}

void LoginServer::ProcessLSAccountUpdate(uint16_t opcode, EQ::Net::Packet &p)
{
	const WorldConfig *Config = WorldConfig::get();
	LogNetcode("Received ServerPacket from LS OpCode {:#04x}", opcode);

	LogNetcode("Received ServerOP_LSAccountUpdate packet from loginserver");
	CanAccountUpdate = true;
}

bool LoginServer::Connect()
{
	char errbuf[1024];
	if ((LoginServerIP = ResolveIP(LoginServerAddress, errbuf)) == 0) {
		LogInfo("Unable to resolve [{}] to an IP", LoginServerAddress);
		return false;
	}

	if (LoginServerIP == 0 || LoginServerPort == 0) {
		LogInfo(
			"Connect info incomplete, cannot connect: [{0}:{1}]",
			LoginServerAddress,
			LoginServerPort
		);

		return false;
	}

	if (IsLegacy) {
		legacy_client = std::make_unique<EQ::Net::ServertalkLegacyClient>(LoginServerAddress, LoginServerPort, false);
		legacy_client->OnConnect(
			[this](EQ::Net::ServertalkLegacyClient *client) {
				if (client) {
					LogInfo(
						"Connected to Legacy Loginserver: [{0}:{1}]",
						LoginServerAddress,
						LoginServerPort
					);

					SendInfo();
					SendStatus();
					zoneserver_list.SendLSZones();

					statusupdate_timer = std::make_unique<EQ::Timer>(

							LoginServer_StatusUpdateInterval, true, [this](EQ::Timer *t) {
								SendStatus();
							}

					);
				}
				else {
					LogInfo(
						"Could not connect to Legacy Loginserver: [{0}:{1}]",
						LoginServerAddress,
						LoginServerPort
					);
				}
			}
		);

		legacy_client->OnMessage(
			ServerOP_UsertoWorldReqLeg,
			std::bind(
				&LoginServer::ProcessUsertoWorldReqLeg,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		legacy_client->OnMessage(
			ServerOP_UsertoWorldReq,
			std::bind(
				&LoginServer::ProcessUsertoWorldReq,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		legacy_client->OnMessage(
			ServerOP_LSClientAuthLeg,
			std::bind(
				&LoginServer::ProcessLSClientAuthLegacy,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		legacy_client->OnMessage(
			ServerOP_LSClientAuth,
			std::bind(
				&LoginServer::ProcessLSClientAuth,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		legacy_client->OnMessage(
			ServerOP_LSFatalError,
			std::bind(
				&LoginServer::ProcessLSFatalError,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		legacy_client->OnMessage(
			ServerOP_SystemwideMessage,
			std::bind(
				&LoginServer::ProcessSystemwideMessage,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		legacy_client->OnMessage(
			ServerOP_LSRemoteAddr,
			std::bind(
				&LoginServer::ProcessLSRemoteAddr,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		legacy_client->OnMessage(
			ServerOP_LSAccountUpdate,
			std::bind(
				&LoginServer::ProcessLSAccountUpdate,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}
	else {
		client = std::make_unique<EQ::Net::ServertalkClient>(LoginServerAddress, LoginServerPort, false, "World", "");
		client->OnConnect(
			[this](EQ::Net::ServertalkClient *client) {
				if (client) {
					LogInfo(
						"Connected to Loginserver: [{0}:{1}]",
						LoginServerAddress,
						LoginServerPort
					);
					SendInfo();
					SendStatus();
					zoneserver_list.SendLSZones();

					statusupdate_timer = std::make_unique<EQ::Timer>(

							LoginServer_StatusUpdateInterval, true, [this](EQ::Timer *t) {
								SendStatus();
							}
						);
				}
				else {
					LogInfo(
						"Could not connect to Loginserver: [{0}:{1}]",
						LoginServerAddress,
						LoginServerPort
					);
				}
			}
		);

		client->OnMessage(
			ServerOP_UsertoWorldReqLeg,
			std::bind(
				&LoginServer::ProcessUsertoWorldReqLeg,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		client->OnMessage(
			ServerOP_UsertoWorldReq,
			std::bind(
				&LoginServer::ProcessUsertoWorldReq,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		client->OnMessage(
			ServerOP_LSClientAuthLeg,
			std::bind(
				&LoginServer::ProcessLSClientAuthLegacy,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		client->OnMessage(
			ServerOP_LSClientAuth,
			std::bind(
				&LoginServer::ProcessLSClientAuth,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		client->OnMessage(
			ServerOP_LSFatalError,
			std::bind(
				&LoginServer::ProcessLSFatalError,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		client->OnMessage(
			ServerOP_SystemwideMessage,
			std::bind(
				&LoginServer::ProcessSystemwideMessage,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		client->OnMessage(
			ServerOP_LSRemoteAddr,
			std::bind(
				&LoginServer::ProcessLSRemoteAddr,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
		client->OnMessage(
			ServerOP_LSAccountUpdate,
			std::bind(
				&LoginServer::ProcessLSAccountUpdate,
				this,
				std::placeholders::_1,
				std::placeholders::_2
			)
		);
	}

	m_keepalive = std::make_unique<EQ::Timer>(5000, true, std::bind(&LoginServer::OnKeepAlive, this, std::placeholders::_1));

	return true;
}

void LoginServer::SendInfo()
{
	const WorldConfig *Config = WorldConfig::get();

	auto pack = new ServerPacket;
	pack->opcode  = ServerOP_NewLSInfo;
	pack->size    = sizeof(ServerNewLSInfo_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerNewLSInfo_Struct *lsi = (ServerNewLSInfo_Struct *) pack->pBuffer;
	strcpy(lsi->protocol_version, EQEMU_PROTOCOL_VERSION);
	strcpy(lsi->server_version, LOGIN_VERSION);
	strcpy(lsi->server_long_name, Config->LongName.c_str());
	strcpy(lsi->server_short_name, Config->ShortName.c_str());
	strn0cpy(lsi->account_name, LoginAccount.c_str(), 30);
	strn0cpy(lsi->account_password, LoginPassword.c_str(), 30);
	if (Config->WorldAddress.length()) {
		strcpy(lsi->remote_ip_address, Config->WorldAddress.c_str());
	}
	if (Config->LocalAddress.length()) {
		strcpy(lsi->local_ip_address, Config->LocalAddress.c_str());
	}
	else {
		auto local_addr = IsLegacy ? legacy_client->Handle()->LocalIP() : client->Handle()->LocalIP();
		strcpy(lsi->local_ip_address, local_addr.c_str());
		WorldConfig::SetLocalAddress(lsi->local_ip_address);
	}
	SendPacket(pack);
	delete pack;
}

void LoginServer::SendStatus()
{
	auto pack = new ServerPacket;
	pack->opcode  = ServerOP_LSStatus;
	pack->size    = sizeof(ServerLSStatus_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	auto loginserver_status = (ServerLSStatus_Struct *) pack->pBuffer;

	if (WorldConfig::get()->Locked) {
		loginserver_status->status = -2;
	}
	else if (numzones <= 0) {
		loginserver_status->status = -2;
	}
	else {
		loginserver_status->status = numplayers;
	}

	loginserver_status->num_zones   = numzones;
	loginserver_status->num_players = numplayers;
	SendPacket(pack);
	delete pack;
}

/**
 * @param pack
 */
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

void LoginServer::SendAccountUpdate(ServerPacket *pack)
{
	auto *ls_account_update = (ServerLSAccountUpdate_Struct *) pack->pBuffer;
	if (CanUpdate()) {
		LogInfo(
			"Sending ServerOP_LSAccountUpdate packet to loginserver: [{0}]:[{1}]",
			LoginServerAddress,
			LoginServerPort
		);
		strn0cpy(ls_account_update->worldaccount, LoginAccount.c_str(), 30);
		strn0cpy(ls_account_update->worldpassword, LoginPassword.c_str(), 30);
		SendPacket(pack);
	}
}

void LoginServer::OnKeepAlive(EQ::Timer *t)
{
	ServerPacket pack(ServerOP_KeepAlive, 0);
	SendPacket(&pack);
}
