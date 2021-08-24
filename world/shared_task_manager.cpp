#include "shared_task_manager.h"
#include "cliententry.h"
#include "clientlist.h"
#include "dynamic_zone.h"
#include "dynamic_zone_manager.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "shared_task_world_messaging.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/repositories/character_task_timers_repository.h"
#include "../common/repositories/shared_task_members_repository.h"
#include "../common/repositories/shared_task_activity_state_repository.h"
#include "../common/repositories/completed_shared_tasks_repository.h"
#include "../common/repositories/completed_shared_task_members_repository.h"
#include "../common/repositories/completed_shared_task_activity_state_repository.h"
#include "../common/repositories/shared_task_dynamic_zones_repository.h"
#include <ctime>

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

std::vector<SharedTaskMember> SharedTaskManager::GetRequestMembers(
	uint32 requestor_character_id,
	const std::vector<CharacterDataRepository::CharacterData> &characters
)
{
	std::vector<SharedTaskMember> request_members = {};
	request_members.reserve(characters.size());

	for (const auto &character : characters) {
		SharedTaskMember member = {};
		member.character_id   = character.id;
		member.character_name = character.name;

		// if the solo/raid/group member is a leader, make sure we tag it as such
		if (character.id == requestor_character_id) {
			member.is_leader = true;
		}

		request_members.emplace_back(member);
	}

	return request_members;
}

void SharedTaskManager::AttemptSharedTaskCreation(
	uint32 requested_task_id,
	uint32 requested_character_id,
	uint32 npc_type_id
)
{
	auto task = GetSharedTaskDataByTaskId(requested_task_id);
	if (task.id != 0 && task.type == TASK_TYPE_SHARED) {
		LogTasksDetail(
			"[AttemptSharedTaskCreation] Found Shared Task ({}) [{}]",
			requested_task_id,
			task.title
		);
	}

	// shared task validation
	auto request = SharedTask::GetRequestCharacters(*m_database, requested_character_id);
	if (!CanRequestSharedTask(task.id, requested_character_id, request)) {
		LogTasksDetail("[AttemptSharedTaskCreation] Shared task validation failed");
		return;
	}

	auto request_members = GetRequestMembers(requested_character_id, request.characters);
	if (!request_members.empty()) {
		for (auto &m: request_members) {
			LogTasksDetail(
				"[AttemptSharedTaskCreation] Request Members ({})",
				m.character_id
			);
		}
	}

	if (request_members.empty()) {
		LogTasksDetail("[AttemptSharedTaskCreation] No additional request members found... Just leader");
	}

	// new shared task instance
	auto new_shared_task = SharedTask{};
	auto activities      = TaskActivitiesRepository::GetWhere(*m_content_database, fmt::format("taskid = {}", task.id));

	// new shared task db object
	auto shared_task_entity = SharedTasksRepository::NewEntity();
	shared_task_entity.task_id       = (int) requested_task_id;
	shared_task_entity.accepted_time = static_cast<int>(std::time(nullptr));
	shared_task_entity.expire_time   = task.duration > 0 ? (std::time(nullptr) + task.duration) : 0;

	auto created_db_shared_task = SharedTasksRepository::InsertOne(*m_database, shared_task_entity);

	// active record
	new_shared_task.SetDbSharedTask(created_db_shared_task);

	// request timer lockouts
	std::vector<CharacterTaskTimersRepository::CharacterTaskTimers> task_timers;
	task_timers.reserve(request_members.size());

	// persist members
	std::vector<SharedTaskMembersRepository::SharedTaskMembers> shared_task_db_members = {};
	shared_task_db_members.reserve(request_members.size());
	for (auto &m: request_members) {
		auto e = SharedTaskMembersRepository::NewEntity();

		e.character_id   = m.character_id;
		e.is_leader      = (m.is_leader ? 1 : 0);
		e.shared_task_id = new_shared_task.GetDbSharedTask().id;

		shared_task_db_members.emplace_back(e);

		new_shared_task.AddCharacterToMemberHistory(m.character_id); // memory member history

		if (task.request_timer_seconds > 0) {
			auto timer = CharacterTaskTimersRepository::NewEntity();
			timer.character_id = m.character_id;
			timer.task_id      = task.id;
			timer.timer_type   = static_cast<int>(TaskTimerType::Request);
			timer.expire_time  = shared_task_entity.accepted_time + task.request_timer_seconds;

			task_timers.emplace_back(timer);
		}
	}

	SharedTaskMembersRepository::InsertMany(*m_database, shared_task_db_members);

	if (!task_timers.empty()) {
		CharacterTaskTimersRepository::InsertMany(*m_database, task_timers);
	}

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
		// only requester (leader) receives back the npc context to trigger task accept event
		uint32_t npc_context_id = m.character_id == requested_character_id ? npc_type_id : 0;
		SendAcceptNewSharedTaskPacket(
			m.character_id,
			requested_task_id,
			npc_context_id,
			shared_task_entity.accepted_time
		);
	}
	SendSharedTaskMemberListToAllMembers(&new_shared_task);

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
		auto removed = t->FindMemberFromCharacterID(requested_character_id);

		// remove self
		RemovePlayerFromSharedTask(t, requested_character_id);
		SendRemovePlayerFromSharedTaskPacket(
			requested_character_id,
			requested_task_id,
			remove_from_db
		);

		// inform clients of removal of self
		SendSharedTaskMemberRemovedToAllMembers(t, removed.character_name);

		client_list.SendCharacterMessageID(
			requested_character_id, Chat::Yellow,
			SharedTaskMessage::PLAYER_HAS_BEEN_REMOVED, {removed.character_name, task.title}
		);

		if (removed.is_leader) {
			ChooseNewLeader(t);
		}
	}
}

