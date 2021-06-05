#include "shared_task_zone_messaging.h"
#include "../common/shared_tasks.h"
#include "../common/servertalk.h"
#include "client.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/repositories/shared_task_members_repository.h"

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
			auto p = reinterpret_cast<ServerSharedTaskRequest_Struct *>(pack->pBuffer);
			auto c = entity_list.GetClientByCharID(p->requested_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskAcceptNewTask] We're back in zone and I found [{}]", c->GetCleanName());

				c->m_requesting_shared_task = true;
				c->GetTaskState()->AcceptNewTask(c, (int) p->requested_task_id, 0);
				c->LoadClientTaskState();
				c->m_requesting_shared_task = false;
			}

			break;
		}
		case ServerOP_SharedTaskUpdate: {
			auto p = reinterpret_cast<ServerSharedTaskActivityUpdate_Struct *>(pack->pBuffer);
			auto c = entity_list.GetClientByCharID(p->source_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskUpdate] We're back in zone and I found [{}]", c->GetCleanName());

				c->m_shared_task_update = true;
				c->GetTaskState()->SharedTaskIncrementDoneCount(
					c,
					(int) p->task_id,
					(int) p->activity_id,
					(int) p->done_count,
					p->ignore_quest_update
				);
				c->m_shared_task_update = false;
			}

			break;
		}
		case ServerOP_SharedTaskAttemptRemove: {
			auto p = reinterpret_cast<ServerSharedTaskAttemptRemove_Struct *>(pack->pBuffer);
			auto c = entity_list.GetClientByCharID(p->requested_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskAttemptRemove] We're back in zone and I found [{}]", c->GetCleanName());

				c->m_requested_shared_task_removal = true;
				c->GetTaskState()->CancelTask(
					c,
					TASKSLOTSHAREDTASK,
					static_cast<TaskType>((int) TASK_TYPE_SHARED),
					p->remove_from_db
				);
				c->m_requested_shared_task_removal = false;
			}

			break;
		}
		case ServerOP_SharedTaskMemberlist: {
			auto p = reinterpret_cast<ServerSharedTaskMemberListPacket_Struct *>(pack->pBuffer);

			LogTasks(
				"[ServerOP_SharedTaskMemberlist] We're back in zone and I'm searching for [{}]",
				p->destination_character_id,
				sizeof(pack->pBuffer)
			);


			// temp hack until we make this better
			auto characters = CharacterDataRepository::GetWhere(
				database,
				fmt::format(
					"id IN (select character_id from shared_task_members where shared_task_id = {})",
					p->shared_task_id
				)
			);

			auto members = SharedTaskMembersRepository::GetWhere(
				database,
				fmt::format("shared_task_id = {}", p->shared_task_id)
			);

			SerializeBuffer buf(sizeof(SharedTaskMemberList_Struct) + 15 * members.size());
			buf.WriteInt32(0); // unknown ids
			buf.WriteInt32(0);
			buf.WriteInt32((int32) members.size());

			for (auto &c : characters) {
				buf.WriteString(c.name);
				buf.WriteInt32(0); // monster mission

				bool      is_leader = false;
				for (auto &m: members) {
					if (m.character_id == c.id && m.is_leader) {
						is_leader = true;
					}
				}

				buf.WriteInt8((int8) (is_leader ? 1 : 0));
			}

			// find character and route packet
			auto c = entity_list.GetClientByCharID(p->destination_character_id);
			if (c) {
				LogTasks("[ServerOP_SharedTaskMemberlist] We're back in zone and I found [{}]", c->GetCleanName());

				auto outapp = new EQApplicationPacket(OP_SharedTaskMemberList, buf);
				c->QueuePacket(outapp);
				safe_delete(outapp);
			}

			break;
		}
		default:
			break;
	}

}
