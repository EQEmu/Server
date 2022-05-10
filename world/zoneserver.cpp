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
#include "zoneserver.h"
#include "clientlist.h"
#include "login_server.h"
#include "login_server_list.h"
#include "zonelist.h"
#include "worlddb.h"
#include "client.h"
#include "../common/md5.h"
#include "world_config.h"
#include "../common/guilds.h"
#include "../common/packet_dump.h"
#include "../common/misc.h"
#include "../common/string_util.h"
#include "cliententry.h"
#include "wguild_mgr.h"
#include "lfplist.h"
#include "adventure_manager.h"
#include "ucs.h"
#include "queryserv.h"
#include "world_store.h"
#include "dynamic_zone.h"
#include "expedition_message.h"
#include "shared_task_world_messaging.h"
#include "../common/shared_tasks.h"
#include "shared_task_manager.h"
#include "../common/content/world_content_service.h"

extern ClientList client_list;
extern GroupLFPList LFPGroupList;
extern ZSList zoneserver_list;
extern LoginServerList loginserverlist;
extern volatile bool RunLoops;
extern volatile bool UCSServerAvailable_;
extern AdventureManager adventure_manager;
extern UCSConnection UCSLink;
extern QueryServConnection QSLink;
extern SharedTaskManager shared_task_manager;

void CatchSignal(int sig_num);

ZoneServer::ZoneServer(std::shared_ptr<EQ::Net::ServertalkServerConnection> in_connection, EQ::Net::ConsoleServer *in_console)
	: tcpc(in_connection), zone_boot_timer(5000) {

	/* Set Process tracking variable defaults */
	memset(zone_name, 0, sizeof(zone_name));
	memset(compiled, 0, sizeof(compiled));
	memset(client_address, 0, sizeof(client_address));
	memset(client_local_address, 0, sizeof(client_local_address));

	zone_server_id = zoneserver_list.GetNextID();
	zone_server_zone_id = 0;
	instance_id = 0;
	zone_os_process_id = 0;
	client_port = 0;
	is_booting_up = false;
	is_authenticated = false;
	is_static_zone = false;
	zone_player_count = 0;

	tcpc->OnMessage(std::bind(&ZoneServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));

	boot_timer_obj = std::make_unique<EQ::Timer>(100, true, [this](EQ::Timer *obj) {
		if (zone_boot_timer.Check()) {
			LSBootUpdate(GetZoneID(), true);
			zone_boot_timer.Disable();
		}
	});

	console = in_console;
}

ZoneServer::~ZoneServer() {
	if (RunLoops) {
		client_list.CLERemoveZSRef(this);
	}
}

bool ZoneServer::SetZone(uint32 in_zone_id, uint32 in_instance_id, bool is_static_zone) {
	is_booting_up = false;

	std::string zone_short_name = ZoneName(in_zone_id, true);
	std::string zone_long_name = ZoneLongName(in_zone_id, true);

	if (in_zone_id) {
		LogInfo(
			"Setting zone process to Zone: {} ({}) ID: {}{}{}",
			zone_long_name,
			zone_short_name,
			in_zone_id,
			(
				in_instance_id ?
				fmt::format(
					" (Instance ID {})",
					in_instance_id
				) :
				""
			),
			is_static_zone ? " (Static)" : ""
		);
	}

	zone_server_zone_id = in_zone_id;
	instance_id = in_instance_id;
	if (in_zone_id) {
		zone_server_previous_zone_id = in_zone_id;
	}

	if (!zone_server_zone_id) {
		client_list.CLERemoveZSRef(this);
		zone_player_count = 0;
		LSSleepUpdate(GetPrevZoneID());
	}

	is_static_zone = is_static_zone;

	strn0cpy(zone_name, zone_short_name.c_str(), sizeof(zone_name));
	strn0cpy(long_name, zone_long_name.c_str(), sizeof(long_name));

	client_list.ZoneBootup(this);
	zone_boot_timer.Start();

	return true;
}