void SharedTaskManager::RemoveEveryoneFromSharedTask(SharedTask *t, uint32 requested_character_id)
{
	// caller validates leader
	LogTasksDetail("[RemoveEveryoneFromSharedTask] Leader [{}]", requested_character_id);

	// inform clients of removal
	for (auto &m: t->GetMembers()) {
		LogTasksDetail(
			"[RemoveEveryoneFromSharedTask] Sending removal to [{}] task_id [{}]",
			m.character_id,
			t->GetTaskData().id
		);

		SendRemovePlayerFromSharedTaskPacket(m.character_id, t->GetTaskData().id, true);

		client_list.SendCharacterMessageID(
			m.character_id, Chat::Yellow,
			SharedTaskMessage::YOU_HAVE_BEEN_REMOVED, {t->GetTaskData().title}
		);
	}

	client_list.SendCharacterMessageID(
		requested_character_id,
		Chat::Red,
		SharedTaskMessage::PLAYER_HAS_BEEN_REMOVED,
		{"Everyone", t->GetTaskData().title}
	);

	RemoveAllMembersFromDynamicZones(t);

	// persistence
	DeleteSharedTask(t->GetDbSharedTask().id);

	PrintSharedTaskState();
}

void SharedTaskManager::DeleteSharedTask(int64 shared_task_id)
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
	SharedTaskDynamicZonesRepository::DeleteWhere(*m_database, fmt::format("shared_task_id = {}", shared_task_id));
}

void SharedTaskManager::LoadSharedTaskState()
{
	LogTasksDetail("[LoadSharedTaskState] Restoring state from the database");

	// load shared tasks
	std::vector<SharedTask> shared_tasks = {};

	// eager load all activity state data
	auto shared_tasks_activity_state_data = SharedTaskActivityStateRepository::All(*m_database);

	// eager load all member state data
	auto shared_task_members_data = SharedTaskMembersRepository::All(*m_database);

	// load character data for member names
	std::vector<CharacterDataRepository::CharacterData> shared_task_character_data;
	if (!shared_task_members_data.empty()) {
		std::vector<uint32_t> character_ids;
		for (const auto       &m: shared_task_members_data) {
			character_ids.emplace_back(m.character_id);
		}

		shared_task_character_data = CharacterDataRepository::GetWhere(
			*m_database,
			fmt::format("id IN ({})", fmt::join(character_ids, ","))
		);
	}

	auto shared_task_dynamic_zones_data = SharedTaskDynamicZonesRepository::All(*m_database);

	// load shared tasks not already completed
	auto st = SharedTasksRepository::GetWhere(*m_database, "TRUE");
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
						e.updated_time   = sta.updated_time;
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

				auto it = std::find_if(
					shared_task_character_data.begin(), shared_task_character_data.end(),
					[&](const CharacterDataRepository::CharacterData &character) {
						return character.id == m.character_id;
					}
				);

				if (it != shared_task_character_data.end()) {
					member.character_name = it->name;
				}

				shared_task_members.emplace_back(member);

				LogTasksDetail(
					"[LoadSharedTaskState] shared_task_id [{}] adding member character_id [{}] character_name [{}] is_leader [{}]",
					s.id,
					member.character_id,
					member.character_name,
					member.is_leader
				);

				// add member to history (if restoring state from a world restart we lost real past member history)
				ns.AddCharacterToMemberHistory(m.character_id);
			}
		}

		ns.SetMembers(shared_task_members);

		// dynamic zones
		for (const auto &dz_entry : shared_task_dynamic_zones_data) {
			if (dz_entry.shared_task_id == s.id) {
				ns.dynamic_zone_ids.emplace_back(static_cast<uint32_t>(dz_entry.dynamic_zone_id));

				LogTasksDetail(
					"[LoadSharedTaskState] shared_task_id [{}] adding dynamic_zone_id [{}]",
					s.id,
					dz_entry.dynamic_zone_id
				);
			}
		}

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

	SetSharedTasks(shared_tasks);

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

				// discard updates out of bounds
				if (a.done_count == a.max_done_count) {
					LogTasksDetail(
						"[SharedTaskActivityUpdate] done_count [{}] is greater than max [{}] discarding...",
						done_count,
						a.max_done_count
					);
					return;
				}

				// if we are progressing
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

					a.done_count   = done_count;
					a.updated_time = std::time(nullptr);

					// if the update came in larger than the max for whatever reason, clamp
					if (a.done_count > a.max_done_count) {
						a.done_count = a.max_done_count;
					}

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

		// check if completed
		bool      is_shared_task_completed = true;
		for (auto &a : shared_task->m_shared_task_activity_state) {
			if (a.done_count != a.max_done_count) {
				is_shared_task_completed = false;
			}
		}

		// mark completed
		if (is_shared_task_completed) {
			auto t = shared_task->GetDbSharedTask();
			if (t.id > 0) {
				LogTasksDetail(
					"[SharedTaskActivityUpdate] Marking shared task [{}] completed",
					shared_task->GetDbSharedTask().id
				);

				// set record
				t.completion_time = std::time(nullptr);
				t.is_locked       = true;
				// update database
				SharedTasksRepository::UpdateOne(*m_database, t);
				// update internally
				shared_task->SetDbSharedTask(t);
				// record completion
				RecordSharedTaskCompletion(shared_task);
				// replay timer lockouts
				AddReplayTimers(shared_task);
			}
		}
	}
}

SharedTask *SharedTaskManager::FindSharedTaskByTaskIdAndCharacterId(uint32 task_id, uint32 character_id)
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
		e.activity_id    = (int) a.activity_id;
		e.done_count     = (int) a.done_count;
		e.completed_time = (int) a.completed_time;
		e.updated_time   = (int) a.updated_time;

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

