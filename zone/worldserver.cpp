/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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
#include <stdarg.h>
#include <limits.h>

#ifdef _WINDOWS
#include <process.h>

#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

#include "../common/eq_packet_structs.h"
#include "../common/misc_functions.h"
#include "../common/rulesys.h"
#include "../common/say_link.h"
#include "../common/servertalk.h"
#include "../common/profanity_manager.h"

#include "client.h"
#include "corpse.h"
#include "entity.h"
#include "quest_parser_collection.h"
#include "guild_mgr.h"
#include "mob.h"
#include "petitions.h"
#include "raids.h"
#include "string_ids.h"
#include "titles.h"
#include "worldserver.h"
#include "zone.h"
#include "zone_config.h"
#include "zone_reload.h"


extern EntityList entity_list;
extern Zone* zone;
extern volatile bool is_zone_loaded;
extern void Shutdown();
extern WorldServer worldserver;
extern PetitionList petition_list;
extern uint32 numclients;
extern volatile bool RunLoops;
extern QuestParserCollection *parse;

// QuestParserCollection *parse = 0;

WorldServer::WorldServer()
{
	cur_groupid = 0;
	last_groupid = 0;
	oocmuted = false;
}

WorldServer::~WorldServer() {
}

void WorldServer::Connect()
{
	m_connection.reset(new EQ::Net::ServertalkClient(Config->WorldIP, Config->WorldTCPPort, false, "Zone", Config->SharedKey));
	m_connection->OnConnect([this](EQ::Net::ServertalkClient *client) {
		OnConnected();
	});

	m_connection->OnMessage(std::bind(&WorldServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));

	m_keepalive.reset(new EQ::Timer(2500, true, std::bind(&WorldServer::OnKeepAlive, this, std::placeholders::_1)));
}

bool WorldServer::SendPacket(ServerPacket *pack)
{
	m_connection->SendPacket(pack);
	return true;
}

std::string WorldServer::GetIP() const
{
	return m_connection->Handle()->RemoteIP();
}

uint16 WorldServer::GetPort() const
{
	return m_connection->Handle()->RemotePort();
}

bool WorldServer::Connected() const
{
	return m_connection->Connected();
}

void WorldServer::SetZoneData(uint32 iZoneID, uint32 iInstanceID) {
	auto pack = new ServerPacket(ServerOP_SetZone, sizeof(SetZone_Struct));
	SetZone_Struct* szs = (SetZone_Struct*)pack->pBuffer;
	szs->zoneid = iZoneID;
	szs->instanceid = iInstanceID;
	if (zone) {
		szs->staticzone = zone->IsStaticZone();
	}
	SendPacket(pack);
	safe_delete(pack);
}

void WorldServer::OnConnected() {
	ServerPacket* pack;

	/* Tell the launcher what our information is */
	pack = new ServerPacket(ServerOP_SetLaunchName, sizeof(LaunchName_Struct));
	LaunchName_Struct* ln = (LaunchName_Struct*)pack->pBuffer;
	strn0cpy(ln->launcher_name, m_launcherName.c_str(), 32);
	strn0cpy(ln->zone_name, m_launchedName.c_str(), 16);
	SendPacket(pack);
	safe_delete(pack);

	/* Tell the Worldserver basic information about this zone process */
	pack = new ServerPacket(ServerOP_SetConnectInfo, sizeof(ServerConnectInfo));
	ServerConnectInfo* sci = (ServerConnectInfo*)pack->pBuffer;

	auto config = ZoneConfig::get();
	sci->port = ZoneConfig::get()->ZonePort;
	if (config->WorldAddress.length() > 0) {
		strn0cpy(sci->address, config->WorldAddress.c_str(), 250);
	}
	if (config->LocalAddress.length() > 0) {
		strn0cpy(sci->local_address, config->LocalAddress.c_str(), 250);
	}

	/* Fetch process ID */
	if (getpid()) {
		sci->process_id = getpid();
	}
	else {
		sci->process_id = 0;
	}

	SendPacket(pack);
	safe_delete(pack);

	if (is_zone_loaded) {
		this->SetZoneData(zone->GetZoneID(), zone->GetInstanceID());
		entity_list.UpdateWho(true);
		this->SendEmoteMessage(0, 0, 15, "Zone connect: %s", zone->GetLongName());
		zone->GetTimeSync();
	}
	else {
		this->SetZoneData(0);
	}

	pack = new ServerPacket(ServerOP_LSZoneBoot, sizeof(ZoneBoot_Struct));
	ZoneBoot_Struct* zbs = (ZoneBoot_Struct*)pack->pBuffer;
	strcpy(zbs->compile_time, LAST_MODIFIED);
	SendPacket(pack);
	safe_delete(pack);
}

