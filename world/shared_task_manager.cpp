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
#include "../common/serialize_buffer.h"

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

			// if the group member is a leader, make sure we tag it as such
			if (c.id == requestor_character_id) {
				member.is_leader = true;
			}

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

			// if the group member is a leader, make sure we tag it as such
			if (c.id == requestor_character_id) {
				member.is_leader = true;
			}

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

	// TODO: Additional validation logic here

	// new shared task instance
	auto new_shared_task = SharedTask{};
	auto activities      = TaskActivitiesRepository::GetWhere(*m_content_database, fmt::format("taskid = {}", task.id));

	// new shared task db object
	auto shared_task_entity = SharedTasksRepository::NewEntity();
	shared_task_entity.task_id       = (int) requested_task_id;
	shared_task_entity.accepted_time = std::time(nullptr);

	auto created_db_shared_task = SharedTasksRepository::InsertOne(*m_database, shared_task_entity);

	// something failed here, bail out
	if (created_db_shared_task.id == 0) {
		// TODO: Handle failure, happy path for now
	}

	// active record
	new_shared_task.SetDbSharedTask(created_db_shared_task);

	// persist members
	std::vector<SharedTaskMembersRepository::SharedTaskMembers> shared_task_db_members = {};
	shared_task_db_members.reserve(request_members.size());
	for (auto &m: request_members) {
		auto e = SharedTaskMembersRepository::NewEntity();

		e.character_id   = m.character_id;
		e.is_leader      = (m.is_leader ? 1 : 0);
		e.shared_task_id = new_shared_task.GetDbSharedTask().id;

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
		e.shared_task_id = new_shared_task.GetDbSharedTask().id;
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

	// send accept to members
	for (auto &m: request_members) {
		SendAcceptNewSharedTaskPacket(m.character_id, requested_task_id);
		SendSharedTaskMemberList(m.character_id, new_shared_task.GetDbSharedTask().id);
	}

	LogTasks(
		"[AttemptSharedTaskCreation] shared_task_id [{}] created successfully | task_id [{}] member_count [{}] activity_count [{}] current tasks in state [{}]",
		new_shared_task.GetDbSharedTask().id,
		task.id,
		request_members.size(),
		shared_task_activity_state.size(),
		m_shared_tasks.size()
	);
}

void SharedTaskManager::AttemptSharedTaskRemoval(
	uint32 requested_task_id,
	uint32 requested_character_id,
	bool remove_from_db // inherited from zone logic - we're just passing through
)
{
	auto task = GetSharedTaskDataByTaskId(requested_task_id);
	if (task.id != 0 && task.type == TASK_TYPE_SHARED) {
		LogTasksDetail(
			"[AttemptSharedTaskRemoval] Found Shared Task data ({}) [{}]",
			requested_task_id,
			task.title
		);
	}

	auto t = FindSharedTaskByTaskIdAndCharacterId(requested_task_id, requested_character_id);
	if (t) {

		// make sure the one requesting is leader before removing everyone and deleting the shared task
		if (IsSharedTaskLeader(t, requested_character_id)) {
			LogTasksDetail(
				"[AttemptSharedTaskRemoval] Leader [{}]",
				requested_character_id
			);

			// inform clients of removal
			for (auto &m: t->GetMembers()) {
				LogTasksDetail(
					"[AttemptSharedTaskRemoval] Sending removal to [{}] shared_task_id [{}]",
					m.character_id,
					requested_task_id
				);

				SendRemovePlayerFromSharedTaskPacket(
					m.character_id,
					requested_task_id,
					remove_from_db
				);
			}

			// persistence
			DeleteSharedTask(t->GetDbSharedTask().id, requested_character_id);

			PrintSharedTaskState();

			return;
		}

		// non-leader
		// remove self
		RemovePlayerFromSharedTask(t, requested_character_id);
		SendRemovePlayerFromSharedTaskPacket(
			requested_character_id,
			requested_task_id,
			remove_from_db
		);

		// inform clients of removal of self
		for (auto &m: t->GetMembers()) {
			LogTasksDetail("[AttemptSharedTaskRemoval] looping character_id [{}]", m.character_id);

			SendSharedTaskMemberList(m.character_id, t->GetDbSharedTask().id);
		}
	}
}