void SharedTaskManager::SendAcceptNewSharedTaskPacket(
	uint32 character_id,
	uint32 task_id,
	uint32_t npc_context_id,
	int accept_time
)
{
	auto p = std::make_unique<ServerPacket>(
		ServerOP_SharedTaskAcceptNewTask,
		sizeof(ServerSharedTaskRequest_Struct)
	);

	auto d = reinterpret_cast<ServerSharedTaskRequest_Struct *>(p->pBuffer);
	d->requested_character_id = character_id;
	d->requested_task_id      = task_id;
	d->requested_npc_type_id  = npc_context_id;
	d->accept_time            = accept_time;

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

void SharedTaskManager::SendSharedTaskMemberList(uint32 character_id, const std::vector<SharedTaskMember> &members)
{
	EQ::Net::DynamicPacket dyn_pack;
	dyn_pack.PutSerialize(0, members);

	SendSharedTaskMemberList(character_id, dyn_pack);
}

void SharedTaskManager::SendSharedTaskMemberList(uint32 character_id, const EQ::Net::DynamicPacket &serialized_members)
{
	// send member list packet
	auto p = std::make_unique<ServerPacket>(
		ServerOP_SharedTaskMemberlist,
		sizeof(ServerSharedTaskMemberListPacket_Struct) + serialized_members.Length()
	);

	auto d = reinterpret_cast<ServerSharedTaskMemberListPacket_Struct *>(p->pBuffer);
	d->destination_character_id = character_id;
	d->cereal_size              = static_cast<uint32_t>(serialized_members.Length());
	memcpy(d->cereal_serialized_members, serialized_members.Data(), serialized_members.Length());

	// send memberlist
	ClientListEntry *cle = client_list.FindCLEByCharacterID(character_id);
	if (cle && cle->Server()) {
		cle->Server()->SendPacket(p.get());
	}
}

void SharedTaskManager::SendSharedTaskMemberChange(
	uint32 character_id,
	int64 shared_task_id,
	const std::string &player_name,
	bool removed
)
{
	uint32_t size = sizeof(ServerSharedTaskMemberChangePacket_Struct);
	auto     p    = std::make_unique<ServerPacket>(ServerOP_SharedTaskMemberChange, size);

	auto d = reinterpret_cast<ServerSharedTaskMemberChangePacket_Struct *>(p->pBuffer);
	d->destination_character_id = character_id;
	d->shared_task_id           = shared_task_id;
	d->removed                  = removed;
	strn0cpy(d->player_name, player_name.c_str(), sizeof(d->player_name));

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

	for (const auto &dz_id : s->dynamic_zone_ids) {
		auto dz = DynamicZone::FindDynamicZoneByID(dz_id);
		if (dz) {
			dz->RemoveMember(character_id);
		}
	}
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

		LogTasksDetail("[PrintSharedTaskState] # Dynamic Zones");

		for (auto &dz_id: s.dynamic_zone_ids) {
			LogTasksDetail(
				"[PrintSharedTaskState] -- dynamic_zone_id [{}]",
				dz_id
			);
		}
	}
}

void SharedTaskManager::RemovePlayerFromSharedTaskByPlayerName(SharedTask *s, const std::string &character_name)
{
	auto member = s->FindMemberFromCharacterName(character_name);
	if (member.character_id == 0) {
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::IS_NOT_MEMBER, {character_name});
		return;
	}

	LogTasksDetail(
		"[RemovePlayerFromSharedTaskByPlayerName] shared_task_id [{}] character_name [{}]",
		s->GetDbSharedTask().id,
		character_name
	);

	auto leader = s->GetLeader(); // get leader now for msg in case leader is one removed

	RemovePlayerFromSharedTask(s, member.character_id);
	SendRemovePlayerFromSharedTaskPacket(
		member.character_id,
		s->GetDbSharedTask().task_id,
		true
	);

	SendSharedTaskMemberRemovedToAllMembers(s, member.character_name);

	// leader and removed player get server messages (leader sees two messages)
	// results in double messages if leader removed self (live behavior)
	client_list.SendCharacterMessageID(
		leader.character_id, Chat::Yellow,
		SharedTaskMessage::PLAYER_HAS_BEEN_REMOVED, {member.character_name, s->GetTaskData().title}
	);

	client_list.SendCharacterMessageID(
		member.character_id, Chat::Yellow,
		SharedTaskMessage::PLAYER_HAS_BEEN_REMOVED, {member.character_name, s->GetTaskData().title}
	);

	if (member.is_leader) {
		ChooseNewLeader(s);
	}
}

void SharedTaskManager::SendSharedTaskMemberListToAllMembers(SharedTask *s)
{
	// serialize once so we don't re-serialize it for every member
	EQ::Net::DynamicPacket dyn_pack;
	dyn_pack.PutSerialize(0, s->GetMembers());

	for (auto &m: s->GetMembers()) {
		SendSharedTaskMemberList(
			m.character_id,
			dyn_pack
		);
	}
}

void SharedTaskManager::SendSharedTaskMemberAddedToAllMembers(SharedTask *s, const std::string &player_name)
{
	for (const auto &m : s->GetMembers()) {
		SendSharedTaskMemberChange(m.character_id, s->GetDbSharedTask().id, player_name, false);
	}
}

void SharedTaskManager::SendSharedTaskMemberRemovedToAllMembers(SharedTask *s, const std::string &player_name)
{
	for (const auto &m : s->GetMembers()) {
		SendSharedTaskMemberChange(m.character_id, s->GetDbSharedTask().id, player_name, true);
	}
}

