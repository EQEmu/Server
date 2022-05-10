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
#include "expedition.h"
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
#include "../common/shared_tasks.h"
#include "shared_task_zone_messaging.h"
#include "dialogue_window.h"

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
	m_connection = std::make_unique<EQ::Net::ServertalkClient>(Config->WorldIP, Config->WorldTCPPort, false, "Zone", Config->SharedKey);
	m_connection->OnConnect([this](EQ::Net::ServertalkClient *client) {
		OnConnected();
	});

	m_connection->OnMessage(std::bind(&WorldServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));

	m_keepalive = std::make_unique<EQ::Timer>(1000, true, std::bind(&WorldServer::OnKeepAlive, this, std::placeholders::_1));
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
		SetZoneData(zone->GetZoneID(), zone->GetInstanceID());
		entity_list.UpdateWho(true);
		SendEmoteMessage(
			0,
			0,
			Chat::Yellow,
			fmt::format(
				"Zone Connected | {}",
				zone->GetZoneDescription()
			).c_str()
		);
		zone->GetTimeSync();
	}
	else {
		SetZoneData(0);
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
	case 0:
	case ServerOP_KeepAlive: {
		break;
	}
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
				entity_list.MessageStatus(
					sem->guilddbid,
					sem->minstatus,
					sem->type,
					(char*)sem->message
				);
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
			if (strstr(sem->message, "^") == 0) {
				entity_list.MessageStatus(
					sem->guilddbid,
					sem->minstatus,
					sem->type,
					sem->message
				);
			} else {
				for (newmessage = strtok((char*)sem->message, "^"); newmessage != nullptr; newmessage = strtok(nullptr, "^")) {
					entity_list.MessageStatus(
						sem->guilddbid,
						sem->minstatus,
						sem->type,
						newmessage
					);
				}
			}
		}
		break;
	}
	case ServerOP_Motd: {
		if (pack->size != sizeof(ServerMotd_Struct))
			break;

		ServerMotd_Struct *smotd = (ServerMotd_Struct *)pack->pBuffer;
		SerializeBuffer buf(100);
		buf.WriteString(smotd->motd);

		auto outapp = std::make_unique<EQApplicationPacket>(OP_MOTD, buf);

		entity_list.QueueClients(0, outapp.get());
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
			SendEmoteMessage(
				0,
				0,
				Chat::Yellow,
				fmt::format(
					"Zone Shutdown | {}",
					zone->GetZoneDescription()
				).c_str()
			);

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
				SendEmoteMessage(
					zst->adminname,
					0,
					Chat::White,
					fmt::format(
						"Zone Bootup Failed | {} Already running",
						zone->GetZoneDescription()
					).c_str()
				);
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
		auto client = entity_list.GetClientByName(szp->name);
		if (client) {
			if (!strcasecmp(szp->adminname, szp->name)) {
				client->Message(
					Chat::White,
					fmt::format(
						"Zoning to {} ({}).",
						ZoneLongName(
							ZoneID(szp->zone)
						),
						ZoneID(szp->zone)
					).c_str()
				);
			} else if (client->GetAnon() == 1 && client->Admin() > szp->adminrank) {
				break;
			} else {
				std::string name = str_tolower(szp->name);
				name[0] = toupper(name[0]);

				SendEmoteMessage(
					szp->adminname,
					0,
					Chat::White,
					fmt::format(
						"Summoning {} to {:.2f}, {:.2f}, {:.2f} in {} ({}).",
						name,
						szp->x_pos,
						szp->y_pos,
						szp->z_pos,
						ZoneLongName(
							ZoneID(szp->zone)
						),
						ZoneID(szp->zone)
					).c_str()
				);
			}

			if (!szp->instance_id) {
				client->MovePC(ZoneID(szp->zone), szp->instance_id, szp->x_pos, szp->y_pos, szp->z_pos, client->GetHeading(), szp->ignorerestrictions, GMSummon);
			} else {
				if (database.GetInstanceID(client->CharacterID(), ZoneID(szp->zone))) {
					client->RemoveFromInstance(database.GetInstanceID(client->CharacterID(), ZoneID(szp->zone)));
				}

				client->AssignToInstance(szp->instance_id);
				client->MovePC(ZoneID(szp->zone), szp->instance_id, szp->x_pos, szp->y_pos, szp->z_pos, client->GetHeading(), szp->ignorerestrictions, GMSummon);
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
				SendEmoteMessage(
					skp->adminname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kick | {} booted{}.",
						skp->name,
						is_zone_loaded ?
						fmt::format(
							" in {}",
							zone->GetZoneDescription()
						) :
						""
					).c_str()
				);
			} else if (client->GetAnon() != 1) {
				SendEmoteMessage(
					skp->adminname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kick | Your Status Level is not high enough to kick {}.",
						skp->name
					).c_str()
				);
			}
		}
		break;
	}
	case ServerOP_KillPlayer: {
		ServerKillPlayer_Struct* skp = (ServerKillPlayer_Struct*)pack->pBuffer;
		Client* client = entity_list.GetClientByName(skp->target);
		if (client) {
			if (skp->admin >= client->Admin()) {
				client->GMKill();
				SendEmoteMessage(
					skp->gmname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kill | {} killed{}",
						skp->target,
						is_zone_loaded ?
						fmt::format(
							"in {}",
							zone->GetZoneDescription()
						) :
						""
					).c_str()
				);
			} else if (client->GetAnon() != 1) {
				SendEmoteMessage(
					skp->gmname,
					0,
					Chat::White,
					fmt::format(
						"Remote Kill | Your Status Level is not high enough to kill {}",
						skp->target
					).c_str()
				);
			}
		}
		break;
	}
	case ServerOP_OnlineGuildMembersResponse:
	case ServerOP_RefreshGuild:
	case ServerOP_DeleteGuild:
	case ServerOP_GuildCharRefresh:
	case ServerOP_GuildMemberUpdate:
	case ServerOP_GuildRankUpdate:
	case ServerOP_LFGuildUpdate:
	{
		guild_mgr.ProcessWorldPacket(pack);
		break;
	}
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
			SendEmoteMessage(
				gmg->myname,
				0,
				Chat::Red,
				fmt::format(
					"Summoning you to {} in {} at {:.2f}, {:.2f}, {:.2f}",
					client->GetCleanName(),
					zone->GetZoneDescription(),
					client->GetX(),
					client->GetY(),
					client->GetZ()
				).c_str()
			);
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
			SendEmoteMessage(
				gmg->myname,
				0,
				Chat::Red,
				fmt::format(
					"Error: {} not found.",
					gmg->gotoname
				).c_str()
			);
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
		std::string time_string = ConvertMillisecondsToTime(ms);
		SendEmoteMessage(
			sus->adminname,
			0,
			Chat::White,
			fmt::format(
				"Zoneserver {} | Uptime: {}",
				sus->zoneserverid,
				time_string
			).c_str()
		);
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
		if (client) {
			SendEmoteMessage(
				rev->adminname,
				0,
				Chat::White,
				fmt::format(
					"Zone {} | {} {}.",
					zone->GetZoneDescription(),
					rev->toggle ? "Revoking" : "Unrevoking",
					client->GetCleanName()
				).c_str()
			);
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
					safe_delete(group);
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
				database.GetGroupLeadershipInfo(group->GetID(), ln, MainTankName, AssistName, PullerName, NPCMarkerName, mentoree_name, &mentor_percent, &GLAA);
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
	case ServerOP_ReloadAAData:
	{
		zone->SendReloadMessage("Alternate Advancement Data");
		zone->LoadAlternateAdvancement();
		break;
	}
	case ServerOP_ReloadAlternateCurrencies:
	{
		zone->SendReloadMessage("Alternate Currencies");
		zone->LoadAlternateCurrencies();
		break;
	}
	case ServerOP_ReloadBlockedSpells:
	{
		zone->SendReloadMessage("Blocked Spells");
		zone->LoadZoneBlockedSpells();
		break;
	}
	case ServerOP_ReloadContentFlags:
	{
		zone->SendReloadMessage("Content Flags");
		content_service.SetExpansionContext()->ReloadContentFlags();
		break;
	}
	case ServerOP_ReloadDoors:
	{
		zone->SendReloadMessage("Doors");
		entity_list.RemoveAllDoors();
		zone->LoadZoneDoors();
		entity_list.RespawnAllDoors();
		break;
	}
	case ServerOP_ReloadGroundSpawns:
	{
		zone->SendReloadMessage("Ground Spawns");
		zone->LoadGroundSpawns();
		break;
	}
	case ServerOP_ReloadLevelEXPMods:
	{
		zone->SendReloadMessage("Level Based Experience Modifiers");
		zone->LoadLevelEXPMods();
		break;
	}
	case ServerOP_ReloadLogs:
	{
		zone->SendReloadMessage("Log Settings");
		LogSys.LoadLogDatabaseSettings();
		break;
	}
	case ServerOP_ReloadMerchants: {
		zone->SendReloadMessage("Merchants");
		entity_list.ReloadMerchants();
		break;
	}
	case ServerOP_ReloadNPCEmotes:
	{
		zone->SendReloadMessage("NPC Emotes");
		zone->LoadNPCEmotes(&zone->NPCEmoteList);
		break;
	}
	case ServerOP_ReloadObjects:
	{
		zone->SendReloadMessage("Objects");
		entity_list.RemoveAllObjects();
		zone->LoadZoneObjects();
		break;
	}
	case ServerOP_ReloadPerlExportSettings:
	{
		zone->SendReloadMessage("Perl Event Export Settings");
		parse->LoadPerlEventExportSettings(parse->perl_event_export_settings);
		break;
	}
	case ServerOP_ReloadRules:
	{
		zone->SendReloadMessage("Rules");
		RuleManager::Instance()->LoadRules(&database, RuleManager::Instance()->GetActiveRuleset(), true);
		break;
	}
	case ServerOP_ReloadStaticZoneData: {
		zone->SendReloadMessage("Static Zone Data");
		zone->ReloadStaticData();
		break;
	}
	case ServerOP_ReloadTasks:
	{
		if (RuleB(Tasks, EnableTaskSystem)) {
			zone->SendReloadMessage("Tasks");
			HandleReloadTasks(pack);
		}

		break;
	}
	case ServerOP_ReloadTitles:
	{
		zone->SendReloadMessage("Titles");
		title_manager.LoadTitles();
		break;
	}
	case ServerOP_ReloadTraps:
	{
		zone->SendReloadMessage("Traps");
		entity_list.UpdateAllTraps(true, true);
		break;
	}
	case ServerOP_ReloadVariables:
	{
		zone->SendReloadMessage("Variables");
		database.LoadVariables();
		break;
	}
	case ServerOP_ReloadVeteranRewards:
	{
		zone->SendReloadMessage("Veteran Rewards");
		zone->LoadVeteranRewards();
		break;
	}
	case ServerOP_ReloadWorld:
	{
		auto* reload_world = (ReloadWorld_Struct*)pack->pBuffer;
		if (zone) {
			zone->ReloadWorld(reload_world->global_repop);
		}
		break;
	}
	case ServerOP_ReloadZonePoints:
	{
		zone->SendReloadMessage("Zone Points");
		content_db.LoadStaticZonePoints(&zone->zone_point_list, zone->GetShortName(), zone->GetInstanceVersion());
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

			for (auto &e : entity_list.GetClientList()) {
				e.second->ReconnectUCS();
			}
		}
		break;
	}
	case ServerOP_CZDialogueWindow:
	{
		CZDialogueWindow_Struct* CZDW = (CZDialogueWindow_Struct*) pack->pBuffer;
		uint8 update_type = CZDW->update_type;
		int update_identifier = CZDW->update_identifier;
		std::string message = CZDW->message;
		const char* client_name = CZDW->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				DialogueWindow::Render(client, message);
			}
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						DialogueWindow::Render(group_member, message);
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						DialogueWindow::Render(raid_member, message);
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					DialogueWindow::Render(client.second, message);
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					DialogueWindow::Render(client.second, message);
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				DialogueWindow::Render(client, message);
			}
		}
		break;
	}
	case ServerOP_CZLDoNUpdate:
	{
		CZLDoNUpdate_Struct* CZLU = (CZLDoNUpdate_Struct*) pack->pBuffer;
		uint8 update_type = CZLU->update_type;
		uint8 update_subtype = CZLU->update_subtype;
		int update_identifier = CZLU->update_identifier;
		uint32 theme_id = CZLU->theme_id;
		int points = CZLU->points;
		const char* client_name = CZLU->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				switch (update_subtype) {
					case CZLDoNUpdateSubtype_AddLoss:
						client->UpdateLDoNWinLoss(theme_id, false);
						break;
					case CZLDoNUpdateSubtype_AddPoints:
						client->UpdateLDoNPoints(theme_id, points);
						break;
					case CZLDoNUpdateSubtype_AddWin:
						client->UpdateLDoNWinLoss(theme_id, true);
						break;
					case CZLDoNUpdateSubtype_RemoveLoss:
						client->UpdateLDoNWinLoss(theme_id, false, true);
						break;
					case CZLDoNUpdateSubtype_RemoveWin:
						client->UpdateLDoNWinLoss(theme_id, true, true);
						break;
					default:
						break;
				}
			}
			break;
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto client_group_member = client_group->members[member_index]->CastToClient();
						switch (update_subtype) {
							case CZLDoNUpdateSubtype_AddLoss:
								client_group_member->UpdateLDoNWinLoss(theme_id, false);
								break;
							case CZLDoNUpdateSubtype_AddPoints:
								client_group_member->UpdateLDoNPoints(theme_id, points);
								break;
							case CZLDoNUpdateSubtype_AddWin:
								client_group_member->UpdateLDoNWinLoss(theme_id, true);
								break;
							case CZLDoNUpdateSubtype_RemoveLoss:
								client_group_member->UpdateLDoNWinLoss(theme_id, false, true);
								break;
							case CZLDoNUpdateSubtype_RemoveWin:
								client_group_member->UpdateLDoNWinLoss(theme_id, true, true);
								break;
							default:
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					auto client_raid_member = client_raid->members[member_index].member;
					if (client_raid_member && client_raid_member->IsClient()) {
						switch (update_subtype) {
							case CZLDoNUpdateSubtype_AddLoss:
								client_raid_member->UpdateLDoNWinLoss(theme_id, false);
								break;
							case CZLDoNUpdateSubtype_AddPoints:
								client_raid_member->UpdateLDoNPoints(theme_id, points);
								break;
							case CZLDoNUpdateSubtype_AddWin:
								client_raid_member->UpdateLDoNWinLoss(theme_id, true);
								break;
							case CZLDoNUpdateSubtype_RemoveLoss:
								client_raid_member->UpdateLDoNWinLoss(theme_id, false, true);
								break;
							case CZLDoNUpdateSubtype_RemoveWin:
								client_raid_member->UpdateLDoNWinLoss(theme_id, true, true);
								break;
							default:
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client : entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					switch (update_subtype) {
						case CZLDoNUpdateSubtype_AddLoss:
							client.second->UpdateLDoNWinLoss(theme_id, false);
							break;
						case CZLDoNUpdateSubtype_AddPoints:
							client.second->UpdateLDoNPoints(theme_id, points);
							break;
						case CZLDoNUpdateSubtype_AddWin:
							client.second->UpdateLDoNWinLoss(theme_id, true);
							break;
						case CZLDoNUpdateSubtype_RemoveLoss:
							client.second->UpdateLDoNWinLoss(theme_id, false, true);
							break;
						case CZLDoNUpdateSubtype_RemoveWin:
							client.second->UpdateLDoNWinLoss(theme_id, true, true);
							break;
						default:
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client : entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					switch (update_subtype) {
						case CZLDoNUpdateSubtype_AddLoss:
							client.second->UpdateLDoNWinLoss(theme_id, false);
							break;
						case CZLDoNUpdateSubtype_AddPoints:
							client.second->UpdateLDoNPoints(theme_id, points);
							break;
						case CZLDoNUpdateSubtype_AddWin:
							client.second->UpdateLDoNWinLoss(theme_id, true);
							break;
						case CZLDoNUpdateSubtype_RemoveLoss:
							client.second->UpdateLDoNWinLoss(theme_id, false, true);
							break;
						case CZLDoNUpdateSubtype_RemoveWin:
							client.second->UpdateLDoNWinLoss(theme_id, true, true);
							break;
						default:
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				switch (update_subtype) {
					case CZLDoNUpdateSubtype_AddLoss:
						client->UpdateLDoNWinLoss(theme_id, false);
						break;
					case CZLDoNUpdateSubtype_AddPoints:
						client->UpdateLDoNPoints(theme_id, points);
						break;
					case CZLDoNUpdateSubtype_AddWin:
						client->UpdateLDoNWinLoss(theme_id, true);
						break;
					case CZLDoNUpdateSubtype_RemoveLoss:
						client->UpdateLDoNWinLoss(theme_id, false, true);
						break;
					case CZLDoNUpdateSubtype_RemoveWin:
						client->UpdateLDoNWinLoss(theme_id, true, true);
						break;
					default:
						break;
				}
			}
			break;
		}
		break;
	}
	case ServerOP_CZMarquee:
	{
		CZMarquee_Struct* CZM = (CZMarquee_Struct*) pack->pBuffer;
		uint8 update_type = CZM->update_type;
		int update_identifier = CZM->update_identifier;
		uint32 type = CZM->type;
		uint32 priority = CZM->priority;
		uint32 fade_in = CZM->fade_in;
		uint32 fade_out = CZM->fade_out;
		uint32 duration = CZM->duration;
		const char* message = CZM->message;
		const char* client_name = CZM->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				client->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
			}
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						group_member->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						raid_member->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					client.second->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					client.second->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				client->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
			}
		}
		break;
	}
	case ServerOP_CZMessage:
	{
		CZMessage_Struct* CZM = (CZMessage_Struct*) pack->pBuffer;
		uint8 update_type = CZM->update_type;
		int update_identifier = CZM->update_identifier;
		uint32 type = CZM->type;
		const char* message = CZM->message;
		const char* client_name = CZM->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				client->Message(type, message);
			}
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						group_member->Message(type, message);
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						raid_member->Message(type, message);
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					client.second->Message(type, message);
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					client.second->Message(type, message);
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				client->Message(type, message);
			}
		}
		break;
	}
	case ServerOP_CZMove:
	{
		CZMove_Struct* CZM = (CZMove_Struct*) pack->pBuffer;
		uint8 update_type = CZM->update_type;
		uint8 update_subtype = CZM->update_subtype;
		int update_identifier = CZM->update_identifier;
		const char* zone_short_name = CZM->zone_short_name;
		uint16 instance_id = CZM->instance_id;
		const char* client_name = CZM->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				switch (update_subtype) {
					case CZMoveUpdateSubtype_MoveZone:
						client->MoveZone(zone_short_name);
						break;
					case CZMoveUpdateSubtype_MoveZoneInstance:
						client->MoveZoneInstance(instance_id);
						break;
				}
			}
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						switch (update_subtype) {
							case CZMoveUpdateSubtype_MoveZone:
								group_member->MoveZone(zone_short_name);
								break;
							case CZMoveUpdateSubtype_MoveZoneInstance:
								group_member->MoveZoneInstance(instance_id);
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						switch (update_subtype) {
							case CZMoveUpdateSubtype_MoveZone:
								raid_member->MoveZone(zone_short_name);
								break;
							case CZMoveUpdateSubtype_MoveZoneInstance:
								raid_member->MoveZoneInstance(instance_id);
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					switch (update_subtype) {
						case CZMoveUpdateSubtype_MoveZone:
							client.second->MoveZone(zone_short_name);
							break;
						case CZMoveUpdateSubtype_MoveZoneInstance:
							client.second->MoveZoneInstance(instance_id);
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					switch (update_subtype) {
						case CZMoveUpdateSubtype_MoveZone:
							client.second->MoveZone(zone_short_name);
							break;
						case CZMoveUpdateSubtype_MoveZoneInstance:
							client.second->MoveZoneInstance(instance_id);
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				switch (update_subtype) {
					case CZMoveUpdateSubtype_MoveZone:
						client->MoveZone(zone_short_name);
						break;
					case CZMoveUpdateSubtype_MoveZoneInstance:
						client->MoveZoneInstance(instance_id);
						break;
				}
			}
		}
		break;
	}
	case ServerOP_CZSetEntityVariable:
	{
		CZSetEntityVariable_Struct* CZSEV = (CZSetEntityVariable_Struct*) pack->pBuffer;
		uint8 update_type = CZSEV->update_type;
		int update_identifier = CZSEV->update_identifier;
		const char* variable_name = CZSEV->variable_name;
		const char* variable_value = CZSEV->variable_value;
		const char* client_name = CZSEV->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				client->SetEntityVariable(variable_name, variable_value);
			}
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						group_member->SetEntityVariable(variable_name, variable_value);
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						raid_member->SetEntityVariable(variable_name, variable_value);
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					client.second->SetEntityVariable(variable_name, variable_value);
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					client.second->SetEntityVariable(variable_name, variable_value);
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				client->SetEntityVariable(variable_name, variable_value);
			}
		} else if (update_type == CZUpdateType_NPC) {
			auto npc = entity_list.GetNPCByNPCTypeID(update_identifier);
			if (npc) {
				npc->SetEntityVariable(variable_name, variable_value);
			}
		}
		break;
	}
	case ServerOP_CZSignal:
	{
		CZSignal_Struct* CZS = (CZSignal_Struct*) pack->pBuffer;
		uint8 update_type = CZS->update_type;
		int update_identifier = CZS->update_identifier;
		uint32 signal = CZS->signal;
		const char* client_name = CZS->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				client->Signal(signal);
			}
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						group_member->Signal(signal);
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						raid_member->Signal(signal);
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					client.second->Signal(signal);
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					client.second->Signal(signal);
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				client->Signal(signal);
			}
		} else if (update_type == CZUpdateType_NPC) {
			auto npc = entity_list.GetNPCByNPCTypeID(update_identifier);
			if (npc) {
				npc->SignalNPC(signal);
			}
		}
		break;
	}
	case ServerOP_CZSpell:
	{
		CZSpell_Struct* CZS = (CZSpell_Struct*) pack->pBuffer;
		uint8 update_type = CZS->update_type;
		uint8 update_subtype = CZS->update_subtype;
		int update_identifier = CZS->update_identifier;
		uint32 spell_id = CZS->spell_id;
		const char* client_name = CZS->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				switch (update_subtype) {
					case CZSpellUpdateSubtype_Cast:
						client->ApplySpellBuff(spell_id);
						break;
					case CZSpellUpdateSubtype_Remove:
						client->BuffFadeBySpellID(spell_id);
						break;
				}
			}
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						switch (update_subtype) {
							case CZSpellUpdateSubtype_Cast:
								group_member->ApplySpellBuff(spell_id);
								break;
							case CZSpellUpdateSubtype_Remove:
								group_member->BuffFadeBySpellID(spell_id);
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						switch (update_subtype) {
							case CZSpellUpdateSubtype_Cast:
								raid_member->ApplySpellBuff(spell_id);
								break;
							case CZSpellUpdateSubtype_Remove:
								raid_member->BuffFadeBySpellID(spell_id);
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					switch (update_subtype) {
						case CZSpellUpdateSubtype_Cast:
							client.second->ApplySpellBuff(spell_id);
							break;
						case CZSpellUpdateSubtype_Remove:
							client.second->BuffFadeBySpellID(spell_id);
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					switch (update_subtype) {
						case CZSpellUpdateSubtype_Cast:
							client.second->ApplySpellBuff(spell_id);
							break;
						case CZSpellUpdateSubtype_Remove:
							client.second->BuffFadeBySpellID(spell_id);
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				switch (update_subtype) {
					case CZSpellUpdateSubtype_Cast:
						client->ApplySpellBuff(spell_id);
						break;
					case CZSpellUpdateSubtype_Remove:
						client->BuffFadeBySpellID(spell_id);
						break;
				}
			}
		}
		break;
	}
	case ServerOP_CZTaskUpdate:
	{
		CZTaskUpdate_Struct* CZTU = (CZTaskUpdate_Struct*) pack->pBuffer;
		uint8 update_type = CZTU->update_type;
		uint8 update_subtype = CZTU->update_subtype;
		int update_identifier = CZTU->update_identifier;
		uint32 task_identifier = CZTU->task_identifier;
		int task_subidentifier = CZTU->task_subidentifier;
		int update_count = CZTU->update_count;
		bool enforce_level_requirement = CZTU->enforce_level_requirement;
		const char* client_name = CZTU->client_name;
		if (update_type == CZUpdateType_Character) {
			auto client = entity_list.GetClientByCharID(update_identifier);
			if (client) {
				switch (update_subtype) {
					case CZTaskUpdateSubtype_ActivityReset:
						client->ResetTaskActivity(task_identifier, task_subidentifier);
						break;
					case CZTaskUpdateSubtype_ActivityUpdate:
						client->UpdateTaskActivity(task_identifier, task_subidentifier, update_count);
						break;
					case CZTaskUpdateSubtype_AssignTask:
						client->AssignTask(task_identifier, task_subidentifier, enforce_level_requirement);
						break;
					case CZTaskUpdateSubtype_DisableTask:
						client->DisableTask(1, reinterpret_cast<int *>(task_identifier));
						break;
					case CZTaskUpdateSubtype_EnableTask:
						client->EnableTask(1, reinterpret_cast<int *>(task_identifier));
						break;
					case CZTaskUpdateSubtype_FailTask:
						client->FailTask(task_identifier);
						break;
					case CZTaskUpdateSubtype_RemoveTask:
						client->RemoveTaskByTaskID(task_identifier);
						break;
				}
			}
			break;
		} else if (update_type == CZUpdateType_Group) {
			auto client_group = entity_list.GetGroupByID(update_identifier);
			if (client_group) {
				for (int member_index = 0; member_index < MAX_GROUP_MEMBERS; member_index++) {
					if (client_group->members[member_index] && client_group->members[member_index]->IsClient()) {
						auto group_member = client_group->members[member_index]->CastToClient();
						switch (update_subtype) {
							case CZTaskUpdateSubtype_ActivityReset:
								group_member->ResetTaskActivity(task_identifier, task_subidentifier);
								break;
							case CZTaskUpdateSubtype_ActivityUpdate:
								group_member->UpdateTaskActivity(task_identifier, task_subidentifier, update_count);
								break;
							case CZTaskUpdateSubtype_AssignTask:
								group_member->AssignTask(task_identifier, task_subidentifier, enforce_level_requirement);
								break;
							case CZTaskUpdateSubtype_DisableTask:
								group_member->DisableTask(1, reinterpret_cast<int *>(task_identifier));
								break;
							case CZTaskUpdateSubtype_EnableTask:
								group_member->EnableTask(1, reinterpret_cast<int *>(task_identifier));
								break;
							case CZTaskUpdateSubtype_FailTask:
								group_member->FailTask(task_identifier);
								break;
							case CZTaskUpdateSubtype_RemoveTask:
								group_member->RemoveTaskByTaskID(task_identifier);
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Raid) {
			auto client_raid = entity_list.GetRaidByID(update_identifier);
			if (client_raid) {
				for (int member_index = 0; member_index < MAX_RAID_MEMBERS; member_index++) {
					if (client_raid->members[member_index].member && client_raid->members[member_index].member->IsClient()) {
						auto raid_member = client_raid->members[member_index].member->CastToClient();
						switch (update_subtype) {
							case CZTaskUpdateSubtype_ActivityReset:
								raid_member->ResetTaskActivity(task_identifier, task_subidentifier);
								break;
							case CZTaskUpdateSubtype_ActivityUpdate:
								raid_member->UpdateTaskActivity(task_identifier, task_subidentifier, update_count);
								break;
							case CZTaskUpdateSubtype_AssignTask:
								raid_member->AssignTask(task_identifier, task_subidentifier, enforce_level_requirement);
								break;
							case CZTaskUpdateSubtype_DisableTask:
								raid_member->DisableTask(1, reinterpret_cast<int *>(task_identifier));
								break;
							case CZTaskUpdateSubtype_EnableTask:
								raid_member->EnableTask(1, reinterpret_cast<int *>(task_identifier));
								break;
							case CZTaskUpdateSubtype_FailTask:
								raid_member->FailTask(task_identifier);
								break;
							case CZTaskUpdateSubtype_RemoveTask:
								raid_member->RemoveTaskByTaskID(task_identifier);
								break;
						}
					}
				}
			}
		} else if (update_type == CZUpdateType_Guild) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GuildID() > 0 && client.second->GuildID() == update_identifier) {
					switch (update_subtype) {
						case CZTaskUpdateSubtype_ActivityReset:
							client.second->ResetTaskActivity(task_identifier, task_subidentifier);
							break;
						case CZTaskUpdateSubtype_ActivityUpdate:
							client.second->UpdateTaskActivity(task_identifier, task_subidentifier, update_count);
							break;
						case CZTaskUpdateSubtype_AssignTask:
							client.second->AssignTask(task_identifier, task_subidentifier, enforce_level_requirement);
							break;
						case CZTaskUpdateSubtype_DisableTask:
							client.second->DisableTask(1, reinterpret_cast<int *>(task_identifier));
							break;
						case CZTaskUpdateSubtype_EnableTask:
							client.second->EnableTask(1, reinterpret_cast<int *>(task_identifier));
							break;
						case CZTaskUpdateSubtype_FailTask:
							client.second->FailTask(task_identifier);
							break;
						case CZTaskUpdateSubtype_RemoveTask:
							client.second->RemoveTaskByTaskID(task_identifier);
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_Expedition) {
			for (auto &client: entity_list.GetClientList()) {
				if (client.second->GetExpedition() && client.second->GetExpedition()->GetID() == update_identifier) {
					switch (update_subtype) {
						case CZTaskUpdateSubtype_ActivityReset:
							client.second->ResetTaskActivity(task_identifier, task_subidentifier);
							break;
						case CZTaskUpdateSubtype_ActivityUpdate:
							client.second->UpdateTaskActivity(task_identifier, task_subidentifier, update_count);
							break;
						case CZTaskUpdateSubtype_AssignTask:
							client.second->AssignTask(task_identifier, task_subidentifier, enforce_level_requirement);
							break;
						case CZTaskUpdateSubtype_DisableTask:
							client.second->DisableTask(1, reinterpret_cast<int *>(task_identifier));
							break;
						case CZTaskUpdateSubtype_EnableTask:
							client.second->EnableTask(1, reinterpret_cast<int *>(task_identifier));
							break;
						case CZTaskUpdateSubtype_FailTask:
							client.second->FailTask(task_identifier);
							break;
						case CZTaskUpdateSubtype_RemoveTask:
							client.second->RemoveTaskByTaskID(task_identifier);
							break;
					}
				}
			}
		} else if (update_type == CZUpdateType_ClientName) {
			auto client = entity_list.GetClientByName(client_name);
			if (client) {
				switch (update_subtype) {
					case CZTaskUpdateSubtype_ActivityReset:
						client->ResetTaskActivity(task_identifier, task_subidentifier);
						break;
					case CZTaskUpdateSubtype_ActivityUpdate:
						client->UpdateTaskActivity(task_identifier, task_subidentifier, update_count);
						break;
					case CZTaskUpdateSubtype_AssignTask:
						client->AssignTask(task_identifier, task_subidentifier, enforce_level_requirement);
						break;
					case CZTaskUpdateSubtype_DisableTask:
						client->DisableTask(1, reinterpret_cast<int *>(task_identifier));
						break;
					case CZTaskUpdateSubtype_EnableTask:
						client->EnableTask(1, reinterpret_cast<int *>(task_identifier));
						break;
					case CZTaskUpdateSubtype_FailTask:
						client->FailTask(task_identifier);
						break;
					case CZTaskUpdateSubtype_RemoveTask:
						client->RemoveTaskByTaskID(task_identifier);
						break;
				}
			}
		}
		break;
	}
	case ServerOP_WWDialogueWindow:
	{
		WWDialogueWindow_Struct* WWDW = (WWDialogueWindow_Struct*) pack->pBuffer;
		std::string message = WWDW->message;
		uint8 min_status = WWDW->min_status;
		uint8 max_status = WWDW->max_status;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
				DialogueWindow::Render(client.second, message);
			}
		}
		break;
	}
	case ServerOP_WWLDoNUpdate:
	{
		WWLDoNUpdate_Struct* WWLU = (WWLDoNUpdate_Struct*) pack->pBuffer;
		uint8 update_type = WWLU->update_type;
		uint32 theme_id = WWLU->theme_id;
		int points = WWLU->points;
		uint8 min_status = WWLU->min_status;
		uint8 max_status = WWLU->max_status;
		for (auto &client : entity_list.GetClientList()) {
			switch (update_type) {
				case WWLDoNUpdateType_AddLoss:
					if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
						client.second->UpdateLDoNWinLoss(theme_id, false);
					}
					break;
				case WWLDoNUpdateType_AddPoints:
					if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
						client.second->UpdateLDoNPoints(theme_id, points);
					}
					break;
				case WWLDoNUpdateType_AddWin:
					if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
						client.second->UpdateLDoNWinLoss(theme_id, true);
					}
					break;
				case WWLDoNUpdateType_RemoveLoss:
					if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
						client.second->UpdateLDoNWinLoss(theme_id, false, true);
					}
					break;
				case WWLDoNUpdateType_RemoveWin:
					if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
						client.second->UpdateLDoNWinLoss(theme_id, true, true);
					}
					break;
			}
		}
		break;
	}
	case ServerOP_WWMarquee:
	{
		WWMarquee_Struct* WWM = (WWMarquee_Struct*) pack->pBuffer;
		uint32 type = WWM->type;
		uint32 priority = WWM->priority;
		uint32 fade_in = WWM->fade_in;
		uint32 fade_out = WWM->fade_out;
		uint32 duration = WWM->duration;
		const char* message = WWM->message;
		uint8 min_status = WWM->min_status;
		uint8 max_status = WWM->max_status;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
				client.second->SendMarqueeMessage(type, priority, fade_in, fade_out, duration, message);
			}
		}
		break;
	}
	case ServerOP_WWMessage:
	{
		WWMessage_Struct* WWM = (WWMessage_Struct*) pack->pBuffer;
		uint32 type = WWM->type;
		const char* message = WWM->message;
		uint8 min_status = WWM->min_status;
		uint8 max_status = WWM->max_status;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
				client.second->Message(type, message);
			}
		}
		break;
	}
	case ServerOP_WWMove:
	{
		WWMove_Struct* WWM = (WWMove_Struct*) pack->pBuffer;
		uint8 update_type = WWM->update_type;
		uint16 instance_id = WWM->instance_id;
		const char* zone_short_name = WWM->zone_short_name;
		uint8 min_status = WWM->min_status;
		uint8 max_status = WWM->max_status;
		for (auto &client : entity_list.GetClientList()) {
			switch (update_type) {
				case WWMoveUpdateType_MoveZone:
					if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
						client.second->MoveZone(zone_short_name);
					}
					break;
				case WWMoveUpdateType_MoveZoneInstance:
					if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
						client.second->MoveZoneInstance(instance_id);
					}
					break;
			}
		}
		break;
	}
	case ServerOP_WWSetEntityVariable:
	{
		WWSetEntityVariable_Struct* WWSEV = (WWSetEntityVariable_Struct*) pack->pBuffer;
		uint8 update_type = WWSEV->update_type;
		const char* variable_name = WWSEV->variable_name;
		const char* variable_value = WWSEV->variable_value;
		uint8 min_status = WWSEV->min_status;
		uint8 max_status = WWSEV->max_status;
		if (update_type == WWSetEntityVariableUpdateType_Character) {
			for (auto &client : entity_list.GetClientList()) {
				if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
					client.second->SetEntityVariable(variable_name, variable_value);
				}
			}
		} else if (update_type == WWSetEntityVariableUpdateType_NPC) {
			for (auto &npc : entity_list.GetNPCList()) {
				npc.second->SetEntityVariable(variable_name, variable_value);
			}
		}
		break;
	}
	case ServerOP_WWSignal:
	{
		WWSignal_Struct* WWS = (WWSignal_Struct*) pack->pBuffer;
		uint8 update_type = WWS->update_type;
		uint32 signal = WWS->signal;
		uint8 min_status = WWS->min_status;
		uint8 max_status = WWS->max_status;
		if (update_type == WWSignalUpdateType_Character) {
			for (auto &client : entity_list.GetClientList()) {
				if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
					client.second->Signal(signal);
				}
			}
		} else if (update_type == WWSignalUpdateType_NPC) {
			for (auto &npc : entity_list.GetNPCList()) {
				npc.second->SignalNPC(signal);
			}
		}
		break;
	}
	case ServerOP_WWSpell:
	{
		WWSpell_Struct* WWS = (WWSpell_Struct*) pack->pBuffer;
		uint8 update_type = WWS->update_type;
		uint32 spell_id = WWS->spell_id;
		uint8 min_status = WWS->min_status;
		uint8 max_status = WWS->max_status;
		if (update_type == WWSpellUpdateType_Cast) {
			for (auto &client : entity_list.GetClientList()) {
				if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
					client.second->ApplySpellBuff(spell_id);
				}
			}
		} else if (update_type == WWSpellUpdateType_Remove) {
			for (auto &client : entity_list.GetClientList()) {
				if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
					client.second->BuffFadeBySpellID(spell_id);
				}
			}
		}
		break;
	}
	case ServerOP_WWTaskUpdate:
	{
		WWTaskUpdate_Struct* WWTU = (WWTaskUpdate_Struct*) pack->pBuffer;
		uint8 update_type = WWTU->update_type;
		uint32 task_identifier = WWTU->task_identifier;
		int task_subidentifier = WWTU->task_subidentifier;
		int update_count = WWTU->update_count;
		bool enforce_level_requirement = WWTU->enforce_level_requirement;
		uint8 min_status = WWTU->min_status;
		uint8 max_status = WWTU->max_status;
		for (auto &client : entity_list.GetClientList()) {
			if (client.second->Admin() >= min_status && (client.second->Admin() <= max_status || max_status == AccountStatus::Player)) {
				switch (update_type) {
					case WWTaskUpdateType_ActivityReset:
						client.second->ResetTaskActivity(task_identifier, task_subidentifier);
						break;
					case WWTaskUpdateType_ActivityUpdate:
						client.second->UpdateTaskActivity(task_identifier, task_subidentifier, update_count);
						break;
					case WWTaskUpdateType_AssignTask:
						client.second->AssignTask(task_identifier, task_subidentifier, enforce_level_requirement);
						break;
					case WWTaskUpdateType_DisableTask:
						client.second->DisableTask(1, reinterpret_cast<int *>(task_identifier));
						break;
					case WWTaskUpdateType_EnableTask:
						client.second->EnableTask(1, reinterpret_cast<int *>(task_identifier));
						break;
					case WWTaskUpdateType_FailTask:
						client.second->FailTask(task_identifier);
						break;
					case WWTaskUpdateType_RemoveTask:
						client.second->RemoveTaskByTaskID(task_identifier);
						break;
				}
			}
		}
		break;
	}
	case ServerOP_UpdateSchedulerEvents: {
		LogScheduler("Received signal from world to update");
		if (m_zone_scheduler) {
			m_zone_scheduler->LoadScheduledEvents();
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
			std::string reload_quest_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
				"#reload quest",
				false,
				"Locally"
			);
			std::string reload_world_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
				"#reload world",
				false,
				"Globally"
			);
			worldserver.SendEmoteMessage(
				0,
				0,
				AccountStatus::ApprenticeGuide,
				Chat::Yellow,
				fmt::format(
					"A quest, plugin, or global script has changed. Reload: [{}] [{}]",
					reload_quest_saylink,
					reload_world_saylink
				).c_str()
			);
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
	case ServerOP_CZClientMessageString:
	{
		auto buf = reinterpret_cast<CZClientMessageString_Struct*>(pack->pBuffer);
		Client* client = entity_list.GetClientByName(buf->client_name);
		if (client) {
			client->MessageString(buf);
		}
		break;
	}
	case ServerOP_ExpeditionCreate:
	case ServerOP_ExpeditionLockout:
	case ServerOP_ExpeditionLockoutDuration:
	case ServerOP_ExpeditionLockState:
	case ServerOP_ExpeditionReplayOnJoin:
	case ServerOP_ExpeditionDzAddPlayer:
	case ServerOP_ExpeditionDzMakeLeader:
	case ServerOP_ExpeditionCharacterLockout:
	{
		Expedition::HandleWorldMessage(pack);
		break;
	}
	case ServerOP_DzCreated:
	case ServerOP_DzDeleted:
	case ServerOP_DzAddRemoveMember:
	case ServerOP_DzSwapMembers:
	case ServerOP_DzRemoveAllMembers:
	case ServerOP_DzDurationUpdate:
	case ServerOP_DzGetMemberStatuses:
	case ServerOP_DzSetCompass:
	case ServerOP_DzSetSafeReturn:
	case ServerOP_DzSetZoneIn:
	case ServerOP_DzUpdateMemberStatus:
	case ServerOP_DzLeaderChanged:
	case ServerOP_DzExpireWarning:
	{
		DynamicZone::HandleWorldMessage(pack);
		break;
	}
	case ServerOP_SharedTaskAcceptNewTask:
	case ServerOP_SharedTaskUpdate:
	case ServerOP_SharedTaskAttemptRemove:
	case ServerOP_SharedTaskMemberlist:
	case ServerOP_SharedTaskMemberChange:
	case ServerOP_SharedTaskInvitePlayer:
	case ServerOP_SharedTaskPurgeAllCommand:
	{
		SharedTaskZoneMessaging::HandleWorldMessage(pack);
		break;
	}
	default: {
		LogInfo("[HandleMessage] Unknown ZS Opcode [{}] size [{}]", (int)pack->opcode, pack->size);
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
		scm->fromadmin = AccountStatus::Player;
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

	return SendEmoteMessage(
		to,
		to_guilddbid,
		AccountStatus::Player,
		type,
		buffer
	);
}