void SharedTaskManager::DeleteSharedTask(int64 shared_task_id, uint32 requested_character_id)
{
	LogTasksDetail(
		"[DeleteSharedTask] shared_task_id [{}]",
		shared_task_id
	);

	// remove internally
	m_shared_tasks.erase(
		std::remove_if(
			m_shared_tasks.begin(),
			m_shared_tasks.end(),
			[&](SharedTask const &s) {
				return s.GetDbSharedTask().id == shared_task_id;
			}
		),
		m_shared_tasks.end()
	);

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

		LogTasksDetail(
			"[LoadSharedTaskState] Loading shared_task_id [{}] task_id [{}]",
			s.id,
			s.task_id
		);

		// shared task db data
		ns.SetDbSharedTask(s);

		// set database task data for internal referencing
		auto task_data = GetSharedTaskDataByTaskId(s.task_id);

		LogTasksDetail("[LoadSharedTaskState] [GetSharedTaskDataByTaskId] task_id [{}]", task_data.id);

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
							"[LoadSharedTaskState] shared_task_id [{}] task_id [{}] activity_id [{}] done_count [{}] max_done_count (goalcount) [{}]",
							s.id,
							s.task_id,
							sta.activity_id,
							e.done_count,
							ad.goalcount
						);

						e.max_done_count = ad.goalcount;
						e.completed_time = sta.completed_time;
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

				LogTasksDetail(
					"[LoadSharedTaskState] shared_task_id [{}] adding member character_id [{}] is_leader [{}]",
					s.id,
					member.character_id,
					member.is_leader
				);
			}
		}

		ns.SetMembers(shared_task_members);

		LogTasks(
			"[LoadSharedTaskState] Loaded shared task state | shared_task_id [{}] task_id [{}] task_title [{}] member_count [{}] state_activity_count [{}]",
			s.id,
			task_data.id,
			task_data.title,
			ns.GetMembers().size(),
			ns.GetActivityState().size()
		);

		shared_tasks.emplace_back(ns);
	}

	m_shared_tasks = shared_tasks;

	LogTasks(
		"[LoadSharedTaskState] Loaded [{}] shared tasks",
		m_shared_tasks.size()
	);

	PrintSharedTaskState();
}