void SharedTaskManager::MakeLeaderByPlayerName(SharedTask *s, const std::string &character_name)
{
	auto new_leader = s->FindMemberFromCharacterName(character_name);
	if (new_leader.character_id == 0) {
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::IS_NOT_MEMBER, {character_name});
		return;
	}

	bool found_new_leader = false;

	std::vector<SharedTaskMember> members = s->GetMembers();
	for (auto                     &m: members) {
		LogTasksDetail(
			"[MakeLeaderByPlayerName] character_id [{}] m.character_id [{}]",
			new_leader.character_id,
			m.character_id
		);

		m.is_leader = false;

		// destination character is in shared task, make swap
		if (new_leader.character_id == m.character_id) {
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
		SendMembersMessageID(
			s, Chat::Yellow, SharedTaskMessage::PLAYER_NOW_LEADER,
			{new_leader.character_name, s->GetTaskData().title}
		);

		for (const auto &dz_id : s->dynamic_zone_ids) {
			auto dz = DynamicZone::FindDynamicZoneByID(dz_id);
			if (dz) {
				dz->SetNewLeader(static_cast<uint32_t>(new_leader.character_id));
			}
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

void SharedTaskManager::InvitePlayerByPlayerName(SharedTask *s, const std::string &player_name)
{
	auto character = CharacterDataRepository::GetWhere(
		*m_database,
		fmt::format("`name` = '{}' LIMIT 1", EscapeString(player_name))
	);

	auto character_id = !character.empty() ? character.front().id : 0;

	// we call validation even for an invalid player so error messages occur
	if (CanAddPlayer(s, character_id, player_name, false)) {
		// send dialogue window
		SendSharedTaskInvitePacket(s, character_id);

		// keep track of active invitations at world
		QueueActiveInvitation(s->GetDbSharedTask().id, character_id);
	}
}

void SharedTaskManager::SendSharedTaskInvitePacket(SharedTask *s, int64 invited_character_id)
{
	auto leader = s->GetLeader();

	// found leader
	if (leader.character_id > 0) {

		// init packet
		auto p = std::make_unique<ServerPacket>(
			ServerOP_SharedTaskInvitePlayer,
			sizeof(ServerSharedTaskInvitePlayer_Struct)
		);

		// fill
		auto d = reinterpret_cast<ServerSharedTaskInvitePlayer_Struct *>(p->pBuffer);
		d->requested_character_id = invited_character_id;
		d->invite_shared_task_id  = s->GetDbSharedTask().id;
		strn0cpy(d->inviter_name, leader.character_name.c_str(), sizeof(d->inviter_name));
		strn0cpy(d->task_name, s->GetTaskData().title.c_str(), sizeof(d->task_name));

		// get requested character zone server
		ClientListEntry *cle = client_list.FindCLEByCharacterID(invited_character_id);
		if (cle && cle->Server()) {
			SendLeaderMessageID(s, Chat::Yellow, SharedTaskMessage::SEND_INVITE_TO, {cle->name()});
			cle->Server()->SendPacket(p.get());
		}
	}
}

void SharedTaskManager::AddPlayerByCharacterIdAndName(
	SharedTask *s,
	int64 character_id,
	const std::string &character_name
)
{
	// fetch
	std::vector<SharedTaskMember> members = s->GetMembers();

	// create
	auto new_member = SharedTaskMember{};
	new_member.character_id   = character_id;
	new_member.character_name = character_name;

	bool does_member_exist = false;

	for (auto &m: s->GetMembers()) {
		if (m.character_id == character_id) {
			does_member_exist = true;
		}
	}

	if (!does_member_exist && CanAddPlayer(s, character_id, character_name, true)) {
		members.push_back(new_member);

		// add request timer (validation will prevent non-expired duplicates)
		if (s->GetTaskData().request_timer_seconds > 0) {
			auto expire_time = s->GetDbSharedTask().accepted_time + s->GetTaskData().request_timer_seconds;
			if (expire_time > std::time(nullptr)) // not already expired
			{
				auto timer = CharacterTaskTimersRepository::NewEntity();
				timer.character_id = character_id;
				timer.task_id      = s->GetDbSharedTask().task_id;
				timer.timer_type   = static_cast<int>(TaskTimerType::Request);
				timer.expire_time  = expire_time;

				CharacterTaskTimersRepository::InsertOne(*m_database, timer);
			}
		}

		// inform client
		SendAcceptNewSharedTaskPacket(character_id, s->GetTaskData().id, 0, s->GetDbSharedTask().accepted_time);

		// add to shared task
		SendSharedTaskMemberAddedToAllMembers(s, character_name);
		s->SetMembers(members);
		SaveMembers(s, members);
		SendSharedTaskMemberList(character_id, s->GetMembers()); // new member gets full member list
		s->AddCharacterToMemberHistory(character_id);

		// add to dzs tied to shared task
		for (const auto &dz_id : s->dynamic_zone_ids) {
			auto dz = DynamicZone::FindDynamicZoneByID(dz_id);
			if (dz) {
				auto status = DynamicZoneMemberStatus::Online;
				dz->AddMember({static_cast<uint32_t>(character_id), character_name, status});
			}
		}
	}
}

SharedTask *SharedTaskManager::FindSharedTaskById(int64 shared_task_id)
{
	for (auto &s: m_shared_tasks) {
		if (s.GetDbSharedTask().id == shared_task_id) {
			return &s;
		}
	}

	return nullptr;
}

void SharedTaskManager::QueueActiveInvitation(int64 shared_task_id, int64 character_id)
{
	LogTasksDetail(
		"[QueueActiveInvitation] shared_task_id [{}] character_id [{}]",
		shared_task_id,
		character_id
	);

	auto active_invitation = SharedTaskActiveInvitation{};
	active_invitation.shared_task_id = shared_task_id;
	active_invitation.character_id   = character_id;

	m_active_invitations.emplace_back(active_invitation);
}

bool SharedTaskManager::IsInvitationActive(uint32 shared_task_id, uint32 character_id)
{
	LogTasksDetail(
		"[IsInvitationActive] shared_task_id [{}] character_id [{}]",
		shared_task_id,
		character_id
	);

	for (auto &i: m_active_invitations) {
		if (i.character_id == character_id && i.shared_task_id == shared_task_id) {
			return true;
		}
	}

	return false;
}

void SharedTaskManager::RemoveActiveInvitation(int64 shared_task_id, int64 character_id)
{
	LogTasksDetail(
		"[RemoveActiveInvitation] shared_task_id [{}] character_id [{}] pre_removal_count [{}]",
		shared_task_id,
		character_id,
		m_active_invitations.size()
	);

	// remove internally
	m_active_invitations.erase(
		std::remove_if(
			m_active_invitations.begin(),
			m_active_invitations.end(),
			[&](SharedTaskActiveInvitation const &i) {
				return i.shared_task_id == shared_task_id && i.character_id == character_id;
			}
		),
		m_active_invitations.end()
	);

	LogTasksDetail(
		"[RemoveActiveInvitation] shared_task_id [{}] character_id [{}] post_removal_count [{}]",
		shared_task_id,
		character_id,
		m_active_invitations.size()
	);
}

void SharedTaskManager::RemoveActiveInvitationByCharacterID(uint32_t character_id)
{
	m_active_invitations.erase(
		std::remove_if(
			m_active_invitations.begin(), m_active_invitations.end(),
			[&](SharedTaskActiveInvitation const &i) {
				return i.character_id == character_id;
			}
		), m_active_invitations.end()
	);
}

void SharedTaskManager::CreateDynamicZone(SharedTask *shared_task, DynamicZone &dz_request)
{
	std::vector<DynamicZoneMember> dz_members;
	for (const auto                &member : shared_task->GetMembers()) {
		dz_members.emplace_back(member.character_id, member.character_name);
		if (member.is_leader) {
			dz_request.SetLeader({member.character_id, member.character_name});
		}
	}

	auto new_dz = dynamic_zone_manager.CreateNew(dz_request, dz_members);
	if (new_dz) {
		auto shared_task_dz = SharedTaskDynamicZonesRepository::NewEntity();
		shared_task_dz.shared_task_id  = shared_task->GetDbSharedTask().id;
		shared_task_dz.dynamic_zone_id = new_dz->GetID();

		SharedTaskDynamicZonesRepository::InsertOne(*m_database, shared_task_dz);

		shared_task->dynamic_zone_ids.emplace_back(new_dz->GetID());
	}
}

void SharedTaskManager::SendLeaderMessage(SharedTask *shared_task, int chat_type, const std::string &message)
{
	if (!shared_task) {
		return;
	}

	for (const auto &member : shared_task->GetMembers()) {
		if (member.is_leader) {
			client_list.SendCharacterMessage(member.character_id, chat_type, message);
			break;
		}
	}
}

void SharedTaskManager::SendLeaderMessageID(
	SharedTask *shared_task, int chat_type,
	int eqstr_id, std::initializer_list<std::string> args
)
{
	if (!shared_task) {
		return;
	}

	for (const auto &member : shared_task->GetMembers()) {
		if (member.is_leader) {
			client_list.SendCharacterMessageID(member.character_id, chat_type, eqstr_id, args);
			break;
		}
	}
}

void SharedTaskManager::SendMembersMessage(SharedTask *shared_task, int chat_type, const std::string &message)
{
	if (!shared_task) {
		return;
	}

	for (const auto &member : shared_task->GetMembers()) {
		client_list.SendCharacterMessage(member.character_id, chat_type, message);
	}
}

void SharedTaskManager::SendMembersMessageID(
	SharedTask *shared_task,
	int chat_type,
	int eqstr_id,
	std::initializer_list<std::string> args
)
{
	if (!shared_task || shared_task->GetMembers().empty()) {
		return;
	}

	// serialize here since using client_list methods would re-serialize for every member
	SerializeBuffer serialized_args;
	for (const auto &arg : args) {
		serialized_args.WriteString(arg);
	}

	uint32_t args_size = static_cast<uint32_t>(serialized_args.size());
	uint32_t pack_size = sizeof(CZClientMessageString_Struct) + args_size;
	auto     pack      = std::make_unique<ServerPacket>(ServerOP_CZClientMessageString, pack_size);
	auto     buf       = reinterpret_cast<CZClientMessageString_Struct *>(pack->pBuffer);
	buf->string_id = eqstr_id;
	buf->chat_type = chat_type;
	buf->args_size = args_size;
	memcpy(buf->args, serialized_args.buffer(), serialized_args.size());

	for (const auto &member : shared_task->GetMembers()) {
		auto character = client_list.FindCLEByCharacterID(member.character_id);
		if (character && character->Server()) {
			strn0cpy(buf->character_name, character->name(), sizeof(buf->character_name));
			character->Server()->SendPacket(pack.get());
		}
	}
}

bool SharedTaskManager::CanRequestSharedTask(
	uint32_t task_id,
	uint32_t character_id,
	const SharedTaskRequestCharacters &request
)
{
	auto task = GetSharedTaskDataByTaskId(task_id);
	if (task.id == 0) {
		return false;
	}

	// this attempts to follow live validation order

	// check if any party members are already in a shared task
	auto shared_task_members = FindCharactersInSharedTasks(request.character_ids);
	if (!shared_task_members.empty()) {
		// messages for every character already in a shared task
		for (const auto &member : shared_task_members) {
			auto it = std::find_if(
				request.characters.begin(), request.characters.end(),
				[&](const CharacterDataRepository::CharacterData &char_data) {
					return char_data.id == member;
				}
			);

			if (it != request.characters.end()) {
				if (it->id == character_id) {
					client_list.SendCharacterMessageID(
						character_id,
						Chat::Red,
						SharedTaskMessage::NO_REQUEST_BECAUSE_HAVE_ONE
					);
				}
				else if (request.group_type == SharedTaskRequestGroupType::Group) {
					client_list.SendCharacterMessageID(
						character_id,
						Chat::Red,
						SharedTaskMessage::NO_REQUEST_BECAUSE_GROUP_HAS_ONE,
						{it->name}
					);
				}
				else {
					client_list.SendCharacterMessageID(
						character_id,
						Chat::Red,
						SharedTaskMessage::NO_REQUEST_BECAUSE_RAID_HAS_ONE,
						{it->name}
					);
				}
			}
		}

		return false;
	}

	// check if any party member's minimum level is too low (pre-2014 this was average level)
	if (task.minlevel > 0 && request.lowest_level < task.minlevel) {
		client_list.SendCharacterMessageID(character_id, Chat::Red, SharedTaskMessage::AVG_LVL_LOW);
		return false;
	}

	// check if any party member's maximum level is too high (pre-2014 this was average level)
	if (task.maxlevel > 0 && request.highest_level > task.maxlevel) {
		client_list.SendCharacterMessageID(character_id, Chat::Red, SharedTaskMessage::AVG_LVL_HIGH);
		return false;
	}

	// allow gm/dev bypass for minimum player count requirements
	auto requester = client_list.FindCLEByCharacterID(character_id);
	bool is_gm     = (requester && requester->GetGM());

	// check if party member count is below the minimum
	if (!is_gm && task.min_players > 0 && request.characters.size() < task.min_players) {
		client_list.SendCharacterMessageID(
			character_id,
			Chat::Red,
			SharedTaskMessage::SHARED_TASK_NOT_MEET_MIN_NUM_PLAYER
		);
		return false;
	}

	// check if party member count is above the maximum
	// todo: live creates the shared task but truncates members if it exceeds max (sorted by leader and raid group numbers)
	if (task.max_players > 0 && request.characters.size() > task.max_players) {
		client_list.SendCharacterMessageID(character_id, Chat::Red, SharedTaskMessage::PARTY_EXCEED_MAX_PLAYER);
		return false;
	}

	// check if party level spread exceeds task's maximum
	if (task.level_spread > 0 && (request.highest_level - request.lowest_level) > task.level_spread) {
		client_list.SendCharacterMessageID(character_id, Chat::Red, SharedTaskMessage::LVL_SPREAD_HIGH);
		return false;
	}

	// check if any party members have a replay or request timer for the task (limit 1, replay checked first)
	auto character_task_timers = CharacterTaskTimersRepository::GetWhere(
		*m_database, fmt::format(
			"character_id IN ({}) AND task_id = {} AND expire_time > NOW() ORDER BY timer_type ASC LIMIT 1",
			fmt::join(request.character_ids, ","), task_id
		)
	);

	if (!character_task_timers.empty()) {
		auto timer_type = static_cast<TaskTimerType>(character_task_timers.front().timer_type);
		auto seconds    = character_task_timers.front().expire_time - std::time(nullptr);
		auto days       = fmt::format_int(seconds / 86400).str();
		auto hours      = fmt::format_int((seconds / 3600) % 24).str();
		auto mins       = fmt::format_int((seconds / 60) % 60).str();

		if (character_task_timers.front().character_id == character_id) {
			if (timer_type == TaskTimerType::Replay) {
				client_list.SendCharacterMessageID(
					character_id,
					Chat::Red,
					SharedTaskMessage::YOU_MUST_WAIT_REPLAY_TIMER, {days, hours, mins}
				);
			}
			else if (timer_type == TaskTimerType::Request) {
				client_list.SendCharacterMessage(
					character_id,
					Chat::Red, fmt::format(
						SharedTaskMessage::GetEQStr(SharedTaskMessage::YOU_MUST_WAIT_REQUEST_TIMER), days, hours, mins
					)
				);
			}
		}
		else {
			auto it = std::find_if(
				request.characters.begin(), request.characters.end(),
				[&](const CharacterDataRepository::CharacterData &char_data) {
					return char_data.id == character_task_timers.front().character_id;
				}
			);

			if (it != request.characters.end() && timer_type == TaskTimerType::Replay) {
				client_list.SendCharacterMessageID(
					character_id,
					Chat::Red,
					SharedTaskMessage::PLAYER_MUST_WAIT_REPLAY_TIMER,
					{it->name, days, hours, mins}
				);
			}
			else if (it != request.characters.end() && timer_type == TaskTimerType::Request) {
				client_list.SendCharacterMessage(
					character_id,
					Chat::Red,
					fmt::format(
						SharedTaskMessage::GetEQStr(SharedTaskMessage::PLAYER_MUST_WAIT_REQUEST_TIMER),
						it->name,
						days,
						hours,
						mins
					)
				);
			}
		}

		return false;
	}

	return true;
}

bool SharedTaskManager::CanAddPlayer(SharedTask *s, uint32_t character_id, std::string player_name, bool accepted)
{
	// this attempts to follow live validation order

	bool allow_invite = true;

	// check if task is locked
	if (s->GetDbSharedTask().is_locked) {
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::TASK_NOT_ALLOWING_PLAYERS_AT_TIME);
		allow_invite = false;
	}

	// check if player is online and in cle (other checks require online)
	auto cle = client_list.FindCLEByCharacterID(character_id);
	if (!cle || !cle->Server()) {
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::PLAYER_NOT_ONLINE_TO_ADD, {player_name});
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::COULD_NOT_BE_INVITED, {player_name});
		return false;
	}

	player_name = cle->name();

	// check if player is already in a shared task
	auto shared_task_members = SharedTaskMembersRepository::GetWhere(
		*m_database,
		fmt::format("character_id = {} LIMIT 1", character_id)
	);

	if (!shared_task_members.empty()) {
		auto shared_task_id = shared_task_members.front().shared_task_id;
		if (shared_task_id == s->GetDbSharedTask().id) {
			SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::CANT_ADD_PLAYER_ALREADY_MEMBER, {player_name});
		}
		else {
			SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::CANT_ADD_PLAYER_ALREADY_ASSIGNED, {player_name});
		}
		allow_invite = false;
	}

	// check if player has an outstanding invite
	for (const auto &invite : m_active_invitations) {
		if (invite.character_id == character_id) {
			if (invite.shared_task_id == s->GetDbSharedTask().id) {
				SendLeaderMessageID(
					s,
					Chat::Red,
					SharedTaskMessage::PLAYER_ALREADY_OUTSTANDING_INVITATION_THIS,
					{player_name}
				);
			}
			else {
				SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::PLAYER_ALREADY_OUTSTANDING_ANOTHER, {player_name});
			}
			allow_invite = false;
			break;
		}
	}

	// check if player has a replay or request timer lockout
	// todo: live allows characters with a request timer to be re-invited if they quit, but only until they zone? (investigate/edge case)
	auto task_timers = CharacterTaskTimersRepository::GetWhere(
		*m_database, fmt::format(
			"character_id = {} AND task_id = {} AND expire_time > NOW() ORDER BY timer_type ASC LIMIT 1",
			character_id, s->GetDbSharedTask().task_id
		));

	if (!task_timers.empty()) {
		auto timer_type = static_cast<TaskTimerType>(task_timers.front().timer_type);
		auto seconds    = task_timers.front().expire_time - std::time(nullptr);
		auto days       = fmt::format_int(seconds / 86400).str();
		auto hours      = fmt::format_int((seconds / 3600) % 24).str();
		auto mins       = fmt::format_int((seconds / 60) % 60).str();

		if (timer_type == TaskTimerType::Replay) {
			SendLeaderMessageID(
				s,
				Chat::Red,
				SharedTaskMessage::CANT_ADD_PLAYER_REPLAY_TIMER, {player_name, days, hours, mins}
			);
		}
		else {
			SendLeaderMessage(
				s,
				Chat::Red,
				fmt::format(
					SharedTaskMessage::GetEQStr(SharedTaskMessage::CANT_ADD_PLAYER_REQUEST_TIMER),
					player_name,
					days,
					hours,
					mins
				)
			);
		}

		allow_invite = false;
	}

	// check if task has maximum players
	if (s->GetTaskData().max_players > 0 && s->GetMembers().size() >= s->GetTaskData().max_players) {
		auto max = fmt::format_int(s->GetTaskData().max_players).str();
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::CANT_ADD_PLAYER_MAX_PLAYERS, {max});
		allow_invite = false;
	}

	// check if task would exceed max level spread
	if (s->GetTaskData().level_spread > 0) {
		auto characters = CharacterDataRepository::GetWhere(
			*m_database,
			fmt::format(
				"id IN (select character_id from shared_task_members where shared_task_id = {})",
				s->GetDbSharedTask().id
			)
		);

		int lowest_level  = cle->level();
		int highest_level = cle->level();

		for (const auto &character : characters) {
			lowest_level  = std::min(lowest_level, character.level);
			highest_level = std::max(highest_level, character.level);
		}

		if ((highest_level - lowest_level) > s->GetTaskData().level_spread) {
			auto max_spread = fmt::format_int(s->GetTaskData().level_spread).str();
			SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::CANT_ADD_PLAYER_MAX_LEVEL_SPREAD, {max_spread});
			allow_invite = false;
		}
	}

	// check if player is below minimum level of task (pre-2014 this was average level)
	if (s->GetTaskData().minlevel > 0 && cle->level() < s->GetTaskData().minlevel) {
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::CANT_ADD_PLAYER_FALL_MIN_AVG_LEVEL);
		allow_invite = false;
	}

	// check if player is above maximum level of task (pre-2014 this was average level)
	if (s->GetTaskData().maxlevel > 0 && cle->level() > s->GetTaskData().maxlevel) {
		SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::CANT_ADD_PLAYER_MAX_AVERAGE_LEVEL);
		allow_invite = false;
	}

	if (!allow_invite) {
		if (!accepted) {
			SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::COULD_NOT_BE_INVITED, {player_name});
		}
		else {
			SendLeaderMessageID(s, Chat::Red, SharedTaskMessage::ACCEPTED_OFFER_TO_JOIN_BUT_COULD_NOT, {player_name});
		}
	}

	return allow_invite;
}

