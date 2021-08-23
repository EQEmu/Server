#include "shared_task_world_messaging.h"
#include "cliententry.h"
#include "worlddb.h"
#include "../common/shared_tasks.h"
#include "../common/eqemu_logsys.h"
#include "../common/repositories/tasks_repository.h"
#include "../common/tasks.h"
#include "cliententry.h"
#include "clientlist.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "shared_task_manager.h"
#include "../common/repositories/shared_task_members_repository.h"
#include "../common/repositories/task_activities_repository.h"
#include "dynamic_zone.h"

extern ClientList        client_list;
extern ZSList            zoneserver_list;
extern SharedTaskManager shared_task_manager;

void SharedTaskWorldMessaging::HandleZoneMessage(ServerPacket *pack)
{
	switch (pack->opcode) {
		case ServerOP_SharedTaskRequest: {
			auto *r = (ServerSharedTaskRequest_Struct *) pack->pBuffer;
			LogTasksDetail(
				"[ServerOP_SharedTaskRequest] Received request from character [{}] task_id [{}] npc_type_id [{}]",
				r->requested_character_id,
				r->requested_task_id,
				r->requested_npc_type_id
			);

			shared_task_manager.AttemptSharedTaskCreation(
				r->requested_task_id,
				r->requested_character_id,
				r->requested_npc_type_id
			);

			break;
		}
		case ServerOP_SharedTaskAttemptRemove: {
			auto *r = (ServerSharedTaskAttemptRemove_Struct *) pack->pBuffer;
			LogTasksDetail(
				"[ServerOP_SharedTaskAttemptRemove] Received request from character [{}] task_id [{}] remove_from_db [{}]",
				r->requested_character_id,
				r->requested_task_id,
				r->remove_from_db
			);

			shared_task_manager.AttemptSharedTaskRemoval(
				r->requested_task_id,
				r->requested_character_id,
				r->remove_from_db
			);

			break;
		}
		case ServerOP_SharedTaskKickPlayers: {
			auto r = reinterpret_cast<ServerSharedTaskKickPlayers_Struct *>(pack->pBuffer);
			LogTasksDetail(
				"[ServerOP_SharedTaskKickPlayers] Received request from character [{}] task_id [{}]",
				r->source_character_id,
				r->task_id
			);

			auto t = shared_task_manager.FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					client_list.SendCharacterMessageID(
						r->source_character_id, Chat::Red,
						SharedTaskMessage::YOU_ARE_NOT_LEADER_COMMAND_ISSUE, {leader.character_name}
					);
				}
				else {
					shared_task_manager.RemoveEveryoneFromSharedTask(t, r->source_character_id);
				}
			}

			break;
		}
		case ServerOP_SharedTaskUpdate: {
			auto *r = (ServerSharedTaskActivityUpdate_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskUpdate] Received request from character [{}] task_id [{}] activity_id [{}] donecount [{}] ignore_quest_update [{}]",
				r->source_character_id,
				r->task_id,
				r->activity_id,
				r->done_count,
				(r->ignore_quest_update ? "true" : "false")
			);

			shared_task_manager.SharedTaskActivityUpdate(
				r->source_character_id,
				r->task_id,
				r->activity_id,
				r->done_count,
				r->ignore_quest_update
			);

			break;
		}
		case ServerOP_SharedTaskRequestMemberlist: {
			auto *r = (ServerSharedTaskRequestMemberlist_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskRequestMemberlist] Received request from character [{}] task_id [{}]",
				r->source_character_id,
				r->task_id
			);

			auto t = shared_task_manager.FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskRequestMemberlist] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				shared_task_manager.SendSharedTaskMemberList(
					r->source_character_id,
					t->GetMembers()
				);
			}

			break;
		}
		case ServerOP_SharedTaskRemovePlayer: {
			auto *r = (ServerSharedTaskRemovePlayer_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskRemovePlayer] Received request from character [{}] task_id [{}] player_name [{}]",
				r->source_character_id,
				r->task_id,
				r->player_name
			);

			auto t = shared_task_manager.FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskRemovePlayer] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					client_list.SendCharacterMessageID(
						r->source_character_id, Chat::Red,
						SharedTaskMessage::YOU_ARE_NOT_LEADER_COMMAND_ISSUE, {leader.character_name}
					);
				}
				else {
					LogTasksDetail(
						"[ServerOP_SharedTaskRemovePlayer] character_id [{}] shared_task_id [{}] is_leader",
						r->source_character_id,
						t->GetDbSharedTask().id
					);

					std::string character_name = r->player_name;
					shared_task_manager.RemovePlayerFromSharedTaskByPlayerName(t, character_name);
				}
			}

			break;
		}
		case ServerOP_SharedTaskMakeLeader: {
			auto *r = (ServerSharedTaskMakeLeader_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskMakeLeader] Received request from character [{}] task_id [{}] player_name [{}]",
				r->source_character_id,
				r->task_id,
				r->player_name
			);

			auto t = shared_task_manager.FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskMakeLeader] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					client_list.SendCharacterMessageID(
						r->source_character_id, Chat::Red,
						SharedTaskMessage::YOU_ARE_NOT_LEADER_COMMAND_ISSUE, {leader.character_name}
					);
				}
				else if (strcasecmp(leader.character_name.c_str(), r->player_name) == 0) {
					client_list.SendCharacterMessageID(
						r->source_character_id,
						Chat::Red,
						SharedTaskMessage::YOU_ALREADY_LEADER
					);
				}
				else {
					LogTasksDetail(
						"[ServerOP_SharedTaskMakeLeader] character_id [{}] shared_task_id [{}] is_leader",
						r->source_character_id,
						t->GetDbSharedTask().id
					);

					std::string character_name = r->player_name;
					shared_task_manager.MakeLeaderByPlayerName(t, character_name);
				}
			}

			break;
		}
		case ServerOP_SharedTaskAddPlayer: {
			auto *r = (ServerSharedTaskAddPlayer_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskAddPlayer] Received request from character [{}] task_id [{}] player_name [{}]",
				r->source_character_id,
				r->task_id,
				r->player_name
			);

			auto t = shared_task_manager.FindSharedTaskByTaskIdAndCharacterId(r->task_id, r->source_character_id);
			if (t) {
				LogTasksDetail(
					"[ServerOP_SharedTaskAddPlayer] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				auto leader = t->GetLeader();
				if (leader.character_id != r->source_character_id) {
					// taskadd is client sided with System color in newer clients, server side might still be red
					client_list.SendCharacterMessageID(
						r->source_character_id, Chat::Red,
						SharedTaskMessage::YOU_ARE_NOT_LEADER_COMMAND_ISSUE, {leader.character_name}
					);
				}
				else {
					LogTasksDetail(
						"[ServerOP_SharedTaskAddPlayer] character_id [{}] shared_task_id [{}] is_leader",
						r->source_character_id,
						t->GetDbSharedTask().id
					);

					std::string character_name = r->player_name;
					shared_task_manager.InvitePlayerByPlayerName(t, character_name);
				}
			}

			break;
		}
		case ServerOP_SharedTaskInviteAcceptedPlayer: {
			auto *r = (ServerSharedTaskInviteAccepted_Struct *) pack->pBuffer;

			LogTasksDetail(
				"[ServerOP_SharedTaskInviteAcceptedPlayer] Received request from source_character_id [{}] shared_task_id [{}] accepted [{}]",
				r->source_character_id,
				r->shared_task_id,
				r->accepted
			);

			auto t = shared_task_manager.FindSharedTaskById(r->shared_task_id);
			if (t && shared_task_manager.IsInvitationActive(r->shared_task_id, r->source_character_id)) {
				LogTasksDetail(
					"[ServerOP_SharedTaskInviteAcceptedPlayer] Found shared task character [{}] shared_task_id [{}]",
					r->source_character_id,
					t->GetDbSharedTask().id
				);

				shared_task_manager.RemoveActiveInvitation(r->shared_task_id, r->source_character_id);

				if (r->accepted) {
					shared_task_manager.AddPlayerByCharacterIdAndName(t, r->source_character_id, r->player_name);
				}
				else {
					shared_task_manager.SendLeaderMessageID(
						t,
						Chat::Red,
						SharedTaskMessage::PLAYER_DECLINED_OFFER,
						{r->player_name}
					);
				}
			}
			break;
		}
		case ServerOP_SharedTaskCreateDynamicZone: {
			auto buf = reinterpret_cast<ServerSharedTaskCreateDynamicZone_Struct *>(pack->pBuffer);

			LogTasksDetail(
				"[ServerOP_SharedTaskCreateDynamicZone] Received request from source_character_id [{}] task_id [{}]",
				buf->source_character_id,
				buf->task_id
			);

			auto t = shared_task_manager.FindSharedTaskByTaskIdAndCharacterId(buf->task_id, buf->source_character_id);
			if (t) {
				DynamicZone dz;
				dz.LoadSerializedDzPacket(buf->cereal_data, buf->cereal_size);

				shared_task_manager.CreateDynamicZone(t, dz);
			}
			break;
		}
		case ServerOP_SharedTaskPurgeAllCommand: {
			LogTasksDetail("[ServerOP_SharedTaskPurgeAllCommand] Received request to purge all shared tasks");

			shared_task_manager.PurgeAllSharedTasks();
			auto p = std::make_unique<ServerPacket>(
				ServerOP_SharedTaskPurgeAllCommand,
				0
			);

			zoneserver_list.SendPacket(p.get());

			break;
		}
		case ServerOP_SharedTaskPlayerList: {
			auto buf = reinterpret_cast<ServerSharedTaskPlayerList_Struct *>(pack->pBuffer);

			LogTasksDetail(
				"[ServerOP_SharedTaskPlayerList] Received request from source_character_id [{}] task_id [{}]",
				buf->source_character_id,
				buf->task_id
			);

			auto s = shared_task_manager.FindSharedTaskByTaskIdAndCharacterId(buf->task_id, buf->source_character_id);
			if (s) {
				std::vector<std::string> player_names;

				for (const auto &member : s->GetMembers()) {
					player_names.emplace_back(member.character_name);

					if (member.is_leader) {
						client_list.SendCharacterMessageID(
							buf->source_character_id, Chat::Yellow,
							SharedTaskMessage::LEADER_PRINT, {member.character_name}
						);
					}
				}

				std::string player_list = fmt::format("{}", fmt::join(player_names, ", "));
				client_list.SendCharacterMessageID(
					buf->source_character_id, Chat::Yellow,
					SharedTaskMessage::MEMBERS_PRINT, {player_list}
				);
			}

			break;
		}
		default:
			break;
	}
}
