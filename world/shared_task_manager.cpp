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
	bool list_has_leader = false;

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
	auto task = GetSharedTaskDataByTaskId(requested_task_id);
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
	auto task = GetSharedTaskDataByTaskId(requested_task_id);
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

	// [x] preload task data and activity data
	// [x] load shared tasks themselves
	// [x] load shared task activities
	// [x] load shared task members

	// load shared tasks
	std::vector<SharedTask> shared_tasks = {};

	// eager load all activity state data
	auto shared_tasks_activity_state_data = SharedTaskActivityStateRepository::All(*m_database);

	// eager load all member state data
	auto shared_task_members_data = SharedTaskMembersRepository::All(*m_database);

	// load shared tasks not already completed
	auto st = SharedTasksRepository::GetWhere(*m_database, "completion_time = 0");
	shared_tasks.reserve(st.size());
	for (auto &s: st) {
		SharedTask ns = {};

		// shared task db data
		ns.m_db_shared_task = s;

		// set database task data for internal referencing
		auto task_data = GetSharedTaskDataByTaskId(s.task_id);
		ns.SetTaskData(task_data);

		// set database task data for internal referencing
		auto activities_data = GetSharedTaskActivityDataByTaskId(s.task_id);
		ns.SetTaskActivityData(activities_data);

		// load activity state into memory
		std::vector<SharedTaskActivityStateEntry> shared_task_activity_state = {};

		// loop through shared task activity state data referencing from memory instead of
		// querying inside this loop each time
		for (auto &sta: shared_tasks_activity_state_data) {

			// filter by current shared task id
			if (sta.shared_task_id == s.id) {

				auto e = SharedTaskActivityStateEntry{};
				e.activity_id = sta.activity_id;
				e.done_count  = sta.done_count;

				// get max done count from activities data
				// loop through activities data in memory and grep on task_id, activity_id to pull goalcount
				for (auto &ad: activities_data) {
					if (ad.taskid == s.task_id && ad.activityid == sta.activity_id) {
						LogTasksDetail(
							"[LoadSharedTaskState] Task activity loop | found activity_id [{}] max_done_count [{}]",
							sta.activity_id,
							ad.goalcount
						);

						e.max_done_count = ad.goalcount;
					}
				}

				shared_task_activity_state.emplace_back(e);
			}
		}

		ns.SetSharedTaskActivityState(shared_task_activity_state);

		// members
		std::vector<SharedTaskMember> shared_task_members = {};
		for (auto                     &m: shared_task_members_data) {
			if (m.shared_task_id == s.id) {
				SharedTaskMember member = {};
				member.character_id = m.character_id;
				member.is_leader    = (m.is_leader ? 1 : 0);

				shared_task_members.emplace_back(member);
			}
		}

		ns.SetMembers(shared_task_members);

		LogTasksDetail(
			"Loaded shared task state | shared_task_id [{}] task_id [{}] task_title [{}] member_count [{}] state_activity_count [{}]",
			s.id,
			task_data.id,
			task_data.title,
			ns.GetMembers().size(),
			ns.GetActivityState().size()
		);

		shared_tasks.emplace_back(ns);
	}

	m_shared_tasks = shared_tasks;

}
SharedTaskManager *SharedTaskManager::LoadTaskData()
{
	m_task_data          = TasksRepository::All(*m_content_database);
	m_task_activity_data = TaskActivitiesRepository::All(*m_content_database);

	LogTasks("Loaded tasks [{}] activities [{}]", m_task_data.size(), m_task_activity_data.size());

	return this;
}

TasksRepository::Tasks SharedTaskManager::GetSharedTaskDataByTaskId(uint32 task_id)
{
	for (auto &t: m_task_data) {
		if (t.id == task_id && t.type == TASK_TYPE_SHARED) {
			return t;
		}
	}

	return TasksRepository::NewEntity();
}

std::vector<TaskActivitiesRepository::TaskActivities>
SharedTaskManager::GetSharedTaskActivityDataByTaskId(uint32 task_id)
{
	std::vector<TaskActivitiesRepository::TaskActivities> activities = {};

	for (auto &a: m_task_activity_data) {
		if (a.taskid == task_id) {
			activities.emplace_back(a);
		}
	}

	return activities;
}

