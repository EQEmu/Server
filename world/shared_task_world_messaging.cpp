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
#include "../common/repositories/task_activities_repository.h"

extern ClientList        client_list;
extern ZSList            zoneserver_list;
extern SharedTaskManager shared_task_manager;

void SharedTaskWorldMessaging::HandleZoneMessage(ServerPacket *pack)
{
	switch (pack->opcode) {
		case ServerOP_SharedTaskRequest: {
			auto *r = (ServerSharedTaskRequest_Struct *) pack->pBuffer;
			LogTasksDetail(
				"[ServerOP_SharedTaskRequest] Received request from character [{}] task_id [{}]",
				r->requested_character_id,
				r->requested_task_id
			);

			shared_task_manager.AttemptSharedTaskCreation(r->requested_task_id, r->requested_character_id);

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
					t->GetDbSharedTask().id
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

				if (shared_task_manager.IsSharedTaskLeader(t, r->source_character_id)) {
					LogTasksDetail(
						"[ServerOP_SharedTaskRemovePlayer] character_id [{}] shared_task_id [{}] is_leader",
						r->source_character_id,
						t->GetDbSharedTask().id
					);

					// TODO: Clean this up a bit more later as other functionality is A-Z'ed
					std::string character_name = r->player_name;
					shared_task_manager.RemovePlayerFromSharedTaskByPlayerName(t, character_name);
				}
			}

			break;
		}
		default:
			break;
	}
}

