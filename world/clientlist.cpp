/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
#include "clientlist.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "client.h"
#include "worlddb.h"
#include "../common/string_util.h"
#include "../common/guilds.h"
#include "../common/races.h"
#include "../common/classes.h"
#include "../common/packet_dump.h"
#include "../common/misc.h"
#include "../common/misc_functions.h"
#include "../common/json/json.h"
#include "../common/event_sub.h"
#include "web_interface.h"
#include "wguild_mgr.h"
#include <set>

extern WebInterfaceList web_interface;

extern ZSList			zoneserver_list;
uint32 numplayers = 0;	//this really wants to be a member variable of ClientList...

ClientList::ClientList()
: CLStale_timer(7500)
{
	NextCLEID = 1;

	m_tick.reset(new EQ::Timer(5000, true, std::bind(&ClientList::OnTick, this, std::placeholders::_1)));
}

ClientList::~ClientList() {
}

void ClientList::Process() {

	if (CLStale_timer.Check())
		CLCheckStale();

	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (!iterator.GetData()->Process()) {
			struct in_addr in;
			in.s_addr = iterator.GetData()->GetIP();
			Log(Logs::Detail, Logs::World_Server,"Removing client from %s:%d", inet_ntoa(in), iterator.GetData()->GetPort());
//the client destructor should take care of this.
//			iterator.GetData()->Free();
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
}

void ClientList::CLERemoveZSRef(ZoneServer* iZS) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->Server() == iZS) {
			iterator.GetData()->ClearServer(); // calling this before LeavingZone() makes CLE not update the number of players in a zone
			iterator.GetData()->LeavingZone();
		}
		iterator.Advance();
	}
}

ClientListEntry* ClientList::GetCLE(uint32 iID) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == iID) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

//Account Limiting Code to limit the number of characters allowed on from a single account at once.
void ClientList::EnforceSessionLimit(uint32 iLSAccountID) {

	ClientListEntry* ClientEntry = 0;

	LinkedListIterator<ClientListEntry*> iterator(clientlist, BACKWARD);

	int CharacterCount = 0;

	iterator.Reset();

	while(iterator.MoreElements()) {

		ClientEntry = iterator.GetData();

		if ((ClientEntry->LSAccountID() == iLSAccountID) &&
			ClientEntry->Online() >= 1 &&
			((ClientEntry->Admin() <= (RuleI(World, ExemptAccountLimitStatus))) || (RuleI(World, ExemptAccountLimitStatus) < 0))) {

			CharacterCount++;
		}
		else if ((ClientEntry->LSAccountID() == iLSAccountID) &&
			ClientEntry->Online() <= 0)
		{
			ClientEntry->SetOnline(CLE_Status_Offline); // Remove the connection
			iterator.RemoveCurrent();
			continue;
		}

		iterator.Advance();
	}
}

bool ClientList::CheckSessionLimit(uint32 iLSAccountID) {

	ClientListEntry* ClientEntry = 0;

	LinkedListIterator<ClientListEntry*> iterator(clientlist, BACKWARD);

	int CharacterCount = 0;

	iterator.Reset();

	while (iterator.MoreElements()) {

		ClientEntry = iterator.GetData();

		if ((ClientEntry->LSAccountID() == iLSAccountID) &&
			ClientEntry->Online() >= 4 &&
			((ClientEntry->Admin() <= (RuleI(World, ExemptAccountLimitStatus))) || (RuleI(World, ExemptAccountLimitStatus) < 0))) {

			CharacterCount++;
		}
		else if ((ClientEntry->LSAccountID() == iLSAccountID) &&
			ClientEntry->Online() <= 0 || 
			(ClientEntry->LSAccountID() == iLSAccountID) &&
			ClientEntry->Online() == 3 ||
			(ClientEntry->LSAccountID() == iLSAccountID) &&
			ClientEntry->Online() == 2 ||
			(ClientEntry->LSAccountID() == iLSAccountID) &&
			ClientEntry->Online() == 1

			)
		{

			if (strlen(ClientEntry->name())) {
				auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
				ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*)pack->pBuffer;
				strcpy(skp->adminname, "SessionLimit");
				strcpy(skp->name, ClientEntry->name());
				skp->adminrank = 255;
				zoneserver_list.SendPacket(pack);
				safe_delete(pack);
			}
			ClientEntry->SetOnline(CLE_Status_Offline); // Remove the connection
			iterator.RemoveCurrent();
			continue;
		}
		iterator.Advance();
	}


	if (CharacterCount >= (RuleI(World, AccountSessionLimit))) {
		return false;
	}

	return true;
}


bool ClientList::CheckCLEIP(uint32 iIP) {
	ClientListEntry* countCLEIPs = 0;
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	int IPInstances = 0;
	bool bAllowConnection = false;

	iterator.Reset();

	while (iterator.MoreElements()) {
		countCLEIPs = iterator.GetData();

		if ((countCLEIPs->GetIP() == iIP) &&
			countCLEIPs->Online() >= 1 &&
			((countCLEIPs->Admin() < (RuleI(World, ExemptMaxClientsStatus))) || (RuleI(World, ExemptMaxClientsStatus) < 0))) {
			IPInstances++;
		}
		else if ((countCLEIPs->GetIP() == iIP) &&
			countCLEIPs->Online() <= 0 &&
			((countCLEIPs->Admin() < (RuleI(World, ExemptMaxClientsStatus))) || (RuleI(World, ExemptMaxClientsStatus) < 0)))
		{

			if (strlen(countCLEIPs->name())) {
				auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
				ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*)pack->pBuffer;
				strcpy(skp->adminname, "SessionLimit");
				strcpy(skp->name, countCLEIPs->name());
				skp->adminrank = 255;
				zoneserver_list.SendPacket(pack);
				safe_delete(pack);
				}

			countCLEIPs->SetOnline(CLE_Status_Offline); // Remove the connection
			iterator.RemoveCurrent();
			continue;
			}
		iterator.Advance();
						}

	if (IPInstances >= database.GetIPExemption(long2ip(iIP).c_str())) {
							return false;
					}

	return true;
}

//Check current CLE Entry IPs against incoming connection

void ClientList::GetCLEIP(uint32 iIP) {

}

uint32 ClientList::GetCLEIPCount(uint32 iIP) {
	ClientListEntry* countCLEIPs = 0;
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	int IPInstances = 0;
	iterator.Reset();

	while (iterator.MoreElements()) {
		countCLEIPs = iterator.GetData();
		if ((countCLEIPs->GetIP() == iIP) && ((countCLEIPs->Admin() < (RuleI(World, ExemptMaxClientsStatus))) || (RuleI(World, ExemptMaxClientsStatus) < 0)) && countCLEIPs->Online() >= CLE_Status_Online) { // If the IP matches, and the connection admin status is below the exempt status, or exempt status is less than 0 (no-one is exempt)
			IPInstances++; // Increment the occurences of this IP address
		}
		iterator.Advance();
	}

	return IPInstances;
}