bool WorldServer::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	va_list argptr;
	char buffer[4096] = { 0 };

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer) - 1, message, argptr);
	va_end(argptr);

	if (!Connected() && to == 0) {
		entity_list.MessageStatus(
			to_guilddbid,
			to_minstatus,
			type,
			buffer
		);
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

	uint16 player_race = GetPlayerRaceValue(From->GetRace());

	if (player_race == PLAYER_RACE_UNKNOWN) {
		player_race = From->GetBaseRace();
	}

	svm->Voice = (player_race * 2) + From->GetGender();

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

void WorldServer::SendReloadTasks(uint8 reload_type, uint32 task_id) {
	auto pack = new ServerPacket(ServerOP_ReloadTasks, sizeof(ReloadTasks_Struct));
	auto rts = (ReloadTasks_Struct*) pack->pBuffer;

	rts->reload_type = reload_type;
	rts->task_id = task_id;

	SendPacket(pack);
}

void WorldServer::HandleReloadTasks(ServerPacket *pack)
{
	auto rts = (ReloadTasks_Struct*) pack->pBuffer;

	LogTasks("Global reload of tasks received with Reload Type [{}] Task ID [{}]", rts->reload_type, rts->task_id);

	switch (rts->reload_type) {
		case RELOADTASKS:
		{
			entity_list.SaveAllClientsTaskState();

			// TODO: Reload at the world level for shared tasks

			if (!rts->task_id) {
				LogTasks("Global reload of all Tasks");
				safe_delete(task_manager);
				task_manager = new TaskManager;
				task_manager->LoadTasks();

				if (zone) {
					task_manager->LoadProximities(zone->GetZoneID());
				}

				entity_list.ReloadAllClientsTaskState();
			} else {
				LogTasks("Global reload of Task ID [{}]", rts->task_id);
				task_manager->LoadTasks(rts->task_id);
				entity_list.ReloadAllClientsTaskState(rts->task_id);
			}

			break;
		}
		case RELOADTASKPROXIMITIES:
		{
			if (zone) {
				LogTasks("Global reload of all Task Proximities");
				task_manager->LoadProximities(zone->GetZoneID());
			}

			break;
		}
		case RELOADTASKGOALLISTS:
		{
			LogTasks("Global reload of all Task Goal Lists");
			task_manager->ReloadGoalLists();
			break;
		}
		case RELOADTASKSETS:
		{
			LogTasks("Global reload of all Task Sets");
			task_manager->LoadTaskSets();
			break;
		}
		default:
		{
			LogTasks("Unhandled global reload of Tasks Reload Type [{}] Task ID [{}]", rts->reload_type, rts->task_id);
			break;
		}
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

	GroupLFPMemberEntry LFPMembers[MAX_GROUP_MEMBERS];
	UpdateLFP(LeaderID, LFPOff, 0, 0, 0, 0, "", LFPMembers);
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

ZoneEventScheduler *WorldServer::GetScheduler() const
{
	return m_zone_scheduler;
}

void WorldServer::SetScheduler(ZoneEventScheduler *scheduler)
{
	WorldServer::m_zone_scheduler = scheduler;
}

