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

extern ClientList client_list;
extern ZSList     zoneserver_list;

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

			auto task = TasksRepository::FindOne(content_db, r->requested_task_id);
			if (task.id != 0 && task.type == TASK_TYPE_SHARED) {
				LogTasksDetail(
					"[ServerOP_SharedTaskRequest] Found Shared Task ({}) [{}]",
					r->requested_task_id,
					task.title
				);
			}

			auto request_members = SharedTask::GetRequestMembers(database, r->requested_character_id);
			if (!request_members.empty()) {
				for (auto &member: request_members) {
					LogTasksDetail(
						"[ServerOP_SharedTaskRequest] Request Members ({}) [{}] level [{}] grouped [{}] raided [{}]",
						member.character_id,
						member.character_name,
						member.level,
						(member.is_grouped ? "true" : "false"),
						(member.is_raided ? "true" : "false")
					);
				}
			}

			if (request_members.empty()) {
				LogTasksDetail("[ServerOP_SharedTaskRequest] No additional request members found... Just leader");
			}

			// confirm shared task request
			auto p   = std::make_unique<ServerPacket>(ServerOP_SharedTaskAcceptNewTask, sizeof(ServerSharedTaskRequest_Struct));
			auto buf = reinterpret_cast<ServerSharedTaskRequest_Struct *>(p->pBuffer);
			buf->requested_character_id = r->requested_character_id;
			buf->requested_task_id      = r->requested_task_id;

			// get requested character zone server
			ClientListEntry *requested_character_cle = client_list.FindCLEByCharacterID(buf->requested_character_id);
			if (requested_character_cle && requested_character_cle->Server()) {
				requested_character_cle->Server()->SendPacket(p.get());
			}

		}
		default:
			break;
	}
}