void ClientList::DisconnectByIP(uint32 iIP) {
	ClientListEntry* countCLEIPs = 0;
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	iterator.Reset();

	while(iterator.MoreElements()) {
		countCLEIPs = iterator.GetData();
		if ((countCLEIPs->GetIP() == iIP)) {
			if(strlen(countCLEIPs->name())) {
				auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
				ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*) pack->pBuffer;
				strcpy(skp->adminname, "SessionLimit");
				strcpy(skp->name, countCLEIPs->name());
				skp->adminrank = 255;
				zoneserver_list.SendPacket(pack);
				safe_delete(pack);
			}
			countCLEIPs->SetOnline(CLE_Status_Offline);
			iterator.RemoveCurrent();
		}
		iterator.Advance();
	}
}


void ClientList::DisconnectCLE(ClientListEntry* entry) {

	if (!entry)
		return;

	ClientListEntry* countCLEIPs = 0;
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	iterator.Reset();

	while (iterator.MoreElements()) {
		countCLEIPs = iterator.GetData();
		if ((countCLEIPs == entry)) {
			if (strlen(countCLEIPs->name())) {
				auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
				ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*)pack->pBuffer;
				strcpy(skp->adminname, "DisconnectCLE");
				strcpy(skp->name, countCLEIPs->name());
				skp->adminrank = 255;
				zoneserver_list.SendPacket(pack);
				safe_delete(pack);
			}
			countCLEIPs->SetOnline(CLE_Status_Offline);
			iterator.RemoveCurrent();
		}
		iterator.Advance();
	}
}


ClientListEntry* ClientList::FindCharacter(const char* name) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (strcasecmp(iterator.GetData()->name(), name) == 0) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

ClientListEntry* ClientList::FindCLEByAccountID(uint32 iAccID) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->AccountID() == iAccID) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

ClientListEntry* ClientList::FindCLEByCharacterID(uint32 iCharID) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CharID() == iCharID) {
			return iterator.GetData();
		}
		iterator.Advance();
	}
	return 0;
}

void ClientList::SendCLEList(const int16& admin, const char* to, WorldTCPConnection* connection, const char* iName) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	char* output = 0;
	uint32 outsize = 0, outlen = 0;
	int x = 0, y = 0;
	int namestrlen = iName == 0 ? 0 : strlen(iName);
	bool addnewline = false;
	char newline[3];
	if (connection->IsConsole())
		strcpy(newline, "\r\n");
	else
		strcpy(newline, "^");

	iterator.Reset();
	while(iterator.MoreElements()) {
		ClientListEntry* cle = iterator.GetData();
		if (admin >= cle->Admin() && (iName == 0 || namestrlen == 0 || strncasecmp(cle->name(), iName, namestrlen) == 0 || strncasecmp(cle->AccountName(), iName, namestrlen) == 0 || strncasecmp(cle->LSName(), iName, namestrlen) == 0)) {
			struct in_addr in;
			in.s_addr = cle->GetIP();
			if (addnewline) {
				AppendAnyLenString(&output, &outsize, &outlen, newline);
			}
			AppendAnyLenString(&output, &outsize, &outlen, "ID: %i  Acc# %i  AccName: %s  IP: %s", cle->GetID(), cle->AccountID(), cle->AccountName(), inet_ntoa(in));
			AppendAnyLenString(&output, &outsize, &outlen, "%s  Stale: %i  Online: %i  Admin: %i", newline, cle->GetStaleCounter(), cle->Online(), cle->Admin());
			if (cle->LSID())
				AppendAnyLenString(&output, &outsize, &outlen, "%s  LSID: %i  LSName: %s  WorldAdmin: %i", newline, cle->LSID(), cle->LSName(), cle->WorldAdmin());
			if (cle->CharID())
				AppendAnyLenString(&output, &outsize, &outlen, "%s  CharID: %i  CharName: %s  Zone: %s (%i)", newline, cle->CharID(), cle->name(), database.GetZoneName(cle->zone()), cle->zone());
			if (outlen >= 3072) {
				connection->SendEmoteMessageRaw(to, 0, 0, 10, output);
				safe_delete(output);
				outsize = 0;
				outlen = 0;
				addnewline = false;
			}
			else
				addnewline = true;
			y++;
		}
		iterator.Advance();
		x++;
	}
	AppendAnyLenString(&output, &outsize, &outlen, "%s%i CLEs in memory. %i CLEs listed. numplayers = %i.", newline, x, y, numplayers);
	connection->SendEmoteMessageRaw(to, 0, 0, 10, output);
	safe_delete(output);
}


void ClientList::CLEAdd(uint32 iLSID, const char* iLoginName, const char* iLoginKey, int16 iWorldAdmin, uint32 ip, uint8 local) {
	auto tmp = new ClientListEntry(GetNextCLEID(), iLSID, iLoginName, iLoginKey, iWorldAdmin, ip, local);

	clientlist.Append(tmp);
}

void ClientList::CLCheckStale() {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckStale()) {
			iterator.RemoveCurrent();
		}
		else
			iterator.Advance();
	}
}

void ClientList::ClientUpdate(ZoneServer* zoneserver, ServerClientList_Struct* scl) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	ClientListEntry* cle;
	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetID() == scl->wid) {
			cle = iterator.GetData();
			if (scl->remove == 2){
				cle->LeavingZone(zoneserver, CLE_Status_Offline);
			}
			else if (scl->remove == 1)
				cle->LeavingZone(zoneserver, CLE_Status_Zoning);
			else
				cle->Update(zoneserver, scl);
			return;
		}
		iterator.Advance();
	}
	if (scl->remove == 2)
		cle = new ClientListEntry(GetNextCLEID(), zoneserver, scl, CLE_Status_Online);
	else if (scl->remove == 1)
		cle = new ClientListEntry(GetNextCLEID(), zoneserver, scl, CLE_Status_Zoning);
	else
		cle = new ClientListEntry(GetNextCLEID(), zoneserver, scl, CLE_Status_InZone);
	clientlist.Insert(cle);
	zoneserver->ChangeWID(scl->charid, cle->GetID());
}