void SharedTaskManager::RecordSharedTaskCompletion(SharedTask *s)
{
	// shared task
	auto t  = s->GetDbSharedTask();
	auto ct = CompletedSharedTasksRepository::NewEntity();

	ct.id              = t.id;
	ct.task_id         = t.task_id;
	ct.accepted_time   = t.accepted_time;
	ct.expire_time     = t.expire_time;
	ct.completion_time = t.completion_time;
	ct.is_locked       = t.is_locked;

	CompletedSharedTasksRepository::InsertOne(*m_database, ct);

	// completed members
	std::vector<CompletedSharedTaskMembersRepository::CompletedSharedTaskMembers> completed_members = {};

	for (auto &m: s->GetMembers()) {
		auto cm = CompletedSharedTaskMembersRepository::NewEntity();

		cm.shared_task_id = t.id;
		cm.character_id   = m.character_id;
		cm.is_leader      = m.is_leader;

		completed_members.emplace_back(cm);
	}

	CompletedSharedTaskMembersRepository::InsertMany(*m_database, completed_members);

	// activities
	std::vector<CompletedSharedTaskActivityStateRepository::CompletedSharedTaskActivityState> completed_states = {};

	for (auto &a: s->GetActivityState()) {
		auto cs = CompletedSharedTaskActivityStateRepository::NewEntity();

		cs.shared_task_id = t.id;
		cs.activity_id    = (int) a.activity_id;
		cs.done_count     = (int) a.done_count;
		cs.updated_time   = a.updated_time;
		cs.completed_time = a.completed_time;

		completed_states.emplace_back(cs);
	}

	CompletedSharedTaskActivityStateRepository::InsertMany(*m_database, completed_states);

}

