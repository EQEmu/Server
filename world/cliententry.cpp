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
#include "worlddb.h"
#include "zoneserver.h"
#include "world_config.h"
#include "../common/guilds.h"
#include "../common/string_util.h"

extern uint32 numplayers;
extern LoginServerList loginserverlist;
extern ClientList		client_list;
extern volatile bool RunLoops;

ClientListEntry::ClientListEntry(uint32 in_id, uint32 iLSID, const char* iLoginName, const char* iLoginKey, int16 iWorldAdmin, uint32 ip, uint8 local)
: id(in_id)
{
	ClearVars(true);

	pIP = ip;
	pLSID = iLSID;
	if(iLSID > 0)
		paccountid = database.GetAccountIDFromLSID(iLSID, paccountname, &padmin);
	strn0cpy(plsname, iLoginName, sizeof(plsname));
	strn0cpy(plskey, iLoginKey, sizeof(plskey));
	pworldadmin = iWorldAdmin;
	plocal=(local==1);

	pinstance = 0;
}

ClientListEntry::ClientListEntry(uint32 in_id, uint32 iAccID, const char* iAccName, MD5& iMD5Pass, int16 iAdmin)
: id(in_id)
{
	ClearVars(true);

	pIP = 0;
	pLSID = 0;
	pworldadmin = 0;

	paccountid = iAccID;
	strn0cpy(paccountname, iAccName, sizeof(paccountname));
	pMD5Pass = iMD5Pass;
	padmin = iAdmin;

	pinstance = 0;
}

ClientListEntry::ClientListEntry(uint32 in_id, ZoneServer* iZS, ServerClientList_Struct* scl, int8 iOnline)
: id(in_id)
{
	ClearVars(true);

	pIP = 0;
	pLSID = scl->LSAccountID;
	strn0cpy(plsname, scl->name, sizeof(plsname));
	strn0cpy(plskey, scl->lskey, sizeof(plskey));
	pworldadmin = 0;

	paccountid = scl->AccountID;
	strn0cpy(paccountname, scl->AccountName, sizeof(paccountname));
	padmin = scl->Admin;

	pinstance = 0;

	if (iOnline >= CLE_Status_Zoning)
		Update(iZS, scl, iOnline);
	else
		SetOnline(iOnline);
}

ClientListEntry::~ClientListEntry() {
	if (RunLoops) {
		Camp(); // updates zoneserver's numplayers
		client_list.RemoveCLEReferances(this);
	}
	for (auto &elem : tell_queue)
		safe_delete_array(elem);
	tell_queue.clear();
}

void ClientListEntry::SetChar(uint32 iCharID, const char* iCharName) {
	pcharid = iCharID;
	strn0cpy(pname, iCharName, sizeof(pname));
}

void ClientListEntry::SetOnline(ZoneServer* iZS, int8 iOnline) {
	if (iZS == this->Server())
		SetOnline(iOnline);
}