void ClientList::CLEKeepAlive(uint32 numupdates, uint32* wid) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	uint32 i;

	iterator.Reset();
	while(iterator.MoreElements()) {
		for (i=0; i<numupdates; i++) {
			if (wid[i] == iterator.GetData()->GetID())
				iterator.GetData()->KeepAlive();
		}
		iterator.Advance();
	}
}


ClientListEntry* ClientList::CheckAuth(uint32 id, const char* iKey, uint32 ip ) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckAuth(id, iKey, ip))
			return iterator.GetData();
		iterator.Advance();
	}
	return 0;
}
ClientListEntry* ClientList::CheckAuth(uint32 iLSID, const char* iKey) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckAuth(iLSID, iKey))
			return iterator.GetData();
		iterator.Advance();
	}
	return 0;
}

ClientListEntry* ClientList::CheckAuth(const char* iName, const char* iPassword) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	MD5 tmpMD5(iPassword);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->CheckAuth(iName, tmpMD5))
			return iterator.GetData();
		iterator.Advance();
	}
	int16 tmpadmin;

	//Log.LogDebugType(Logs::Detail, Logs::World_Server,"Login with '%s' and '%s'", iName, iPassword);

	uint32 accid = database.CheckLogin(iName, iPassword, &tmpadmin);
	if (accid) {
		uint32 lsid = 0;
		database.GetAccountIDByName(iName, &tmpadmin, &lsid);
		auto tmp = new ClientListEntry(GetNextCLEID(), lsid, iName, tmpMD5, tmpadmin, 0, 0);
		clientlist.Append(tmp);
		return tmp;
	}
	return 0;
}

void ClientList::SendOnlineGuildMembers(uint32 FromID, uint32 GuildID)
{
	int PacketLength = 8;

	uint32 Count = 0;
	ClientListEntry* from = this->FindCLEByCharacterID(FromID);

	if(!from)
	{
		Log(Logs::Detail, Logs::World_Server,"Invalid client. FromID=%i GuildID=%i", FromID, GuildID);
		return;
	}

	LinkedListIterator<ClientListEntry*> Iterator(clientlist);

	Iterator.Reset();

	while(Iterator.MoreElements())
	{
		ClientListEntry* CLE = Iterator.GetData();

		if(CLE && (CLE->GuildID() == GuildID))
		{
			PacketLength += (strlen(CLE->name()) + 5);
			++Count;
		}

		Iterator.Advance();

	}

	Iterator.Reset();

	auto pack = new ServerPacket(ServerOP_OnlineGuildMembersResponse, PacketLength);

	char *Buffer = (char *)pack->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, FromID);
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, Count);

	while(Iterator.MoreElements())
	{
		ClientListEntry* CLE = Iterator.GetData();

		if(CLE && (CLE->GuildID() == GuildID))
		{
			VARSTRUCT_ENCODE_STRING(Buffer, CLE->name());
			VARSTRUCT_ENCODE_TYPE(uint32, Buffer, CLE->zone());
		}

		Iterator.Advance();
	}
	zoneserver_list.SendPacket(from->zone(), from->instance(), pack);
	safe_delete(pack);
}


