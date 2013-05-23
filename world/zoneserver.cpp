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
#include "../common/debug.h"
#include "zoneserver.h"
#include "clientlist.h"
#include "LoginServer.h"
#include "LoginServerList.h"
#include "zonelist.h"
#include "worlddb.h"
#include "console.h"
#include "client.h"
#include "../common/md5.h"
#include "WorldConfig.h"
#include "../common/guilds.h"
#include "../common/packet_dump.h"
#include "../common/misc.h"
#include "../common/StringUtil.h"
#include "cliententry.h"
#include "wguild_mgr.h"
#include "lfplist.h"
#include "AdventureManager.h"
#include "ucs.h"
#include "queryserv.h"

extern ClientList client_list;
extern GroupLFPList LFPGroupList;
extern ZSList zoneserver_list;
extern ConsoleList console_list;
extern LoginServerList loginserverlist;
extern volatile bool RunLoops;
extern AdventureManager adventure_manager;
extern UCSConnection UCSLink;
extern QueryServConnection QSLink;
void CatchSignal(int sig_num);

ZoneServer::ZoneServer(EmuTCPConnection* itcpc)
: WorldTCPConnection(), tcpc(itcpc), ls_zboot(5000) {
	ID = zoneserver_list.GetNextID();
	memset(zone_name, 0, sizeof(zone_name));
	memset(compiled, 0, sizeof(compiled));
	zoneID = 0;
	instanceID = 0;

	memset(clientaddress, 0, sizeof(clientaddress));
	clientport = 0;
	BootingUp = false;
	authenticated = false;
	staticzone = false;
	pNumPlayers = 0;
}

ZoneServer::~ZoneServer() {
	if (RunLoops)
		client_list.CLERemoveZSRef(this);
	tcpc->Free();
}

bool ZoneServer::SetZone(uint32 iZoneID, uint32 iInstanceID, bool iStaticZone) {
	BootingUp = false;

	const char* zn = MakeLowerString(database.GetZoneName(iZoneID));
	char*	longname;

	if (iZoneID)
		zlog(WORLD__ZONE,"Setting to '%s' (%d:%d)%s",(zn) ? zn : "",iZoneID, iInstanceID,
			iStaticZone ? " (Static)" : "");

	zoneID = iZoneID;
	instanceID = iInstanceID;
	if(iZoneID!=0)
		oldZoneID = iZoneID;
	if (zoneID == 0) {
		client_list.CLERemoveZSRef(this);
		pNumPlayers = 0;
		LSSleepUpdate(GetPrevZoneID());
	}

	staticzone = iStaticZone;

	if (zn)
	{
		strn0cpy(zone_name, zn, sizeof(zone_name));
		if( database.GetZoneLongName( (char*)zone_name, &longname, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr ) )
		{
			strn0cpy(long_name, longname, sizeof(long_name));
			safe_delete_array( longname );
		}
		else
			strcpy(long_name, "");
	}
	else
	{
		strcpy(zone_name, "");
		strcpy(long_name, "");
	}

	client_list.ZoneBootup(this);
	ls_zboot.Start();

	return true;
}