SharedTaskManager *SharedTaskManager::LoadTaskData()
{
	m_task_data          = TasksRepository::All(*m_content_database);
	m_task_activity_data = TaskActivitiesRepository::All(*m_content_database);

	LogTasks("[LoadTaskData] Loaded tasks [{}] activities [{}]", m_task_data.size(), m_task_activity_data.size());

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
			shared_task->GetDbSharedTask().id,
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
						shared_task->GetDbSharedTask().id,
						source_character_id,
						task_id,
						activity_id,
						a.done_count,
						done_count
					);

					a.done_count = done_count;

					// if the activity is done, lets mark it as such
					if (a.done_count == a.max_done_count) {
						a.completed_time = std::time(nullptr);
					}

					// sync state as each update comes in (for now)
					SaveSharedTaskActivityState(
						shared_task->GetDbSharedTask().id,
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
					shared_task->GetDbSharedTask().id,
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

SharedTask *SharedTaskManager::FindSharedTaskByTaskIdAndCharacterId(uint32 task_id, uint32 character_id)
{
//	LogTasksDetail("[FindSharedTaskByTaskIdAndCharacterId] pre task_id [{}] character_id [{}]", task_id, character_id);

	for (auto &s: m_shared_tasks) {
//		LogTasksDetail("[FindSharedTaskByTaskIdAndCharacterId] task_id [{}] character_id [{}]", task_id, character_id);
		// grep for task
		if (s.GetTaskData().id == task_id) {
//			LogTasksDetail("[FindSharedTaskByTaskIdAndCharacterId] -- task_id [{}] character_id [{}]", task_id, character_id);
			// find member in shared task
			for (auto &m: s.GetMembers()) {
//				LogTasksDetail("[FindSharedTaskByTaskIdAndCharacterId] -- m -- m.character_id [{}] task_id [{}] character_id [{}]", m.character_id, task_id, character_id);
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
		e.activity_id    = (int) a.activity_id;
		e.done_count     = (int) a.done_count;
		e.completed_time = (int) a.completed_time;

		shared_task_db_activities.emplace_back(e);
	}

	SharedTaskActivityStateRepository::DeleteWhere(*m_database, fmt::format("shared_task_id = {}", shared_task_id));
	SharedTaskActivityStateRepository::InsertMany(*m_database, shared_task_db_activities);
}

bool SharedTaskManager::IsSharedTaskLeader(SharedTask *s, uint32 character_id)
{
	for (auto &m: s->GetMembers()) {
		if (m.character_id == character_id && m.is_leader) {
			return true;
		}
	}

	return false;
}

void SharedTaskManager::SendAcceptNewSharedTaskPacket(uint32 character_id, uint32 task_id)
{
	auto p = std::make_unique<ServerPacket>(
		ServerOP_SharedTaskAcceptNewTask,
		sizeof(ServerSharedTaskRequest_Struct)
	);

	auto d = reinterpret_cast<ServerSharedTaskRequest_Struct *>(p->pBuffer);
	d->requested_character_id = character_id;
	d->requested_task_id      = task_id;

	// get requested character zone server
	ClientListEntry *cle = client_list.FindCLEByCharacterID(character_id);
	if (cle && cle->Server()) {
		cle->Server()->SendPacket(p.get());
	}
}

void SharedTaskManager::SendRemovePlayerFromSharedTaskPacket(
	uint32 character_id,
	uint32 task_id,
	bool remove_from_db
)
{
	// confirm shared task request: inform clients
	auto p = std::make_unique<ServerPacket>(
		ServerOP_SharedTaskAttemptRemove,
		sizeof(ServerSharedTaskAttemptRemove_Struct)
	);

	auto d = reinterpret_cast<ServerSharedTaskAttemptRemove_Struct *>(p->pBuffer);
	d->requested_character_id = character_id;
	d->requested_task_id      = task_id;
	d->remove_from_db         = remove_from_db;

	// get requested character zone server
	ClientListEntry *cle = client_list.FindCLEByCharacterID(character_id);
	if (cle && cle->Server()) {
		cle->Server()->SendPacket(p.get());
	}
}

void SharedTaskManager::SendSharedTaskMemberList(uint32 character_id, int64 shared_task_id)
{
	// send member list packet
	// TODO: move this to serialized list sent over the wire
	auto p = std::make_unique<ServerPacket>(
		ServerOP_SharedTaskMemberlist,
		sizeof(ServerSharedTaskMemberListPacket_Struct)
	);

	auto d = reinterpret_cast<ServerSharedTaskMemberListPacket_Struct *>(p->pBuffer);
	d->destination_character_id = character_id;
	d->shared_task_id           = shared_task_id;

	// send memberlist
	ClientListEntry *cle = client_list.FindCLEByCharacterID(character_id);
	if (cle && cle->Server()) {
		cle->Server()->SendPacket(p.get());
	}
}

void SharedTaskManager::RemovePlayerFromSharedTask(SharedTask *s, uint32 character_id)
{
	SharedTaskMembersRepository::DeleteWhere(
		*m_database,
		fmt::format(
			"shared_task_id = {} and character_id = {}",
			s->GetDbSharedTask().id,
			character_id
		)
	);

	// remove internally
	s->m_members.erase(
		std::remove_if(
			s->m_members.begin(),
			s->m_members.end(),
			[&](SharedTaskMember const &m) {
				return m.character_id == character_id;
			}
		),
		s->m_members.end()
	);
}

void SharedTaskManager::PrintSharedTaskState()
{
	for (auto &s: m_shared_tasks) {
		auto task = GetSharedTaskDataByTaskId(s.GetDbSharedTask().task_id);

		LogTasksDetail("[PrintSharedTaskState] # Shared Task");

		LogTasksDetail(
			"[PrintSharedTaskState] shared_task_id [{}] task_id [{}] task_title [{}] member_count [{}] state_activity_count [{}]",
			s.GetDbSharedTask().id,
			task.id,
			task.title,
			s.GetMembers().size(),
			s.GetActivityState().size()
		);

		LogTasksDetail("[PrintSharedTaskState] # Activities");

		// activity state
		for (auto &a: s.m_shared_task_activity_state) {
			LogTasksDetail(
				"[PrintSharedTaskState] -- activity_id [{}] done_count [{}] max_done_count [{}] completed_time [{}]",
				a.activity_id,
				a.done_count,
				a.max_done_count,
				a.completed_time
			);
		}

		LogTasksDetail("[PrintSharedTaskState] # Members");

		// members
		for (auto &m: s.m_members) {
			LogTasksDetail(
				"[PrintSharedTaskState] -- character_id [{}] is_leader [{}]",
				m.character_id,
				m.is_leader
			);
		}
	}
}

void SharedTaskManager::RemovePlayerFromSharedTaskByPlayerName(SharedTask *s, const std::string &character_name)
{
	auto character = CharacterDataRepository::GetWhere(
		*m_database,
		fmt::format("`name` = '{}' LIMIT 1", EscapeString(character_name))
	);

	if (!character.empty()) {
		int64 character_id = character[0].id;

		for (auto &m: s->GetMembers()) {
			LogTasksDetail(
				"[RemovePlayerFromSharedTaskByPlayerName] character_id [{}] m.character_name [{}]",
				character_id,
				m.character_id
			);

			// not leader being removed
			if (character_id == m.character_id) {
				LogTasksDetail(
					"[RemovePlayerFromSharedTaskByPlayerName] shared_task_id [{}] character_name [{}]",
					s->GetDbSharedTask().id,
					character_name
				);

				RemovePlayerFromSharedTask(s, m.character_id);
				SendRemovePlayerFromSharedTaskPacket(
					m.character_id,
					s->GetDbSharedTask().task_id,
					true
				);
			}
		}

		SendSharedTaskMemberListToAllMembers(s);
	}
}

void SharedTaskManager::SendSharedTaskMemberListToAllMembers(SharedTask *s)
{
	for (auto &m: s->GetMembers()) {
		SendSharedTaskMemberList(
			m.character_id,
			s->GetDbSharedTask().id
		);
	}
}

void SharedTaskManager::MakeLeaderByPlayerName(SharedTask *s, const std::string &character_name)
{
	auto character = CharacterDataRepository::GetWhere(
		*m_database,
		fmt::format("`name` = '{}' LIMIT 1", EscapeString(character_name))
	);

	if (!character.empty()) {
		int64 character_id     = character[0].id;
		bool  found_new_leader = false;

		std::vector<SharedTaskMember> members = s->GetMembers();
		for (auto                     &m: members) {
			LogTasksDetail(
				"[MakeLeaderByPlayerName] character_id [{}] m.character_name [{}]",
				character_id,
				m.character_id
			);

			m.is_leader = false;

			// destination character is in shared task, make swap
			if (character_id == m.character_id) {
				found_new_leader = true;
				LogTasksDetail(
					"[MakeLeaderByPlayerName] shared_task_id [{}] character_name [{}]",
					s->GetDbSharedTask().id,
					character_name
				);

				m.is_leader = true;
			}
		}

		if (found_new_leader) {
			s->SetMembers(members);
			SaveMembers(s, members);
			SendSharedTaskMemberListToAllMembers(s);
		}
	}
}

void SharedTaskManager::SaveMembers(SharedTask *s, std::vector<SharedTaskMember> members)
{
	std::vector<SharedTaskMembersRepository::SharedTaskMembers> dm = {};
	dm.reserve(members.size());
	for (auto &m: members) {
		auto e = SharedTaskMembersRepository::NewEntity();

		e.character_id   = m.character_id;
		e.is_leader      = (m.is_leader ? 1 : 0);
		e.shared_task_id = s->GetDbSharedTask().id;

		dm.emplace_back(e);
	}

	SharedTaskMembersRepository::DeleteWhere(*m_database, fmt::format("shared_task_id = {}", s->GetDbSharedTask().id));
	SharedTaskMembersRepository::InsertMany(*m_database, dm);
}