void ClientList::SendWhoAll(uint32 fromid,const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection) {
	try{
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	LinkedListIterator<ClientListEntry*> countclients(clientlist);
	ClientListEntry* cle = 0;
	ClientListEntry* countcle = 0;
	//char tmpgm[25] = "";
	//char accinfo[150] = "";
	char line[300] = "";
	//char tmpguild[50] = "";
	//char LFG[10] = "";
	//uint32 x = 0;
	int whomlen = 0;
	if (whom) {
		// fixes for client converting some queries into a race query instead of zone
		if (whom->wrace == 221) {
			whom->wrace = 0xFFFF;
			strcpy(whom->whom, "scarlet");
		}
		if (whom->wrace == 327) {
			whom->wrace = 0xFFFF;
			strcpy(whom->whom, "crystal");
		}
		if (whom->wrace == 103) {
			whom->wrace = 0xFFFF;
			strcpy(whom->whom, "kedge");
		}
		if (whom->wrace == 230) {
			whom->wrace = 0xFFFF;
			strcpy(whom->whom, "akheva");
		}
		if (whom->wrace == 229) {
			whom->wrace = 0xFFFF;
			strcpy(whom->whom, "netherbian");
		}

		whomlen = strlen(whom->whom);
		if(whom->wrace == 0x001A) // 0x001A is the old Froglok race number and is sent by the client for /who all froglok
			whom->wrace = FROGLOK; // This is what EQEmu uses for the Froglok Race number.
	}

	char* output = 0;
	uint32 outsize = 0, outlen = 0;
	uint32 totalusers=0;
	uint32 totallength=0;
	AppendAnyLenString(&output, &outsize, &outlen, "Players on server:");
	if (connection->IsConsole())
		AppendAnyLenString(&output, &outsize, &outlen, "\r\n");
	else
		AppendAnyLenString(&output, &outsize, &outlen, "\n");
	countclients.Reset();
	while(countclients.MoreElements()){
		countcle = countclients.GetData();
		const char* tmpZone = database.GetZoneName(countcle->zone());
		if (
	(countcle->Online() >= CLE_Status_Zoning) &&
	(!countcle->GetGM() || countcle->Anon() != 1 || admin >= countcle->Admin()) &&
	(whom == 0 || (
		((countcle->Admin() >= 80 && countcle->GetGM()) || whom->gmlookup == 0xFFFF) &&
		(whom->lvllow == 0xFFFF || (countcle->level() >= whom->lvllow && countcle->level() <= whom->lvlhigh && (countcle->Anon()==0 || admin > countcle->Admin()))) &&
		(whom->wclass == 0xFFFF || (countcle->class_() == whom->wclass && (countcle->Anon()==0 || admin > countcle->Admin()))) &&
		(whom->wrace == 0xFFFF || (countcle->race() == whom->wrace && (countcle->Anon()==0 || admin > countcle->Admin()))) &&
		(whomlen == 0 || (
			(tmpZone != 0 && strncasecmp(tmpZone, whom->whom, whomlen) == 0) ||
			strncasecmp(countcle->name(),whom->whom, whomlen) == 0 ||
			(strncasecmp(guild_mgr.GetGuildName(countcle->GuildID()), whom->whom, whomlen) == 0) ||
			(admin >= 100 && strncasecmp(countcle->AccountName(), whom->whom, whomlen) == 0)
		))
	))
) {
			if((countcle->Anon()>0 && admin>=countcle->Admin() && admin>0) || countcle->Anon()==0 ){
				totalusers++;
				if(totalusers<=20 || admin>=100)
					totallength=totallength+strlen(countcle->name())+strlen(countcle->AccountName())+strlen(guild_mgr.GetGuildName(countcle->GuildID()))+5;
			}
			else if((countcle->Anon()>0 && admin<=countcle->Admin()) || (countcle->Anon()==0 && !countcle->GetGM())) {
				totalusers++;
				if(totalusers<=20 || admin>=100)
					totallength=totallength+strlen(countcle->name())+strlen(guild_mgr.GetGuildName(countcle->GuildID()))+5;
			}
		}
		countclients.Advance();
	}
	uint32 plid=fromid;
	uint32 playerineqstring=5001;
	const char line2[]="---------------------------";
	uint8 unknown35=0x0A;
	uint32 unknown36=0;
	uint32 playersinzonestring=5028;
	if(totalusers>20 && admin<100){
		totalusers=20;
		playersinzonestring=5033;
	}
	else if(totalusers>1)
		playersinzonestring=5036;
	uint32 unknown44[2];
	unknown44[0]=0;
	unknown44[1]=0;
	uint32 unknown52=totalusers;
	uint32 unknown56=1;
	auto pack2 = new ServerPacket(ServerOP_WhoAllReply, 64 + totallength + (49 * totalusers));
	memset(pack2->pBuffer,0,pack2->size);
	uchar *buffer=pack2->pBuffer;
	uchar *bufptr=buffer;
	//memset(buffer,0,pack2->size);
	memcpy(bufptr,&plid, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&playerineqstring, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&line2, strlen(line2));
	bufptr+=strlen(line2);
	memcpy(bufptr,&unknown35, sizeof(uint8));
	bufptr+=sizeof(uint8);
	memcpy(bufptr,&unknown36, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&playersinzonestring, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&unknown44[0], sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&unknown44[1], sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&unknown52, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&unknown56, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&totalusers, sizeof(uint32));
	bufptr+=sizeof(uint32);

	iterator.Reset();
	int idx=-1;
	while(iterator.MoreElements()) {
		cle = iterator.GetData();

		const char* tmpZone = database.GetZoneName(cle->zone());
		if (
	(cle->Online() >= CLE_Status_Zoning) &&
	(!cle->GetGM() || cle->Anon() != 1 || admin >= cle->Admin()) &&
	(whom == 0 || (
		((cle->Admin() >= 80 && cle->GetGM()) || whom->gmlookup == 0xFFFF) &&
		(whom->lvllow == 0xFFFF || (cle->level() >= whom->lvllow && cle->level() <= whom->lvlhigh && (cle->Anon()==0 || admin>cle->Admin()))) &&
		(whom->wclass == 0xFFFF || (cle->class_() == whom->wclass && (cle->Anon()==0 || admin>cle->Admin()))) &&
		(whom->wrace == 0xFFFF || (cle->race() == whom->wrace && (cle->Anon()==0 || admin>cle->Admin()))) &&
		(whomlen == 0 || (
			(tmpZone != 0 && strncasecmp(tmpZone, whom->whom, whomlen) == 0) ||
			strncasecmp(cle->name(),whom->whom, whomlen) == 0 ||
			(strncasecmp(guild_mgr.GetGuildName(cle->GuildID()), whom->whom, whomlen) == 0) ||
			(admin >= 100 && strncasecmp(cle->AccountName(), whom->whom, whomlen) == 0)
		))
	))
) {
			line[0] = 0;
			uint32 rankstring=0xFFFFFFFF;
				if((cle->Anon()==1 && cle->GetGM() && cle->Admin()>admin) || (idx>=20 && admin<100)){ //hide gms that are anon from lesser gms and normal players, cut off at 20
					rankstring=0;
					iterator.Advance();
					continue;
				} else if (cle->GetGM()) {
					if (cle->Admin() >=250)
						rankstring=5021;
					else if (cle->Admin() >= 200)
						rankstring=5020;
					else if (cle->Admin() >= 180)
						rankstring=5019;
					else if (cle->Admin() >= 170)
						rankstring=5018;
					else if (cle->Admin() >= 160)
						rankstring=5017;
					else if (cle->Admin() >= 150)
						rankstring=5016;
					else if (cle->Admin() >= 100)
						rankstring=5015;
					else if (cle->Admin() >= 95)
						rankstring=5014;
					else if (cle->Admin() >= 90)
						rankstring=5013;
					else if (cle->Admin() >= 85)
						rankstring=5012;
					else if (cle->Admin() >= 81)
						rankstring=5011;
					else if (cle->Admin() >= 80)
						rankstring=5010;
					else if (cle->Admin() >= 50)
						rankstring=5009;
					else if (cle->Admin() >= 20)
						rankstring=5008;
					else if (cle->Admin() >= 10)
						rankstring=5007;
				}
			idx++;
			char guildbuffer[67]={0};
			if (cle->GuildID() != GUILD_NONE && cle->GuildID()>0)
				sprintf(guildbuffer,"<%s>", guild_mgr.GetGuildName(cle->GuildID()));
			uint32 formatstring=5025;
			if(cle->Anon()==1 && (admin<cle->Admin() || admin==0))
				formatstring=5024;
			else if(cle->Anon()==1 && admin>=cle->Admin() && admin>0)
				formatstring=5022;
			else if(cle->Anon()==2 && (admin<cle->Admin() || admin==0))
				formatstring=5023;//display guild
			else if(cle->Anon()==2 && admin>=cle->Admin() && admin>0)
				formatstring=5022;//display everything

	//war* wars2 = (war*)pack2->pBuffer;

	uint32 plclass_=0;
	uint32 pllevel=0;
	uint32 pidstring=0xFFFFFFFF;//5003;
	uint32 plrace=0;
	uint32 zonestring=0xFFFFFFFF;
	uint32 plzone=0;
	uint32 unknown80[2];
	if(cle->Anon()==0 || (admin>=cle->Admin() && admin>0)){
		plclass_=cle->class_();
		pllevel=cle->level();
		if(admin>=100)
			pidstring=5003;
		plrace=cle->race();
		zonestring=5006;
		plzone=cle->zone();
	}


	if(admin>=cle->Admin() && admin>0)
		unknown80[0]=cle->Admin();
	else
		unknown80[0]=0xFFFFFFFF;
	unknown80[1]=0xFFFFFFFF;//1035


	//char plstatus[20]={0};
	//sprintf(plstatus, "Status %i",cle->Admin());
	char plname[64]={0};
	strcpy(plname,cle->name());

	char placcount[30]={0};
	if(admin>=cle->Admin() && admin>0)
		strcpy(placcount,cle->AccountName());

	memcpy(bufptr,&formatstring, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&pidstring, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&plname, strlen(plname)+1);
	bufptr+=strlen(plname)+1;
	memcpy(bufptr,&rankstring, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&guildbuffer, strlen(guildbuffer)+1);
	bufptr+=strlen(guildbuffer)+1;
	memcpy(bufptr,&unknown80[0], sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&unknown80[1], sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&zonestring, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&plzone, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&plclass_, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&pllevel, sizeof(uint32));
	bufptr+=sizeof(uint32);
	memcpy(bufptr,&plrace, sizeof(uint32));
	bufptr+=sizeof(uint32);
	uint32 ending=0;
	memcpy(bufptr,&placcount, strlen(placcount)+1);
	bufptr+=strlen(placcount)+1;
	ending=207;
	memcpy(bufptr,&ending, sizeof(uint32));
	bufptr+=sizeof(uint32);
		}
		iterator.Advance();
	}
	//zoneserver_list.SendPacket(pack2); // NO NO NO WHY WOULD YOU SEND IT TO EVERY ZONE SERVER?!?
	SendPacket(to,pack2);
	safe_delete(pack2);
	safe_delete_array(output);
	}
	catch(...){
		Log(Logs::Detail, Logs::World_Server,"Unknown error in world's SendWhoAll (probably mem error), ignoring...");
		return;
	}
}