void SharedTaskManager::AddReplayTimers(SharedTask *s)
{
	if (s->GetTaskData().replay_timer_seconds > 0) {
		auto expire_time = s->GetDbSharedTask().accepted_time + s->GetTaskData().replay_timer_seconds;
		auto seconds     = expire_time - std::time(nullptr);
		if (seconds > 0) // not already expired
		{
			std::vector<CharacterTaskTimersRepository::CharacterTaskTimers> task_timers;

			// on live past members of the shared task also receive lockouts (use member history)
			for (const auto &member_id : s->member_id_history) {
				auto timer = CharacterTaskTimersRepository::NewEntity();
				timer.character_id = member_id;
				timer.task_id      = s->GetTaskData().id;
				timer.timer_type   = static_cast<int>(TaskTimerType::Replay);
				timer.expire_time  = expire_time;

				task_timers.emplace_back(timer);

				client_list.SendCharacterMessage(
					member_id,
					Chat::Yellow,
					fmt::format(
						SharedTaskMessage::GetEQStr(SharedTaskMessage::RECEIVED_REPLAY_TIMER),
						s->GetTaskData().title,
						fmt::format_int(seconds / 86400).c_str(),       // days
						fmt::format_int((seconds / 3600) % 24).c_str(), // hours
						fmt::format_int((seconds / 60) % 60).c_str()    // minutes
					)
				);
			}

			if (!task_timers.empty()) {
				// replay timers replace any existing timer (even if it expires sooner)
				// this can occur if a player has a timer for being a past member of
				// a shared task but joined another before the first was completed
				CharacterTaskTimersRepository::DeleteWhere(
					*m_database,
					fmt::format(
						"task_id = {} AND character_id IN ({})",
						s->GetTaskData().id, fmt::join(s->member_id_history, ",")
					)
				);

				CharacterTaskTimersRepository::InsertMany(*m_database, task_timers);
			}
		}
	}
}