void SharedTaskManager::SharedTaskActivityUpdate(
	uint32 source_character_id,
	uint32 task_id,
	uint32 activity_id,
	uint32 done_count,
	bool ignore_quest_update
)
{
	auto shared_task = FindSharedTaskByTaskIdAndCharacterId(task_id, source_character_id);
	if (shared_task) {
		LogTasksDetail(
			"[SharedTaskActivityUpdate] shared_task_id [{}] character_id [{}] task_id [{}] activity_id [{}] done_count [{}]",
			shared_task->m_db_shared_task.id,
			source_character_id,
			task_id,
			activity_id,
			done_count
		);

		for (auto &a : shared_task->m_shared_task_activity_state) {
			if (a.activity_id == activity_id) {
				if (a.done_count < done_count) {
					LogTasksDetail(
						"[SharedTaskActivityUpdate] Propagating update for shared_task_id [{}] character_id [{}] task_id [{}] activity_id [{}] old_done_count [{}] new_done_count [{}]",
						shared_task->m_db_shared_task.id,
						source_character_id,
						task_id,
						activity_id,
						a.done_count,
						done_count
					);

					a.done_count = done_count;

					// sync state as each update comes in (for now)
					SaveSharedTaskActivityState(
						shared_task->m_db_shared_task.id,
						shared_task->m_shared_task_activity_state
					);

					shared_task->SetSharedTaskActivityState(shared_task->m_shared_task_activity_state);

					LogTasksDetail(
						"[SharedTaskActivityUpdate] Debug done_count [{}]",
						a.done_count
					);

					// TODO: Check for cap / max here

					// loop through members - send update
					for (auto &m: shared_task->GetMembers()) {

						// confirm task update to client(s)
						auto p = std::make_unique<ServerPacket>(
							ServerOP_SharedTaskUpdate,
							sizeof(ServerSharedTaskActivityUpdate_Struct)
						);

						auto d = reinterpret_cast<ServerSharedTaskActivityUpdate_Struct *>(p->pBuffer);
						d->source_character_id = m.character_id;
						d->task_id             = task_id;
						d->activity_id         = activity_id;
						d->done_count          = done_count;
						d->ignore_quest_update = ignore_quest_update;

						// get requested character zone server
						ClientListEntry *c = client_list.FindCLEByCharacterID(m.character_id);
						if (c && c->Server()) {
							c->Server()->SendPacket(p.get());
						}
					}

					break;
				}

				LogTasksDetail(
					"[SharedTaskActivityUpdate] Discarding duplicate update for shared_task_id [{}] character_id [{}] task_id [{}] activity_id [{}] done_count [{}] ignore_quest_update [{}]",
					shared_task->m_db_shared_task.id,
					source_character_id,
					task_id,
					activity_id,
					done_count,
					(ignore_quest_update ? "true" : "false")
				);
			}
		}
	}
}

SharedTask * SharedTaskManager::FindSharedTaskByTaskIdAndCharacterId(uint32 task_id, uint32 character_id)
{
	for (auto &s: m_shared_tasks) {

		// grep for task
		if (s.GetTaskData().id == task_id) {

			// find member in shared task
			for (auto &m: s.GetMembers()) {
				if (m.character_id == character_id) {
					return &s;
				}
			}
		}
	}

	return nullptr;
}

void SharedTaskManager::SaveSharedTaskActivityState(
	int64 shared_task_id,
	std::vector<SharedTaskActivityStateEntry> activity_state
)
{
	// transfer from memory to database
	std::vector<SharedTaskActivityStateRepository::SharedTaskActivityState> shared_task_db_activities = {};
	shared_task_db_activities.reserve(activity_state.size());

	for (auto &a: activity_state) {

		// entry
		auto e = SharedTaskActivityStateRepository::NewEntity();
		e.shared_task_id = shared_task_id;
		e.activity_id    = (int)a.activity_id;
		e.done_count     = (int)a.done_count;

		shared_task_db_activities.emplace_back(e);
	}

	SharedTaskActivityStateRepository::DeleteWhere(*m_database, fmt::format("shared_task_id = {}", shared_task_id));
	SharedTaskActivityStateRepository::InsertMany(*m_database, shared_task_db_activities);
}