void ClientList::SendFriendsWho(ServerFriendsWho_Struct *FriendsWho, WorldTCPConnection* connection) {

	std::vector<ClientListEntry*> FriendsCLEs;
	FriendsCLEs.reserve(100);

	char Friend_[65];

	char *FriendsPointer = FriendsWho->FriendsString;

	// FriendsString is a comma delimited list of names.

	char *Seperator = nullptr;

	Seperator = strchr(FriendsPointer, ',');
	if(!Seperator) Seperator = strchr(FriendsPointer, '\0');

	uint32 TotalLength=0;

	while(Seperator != nullptr) {

		if((Seperator - FriendsPointer) > 64) return;

		strncpy(Friend_, FriendsPointer, Seperator - FriendsPointer);
		Friend_[Seperator - FriendsPointer] = 0;

		ClientListEntry* CLE = FindCharacter(Friend_);
		if(CLE && CLE->name() && (CLE->Online() >= CLE_Status_Zoning) && !(CLE->GetGM() && CLE->Anon())) {
			FriendsCLEs.push_back(CLE);
			TotalLength += strlen(CLE->name());
			int GuildNameLength = strlen(guild_mgr.GetGuildName(CLE->GuildID()));
			if(GuildNameLength>0)
				TotalLength += (GuildNameLength + 2);
		}

		if(Seperator[0] == '\0') break;

		FriendsPointer = Seperator + 1;
		Seperator = strchr(FriendsPointer, ',');
		if(!Seperator) Seperator = strchr(FriendsPointer, '\0');
	}


	try{
		ClientListEntry* cle;
		int FriendsOnline = FriendsCLEs.size();
		int PacketLength = sizeof(WhoAllReturnStruct) + (47 * FriendsOnline) + TotalLength;
		auto pack2 = new ServerPacket(ServerOP_WhoAllReply, PacketLength);
		memset(pack2->pBuffer,0,pack2->size);
		uchar *buffer=pack2->pBuffer;
		uchar *bufptr=buffer;

		WhoAllReturnStruct *WARS = (WhoAllReturnStruct *)bufptr;

		WARS->id = FriendsWho->FromID;
		WARS->playerineqstring = 0xffffffff;
		strcpy(WARS->line, "");
		WARS->unknown35 = 0x0a;
		WARS->unknown36 = 0x00;

		if(FriendsCLEs.size() == 1)
			WARS->playersinzonestring = 5028; // 5028 There is %1 player in EverQuest.
		else
			WARS->playersinzonestring = 5036; // 5036 There are %1 players in EverQuest.

		WARS->unknown44[0] = 0;
		WARS->unknown44[1] = 0;
		WARS->unknown52 = FriendsOnline;
		WARS->unknown56 = 1;
		WARS->playercount = FriendsOnline;

		bufptr+=sizeof(WhoAllReturnStruct);

		for(int CLEEntry = 0; CLEEntry < FriendsOnline; CLEEntry++) {

			cle = FriendsCLEs[CLEEntry];

			char GuildName[67]={0};
			if (cle->GuildID() != GUILD_NONE && cle->GuildID()>0)
				sprintf(GuildName,"<%s>", guild_mgr.GetGuildName(cle->GuildID()));
			uint32 FormatMSGID=5025; // 5025 %T1[%2 %3] %4 (%5) %6 %7 %8 %9
			if(cle->Anon()==1)
				FormatMSGID=5024; // 5024 %T1[ANONYMOUS] %2 %3
			else if(cle->Anon()==2)
				FormatMSGID=5023; // 5023 %T1[ANONYMOUS] %2 %3 %4

			uint32 PlayerClass=0;
			uint32 PlayerLevel=0;
			uint32 PlayerRace=0;
			uint32 ZoneMSGID=0xffffffff;
			uint32 PlayerZone=0;

			if(cle->Anon()==0) {
				PlayerClass=cle->class_();
				PlayerLevel=cle->level();
				PlayerRace=cle->race();
				ZoneMSGID=5006; // 5006 ZONE: %1
				PlayerZone=cle->zone();
			}

			char PlayerName[64]={0};
			strcpy(PlayerName,cle->name());

			WhoAllPlayerPart1* WAPP1 = (WhoAllPlayerPart1*)bufptr;

			WAPP1->FormatMSGID = FormatMSGID;
			WAPP1->PIDMSGID = 0xffffffff;
			strcpy(WAPP1->Name, PlayerName);

			bufptr += sizeof(WhoAllPlayerPart1) + strlen(PlayerName);
			WhoAllPlayerPart2* WAPP2 = (WhoAllPlayerPart2*)bufptr;

			WAPP2->RankMSGID = 0xffffffff;
			strcpy(WAPP2->Guild, GuildName);

			bufptr += sizeof(WhoAllPlayerPart2) + strlen(GuildName);
			WhoAllPlayerPart3* WAPP3 = (WhoAllPlayerPart3*)bufptr;

			WAPP3->Unknown80[0] = 0xffffffff;
			WAPP3->Unknown80[1] = 0xffffffff;
			WAPP3->ZoneMSGID = ZoneMSGID;
			WAPP3->Zone = PlayerZone;
			WAPP3->Class_ = PlayerClass;
			WAPP3->Level = PlayerLevel;
			WAPP3->Race = PlayerRace;
			WAPP3->Account[0] = 0;

			bufptr += sizeof(WhoAllPlayerPart3);

			WhoAllPlayerPart4* WAPP4 = (WhoAllPlayerPart4*)bufptr;
			WAPP4->Unknown100 = 207;

			bufptr += sizeof(WhoAllPlayerPart4);

		}
		SendPacket(FriendsWho->FromName,pack2);
		safe_delete(pack2);
	}
	catch(...){
		Log(Logs::Detail, Logs::World_Server,"Unknown error in world's SendFriendsWho (probably mem error), ignoring...");
		return;
	}
}

