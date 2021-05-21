#include "shared_task_manager.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/repositories/task_activities_repository.h"
#include "cliententry.h"
#include "clientlist.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "shared_task_world_messaging.h"
#include "../common/repositories/shared_tasks_repository.h"
#include "../common/repositories/shared_task_members_repository.h"
#include "../common/repositories/shared_task_activity_state_repository.h"

extern ClientList client_list;
extern ZSList     zoneserver_list;

SharedTaskManager *SharedTaskManager::SetDatabase(Database *db)
{
	SharedTaskManager::m_database = db;

	return this;
}

SharedTaskManager *SharedTaskManager::SetContentDatabase(Database *db)
{
	SharedTaskManager::m_content_database = db;

	return this;
}

std::vector<SharedTaskMember> SharedTaskManager::GetRequestMembers(uint32 requestor_character_id)
{
	std::vector<SharedTaskMember> request_members = {};

	// raid
	auto raid_characters = CharacterDataRepository::GetWhere(
		*m_database,
		fmt::format(
			"id IN (select charid from raid_members where raidid = (select raidid from raid_members where charid = {}))",
			requestor_character_id
		)
	);

	if (!raid_characters.empty()) {
		request_members.reserve(raid_characters.size());
		for (auto &c: raid_characters) {
			SharedTaskMember member = {};
			member.character_id   = c.id;
			member.character_name = c.name;
			member.is_raided      = true;
			member.level          = c.level;

			request_members.emplace_back(member);
		}

		return request_members;
	}

	// group
	auto group_characters = CharacterDataRepository::GetWhere(
		*m_database,
		fmt::format(
			"id IN (select charid from group_id where groupid = (select groupid from group_id where charid = {}))",
			requestor_character_id
		)
	);

	if (!group_characters.empty()) {
		request_members.reserve(request_members.size());
		for (auto &c: group_characters) {
			SharedTaskMember member = {};
			member.character_id   = c.id;
			member.character_name = c.name;
			member.is_grouped     = true;
			member.level          = c.level;

			request_members.emplace_back(member);
		}
	}

	// if we didn't pull the requested character from the db, let's pull it by now
	// most shared tasks are not going to be single character / solo for us to typically get here
	// maybe we're a GM testing a shared task solo
	bool      list_has_leader = false;
	for (auto &m: request_members) {
		if (m.character_id == requestor_character_id) {
			list_has_leader = true;
		}
	}

	if (!list_has_leader) {
		auto leader = CharacterDataRepository::FindOne(*m_database, requestor_character_id);
		if (leader.id != 0) {
			SharedTaskMember member = {};
			member.character_id   = leader.id;
			member.character_name = leader.name;
			member.level          = leader.level;
			member.is_leader      = true;

			request_members.emplace_back(member);
		}
	}

	return request_members;
}