void ClientListEntry::SetOnline(int8 iOnline) {
	if (iOnline >= CLE_Status_Online && pOnline < CLE_Status_Online)
		numplayers++;
	else if (iOnline < CLE_Status_Online && pOnline >= CLE_Status_Online) {
		numplayers--;
	}
	if (iOnline != CLE_Status_Online || pOnline < CLE_Status_Online)
		pOnline = iOnline;
	if (iOnline < CLE_Status_Zoning)
		Camp();
	if (pOnline >= CLE_Status_Online)
		stale = 0;
}
void ClientListEntry::LSUpdate(ZoneServer* iZS){
	if(WorldConfig::get()->UpdateStats){
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_LSZoneInfo;
		pack->size = sizeof(ZoneInfo_Struct);
		pack->pBuffer = new uchar[pack->size];
		ZoneInfo_Struct* zone =(ZoneInfo_Struct*)pack->pBuffer;
		zone->count=iZS->NumPlayers();
		zone->zone = iZS->GetZoneID();
		zone->zone_wid = iZS->GetID();
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}
void ClientListEntry::LSZoneChange(ZoneToZone_Struct* ztz){
	if(WorldConfig::get()->UpdateStats){
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_LSPlayerZoneChange;
		pack->size = sizeof(ServerLSPlayerZoneChange_Struct);
		pack->pBuffer = new uchar[pack->size];
		ServerLSPlayerZoneChange_Struct* zonechange =(ServerLSPlayerZoneChange_Struct*)pack->pBuffer;
		zonechange->lsaccount_id = LSID();
		zonechange->from = ztz->current_zone_id;
		zonechange->to = ztz->requested_zone_id;
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}
void ClientListEntry::Update(ZoneServer* iZS, ServerClientList_Struct* scl, int8 iOnline) {
	if (pzoneserver != iZS) {
		if (pzoneserver){
			pzoneserver->RemovePlayer();
			LSUpdate(pzoneserver);
		}
		if (iZS){
			iZS->AddPlayer();
			LSUpdate(iZS);
		}
	}
	pzoneserver = iZS;
	pzone = scl->zone;
	pinstance = scl->instance_id;
	pcharid = scl->charid;

	strcpy(pname, scl->name);
	if (paccountid == 0) {
		paccountid = scl->AccountID;
		strcpy(paccountname, scl->AccountName);
		strcpy(plsname, scl->AccountName);
		pIP = scl->IP;
		pLSID = scl->LSAccountID;
		strn0cpy(plskey, scl->lskey, sizeof(plskey));
	}
	padmin = scl->Admin;
	plevel = scl->level;
	pclass_ = scl->class_;
	prace = scl->race;
	panon = scl->anon;
	ptellsoff = scl->tellsoff;
	pguild_id = scl->guild_id;
	pLFG = scl->LFG;
	gm = scl->gm;
	pClientVersion = scl->ClientVersion;

	// Fields from the LFG Window
	if((scl->LFGFromLevel != 0) && (scl->LFGToLevel != 0)) {
		pLFGFromLevel = scl->LFGFromLevel;
		pLFGToLevel = scl->LFGToLevel;
		pLFGMatchFilter = scl->LFGMatchFilter;
		memcpy(pLFGComments, scl->LFGComments, sizeof(pLFGComments));
	}

	SetOnline(iOnline);
}

void ClientListEntry::LeavingZone(ZoneServer* iZS, int8 iOnline) {
	if (iZS != 0 && iZS != pzoneserver)
		return;
	SetOnline(iOnline);

	if (pzoneserver){
		pzoneserver->RemovePlayer();
		LSUpdate(pzoneserver);
	}
	pzoneserver = 0;
	pzone = 0;
}

void ClientListEntry::ClearVars(bool iAll) {
	if (iAll) {
		pOnline = CLE_Status_Never;
		stale = 0;

		pLSID = 0;
		memset(plsname, 0, sizeof(plsname));
		memset(plskey, 0, sizeof(plskey));
		pworldadmin = 0;

		paccountid = 0;
		memset(paccountname, 0, sizeof(paccountname));
		padmin = 0;
	}
	pzoneserver = 0;
	pzone = 0;
	pcharid = 0;
	memset(pname, 0, sizeof(pname));
	plevel = 0;
	pclass_ = 0;
	prace = 0;
	panon = 0;
	ptellsoff = 0;
	pguild_id = GUILD_NONE;
	pLFG = 0;
	gm = 0;
	pClientVersion = 0;
	for (auto &elem : tell_queue)
		safe_delete_array(elem);
	tell_queue.clear();
}

void ClientListEntry::Camp(ZoneServer* iZS) {
	if (iZS != 0 && iZS != pzoneserver)
		return;
	if (pzoneserver){
		pzoneserver->RemovePlayer();
		LSUpdate(pzoneserver);
	}

	ClearVars();

	stale = 0;
}

bool ClientListEntry::CheckStale() {
	stale++;
	if (stale > 20) {
		if (pOnline > CLE_Status_Offline)
			SetOnline(CLE_Status_Offline);
		else
			return true;
	}
	return false;
}

bool ClientListEntry::CheckAuth(uint32 iLSID, const char* iKey) {
	if (strncmp(plskey, iKey,10) == 0) {
		if (paccountid == 0 && LSID()>0) {
			int16 tmpStatus = WorldConfig::get()->DefaultStatus;
			paccountid = database.CreateAccount(plsname, 0, tmpStatus, LSID());
			if (!paccountid) {
				Log.Out(Logs::Detail, Logs::World_Server,"Error adding local account for LS login: '%s', duplicate name?" ,plsname);
				return false;
			}
			strn0cpy(paccountname, plsname, sizeof(paccountname));
			padmin = tmpStatus;
		}
		std::string lsworldadmin;
		if (database.GetVariable("honorlsworldadmin", lsworldadmin))
			if (atoi(lsworldadmin.c_str()) == 1 && pworldadmin != 0 && (padmin < pworldadmin || padmin == 0))
				padmin = pworldadmin;
		return true;
	}
	return false;
}

bool ClientListEntry::CheckAuth(const char* iName, MD5& iMD5Password) {
	if (LSAccountID() == 0 && strcmp(paccountname, iName) == 0 && pMD5Pass == iMD5Password)
		return true;
	return false;
}

bool ClientListEntry::CheckAuth(uint32 id, const char* iKey, uint32 ip) {
	if (pIP==ip && strncmp(plskey, iKey,10) == 0){
		paccountid = id;
		database.GetAccountFromID(id,paccountname,&padmin);
		return true;
	}
	return false;
}

void ClientListEntry::ProcessTellQueue()
{
	if (!Server())
		return;

	ServerPacket *pack;
	auto it = tell_queue.begin();
	while (it != tell_queue.end()) {
		pack = new ServerPacket(ServerOP_ChannelMessage, sizeof(ServerChannelMessage_Struct) + strlen((*it)->message) + 1);
		memcpy(pack->pBuffer, *it, pack->size);
		pack->Deflate();
		Server()->SendPacket(pack);
		safe_delete(pack);
		safe_delete_array(*it);
		it = tell_queue.erase(it);
	}
	return;
}