void ClientList::SendLFGMatches(ServerLFGMatchesRequest_Struct *smrs) {

	// Send back matches when someone searches player's Looking For A Group.

	LinkedListIterator<ClientListEntry*> Iterator(clientlist);
	ClientListEntry* CLE = 0;
	int Matches = 0;

	Iterator.Reset();

	// We run the ClientList twice. The first time is to determine how big the outgoing packet needs to be.
	while(Iterator.MoreElements()) {
		CLE = Iterator.GetData();
		if(CLE->LFG()) {
			unsigned int BitMask = 1 << CLE->class_();
			// First we check that the player meets the level and class criteria of the person
			// doing the search.
			if((CLE->level() >= smrs->FromLevel) && (CLE->level() <= smrs->ToLevel) &&
				(BitMask & smrs->Classes))
				// Then we check if if the player doing the search meets the level criteria specified
				// by the player who is LFG.
				//
				// GetLFGMatchFilter returns the setting of the 'Only players who match my posted filters
				//						can query me' checkbox.
				//
				// FromLevel and ToLevel are the settings of the 'Want group levels:' boxes.
				if(!CLE->GetLFGMatchFilter() || ((smrs->QuerierLevel >= CLE->GetLFGFromLevel()) &&
								(smrs->QuerierLevel <= CLE->GetLFGToLevel())))
					Matches++;
		}
		Iterator.Advance();
	}
	auto Pack = new ServerPacket(ServerOP_LFGMatches, (sizeof(ServerLFGMatchesResponse_Struct) * Matches) + 4);

	char *Buf = (char *)Pack->pBuffer;
	// FromID is the Entity ID of the player doing the search.
	VARSTRUCT_ENCODE_TYPE(uint32, Buf, smrs->FromID);

	ServerLFGMatchesResponse_Struct* Buffer = (ServerLFGMatchesResponse_Struct*)Buf;

	Iterator.Reset();

	if(Matches) {
		while(Iterator.MoreElements() && (Matches > 0)) {
			CLE = Iterator.GetData();
			if(CLE->LFG()) {
				unsigned int BitMask = 1 << CLE->class_();
				if((CLE->level() >= smrs->FromLevel) && (CLE->level() <= smrs->ToLevel) &&
					(BitMask & smrs->Classes)) {
					Matches--;
					strcpy(Buffer->Name, CLE->name());
					Buffer->Class_ = CLE->class_();
					Buffer->Level = CLE->level();
					Buffer->Zone = CLE->zone();
					// If the LFG player is anon, level and class are still displayed, but
					// zone shows as UNAVAILABLE.
					Buffer->Anon = (CLE->Anon() != 0);
					// The client can filter on Guildname
					Buffer->GuildID = CLE->GuildID();
					strcpy(Buffer->Comments, CLE->GetLFGComments());
					Buffer++;
				}
			}
			Iterator.Advance();
		}
	}
	SendPacket(smrs->FromName,Pack);
	safe_delete(Pack);
}