void ZoneServer::LSShutDownUpdate(uint32 zone_id) {
	if (WorldConfig::get()->UpdateStats) {
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_LSZoneShutdown;
		pack->size = sizeof(ZoneShutdown_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, pack->size);
		auto zsd = (ZoneShutdown_Struct*) pack->pBuffer;
		zsd->zone = zone_id ? zone_id : GetPrevZoneID();
		zsd->zone_wid = GetID();
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}
void ZoneServer::LSBootUpdate(uint32 zone_id, uint32 instanceid, bool startup) {
	if (WorldConfig::get()->UpdateStats) {
		auto pack = new ServerPacket;
		pack->opcode = startup ? ServerOP_LSZoneStart : ServerOP_LSZoneBoot;
		pack->size = sizeof(ZoneBoot_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, pack->size);
		auto bootup = (ZoneBoot_Struct*) pack->pBuffer;

		if (startup) {
			strcpy(bootup->compile_time, GetCompileTime());
		}

		bootup->zone = zone_id;
		bootup->zone_wid = GetID();
		bootup->instance = instanceid;
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}

void ZoneServer::LSSleepUpdate(uint32 zone_id) {
	if (WorldConfig::get()->UpdateStats) {
		auto pack = new ServerPacket;
		pack->opcode = ServerOP_LSZoneSleep;
		pack->size = sizeof(ServerLSZoneSleep_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, pack->size);
		auto sleep = (ServerLSZoneSleep_Struct*) pack->pBuffer;
		sleep->zone = zone_id;
		sleep->zone_wid = GetID();
		loginserverlist.SendPacket(pack);
		safe_delete(pack);
	}
}

void ZoneServer::HandleMessage(uint16 opcode, const EQ::Net::Packet &p) {
	ServerPacket tpack(opcode, p);
	auto pack = &tpack;

	switch (opcode) {
		case 0:
		case ServerOP_KeepAlive:
		case ServerOP_ZAAuth: {
			break;
		}
		case ServerOP_LSZoneBoot: {
			if (pack->size == sizeof(ZoneBoot_Struct)) {
				auto zbs = (ZoneBoot_Struct*) pack->pBuffer;
				SetCompile(zbs->compile_time);
			}

			break;
		}
		case ServerOP_GroupInvite: {
			if (pack->size != sizeof(GroupInvite_Struct)) {
				break;
			}

			auto gis = (GroupInvite_Struct*) pack->pBuffer;
			client_list.SendPacket(gis->invitee_name, pack);
			break;
		}
		case ServerOP_GroupFollow: {
			if (pack->size != sizeof(ServerGroupFollow_Struct)) {
				break;
			}

			auto sgfs = (ServerGroupFollow_Struct*) pack->pBuffer;
			client_list.SendPacket(sgfs->gf.name1, pack);
			break;
		}
		case ServerOP_GroupFollowAck: {
			if (pack->size != sizeof(ServerGroupFollowAck_Struct)) {
				break;
			}

			auto sgfas = (ServerGroupFollowAck_Struct*) pack->pBuffer;
			client_list.SendPacket(sgfas->Name, pack);
			break;
		}
		case ServerOP_GroupCancelInvite: {
			if (pack->size != sizeof(GroupCancel_Struct)) {
				break;
			}

			auto gcs = (GroupCancel_Struct*) pack->pBuffer;
			client_list.SendPacket(gcs->name1, pack);
			break;
		}
		case ServerOP_GroupIDReq: {
			SendGroupIDs();
			break;
		}
		case ServerOP_GroupLeave: {
			if (pack->size != sizeof(ServerGroupLeave_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_GroupJoin: {
			if (pack->size != sizeof(ServerGroupJoin_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_ForceGroupUpdate: {
			if (pack->size != sizeof(ServerForceGroupUpdate_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_DisbandGroup: {
			if (pack->size != sizeof(ServerDisbandGroup_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidAdd: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidRemove: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidDisband: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidLockFlag: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidChangeGroup: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_UpdateGroup: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidGroupDisband: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidGroupAdd: {
			if (pack->size != sizeof(ServerRaidGroupAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidGroupRemove: {
			if (pack->size != sizeof(ServerRaidGroupAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidGroupLeader: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidLeader: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_DetailsChange: {
			if (pack->size != sizeof(ServerRaidGeneralAction_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RaidMOTD: {
			if (pack->size < sizeof(ServerRaidMOTD_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_SpawnCondition: {
			if (pack->size != sizeof(ServerSpawnCondition_Struct)) {
				break;
			}

			auto ssc = (ServerSpawnCondition_Struct*) pack->pBuffer;
			zoneserver_list.SendPacket(ssc->zoneID, ssc->instanceID, pack);
			break;
		}
		case ServerOP_SpawnEvent: {
			if (pack->size != sizeof(ServerSpawnEvent_Struct)) {
				break;
			}

			auto sse = (ServerSpawnEvent_Struct*) pack->pBuffer;
			zoneserver_list.SendPacket(sse->zoneID, 0, pack);
			break;
		}
		case ServerOP_ChannelMessage: {
			if (pack->size < sizeof(ServerChannelMessage_Struct)) {
				break;
			}

			auto scm = (ServerChannelMessage_Struct*) pack->pBuffer;
			if (scm->chan_num == ChatChannel_UCSRelay) {
				UCSLink.SendMessage(scm->from, scm->message);
				break;
			}

			if (scm->chan_num == ChatChannel_Tell || scm->chan_num == ChatChannel_TellEcho) {
				if (scm->deliverto[0] == '*') {
					if (console) {
						auto con = console->FindByAccountName(&scm->deliverto[1]);
						if (
							!con ||
							(
								!con->SendChannelMessage(
									scm,
									[&scm]() {
										auto pack = new ServerPacket(ServerOP_ChannelMessage, sizeof(ServerChannelMessage_Struct) + strlen(scm->message) + 1);
										memcpy(pack->pBuffer, scm, pack->size);
										auto scm2 = (ServerChannelMessage_Struct*) pack->pBuffer;
										strcpy(scm2->deliverto, scm2->from);
										scm2->noreply = true;
										client_list.SendPacket(scm->from, pack);
										safe_delete(pack);
									}
								)
							) &&
							!scm->noreply
						) {
							zoneserver_list.SendEmoteMessage(
								scm->from,
								0,
								AccountStatus::Player,
								Chat::White,
								fmt::format(
									"{} is not online at this time.",
									scm->to
								).c_str()
							);
						}
					}
					break;
				}

				auto cle = client_list.FindCharacter(scm->deliverto);
				if (
					!cle ||
					cle->Online() < CLE_Status::Zoning ||
					(
						cle->TellsOff() &&
						(
							(
								cle->Anon() == 1 &&
								scm->fromadmin < cle->Admin()
							) ||
							scm->fromadmin < AccountStatus::QuestTroupe
						)
					)
				) {
					if (!scm->noreply) {
						auto sender = client_list.FindCharacter(scm->from);
						if (!sender || !sender->Server()) {
							break;
						}

						scm->noreply = true;
						scm->queued = 3; // offline
						scm->chan_num = ChatChannel_TellEcho;
						strcpy(scm->deliverto, scm->from);
						sender->Server()->SendPacket(pack);
					}
				} else if (cle->Online() == CLE_Status::Zoning) {
					if (!scm->noreply) {
						auto sender = client_list.FindCharacter(scm->from);
						if (cle->TellQueueFull()) {
							if (!sender || !sender->Server()) {
								break;
							}

							scm->noreply = true;
							scm->queued = 2; // queue full
							scm->chan_num = ChatChannel_TellEcho;
							strcpy(scm->deliverto, scm->from);
							sender->Server()->SendPacket(pack);
						} else {
							size_t struct_size = sizeof(ServerChannelMessage_Struct) + strlen(scm->message) + 1;
							auto temp = (ServerChannelMessage_Struct *) new uchar[struct_size];
							memset(temp, 0, struct_size); // just in case, was seeing some corrupt messages, but it shouldn't happen
							memcpy(temp, scm, struct_size);
							temp->noreply = true;
							cle->PushToTellQueue(temp); // deallocation is handled in processing or deconstructor

							if (!sender || !sender->Server()) {
								break;
							}

							scm->noreply = true;
							scm->queued = 1; // queued
							scm->chan_num = ChatChannel_TellEcho;
							strcpy(scm->deliverto, scm->from);
							sender->Server()->SendPacket(pack);
						}
					}
				} else if (!cle->Server()) {
					if (!scm->noreply)
						zoneserver_list.SendEmoteMessage(
							scm->from,
							0,
							AccountStatus::Player,
							Chat::White,
							fmt::format(
								"You told {}, '{} is not contactable at this time'",
								scm->to,
								scm->to
							).c_str()
						);
				} else {
					cle->Server()->SendPacket(pack);
				}
			} else {
				if (
					scm->chan_num == ChatChannel_OOC ||
					scm->chan_num == ChatChannel_Broadcast ||
					scm->chan_num == ChatChannel_GMSAY
				) {
					if (console) {
						console->SendChannelMessage(
							scm,
							[&scm]() {
								auto pack = new ServerPacket(ServerOP_ChannelMessage, sizeof(ServerChannelMessage_Struct) + strlen(scm->message) + 1);
								memcpy(pack->pBuffer, scm, pack->size);
								auto scm2 = (ServerChannelMessage_Struct*) pack->pBuffer;
								strcpy(scm2->deliverto, scm2->from);
								scm2->noreply = true;
								client_list.SendPacket(scm->from, pack);
								safe_delete(pack);
							}
						);
					}
				}
				zoneserver_list.SendPacket(pack);
			}

			break;
		}
		case ServerOP_EmoteMessage: {
			auto sem = (ServerEmoteMessage_Struct*) pack->pBuffer;
			zoneserver_list.SendEmoteMessageRaw(
				sem->to,
				sem->guilddbid,
				sem->minstatus,
				sem->type,
				sem->message
			);
			break;
		}
		case ServerOP_VoiceMacro: {
			auto svm = (ServerVoiceMacro_Struct*) pack->pBuffer;
			if (svm->Type == VoiceMacroTell) {
				auto cle = client_list.FindCharacter(svm->To);
				if (
					!cle ||
					cle->Online() < CLE_Status::Zoning ||
					!cle->Server()
				) {
					zoneserver_list.SendEmoteMessage(
						svm->From,
						0,
						AccountStatus::Player,
						Chat::White,
						fmt::format(
							"'{} is not online at this time'",
							svm->To
						).c_str()
					);
					break;
				}
				cle->Server()->SendPacket(pack);
			} else {
				zoneserver_list.SendPacket(pack);
			}

			break;
		}
		case ServerOP_RezzPlayer: {
			auto rps = (RezzPlayer_Struct*) pack->pBuffer;
			if (zoneserver_list.SendPacket(pack)) {
				LogInfo("Sent Rez packet for [{}]", rps->rez.your_name);
			} else {
				LogInfo("Could not send Rez packet for [{}]", rps->rez.your_name);
			}

			break;
		}
		case ServerOP_RezzPlayerReject: {
			auto recipient = (char*) pack->pBuffer;
			client_list.SendPacket(recipient, pack);
			break;
		}
		case ServerOP_MultiLineMsg: {
			auto mlm = (ServerMultiLineMsg_Struct*) pack->pBuffer;
			client_list.SendPacket(mlm->to, pack);
			break;
		}
		case ServerOP_SetZone: {
			if (pack->size != sizeof(SetZone_Struct)) {
				break;
			}

			auto szs = (SetZone_Struct*) pack->pBuffer;
			if (szs->zoneid) {
				if (ZoneName(szs->zoneid)) {
					SetZone(szs->zoneid, szs->instanceid, szs->staticzone);
				} else {
					SetZone(0);
				}
			} else {
				SetZone(0);
			}

			break;
		}
		case ServerOP_SetConnectInfo: {
			if (pack->size != sizeof(ServerConnectInfo)) {
				break;
			}

			auto sci = (ServerConnectInfo*) pack->pBuffer;

			if (!sci->port) {
				client_port = zoneserver_list.GetAvailableZonePort();

				ServerPacket p(ServerOP_SetConnectInfo, sizeof(ServerConnectInfo));
				memset(p.pBuffer, 0, sizeof(ServerConnectInfo));
				ServerConnectInfo* sci = (ServerConnectInfo*)p.pBuffer;
				sci->port = client_port;
				SendPacket(&p);
				LogInfo("Auto zone port configuration. Telling zone to use port [{}]", client_port);
			} else {
				client_port = sci->port;
				LogInfo("Zone specified port [{}]", client_port);
			}

			if (sci->address[0]) {
				strn0cpy(client_address, sci->address, 250);
				LogInfo("Zone specified address [{}]", sci->address);
			}

			if (sci->local_address[0]) {
				strn0cpy(client_local_address, sci->local_address, 250);
				LogInfo("Zone specified local address [{}]", sci->local_address);
			}

			if (sci->process_id) {
				zone_os_process_id = sci->process_id;
			}

			break;
		}
		case ServerOP_SetLaunchName: {
			if (pack->size != sizeof(LaunchName_Struct)) {
				break;
			}

			auto ln = (const LaunchName_Struct*) pack->pBuffer;
			launcher_name = ln->launcher_name;
			launched_name = ln->zone_name;
			LogInfo("Zone started with name [{}] by launcher [{}]", launched_name.c_str(), launcher_name.c_str());
			break;
		}
		case ServerOP_ShutdownAll: {
			if (!pack->size) {
				zoneserver_list.SendPacket(pack);
				zoneserver_list.Process();
				CatchSignal(2);
			} else {
				auto wsd = (WorldShutDown_Struct*) pack->pBuffer;
				if (!wsd->time && !wsd->interval && zoneserver_list.shutdowntimer->Enabled()) {
					zoneserver_list.shutdowntimer->Disable();
					zoneserver_list.reminder->Disable();
				} else {
					zoneserver_list.shutdowntimer->SetTimer(wsd->time);
					zoneserver_list.reminder->SetTimer(wsd->interval - 1000);
					zoneserver_list.reminder->SetAtTrigger(wsd->interval);
					zoneserver_list.shutdowntimer->Start();
					zoneserver_list.reminder->Start();
				}
			}

			break;
		}
		case ServerOP_ZoneShutdown: {
			auto s = (ServerZoneStateChange_struct*) pack->pBuffer;
			ZoneServer* zs = 0;
			if (s->ZoneServerID) {
				zs = zoneserver_list.FindByID(s->ZoneServerID);
			} else if (s->zoneid) {
				zs = zoneserver_list.FindByName(ZoneName(s->zoneid));
			} else {
				zoneserver_list.SendEmoteMessage(
					s->adminname,
					0,
					AccountStatus::Player,
					Chat::White,
					"Error: SOP_ZoneShutdown: neither ID nor name specified"
				);
			}

			if (!zs) {
				zoneserver_list.SendEmoteMessage(
					s->adminname,
					0,
					AccountStatus::Player,
					Chat::White,
					"Error: SOP_ZoneShutdown: zoneserver not found"
				);
			} else {
				zs->SendPacket(pack);
			}

			break;
		}
		case ServerOP_ZoneBootup: {
			auto s = (ServerZoneStateChange_struct*) pack->pBuffer;
			zoneserver_list.SOPZoneBootup(s->adminname, s->ZoneServerID, ZoneName(s->zoneid), s->makestatic);
			break;
		}
		case ServerOP_ZoneStatus: {
			if (pack->size >= 1) {
				zoneserver_list.SendZoneStatus((char *)&pack->pBuffer[1], (uint8)pack->pBuffer[0], this);
			}

			break;
		}
		case ServerOP_AcceptWorldEntrance: {
			if (pack->size != sizeof(WorldToZone_Struct)) {
				break;
			}

			auto wtz = (WorldToZone_Struct*) pack->pBuffer;
			auto client = client_list.FindByAccountID(wtz->account_id);
			if (client) {
				client->Clearance(wtz->response);
			}
		}
		case ServerOP_ZoneToZoneRequest: {
			// ZoneChange is received by the zone the player is in, then the
			// zone sends a ZTZ which ends up here. This code then find the target
			// (ingress point) and boots it if needed, then sends the ZTZ to it.
			// The ingress server will decide wether the player can enter, then will
			// send back the ZTZ to here. This packet is passed back to the egress
			// server, which will send a ZoneChange response back to the client
			// which can be an error, or a success, in which case the client will
			// disconnect, and their zone location will be saved when ~Client is
			// called, so it will be available when they ask to zone.

			if (pack->size != sizeof(ZoneToZone_Struct)) {
				break;
			}

			auto ztz = (ZoneToZone_Struct*) pack->pBuffer;
			ClientListEntry* client = nullptr;
			if (WorldConfig::get()->UpdateStats) {
				client = client_list.FindCharacter(ztz->name);
			}

			LogInfo("ZoneToZone request for [{}] current zone [{}] req zone [{}]", ztz->name, ztz->current_zone_id, ztz->requested_zone_id);

			/* This is a request from the egress zone */
			if (GetZoneID() == ztz->current_zone_id && GetInstanceID() == ztz->current_instance_id) {
				LogInfo("Processing ZTZ for egress from zone for client [{}]", ztz->name);

				if (
					ztz->admin < AccountStatus::QuestTroupe &&
					ztz->ignorerestrictions < 2 &&
					zoneserver_list.IsZoneLocked(ztz->requested_zone_id)
				) {
					ztz->response = 0;
					SendPacket(pack);
					break;
				}

				auto ingress_server = (
					ztz->requested_instance_id ?
					zoneserver_list.FindByInstanceID(ztz->requested_instance_id) :
					zoneserver_list.FindByZoneID(ztz->requested_zone_id)
				);

				if (ingress_server) {
					LogInfo("Found a zone already booted for [{}]", ztz->name);
					ztz->response = 1;
				} else {
					int server_id;
					if ((server_id = zoneserver_list.TriggerBootup(ztz->requested_zone_id, ztz->requested_instance_id))) {
						LogInfo("Successfully booted a zone for [{}]", ztz->name);
						ztz->response = 1;
						ingress_server = zoneserver_list.FindByID(server_id);
					} else {
						LogError("failed to boot a zone for [{}]", ztz->name);
						ztz->response = 0;
					}
				}

				if (ztz->response && client) {
					client->LSZoneChange(ztz);
				}

				SendPacket(pack);	// send back to egress server
				if (ingress_server) {
					ingress_server->SendPacket(pack);	// inform target server
				}
			} else {
				LogInfo("Processing ZTZ for ingress to zone for client [{}]", ztz->name);
				auto egress_server = (
					ztz->current_instance_id ?
					zoneserver_list.FindByInstanceID(ztz->current_instance_id) :
					zoneserver_list.FindByZoneID(ztz->current_zone_id)
				);

				if (egress_server) {
					egress_server->SendPacket(pack);
				}
			}

			break;
		}
		case ServerOP_ClientList: {
			if (pack->size != sizeof(ServerClientList_Struct)) {
				LogInfo("Wrong size on ServerOP_ClientList. Got: [{}], Expected: [{}]", pack->size, sizeof(ServerClientList_Struct));
				break;
			}

			auto scl = (ServerClientList_Struct*) pack->pBuffer;
			client_list.ClientUpdate(this, scl);
			break;
		}
		case ServerOP_ClientListKA: {
			auto sclka = (ServerClientListKeepAlive_Struct*) pack->pBuffer;
			if (pack->size < 4 || pack->size != 4 + (4 * sclka->numupdates)) {
				LogInfo("Wrong size on ServerOP_ClientListKA. Got: [{}], Expected: [{}]", pack->size, (4 + (4 * sclka->numupdates)));
				break;
			}

			client_list.CLEKeepAlive(sclka->numupdates, sclka->wid);
			break;
		}
		case ServerOP_Who: {
			auto whoall = (ServerWhoAll_Struct*) pack->pBuffer;
			auto whom = new Who_All_Struct;
			memset(whom, 0, sizeof(Who_All_Struct));
			whom->gmlookup = whoall->gmlookup;
			whom->lvllow = whoall->lvllow;
			whom->lvlhigh = whoall->lvlhigh;
			whom->wclass = whoall->wclass;
			whom->wrace = whoall->wrace;
			strn0cpy(whom->whom, whoall->whom, sizeof(whom->whom));
			client_list.SendWhoAll(whoall->fromid, whoall->from, whoall->admin, whom, this);
			safe_delete(whom);
			break;
		}
		case ServerOP_RequestOnlineGuildMembers: {
			auto srogms = (ServerRequestOnlineGuildMembers_Struct*) pack->pBuffer;
			client_list.SendOnlineGuildMembers(srogms->FromID, srogms->GuildID);
			break;
		}
		case ServerOP_ClientVersionSummary: {
			auto srcvss = (ServerRequestClientVersionSummary_Struct*) pack->pBuffer;
			client_list.SendClientVersionSummary(srcvss->Name);
			break;
		}
		case ServerOP_FriendsWho: {
			auto sfw = (ServerFriendsWho_Struct*) pack->pBuffer;
			client_list.SendFriendsWho(sfw, this);
			break;
		}
		case ServerOP_LFGMatches: {
			auto smrs = (ServerLFGMatchesRequest_Struct*) pack->pBuffer;
			client_list.SendLFGMatches(smrs);
			break;
		}
		case ServerOP_LFPMatches: {
			auto smrs = (ServerLFPMatchesRequest_Struct*) pack->pBuffer;
			LFPGroupList.SendLFPMatches(smrs);
			break;
		}
		case ServerOP_LFPUpdate: {
			auto sus = (ServerLFPUpdate_Struct*) pack->pBuffer;
			if (sus->Action) {
				LFPGroupList.UpdateGroup(sus);
			} else {
				LFPGroupList.RemoveGroup(sus);
			}

			break;
		}
		case ServerOP_DeleteGuild:
		case ServerOP_GuildCharRefresh:
		case ServerOP_GuildMemberUpdate:
		case ServerOP_RefreshGuild: {
			guild_mgr.ProcessZonePacket(pack);
			break;
		}
		case ServerOP_FlagUpdate: {
			auto cle = client_list.FindCLEByAccountID(*((uint32*) pack->pBuffer));
			if (cle) {
				cle->SetAdmin(*((int16*)&pack->pBuffer[4]));
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_GMGoto: {
			if (pack->size != sizeof(ServerGMGoto_Struct)) {
				LogInfo("Wrong size on ServerOP_GMGoto. Got: [{}], Expected: [{}]", pack->size, sizeof(ServerGMGoto_Struct));
				break;
			}

			auto gmg = (ServerGMGoto_Struct*) pack->pBuffer;
			auto cle = client_list.FindCharacter(gmg->gotoname);
			if (cle) {
				if (!cle->Server()) {
					SendEmoteMessage(
						gmg->myname,
						0,
						AccountStatus::Player,
						Chat::Red,
						fmt::format(
							"Error: Cannot identify {}'s zoneserver.",
							gmg->gotoname
						).c_str()
					);
				} else if (cle->Anon() == 1 && cle->Admin() > gmg->admin) { // no snooping for anon GMs
					SendEmoteMessage(
						gmg->myname,
						0,
						AccountStatus::Player,
						Chat::Red,
						fmt::format(
							"Error: {} not found.",
							gmg->gotoname
						).c_str()
					);
				} else {
					cle->Server()->SendPacket(pack);
				}
			} else {
				SendEmoteMessage(
					gmg->myname,
					0,
					AccountStatus::Player,
					Chat::Red,
					fmt::format(
						"Error: {} not found",
						gmg->gotoname
					).c_str()
				);
			}

			break;
		}
		case ServerOP_Lock: {
			if (pack->size != sizeof(ServerLock_Struct)) {
				LogInfo("Wrong size on ServerOP_Lock. Got: [{}], Expected: [{}]", pack->size, sizeof(ServerLock_Struct));
				break;
			}

			auto slock = (ServerLock_Struct*) pack->pBuffer;
			if (slock->mode >= 1) {
				WorldConfig::LockWorld();
			} else {
				WorldConfig::UnlockWorld();
			}

			if (loginserverlist.Connected()) {
				loginserverlist.SendStatus();
				SendEmoteMessage(
					slock->myname,
					0,
					AccountStatus::Player,
					Chat::Red,
					fmt::format(
						"World {}.",
						slock->mode ? "locked" : "unlocked"
					).c_str()
				);
			} else {
				SendEmoteMessage(
					slock->myname,
					0,
					AccountStatus::Player,
					Chat::Red,
					fmt::format(
						"World {}, but login server not connected.",
						slock->mode ? "locked" : "unlocked"
					).c_str()
				);
			}

			break;
		}
		case ServerOP_Motd: {
			if (pack->size != sizeof(ServerMotd_Struct)) {
				LogInfo("Wrong size on ServerOP_Motd. Got: [{}], Expected: [{}]", pack->size, sizeof(ServerMotd_Struct));
				break;
			}

			auto smotd = (ServerMotd_Struct*) pack->pBuffer;
			database.SetVariable("MOTD", smotd->motd);
			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_Uptime: {
			if (pack->size != sizeof(ServerUptime_Struct)) {
				LogInfo("Wrong size on ServerOP_Uptime. Got: [{}], Expected: [{}]", pack->size, sizeof(ServerUptime_Struct));
				break;
			}

			auto sus = (ServerUptime_Struct*) pack->pBuffer;
			if (!sus->zoneserverid) {
				ZSList::ShowUpTime(this, sus->adminname);
			} else {
				auto zs = zoneserver_list.FindByID(sus->zoneserverid);
				if (zs) {
					zs->SendPacket(pack);
				}
			}

			break;
		}
		case ServerOP_GetWorldTime: {
			LogInfo("Broadcasting a world time update");
			auto pack = new ServerPacket;

			pack->opcode = ServerOP_SyncWorldTime;
			pack->size = sizeof(eqTimeOfDay);
			pack->pBuffer = new uchar[pack->size];
			memset(pack->pBuffer, 0, pack->size);
			auto tod = (eqTimeOfDay*) pack->pBuffer;
			tod->start_eqtime = zoneserver_list.worldclock.getStartEQTime();
			tod->start_realtime = zoneserver_list.worldclock.getStartRealTime();
			SendPacket(pack);
			safe_delete(pack);
			break;
		}
		case ServerOP_SetWorldTime: {
			LogNetcode("Received SetWorldTime");
			auto newtime = (eqTimeOfDay*) pack->pBuffer;
			zoneserver_list.worldclock.SetCurrentEQTimeOfDay(newtime->start_eqtime, newtime->start_realtime);
			LogInfo("New time = [{}]-[{}]-[{}] [{}]:[{}] ([{}])\n", newtime->start_eqtime.year, newtime->start_eqtime.month, (int)newtime->start_eqtime.day, (int)newtime->start_eqtime.hour, (int)newtime->start_eqtime.minute, (int)newtime->start_realtime);
			database.SaveTime((int)newtime->start_eqtime.minute, (int)newtime->start_eqtime.hour, (int)newtime->start_eqtime.day, newtime->start_eqtime.month, newtime->start_eqtime.year);
			zoneserver_list.SendTimeSync();
			break;
		}
		case ServerOP_IPLookup: {
			if (pack->size < sizeof(ServerGenericWorldQuery_Struct)) {
				LogInfo("Wrong size on ServerOP_IPLookup. Got: [{}], Expected (at least): [{}]", pack->size, sizeof(ServerGenericWorldQuery_Struct));
				break;
			}

			auto sgwq = (ServerGenericWorldQuery_Struct*) pack->pBuffer;
			if (pack->size == sizeof(ServerGenericWorldQuery_Struct)) {
				client_list.SendCLEList(sgwq->admin, sgwq->from, this);
			} else {
				client_list.SendCLEList(sgwq->admin, sgwq->from, this, sgwq->query);
			}

			break;
		}
		case ServerOP_LockZone: {
			if (pack->size < sizeof(ServerLockZone_Struct)) {
				LogInfo("Wrong size on ServerOP_LockZone. Got: [{}], Expected: [{}]", pack->size, sizeof(ServerLockZone_Struct));
				break;
			}

			auto lock_zone = (ServerLockZone_Struct*) pack->pBuffer;
			if (lock_zone->op == ServerLockType::List) {
				zoneserver_list.ListLockedZones(lock_zone->adminname, this);
				break;
			} else if (
				lock_zone->op == ServerLockType::Lock ||
				lock_zone->op == ServerLockType::Unlock
			) {
				if (zoneserver_list.SetLockedZone(lock_zone->zoneID, lock_zone->op == ServerLockType::Lock)) {
					zoneserver_list.SendEmoteMessage(
						0,
						0,
						AccountStatus::QuestTroupe,
						Chat::White,
						fmt::format(
							"Zone {} | Name: {} ({}) ID: {}",
							lock_zone->op == ServerLockType::Lock ? "Locked" : "Unlocked",
							ZoneLongName(lock_zone->zoneID),
							ZoneName(lock_zone->zoneID),
							lock_zone->zoneID
						).c_str()
					);
				} else {
					SendEmoteMessageRaw(
						lock_zone->adminname,
						0,
						AccountStatus::Player,
						Chat::White,
						fmt::format(
							"Zone Failed to {} | Name: {} ({}) ID: {}",
							lock_zone->op == ServerLockType::Lock ? "Lock" : "Unlock",
							ZoneLongName(lock_zone->zoneID),
							ZoneName(lock_zone->zoneID),
							lock_zone->zoneID
						).c_str()
					);
				}
				break;
			}

			break;
		}
		case ServerOP_Revoke: {
			auto rev = (RevokeStruct*) pack->pBuffer;
			auto cle = client_list.FindCharacter(rev->name);
			if (cle && cle->Server()) {
				cle->Server()->SendPacket(pack);
			}

			break;
		}
		case ServerOP_SpawnPlayerCorpse: {
			auto s = (SpawnPlayerCorpse_Struct*) pack->pBuffer;
			auto zs = zoneserver_list.FindByZoneID(s->zone_id);
			if (zs) {
				zs->SendPacket(pack);
			}

			break;
		}
		case ServerOP_Consent_Response: {
			auto s = (ServerOP_Consent_Struct*) pack->pBuffer;

			auto owner_zs = (
				s->instance_id ?
				zoneserver_list.FindByInstanceID(s->instance_id) :
				zoneserver_list.FindByZoneID(s->zone_id)
			);

			if (owner_zs) {
				owner_zs->SendPacket(pack);
			} else {
				LogInfo("Unable to locate zone record for zone id [{}] or instance id [{}] in zoneserver list for ServerOP_Consent_Response operation", s->zone_id, s->instance_id);
			}

			if (s->consent_type == EQ::consent::Normal) {
				auto cle = client_list.FindCharacter(s->grantname);
				if (cle) {
					auto granted_zs = (
						cle->instance() ?
						zoneserver_list.FindByInstanceID(cle->instance()) :
						zoneserver_list.FindByZoneID(cle->zone())
					);

					if (granted_zs && granted_zs != owner_zs) {
						granted_zs->SendPacket(pack);
					}
				}
			}

			break;
		}
		case ServerOP_InstanceUpdateTime: {
			auto iut = (ServerInstanceUpdateTime_Struct*) pack->pBuffer;
			auto zm = zoneserver_list.FindByInstanceID(iut->instance_id);
			if (zm) {
				zm->SendPacket(pack);
			}

			break;
		}
		case ServerOP_QGlobalUpdate: {
			if (pack->size != sizeof(ServerQGlobalUpdate_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_QGlobalDelete: {
			if (pack->size != sizeof(ServerQGlobalDelete_Struct)) {
				break;
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_AdventureRequest: {
			adventure_manager.CalculateAdventureRequestReply((const char*) pack->pBuffer);
			break;
		}
		case ServerOP_AdventureRequestCreate: {
			adventure_manager.TryAdventureCreate((const char*) pack->pBuffer);
			break;
		}
		case ServerOP_AdventureDataRequest: {
			AdventureFinishEvent fe;
			while (adventure_manager.PopFinishedEvent((const char*) pack->pBuffer, fe)) {
				adventure_manager.SendAdventureFinish(fe);
			}
			adventure_manager.GetAdventureData((const char*) pack->pBuffer);
			break;
		}
		case ServerOP_AdventureClickDoor: {
			auto pcad = (ServerPlayerClickedAdventureDoor_Struct*) pack->pBuffer;
			adventure_manager.PlayerClickedDoor(pcad->player, pcad->zone_id, pcad->id);
			break;
		}
		case ServerOP_AdventureLeave: {
			adventure_manager.LeaveAdventure((const char*) pack->pBuffer);
			break;
		}
		case ServerOP_AdventureCountUpdate: {
			auto sc = (ServerAdventureCount_Struct*) pack->pBuffer;
			adventure_manager.IncrementCount(sc->instance_id);
			break;
		}
		case ServerOP_AdventureAssaCountUpdate: {
			adventure_manager.IncrementAssassinationCount(*((uint16*) pack->pBuffer));
			break;
		}
		case ServerOP_AdventureZoneData: {
			adventure_manager.GetZoneData(*((uint16*) pack->pBuffer));
			break;
		}
		case ServerOP_AdventureLeaderboard: {
			auto lr = (ServerLeaderboardRequest_Struct*) pack->pBuffer;
			adventure_manager.DoLeaderboardRequest(lr->player, lr->type);
			break;
		}
		case ServerOP_LSAccountUpdate: {
			LogNetcode("Received ServerOP_LSAccountUpdate packet from zone");
			loginserverlist.SendAccountUpdate(pack);
			break;
		}
		case ServerOP_UCSMailMessage: {
			UCSLink.SendPacket(pack);
			break;
		}
		case ServerOP_UCSServerStatusRequest: {
			auto ucsss = (UCSServerStatus_Struct*) pack->pBuffer;
			auto zs = zoneserver_list.FindByPort(ucsss->port);
			if (!zs) {
				break;
			}

			auto outapp = new ServerPacket(ServerOP_UCSServerStatusReply, sizeof(UCSServerStatus_Struct));
			ucsss = (UCSServerStatus_Struct*)outapp->pBuffer;
			ucsss->available = (UCSServerAvailable_ ? 1 : 0);
			ucsss->timestamp = Timer::GetCurrentTime();
			zs->SendPacket(outapp);
			safe_delete(outapp);
			break;
		}
		case ServerOP_Speech:
		case ServerOP_QSSendQuery:
		case ServerOP_QSPlayerLogDeletes:
		case ServerOP_QSPlayerDropItem:
		case ServerOP_QSPlayerLogHandins:
		case ServerOP_QSPlayerLogMerchantTransactions:
		case ServerOP_QSPlayerLogMoves:
		case ServerOP_QSPlayerLogNPCKills:
		case ServerOP_QSPlayerLogTrades:
		case ServerOP_QueryServGeneric: {
			QSLink.SendPacket(pack);
			break;
		}
		case ServerOP_CZDialogueWindow:
		case ServerOP_CZLDoNUpdate:
		case ServerOP_CZMarquee:
		case ServerOP_CZMessage:
		case ServerOP_CZMove:
		case ServerOP_CZSetEntityVariable:
		case ServerOP_CZSignal:
		case ServerOP_CZSpell:
		case ServerOP_CZTaskUpdate:
		case ServerOP_CameraShake:
		case ServerOP_Consent:
		case ServerOP_DepopAllPlayersCorpses:
		case ServerOP_DepopPlayerCorpse:
		case ServerOP_ExpeditionLockState:
		case ServerOP_ExpeditionLockout:
		case ServerOP_ExpeditionLockoutDuration:
		case ServerOP_ExpeditionReplayOnJoin:
		case ServerOP_GuildRankUpdate:
		case ServerOP_ItemStatus:
		case ServerOP_KickPlayer:
		case ServerOP_KillPlayer:
		case ServerOP_OOZGroupMessage:
		case ServerOP_Petition:
		case ServerOP_RaidGroupSay:
		case ServerOP_RaidSay:
		case ServerOP_RefreshCensorship:
		case ServerOP_ReloadAAData:
		case ServerOP_ReloadAlternateCurrencies:
		case ServerOP_ReloadBlockedSpells:
		case ServerOP_ReloadDoors:
		case ServerOP_ReloadGroundSpawns:
		case ServerOP_ReloadLevelEXPMods:
		case ServerOP_ReloadMerchants:
		case ServerOP_ReloadNPCEmotes:
		case ServerOP_ReloadObjects:
		case ServerOP_ReloadPerlExportSettings:
		case ServerOP_ReloadRules:
		case ServerOP_ReloadStaticZoneData:
		case ServerOP_ReloadTitles:
		case ServerOP_ReloadTraps:
		case ServerOP_ReloadVariables:
		case ServerOP_ReloadVeteranRewards:
		case ServerOP_ReloadWorld:
		case ServerOP_ReloadZonePoints:
		case ServerOP_RezzPlayerAccept:
		case ServerOP_SpawnStatusChange:
		case ServerOP_UpdateSpawn:
		case ServerOP_WWDialogueWindow:
		case ServerOP_WWLDoNUpdate:
		case ServerOP_WWMarquee:
		case ServerOP_WWMessage:
		case ServerOP_WWMove:
		case ServerOP_WWSetEntityVariable:
		case ServerOP_WWSignal:
		case ServerOP_WWSpell:
		case ServerOP_WWTaskUpdate:
		case ServerOP_ZonePlayer: {
			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_ReloadContentFlags: {
			zoneserver_list.SendPacket(pack);
			content_service.SetExpansionContext()->ReloadContentFlags();
			break;
		}	
		case ServerOP_ReloadLogs: {
			zoneserver_list.SendPacket(pack);
			LogSys.LoadLogDatabaseSettings();
			break;
		}
		case ServerOP_ReloadTasks: {
			shared_task_manager.LoadTaskData();
			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_ChangeSharedMem: {
			auto hotfix_name = std::string((char*) pack->pBuffer);

			LogInfo("Loading items");
			if (!database.LoadItems(hotfix_name)) {
				LogInfo("Error: Could not load item data. But ignoring");
			}

			LogInfo("Loading skill caps");
			if (!content_db.LoadSkillCaps(hotfix_name)) {
				LogInfo("Error: Could not load skill cap data. But ignoring");
			}

			zoneserver_list.SendPacket(pack);
			break;
		}
		case ServerOP_RequestTellQueue: {
			auto rtq = (ServerRequestTellQueue_Struct*) pack->pBuffer;
			auto cle = client_list.FindCharacter(rtq->name);
			if (!cle || cle->TellQueueEmpty()) {
				break;
			}

			cle->ProcessTellQueue();
			break;
		}
		case ServerOP_CZClientMessageString: {
			auto buf = reinterpret_cast<CZClientMessageString_Struct*>(pack->pBuffer);
			client_list.SendPacket(buf->client_name, pack);
			break;
		}
		case ServerOP_SharedTaskRequest:
		case ServerOP_SharedTaskAddPlayer:
		case ServerOP_SharedTaskAttemptRemove:
		case ServerOP_SharedTaskUpdate:
		case ServerOP_SharedTaskRequestMemberlist:
		case ServerOP_SharedTaskRemovePlayer:
		case ServerOP_SharedTaskInviteAcceptedPlayer:
		case ServerOP_SharedTaskMakeLeader:
		case ServerOP_SharedTaskCreateDynamicZone:
		case ServerOP_SharedTaskPurgeAllCommand:
		case ServerOP_SharedTaskPlayerList:
		case ServerOP_SharedTaskKickPlayers: {
			SharedTaskWorldMessaging::HandleZoneMessage(pack);
			break;
		}
		case ServerOP_ExpeditionCreate:
		case ServerOP_ExpeditionDzAddPlayer:
		case ServerOP_ExpeditionDzMakeLeader:
		case ServerOP_ExpeditionCharacterLockout:
		case ServerOP_ExpeditionSaveInvite:
		case ServerOP_ExpeditionRequestInvite: {
			ExpeditionMessage::HandleZoneMessage(pack);
			break;
		}
		case ServerOP_DzCreated:
		case ServerOP_DzAddRemoveMember:
		case ServerOP_DzSwapMembers:
		case ServerOP_DzRemoveAllMembers:
		case ServerOP_DzGetMemberStatuses:
		case ServerOP_DzSetSecondsRemaining:
		case ServerOP_DzSetCompass:
		case ServerOP_DzSetSafeReturn:
		case ServerOP_DzSetZoneIn:
		case ServerOP_DzUpdateMemberStatus: {
			DynamicZone::HandleZoneMessage(pack);
			break;
		}
		default: {
			LogInfo("Unknown ServerOPcode from zone {:#04x}, size [{}]", pack->opcode, pack->size);
			DumpPacket(pack->pBuffer, pack->size);
			break;
		}
	}
}

void ZoneServer::SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...) {
	if (!message) {
		return;
	}

	va_list argptr;
	char buffer[1024];

	va_start(argptr, message);
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	va_end(argptr);
	SendEmoteMessageRaw(
		to,
		to_guilddbid,
		to_minstatus,
		type,
		buffer
	);
}

void ZoneServer::SendEmoteMessageRaw(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message) {
	if (!message) {
		return;
	}

	auto pack = new ServerPacket;

	pack->opcode = ServerOP_EmoteMessage;
	pack->size = sizeof(ServerEmoteMessage_Struct) + strlen(message) + 1;
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	auto sem = (ServerEmoteMessage_Struct*) pack->pBuffer;

	if (to != 0) {
		strcpy((char *)sem->to, to);
	} else {
		sem->to[0] = 0;
	}

	sem->guilddbid = to_guilddbid;
	sem->minstatus = to_minstatus;
	sem->type = type;
	strcpy(&sem->message[0], message);

	SendPacket(pack);
	delete pack;
}

void ZoneServer::SendGroupIDs() {
	auto pack = new ServerPacket(ServerOP_GroupIDReply, sizeof(ServerGroupIDReply_Struct));
	auto sgi = (ServerGroupIDReply_Struct*) pack->pBuffer;
	zoneserver_list.NextGroupIDs(sgi->start, sgi->end);
	SendPacket(pack);
	delete pack;
}


void ZoneServer::SendKeepAlive()
{
	ServerPacket pack(ServerOP_KeepAlive, 0);
	SendPacket(&pack);
}

void ZoneServer::ChangeWID(uint32 iCharID, uint32 iWID) {
	auto pack = new ServerPacket(ServerOP_ChangeWID, sizeof(ServerChangeWID_Struct));
	auto scw = (ServerChangeWID_Struct*) pack->pBuffer;
	scw->charid = iCharID;
	scw->newwid = iWID;
	zoneserver_list.SendPacket(pack);
	delete pack;
}


void ZoneServer::TriggerBootup(uint32 in_zone_id, uint32 in_instance_id, const char* admin_name, bool is_static_zone) {
	is_booting_up = true;
	zone_server_zone_id = in_zone_id;
	instance_id = in_instance_id;

	auto pack = new ServerPacket(ServerOP_ZoneBootup, sizeof(ServerZoneStateChange_struct));
	auto s = (ServerZoneStateChange_struct*) pack->pBuffer;
	s->ZoneServerID = zone_server_id;
	if (admin_name) {
		strn0cpy(s->adminname, admin_name, sizeof(s->adminname));
	}

	s->zoneid = in_zone_id ? in_zone_id : GetZoneID();
	s->instanceid = in_instance_id;
	s->makestatic = is_static_zone;
	SendPacket(pack);
	delete pack;
	LSBootUpdate(in_zone_id, in_instance_id);
}

void ZoneServer::IncomingClient(Client* client) {
	is_booting_up = true;
	auto pack = new ServerPacket(ServerOP_ZoneIncClient, sizeof(ServerZoneIncomingClient_Struct));
	auto s = (ServerZoneIncomingClient_Struct*) pack->pBuffer;
	s->zoneid = GetZoneID();
	s->instanceid = GetInstanceID();
	s->wid = client->GetWID();
	s->ip = client->GetIP();
	s->accid = client->GetAccountID();
	s->admin = client->GetAdmin();
	s->charid = client->GetCharID();
	s->lsid = client->GetLSID();

	if (client->GetCLE()) {
		s->tellsoff = client->GetCLE()->TellsOff();
	}

	strn0cpy(s->charname, client->GetCharName(), sizeof(s->charname));
	strn0cpy(s->lskey, client->GetLSKey(), sizeof(s->lskey));
	SendPacket(pack);
	delete pack;
}
