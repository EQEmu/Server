#include "shared_task_zone_messaging.h"
#include "../common/shared_tasks.h"
#include "../common/servertalk.h"
#include "client.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

void SharedTaskZoneMessaging::HandleWorldMessage(ServerPacket *pack)
{
// ServerOP_SharedTaskAcceptNewTask
	switch (pack->opcode) {
		case ServerOP_SharedTaskAcceptNewTask: {
			auto buf              = reinterpret_cast<ServerSharedTaskRequest_Struct *>(pack->pBuffer);
			auto requested_client = entity_list.GetClientByCharID(buf->requested_character_id);
			if (requested_client) {
				LogTasks("We're back in zone and I found [{}]", requested_client->GetCleanName());

				requested_client->m_requesting_shared_task = true;
				requested_client->GetTaskState()->AcceptNewTask(requested_client, buf->requested_task_id, 0);
				requested_client->m_requesting_shared_task = false;
			}

		}
		default:
			break;
	}

}
