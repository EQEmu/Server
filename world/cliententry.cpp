/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
#include "cliententry.h"
#include "clientlist.h"
#include "login_server.h"
#include "login_server_list.h"
#include "shared_task_manager.h"
#include "worlddb.h"
#include "zoneserver.h"
#include "world_config.h"
#include "../common/guilds.h"
#include "../common/string_util.h"

extern uint32          numplayers;
extern LoginServerList loginserverlist;
extern ClientList      client_list;
extern volatile bool   RunLoops;
extern SharedTaskManager shared_task_manager;

/**
 * @param in_id
 * @param in_loginserver_id
 * @param in_loginserver_name
 * @param in_login_name
 * @param in_login_key
 * @param in_is_world_admin
 * @param ip
 * @param local
 */
ClientListEntry::ClientListEntry(
	uint32 in_id,
	uint32 in_loginserver_id,
	const char *in_loginserver_name,
	const char *in_login_name,
	const char *in_login_key,
	int16 in_is_world_admin,
	uint32 ip,
	uint8 local
)
	: id(in_id)
{
	ClearVars(true);

	LogDebug(
		"ClientListEntry in_id [{0}] in_loginserver_id [{1}] in_loginserver_name [{2}] in_login_name [{3}] in_login_key [{4}] "
		" in_is_world_admin [{5}] ip [{6}] local [{7}]",
		in_id,
		in_loginserver_id,
		in_loginserver_name,
		in_login_name,
		in_login_key,
		in_is_world_admin,
		ip,
		local
	);

	pIP   = ip;
	pLSID = in_loginserver_id;
	if (in_loginserver_id > 0) {
		paccountid = database.GetAccountIDFromLSID(in_loginserver_name, in_loginserver_id, paccountname, &padmin);
	}

	strn0cpy(loginserver_account_name, in_login_name, sizeof(loginserver_account_name));
	strn0cpy(plskey, in_login_key, sizeof(plskey));
	strn0cpy(source_loginserver, in_loginserver_name, sizeof(source_loginserver));
	pworldadmin = in_is_world_admin;
	plocal      = (local == 1);

	memset(pLFGComments, 0, 64);
}

ClientListEntry::ClientListEntry(uint32 in_id, ZoneServer *iZS, ServerClientList_Struct *scl, CLE_Status iOnline)
	: id(in_id)
{
	ClearVars(true);

	pIP   = 0;
	pLSID = scl->LSAccountID;
	strn0cpy(loginserver_account_name, scl->name, sizeof(loginserver_account_name));
	strn0cpy(plskey, scl->lskey, sizeof(plskey));
	pworldadmin = 0;

	paccountid = scl->AccountID;
	strn0cpy(paccountname, scl->AccountName, sizeof(paccountname));
	padmin = scl->Admin;

	pinstance       = 0;
	pLFGFromLevel   = 0;
	pLFGToLevel     = 0;
	pLFGMatchFilter = false;
	memset(pLFGComments, 0, 64);

	if (iOnline >= CLE_Status::Zoning) {
		Update(iZS, scl, iOnline);
	}
	else {
		SetOnline(iOnline);
	}
}

ClientListEntry::~ClientListEntry()
{
	if (RunLoops) {
		Camp(); // updates zoneserver's numplayers
		client_list.RemoveCLEReferances(this);
	}
	for (auto &elem : tell_queue)
		safe_delete_array(elem);
	tell_queue.clear();
}

void ClientListEntry::SetChar(uint32 iCharID, const char *iCharName)
{
	pcharid = iCharID;
	strn0cpy(pname, iCharName, sizeof(pname));
}

void ClientListEntry::SetOnline(ZoneServer *iZS, CLE_Status iOnline)
{
	if (iZS == Server()) {
		SetOnline(iOnline);
	}
}