void ClientList::ConsoleSendWhoAll(const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);
	ClientListEntry* cle = 0;
	char tmpgm[25] = "";
	char accinfo[150] = "";
	char line[300] = "";
	char tmpguild[50] = "";
	char LFG[10] = "";
	uint32 x = 0;
	int whomlen = 0;
	if (whom)
		whomlen = strlen(whom->whom);

	char* output = 0;
	uint32 outsize = 0, outlen = 0;
	AppendAnyLenString(&output, &outsize, &outlen, "Players on server:");
	if (connection->IsConsole())
		AppendAnyLenString(&output, &outsize, &outlen, "\r\n");
	else
		AppendAnyLenString(&output, &outsize, &outlen, "\n");
	iterator.Reset();
	while (iterator.MoreElements()) {
		cle = iterator.GetData();
		const char* tmpZone = database.GetZoneName(cle->zone());
		if (
			(cle->Online() >= CLE_Status_Zoning)
			&& (whom == 0 || (
				((cle->Admin() >= 80 && cle->GetGM()) || whom->gmlookup == 0xFFFF) &&
				(whom->lvllow == 0xFFFF || (cle->level() >= whom->lvllow && cle->level() <= whom->lvlhigh)) &&
				(whom->wclass == 0xFFFF || cle->class_() == whom->wclass) &&
				(whom->wrace == 0xFFFF || cle->race() == whom->wrace) &&
				(whomlen == 0 || (
					(tmpZone != 0 && strncasecmp(tmpZone, whom->whom, whomlen) == 0) ||
					strncasecmp(cle->name(), whom->whom, whomlen) == 0 ||
					(strncasecmp(guild_mgr.GetGuildName(cle->GuildID()), whom->whom, whomlen) == 0) ||
					(admin >= 100 && strncasecmp(cle->AccountName(), whom->whom, whomlen) == 0)
					))
				))
			) {
			line[0] = 0;
			// MYRA - use new (5.x) Status labels in who for telnet connection
			if (cle->Admin() >= 250)
				strcpy(tmpgm, "* GM-Impossible * ");
			else if (cle->Admin() >= 200)
				strcpy(tmpgm, "* GM-Mgmt * ");
			else if (cle->Admin() >= 180)
				strcpy(tmpgm, "* GM-Coder * ");
			else if (cle->Admin() >= 170)
				strcpy(tmpgm, "* GM-Areas * ");
			else if (cle->Admin() >= 160)
				strcpy(tmpgm, "* QuestMaster * ");
			else if (cle->Admin() >= 150)
				strcpy(tmpgm, "* GM-Lead Admin * ");
			else if (cle->Admin() >= 100)
				strcpy(tmpgm, "* GM-Admin * ");
			else if (cle->Admin() >= 95)
				strcpy(tmpgm, "* GM-Staff * ");
			else if (cle->Admin() >= 90)
				strcpy(tmpgm, "* EQ Support * ");
			else if (cle->Admin() >= 85)
				strcpy(tmpgm, "* GM-Tester * ");
			else if (cle->Admin() >= 81)
				strcpy(tmpgm, "* Senior Guide * ");
			else if (cle->Admin() >= 80)
				strcpy(tmpgm, "* QuestTroupe * ");
			else if (cle->Admin() >= 50)
				strcpy(tmpgm, "* Guide * ");
			else if (cle->Admin() >= 20)
				strcpy(tmpgm, "* Apprentice Guide * ");
			else if (cle->Admin() >= 10)
				strcpy(tmpgm, "* Steward * ");
			else
				tmpgm[0] = 0;
			// end Myra

			if (guild_mgr.GuildExists(cle->GuildID())) {
				snprintf(tmpguild, 36, " <%s>", guild_mgr.GetGuildName(cle->GuildID()));
			}
			else
				tmpguild[0] = 0;

			if (cle->LFG())
				strcpy(LFG, " LFG");
			else
				LFG[0] = 0;

			if (admin >= 150 && admin >= cle->Admin()) {
				sprintf(accinfo, " AccID: %i AccName: %s LSID: %i Status: %i", cle->AccountID(), cle->AccountName(), cle->LSAccountID(), cle->Admin());
			}
			else
				accinfo[0] = 0;

			if (cle->Anon() == 2) { // Roleplay
				if (admin >= 100 && admin >= cle->Admin())
					sprintf(line, "  %s[RolePlay %i %s] %s (%s)%s zone: %s%s%s", tmpgm, cle->level(), GetClassIDName(cle->class_(), cle->level()), cle->name(), GetRaceIDName(cle->race()), tmpguild, tmpZone, LFG, accinfo);
				else if (cle->Admin() >= 80 && admin < 80 && cle->GetGM()) {
					iterator.Advance();
					continue;
				}
				else
					sprintf(line, "  %s[ANONYMOUS] %s%s%s%s", tmpgm, cle->name(), tmpguild, LFG, accinfo);
			}
			else if (cle->Anon() == 1) { // Anon
				if (admin >= 100 && admin >= cle->Admin())
					sprintf(line, "  %s[ANON %i %s] %s (%s)%s zone: %s%s%s", tmpgm, cle->level(), GetClassIDName(cle->class_(), cle->level()), cle->name(), GetRaceIDName(cle->race()), tmpguild, tmpZone, LFG, accinfo);
				else if (cle->Admin() >= 80 && cle->GetGM()) {
					iterator.Advance();
					continue;
				}
				else
					sprintf(line, "  %s[ANONYMOUS] %s%s%s", tmpgm, cle->name(), LFG, accinfo);
			}
			else
				sprintf(line, "  %s[%i %s] %s (%s)%s zone: %s%s%s", tmpgm, cle->level(), GetClassIDName(cle->class_(), cle->level()), cle->name(), GetRaceIDName(cle->race()), tmpguild, tmpZone, LFG, accinfo);

			AppendAnyLenString(&output, &outsize, &outlen, line);
			if (outlen >= 3584) {
				connection->SendEmoteMessageRaw(to, 0, 0, 10, output);
				safe_delete(output);
				outsize = 0;
				outlen = 0;
			}
			else {
				if (connection->IsConsole())
					AppendAnyLenString(&output, &outsize, &outlen, "\r\n");
				else
					AppendAnyLenString(&output, &outsize, &outlen, "\n");
			}
			x++;
			if (x >= 20 && admin < 80)
				break;
		}
		iterator.Advance();
	}

	if (x >= 20 && admin < 80)
		AppendAnyLenString(&output, &outsize, &outlen, "too many results...20 players shown");
	else
		AppendAnyLenString(&output, &outsize, &outlen, "%i players online", x);
	if (admin >= 150 && (whom == 0 || whom->gmlookup != 0xFFFF)) {
		if (connection->IsConsole())
			AppendAnyLenString(&output, &outsize, &outlen, "\r\n");
		else
			AppendAnyLenString(&output, &outsize, &outlen, "\n");
		
		//console_list.SendConsoleWho(connection, to, admin, &output, &outsize, &outlen);
	}
	if (output)
		connection->SendEmoteMessageRaw(to, 0, 0, 10, output);
	safe_delete(output);
}

void ClientList::Add(Client* client) {
	list.Insert(client);
}

Client* ClientList::FindByAccountID(uint32 account_id) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		Log(Logs::Detail, Logs::World_Server, "ClientList[0x%08x]::FindByAccountID(%p) iterator.GetData()[%p]", this, account_id, iterator.GetData());
		if (iterator.GetData()->GetAccountID() == account_id) {
			Client* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

Client* ClientList::FindByName(char* charname) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		Log(Logs::Detail, Logs::World_Server, "ClientList[0x%08x]::FindByName(\"%s\") iterator.GetData()[%p]", this, charname, iterator.GetData());
		if (iterator.GetData()->GetCharName() == charname) {
			Client* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

Client* ClientList::Get(uint32 ip, uint16 port) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->GetIP() == ip && iterator.GetData()->GetPort() == port)
		{
			Client* tmp = iterator.GetData();
			return tmp;
		}
		iterator.Advance();
	}
	return 0;
}

void ClientList::ZoneBootup(ZoneServer* zs) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements())
	{
		if (iterator.GetData()->WaitingForBootup()) {
			if (iterator.GetData()->GetZoneID() == zs->GetZoneID()
				&& iterator.GetData()->GetInstanceID() == zs->GetInstanceID()) {
				iterator.GetData()->EnterWorld(false);
			}
			else if (iterator.GetData()->WaitingForBootup() == zs->GetID()) {
				iterator.GetData()->TellClientZoneUnavailable();
			}
		}
		iterator.Advance();
	}
}

void ClientList::RemoveCLEReferances(ClientListEntry* cle) {
	LinkedListIterator<Client*> iterator(list);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GetCLE() == cle) {
			iterator.GetData()->SetCLE(0);
		}
		iterator.Advance();
	}
}


bool ClientList::SendPacket(const char* to, ServerPacket* pack) {
	if (to == 0 || to[0] == 0) {
		zoneserver_list.SendPacket(pack);
		return true;
	}
	else if (to[0] == '*') {
		// Cant send a packet to a console....
		return false;
	}
	else {
		ClientListEntry* cle = FindCharacter(to);
		if (cle != nullptr) {
			if (cle->Server() != nullptr) {
				cle->Server()->SendPacket(pack);
				return true;
			}
			return false;
		} else {
			ZoneServer* zs = zoneserver_list.FindByName(to);
			if (zs != nullptr) {
				zs->SendPacket(pack);
				return true;
			}
			return false;
		}
	}
	return false;
}

