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
		}
		case ServerOP_SharedTaskAttemptRemove: {
			auto *r = (ServerSharedTaskRequest_Struct *) pack->pBuffer;
			LogTasksDetail(
				"[ServerOP_SharedTaskAttemptRemove] Received request from character [{}] task_id [{}]",
				r->requested_character_id,
				r->requested_task_id
			);

			shared_task_manager.AttemptSharedTaskRemoval(r->requested_task_id, r->requested_character_id);
		}
		default:
			break;
	}
}