/* Zone Process Packets from World */
void WorldServer::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	ServerPacket *pack = &tpack;

	switch (opcode) {
	case 0: {
		break;
	}
	case ServerOP_KeepAlive: {
		// ignore this
		break;
	}
							 // World is tellins us what port to use.
	case ServerOP_SetConnectInfo: {
		if (pack->size != sizeof(ServerConnectInfo))
			break;
		ServerConnectInfo* sci = (ServerConnectInfo*)pack->pBuffer;

		if (sci->port == 0) {
			LogCritical("World did not have a port to assign from this server, the port range was not large enough.");
			Shutdown();
		}
		else {
			LogInfo("World assigned Port: [{}] for this zone", sci->port);
			ZoneConfig::SetZonePort(sci->port);
		}
		break;
	}
	case ServerOP_ChannelMessage: {
		if (!is_zone_loaded)
			break;
		ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*)pack->pBuffer;
		if (scm->deliverto[0] == 0) {
			entity_list.ChannelMessageFromWorld(scm->from, scm->to, scm->chan_num, scm->guilddbid, scm->language, scm->lang_skill, scm->message);
		}
		else {
			Client* client = entity_list.GetClientByName(scm->deliverto);
			if (client && client->Connected()) {
				if (scm->chan_num == ChatChannel_TellEcho) {
					if (scm->queued == 1) // tell was queued
						client->Tell_StringID(QUEUED_TELL, scm->to, scm->message);
					else if (scm->queued == 2) // tell queue was full
						client->Tell_StringID(QUEUE_TELL_FULL, scm->to, scm->message);
					else if (scm->queued == 3) // person was offline
						client->MessageString(Chat::EchoTell, TOLD_NOT_ONLINE, scm->to);
					else // normal tell echo "You told Soanso, 'something'"
							// tell echo doesn't use language, so it looks normal to you even if nobody can understand your tells
						client->ChannelMessageSend(scm->from, scm->to, scm->chan_num, 0, 100, scm->message);
				}
				else if (scm->chan_num == ChatChannel_Tell) {
					client->ChannelMessageSend(scm->from, scm->to, scm->chan_num, scm->language, scm->lang_skill, scm->message);
					if (scm->queued == 0) { // this is not a queued tell
						// if it's a tell, echo back to acknowledge it and make it show on the sender's client
						scm->chan_num = ChatChannel_TellEcho;
						memset(scm->deliverto, 0, sizeof(scm->deliverto));
						strcpy(scm->deliverto, scm->from);
						SendPacket(pack);
					}
				}
				else {
					client->ChannelMessageSend(scm->from, scm->to, scm->chan_num, scm->language, scm->lang_skill, scm->message);
				}
			}
		}
		break;
	}
	case ServerOP_VoiceMacro: {

		if (!is_zone_loaded)
			break;

		ServerVoiceMacro_Struct* svm = (ServerVoiceMacro_Struct*)pack->pBuffer;

		auto outapp = new EQApplicationPacket(OP_VoiceMacroOut, sizeof(VoiceMacroOut_Struct));
		VoiceMacroOut_Struct* vmo = (VoiceMacroOut_Struct*)outapp->pBuffer;

		strcpy(vmo->From, svm->From);
		vmo->Type = svm->Type;
		vmo->Voice = svm->Voice;
		vmo->MacroNumber = svm->MacroNumber;

		switch (svm->Type) {
		case VoiceMacroTell: {
			Client* c = entity_list.GetClientByName(svm->To);
			if (!c)
				break;

			c->QueuePacket(outapp);
			break;
		}

		case VoiceMacroGroup: {
			Group* g = entity_list.GetGroupByID(svm->GroupID);

			if (!g)
				break;

			for (unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (g->members[i] && g->members[i]->IsClient())
					g->members[i]->CastToClient()->QueuePacket(outapp);

			}
			break;
		}

		case VoiceMacroRaid: {
			Raid *r = entity_list.GetRaidByID(svm->RaidID);

			if (!r)
				break;

			for (int i = 0; i < MAX_RAID_MEMBERS; i++)
				if (r->members[i].member)
					r->members[i].member->QueuePacket(outapp);

			break;
		}
		}
		safe_delete(outapp);
		break;
	}

	case ServerOP_SpawnCondition: {
		if (pack->size != sizeof(ServerSpawnCondition_Struct))
			break;
		if (!is_zone_loaded)
			break;
		ServerSpawnCondition_Struct* ssc = (ServerSpawnCondition_Struct*)pack->pBuffer;

		zone->spawn_conditions.SetCondition(zone->GetShortName(), zone->GetInstanceID(), ssc->condition_id, ssc->value, true);
		break;
	}
	case ServerOP_SpawnEvent: {
		if (pack->size != sizeof(ServerSpawnEvent_Struct))
			break;
		if (!is_zone_loaded)
			break;
		ServerSpawnEvent_Struct* sse = (ServerSpawnEvent_Struct*)pack->pBuffer;

		zone->spawn_conditions.ReloadEvent(sse->event_id);

		break;
	}
	case ServerOP_AcceptWorldEntrance: {
		if (pack->size != sizeof(WorldToZone_Struct))
			break;
		if (!is_zone_loaded)
			break;
		WorldToZone_Struct* wtz = (WorldToZone_Struct*)pack->pBuffer;

		if (zone->GetMaxClients() != 0 && numclients >= zone->GetMaxClients())
			wtz->response = -1;
		else
			wtz->response = 1;

		SendPacket(pack);
		break;
	}
	case ServerOP_ZoneToZoneRequest: {
		if (pack->size != sizeof(ZoneToZone_Struct))
			break;
		if (!is_zone_loaded)
			break;
		ZoneToZone_Struct* ztz = (ZoneToZone_Struct*)pack->pBuffer;

		if (ztz->current_zone_id == zone->GetZoneID()
			&& ztz->current_instance_id == zone->GetInstanceID()) {
			// it's a response
			Entity* entity = entity_list.GetClientByName(ztz->name);
			if (entity == 0)
				break;

			EQApplicationPacket *outapp;
			outapp = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
			ZoneChange_Struct* zc2 = (ZoneChange_Struct*)outapp->pBuffer;

			if (ztz->response <= 0) {
				zc2->success = ZONE_ERROR_NOTREADY;
				entity->CastToMob()->SetZone(ztz->current_zone_id, ztz->current_instance_id);
				entity->CastToClient()->SetZoning(false);
			}
			else {
				entity->CastToClient()->UpdateWho(1);
				strn0cpy(zc2->char_name, entity->CastToMob()->GetName(), 64);
				zc2->zoneID = ztz->requested_zone_id;
				zc2->instanceID = ztz->requested_instance_id;
				zc2->success = 1;

				// This block is necessary to clean up any merc objects owned by a Client. Maybe we should do this for bots, too?
				if (entity->CastToClient()->GetMerc() != nullptr)
				{
					entity->CastToClient()->GetMerc()->ProcessClientZoneChange(entity->CastToClient());
				}

				entity->CastToMob()->SetZone(ztz->requested_zone_id, ztz->requested_instance_id);

				if (ztz->ignorerestrictions == 3)
					entity->CastToClient()->GoToSafeCoords(ztz->requested_zone_id, ztz->requested_instance_id);
			}

			outapp->priority = 6;
			entity->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);

			switch (ztz->response)
			{
			case -2: {
				entity->CastToClient()->Message(Chat::Red, "You do not own the required locations to enter this zone.");
				break;
			}
			case -1: {
				entity->CastToClient()->Message(Chat::Red, "The zone is currently full, please try again later.");
				break;
			}
			case 0: {
				entity->CastToClient()->Message(Chat::Red, "All zone servers are taken at this time, please try again later.");
				break;
			}
			}
		}
		else {
			// it's a request
			ztz->response = 0;

			if (zone->GetMaxClients() != 0 && numclients >= zone->GetMaxClients())
				ztz->response = -1;
			else {
				ztz->response = 1;
				// since they asked about comming, lets assume they are on their way and not shut down.
				zone->StartShutdownTimer(AUTHENTICATION_TIMEOUT * 1000);
			}

			SendPacket(pack);
			break;
		}
		break;
	}
	case ServerOP_WhoAllReply: {
		if (!is_zone_loaded)
			break;


		WhoAllReturnStruct* wars = (WhoAllReturnStruct*)pack->pBuffer;
		if (wars && wars->id != 0 && wars->id<0xFFFFFFFF) {
			Client* client = entity_list.GetClientByID(wars->id);
			if (client) {
				if (pack->size == 64)//no results
					client->MessageString(Chat::White, WHOALL_NO_RESULTS);
				else {
					auto outapp = new EQApplicationPacket(OP_WhoAllResponse, pack->size);
					memcpy(outapp->pBuffer, pack->pBuffer, pack->size);
					client->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
			else {
				LogDebug("[CLIENT] id=[{}], playerineqstring=[{}], playersinzonestring=[{}]. Dumping WhoAllReturnStruct:",
					wars->id, wars->playerineqstring, wars->playersinzonestring);
			}
		}
		else
			LogError("WhoAllReturnStruct: Could not get return struct!");
		break;
	}
	case ServerOP_EmoteMessage: {
		if (!is_zone_loaded)
			break;
		ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*)pack->pBuffer;
		if (sem->to[0] != 0) {
			if (strcasecmp(sem->to, zone->GetShortName()) == 0)
				entity_list.MessageStatus(sem->guilddbid, sem->minstatus, sem->type, (char*)sem->message);
			else {
				Client* client = entity_list.GetClientByName(sem->to);
				if (client) {
					char* newmessage = 0;
					if (strstr(sem->message, "^") == 0)
						client->Message(sem->type, (char*)sem->message);
					else {
						for (newmessage = strtok((char*)sem->message, "^"); newmessage != nullptr; newmessage = strtok(nullptr, "^"))
							client->Message(sem->type, newmessage);
					}
				}
			}
		}
		else {
			char* newmessage = 0;
			if (strstr(sem->message, "^") == 0)
				entity_list.MessageStatus(sem->guilddbid, sem->minstatus, sem->type, sem->message);
			else {
				for (newmessage = strtok((char*)sem->message, "^"); newmessage != nullptr; newmessage = strtok(nullptr, "^"))
					entity_list.MessageStatus(sem->guilddbid, sem->minstatus, sem->type, newmessage);
			}
		}
		break;
	}
	case ServerOP_Motd: {
		ServerMotd_Struct* smotd = (ServerMotd_Struct*)pack->pBuffer;
		EQApplicationPacket *outapp;
		outapp = new EQApplicationPacket(OP_MOTD);
		char tmp[500] = { 0 };
		sprintf(tmp, "%s", smotd->motd);

		outapp->size = strlen(tmp) + 1;
		outapp->pBuffer = new uchar[outapp->size];
		memset(outapp->pBuffer, 0, outapp->size);
		strcpy((char*)outapp->pBuffer, tmp);

		entity_list.QueueClients(0, outapp);
		safe_delete(outapp);

		break;
	}
	case ServerOP_ShutdownAll: {
		entity_list.Save();
		Shutdown();
		break;
	}
	case ServerOP_ZoneShutdown: {
		if (pack->size != sizeof(ServerZoneStateChange_struct)) {
			std::cout << "Wrong size on ServerOP_ZoneShutdown. Got: " << pack->size << ", Expected: " << sizeof(ServerZoneStateChange_struct) << std::endl;
			break;
		}
		// Annouce the change to the world
		if (!is_zone_loaded) {
			SetZoneData(0);
		}
		else {
			SendEmoteMessage(0, 0, 15, "Zone shutdown: %s", zone->GetLongName());

			ServerZoneStateChange_struct* zst = (ServerZoneStateChange_struct *)pack->pBuffer;
			std::cout << "Zone shutdown by " << zst->adminname << std::endl;
			Zone::Shutdown();
		}
		break;
	}
	case ServerOP_ZoneBootup: {
		if (pack->size != sizeof(ServerZoneStateChange_struct)) {
			std::cout << "Wrong size on ServerOP_ZoneBootup. Got: " << pack->size << ", Expected: " << sizeof(ServerZoneStateChange_struct) << std::endl;
			break;
		}
		ServerZoneStateChange_struct* zst = (ServerZoneStateChange_struct *)pack->pBuffer;
		if (is_zone_loaded) {
			SetZoneData(zone->GetZoneID(), zone->GetInstanceID());
			if (zst->zoneid == zone->GetZoneID()) {
				// This packet also doubles as "incoming client" notification, lets not shut down before they get here
				zone->StartShutdownTimer(AUTHENTICATION_TIMEOUT * 1000);
			}
			else {
				SendEmoteMessage(zst->adminname, 0, 0, "Zone bootup failed: Already running '%s'", zone->GetShortName());
			}
			break;
		}

		if (zst->adminname[0] != 0)
			std::cout << "Zone bootup by " << zst->adminname << std::endl;

		Zone::Bootup(zst->zoneid, zst->instanceid, zst->makestatic);
		break;
	}
	case ServerOP_ZoneIncClient: {
		if (pack->size != sizeof(ServerZoneIncomingClient_Struct)) {
			std::cout << "Wrong size on ServerOP_ZoneIncClient. Got: " << pack->size << ", Expected: " << sizeof(ServerZoneIncomingClient_Struct) << std::endl;
			break;
		}
		ServerZoneIncomingClient_Struct* szic = (ServerZoneIncomingClient_Struct*)pack->pBuffer;
		if (is_zone_loaded) {
			SetZoneData(zone->GetZoneID(), zone->GetInstanceID());

			if (szic->zoneid == zone->GetZoneID()) {
				auto client = entity_list.GetClientByLSID(szic->lsid);
				if (client) {
					client->Kick("Dropped by world CLE subsystem");
					client->Save();
				}

				zone->RemoveAuth(szic->lsid);
				zone->AddAuth(szic);
				// This packet also doubles as "incoming client" notification, lets not shut down before they get here
				zone->StartShutdownTimer(AUTHENTICATION_TIMEOUT * 1000);
			}
		}
		else {
			if ((Zone::Bootup(szic->zoneid, szic->instanceid))) {
				zone->AddAuth(szic);
			}
		}
		break;
	}
	case ServerOP_DropClient: {
		if (pack->size != sizeof(ServerZoneDropClient_Struct)) {
			break;
		}

		ServerZoneDropClient_Struct* drop = (ServerZoneDropClient_Struct*)pack->pBuffer;
		if (zone) {
			zone->RemoveAuth(drop->lsid);

			auto client = entity_list.GetClientByLSID(drop->lsid);
			if (client) {
				client->Kick("Dropped by world CLE subsystem");
				client->Save();
			}
		}
		break;
	}
	case ServerOP_ZonePlayer: {
		ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(szp->name);
		// printf("Zoning %s to %s(%u) - %u\n", client != nullptr ? client->GetCleanName() : "Unknown", szp->zone, ZoneID(szp->zone), szp->instance_id);
		if (client) {
			if (strcasecmp(szp->adminname, szp->name) == 0)
				client->Message(Chat::White, "Zoning to: %s", szp->zone);
			else if (client->GetAnon() == 1 && client->Admin() > szp->adminrank)
				break;
			else {
				SendEmoteMessage(szp->adminname, 0, 0, "Summoning %s to %s %1.1f, %1.1f, %1.1f", szp->name, szp->zone, szp->x_pos, szp->y_pos, szp->z_pos);
			}
			if (!szp->instance_id) {
				client->MovePC(ZoneID(szp->zone), szp->instance_id, szp->x_pos, szp->y_pos, szp->z_pos, client->GetHeading(), szp->ignorerestrictions, GMSummon);
			}
			else {
				if (database.GetInstanceID(client->CharacterID(), ZoneID(szp->zone)) == 0) {
					client->AssignToInstance(szp->instance_id);
					client->MovePC(ZoneID(szp->zone), szp->instance_id, szp->x_pos, szp->y_pos, szp->z_pos, client->GetHeading(), szp->ignorerestrictions, GMSummon);
				}
				else {
					client->RemoveFromInstance(database.GetInstanceID(client->CharacterID(), ZoneID(szp->zone)));
					client->AssignToInstance(szp->instance_id);
					client->MovePC(ZoneID(szp->zone), szp->instance_id, szp->x_pos, szp->y_pos, szp->z_pos, client->GetHeading(), szp->ignorerestrictions, GMSummon);
				}
			}
		}
		break;
	}
	case ServerOP_KickPlayer: {
		ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(skp->name);
		if (client) {
			if (skp->adminrank >= client->Admin()) {
				client->WorldKick();
				if (is_zone_loaded)
					SendEmoteMessage(skp->adminname, 0, 0, "Remote Kick: %s booted in zone %s.", skp->name, zone->GetShortName());
				else
					SendEmoteMessage(skp->adminname, 0, 0, "Remote Kick: %s booted.", skp->name);
			}
			else if (client->GetAnon() != 1)
				SendEmoteMessage(skp->adminname, 0, 0, "Remote Kick: Your avatar level is not high enough to kick %s", skp->name);
		}
		break;
	}
	case ServerOP_KillPlayer: {
		ServerKillPlayer_Struct* skp = (ServerKillPlayer_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(skp->target);
		if (client) {
			if (skp->admin >= client->Admin()) {
				client->GMKill();
				if (is_zone_loaded)
					SendEmoteMessage(skp->gmname, 0, 0, "Remote Kill: %s killed in zone %s.", skp->target, zone->GetShortName());
				else
					SendEmoteMessage(skp->gmname, 0, 0, "Remote Kill: %s killed.", skp->target);
			}
			else if (client->GetAnon() != 1)
				SendEmoteMessage(skp->gmname, 0, 0, "Remote Kill: Your avatar level is not high enough to kill %s", skp->target);
		}
		break;
	}

							  //hand all the guild related packets to the guild manager for processing.
	case ServerOP_OnlineGuildMembersResponse:
	case ServerOP_RefreshGuild:
		//	case ServerOP_GuildInvite:
	case ServerOP_DeleteGuild:
	case ServerOP_GuildCharRefresh:
	case ServerOP_GuildMemberUpdate:
	case ServerOP_GuildRankUpdate:
	case ServerOP_LFGuildUpdate:
		//	case ServerOP_GuildGMSet:
		//	case ServerOP_GuildGMSetRank:
		//	case ServerOP_GuildJoin:
		guild_mgr.ProcessWorldPacket(pack);
		break;

	case ServerOP_FlagUpdate: {
		Client* client = entity_list.GetClientByAccID(*((uint32*)pack->pBuffer));
		if (client) {
			client->UpdateAdmin();
		}
		break;
	}
	case ServerOP_GMGoto: {
		if (pack->size != sizeof(ServerGMGoto_Struct)) {
			std::cout << "Wrong size on ServerOP_GMGoto. Got: " << pack->size << ", Expected: " << sizeof(ServerGMGoto_Struct) << std::endl;
			break;
		}
		if (!is_zone_loaded)
			break;
		ServerGMGoto_Struct* gmg = (ServerGMGoto_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(gmg->gotoname);
		if (client) {
			SendEmoteMessage(gmg->myname, 0, 13, "Summoning you to: %s @ %s, %1.1f, %1.1f, %1.1f", client->GetName(), zone->GetShortName(), client->GetX(), client->GetY(), client->GetZ());
			auto outpack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
			ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*)outpack->pBuffer;
			strcpy(szp->adminname, gmg->myname);
			strcpy(szp->name, gmg->myname);
			strcpy(szp->zone, zone->GetShortName());
			szp->instance_id = zone->GetInstanceID();
			szp->x_pos = client->GetX();
			szp->y_pos = client->GetY();
			szp->z_pos = client->GetZ();
			SendPacket(outpack);
			safe_delete(outpack);
		}
		else {
			SendEmoteMessage(gmg->myname, 0, 13, "Error: %s not found", gmg->gotoname);
		}
		break;
	}
	case ServerOP_MultiLineMsg: {
		ServerMultiLineMsg_Struct* mlm = (ServerMultiLineMsg_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(mlm->to);
		if (client) {
			auto outapp = new EQApplicationPacket(OP_MultiLineMsg, strlen(mlm->message));
			strcpy((char*)outapp->pBuffer, mlm->message);
			client->QueuePacket(outapp);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_Uptime: {
		if (pack->size != sizeof(ServerUptime_Struct)) {
			std::cout << "Wrong size on ServerOP_Uptime. Got: " << pack->size << ", Expected: " << sizeof(ServerUptime_Struct) << std::endl;
			break;
		}
		ServerUptime_Struct* sus = (ServerUptime_Struct*)pack->pBuffer;
		uint32 ms = Timer::GetCurrentTime();
		uint32 d = ms / 86400000;
		ms -= d * 86400000;
		uint32 h = ms / 3600000;
		ms -= h * 3600000;
		uint32 m = ms / 60000;
		ms -= m * 60000;
		uint32 s = ms / 1000;
		if (d)
			this->SendEmoteMessage(sus->adminname, 0, 0, "Zone #%i Uptime: %02id %02ih %02im %02is", sus->zoneserverid, d, h, m, s);
		else if (h)
			this->SendEmoteMessage(sus->adminname, 0, 0, "Zone #%i Uptime: %02ih %02im %02is", sus->zoneserverid, h, m, s);
		else
			this->SendEmoteMessage(sus->adminname, 0, 0, "Zone #%i Uptime: %02im %02is", sus->zoneserverid, m, s);
	}
	case ServerOP_Petition: {
		std::cout << "Got Server Requested Petition List Refresh" << std::endl;
		ServerPetitionUpdate_Struct* sus = (ServerPetitionUpdate_Struct*)pack->pBuffer;
		// this was typoed to = instead of ==, not that it acts any different now though..
		if (sus->status == 0) petition_list.ReadDatabase();
		else if (sus->status == 1) petition_list.ReadDatabase(); // Until I fix this to be better....
		break;
	}
	case ServerOP_RezzPlayer: {
		RezzPlayer_Struct* srs = (RezzPlayer_Struct*)pack->pBuffer;
		if (srs->rezzopcode == OP_RezzRequest)
		{
			// The Rezz request has arrived in the zone the player to be rezzed is currently in,
			// so we send the request to their client which will bring up the confirmation box.
			Client* client = entity_list.GetClientByName(srs->rez.your_name);
			if (client)
			{
				if (client->IsRezzPending())
				{
					auto Response = new ServerPacket(ServerOP_RezzPlayerReject,
						strlen(srs->rez.rezzer_name) + 1);

					char *Buffer = (char *)Response->pBuffer;
					sprintf(Buffer, "%s", srs->rez.rezzer_name);
					worldserver.SendPacket(Response);
					safe_delete(Response);
					break;
				}
				//pendingrezexp is the amount of XP on the corpse. Setting it to a value >= 0
				//also serves to inform Client::OPRezzAnswer to expect a packet.
				client->SetPendingRezzData(srs->exp, srs->dbid, srs->rez.spellid, srs->rez.corpse_name);
				LogSpells("OP_RezzRequest in zone [{}] for [{}], spellid:[{}]",
					zone->GetShortName(), client->GetName(), srs->rez.spellid);
				auto outapp = new EQApplicationPacket(OP_RezzRequest,
					sizeof(Resurrect_Struct));
				memcpy(outapp->pBuffer, &srs->rez, sizeof(Resurrect_Struct));
				client->QueuePacket(outapp);
				safe_delete(outapp);
				break;
			}
		}
		if (srs->rezzopcode == OP_RezzComplete) {
			// We get here when the Rezz complete packet has come back via the world server
			// to the zone that the corpse is in.
			Corpse* corpse = entity_list.GetCorpseByName(srs->rez.corpse_name);
			if (corpse && corpse->IsCorpse()) {
				LogSpells("OP_RezzComplete received in zone [{}] for corpse [{}]",
					zone->GetShortName(), srs->rez.corpse_name);

				LogSpells("Found corpse. Marking corpse as rezzed if needed");
				// I don't know why Rezzed is not set to true in CompleteRezz().
				if (!IsEffectInSpell(srs->rez.spellid, SE_SummonToCorpse)) {
					corpse->IsRezzed(true);
					corpse->CompleteResurrection();
				}
			}
		}

		break;
	}
	case ServerOP_RezzPlayerReject:
	{
		char *Rezzer = (char *)pack->pBuffer;

		Client* c = entity_list.GetClientByName(Rezzer);

		if (c)
			c->MessageString(Chat::SpellWornOff, REZZ_ALREADY_PENDING);

		break;
	}
	case ServerOP_ZoneReboot: {
		std::cout << "Got Server Requested Zone reboot" << std::endl;
		ServerZoneReboot_Struct* zb = (ServerZoneReboot_Struct*)pack->pBuffer;
		break;
	}
	case ServerOP_SyncWorldTime: {
		if (zone != 0 && !zone->is_zone_time_localized) {
			LogInfo("[{}] Received Message SyncWorldTime", __FUNCTION__);

			eqTimeOfDay* newtime = (eqTimeOfDay*)pack->pBuffer;
			zone->zone_time.SetCurrentEQTimeOfDay(newtime->start_eqtime, newtime->start_realtime);
			auto outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
			TimeOfDay_Struct* time_of_day = (TimeOfDay_Struct*)outapp->pBuffer;
			zone->zone_time.GetCurrentEQTimeOfDay(time(0), time_of_day);
			entity_list.QueueClients(0, outapp, false);
			safe_delete(outapp);

			char time_message[255];
			time_t current_time = time(nullptr);
			TimeOfDay_Struct eq_time;
			zone->zone_time.GetCurrentEQTimeOfDay(current_time, &eq_time);

			sprintf(time_message, "EQTime [%02d:%s%d %s]",
				((eq_time.hour - 1) % 12) == 0 ? 12 : ((eq_time.hour - 1) % 12),
				(eq_time.minute < 10) ? "0" : "",
				eq_time.minute,
				(eq_time.hour >= 13) ? "pm" : "am"
				);

			LogInfo("Time Broadcast Packet: {}", time_message);
			zone->SetZoneHasCurrentTime(true);

		}
		if (zone && zone->is_zone_time_localized) {
			LogInfo("Received request to sync time from world, but our time is localized currently");
		}
		break;
	}
	case ServerOP_RefreshCensorship: {
		if (!EQ::ProfanityManager::LoadProfanityList(&database))
			LogError("Received request to refresh the profanity list..but, the action failed");
		break;
	}
	case ServerOP_ChangeWID: {
		if (pack->size != sizeof(ServerChangeWID_Struct)) {
			std::cout << "Wrong size on ServerChangeWID_Struct. Got: " << pack->size << ", Expected: " << sizeof(ServerChangeWID_Struct) << std::endl;
			break;
		}
		ServerChangeWID_Struct* scw = (ServerChangeWID_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByCharID(scw->charid);
		if (client)
			client->SetWID(scw->newwid);
		break;
	}
	case ServerOP_OOCMute: {
		oocmuted = *(pack->pBuffer);
		break;
	}
	case ServerOP_Revoke: {
		RevokeStruct* rev = (RevokeStruct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(rev->name);
		if (client)
		{
			SendEmoteMessage(rev->adminname, 0, 0, "%s: %srevoking %s", zone->GetShortName(), rev->toggle ? "" : "un", client->GetName());
			client->SetRevoked(rev->toggle);
		}
		break;
	}
	case ServerOP_GroupIDReply: {
		ServerGroupIDReply_Struct* ids = (ServerGroupIDReply_Struct*)pack->pBuffer;
		cur_groupid = ids->start;
		last_groupid = ids->end;
		break;
	}
	case ServerOP_GroupLeave: {
		ServerGroupLeave_Struct* gl = (ServerGroupLeave_Struct*)pack->pBuffer;
		if (zone) {
			if (gl->zoneid == zone->GetZoneID() && gl->instance_id == zone->GetInstanceID())
				break;

			entity_list.SendGroupLeave(gl->gid, gl->member_name);
		}
		break;
	}
	case ServerOP_GroupInvite: {
		// A player in another zone invited a player in this zone to join their group.
		GroupInvite_Struct* gis = (GroupInvite_Struct*)pack->pBuffer;

		Mob *Invitee = entity_list.GetMob(gis->invitee_name);

		if (Invitee && Invitee->IsClient() && Invitee->CastToClient()->MercOnlyOrNoGroup() && !Invitee->IsRaidGrouped())
		{
			auto outapp = new EQApplicationPacket(OP_GroupInvite, sizeof(GroupInvite_Struct));
			memcpy(outapp->pBuffer, gis, sizeof(GroupInvite_Struct));
			Invitee->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);
		}

		break;
	}
	case ServerOP_GroupFollow: {
		// Player in another zone accepted a group invitation from a player in this zone.
		ServerGroupFollow_Struct* sgfs = (ServerGroupFollow_Struct*)pack->pBuffer;

		Mob* Inviter = entity_list.GetClientByName(sgfs->gf.name1);

		if (Inviter && Inviter->IsClient())
		{
			Group* group = entity_list.GetGroupByClient(Inviter->CastToClient());

			if (!group)
			{
				//Make new group
				group = new Group(Inviter);

				if (!group)
				{
					break;
				}

				entity_list.AddGroup(group);

				if (group->GetID() == 0) {
					Inviter->Message(Chat::Red, "Unable to get new group id. Cannot create group.");
					break;
				}

				database.SetGroupID(Inviter->GetName(), group->GetID(), Inviter->CastToClient()->CharacterID(), false);
				database.SetGroupLeaderName(group->GetID(), Inviter->GetName());
				group->UpdateGroupAAs();

				if (Inviter->CastToClient()->ClientVersion() < EQ::versions::ClientVersion::SoD)
				{
					auto outapp =
						new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
					GroupJoin_Struct* outgj = (GroupJoin_Struct*)outapp->pBuffer;
					strcpy(outgj->membername, Inviter->GetName());
					strcpy(outgj->yourname, Inviter->GetName());
					outgj->action = groupActInviteInitial; // 'You have formed the group'.
					group->GetGroupAAs(&outgj->leader_aas);
					Inviter->CastToClient()->QueuePacket(outapp);
					safe_delete(outapp);
				}
				else
				{
					// SoD and later
					Inviter->CastToClient()->SendGroupCreatePacket();
					Inviter->CastToClient()->SendGroupLeaderChangePacket(Inviter->GetName());
					Inviter->CastToClient()->SendGroupJoinAcknowledge();
				}

				group->GetXTargetAutoMgr()->merge(*Inviter->CastToClient()->GetXTargetAutoMgr());
				Inviter->CastToClient()->GetXTargetAutoMgr()->clear();
				Inviter->CastToClient()->SetXTargetAutoMgr(group->GetXTargetAutoMgr());
			}

			if (!group)
			{
				break;
			}

			auto outapp = new EQApplicationPacket(OP_GroupFollow, sizeof(GroupGeneric_Struct));
			GroupGeneric_Struct *gg = (GroupGeneric_Struct *)outapp->pBuffer;
			strn0cpy(gg->name1, sgfs->gf.name1, sizeof(gg->name1));
			strn0cpy(gg->name2, sgfs->gf.name2, sizeof(gg->name2));
			Inviter->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);

			if (!group->AddMember(nullptr, sgfs->gf.name2, sgfs->CharacterID))
				break;

			if (Inviter->CastToClient()->IsLFP())
				Inviter->CastToClient()->UpdateLFP();

			auto pack2 = new ServerPacket(ServerOP_GroupJoin, sizeof(ServerGroupJoin_Struct));
			ServerGroupJoin_Struct* gj = (ServerGroupJoin_Struct*)pack2->pBuffer;
			gj->gid = group->GetID();
			gj->zoneid = zone->GetZoneID();
			gj->instance_id = zone->GetInstanceID();
			strn0cpy(gj->member_name, sgfs->gf.name2, sizeof(gj->member_name));
			worldserver.SendPacket(pack2);
			safe_delete(pack2);



			// Send acknowledgement back to the Invitee to let them know we have added them to the group.
			auto pack3 =
				new ServerPacket(ServerOP_GroupFollowAck, sizeof(ServerGroupFollowAck_Struct));
			ServerGroupFollowAck_Struct* sgfas = (ServerGroupFollowAck_Struct*)pack3->pBuffer;
			strn0cpy(sgfas->Name, sgfs->gf.name2, sizeof(sgfas->Name));
			worldserver.SendPacket(pack3);
			safe_delete(pack3);
		}
		break;
	}
	case ServerOP_GroupFollowAck: {
		// The Inviter (in another zone) has successfully added the Invitee (in this zone) to the group.
		ServerGroupFollowAck_Struct* sgfas = (ServerGroupFollowAck_Struct*)pack->pBuffer;

		Client *client = entity_list.GetClientByName(sgfas->Name);

		if (!client)
			break;

		uint32 groupid = database.GetGroupID(client->GetName());

		Group* group = nullptr;

		if (groupid > 0)
		{
			group = entity_list.GetGroupByID(groupid);

			if (!group)
			{	//nobody from our group is here... start a new group
				group = new Group(groupid);

				if (group->GetID() != 0)
					entity_list.AddGroup(group, groupid);
				else
					group = nullptr;
			}

			if (group)
				group->UpdatePlayer(client);
			else
			{
				if (client->GetMerc())
					database.SetGroupID(client->GetMerc()->GetCleanName(), 0, client->CharacterID(), true);
				database.SetGroupID(client->GetName(), 0, client->CharacterID(), false);	//cannot re-establish group, kill it
			}

		}

		if (group)
		{
			if (client->GetMerc())
			{
				client->GetMerc()->MercJoinClientGroup();
			}
			database.RefreshGroupFromDB(client);

			group->SendHPManaEndPacketsTo(client);

			// If the group leader is not set, pull the group leader information from the database.
			if (!group->GetLeader())
			{
				char ln[64];
				char MainTankName[64];
				char AssistName[64];
				char PullerName[64];
				char NPCMarkerName[64];
				char mentoree_name[64];
				int mentor_percent;
				GroupLeadershipAA_Struct GLAA;
				memset(ln, 0, 64);
				strcpy(ln, database.GetGroupLeadershipInfo(group->GetID(), ln, MainTankName, AssistName, PullerName, NPCMarkerName, mentoree_name, &mentor_percent, &GLAA));
				Client *lc = entity_list.GetClientByName(ln);
				if (lc)
					group->SetLeader(lc);

				group->SetMainTank(MainTankName);
				group->SetMainAssist(AssistName);
				group->SetPuller(PullerName);
				group->SetNPCMarker(NPCMarkerName);
				group->SetGroupAAs(&GLAA);
				group->SetGroupMentor(mentor_percent, mentoree_name);
				client->JoinGroupXTargets(group);
			}
		}
		else if (client->GetMerc())
		{
			client->GetMerc()->MercJoinClientGroup();
		}
		break;

	}
	case ServerOP_GroupCancelInvite: {

		GroupCancel_Struct* sgcs = (GroupCancel_Struct*)pack->pBuffer;

		Mob* Inviter = entity_list.GetClientByName(sgcs->name1);

		if (Inviter && Inviter->IsClient())
		{
			auto outapp = new EQApplicationPacket(OP_GroupCancelInvite, sizeof(GroupCancel_Struct));
			memcpy(outapp->pBuffer, sgcs, sizeof(GroupCancel_Struct));
			Inviter->CastToClient()->QueuePacket(outapp);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_GroupJoin: {
		ServerGroupJoin_Struct* gj = (ServerGroupJoin_Struct*)pack->pBuffer;
		if (zone) {
			if (gj->zoneid == zone->GetZoneID() && gj->instance_id == zone->GetInstanceID())
				break;

			Group* g = entity_list.GetGroupByID(gj->gid);
			if (g)
				g->AddMember(gj->member_name);

			entity_list.SendGroupJoin(gj->gid, gj->member_name);
		}
		break;
	}

	case ServerOP_ForceGroupUpdate: {
		ServerForceGroupUpdate_Struct* fgu = (ServerForceGroupUpdate_Struct*)pack->pBuffer;
		if (zone) {
			if (fgu->origZoneID == zone->GetZoneID() && fgu->instance_id == zone->GetInstanceID())
				break;

			entity_list.ForceGroupUpdate(fgu->gid);
		}
		break;
	}

	case ServerOP_OOZGroupMessage: {
		ServerGroupChannelMessage_Struct* gcm = (ServerGroupChannelMessage_Struct*)pack->pBuffer;
		if (zone) {
			if (gcm->zoneid == zone->GetZoneID() && gcm->instanceid == zone->GetInstanceID())
				break;

			entity_list.GroupMessage(gcm->groupid, gcm->from, gcm->message);
		}
		break;
	}
	case ServerOP_DisbandGroup: {
		ServerDisbandGroup_Struct* sd = (ServerDisbandGroup_Struct*)pack->pBuffer;
		if (zone) {
			if (sd->zoneid == zone->GetZoneID() && sd->instance_id == zone->GetInstanceID())
				break;

			Group *g = entity_list.GetGroupByID(sd->groupid);
			if (g)
				g->DisbandGroup();
		}
		break;
	}
	case ServerOP_RaidAdd: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				r->SendRaidAddAll(rga->playername);
			}
		}
		break;
	}

	case ServerOP_RaidRemove: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->SendRaidRemoveAll(rga->playername);
				Client *rem = entity_list.GetClientByName(rga->playername);
				if (rem) {
					rem->LeaveRaidXTargets(r);
					r->SendRaidDisband(rem);
				}
				r->LearnMembers();
				r->VerifyRaid();
			}
		}
		break;
	}

	case ServerOP_RaidDisband: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->SendRaidDisbandAll();
				r->LearnMembers();
				r->VerifyRaid();
			}
		}
		break;
	}

	case ServerOP_RaidLockFlag: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->GetRaidDetails(); //update our details
				if (rga->gid)
					r->SendRaidLock();
				else
					r->SendRaidUnlock();
			}
		}
		break;
	}

	case ServerOP_RaidChangeGroup: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				Client *c = entity_list.GetClientByName(rga->playername);
				if (c) {
					r->SendRaidDisband(c);
					r->SendRaidRemoveAll(rga->playername);
					r->SendRaidCreate(c);
					r->SendMakeLeaderPacketTo(r->leadername, c);
					r->SendBulkRaid(c);
					r->SendRaidAddAll(rga->playername);
					if (r->IsLocked()) { r->SendRaidLockTo(c); }
				}
				else {
					r->SendRaidRemoveAll(rga->playername);
					r->SendRaidAddAll(rga->playername);
				}
			}
		}
		break;
	}

	case ServerOP_UpdateGroup: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->GroupUpdate(rga->gid, false);
			}
		}
		break;
	}

	case ServerOP_RaidGroupLeader: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;
		}
		break;
	}

	case ServerOP_RaidLeader: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				Client *c = entity_list.GetClientByName(rga->playername);
				strn0cpy(r->leadername, rga->playername, 64);
				if (c) {
					r->SetLeader(c);
				}
				r->LearnMembers();
				r->VerifyRaid();
				r->SendMakeLeaderPacket(rga->playername);
			}
		}
		break;
	}

	case ServerOP_DetailsChange: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->GetRaidDetails();
				r->LearnMembers();
				r->VerifyRaid();
			}
		}
		break;
	}

	case ServerOP_RaidGroupDisband: {
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		if (zone) {
			if (rga->zoneid == zone->GetZoneID() && rga->instance_id == zone->GetInstanceID())
				break;

			Client *c = entity_list.GetClientByName(rga->playername);
			if (c)
			{
				auto outapp =
					new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
				GroupUpdate_Struct* gu = (GroupUpdate_Struct*)outapp->pBuffer;
				gu->action = groupActDisband;
				strn0cpy(gu->leadersname, c->GetName(), 64);
				strn0cpy(gu->yourname, c->GetName(), 64);
				c->FastQueuePacket(&outapp);
			}
		}
		break;
	}

	case ServerOP_RaidGroupAdd: {
		ServerRaidGroupAction_Struct* rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
		if (zone) {
			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				GroupJoin_Struct* gj = (GroupJoin_Struct*)outapp->pBuffer;
				strn0cpy(gj->membername, rga->membername, 64);
				gj->action = groupActJoin;

				for (int x = 0; x < MAX_RAID_MEMBERS; x++)
				{
					if (r->members[x].member)
					{
						if (strcmp(r->members[x].member->GetName(), rga->membername) != 0) {
							if ((rga->gid < 12) && rga->gid == r->members[x].GroupNumber)
							{
								strn0cpy(gj->yourname, r->members[x].member->GetName(), 64);
								r->members[x].member->QueuePacket(outapp);
							}
						}
					}
				}
				safe_delete(outapp);
			}
		}
		break;
	}

	case ServerOP_RaidGroupRemove: {
		ServerRaidGroupAction_Struct* rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
		if (zone) {
			Raid *r = entity_list.GetRaidByID(rga->rid);
			if (r) {
				r->LearnMembers();
				r->VerifyRaid();
				auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				GroupJoin_Struct* gj = (GroupJoin_Struct*)outapp->pBuffer;
				strn0cpy(gj->membername, rga->membername, 64);
				gj->action = groupActLeave;

				for (int x = 0; x < MAX_RAID_MEMBERS; x++)
				{
					if (r->members[x].member)
					{
						if (strcmp(r->members[x].member->GetName(), rga->membername) != 0) {
							if ((rga->gid < 12) && rga->gid == r->members[x].GroupNumber)
							{
								strn0cpy(gj->yourname, r->members[x].member->GetName(), 64);
								r->members[x].member->QueuePacket(outapp);
							}
						}
					}
				}
				safe_delete(outapp);
			}
		}
		break;
	}

	case ServerOP_RaidGroupSay: {
		ServerRaidMessage_Struct* rmsg = (ServerRaidMessage_Struct*)pack->pBuffer;
		if (zone) {
			Raid *r = entity_list.GetRaidByID(rmsg->rid);
			if (r)
			{
				for (int x = 0; x < MAX_RAID_MEMBERS; x++)
				{
					if (r->members[x].member) {
						if (strcmp(rmsg->from, r->members[x].member->GetName()) != 0)
						{
							if (r->members[x].GroupNumber == rmsg->gid) {
								if (r->members[x].member->GetFilter(FilterGroupChat) != 0)
								{
									r->members[x].member->ChannelMessageSend(rmsg->from, r->members[x].member->GetName(), ChatChannel_Group, rmsg->language, rmsg->lang_skill, rmsg->message);
								}
							}
						}
					}
				}
			}
		}
		break;
	}

	case ServerOP_RaidSay: {
		ServerRaidMessage_Struct* rmsg = (ServerRaidMessage_Struct*)pack->pBuffer;
		if (zone)
		{
			Raid *r = entity_list.GetRaidByID(rmsg->rid);
			if (r)
			{
				for (int x = 0; x < MAX_RAID_MEMBERS; x++)
				{
					if (r->members[x].member) {
						if (strcmp(rmsg->from, r->members[x].member->GetName()) != 0)
						{
							if (r->members[x].member->GetFilter(FilterGroupChat) != 0)
							{
								r->members[x].member->ChannelMessageSend(rmsg->from, r->members[x].member->GetName(), ChatChannel_Raid, rmsg->language, rmsg->lang_skill, rmsg->message);
							}
						}
					}
				}
			}
		}
		break;
	}

	case ServerOP_RaidMOTD: {
		ServerRaidMOTD_Struct *rmotd = (ServerRaidMOTD_Struct *)pack->pBuffer;
		if (!zone)
			break;
		Raid *r = entity_list.GetRaidByID(rmotd->rid);
		if (!r)
			break;
		r->SetRaidMOTD(std::string(rmotd->motd));
		r->SendRaidMOTD();
		break;
	}

	case ServerOP_SpawnPlayerCorpse: {
		SpawnPlayerCorpse_Struct* s = (SpawnPlayerCorpse_Struct*)pack->pBuffer;
		Corpse* NewCorpse = database.LoadCharacterCorpse(s->player_corpse_id);
		if (NewCorpse)
			NewCorpse->Spawn();
		else
			LogError("Unable to load player corpse id [{}] for zone [{}]", s->player_corpse_id, zone->GetShortName());

		break;
	}
	case ServerOP_Consent: {
		ServerOP_Consent_Struct* s = (ServerOP_Consent_Struct*)pack->pBuffer;

		bool found_corpse = false;
		for (auto const& it : entity_list.GetCorpseList()) {
			if (it.second->IsPlayerCorpse() && strcmp(it.second->GetOwnerName(), s->ownername) == 0) {
				if (s->consent_type == EQ::consent::Normal) {
					if (s->permission == 1) {
						it.second->AddConsentName(s->grantname);
					}
					else {
						it.second->RemoveConsentName(s->grantname);
					}
				}
				else if (s->consent_type == EQ::consent::Group) {
					it.second->SetConsentGroupID(s->consent_id);
				}
				else if (s->consent_type == EQ::consent::Raid) {
					it.second->SetConsentRaidID(s->consent_id);
				}
				else if (s->consent_type == EQ::consent::Guild) {
					it.second->SetConsentGuildID(s->consent_id);
				}
				found_corpse = true;
			}
		}

		if (found_corpse) {
			// forward the grant/deny message for this zone to both owner and granted
			auto outapp = new ServerPacket(ServerOP_Consent_Response, sizeof(ServerOP_Consent_Struct));
			ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)outapp->pBuffer;
			memcpy(outapp->pBuffer, s, sizeof(ServerOP_Consent_Struct));
			if (zone) {
				strn0cpy(scs->zonename, zone->GetLongName(), sizeof(scs->zonename));
			}
			worldserver.SendPacket(outapp);
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_Consent_Response: {
		ServerOP_Consent_Struct* s = (ServerOP_Consent_Struct*)pack->pBuffer;
		Client* owner_client = entity_list.GetClientByName(s->ownername);
		Client* grant_client = nullptr;
		if (s->consent_type == EQ::consent::Normal) {
			grant_client = entity_list.GetClientByName(s->grantname);
		}
		if (owner_client || grant_client) {
			auto outapp = new EQApplicationPacket(OP_ConsentResponse, sizeof(ConsentResponse_Struct));
			ConsentResponse_Struct* crs = (ConsentResponse_Struct*)outapp->pBuffer;
			strn0cpy(crs->grantname, s->grantname, sizeof(crs->grantname));
			strn0cpy(crs->ownername, s->ownername, sizeof(crs->ownername));
			crs->permission = s->permission;
			strn0cpy(crs->zonename, s->zonename, sizeof(crs->zonename));
			if (owner_client) {
				owner_client->QueuePacket(outapp); // confirmation message to the owner
			}
			if (grant_client) {
				grant_client->QueuePacket(outapp); // message to the client being granted/denied
			}
			safe_delete(outapp);
		}
		break;
	}
	case ServerOP_ReloadTasks: {
		if (RuleB(Tasks, EnableTaskSystem)) {
			HandleReloadTasks(pack);
		}
		break;
	}
	case ServerOP_LFGMatches: {
		HandleLFGMatches(pack);
		break;
	}
	case ServerOP_LFPMatches: {
		HandleLFPMatches(pack);
		break;
	}

	case ServerOP_UpdateSpawn: {
		if (zone)
		{
			UpdateSpawnTimer_Struct *ust = (UpdateSpawnTimer_Struct*)pack->pBuffer;
			LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
			iterator.Reset();
			while (iterator.MoreElements())
			{
				if (iterator.GetData()->GetID() == ust->id)
				{
					if (!iterator.GetData()->NPCPointerValid())
					{
						iterator.GetData()->SetTimer(ust->duration);
					}
					break;
				}
				iterator.Advance();
			}
		}
		break;
	}

	case ServerOP_InstanceUpdateTime:
	{
		ServerInstanceUpdateTime_Struct *iut = (ServerInstanceUpdateTime_Struct*)pack->pBuffer;
		if (zone)
		{
			if (zone->GetInstanceID() == iut->instance_id)
			{
				zone->SetInstanceTimer(iut->new_duration);
			}
		}
		break;
	}

	case ServerOP_DepopAllPlayersCorpses:
	{
		ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct *)pack->pBuffer;

		if (zone && !((zone->GetZoneID() == sdapcs->ZoneID) && (zone->GetInstanceID() == sdapcs->InstanceID)))
			entity_list.RemoveAllCorpsesByCharID(sdapcs->CharacterID);

		break;

	}

	case ServerOP_DepopPlayerCorpse:
	{
		ServerDepopPlayerCorpse_Struct *sdpcs = (ServerDepopPlayerCorpse_Struct *)pack->pBuffer;

		if (zone && !((zone->GetZoneID() == sdpcs->ZoneID) && (zone->GetInstanceID() == sdpcs->InstanceID)))
			entity_list.RemoveCorpseByDBID(sdpcs->DBID);

		break;

	}

	case ServerOP_ReloadTitles:
	{
		title_manager.LoadTitles();
		break;
	}

	case ServerOP_SpawnStatusChange:
	{
		if (zone)
		{
			ServerSpawnStatusChange_Struct *ssc = (ServerSpawnStatusChange_Struct*)pack->pBuffer;
			LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
			iterator.Reset();
			Spawn2 *found_spawn = nullptr;
			while (iterator.MoreElements())
			{
				Spawn2* cur = iterator.GetData();
				if (cur->GetID() == ssc->id)
				{
					found_spawn = cur;
					break;
				}
				iterator.Advance();
			}

			if (found_spawn)
			{
				if (ssc->new_status == 0)
				{
					found_spawn->Disable();
				}
				else
				{
					found_spawn->Enable();
				}
			}
		}
		break;
	}

	case ServerOP_QGlobalUpdate:
	{
		if (pack->size != sizeof(ServerQGlobalUpdate_Struct))
		{
			break;
		}

		if (zone)
		{
			ServerQGlobalUpdate_Struct *qgu = (ServerQGlobalUpdate_Struct*)pack->pBuffer;
			if (qgu->from_zone_id != zone->GetZoneID() || qgu->from_instance_id != zone->GetInstanceID())
			{
				QGlobal temp;
				temp.npc_id = qgu->npc_id;
				temp.char_id = qgu->char_id;
				temp.zone_id = qgu->zone_id;
				temp.expdate = qgu->expdate;
				temp.name.assign(qgu->name);
				temp.value.assign(qgu->value);
				entity_list.UpdateQGlobal(qgu->id, temp);
				zone->UpdateQGlobal(qgu->id, temp);
			}
		}
		break;
	}

	case ServerOP_QGlobalDelete:
	{
		if (pack->size != sizeof(ServerQGlobalDelete_Struct))
		{
			break;
		}

		if (zone)
		{
			ServerQGlobalDelete_Struct *qgd = (ServerQGlobalDelete_Struct*)pack->pBuffer;
			if (qgd->from_zone_id != zone->GetZoneID() || qgd->from_instance_id != zone->GetInstanceID())
			{
				entity_list.DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
				zone->DeleteQGlobal(std::string((char*)qgd->name), qgd->npc_id, qgd->char_id, qgd->zone_id);
			}
		}
		break;
	}

	case ServerOP_AdventureRequestAccept:
	{
		ServerAdventureRequestAccept_Struct *ars = (ServerAdventureRequestAccept_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(ars->leader);
		if (c)
		{
			c->NewAdventure(ars->id, ars->theme, ars->text, ars->member_count, (const char*)(pack->pBuffer + sizeof(ServerAdventureRequestAccept_Struct)));
			c->ClearPendingAdventureRequest();
		}
		break;
	}

	case ServerOP_AdventureRequestDeny:
	{
		ServerAdventureRequestDeny_Struct *ars = (ServerAdventureRequestDeny_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(ars->leader);
		if (c)
		{
			c->SendAdventureError(ars->reason);
			c->ClearPendingAdventureRequest();
		}
		break;
	}

	case ServerOP_AdventureCreateDeny:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureData();
			c->ClearPendingAdventureCreate();
		}
		break;
	}

	case ServerOP_AdventureData:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearAdventureData();
			auto adv_data = new char[pack->size];
			memcpy(adv_data, pack->pBuffer, pack->size);
			c->SetAdventureData(adv_data);
			c->ClearPendingAdventureData();
			c->ClearPendingAdventureCreate();
			c->SendAdventureDetails();
		}
		break;
	}

	case ServerOP_AdventureDataClear:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			if (c->HasAdventureData())
			{
				c->ClearAdventureData();
				c->SendAdventureError("You are not currently assigned to an adventure.");
			}
		}
		break;
	}

	case ServerOP_AdventureClickDoorReply:
	{
		ServerPlayerClickedAdventureDoorReply_Struct *adr = (ServerPlayerClickedAdventureDoorReply_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(adr->player);
		if (c)
		{
			c->ClearPendingAdventureDoorClick();
			c->MovePC(adr->zone_id, adr->instance_id, adr->x, adr->y, adr->z, adr->h, 0);
		}
		break;
	}

	case ServerOP_AdventureClickDoorError:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureDoorClick();
			c->MessageString(Chat::Red, 5141);
		}
		break;
	}

	case ServerOP_AdventureLeaveReply:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureLeave();
			c->ClearCurrentAdventure();
		}
		break;
	}

	case ServerOP_AdventureLeaveDeny:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			c->ClearPendingAdventureLeave();
			c->Message(Chat::Red, "You cannot leave this adventure at this time.");
		}
		break;
	}

	case ServerOP_AdventureCountUpdate:
	{
		ServerAdventureCountUpdate_Struct *ac = (ServerAdventureCountUpdate_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(ac->player);
		if (c)
		{
			c->SendAdventureCount(ac->count, ac->total);
		}
		break;
	}

	case ServerOP_AdventureZoneData:
	{
		if (zone)
		{
			safe_delete(zone->adv_data);
			zone->adv_data = new char[pack->size];
			memcpy(zone->adv_data, pack->pBuffer, pack->size);
			ServerZoneAdventureDataReply_Struct* ds = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
		}
		break;
	}

	case ServerOP_AdventureFinish:
	{
		ServerAdventureFinish_Struct *af = (ServerAdventureFinish_Struct*)pack->pBuffer;
		Client *c = entity_list.GetClientByName(af->player);
		if (c)
		{
			c->AdventureFinish(af->win, af->theme, af->points);
		}
		break;
	}

	case ServerOP_AdventureLeaderboard:
	{
		Client *c = entity_list.GetClientByName((const char*)pack->pBuffer);
		if (c)
		{
			auto outapp = new EQApplicationPacket(OP_AdventureLeaderboardReply,
				sizeof(AdventureLeaderboard_Struct));
			memcpy(outapp->pBuffer, pack->pBuffer + 64, sizeof(AdventureLeaderboard_Struct));
			c->FastQueuePacket(&outapp);
		}
		break;
	}
	case ServerOP_ReloadRules: {
		worldserver.SendEmoteMessage(
			0, 0, 100, 15,
			"Rules reloaded for Zone: '%s' Instance ID: %u",
			(zone ? zone->GetLongName() : StringFormat("Null zone pointer [pid]:[%i]", getpid()).c_str()),
			(zone ? zone->GetInstanceID() : 0xFFFFFFFFF)
		);
		RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset(), true);
		break;
	}
	case ServerOP_ReloadLogs: {
		database.LoadLogSettings(LogSys.log_settings);
		break;
	}
	case ServerOP_ReloadPerlExportSettings: {
		parse->LoadPerlEventExportSettings(parse->perl_event_export_settings);
		break;
	}
	case ServerOP_CameraShake:
	{
		if (zone)
		{
			ServerCameraShake_Struct *scss = (ServerCameraShake_Struct*)pack->pBuffer;
			entity_list.CameraEffect(scss->duration, scss->intensity);
		}
		break;
	}
	case ServerOP_QueryServGeneric:
	{
		pack->SetReadPosition(8);
		char From[64];
		pack->ReadString(From);

		Client *c = entity_list.GetClientByName(From);

		if (!c)
			return;

		uint32 Type = pack->ReadUInt32();;

		switch (Type)
		{
		case QSG_LFGuild:
		{
			c->HandleLFGuildResponse(pack);
			break;
		}

		default:
			break;
		}

		break;
	}
	case ServerOP_UCSServerStatusReply:
	{
		auto ucsss = (UCSServerStatus_Struct *) pack->pBuffer;
		if (zone) {
			zone->SetUCSServerAvailable((ucsss->available != 0), ucsss->timestamp);
			LogInfo("UCS Server is now [{}]", (ucsss->available == 1 ? "online" : "offline"));
		}
		break;
	}
	case ServerOP_CZCastSpellPlayer:
	{
		CZCastSpellPlayer_Struct* CZSC = (CZCastSpellPlayer_Struct*) pack->pBuffer;
		Client* client = entity_list.GetClientByCharID(CZSC->character_id);
		if (client) {
			client->SpellFinished(CZSC->spell_id, client);
		}
		break;
	}
	case ServerOP_CZCastSpellGroup:
	{
		CZCastSpellGroup_Struct* CZSC = (CZCastSpellGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZSC->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->SpellFinished(CZSC->spell_id, group_member);
				}
			}
		}
		break;
	}
	case ServerOP_CZCastSpellRaid:
	{
		CZCastSpellRaid_Struct* CZSC = (CZCastSpellRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZSC->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->SpellFinished(CZSC->spell_id, raid_member);
				}
			}
		}
		break;
	}
	case ServerOP_CZCastSpellGuild:
	{
		CZCastSpellGuild_Struct* CZSC = (CZCastSpellGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZSC->guild_id) {
				client.second->SpellFinished(CZSC->spell_id, client.second);
			}
		}
		break;
	}
	case ServerOP_CZMarqueePlayer:
	{
		CZMarqueePlayer_Struct* CZMS = (CZMarqueePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZMS->character_id);
		std::string message = CZMS->message;
		if (client) {
			client->SendMarqueeMessage(CZMS->type, CZMS->priority, CZMS->fade_in, CZMS->fade_out, CZMS->duration, message);
		}
		break;
	}
	case ServerOP_CZMarqueeGroup:
	{
		CZMarqueeGroup_Struct* CZMS = (CZMarqueeGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZMS->group_id);
		std::string message = CZMS->message;
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->SendMarqueeMessage(CZMS->type, CZMS->priority, CZMS->fade_in, CZMS->fade_out, CZMS->duration, message);
				}
			}
		}
		break;
	}
	case ServerOP_CZMarqueeRaid:
	{
		CZMarqueeRaid_Struct* CZMS = (CZMarqueeRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZMS->raid_id);
		std::string message = CZMS->message;
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->SendMarqueeMessage(CZMS->type, CZMS->priority, CZMS->fade_in, CZMS->fade_out, CZMS->duration, message);
				}
			}
		}
		break;
	}
	case ServerOP_CZMarqueeGuild:
	{
		CZMarqueeGuild_Struct* CZMS = (CZMarqueeGuild_Struct*) pack->pBuffer;
		std::string message = CZMS->message;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZMS->guild_id) {
				client.second->SendMarqueeMessage(CZMS->type, CZMS->priority, CZMS->fade_in, CZMS->fade_out, CZMS->duration, message);
			}
		}
		break;
	}
	case ServerOP_CZMessagePlayer:
	{
		CZMessagePlayer_Struct* CZCS = (CZMessagePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByName(CZCS->character_name);
		if (client) {
			client->Message(CZCS->type, CZCS->message);
		}
		break;
	}
	case ServerOP_CZMessageGroup:
	{
		CZMessageGroup_Struct* CZGM = (CZMessageGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZGM->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->Message(CZGM->type, CZGM->message);
				}
			}
		}
		break;
	}
	case ServerOP_CZMessageRaid:
	{
		CZMessageRaid_Struct* CZRM = (CZMessageRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZRM->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->Message(CZRM->type, CZRM->message);
				}
			}
		}
		break;
	}
	case ServerOP_CZMessageGuild:
	{
		CZMessageGuild_Struct* CZGM = (CZMessageGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZGM->guild_id) {
				client.second->Message(CZGM->type, CZGM->message);
			}
		}
		break;
	}
	case ServerOP_CZMovePlayer:
	{
		CZMovePlayer_Struct* CZMP = (CZMovePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZMP->character_id);
		if (client) {
			client->MoveZone(CZMP->zone_short_name);
		}
		break;
	}
	case ServerOP_CZMoveGroup:
	{
		CZMoveGroup_Struct* CZMG = (CZMoveGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZMG->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->MoveZone(CZMG->zone_short_name);
				}
			}
		}
		break;
	}
	case ServerOP_CZMoveRaid:
	{
		CZMoveRaid_Struct* CZMR = (CZMoveRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZMR->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->MoveZone(CZMR->zone_short_name);
				}
			}
		}
		break;
	}
	case ServerOP_CZMoveGuild:
	{
		CZMoveGuild_Struct* CZMG = (CZMoveGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZMG->guild_id) {
				client.second->MoveZone(CZMG->zone_short_name);
			}
		}
		break;
	}

	case ServerOP_CZMoveInstancePlayer:
	{
		CZMoveInstancePlayer_Struct* CZMP = (CZMoveInstancePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZMP->character_id);
		if (client) {
			client->MoveZoneInstance(CZMP->instance_id);
		}
		break;
	}
	case ServerOP_CZMoveInstanceGroup:
	{
		CZMoveInstanceGroup_Struct* CZMG = (CZMoveInstanceGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZMG->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->MoveZoneInstance(CZMG->instance_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZMoveInstanceRaid:
	{
		CZMoveInstanceRaid_Struct* CZMR = (CZMoveInstanceRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZMR->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->MoveZoneInstance(CZMR->instance_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZMoveInstanceGuild:
	{
		CZMoveInstanceGuild_Struct* CZMG = (CZMoveInstanceGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZMG->guild_id) {
				client.second->MoveZoneInstance(CZMG->instance_id);
			}
		}
		break;
	}
	case ServerOP_CZRemoveSpellPlayer:
	{
		CZRemoveSpellPlayer_Struct* CZRS = (CZRemoveSpellPlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZRS->character_id);
		if (client) {
			client->BuffFadeBySpellID(CZRS->spell_id);
		}
		break;
	}
	case ServerOP_CZRemoveSpellGroup:
	{
		CZRemoveSpellGroup_Struct* CZRS = (CZRemoveSpellGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZRS->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->BuffFadeBySpellID(CZRS->spell_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZRemoveSpellRaid:
	{
		CZRemoveSpellRaid_Struct* CZRS = (CZRemoveSpellRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZRS->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->BuffFadeBySpellID(CZRS->spell_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZRemoveSpellGuild:
	{
		CZRemoveSpellGuild_Struct* CZRS = (CZRemoveSpellGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZRS->guild_id) {
				client.second->BuffFadeBySpellID(CZRS->spell_id);
			}
		}
		break;
	}
	case ServerOP_CZSetEntityVariableByClientName:
	{
		CZSetEntVarByClientName_Struct* CZCS = (CZSetEntVarByClientName_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByName(CZCS->character_name);
		if (client) {
			client->SetEntityVariable(CZCS->variable_name, CZCS->variable_value);
		}
		break;
	}
	case ServerOP_CZSetEntityVariableByGroupID:
	{
		CZSetEntVarByGroupID_Struct* CZCS = (CZSetEntVarByGroupID_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZCS->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->SetEntityVariable(CZCS->variable_name, CZCS->variable_value);
				}
			}
		}
		break;
	}
	case ServerOP_CZSetEntityVariableByRaidID:
	{
		CZSetEntVarByRaidID_Struct* CZCS = (CZSetEntVarByRaidID_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZCS->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->SetEntityVariable(CZCS->variable_name, CZCS->variable_value);
				}
			}
		}
		break;
	}
	case ServerOP_CZSetEntityVariableByGuildID:
	{
		CZSetEntVarByGuildID_Struct* CZCS = (CZSetEntVarByGuildID_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZCS->guild_id) {
				client.second->SetEntityVariable(CZCS->variable_name, CZCS->variable_value);
			}
		}
		break;
	}
	case ServerOP_CZSetEntityVariableByNPCTypeID:
	{
		CZSetEntVarByNPCTypeID_Struct* CZM = (CZSetEntVarByNPCTypeID_Struct*) pack->pBuffer;
		auto npc = entity_list.GetNPCByNPCTypeID(CZM->npctype_id);
		if (npc != 0) {
			npc->SetEntityVariable(CZM->variable_name, CZM->variable_value);
		}
		break;
	}
	case ServerOP_CZSignalNPC:
	{
		CZNPCSignal_Struct* CZCN = (CZNPCSignal_Struct*) pack->pBuffer;
		auto npc = entity_list.GetNPCByNPCTypeID(CZCN->npctype_id);
		if (npc != 0) {
			npc->SignalNPC(CZCN->signal);
		}
		break;
	}
	case ServerOP_CZSignalClient:
	{
		CZClientSignal_Struct* CZCS = (CZClientSignal_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZCS->character_id);
		if (client) {
			client->Signal(CZCS->signal);
		}
		break;
	}
	case ServerOP_CZSignalGroup:
	{
		CZGroupSignal_Struct* CZGS = (CZGroupSignal_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZGS->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->Signal(CZGS->signal);
				}
			}
		}
		break;
	}
	case ServerOP_CZSignalRaid:
	{
		CZRaidSignal_Struct* CZRS = (CZRaidSignal_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZRS->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->Signal(CZRS->signal);
				}
			}
		}
		break;
	}
	case ServerOP_CZSignalGuild:
	{
		CZGuildSignal_Struct* CZGS = (CZGuildSignal_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZGS->guild_id) {
				client.second->Signal(CZGS->signal);
			}
		}
		break;
	}
	case ServerOP_CZSignalClientByName:
	{
		CZClientSignalByName_Struct* CZCS = (CZClientSignalByName_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByName(CZCS->character_name);
		if (client) {
			client->Signal(CZCS->signal);
		}
		break;
	}
	case ServerOP_CZTaskAssignPlayer:
	{
		CZTaskAssignPlayer_Struct* CZTA = (CZTaskAssignPlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZTA->character_id);
		if (client) {
			client->AssignTask(CZTA->task_id, CZTA->npc_entity_id, CZTA->enforce_level_requirement);
		}
		break;
	}
	case ServerOP_CZTaskAssignGroup:
	{
		CZTaskAssignGroup_Struct* CZTA = (CZTaskAssignGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZTA->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->AssignTask(CZTA->task_id, CZTA->npc_entity_id, CZTA->enforce_level_requirement);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskAssignRaid:
	{
		CZTaskAssignRaid_Struct* CZTA = (CZTaskAssignRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZTA->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->AssignTask(CZTA->task_id, CZTA->npc_entity_id, CZTA->enforce_level_requirement);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskAssignGuild:
	{
		CZTaskAssignGuild_Struct* CZTA = (CZTaskAssignGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZTA->guild_id) {
				client.second->AssignTask(CZTA->task_id, CZTA->npc_entity_id, CZTA->enforce_level_requirement);
			}
		}
		break;
	}
	case ServerOP_CZTaskActivityResetPlayer:
	{
		CZTaskActivityResetPlayer_Struct* CZRA = (CZTaskActivityResetPlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZRA->character_id);
		if (client) {
			client->ResetTaskActivity(CZRA->task_id, CZRA->activity_id);
		}
		break;
	}
	case ServerOP_CZTaskActivityResetGroup:
	{
		CZTaskActivityResetGroup_Struct* CZRA = (CZTaskActivityResetGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZRA->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->ResetTaskActivity(CZRA->task_id, CZRA->activity_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskActivityResetRaid:
	{
		CZTaskActivityResetRaid_Struct* CZRA = (CZTaskActivityResetRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZRA->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->ResetTaskActivity(CZRA->task_id, CZRA->activity_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskActivityResetGuild:
	{
		CZTaskActivityResetGuild_Struct* CZRA = (CZTaskActivityResetGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZRA->guild_id) {
				client.second->ResetTaskActivity(CZRA->task_id, CZRA->activity_id);
			}
		}
		break;
	}
	case ServerOP_CZTaskActivityUpdatePlayer:
	{
		CZTaskActivityUpdatePlayer_Struct* CZUA = (CZTaskActivityUpdatePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZUA->character_id);
		if (client) {
			client->UpdateTaskActivity(CZUA->task_id, CZUA->activity_id, CZUA->activity_count);
		}
		break;
	}
	case ServerOP_CZTaskActivityUpdateGroup:
	{
		CZTaskActivityUpdateGroup_Struct* CZUA = (CZTaskActivityUpdateGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZUA->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->UpdateTaskActivity(CZUA->task_id, CZUA->activity_id, CZUA->activity_count);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskActivityUpdateRaid:
	{
		CZTaskActivityUpdateRaid_Struct* CZUA = (CZTaskActivityUpdateRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZUA->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->UpdateTaskActivity(CZUA->task_id, CZUA->activity_id, CZUA->activity_count);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskActivityUpdateGuild:
	{
		CZTaskActivityUpdateGuild_Struct* CZUA = (CZTaskActivityUpdateGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZUA->guild_id) {
				client.second->UpdateTaskActivity(CZUA->task_id, CZUA->activity_id, CZUA->activity_count);
			}
		}
		break;
	}
	case ServerOP_CZTaskDisablePlayer:
	{
		CZTaskDisablePlayer_Struct* CZUA = (CZTaskDisablePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZUA->character_id);
		if (client) {
			client->DisableTask(1, (int*) CZUA->task_id);
		}
		break;
	}
	case ServerOP_CZTaskDisableGroup:
	{
		CZTaskDisableGroup_Struct* CZUA = (CZTaskDisableGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZUA->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->DisableTask(1, (int*) CZUA->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskDisableRaid:
	{
		CZTaskDisableRaid_Struct* CZUA = (CZTaskDisableRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZUA->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->DisableTask(1, (int*) CZUA->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskDisableGuild:
	{
		CZTaskDisableGuild_Struct* CZUA = (CZTaskDisableGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZUA->guild_id) {
				client.second->DisableTask(1, (int*) CZUA->task_id);
			}
		}
		break;
	}
	case ServerOP_CZTaskEnablePlayer:
	{
		CZTaskEnablePlayer_Struct* CZUA = (CZTaskEnablePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZUA->character_id);
		if (client) {
			client->EnableTask(1, (int*) CZUA->task_id);
		}
		break;
	}
	case ServerOP_CZTaskEnableGroup:
	{
		CZTaskEnableGroup_Struct* CZUA = (CZTaskEnableGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZUA->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->EnableTask(1, (int*) CZUA->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskEnableRaid:
	{
		CZTaskEnableRaid_Struct* CZUA = (CZTaskEnableRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZUA->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->EnableTask(1, (int*) CZUA->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskEnableGuild:
	{
		CZTaskEnableGuild_Struct* CZUA = (CZTaskEnableGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZUA->guild_id) {
				client.second->EnableTask(1, (int*) CZUA->task_id);
			}
		}
		break;
	}
	case ServerOP_CZTaskFailPlayer:
	{
		CZTaskFailPlayer_Struct* CZUA = (CZTaskFailPlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZUA->character_id);
		if (client) {
			client->FailTask(CZUA->task_id);
		}
		break;
	}
	case ServerOP_CZTaskFailGroup:
	{
		CZTaskFailGroup_Struct* CZUA = (CZTaskFailGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZUA->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->FailTask(CZUA->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskFailRaid:
	{
		CZTaskFailRaid_Struct* CZUA = (CZTaskFailRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZUA->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->FailTask(CZUA->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskFailGuild:
	{
		CZTaskFailGuild_Struct* CZUA = (CZTaskFailGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZUA->guild_id) {
				client.second->FailTask(CZUA->task_id);
			}
		}
		break;
	}
	case ServerOP_CZTaskRemovePlayer:
	{
		CZTaskRemovePlayer_Struct* CZTR = (CZTaskRemovePlayer_Struct*) pack->pBuffer;
		auto client = entity_list.GetClientByCharID(CZTR->character_id);
		if (client) {
			client->RemoveTaskByTaskID(CZTR->task_id);
		}
		break;
	}
	case ServerOP_CZTaskRemoveGroup:
	{
		CZTaskRemoveGroup_Struct* CZTR = (CZTaskRemoveGroup_Struct*) pack->pBuffer;
		auto client_group = entity_list.GetGroupByID(CZTR->group_id);
		if (client_group) {
			for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
				if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
					auto group_member = client_group->members[member_index]->CastToClient();
					group_member->RemoveTaskByTaskID(CZTR->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskRemoveRaid:
	{
		CZTaskRemoveRaid_Struct* CZTR = (CZTaskRemoveRaid_Struct*) pack->pBuffer;
		auto client_raid = entity_list.GetRaidByID(CZTR->raid_id);
		if (client_raid) {
			for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
				if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
					auto raid_member = client_raid->members[member_index].member->CastToClient();
					raid_member->RemoveTaskByTaskID(CZTR->task_id);
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskRemoveGuild:
	{
		CZTaskRemoveGuild_Struct* CZTR = (CZTaskRemoveGuild_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->GuildID() > 0 && client.second->GuildID() == CZTR->guild_id) {
				client.second->RemoveTaskByTaskID(CZTR->task_id);
			}
		}
		break;
	}
	case ServerOP_WWAssignTask:
	{
		WWAssignTask_Struct* WWAT = (WWAssignTask_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWAT->min_status && (client_status <= WWAT->max_status || WWAT->max_status == 0)) {
				client.second->AssignTask(WWAT->task_id, WWAT->npc_entity_id, WWAT->enforce_level_requirement);
			}
		}
		break;
	}
	case ServerOP_WWCastSpell:
	{
		WWCastSpell_Struct* WWCS = (WWCastSpell_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWCS->min_status && (client_status <= WWCS->max_status || WWCS->max_status == 0)) {
				client.second->SpellFinished(WWCS->spell_id, client.second);
			}
		}
		break;
	}
	case ServerOP_WWDisableTask:
	{
		WWDisableTask_Struct* WWDT = (WWDisableTask_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWDT->min_status && (client_status <= WWDT->max_status || WWDT->max_status == 0)) {
				client.second->DisableTask(1, (int *) WWDT->task_id);
			}
		}
		break;
	}
	case ServerOP_WWEnableTask:
	{
		WWEnableTask_Struct* WWET = (WWEnableTask_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWET->min_status && (client_status <= WWET->max_status || WWET->max_status == 0)) {
				client.second->EnableTask(1, (int *) WWET->task_id);
			}
		}
		break;
	}
	case ServerOP_WWFailTask:
	{
		WWFailTask_Struct* WWFT = (WWFailTask_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWFT->min_status && (client_status <= WWFT->max_status || WWFT->max_status == 0)) {
				client.second->FailTask(WWFT->task_id);
			}
		}
		break;
	}
	case ServerOP_WWMarquee:
	{
		WWMarquee_Struct* WWMS = (WWMarquee_Struct*) pack->pBuffer;
		std::string message = WWMS->message;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWMS->min_status && (client_status <= WWMS->max_status || WWMS->max_status == 0)) {
				client.second->SendMarqueeMessage(WWMS->type, WWMS->priority, WWMS->fade_in, WWMS->fade_out, WWMS->duration, message);
			}
		}
		break;
	}
	case ServerOP_WWMessage:
	{
		WWMessage_Struct* WWMS = (WWMessage_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWMS->min_status && (client_status <= WWMS->max_status || WWMS->max_status == 0)) {
				client.second->Message(WWMS->type, WWMS->message);
			}
		}
		break;
	}
	case ServerOP_WWMove:
	{
		WWMove_Struct* WWMS = (WWMove_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWMS->min_status && (client_status <= WWMS->max_status || WWMS->max_status == 0)) {
				client.second->MoveZone(WWMS->zone_short_name);
			}
		}
		break;
	}
	case ServerOP_WWMoveInstance:
	{
		WWMoveInstance_Struct* WWMS = (WWMoveInstance_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWMS->min_status && (client_status <= WWMS->max_status || WWMS->max_status == 0)) {
				client.second->MoveZoneInstance(WWMS->instance_id);
			}
		}
		break;
	}
	case ServerOP_WWRemoveSpell:
	{
		WWRemoveSpell_Struct* WWRS = (WWRemoveSpell_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWRS->min_status && (client_status <= WWRS->max_status || WWRS->max_status == 0)) {
				client.second->BuffFadeBySpellID(WWRS->spell_id);
			}
		}
		break;
	}
	case ServerOP_WWRemoveTask:
	{
		WWRemoveTask_Struct* WWRT = (WWRemoveTask_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWRT->min_status && (client_status <= WWRT->max_status || WWRT->max_status == 0)) {
				client.second->RemoveTaskByTaskID(WWRT->task_id);
			}
		}
		break;
	}
	case ServerOP_WWResetActivity:
	{
		WWResetActivity_Struct* WWRA = (WWResetActivity_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWRA->min_status && (client_status <= WWRA->max_status || WWRA->max_status == 0)) {
				client.second->ResetTaskActivity(WWRA->task_id, WWRA->activity_id);
			}
		}
		break;
	}
	case ServerOP_WWSetEntityVariableClient:
	{
		WWSetEntVarClient_Struct* WWSC = (WWSetEntVarClient_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWSC->min_status && (client_status <= WWSC->max_status || WWSC->max_status == 0)) {
				client.second->SetEntityVariable(WWSC->variable_name, WWSC->variable_value);
			}
		}
		break;
	}
	case ServerOP_WWSetEntityVariableNPC:
	{
		WWSetEntVarNPC_Struct* WWSN = (WWSetEntVarNPC_Struct*) pack->pBuffer;
		for (auto &npc : entity_list.GetNPCList()) {
			npc.second->SetEntityVariable(WWSN->variable_name, WWSN->variable_value);
		}
		break;
	}
	case ServerOP_WWSignalClient:
	{
		WWSignalClient_Struct* WWSC = (WWSignalClient_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWSC->min_status && (client_status <= WWSC->max_status || WWSC->max_status == 0)) {
				client.second->Signal(WWSC->signal);
			}
		}
		break;
	}
	case ServerOP_WWSignalNPC:
	{
		WWSignalNPC_Struct* WWSN = (WWSignalNPC_Struct*) pack->pBuffer;
		for (auto &npc : entity_list.GetNPCList()) {
			npc.second->SignalNPC(WWSN->signal);
		}
		break;
	}
	case ServerOP_WWUpdateActivity:
	{
		WWUpdateActivity_Struct* WWUA = (WWUpdateActivity_Struct*) pack->pBuffer;
		for (auto &client : entity_list.GetClientList()) {
			auto client_status = client.second->Admin();
			if (client_status >= WWUA->min_status && (client_status <= WWUA->max_status || WWUA->max_status == 0)) {
				client.second->UpdateTaskActivity(WWUA->task_id, WWUA->activity_id, WWUA->activity_count);
			}
		}
		break;
	}

	case ServerOP_ReloadWorld:
	{
		auto* reload_world = (ReloadWorld_Struct*)pack->pBuffer;
		if (zone) {
			zone->ReloadWorld(reload_world->Option);
		}
		break;
	}

	case ServerOP_HotReloadQuests:
	{
		if (!zone) {
			break;
		}

		auto *hot_reload_quests = (HotReloadQuestsStruct *) pack->pBuffer;

		LogHotReloadDetail(
			"Receiving request [HotReloadQuests] | request_zone [{}] current_zone [{}]",
			hot_reload_quests->zone_short_name,
			zone->GetShortName()
		);

		std::string request_zone_short_name = hot_reload_quests->zone_short_name;
		std::string local_zone_short_name   = zone->GetShortName();
		bool can_reload_global_script = (request_zone_short_name == "all" && RuleB(HotReload, QuestsAutoReloadGlobalScripts));

		if (request_zone_short_name == local_zone_short_name || can_reload_global_script) {
			zone->SetQuestHotReloadQueued(true);
		} else if (request_zone_short_name == "all") {
			std::string reload_quest_saylink = EQ::SayLinkEngine::GenerateQuestSaylink("#reloadquest", false, "Locally");
			std::string reload_world_saylink = EQ::SayLinkEngine::GenerateQuestSaylink("#reloadworld", false, "Globally");
			worldserver.SendEmoteMessage(0, 0, 20, 15, "A quest, plugin, or global script has changed reload quests [%s] [%s].", reload_quest_saylink.c_str(), reload_world_saylink.c_str());
		}

		break;
	}

	case ServerOP_ChangeSharedMem:
	{
		std::string hotfix_name = std::string((char*)pack->pBuffer);
		LogInfo("Loading items");
		if (!content_db.LoadItems(hotfix_name)) {
			LogError("Loading items failed!");
		}

		LogInfo("Loading npc faction lists");
		if (!content_db.LoadNPCFactionLists(hotfix_name)) {
			LogError("Loading npcs faction lists failed!");
		}

		LogInfo("Loading loot tables");
		if (!content_db.LoadLoot(hotfix_name)) {
			LogError("Loading loot failed!");
		}

		LogInfo("Loading skill caps");
		if (!content_db.LoadSkillCaps(std::string(hotfix_name))) {
			LogError("Loading skill caps failed!");
		}

		LogInfo("Loading spells");
		if (!content_db.LoadSpells(hotfix_name, &SPDAT_RECORDS, &spells)) {
			LogError("Loading spells failed!");
		}

		LogInfo("Loading base data");
		if (!content_db.LoadBaseData(hotfix_name)) {
			LogError("Loading base data failed!");
		}
		break;
	}
	default: {
		std::cout << " Unknown ZSopcode:" << (int)pack->opcode;
		std::cout << " size:" << pack->size << std::endl;
		break;
	}
	}
}

bool WorldServer::SendChannelMessage(Client* from, const char* to, uint8 chan_num, uint32 guilddbid, uint8 language, uint8 lang_skill, const char* message, ...) {
	if (!worldserver.Connected())
		return false;
	va_list argptr;
	char buffer[512];

	va_start(argptr, message);
	vsnprintf(buffer, 512, message, argptr);
	va_end(argptr);
	buffer[511] = '\0';

	auto pack = new ServerPacket(ServerOP_ChannelMessage, sizeof(ServerChannelMessage_Struct) + strlen(buffer) + 1);
	ServerChannelMessage_Struct* scm = (ServerChannelMessage_Struct*)pack->pBuffer;

	if (from == 0) {
		strcpy(scm->from, "ZServer");
		scm->fromadmin = 0;
	}
	else {
		strcpy(scm->from, from->GetName());
		scm->fromadmin = from->Admin();
	}
	if (to == 0) {
		scm->to[0] = 0;
		scm->deliverto[0] = '\0';
	}
	else {
		strn0cpy(scm->to, to, sizeof(scm->to));
		strn0cpy(scm->deliverto, to, sizeof(scm->deliverto));
	}
	scm->noreply = false;
	scm->chan_num = chan_num;
	scm->guilddbid = guilddbid;
	scm->language = language;
	scm->lang_skill = lang_skill;
	scm->queued = 0;
	strcpy(scm->message, buffer);

	bool ret = SendPacket(pack);
	safe_delete(pack);
	return ret;
}

bool WorldServer::SendEmoteMessage(const char* to, uint32 to_guilddbid, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096] = { 0 };

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer) - 1, message, argptr);
	va_end(argptr);

	return SendEmoteMessage(to, to_guilddbid, 0, type, buffer);
}

bool WorldServer::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096] = { 0 };

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer) - 1, message, argptr);
	va_end(argptr);

	if (!Connected() && to == 0) {
		entity_list.MessageStatus(to_guilddbid, to_minstatus, type, buffer);
		return false;
	}

	auto pack = new ServerPacket(ServerOP_EmoteMessage, sizeof(ServerEmoteMessage_Struct) + strlen(buffer) + 1);
	ServerEmoteMessage_Struct* sem = (ServerEmoteMessage_Struct*)pack->pBuffer;
	sem->type = type;
	if (to != 0)
		strcpy(sem->to, to);
	sem->guilddbid = to_guilddbid;
	sem->minstatus = to_minstatus;
	strcpy(sem->message, buffer);

	bool ret = SendPacket(pack);
	safe_delete(pack);
	return ret;
}

bool WorldServer::SendVoiceMacro(Client* From, uint32 Type, char* Target, uint32 MacroNumber, uint32 GroupOrRaidID) {

	if (!worldserver.Connected() || !From)
		return false;

	auto pack = new ServerPacket(ServerOP_VoiceMacro, sizeof(ServerVoiceMacro_Struct));

	ServerVoiceMacro_Struct* svm = (ServerVoiceMacro_Struct*)pack->pBuffer;

	strcpy(svm->From, From->GetName());

	switch (Type) {

	case VoiceMacroTell:
		strcpy(svm->To, Target);
		break;

	case VoiceMacroGroup:
		svm->GroupID = GroupOrRaidID;
		break;

	case VoiceMacroRaid:
		svm->RaidID = GroupOrRaidID;
		break;
	}

	svm->Type = Type;

	svm->Voice = (GetPlayerRaceValue(From->GetRace()) * 2) + From->GetGender();

	svm->MacroNumber = MacroNumber;

	bool Ret = SendPacket(pack);

	safe_delete(pack);

	return Ret;
}

bool WorldServer::RezzPlayer(EQApplicationPacket* rpack, uint32 rezzexp, uint32 dbid, uint16 opcode)
{
	LogSpells("WorldServer::RezzPlayer rezzexp is [{}] (0 is normal for RezzComplete", rezzexp);
	auto pack = new ServerPacket(ServerOP_RezzPlayer, sizeof(RezzPlayer_Struct));
	RezzPlayer_Struct* sem = (RezzPlayer_Struct*)pack->pBuffer;
	sem->rezzopcode = opcode;
	sem->rez = *(Resurrect_Struct*)rpack->pBuffer;
	sem->exp = rezzexp;
	sem->dbid = dbid;
	bool ret = SendPacket(pack);
	if (ret)
		LogSpells("Sending player rezz packet to world spellid:[{}]", sem->rez.spellid);
	else
		LogSpells("NOT Sending player rezz packet to world");

	safe_delete(pack);
	return ret;
}

void WorldServer::SendReloadTasks(int Command, int TaskID) {
	auto pack = new ServerPacket(ServerOP_ReloadTasks, sizeof(ReloadTasks_Struct));
	ReloadTasks_Struct* rts = (ReloadTasks_Struct*)pack->pBuffer;

	rts->Command = Command;
	rts->Parameter = TaskID;

	SendPacket(pack);
}

void WorldServer::HandleReloadTasks(ServerPacket *pack)
{
	ReloadTasks_Struct* rts = (ReloadTasks_Struct*)pack->pBuffer;

	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Zone received ServerOP_ReloadTasks from World, Command %i", rts->Command);

	switch (rts->Command) {
	case RELOADTASKS:
		entity_list.SaveAllClientsTaskState();

		if (rts->Parameter == 0) {
			Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Reload ALL tasks");
			safe_delete(taskmanager);
			taskmanager = new TaskManager;
			taskmanager->LoadTasks();
			if (zone)
				taskmanager->LoadProximities(zone->GetZoneID());
			entity_list.ReloadAllClientsTaskState();
		}
		else {
			Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Reload only task %i", rts->Parameter);
			taskmanager->LoadTasks(rts->Parameter);
			entity_list.ReloadAllClientsTaskState(rts->Parameter);
		}

		break;

	case RELOADTASKPROXIMITIES:
		if (zone) {
			Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Reload task proximities");
			taskmanager->LoadProximities(zone->GetZoneID());
		}
		break;

	case RELOADTASKGOALLISTS:
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Reload task goal lists");
		taskmanager->ReloadGoalLists();
		break;

	case RELOADTASKSETS:
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Reload task sets");
		taskmanager->LoadTaskSets();
		break;

	default:
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Unhandled ServerOP_ReloadTasks command %i", rts->Command);

	}


}


uint32 WorldServer::NextGroupID() {
	//this system wastes a lot of potential group IDs (~5%), but
	//if you are creating 2 billion groups in 1 run of the emu,
	//something else is wrong...
	if (cur_groupid >= last_groupid) {
		//this is an error... This means that 50 groups were created before
		//1 packet could make the zone->world->zone trip... so let it error.
		LogError("Ran out of group IDs before the server sent us more");
		return(0);
	}
	if (cur_groupid > (last_groupid - /*50*/995)) {
		//running low, request more
		auto pack = new ServerPacket(ServerOP_GroupIDReq);
		SendPacket(pack);
		safe_delete(pack);
	}
	printf("Handing out new group id %d\n", cur_groupid);
	return(cur_groupid++);
}

void WorldServer::UpdateLFP(uint32 LeaderID, uint8 Action, uint8 MatchFilter, uint32 FromLevel, uint32 ToLevel, uint32 Classes,
	const char *Comments, GroupLFPMemberEntry *LFPMembers) {

	auto pack = new ServerPacket(ServerOP_LFPUpdate, sizeof(ServerLFPUpdate_Struct));
	ServerLFPUpdate_Struct* sus = (ServerLFPUpdate_Struct*)pack->pBuffer;

	sus->LeaderID = LeaderID;
	sus->Action = Action;
	sus->MatchFilter = MatchFilter;
	sus->FromLevel = FromLevel;
	sus->ToLevel = ToLevel;
	sus->Classes = Classes;
	strcpy(sus->Comments, Comments);
	memcpy(sus->Members, LFPMembers, sizeof(sus->Members));
	SendPacket(pack);
	safe_delete(pack);

}

void WorldServer::UpdateLFP(uint32 LeaderID, GroupLFPMemberEntry *LFPMembers) {

	UpdateLFP(LeaderID, LFPMemberUpdate, 0, 0, 0, 0, "", LFPMembers);
}

void WorldServer::StopLFP(uint32 LeaderID) {

	GroupLFPMemberEntry LFPMembers;
	UpdateLFP(LeaderID, LFPOff, 0, 0, 0, 0, "", &LFPMembers);
}

void WorldServer::HandleLFGMatches(ServerPacket *pack) {

	char *Buffer = (char *)pack->pBuffer;

	int PacketLength = 4;

	int Entries = (pack->size - 4) / sizeof(ServerLFGMatchesResponse_Struct);

	uint32 EntityID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);

	Client* client = entity_list.GetClientByID(EntityID);

	if (client) {
		ServerLFGMatchesResponse_Struct* smrs = (ServerLFGMatchesResponse_Struct*)Buffer;

		for (int i = 0; i<Entries; i++) {
			PacketLength = PacketLength + 12 + strlen(smrs->Name) + strlen(smrs->Comments);
			smrs++;
		}

		auto outapp = new EQApplicationPacket(OP_LFGGetMatchesResponse, PacketLength);

		smrs = (ServerLFGMatchesResponse_Struct*)Buffer;

		char *OutBuffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0x00074af); // Unknown

		for (int i = 0; i<Entries; i++) {
			VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Comments);
			VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Name);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Class_);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Level);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Zone);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->GuildID);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Anon);
			smrs++;
		}
		client->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void WorldServer::HandleLFPMatches(ServerPacket *pack) {

	char *Buffer = (char *)pack->pBuffer;

	int PacketLength = 4;

	int Entries = (pack->size - 4) / sizeof(ServerLFPMatchesResponse_Struct);

	uint32 EntityID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);

	ServerLFPMatchesResponse_Struct* smrs = (ServerLFPMatchesResponse_Struct*)Buffer;

	Client* client = entity_list.GetClientByID(EntityID);

	if (client) {
		for (int i = 0; i<Entries; i++) {
			PacketLength += strlen(smrs->Comments) + 11;
			for (unsigned int j = 0; j<MAX_GROUP_MEMBERS; j++) {

				if (smrs->Members[j].Name[0] != '\0')
					PacketLength += strlen(smrs->Members[j].Name) + 9;
			}
			smrs++;
		}
		auto outapp = new EQApplicationPacket(OP_LFPGetMatchesResponse, PacketLength);

		smrs = (ServerLFPMatchesResponse_Struct*)Buffer;

		char *OutBuffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, 0x00074af); // Unknown

		for (int i = 0; i<Entries; i++) {

			int MemberCount = 0;

			for (unsigned int j = 0; j<MAX_GROUP_MEMBERS; j++)
				if (smrs->Members[j].Name[0] != '\0')
					MemberCount++;

			VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Comments);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->FromLevel);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->ToLevel);
			VARSTRUCT_ENCODE_TYPE(uint32, OutBuffer, smrs->Classes);
			VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, MemberCount);

			for (unsigned int j = 0; j<MAX_GROUP_MEMBERS; j++) {
				if (smrs->Members[j].Name[0] != '\0') {
					VARSTRUCT_ENCODE_STRING(OutBuffer, smrs->Members[j].Name);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].Class);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].Level);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].Zone);
					VARSTRUCT_ENCODE_TYPE(uint16, OutBuffer, smrs->Members[j].GuildID);
				}
			}
			smrs++;
		}
		client->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void WorldServer::RequestTellQueue(const char *who)
{
	if (!who)
		return;

	auto pack = new ServerPacket(ServerOP_RequestTellQueue, sizeof(ServerRequestTellQueue_Struct));
	ServerRequestTellQueue_Struct* rtq = (ServerRequestTellQueue_Struct*)pack->pBuffer;

	strn0cpy(rtq->name, who, sizeof(rtq->name));

	SendPacket(pack);
	safe_delete(pack);
	return;
}

void WorldServer::OnKeepAlive(EQ::Timer *t)
{
	ServerPacket pack(ServerOP_KeepAlive, 0);
	SendPacket(&pack);
}