void ClientListEntry::SetOnline(CLE_Status iOnline)
{
	LogClientLogin(
		"ClientListEntry::SetOnline for [{}] ({}) = [{}] ({})",
		AccountName(),
		AccountID(),
		CLEStatusString[CLE_Status::Online],
		iOnline
	);

	if (iOnline >= CLE_Status::Online && pOnline < CLE_Status::Online) {
		numplayers++;
	}
	else if (iOnline < CLE_Status::Online && pOnline >= CLE_Status::Online) {
		numplayers--;
	}
	if (iOnline != CLE_Status::Online || pOnline < CLE_Status::Online) {
		pOnline = iOnline;
	}
	if (iOnline < CLE_Status::Zoning) {
		Camp();
	}
	if (pOnline >= CLE_Status::Online) {
		stale = 0;
	}
}

void ClientListEntry::LSUpdate(ZoneServer *iZS)
{
	if (WorldConfig::get()->UpdateStats) {
		auto pack = new ServerPacket;
		pack->opcode  = ServerOP_LSZoneInfo;
		pack->size    = sizeof(ZoneInfo_Struct);
		pack->pBuffer = new uchar[pack->size];
		ZoneInfo_Struct *zone = (ZoneInfo_Struct *) pack->pBuffer;
		zone->count    = iZS->NumPlayers();
		zone->zone     = iZS->GetZoneID();
		zone->zone_wid = iZS->GetID();
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}
void ClientListEntry::LSZoneChange(ZoneToZone_Struct *ztz)
{
	if (WorldConfig::get()->UpdateStats) {
		auto pack = new ServerPacket;
		pack->opcode  = ServerOP_LSPlayerZoneChange;
		pack->size    = sizeof(ServerLSPlayerZoneChange_Struct);
		pack->pBuffer = new uchar[pack->size];
		ServerLSPlayerZoneChange_Struct *zonechange = (ServerLSPlayerZoneChange_Struct *) pack->pBuffer;
		zonechange->lsaccount_id = LSID();
		zonechange->from         = ztz->current_zone_id;
		zonechange->to           = ztz->requested_zone_id;
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}

void ClientListEntry::Update(ZoneServer *iZS, ServerClientList_Struct *scl, CLE_Status iOnline)
{
	if (pzoneserver != iZS) {
		if (pzoneserver) {
			pzoneserver->RemovePlayer();
			LSUpdate(pzoneserver);
		}
		if (iZS) {
			iZS->AddPlayer();
			LSUpdate(iZS);
		}
	}
	pzoneserver    = iZS;
	pzone          = scl->zone;
	pinstance      = scl->instance_id;
	pcharid        = scl->charid;

	strcpy(pname, scl->name);
	if (paccountid == 0) {
		paccountid = scl->AccountID;
		strcpy(paccountname, scl->AccountName);
		strcpy(loginserver_account_name, scl->AccountName);
		pIP   = scl->IP;
		pLSID = scl->LSAccountID;
		strn0cpy(plskey, scl->lskey, sizeof(plskey));
	}
	padmin         = scl->Admin;
	plevel         = scl->level;
	pclass_        = scl->class_;
	prace          = scl->race;
	panon          = scl->anon;
	ptellsoff      = scl->tellsoff;
	pguild_id      = scl->guild_id;
	pLFG           = scl->LFG;
	gm             = scl->gm;
	pClientVersion = scl->ClientVersion;

	// Fields from the LFG Window
	if ((scl->LFGFromLevel != 0) && (scl->LFGToLevel != 0)) {
		pLFGFromLevel   = scl->LFGFromLevel;
		pLFGToLevel     = scl->LFGToLevel;
		pLFGMatchFilter = scl->LFGMatchFilter;
		memcpy(pLFGComments, scl->LFGComments, sizeof(pLFGComments));
	}

	SetOnline(iOnline);
}

void ClientListEntry::LeavingZone(ZoneServer *iZS, CLE_Status iOnline)
{
	if (iZS != 0 && iZS != pzoneserver) {
		return;
	}
	SetOnline(iOnline);

	shared_task_manager.RemoveActiveInvitationByCharacterID(CharID());

	if (pzoneserver) {
		pzoneserver->RemovePlayer();
		LSUpdate(pzoneserver);
	}
	pzoneserver = 0;
	pzone       = 0;
}

void ClientListEntry::ClearVars(bool iAll)
{
	if (iAll) {
		pOnline = CLE_Status::Never;
		stale   = 0;

		pLSID = 0;
		memset(loginserver_account_name, 0, sizeof(loginserver_account_name));
		memset(plskey, 0, sizeof(plskey));
		pworldadmin = 0;

		paccountid = 0;
		memset(paccountname, 0, sizeof(paccountname));
		padmin = AccountStatus::Player;
	}
	pzoneserver = 0;
	pzone       = 0;
	pcharid     = 0;
	memset(pname, 0, sizeof(pname));
	plevel         = 0;
	pclass_        = 0;
	prace          = 0;
	panon          = 0;
	ptellsoff      = 0;
	pguild_id      = GUILD_NONE;
	pLFG           = 0;
	gm             = 0;
	pClientVersion = 0;
	for (auto &elem : tell_queue)
		safe_delete_array(elem);
	tell_queue.clear();
}

void ClientListEntry::Camp(ZoneServer *iZS)
{
	if (iZS != 0 && iZS != pzoneserver) {
		return;
	}
	if (pzoneserver) {
		pzoneserver->RemovePlayer();
		LSUpdate(pzoneserver);
	}

	ClearVars();

	stale = 0;
}

bool ClientListEntry::CheckStale()
{
	stale++;
	if (stale > 20) {
		if (pOnline > CLE_Status::Offline) {
			SetOnline(CLE_Status::Offline);
		}

		return true;
	}
	return false;
}

bool ClientListEntry::CheckAuth(uint32 loginserver_account_id, const char *key_password)
{
	LogDebug(
		"ClientListEntry::CheckAuth ls_account_id [{0}] key_password [{1}] plskey [{2}]",
		loginserver_account_id,
		key_password,
		plskey
	);
	if (pLSID == loginserver_account_id && strncmp(plskey, key_password, 10) == 0) {

		LogDebug(
			"ClientListEntry::CheckAuth ls_account_id [{0}] key_password [{1}] plskey [{2}] lsid [{3}] paccountid [{4}]",
			loginserver_account_id,
			key_password,
			plskey,
			LSID(),
			paccountid
		);

		if (paccountid == 0 && LSID() > 0) {
			int16 default_account_status = WorldConfig::get()->DefaultStatus;

			paccountid = database.CreateAccount(
				loginserver_account_name,
				0,
				default_account_status,
				source_loginserver,
				LSID()
			);

			if (!paccountid) {
				LogInfo(
					"Error adding local account for LS login: [{0}:{1}], duplicate name",
					source_loginserver,
					loginserver_account_name
				);
				return false;
			}
			strn0cpy(paccountname, loginserver_account_name, sizeof(paccountname));
			padmin = default_account_status;
		}
		std::string lsworldadmin;
		if (database.GetVariable("honorlsworldadmin", lsworldadmin)) {
			if (atoi(lsworldadmin.c_str()) == 1 && pworldadmin != 0 && (padmin < pworldadmin || padmin == AccountStatus::Player)) {
				padmin = pworldadmin;
			}
		}
		return true;
	}
	return false;
}

void ClientListEntry::ProcessTellQueue()
{
	if (!Server()) {
		return;
	}

	ServerPacket *pack;
	auto         it = tell_queue.begin();
	while (it != tell_queue.end()) {
		pack = new ServerPacket(
			ServerOP_ChannelMessage,
			sizeof(ServerChannelMessage_Struct) + strlen((*it)->message) + 1
		);
		memcpy(pack->pBuffer, *it, pack->size);
		Server()->SendPacket(pack);
		safe_delete(pack);
		safe_delete_array(*it);
		it = tell_queue.erase(it);
	}
	return;
}