void ClientList::SendGuildPacket(uint32 guild_id, ServerPacket* pack) {
	std::set<uint32> zone_ids;

	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		if (iterator.GetData()->GuildID() == guild_id) {
			zone_ids.insert(iterator.GetData()->zone());
		}
		iterator.Advance();
	}

	//now we know all the zones, send it to each one... this is kinda a shitty way to do this
	//since its basically O(n^2)
	std::set<uint32>::iterator cur, end;
	cur = zone_ids.begin();
	end = zone_ids.end();
	for(; cur != end; cur++) {
		zoneserver_list.SendPacket(*cur, pack);
	}
}

void ClientList::UpdateClientGuild(uint32 char_id, uint32 guild_id) {
	LinkedListIterator<ClientListEntry*> iterator(clientlist);

	iterator.Reset();
	while(iterator.MoreElements()) {
		ClientListEntry *cle = iterator.GetData();
		if (cle->CharID() == char_id) {
			cle->SetGuild(guild_id);
		}
		iterator.Advance();
	}
}



int ClientList::GetClientCount() {
	return(numplayers);
}

void ClientList::GetClients(const char *zone_name, std::vector<ClientListEntry *> &res) {
	LinkedListIterator<ClientListEntry *> iterator(clientlist);
	iterator.Reset();

	if(zone_name[0] == '\0') {
		while(iterator.MoreElements()) {
			ClientListEntry* tmp = iterator.GetData();
			res.push_back(tmp);
			iterator.Advance();
		}
	} else {
		uint32 zoneid = database.GetZoneID(zone_name);
		while(iterator.MoreElements()) {
			ClientListEntry* tmp = iterator.GetData();
			if(tmp->zone() == zoneid)
				res.push_back(tmp);
			iterator.Advance();
		}
	}
}

void ClientList::SendClientVersionSummary(const char *Name)
{
	uint32 ClientTitaniumCount = 0;
	uint32 ClientSoFCount = 0;
	uint32 ClientSoDCount = 0;
	uint32 ClientUnderfootCount = 0;
	uint32 ClientRoFCount = 0;
	uint32 ClientRoF2Count = 0;


	LinkedListIterator<ClientListEntry*> Iterator(clientlist);

	Iterator.Reset();

	while(Iterator.MoreElements())
	{
		ClientListEntry* CLE = Iterator.GetData();

		if(CLE && CLE->zone())
		{
			switch(CLE->GetClientVersion())
			{
				case 1:
				{
					break;
				}
				case 2:
				{
					++ClientTitaniumCount;
					break;
				}
				case 3:
				{
					++ClientSoFCount;
					break;
				}
				case 4:
				{
					++ClientSoDCount;
					break;
				}
				case 5:
				{
					++ClientUnderfootCount;
					break;
				}
				case 6:
				{
					++ClientRoFCount;
					break;
				}
				case 7:
				{
					++ClientRoF2Count;
					break;
				}
				default:
					break;
			}
		}

		Iterator.Advance();

	}

	zoneserver_list.SendEmoteMessage(Name, 0, 0, 13, "There are %i Titanium, %i SoF, %i SoD, %i UF, %i RoF, %i RoF2 clients currently connected.",
		ClientTitaniumCount, ClientSoFCount, ClientSoDCount, ClientUnderfootCount, ClientRoFCount, ClientRoF2Count); 
}

void ClientList::OnTick(EQ::Timer *t)
{
	if (!EventSubscriptionWatcher::Get()->IsSubscribed("EQW::ClientUpdate")) {
		return;
	}

	Json::Value out;
	out["event"] = "EQW::ClientUpdate";
	out["data"] = Json::Value();

	LinkedListIterator<ClientListEntry*> Iterator(clientlist);

	Iterator.Reset();

	while (Iterator.MoreElements())
	{
		ClientListEntry* cle = Iterator.GetData();
		
		Json::Value outclient;

		outclient["Online"] = cle->Online();
		outclient["ID"] = cle->GetID();
		outclient["IP"] = cle->GetIP();
		outclient["LSID"] = cle->LSID();
		outclient["LSAccountID"] = cle->LSAccountID();
		outclient["LSName"] = cle->LSName();
		outclient["WorldAdmin"] = cle->WorldAdmin();

		outclient["AccountID"] = cle->AccountID();
		outclient["AccountName"] = cle->AccountName();
		outclient["Admin"] = cle->Admin();

		auto server = cle->Server();
		if (server) {
			outclient["Server"]["CAddress"] = server->GetCAddress();
			outclient["Server"]["CLocalAddress"] = server->GetCLocalAddress();
			outclient["Server"]["CompileTime"] = server->GetCompileTime();
			outclient["Server"]["CPort"] = server->GetCPort();
			outclient["Server"]["ID"] = server->GetID();
			outclient["Server"]["InstanceID"] = server->GetInstanceID();
			outclient["Server"]["IP"] = server->GetIP();
			outclient["Server"]["LaunchedName"] = server->GetLaunchedName();
			outclient["Server"]["LaunchName"] = server->GetLaunchName();
			outclient["Server"]["Port"] = server->GetPort();
			outclient["Server"]["PrevZoneID"] = server->GetPrevZoneID();
			outclient["Server"]["UUID"] = server->GetUUID();
			outclient["Server"]["ZoneID"] = server->GetZoneID();
			outclient["Server"]["ZoneLongName"] = server->GetZoneLongName();
			outclient["Server"]["ZoneName"] = server->GetZoneName();
			outclient["Server"]["ZoneOSProcessID"] = server->GetZoneOSProcessID();
			outclient["Server"]["NumPlayers"] = server->NumPlayers();
			outclient["Server"]["BootingUp"] = server->IsBootingUp();
			outclient["Server"]["StaticZone"] = server->IsStaticZone();
		}
		else {
			outclient["Server"] = Json::Value();
		}

		outclient["CharID"] = cle->CharID();
		outclient["name"] = cle->name();
		outclient["zone"] = cle->zone();
		outclient["instance"] = cle->instance();
		outclient["level"] = cle->level();
		outclient["class_"] = cle->class_();
		outclient["race"] = cle->race();
		outclient["Anon"] = cle->Anon();

		outclient["TellsOff"] = cle->TellsOff();
		outclient["GuildID"] = cle->GuildID();
		outclient["LFG"] = cle->LFG();
		outclient["GM"] = cle->GetGM();
		outclient["LocalClient"] = cle->IsLocalClient();
		outclient["LFGFromLevel"] = cle->GetLFGFromLevel();
		outclient["LFGToLevel"] = cle->GetLFGToLevel();
		outclient["LFGMatchFilter"] = cle->GetLFGMatchFilter();
		outclient["LFGComments"] = cle->GetLFGComments();
		outclient["ClientVersion"] = cle->GetClientVersion();
		out["data"].append(outclient);

		Iterator.Advance();
	}

	web_interface.SendEvent(out);
}