void SharedTaskManager::AttemptSharedTaskCreation(uint32 requested_task_id, uint32 requested_character_id)
{
	// TODO: Move to memory reference later
	auto task = TasksRepository::FindOne(*m_content_database, requested_task_id);
	if (task.id != 0 && task.type == TASK_TYPE_SHARED) {
		LogTasksDetail(
			"[AttemptSharedTaskCreation] Found Shared Task ({}) [{}]",
			requested_task_id,
			task.title
		);
	}

	auto request_members = GetRequestMembers(requested_character_id);
	if (!request_members.empty()) {
		for (auto &m: request_members) {
			LogTasksDetail(
				"[AttemptSharedTaskCreation] Request Members ({}) [{}] level [{}] grouped [{}] raided [{}]",
				m.character_id,
				m.character_name,
				m.level,
				(m.is_grouped ? "true" : "false"),
				(m.is_raided ? "true" : "false")
			);
		}
	}

	if (request_members.empty()) {
		LogTasksDetail("[AttemptSharedTaskCreation] No additional request members found... Just leader");
	}

	// confirm shared task request
	auto p = std::make_unique<ServerPacket>(
		ServerOP_SharedTaskAcceptNewTask,
		sizeof(ServerSharedTaskRequest_Struct)
	);
	auto d = reinterpret_cast<ServerSharedTaskRequest_Struct *>(p->pBuffer);
	d->requested_character_id = requested_character_id;
	d->requested_task_id      = requested_task_id;

	// get requested character zone server
	ClientListEntry *requested_character_cle = client_list.FindCLEByCharacterID(d->requested_character_id);
	if (requested_character_cle && requested_character_cle->Server()) {
		requested_character_cle->Server()->SendPacket(p.get());
	}

	// TODO: Additional validation logic here

	// new shared task instance
	auto new_shared_task = SharedTask{};
	auto activities      = TaskActivitiesRepository::GetWhere(*m_content_database, fmt::format("taskid = {}", task.id));

	// new shared task db object
	auto shared_task_entity = SharedTasksRepository::NewEntity();
	shared_task_entity.task_id       = requested_task_id;
	shared_task_entity.accepted_time = std::time(nullptr);

	auto created_db_shared_task = SharedTasksRepository::InsertOne(*m_database, shared_task_entity);

	// something failed here, bail out
	if (created_db_shared_task.id == 0) {
		// TODO: Handle failure, happy path for now
	}

	// active record
	new_shared_task.m_db_shared_task = created_db_shared_task;

	// persist members
	std::vector<SharedTaskMembersRepository::SharedTaskMembers> shared_task_db_members = {};
	shared_task_db_members.reserve(request_members.size());
	for (auto &m: request_members) {
		auto e = SharedTaskMembersRepository::NewEntity();

		e.character_id   = m.character_id;
		e.is_leader      = (m.is_leader ? 1 : 0);
		e.shared_task_id = new_shared_task.m_db_shared_task.id;

		shared_task_db_members.emplace_back(e);
	}

	SharedTaskMembersRepository::InsertMany(*m_database, shared_task_db_members);

	// activity state (memory)
	std::vector<SharedTaskActivityStateEntry> shared_task_activity_state = {};
	shared_task_activity_state.reserve(activities.size());
	for (auto &a: activities) {

		// entry
		auto e = SharedTaskActivityStateEntry{};
		e.activity_id    = a.activityid;
		e.done_count     = 0;
		e.max_done_count = a.goalcount;

		shared_task_activity_state.emplace_back(e);
	}

	// activity state (database)
	std::vector<SharedTaskActivityStateRepository::SharedTaskActivityState> shared_task_db_activities = {};
	shared_task_db_activities.reserve(activities.size());
	for (auto &a: activities) {

		// entry
		auto e = SharedTaskActivityStateRepository::NewEntity();
		e.shared_task_id = new_shared_task.m_db_shared_task.id;
		e.activity_id    = a.activityid;
		e.done_count     = 0;

		shared_task_db_activities.emplace_back(e);
	}

	SharedTaskActivityStateRepository::InsertMany(*m_database, shared_task_db_activities);

	// state
	new_shared_task.SetSharedTaskActivityState(shared_task_activity_state);

	// set database data in memory to make it easier for any later referencing
	new_shared_task.SetTaskData(task);
	new_shared_task.SetTaskActivityData(activities);
	new_shared_task.SetMembers(request_members);

	// add to shared tasks list
	m_shared_tasks.emplace_back(new_shared_task);

	LogTasks(
		"[AttemptSharedTaskCreation] shared_task_id [{}] created successfully | task_id [{}] member_count [{}] activity_count [{}] current tasks in state [{}]",
		new_shared_task.m_db_shared_task.id,
		task.id,
		request_members.size(),
		shared_task_activity_state.size(),
		m_shared_tasks.size()
	);
}

void SharedTaskManager::AttemptSharedTaskRemoval(uint32 requested_task_id, uint32 requested_character_id)
{
	// TODO: Move to memory reference later
	auto task = TasksRepository::FindOne(*m_content_database, requested_task_id);
	if (task.id != 0 && task.type == TASK_TYPE_SHARED) {
		LogTasksDetail(
			"[AttemptSharedTaskRemoval] Found Shared Task ({}) [{}]",
			requested_task_id,
			task.title
		);
	}

	// check for active shared tasks
	for (auto &t: m_shared_tasks) {
		LogTasksDetail(
			"[AttemptSharedTaskRemoval] Looping tasks | task_id [{}] shared_task_id [{}]",
			t.GetTaskData().id,
			t.m_db_shared_task.id
		);

		if (t.GetTaskData().id == requested_task_id) {
			// get members from shared task
			for (auto &m: t.GetMembers()) {
				LogTasksDetail(
					"[AttemptSharedTaskRemoval] Looping members | character_id [{}]",
					m.character_id
				);

				// TODO: Happy path removal just for now, add additional handling later
				if (m.character_id == requested_character_id && m.is_leader) {
					DeleteSharedTask(t.m_db_shared_task.id);
				}
			}
		}
	}
}

void SharedTaskManager::DeleteSharedTask(int64 shared_task_id)
{
	LogTasksDetail("[DeleteSharedTask] shared_task_id [{}]", shared_task_id);

	// remove internally
	m_shared_tasks.erase(
		std::remove_if(
			m_shared_tasks.begin(),
			m_shared_tasks.end(),
			[&](SharedTask const &s) {
				return s.m_db_shared_task.id == shared_task_id;
			}
		),
		m_shared_tasks.end());

	// database
	SharedTasksRepository::DeleteWhere(*m_database, fmt::format("id = {}", shared_task_id));
	SharedTaskMembersRepository::DeleteWhere(*m_database, fmt::format("shared_task_id = {}", shared_task_id));
	SharedTaskActivityStateRepository::DeleteWhere(*m_database, fmt::format("shared_task_id = {}", shared_task_id));
}

void SharedTaskManager::LoadSharedTaskState()
{
	LogTasksDetail("[LoadSharedTaskState] Restoring state from the database");

	// do stuff


}