void ZoneServer::LSShutDownUpdate(uint32 zoneid){
	if(WorldConfig::get()->UpdateStats){
		ServerPacket* pack = new ServerPacket;
		pack->opcode = ServerOP_LSZoneShutdown;
		pack->size = sizeof(ZoneShutdown_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer,0,pack->size);
		ZoneShutdown_Struct* zsd =(ZoneShutdown_Struct*)pack->pBuffer;
		if(zoneid==0)
			zsd->zone = GetPrevZoneID();
		else
			zsd->zone = zoneid;
		zsd->zone_wid = GetID();
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}
void ZoneServer::LSBootUpdate(uint32 zoneid, uint32 instanceid, bool startup){
	if(WorldConfig::get()->UpdateStats){
		ServerPacket* pack = new ServerPacket;
		if(startup)
			pack->opcode = ServerOP_LSZoneStart;
		else
			pack->opcode = ServerOP_LSZoneBoot;
		pack->size = sizeof(ZoneBoot_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer,0,pack->size);
		ZoneBoot_Struct* bootup =(ZoneBoot_Struct*)pack->pBuffer;
		if(startup)
			strcpy(bootup->compile_time,GetCompileTime());
		bootup->zone = zoneid;
		bootup->zone_wid = GetID();
		bootup->instance = instanceid;
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}

void ZoneServer::LSSleepUpdate(uint32 zoneid){
	if(WorldConfig::get()->UpdateStats){
		ServerPacket* pack = new ServerPacket;
		pack->opcode = ServerOP_LSZoneSleep;
		pack->size = sizeof(ServerLSZoneSleep_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer,0,pack->size);
		ServerLSZoneSleep_Struct* sleep =(ServerLSZoneSleep_Struct*)pack->pBuffer;
		sleep->zone = zoneid;
		sleep->zone_wid = GetID();
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}

bool ZoneServer::Process() {
	if (!tcpc->Connected())
		return false;
	if(ls_zboot.Check()){
		LSBootUpdate(GetZoneID(), true);
		ls_zboot.Disable();
	}
	ServerPacket *pack = 0;
	while((pack = tcpc->PopPacket())) {
		_hex(WORLD__ZONE_TRACE,pack->pBuffer,pack->size);
		if (!authenticated) {
			if (WorldConfig::get()->SharedKey.length() > 0) {
				if (pack->opcode == ServerOP_ZAAuth && pack->size == 16) {
					uint8 tmppass[16];
					MD5::Generate((const uchar*) WorldConfig::get()->SharedKey.c_str(), WorldConfig::get()->SharedKey.length(), tmppass);
					if (memcmp(pack->pBuffer, tmppass, 16) == 0)
						authenticated = true;
					else {
						struct in_addr in;
						in.s_addr = GetIP();
						zlog(WORLD__ZONE_ERR,"Zone authorization failed.");
						ServerPacket* pack = new ServerPacket(ServerOP_ZAAuthFailed);
						SendPacket(pack);
						delete pack;
						Disconnect();
						return false;
					}
				}
				else {
					struct in_addr in;
					in.s_addr = GetIP();
					zlog(WORLD__ZONE_ERR,"Zone authorization failed.");
					ServerPacket* pack = new ServerPacket(ServerOP_ZAAuthFailed);
					SendPacket(pack);
					delete pack;
					Disconnect();
					return false;
				}
			}
			else
			{
				_log(WORLD__ZONE,"**WARNING** You have not configured a world shared key in your config file. You should add a <key>STRING</key> element to your <world> element to prevent unauthroized zone access.");
				authenticated = true;
			}
		}
		switch(pack->opcode) {
			case 0:
				break;
			case ServerOP_KeepAlive: {
				// ignore this
				break;
			}
			case ServerOP_ZAAuth: {
				break;
			}
			case ServerOP_LSZoneBoot:{
				if(pack->size==sizeof(ZoneBoot_Struct)){
					ZoneBoot_Struct* zbs= (ZoneBoot_Struct*)pack->pBuffer;
					SetCompile(zbs->compile_time);
				}
				break;
			}
			case ServerOP_GroupInvite: {
				if(pack->size != sizeof(GroupInvite_Struct))
					break;

				GroupInvite_Struct* gis = (GroupInvite_Struct*) pack->pBuffer;

				client_list.SendPacket(gis->invitee_name, pack);
				break;
			}
			case ServerOP_GroupFollow: {
				if(pack->size != sizeof(ServerGroupFollow_Struct))
					break;

				ServerGroupFollow_Struct *sgfs = (ServerGroupFollow_Struct *) pack->pBuffer;

				client_list.SendPacket(sgfs->gf.name1, pack);
				break;
			}
			case ServerOP_GroupFollowAck: {
				if(pack->size != sizeof(ServerGroupFollowAck_Struct))
					break;

				ServerGroupFollowAck_Struct *sgfas = (ServerGroupFollowAck_Struct *) pack->pBuffer;

				client_list.SendPacket(sgfas->Name, pack);
				break;
			}
			case ServerOP_GroupCancelInvite: {
				if(pack->size != sizeof(GroupCancel_Struct))
					break;

				GroupCancel_Struct *gcs = (GroupCancel_Struct *) pack->pBuffer;

				client_list.SendPacket(gcs->name1, pack);
				break;
			}
			case ServerOP_GroupIDReq: {
				SendGroupIDs();
				break;
			}
			case ServerOP_GroupLeave: {
				if(pack->size != sizeof(ServerGroupLeave_Struct))
					break;
				zoneserver_list.SendPacket(pack); //bounce it to all zones
				break;
			}

			case ServerOP_GroupJoin: {
				if(pack->size != sizeof(ServerGroupJoin_Struct))
					break;
				zoneserver_list.SendPacket(pack); //bounce it to all zones
				break;
			}

			case ServerOP_ForceGroupUpdate: {
				if(pack->size != sizeof(ServerForceGroupUpdate_Struct))
					break;
				zoneserver_list.SendPacket(pack); //bounce it to all zones
				break;
			}

			case ServerOP_OOZGroupMessage: {
				zoneserver_list.SendPacket(pack); //bounce it to all zones
				break;
			}

			case ServerOP_DisbandGroup: {
				if(pack->size != sizeof(ServerDisbandGroup_Struct))
					break;
				zoneserver_list.SendPacket(pack); //bounce it to all zones
				break;
			}

			case ServerOP_RaidAdd:{
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidRemove: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidDisband: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidLockFlag: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidChangeGroup: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_UpdateGroup: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidGroupDisband: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidGroupAdd: {
				if(pack->size != sizeof(ServerRaidGroupAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidGroupRemove: {
				if(pack->size != sizeof(ServerRaidGroupAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidGroupSay: {
				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidSay: {
				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidGroupLeader: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_RaidLeader: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_DetailsChange: {
				if(pack->size != sizeof(ServerRaidGeneralAction_Struct))
					break;

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_SpawnCondition: {
				if(pack->size != sizeof(ServerSpawnCondition_Struct))
					break;
				//bounce the packet to the correct zone server, if its up
				ServerSpawnCondition_Struct* ssc = (ServerSpawnCondition_Struct*)pack->pBuffer;
				zoneserver_list.SendPacket(ssc->zoneID, ssc->instanceID, pack);
				break;
			}
			case ServerOP_SpawnEvent: {
				if(pack->size != sizeof(ServerSpawnEvent_Struct))
					break;
				//bounce the packet to the correct zone server, if its up
				ServerSpawnEvent_Struct* sse = (ServerSpawnEvent_Struct*)pack->pBuffer;
				zoneserver_list.SendPacket(sse->zoneID, 0, pack);
				break;
			}
			case ServerOP_ChannelMessage: {
				ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*) pack->pBuffer;
				if(scm->chan_num == 20)
				{
					UCSLink.SendMessage(scm->from, scm->message);
					break;
				}
				if (scm->chan_num == 7 || scm->chan_num == 14) {
					if (scm->deliverto[0] == '*') {
						Console* con = 0;
						con = console_list.FindByAccountName(&scm->deliverto[1]);
						if (((!con) || (!con->SendChannelMessage(scm))) && (!scm->noreply))
							zoneserver_list.SendEmoteMessage(scm->from, 0, 0, 0, "You told %s, '%s is not online at this time'", scm->to, scm->to);
						break;
					}
					ClientListEntry* cle = client_list.FindCharacter(scm->deliverto);
					if (cle == 0 || cle->Online() < CLE_Status_Zoning || (cle->TellsOff() && ((cle->Anon() == 1 && scm->fromadmin < cle->Admin()) || scm->fromadmin < 80))) {
						if (!scm->noreply)
							zoneserver_list.SendEmoteMessage(scm->from, 0, 0, 0, "You told %s, '%s is not online at this time'", scm->to, scm->to);
					}
					else if (cle->Online() == CLE_Status_Zoning) {
						if (!scm->noreply) {
							char errbuf[MYSQL_ERRMSG_SIZE];
							char *query = 0;
							MYSQL_RES *result;
							//MYSQL_ROW row; Trumpcard - commenting. Currently unused.
							time_t rawtime;
							struct tm * timeinfo;
							time ( &rawtime );
							timeinfo = localtime ( &rawtime );
							char *telldate=asctime(timeinfo);
							if (database.RunQuery(query, MakeAnyLenString(&query, "SELECT name from character_ where name='%s'",scm->deliverto), errbuf, &result)) {
								safe_delete(query);
								if (result!=0) {
									if (database.RunQuery(query, MakeAnyLenString(&query, "INSERT INTO tellque (Date,Receiver,Sender,Message) values('%s','%s','%s','%s')",telldate,scm->deliverto,scm->from,scm->message), errbuf, &result))
										zoneserver_list.SendEmoteMessage(scm->from, 0, 0, 0, "Your message has been added to the %s's que.", scm->to);
									else
										zoneserver_list.SendEmoteMessage(scm->from, 0, 0, 0, "You told %s, '%s is not online at this time'", scm->to, scm->to);
									safe_delete(query);
								}
								else
									zoneserver_list.SendEmoteMessage(scm->from, 0, 0, 0, "You told %s, '%s is not online at this time'", scm->to, scm->to);
								mysql_free_result(result);
							}
							else
								safe_delete(query);
						}
					//		zoneserver_list.SendEmoteMessage(scm->from, 0, 0, 0, "You told %s, '%s is not online at this time'", scm->to, scm->to);
					}
					else if (cle->Server() == 0) {
						if (!scm->noreply)
							zoneserver_list.SendEmoteMessage(scm->from, 0, 0, 0, "You told %s, '%s is not contactable at this time'", scm->to, scm->to);
					}
					else
						cle->Server()->SendPacket(pack);
				}
				else {
					if (scm->chan_num == 5 || scm->chan_num == 6 || scm->chan_num == 11) {
						console_list.SendChannelMessage(scm);
					}
					zoneserver_list.SendPacket(pack);
				}
				break;
			}
			case ServerOP_EmoteMessage: {
				ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*) pack->pBuffer;
				zoneserver_list.SendEmoteMessageRaw(sem->to, sem->guilddbid, sem->minstatus, sem->type, sem->message);
				break;
			}
			case ServerOP_VoiceMacro: {

				ServerVoiceMacro_Struct* svm = (ServerVoiceMacro_Struct*) pack->pBuffer;

				if(svm->Type == VoiceMacroTell) {

					ClientListEntry* cle = client_list.FindCharacter(svm->To);

					if (!cle || (cle->Online() < CLE_Status_Zoning) || !cle->Server()) {

						zoneserver_list.SendEmoteMessage(svm->From, 0, 0, 0, "'%s is not online at this time'", svm->To);

						break;
					}

					cle->Server()->SendPacket(pack);
				}
				else
					zoneserver_list.SendPacket(pack);

				break;
			}

			case ServerOP_RezzPlayerAccept: {
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_RezzPlayer: {

				RezzPlayer_Struct* sRezz = (RezzPlayer_Struct*) pack->pBuffer;
				if (zoneserver_list.SendPacket(pack)){
					zlog(WORLD__ZONE,"Sent Rez packet for %s",sRezz->rez.your_name);
				}
				else {
					zlog(WORLD__ZONE,"Could not send Rez packet for %s",sRezz->rez.your_name);
				}
				break;
			}
			case ServerOP_RezzPlayerReject:
			{
				char *Recipient = (char *)pack->pBuffer;
				client_list.SendPacket(Recipient, pack);
				break;
			}

			case ServerOP_MultiLineMsg: {
				ServerMultiLineMsg_Struct* mlm = (ServerMultiLineMsg_Struct*) pack->pBuffer;
				client_list.SendPacket(mlm->to, pack);
				break;
			}
			case ServerOP_SetZone: {
				if(pack->size != sizeof(SetZone_Struct))
					break;

				SetZone_Struct* szs = (SetZone_Struct*) pack->pBuffer;
				if (szs->zoneid != 0) {
					if(database.GetZoneName(szs->zoneid))
						SetZone(szs->zoneid, szs->instanceid, szs->staticzone);
					else
						SetZone(0);
				}
				else
					SetZone(0);

				break;
			}
			case ServerOP_SetConnectInfo: {
				if (pack->size != sizeof(ServerConnectInfo))
						break;
				ServerConnectInfo* sci = (ServerConnectInfo*) pack->pBuffer;

				if (!sci->port) {
					clientport=zoneserver_list.GetAvailableZonePort();

					ServerPacket p(ServerOP_SetConnectInfo, sizeof(ServerConnectInfo));
					memset(p.pBuffer,0,sizeof(ServerConnectInfo));
					ServerConnectInfo* sci = (ServerConnectInfo*) p.pBuffer;
					sci->port = clientport;
					SendPacket(&p);
					zlog(WORLD__ZONE,"Auto zone port configuration. Telling zone to use port %d",clientport);
				} else {
					clientport=sci->port;
					zlog(WORLD__ZONE,"Zone specified port %d, must be a previously allocated zone reconnecting.",clientport);
				}

			}
			case ServerOP_SetLaunchName: {
				if(pack->size != sizeof(LaunchName_Struct))
					break;
				const LaunchName_Struct* ln = (const LaunchName_Struct*)pack->pBuffer;
				launcher_name = ln->launcher_name;
				launched_name = ln->zone_name;
				zlog(WORLD__ZONE, "Zone started with name %s by launcher %s", launched_name.c_str(), launcher_name.c_str());
				break;
			}
			case ServerOP_ShutdownAll: {
				if(pack->size==0){
					zoneserver_list.SendPacket(pack);
					zoneserver_list.Process();
					CatchSignal(2);
				}
				else{
					WorldShutDown_Struct* wsd=(WorldShutDown_Struct*)pack->pBuffer;
					if(wsd->time==0 && wsd->interval==0 && zoneserver_list.shutdowntimer->Enabled()){
						zoneserver_list.shutdowntimer->Disable();
						zoneserver_list.reminder->Disable();
					}
					else{
						zoneserver_list.shutdowntimer->SetTimer(wsd->time);
						zoneserver_list.reminder->SetTimer(wsd->interval-1000);
						zoneserver_list.reminder->SetAtTrigger(wsd->interval);
						zoneserver_list.shutdowntimer->Start();
						zoneserver_list.reminder->Start();
					}
				}
				break;
			}
			case ServerOP_ZoneShutdown: {
				ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
				ZoneServer* zs = 0;
				if (s->ZoneServerID != 0)
					zs = zoneserver_list.FindByID(s->ZoneServerID);
				else if (s->zoneid != 0)
					zs = zoneserver_list.FindByName(database.GetZoneName(s->zoneid));
				else
					zoneserver_list.SendEmoteMessage(s->adminname, 0, 0, 0, "Error: SOP_ZoneShutdown: neither ID nor name specified");

				if (zs == 0)
					zoneserver_list.SendEmoteMessage(s->adminname, 0, 0, 0, "Error: SOP_ZoneShutdown: zoneserver not found");
				else
					zs->SendPacket(pack);
				break;
			}
			case ServerOP_ZoneBootup: {
				ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
				zoneserver_list.SOPZoneBootup(s->adminname, s->ZoneServerID, database.GetZoneName(s->zoneid), s->makestatic);
				break;
			}
			case ServerOP_ZoneStatus: {
				if (pack->size >= 1)
					zoneserver_list.SendZoneStatus((char *) &pack->pBuffer[1], (uint8) pack->pBuffer[0], this);
				break;

			}
			case ServerOP_AcceptWorldEntrance: {
				if(pack->size != sizeof(WorldToZone_Struct))
					break;

				WorldToZone_Struct* wtz = (WorldToZone_Struct*) pack->pBuffer;
				Client* client = 0;
				client = client_list.FindByAccountID(wtz->account_id);
				if(client != 0)
					client->Clearance(wtz->response);
			}
			case ServerOP_ZoneToZoneRequest: {
			//
			// solar: ZoneChange is received by the zone the player is in, then the
			// zone sends a ZTZ which ends up here. This code then find the target
			// (ingress point) and boots it if needed, then sends the ZTZ to it.
			// The ingress server will decide wether the player can enter, then will
			// send back the ZTZ to here. This packet is passed back to the egress
			// server, which will send a ZoneChange response back to the client
			// which can be an error, or a success, in which case the client will
			// disconnect, and their zone location will be saved when ~Client is
			// called, so it will be available when they ask to zone.
			//


				if(pack->size != sizeof(ZoneToZone_Struct))
					break;
				ZoneToZone_Struct* ztz = (ZoneToZone_Struct*) pack->pBuffer;
				ClientListEntry* client = nullptr;
				if(WorldConfig::get()->UpdateStats)
					client = client_list.FindCharacter(ztz->name);

				zlog(WORLD__ZONE,"ZoneToZone request for %s current zone %d req zone %d\n",
					ztz->name, ztz->current_zone_id, ztz->requested_zone_id);

				if(GetZoneID() == ztz->current_zone_id && GetInstanceID() == ztz->current_instance_id)	// this is a request from the egress zone
				{
					zlog(WORLD__ZONE,"Processing ZTZ for egress from zone for client %s\n", ztz->name);

					if
					(
						ztz->admin < 80 &&
						ztz->ignorerestrictions < 2 &&
						zoneserver_list.IsZoneLocked(ztz->requested_zone_id)
					)
					{
						ztz->response = 0;
						SendPacket(pack);
						break;
					}

					ZoneServer *ingress_server = nullptr;
					if(ztz->requested_instance_id > 0)
					{
						ingress_server = zoneserver_list.FindByInstanceID(ztz->requested_instance_id);
					}
					else
					{
						ingress_server = zoneserver_list.FindByZoneID(ztz->requested_zone_id);

					}

					if(ingress_server)	// found a zone already running
					{
						_log(WORLD__ZONE,"Found a zone already booted for %s\n", ztz->name);
						ztz->response = 1;
					}
					else	// need to boot one
					{
						int server_id;
						if ((server_id = zoneserver_list.TriggerBootup(ztz->requested_zone_id, ztz->requested_instance_id))){
							_log(WORLD__ZONE,"Successfully booted a zone for %s\n", ztz->name);
							// bootup successful, ready to rock
							ztz->response = 1;
							ingress_server = zoneserver_list.FindByID(server_id);
						}
						else
						{
							_log(WORLD__ZONE_ERR,"FAILED to boot a zone for %s\n", ztz->name);
							// bootup failed, send back error code 0
							ztz->response = 0;
						}
					}
					if(ztz->response!=0 && client)
						client->LSZoneChange(ztz);
					SendPacket(pack);	// send back to egress server
					if(ingress_server)	// if we couldn't boot one, this is 0
					{
						ingress_server->SendPacket(pack);	// inform target server
					}
				}
				else	// this is response from the ingress server, route it back to the egress server
				{
					zlog(WORLD__ZONE,"Processing ZTZ for ingress to zone for client %s\n", ztz->name);
					ZoneServer *egress_server = nullptr;
					if(ztz->current_instance_id > 0)
					{
						egress_server = zoneserver_list.FindByInstanceID(ztz->current_instance_id);
					}
					else
					{
						egress_server = zoneserver_list.FindByZoneID(ztz->current_zone_id);
					}

					if(egress_server)
					{
						egress_server->SendPacket(pack);
					}
				}

				break;
			}
			case ServerOP_ClientList: {
				if (pack->size != sizeof(ServerClientList_Struct)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_ClientList. Got: %d, Expected: %d",pack->size,sizeof(ServerClientList_Struct));
					break;
				}
				client_list.ClientUpdate(this, (ServerClientList_Struct*) pack->pBuffer);
				break;
			}
			case ServerOP_ClientListKA: {
				ServerClientListKeepAlive_Struct* sclka = (ServerClientListKeepAlive_Struct*) pack->pBuffer;
				if (pack->size < 4 || pack->size != 4 + (4 * sclka->numupdates)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_ClientListKA. Got: %d, Expected: %d",pack->size, (4 + (4 * sclka->numupdates)));
					break;
				}
				client_list.CLEKeepAlive(sclka->numupdates, sclka->wid);
				break;
			}
			case ServerOP_Who: {
				ServerWhoAll_Struct* whoall = (ServerWhoAll_Struct*) pack->pBuffer;
				Who_All_Struct* whom = new Who_All_Struct;
				memset(whom,0,sizeof(Who_All_Struct));
				whom->gmlookup = whoall->gmlookup;
				whom->lvllow = whoall->lvllow;
				whom->lvlhigh = whoall->lvlhigh;
				whom->wclass = whoall->wclass;
				whom->wrace = whoall->wrace;
				strcpy(whom->whom,whoall->whom);
				client_list.SendWhoAll(whoall->fromid,whoall->from, whoall->admin, whom, this);
				delete whom;
				break;
			}
			case ServerOP_RequestOnlineGuildMembers:
			{
				ServerRequestOnlineGuildMembers_Struct *srogms = (ServerRequestOnlineGuildMembers_Struct*) pack->pBuffer;
				zlog(GUILDS__IN_PACKETS, "ServerOP_RequestOnlineGuildMembers Recieved. FromID=%i GuildID=%i", srogms->FromID, srogms->GuildID);
				client_list.SendOnlineGuildMembers(srogms->FromID, srogms->GuildID);
				break;
			}
			case ServerOP_ClientVersionSummary:
			{
				ServerRequestClientVersionSummary_Struct *srcvss = (ServerRequestClientVersionSummary_Struct*) pack->pBuffer;
				client_list.SendClientVersionSummary(srcvss->Name);
				break;
			}
			case ServerOP_ReloadRules:
			{
				zoneserver_list.SendPacket(pack);
				RuleManager::Instance()->LoadRules(&database, "default");
				break;
			}
			case ServerOP_ReloadRulesWorld:
			{
				RuleManager::Instance()->LoadRules(&database, "default");
				break;
			}
			case ServerOP_CameraShake:
			{
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_FriendsWho: {
				ServerFriendsWho_Struct* FriendsWho = (ServerFriendsWho_Struct*) pack->pBuffer;
				client_list.SendFriendsWho(FriendsWho, this);
				break;
			}
			case ServerOP_LFGMatches: {
				ServerLFGMatchesRequest_Struct* smrs = (ServerLFGMatchesRequest_Struct*) pack->pBuffer;
				client_list.SendLFGMatches(smrs);
				break;
			}
			case ServerOP_LFPMatches: {
				ServerLFPMatchesRequest_Struct* smrs = (ServerLFPMatchesRequest_Struct*) pack->pBuffer;
				LFPGroupList.SendLFPMatches(smrs);
				break;
			}
			case ServerOP_LFPUpdate: {
				ServerLFPUpdate_Struct* sus = (ServerLFPUpdate_Struct*) pack->pBuffer;
				if(sus->Action)
					LFPGroupList.UpdateGroup(sus);
				else
					LFPGroupList.RemoveGroup(sus);
				break;
			}
			case ServerOP_ZonePlayer: {
				//ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*) pack->pBuffer;
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_KickPlayer: {
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_KillPlayer: {
				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_GuildRankUpdate:
			{
				zoneserver_list.SendPacket(pack);
				break;
			}
			//these opcodes get processed by the guild manager.
			case ServerOP_RefreshGuild:
			case ServerOP_DeleteGuild:
			case ServerOP_GuildCharRefresh:
			case ServerOP_GuildMemberUpdate: {
				guild_mgr.ProcessZonePacket(pack);
				break;
			}

			case ServerOP_FlagUpdate: {
				ClientListEntry* cle = client_list.FindCLEByAccountID(*((uint32*) pack->pBuffer));
				if (cle)
					cle->SetAdmin(*((int16*) &pack->pBuffer[4]));
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_GMGoto: {
				if (pack->size != sizeof(ServerGMGoto_Struct)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_GMGoto. Got: %d, Expected: %d",pack->size,sizeof(ServerGMGoto_Struct));
					break;
				}
				ServerGMGoto_Struct* gmg = (ServerGMGoto_Struct*) pack->pBuffer;
				ClientListEntry* cle = client_list.FindCharacter(gmg->gotoname);
				if (cle != 0) {
					if (cle->Server() == 0)
						this->SendEmoteMessage(gmg->myname, 0, 0, 13, "Error: Cannot identify %s's zoneserver.", gmg->gotoname);
					else if (cle->Anon() == 1 && cle->Admin() > gmg->admin) // no snooping for anon GMs
						this->SendEmoteMessage(gmg->myname, 0, 0, 13, "Error: %s not found", gmg->gotoname);
					else
						cle->Server()->SendPacket(pack);
				}
				else {
					this->SendEmoteMessage(gmg->myname, 0, 0, 13, "Error: %s not found", gmg->gotoname);
				}
				break;
			}
			case ServerOP_Lock: {
				if (pack->size != sizeof(ServerLock_Struct)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_Lock. Got: %d, Expected: %d",pack->size,sizeof(ServerLock_Struct));
					break;
				}
				ServerLock_Struct* slock = (ServerLock_Struct*) pack->pBuffer;
				if (slock->mode >= 1)
					WorldConfig::LockWorld();
				else
					WorldConfig::UnlockWorld();
				if (loginserverlist.Connected()) {
					loginserverlist.SendStatus();
					if (slock->mode >= 1)
						this->SendEmoteMessage(slock->myname, 0, 0, 13, "World locked");
					else
						this->SendEmoteMessage(slock->myname, 0, 0, 13, "World unlocked");
				}
				else {
					if (slock->mode >= 1)
						this->SendEmoteMessage(slock->myname, 0, 0, 13, "World locked, but login server not connected.");
					else
						this->SendEmoteMessage(slock->myname, 0, 0, 13, "World unlocked, but login server not conencted.");
				}
				break;
								}
			case ServerOP_Motd: {
				if (pack->size != sizeof(ServerMotd_Struct)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_Motd. Got: %d, Expected: %d",pack->size,sizeof(ServerMotd_Struct));
					break;
				}
				ServerMotd_Struct* smotd = (ServerMotd_Struct*) pack->pBuffer;
				database.SetVariable("MOTD",smotd->motd);
				//this->SendEmoteMessage(smotd->myname, 0, 0, 13, "Updated Motd.");
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_Uptime: {
				if (pack->size != sizeof(ServerUptime_Struct)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_Uptime. Got: %d, Expected: %d",pack->size,sizeof(ServerUptime_Struct));
					break;
				}
				ServerUptime_Struct* sus = (ServerUptime_Struct*) pack->pBuffer;
				if (sus->zoneserverid == 0) {
					ZSList::ShowUpTime(this, sus->adminname);
				}
				else {
					ZoneServer* zs = zoneserver_list.FindByID(sus->zoneserverid);
					if (zs)
						zs->SendPacket(pack);
				}
				break;
			}
			case ServerOP_Petition: {
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_GetWorldTime: {
				zlog(WORLD__ZONE,"Broadcasting a world time update");
				ServerPacket* pack = new ServerPacket;

				pack->opcode = ServerOP_SyncWorldTime;
				pack->size = sizeof(eqTimeOfDay);
				pack->pBuffer = new uchar[pack->size];
				memset(pack->pBuffer, 0, pack->size);
				eqTimeOfDay* tod = (eqTimeOfDay*) pack->pBuffer;
				tod->start_eqtime=zoneserver_list.worldclock.getStartEQTime();
				tod->start_realtime=zoneserver_list.worldclock.getStartRealTime();
				SendPacket(pack);
				delete pack;
				break;
			}
			case ServerOP_SetWorldTime: {
				zlog(WORLD__ZONE,"Received SetWorldTime");
				eqTimeOfDay* newtime = (eqTimeOfDay*) pack->pBuffer;
				zoneserver_list.worldclock.setEQTimeOfDay(newtime->start_eqtime, newtime->start_realtime);
				zlog(WORLD__ZONE,"New time = %d-%d-%d %d:%d (%d)\n", newtime->start_eqtime.year, newtime->start_eqtime.month, (int)newtime->start_eqtime.day, (int)newtime->start_eqtime.hour, (int)newtime->start_eqtime.minute, (int)newtime->start_realtime);
				zoneserver_list.worldclock.saveFile(WorldConfig::get()->EQTimeFile.c_str());
				zoneserver_list.SendTimeSync();
				break;
			}
			case ServerOP_IPLookup: {
				if (pack->size < sizeof(ServerGenericWorldQuery_Struct)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_IPLookup. Got: %d, Expected (at least): %d",pack->size,sizeof(ServerGenericWorldQuery_Struct));
					break;
				}
				ServerGenericWorldQuery_Struct* sgwq = (ServerGenericWorldQuery_Struct*) pack->pBuffer;
				if (pack->size == sizeof(ServerGenericWorldQuery_Struct))
					client_list.SendCLEList(sgwq->admin, sgwq->from, this);
				else
					client_list.SendCLEList(sgwq->admin, sgwq->from, this, sgwq->query);
				break;
			}
			case ServerOP_LockZone: {
				if (pack->size < sizeof(ServerLockZone_Struct)) {
					zlog(WORLD__ZONE_ERR,"Wrong size on ServerOP_LockZone. Got: %d, Expected: %d",pack->size,sizeof(ServerLockZone_Struct));
					break;
				}
				ServerLockZone_Struct* s = (ServerLockZone_Struct*) pack->pBuffer;
				switch (s->op) {
					case 0:
						zoneserver_list.ListLockedZones(s->adminname, this);
						break;
					case 1:
						if (zoneserver_list.SetLockedZone(s->zoneID, true))
							zoneserver_list.SendEmoteMessage(0, 0, 80, 15, "Zone locked: %s", database.GetZoneName(s->zoneID));
						else
							this->SendEmoteMessageRaw(s->adminname, 0, 0, 0, "Failed to change lock");
						break;
					case 2:
						if (zoneserver_list.SetLockedZone(s->zoneID, false))
							zoneserver_list.SendEmoteMessage(0, 0, 80, 15, "Zone unlocked: %s", database.GetZoneName(s->zoneID));
						else
							this->SendEmoteMessageRaw(s->adminname, 0, 0, 0, "Failed to change lock");
						break;
				}
				break;
			}
			case ServerOP_ItemStatus: {
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_OOCMute: {
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_Revoke: {
				RevokeStruct* rev = (RevokeStruct*)pack->pBuffer;
				ClientListEntry* cle = client_list.FindCharacter(rev->name);
				if (cle != 0 && cle->Server() != 0)
				{
					cle->Server()->SendPacket(pack);
				}
				break;
			}
			case ServerOP_SpawnPlayerCorpse: {
				SpawnPlayerCorpse_Struct* s = (SpawnPlayerCorpse_Struct*)pack->pBuffer;
				ZoneServer* zs = zoneserver_list.FindByZoneID(s->zone_id);
				if(zs) {
					if (zs->SendPacket(pack)) {
						zlog(WORLD__ZONE,"Sent request to spawn player corpse id %i in zone %u.",s->player_corpse_id, s->zone_id);
					}
					else {
						zlog(WORLD__ZONE_ERR,"Could not send request to spawn player corpse id %i in zone %u.",s->player_corpse_id, s->zone_id);
					}
				}
				break;
			}
			case ServerOP_Consent: {
				// Message string id's likely to be used here are:
				// CONSENT_YOURSELF = 399
				// CONSENT_INVALID_NAME = 397
				// TARGET_NOT_FOUND = 101
				ZoneServer* zs;
				ServerOP_Consent_Struct* s = (ServerOP_Consent_Struct*)pack->pBuffer;
				ClientListEntry* cle = client_list.FindCharacter(s->grantname);
				if(cle) {
					if(cle->instance() != 0)
					{
						zs = zoneserver_list.FindByInstanceID(cle->instance());
						if(zs) {
							if(zs->SendPacket(pack)) {
								zlog(WORLD__ZONE, "Sent consent packet from player %s to player %s in zone %u.", s->ownername, s->grantname, cle->instance());
							}
							else {
								zlog(WORLD__ZONE_ERR, "Unable to locate zone record for instance id %u in zoneserver list for ServerOP_Consent operation.", s->instance_id);
							}
						}
						else
						{
							delete pack;
							pack = new ServerPacket(ServerOP_Consent_Response, sizeof(ServerOP_Consent_Struct));
							ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)pack->pBuffer;
							strcpy(scs->grantname, s->grantname);
							strcpy(scs->ownername, s->ownername);
							scs->permission = s->permission;
							scs->zone_id = s->zone_id;
							scs->instance_id = s->instance_id;
							scs->message_string_id = 101;
							zs = zoneserver_list.FindByInstanceID(s->instance_id);
							if(zs) {
								if(!zs->SendPacket(pack))
									zlog(WORLD__ZONE_ERR, "Unable to send consent response back to player %s in instance %u.", s->ownername, zs->GetInstanceID());
							}
							else {
								zlog(WORLD__ZONE_ERR, "Unable to locate zone record for instance id %u in zoneserver list for ServerOP_Consent_Response operation.", s->instance_id);
							}
						}
					}
					else
					{
						zs = zoneserver_list.FindByZoneID(cle->zone());
						if(zs) {
							if(zs->SendPacket(pack)) {
								zlog(WORLD__ZONE, "Sent consent packet from player %s to player %s in zone %u.", s->ownername, s->grantname, cle->zone());
							}
							else {
								zlog(WORLD__ZONE_ERR, "Unable to locate zone record for zone id %u in zoneserver list for ServerOP_Consent operation.", s->zone_id);
							}
						}
						else {
							// send target not found back to requester
							delete pack;
							pack = new ServerPacket(ServerOP_Consent_Response, sizeof(ServerOP_Consent_Struct));
							ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)pack->pBuffer;
							strcpy(scs->grantname, s->grantname);
							strcpy(scs->ownername, s->ownername);
							scs->permission = s->permission;
							scs->zone_id = s->zone_id;
							scs->message_string_id = 101;
							zs = zoneserver_list.FindByZoneID(s->zone_id);
							if(zs) {
								if(!zs->SendPacket(pack))
									zlog(WORLD__ZONE_ERR, "Unable to send consent response back to player %s in zone %s.", s->ownername, zs->GetZoneName());
							}
							else {
								zlog(WORLD__ZONE_ERR, "Unable to locate zone record for zone id %u in zoneserver list for ServerOP_Consent_Response operation.", s->zone_id);
							}
						}
					}
				}
				else {
					// send target not found back to requester
					delete pack;
					pack = new ServerPacket(ServerOP_Consent_Response, sizeof(ServerOP_Consent_Struct));
					ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)pack->pBuffer;
					strcpy(scs->grantname, s->grantname);
					strcpy(scs->ownername, s->ownername);
					scs->permission = s->permission;
					scs->zone_id = s->zone_id;
					scs->message_string_id = 397;
					zs = zoneserver_list.FindByZoneID(s->zone_id);
					if(zs) {
						if(!zs->SendPacket(pack))
							zlog(WORLD__ZONE_ERR, "Unable to send consent response back to player %s in zone %s.", s->ownername, zs->GetZoneName());
					}
					else {
						zlog(WORLD__ZONE_ERR, "Unable to locate zone record for zone id %u in zoneserver list for ServerOP_Consent_Response operation.", s->zone_id);
					}
				}
				break;
			}
			case ServerOP_Consent_Response: {
				// Message string id's likely to be used here are:
				// CONSENT_YOURSELF = 399
				// CONSENT_INVALID_NAME = 397
				// TARGET_NOT_FOUND = 101
				ServerOP_Consent_Struct* s = (ServerOP_Consent_Struct*)pack->pBuffer;
				if(s->instance_id != 0)
				{
					ZoneServer* zs = zoneserver_list.FindByInstanceID(s->instance_id);
					if(zs) {
						if(!zs->SendPacket(pack))
							zlog(WORLD__ZONE_ERR, "Unable to send consent response back to player %s in instance %u.", s->ownername, zs->GetInstanceID());
					}
					else {
						zlog(WORLD__ZONE_ERR, "Unable to locate zone record for instance id %u in zoneserver list for ServerOP_Consent_Response operation.", s->instance_id);
					}
				}
				else
				{
					ZoneServer* zs = zoneserver_list.FindByZoneID(s->zone_id);
					if(zs) {
						if(!zs->SendPacket(pack))
							zlog(WORLD__ZONE_ERR, "Unable to send consent response back to player %s in zone %s.", s->ownername, zs->GetZoneName());
					}
					else {
						zlog(WORLD__ZONE_ERR, "Unable to locate zone record for zone id %u in zoneserver list for ServerOP_Consent_Response operation.", s->zone_id);
					}
				}
				break;
			}

			case ServerOP_InstanceUpdateTime :
			{
				ServerInstanceUpdateTime_Struct *iut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
				ZoneServer *zm = zoneserver_list.FindByInstanceID(iut->instance_id);
				if(zm)
				{
					zm->SendPacket(pack);
				}
				break;
			}
			case ServerOP_QGlobalUpdate:
			{
				if(pack->size != sizeof(ServerQGlobalUpdate_Struct))
				{
					break;
				}

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_QGlobalDelete:
			{
				if(pack->size != sizeof(ServerQGlobalDelete_Struct))
				{
					break;
				}

				zoneserver_list.SendPacket(pack);
				break;
			}

			case ServerOP_AdventureRequest:
			{
				adventure_manager.CalculateAdventureRequestReply((const char*)pack->pBuffer);
				break;
			}

			case ServerOP_AdventureRequestCreate:
			{
				adventure_manager.TryAdventureCreate((const char*)pack->pBuffer);
				break;
			}

			case ServerOP_AdventureDataRequest:
			{
				AdventureFinishEvent fe;
				while(adventure_manager.PopFinishedEvent((const char*)pack->pBuffer, fe))
				{
					adventure_manager.SendAdventureFinish(fe);
				}
				adventure_manager.GetAdventureData((const char*)pack->pBuffer);
				break;
			}

			case ServerOP_AdventureClickDoor:
			{
				ServerPlayerClickedAdventureDoor_Struct *pcad = (ServerPlayerClickedAdventureDoor_Struct*)pack->pBuffer;
				adventure_manager.PlayerClickedDoor(pcad->player, pcad->zone_id, pcad->id);
				break;
			}

			case ServerOP_AdventureLeave:
			{
				adventure_manager.LeaveAdventure((const char*)pack->pBuffer);
				break;
			}

			case ServerOP_AdventureCountUpdate:
			{
				ServerAdventureCount_Struct *sc = (ServerAdventureCount_Struct*)pack->pBuffer;
				adventure_manager.IncrementCount(sc->instance_id);
				break;
			}

			case ServerOP_AdventureAssaCountUpdate:
			{
				adventure_manager.IncrementAssassinationCount(*((uint16*)pack->pBuffer));
				break;
			}

			case ServerOP_AdventureZoneData:
			{
				adventure_manager.GetZoneData(*((uint16*)pack->pBuffer));
				break;
			}

			case ServerOP_AdventureLeaderboard:
			{
				ServerLeaderboardRequest_Struct *lr = (ServerLeaderboardRequest_Struct*)pack->pBuffer;
				adventure_manager.DoLeaderboardRequest(lr->player, lr->type);
				break;
			}

			case ServerOP_LSAccountUpdate:
			{
				zlog(WORLD__ZONE, "Received ServerOP_LSAccountUpdate packet from zone");
				loginserverlist.SendAccountUpdate(pack);
				break;
			}

			case ServerOP_UCSMailMessage:
			{
				UCSLink.SendPacket(pack);
				break;
			}

			case ServerOP_QueryServGeneric:
			case ServerOP_Speech:
			case ServerOP_QSPlayerLogTrades:
			{
				QSLink.SendPacket(pack);
				break;
			}
			case ServerOP_QSPlayerLogHandins:
			{
				QSLink.SendPacket(pack);
				break;
			}
			case ServerOP_QSPlayerLogNPCKills:
			{
				QSLink.SendPacket(pack);
				break;
			}
			case ServerOP_QSPlayerLogDeletes:
			{
				QSLink.SendPacket(pack);
				break;
			}
			case ServerOP_QSPlayerLogMoves:
			{
				QSLink.SendPacket(pack);
				break;
			}
			case ServerOP_QSMerchantLogTransactions:
			{
				QSLink.SendPacket(pack);
				break;
			}
			case ServerOP_CZSignalClientByName:
			case ServerOP_CZMessagePlayer:
			case ServerOP_CZSignalClient:
			{
				zoneserver_list.SendPacket(pack);
				break;
			}
			case ServerOP_DepopAllPlayersCorpses:
			case ServerOP_DepopPlayerCorpse:
			case ServerOP_ReloadTitles:
			case ServerOP_SpawnStatusChange:
			case ServerOP_ReloadTasks:
			case ServerOP_ReloadWorld:
			case ServerOP_UpdateSpawn:
			{
				zoneserver_list.SendPacket(pack);
				break;
			}
			default:
			{
				zlog(WORLD__ZONE_ERR,"Unknown ServerOPcode from zone 0x%04x, size %d",pack->opcode,pack->size);
				DumpPacket(pack->pBuffer, pack->size);
				break;
			}
		}

		delete pack;
	}
	return true;
}

void ZoneServer::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	if (!message)
		return;
	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);
	SendEmoteMessageRaw(to, to_guilddbid, to_minstatus, type, buffer);
}

void ZoneServer::SendEmoteMessageRaw(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message) {
	if (!message)
		return;
	ServerPacket* pack = new ServerPacket;

	pack->opcode = ServerOP_EmoteMessage;
	pack->size = sizeof(ServerEmoteMessage_Struct)+strlen(message)+1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*) pack->pBuffer;

	if (to != 0) {
		strcpy((char *) sem->to, to);
	}
	else {
		sem->to[0] = 0;
	}

	sem->guilddbid = to_guilddbid;
	sem->minstatus = to_minstatus;
	sem->type = type;
	strcpy(&sem->message[0], message);

	pack->Deflate();
	SendPacket(pack);
	delete pack;
}

void ZoneServer::SendGroupIDs() {
	ServerPacket* pack = new ServerPacket(ServerOP_GroupIDReply, sizeof(ServerGroupIDReply_Struct));
	ServerGroupIDReply_Struct* sgi = (ServerGroupIDReply_Struct*)pack->pBuffer;
	zoneserver_list.NextGroupIDs(sgi->start, sgi->end);
	SendPacket(pack);
	delete pack;
}

void ZoneServer::ChangeWID(uint32 iCharID, uint32 iWID) {
	ServerPacket* pack = new ServerPacket(ServerOP_ChangeWID, sizeof(ServerChangeWID_Struct));
	ServerChangeWID_Struct* scw = (ServerChangeWID_Struct*) pack->pBuffer;
	scw->charid = iCharID;
	scw->newwid = iWID;
	zoneserver_list.SendPacket(pack);
	delete pack;
}


void ZoneServer::TriggerBootup(uint32 iZoneID, uint32 iInstanceID, const char* adminname, bool iMakeStatic) {
	BootingUp = true;
	zoneID = iZoneID;
	instanceID = iInstanceID;

	ServerPacket* pack = new ServerPacket(ServerOP_ZoneBootup, sizeof(ServerZoneStateChange_struct));
	ServerZoneStateChange_struct* s = (ServerZoneStateChange_struct *) pack->pBuffer;
	s->ZoneServerID = ID;
	if (adminname != 0)
		strcpy(s->adminname, adminname);

	if (iZoneID == 0)
		s->zoneid = this->GetZoneID();
	else
		s->zoneid = iZoneID;

	s->instanceid = iInstanceID;
	s->makestatic = iMakeStatic;
	SendPacket(pack);
	delete pack;
	LSBootUpdate(iZoneID, iInstanceID);
}

void ZoneServer::IncommingClient(Client* client) {
	BootingUp = true;
	ServerPacket* pack = new ServerPacket(ServerOP_ZoneIncClient, sizeof(ServerZoneIncommingClient_Struct));
	ServerZoneIncommingClient_Struct* s = (ServerZoneIncommingClient_Struct*) pack->pBuffer;
	s->zoneid = GetZoneID();
	s->instanceid = GetInstanceID();
	s->wid = client->GetWID();
	s->ip = client->GetIP();
	s->accid = client->GetAccountID();
	s->admin = client->GetAdmin();
	s->charid = client->GetCharID();
	if (client->GetCLE())
		s->tellsoff = client->GetCLE()->TellsOff();
	strn0cpy(s->charname, client->GetCharName(), sizeof(s->charname));
	strn0cpy(s->lskey, client->GetLSKey(), sizeof(s->lskey));
	SendPacket(pack);
	delete pack;
}