// memory search
std::vector<uint32_t> SharedTaskManager::FindCharactersInSharedTasks(const std::vector<uint32_t> &find_characters)
{
	std::vector<uint32_t> characters = {};

	for (auto &s: m_shared_tasks) {
		// loop through members
		for (auto &m: s.GetMembers()) {
			// compare members with requested characters
			for (auto &find_character_id: find_characters) {
				// found character, add to list
				if (find_character_id == m.character_id) {
					characters.emplace_back(m.character_id);
				}
			}
		}
	}

	return characters;
}

void SharedTaskManager::PurgeAllSharedTasks()
{
	for (auto &shared_task : m_shared_tasks) {
		RemoveAllMembersFromDynamicZones(&shared_task);
	}

	SharedTasksRepository::Truncate(*m_database);
	SharedTaskMembersRepository::Truncate(*m_database);
	SharedTaskActivityStateRepository::Truncate(*m_database);
	SharedTaskDynamicZonesRepository::Truncate(*m_database);
	CompletedSharedTasksRepository::Truncate(*m_database);
	CompletedSharedTaskMembersRepository::Truncate(*m_database);
	CompletedSharedTaskActivityStateRepository::Truncate(*m_database);

	LoadSharedTaskState();
}

void SharedTaskManager::RemoveAllMembersFromDynamicZones(SharedTask *s)
{
	for (const auto &dz_id : s->dynamic_zone_ids) {
		auto dz = DynamicZone::FindDynamicZoneByID(dz_id);
		if (dz) {
			dz->RemoveAllMembers();
		}
	}
}

void SharedTaskManager::ChooseNewLeader(SharedTask *s)
{
	// live doesn't prioritize choosing an online player here
	auto members = s->GetMembers();
	auto it      = std::find_if(
		members.begin(), members.end(),
		[&](const SharedTaskMember &member) {
			return !member.is_leader;
		}
	);

	if (it != members.end()) {
		MakeLeaderByPlayerName(s, it->character_name);
	}
}

const std::vector<SharedTask> &SharedTaskManager::GetSharedTasks() const
{
	return m_shared_tasks;
}

void SharedTaskManager::SetSharedTasks(const std::vector<SharedTask> &shared_tasks)
{
	SharedTaskManager::m_shared_tasks = shared_tasks;
}

SharedTaskManager *SharedTaskManager::PurgeExpiredSharedTasks()
{
	auto      now = std::time(nullptr);
	for (auto &s: m_shared_tasks) {
		if (s.GetDbSharedTask().expire_time > 0 && s.GetDbSharedTask().expire_time <= now) {
			LogTasksDetail("[PurgeExpiredSharedTasks] Deleting expired task [{}]", s.GetDbSharedTask().id);
			DeleteSharedTask(s.GetDbSharedTask().id);
		}
	}

	return this;
}
