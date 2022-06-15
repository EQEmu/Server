#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/repositories/character_activities_repository.h"
#include "../common/repositories/character_data_repository.h"
#include "../common/repositories/character_tasks_repository.h"
#include "../common/repositories/completed_tasks_repository.h"
#include "../common/repositories/task_activities_repository.h"
#include "../common/repositories/tasks_repository.h"
#include "../common/repositories/tasksets_repository.h"
#include "client.h"
#include "string_ids.h"
#include "task_manager.h"
#include "../common/repositories/shared_task_activity_state_repository.h"
#include "../common/repositories/shared_task_members_repository.h"
#include "../common/shared_tasks.h"
#include "worldserver.h"

extern WorldServer worldserver;

TaskManager::TaskManager()
{
	for (auto &task : m_task_data) {
		task = nullptr;
	}
}

TaskManager::~TaskManager()
{
	for (auto &task : m_task_data) {
		if (task != nullptr) {
			safe_delete(task);
		}
	}
}

bool TaskManager::LoadTaskSets()
{
	// Clear all task sets in memory. Done so we can reload them on the fly if required by just calling
	// this method again.
	for (auto &task_set : m_task_sets) {
		task_set.clear();
	}

	auto rows = TasksetsRepository::GetWhere(
		content_db,
		fmt::format(
			"`id` > 0 AND `id` < {} AND `taskid` >= 0 AND `taskid` < {} ORDER BY `id`, `taskid` ASC",
			MAXTASKSETS,
			MAXTASKS
		)
	);

	for (auto &task_set: rows) {
		m_task_sets[task_set.id].push_back(task_set.taskid);
		LogTasksDetail("[LoadTaskSets] Adding task_id [{}] to task_set [{}]", task_set.taskid, task_set.id);
	}

	return true;
}

void TaskManager::ReloadGoalLists()
{
	if (!m_goal_list_manager.LoadLists()) {
		Log(Logs::Detail, Logs::Tasks, "TaskManager::LoadTasks LoadLists failed");
	}
}

bool TaskManager::LoadTasks(int single_task)
{
	std::string task_query_filter = fmt::format("id = {}", single_task);
	std::string query;
	if (single_task == 0) {
		if (!m_goal_list_manager.LoadLists()) {
			LogTasks("[TaskManager::LoadTasks] LoadLists failed");
		}

		if (!LoadTaskSets()) {
			LogTasks("[TaskManager::LoadTasks] LoadTaskSets failed");
		}

		task_query_filter = fmt::format("id < {}", MAXTASKS);
	}

	// load task level data
	auto repo_tasks = TasksRepository::GetWhere(content_db, task_query_filter);

	for (auto &task: repo_tasks) {
		int task_id = task.id;

		if ((task_id <= 0) || (task_id >= MAXTASKS)) {
			// This shouldn't happen, as the SELECT is bounded by MAXTASKS
			LogError("[TASKS]Task ID [{}] out of range while loading tasks from database", task_id);
			continue;
		}

		// load task data
		m_task_data[task_id] = new TaskInformation();
		m_task_data[task_id]->type                    = static_cast<TaskType>(task.type);
		m_task_data[task_id]->duration                = task.duration;
		m_task_data[task_id]->duration_code           = static_cast<DurationCode>(task.duration_code);
		m_task_data[task_id]->title                   = task.title;
		m_task_data[task_id]->description             = task.description;
		m_task_data[task_id]->reward                  = task.reward;
		m_task_data[task_id]->reward_id               = task.rewardid;
		m_task_data[task_id]->cash_reward             = task.cashreward;
		m_task_data[task_id]->experience_reward       = task.xpreward;
		m_task_data[task_id]->reward_method           = (TaskMethodType) task.rewardmethod;
		m_task_data[task_id]->reward_radiant_crystals = task.reward_radiant_crystals;
		m_task_data[task_id]->reward_ebon_crystals    = task.reward_ebon_crystals;
		m_task_data[task_id]->faction_reward          = task.faction_reward;
		m_task_data[task_id]->min_level               = task.minlevel;
		m_task_data[task_id]->max_level               = task.maxlevel;
		m_task_data[task_id]->level_spread            = task.level_spread;
		m_task_data[task_id]->min_players             = task.min_players;
		m_task_data[task_id]->max_players             = task.max_players;
		m_task_data[task_id]->repeatable              = task.repeatable;
		m_task_data[task_id]->completion_emote        = task.completion_emote;
		m_task_data[task_id]->replay_timer_seconds    = task.replay_timer_seconds;
		m_task_data[task_id]->request_timer_seconds   = task.request_timer_seconds;
		m_task_data[task_id]->activity_count          = 0;
		m_task_data[task_id]->sequence_mode           = ActivitiesSequential;
		m_task_data[task_id]->last_step               = 0;

		LogTasksDetail(
			"[LoadTasks] (Task) task_id [{}] type [{}] () duration [{}] duration_code [{}] title [{}] description [{}] "
			" reward [{}] rewardid [{}] cashreward [{}] xpreward [{}] rewardmethod [{}] faction_reward [{}] minlevel [{}] "
			" maxlevel [{}] level_spread [{}] min_players [{}] max_players [{}] repeatable [{}] completion_emote [{}]",
			" replay_timer_seconds [{}] request_timer_seconds [{}]",
			task.id,
			task.type,
			task.duration,
			task.duration_code,
			task.title,
			task.description,
			task.reward,
			task.rewardid,
			task.cashreward,
			task.xpreward,
			task.rewardmethod,
			task.faction_reward,
			task.minlevel,
			task.maxlevel,
			task.level_spread,
			task.min_players,
			task.max_players,
			task.repeatable,
			task.completion_emote,
			task.replay_timer_seconds,
			task.request_timer_seconds
		);
	}

	LogTasks("Loaded [{}] Tasks", repo_tasks.size());

	std::string activities_query_filter = fmt::format(
		"taskid = {} and activityid < {} ORDER BY taskid, activityid ASC",
		single_task,
		MAXACTIVITIESPERTASK
	);

	// if loading only a single task
	if (single_task == 0) {
		activities_query_filter = fmt::format(
			"taskid < {} and activityid < {} ORDER BY taskid, activityid ASC",
			MAXTASKS,
			MAXACTIVITIESPERTASK
		);
	}

	// load activity data
	auto      task_activities = TaskActivitiesRepository::GetWhere(content_db, activities_query_filter);
	for (auto &task_activity: task_activities) {
		int task_id     = task_activity.taskid;
		int step        = task_activity.step;
		int activity_id = task_activity.activityid;

		if ((task_id <= 0) || (task_id >= MAXTASKS) || (activity_id < 0) || (activity_id >= MAXACTIVITIESPERTASK)) {

			// This shouldn't happen, as the SELECT is bounded by MAXTASKS
			LogTasks(
				"[LoadTasks] Error: Task or activity_information ID ([{}], [{}]) out of range while loading activities from database",
				task_id,
				activity_id
			);
			continue;
		}

		if (m_task_data[task_id] == nullptr) {
			LogTasks(
				"[LoadTasks] Error: activity_information for non-existent task ([{}], [{}]) while loading activities from database",
				task_id,
				activity_id
			);
			continue;
		}

		// create pointer to activity data since declarations get unruly long
		int                 activity_index = m_task_data[task_id]->activity_count;
		ActivityInformation *activity_data = &m_task_data[task_id]->activity_information[activity_index];

		m_task_data[task_id]->activity_information[m_task_data[task_id]->activity_count].step_number = step;

		if (step != 0) {
			m_task_data[task_id]->sequence_mode = ActivitiesStepped;
		}

		if (step > m_task_data[task_id]->last_step) {
			m_task_data[task_id]->last_step = step;
		}

		// Task Activities MUST be numbered sequentially from 0. If not, log an error
		// and set the task to nullptr. Subsequent activities for this task will raise
		// ERR_NOTASK errors.
		// Change to (activityID != (Tasks[taskID]->activity_count + 1)) to index from 1
		if (activity_id != m_task_data[task_id]->activity_count) {
			LogTasks(
				"[LoadTasks] Error: Activities for Task [{}] (activity_id [{}]) are not sequential starting at 0. Not loading task ",
				task_id,
				activity_id
			);
			m_task_data[task_id] = nullptr;
			continue;
		}

		// set activity data
		activity_data->activity_type        = static_cast<TaskActivityType>(task_activity.activitytype);
		activity_data->target_name          = task_activity.target_name;
		activity_data->item_list            = task_activity.item_list;
		activity_data->skill_list           = task_activity.skill_list;
		activity_data->skill_id             = StringIsNumber(task_activity.skill_list) ? std::stoi(task_activity.skill_list) : 0; // for older clients
		activity_data->spell_list           = task_activity.spell_list;
		activity_data->spell_id             = StringIsNumber(task_activity.spell_list) ? std::stoi(task_activity.spell_list) : 0; // for older clients
		activity_data->description_override = task_activity.description_override;
		activity_data->goal_id              = task_activity.goalid;
		activity_data->goal_method          = (TaskMethodType) task_activity.goalmethod;
		activity_data->goal_match_list      = task_activity.goal_match_list;
		activity_data->goal_count           = task_activity.goalcount;
		activity_data->deliver_to_npc       = task_activity.delivertonpc;

		// zones
		activity_data->zones = task_activity.zones;
		auto zones = SplitString(
			task_activity.zones,
			';'
		);

		for (auto &&e : zones) {
			activity_data->zone_ids.push_back(std::stoi(e));
		}

		activity_data->optional = task_activity.optional;

		LogTasksDetail(
			"[LoadTasks] (Activity) task_id [{}] activity_id [{}] slot [{}] activity_type [{}] goal_id [{}] goal_method [{}] goal_count [{}] zones [{}]"
			" target_name [{}] item_list [{}] skill_list [{}] spell_list [{}] description_override [{}] sequence [{}]",
			task_id,
			activity_id,
			m_task_data[task_id]->activity_count,
			static_cast<int32_t>(activity_data->activity_type),
			activity_data->goal_id,
			activity_data->goal_method,
			activity_data->goal_count,
			activity_data->zones.c_str(),
			activity_data->target_name.c_str(),
			activity_data->item_list.c_str(),
			activity_data->skill_list.c_str(),
			activity_data->spell_list.c_str(),
			activity_data->description_override.c_str(),
			(m_task_data[task_id]->sequence_mode == ActivitiesStepped ? "stepped" : "sequential")
		);

		m_task_data[task_id]->activity_count++;
	}

	LogTasks("Loaded [{}] Task Activities", task_activities.size());

	return true;
}

bool TaskManager::SaveClientState(Client *client, ClientTaskState *client_task_state)
{
	// I am saving the slot in the ActiveTasks table, because unless a Task is cancelled/completed, the client
	// doesn't seem to like tasks moving slots between zoning and you can end up with 'bogus' activities if the task
	// previously in that slot had more activities than the one now occupying it. Hopefully retaining the slot
	// number for the duration of a session will overcome this.
	if (!client || !client_task_state) {
		return false;
	}

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::SaveClientState {}";

	int character_id = client->CharacterID();

	LogTasks("[SaveClientState] character_id [{}]", character_id);

	if (client_task_state->m_active_task_count > 0 ||
		client_task_state->m_active_task.task_id != TASKSLOTEMPTY ||
		client_task_state->m_active_shared_task.task_id != TASKSLOTEMPTY) {
		for (auto &active_task : client_task_state->m_active_tasks) {
			int task_id = active_task.task_id;
			if (task_id == TASKSLOTEMPTY) {
				continue;
			}

			int slot = active_task.slot;
			if (active_task.updated) {

				LogTasks(
					"[SaveClientState] character_id [{}] updating task_index [{}] task_id [{}]",
					character_id,
					slot,
					task_id
				);

				std::string query = StringFormat(
					"REPLACE INTO character_tasks (charid, taskid, slot, type, acceptedtime) "
					"VALUES (%i, %i, %i, %i, %i)",
					character_id,
					task_id,
					slot,
					static_cast<int>(m_task_data[task_id]->type),
					active_task.accepted_time
				);

				auto results = database.QueryDatabase(query);
				if (!results.Success()) {
					LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
				}
				else {
					active_task.updated = false;
				}
			}

			std::string query =
							"REPLACE INTO character_activities (charid, taskid, activityid, donecount, completed) "
							"VALUES ";

			int updated_activity_count = 0;

			for (int activity_index = 0; activity_index < m_task_data[task_id]->activity_count; ++activity_index) {
				if (!active_task.activity[activity_index].updated) {
					continue;
				}

				LogTasks(
					"[SaveClientState] Updating activity character_id [{}] updating task_index [{}] task_id [{}] activity_index [{}]",
					character_id,
					slot,
					task_id,
					activity_index
				);

				if (updated_activity_count == 0) {
					query +=
						StringFormat(
							"(%i, %i, %i, %i, %i)", character_id, task_id, activity_index,
							active_task.activity[activity_index].done_count,
							active_task.activity[activity_index].activity_state ==
							ActivityCompleted
						);
				}
				else {
					query +=
						StringFormat(
							", (%i, %i, %i, %i, %i)", character_id, task_id, activity_index,
							active_task.activity[activity_index].done_count,
							active_task.activity[activity_index].activity_state ==
							ActivityCompleted
						);
				}

				updated_activity_count++;
			}

			if (updated_activity_count == 0) {
				continue;
			}

			auto results = database.QueryDatabase(query);

			if (!results.Success()) {
				LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
				continue;
			}

			active_task.updated = false;
			for (int activity_index = 0; activity_index < m_task_data[task_id]->activity_count; ++activity_index) {
				active_task.activity[activity_index].updated = false;
			}
		}
	}

	if (!RuleB(TaskSystem, RecordCompletedTasks) || (client_task_state->m_completed_tasks.size() <=
													 (unsigned int) client_task_state->m_last_completed_task_loaded)) {
		client_task_state->m_last_completed_task_loaded = client_task_state->m_completed_tasks.size();
		return true;
	}

	const char *completed_task_query = "REPLACE INTO completed_tasks (charid, completedtime, taskid, activityid) "
									   "VALUES (%i, %i, %i, %i)";

	for (unsigned int task_index = client_task_state->m_last_completed_task_loaded;
		task_index < client_task_state->m_completed_tasks.size();
		task_index++) {

		int task_id = client_task_state->m_completed_tasks[task_index].task_id;

		if ((task_id <= 0) || (task_id >= MAXTASKS) || (m_task_data[task_id] == nullptr)) {
			continue;
		}

		// we don't record completed shared tasks in the task quest log
		if (m_task_data[task_id]->type == TaskType::Shared) {
			break;
		}

		// First we save a record with an activity_id of -1.
		// This indicates this task was completed at the given time. We infer that all
		// none optional activities were completed.
		//
		std::string query = StringFormat(
			completed_task_query,
			character_id,
			client_task_state->m_completed_tasks[task_index].completed_time,
			task_id,
			-1
		);

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
			continue;
		}

		// If the Rule to record non-optional task completion is not enabled, don't save it
		if (!RuleB(TaskSystem, RecordCompletedOptionalActivities)) {
			continue;
		}

		// Insert one record for each completed optional task.
		for (int activity_id = 0; activity_id < m_task_data[task_id]->activity_count; activity_id++) {
			if (!m_task_data[task_id]->activity_information[activity_id].optional ||
				!client_task_state->m_completed_tasks[task_index].activity_done[activity_id]) {
				continue;
			}

			query = StringFormat(
				completed_task_query,
				character_id,
				client_task_state->m_completed_tasks[task_index].completed_time,
				task_id, activity_id
			);

			results = database.QueryDatabase(query);
			if (!results.Success()) {
				LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
			}
		}
	}

	client_task_state->m_last_completed_task_loaded = client_task_state->m_completed_tasks.size();
	return true;
}

int TaskManager::FirstTaskInSet(int task_set)
{
	if ((task_set <= 0) || (task_set >= MAXTASKSETS)) {
		return 0;
	}

	if (m_task_sets[task_set].empty()) {
		return 0;
	}

	auto iterator = m_task_sets[task_set].begin();
	while (iterator != m_task_sets[task_set].end()) {
		if ((*iterator) > 0) {
			return (*iterator);
		}
		++iterator;
	}

	return 0;
}

int TaskManager::LastTaskInSet(int task_set)
{
	if ((task_set <= 0) || (task_set >= MAXTASKSETS)) {
		return 0;
	}

	if (m_task_sets[task_set].empty()) {
		return 0;
	}

	return m_task_sets[task_set][m_task_sets[task_set].size() - 1];
}

int TaskManager::NextTaskInSet(int task_set, int task_id)
{
	if ((task_set <= 0) || (task_set >= MAXTASKSETS)) {
		return 0;
	}

	if (m_task_sets[task_set].empty()) {
		return 0;
	}

	for (int i : m_task_sets[task_set]) {
		if (i > task_id) {
			return i;
		}
	}

	return 0;
}

bool TaskManager::ValidateLevel(int task_id, int player_level)
{
	if (m_task_data[task_id] == nullptr) {
		return false;
	}

	if (m_task_data[task_id]->min_level && (player_level < m_task_data[task_id]->min_level)) {
		return false;
	}

	if (m_task_data[task_id]->max_level && (player_level > m_task_data[task_id]->max_level)) {
		return false;
	}

	return true;
}

std::string TaskManager::GetTaskName(uint32 task_id)
{
	if (task_id > 0 && task_id < MAXTASKS) {
		if (m_task_data[task_id] != nullptr) {
			return m_task_data[task_id]->title;
		}
	}

	return std::string();
}

TaskType TaskManager::GetTaskType(uint32 task_id)
{
	if (task_id > 0 && task_id < MAXTASKS) {
		if (m_task_data[task_id] != nullptr) {
			return m_task_data[task_id]->type;
		}
	}
	return TaskType::Task;
}

void TaskManager::TaskSetSelector(Client *client, ClientTaskState *client_task_state, Mob *mob, int task_set_id)
{
	int task_list[MAXCHOOSERENTRIES];
	int task_list_index = 0;
	int player_level    = client->GetLevel();

	LogTasks(
		"TaskSetSelector called for task_set_id [{}] EnableTaskSize is [{}]",
		task_set_id,
		client_task_state->m_enabled_tasks.size()
	);

	if (task_set_id <= 0 || task_set_id >= MAXTASKSETS) {
		return;
	}

	// forward to shared task selector validation if set contains a shared task
	for (const auto& task_id : m_task_sets[task_set_id])
	{
		if (m_task_data[task_id] && m_task_data[task_id]->type == TaskType::Shared) {
			SharedTaskSelector(client, mob, m_task_sets[task_set_id].size(), m_task_sets[task_set_id].data());
			return;
		}
	}

	if (client->HasTaskRequestCooldownTimer()) {
		client->SendTaskRequestCooldownTimerMessage();
		return;
	}

	if (m_task_sets[task_set_id].empty()) {
		client->MessageString(Chat::Yellow, NO_TASK_OFFERS, ".", ".", client->GetName());
		return;
	}

	bool all_enabled = false;

	// A task_id of 0 in a TaskSet indicates that all Tasks in the set are enabled for all players.
	if (m_task_sets[task_set_id][0] == 0) {
		LogTasks("[TaskSetSelector] TaskSets[{}][0] == 0. All Tasks in Set enabled.", task_set_id);
		all_enabled = true;
	}

	auto iterator = m_task_sets[task_set_id].begin();
	if (all_enabled) {
		++iterator;
	} // skip first when all enabled since it's useless data

	while (iterator != m_task_sets[task_set_id].end() && task_list_index < MAXCHOOSERENTRIES) {
		auto task = *iterator;
		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if ((all_enabled || client_task_state->IsTaskEnabled(task)) && ValidateLevel(task, player_level) &&
			!client_task_state->IsTaskActive(task) && client_task_state->HasSlotForTask(m_task_data[task]) &&
			// this slot checking is a bit silly, but we allow mixing of task types ...
			(IsTaskRepeatable(task) || !client_task_state->IsTaskCompleted(task))) {
			task_list[task_list_index++] = task;
		}

		++iterator;
	}

	if (task_list_index > 0) {
		SendTaskSelector(client, mob, task_list_index, task_list);
	}
	else {
		client->MessageString(Chat::Yellow, NO_TASK_OFFERS, ".", ".", client->GetName());
	}
}

// unlike the non-Quest version of this function, it does not check enabled, that is assumed the responsibility of the quest to handle
// we do however still want it to check the other stuff like level, active, room, etc
void TaskManager::TaskQuestSetSelector(
	Client *client,
	ClientTaskState *client_task_state,
	Mob *mob,
	int count,
	int *tasks
)
{
	int task_list[MAXCHOOSERENTRIES];
	int task_list_index = 0;
	int player_level    = client->GetLevel();

	LogTasks("[UPDATE] TaskQuestSetSelector called for array size [{}]", count);

	if (count <= 0) {
		return;
	}

	// live prevents mixing selection types (also uses diff opcodes for solo vs shared tasks)
	// to keep shared task validation live-like (and simple), any shared task will
	// forward this to shared task validation and non-shared tasks will be dropped
	for (int i = 0; i < count; ++i) {
		auto task = tasks[i];
		if (m_task_data[task] && m_task_data[task]->type == TaskType::Shared) {
			SharedTaskSelector(client, mob, count, tasks);
			return;
		}
	}

	if (client->HasTaskRequestCooldownTimer()) {
		client->SendTaskRequestCooldownTimerMessage();
		return;
	}

	for (int i = 0; i < count; ++i) {
		auto task = tasks[i];
		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if (ValidateLevel(task, player_level) && !client_task_state->IsTaskActive(task) &&
			client_task_state->HasSlotForTask(m_task_data[task]) &&
			// this slot checking is a bit silly, but we allow mixing of task types ...
			(IsTaskRepeatable(task) || !client_task_state->IsTaskCompleted(task))) {
			task_list[task_list_index++] = task;
		}
	}

	if (task_list_index > 0) {
		SendTaskSelector(client, mob, task_list_index, task_list);
	}
	else {
		client->MessageString(Chat::Yellow, NO_TASK_OFFERS, ".", ".", client->GetName());
	}
}

void TaskManager::SharedTaskSelector(Client *client, Mob *mob, int count, const int *tasks)
{
	LogTasks("[UPDATE] SharedTaskSelector called for array size [{}]", count);

	if (count <= 0 || client->HasTaskRequestCooldownTimer()) {
		client->SendTaskRequestCooldownTimerMessage();
		return;
	}

	// check if requester already has a shared task (no need to query group/raid members if so)
	if (client->GetTaskState()->HasActiveSharedTask()) {
		client->MessageString(Chat::Red, SharedTaskMessage::NO_REQUEST_BECAUSE_HAVE_ONE);
		return;
	}

	// get group/raid member character data from db (need to query for character ids)
	auto request = SharedTask::GetRequestCharacters(database, client->CharacterID());

	// check if any group/raid member already has a shared task (already checked solo character)
	bool validation_failed = false;
	if (request.group_type != SharedTaskRequestGroupType::Solo) {
		auto shared_task_members = SharedTaskMembersRepository::GetWhere(
			database,
			fmt::format("character_id IN ({}) LIMIT 1", fmt::join(request.character_ids, ",")));

		if (!shared_task_members.empty()) {
			validation_failed = true;

			auto it = std::find_if(
				request.characters.begin(), request.characters.end(),
				[&](const CharacterDataRepository::CharacterData &char_data) {
					return char_data.id == shared_task_members.front().character_id;
				}
			);

			if (it != request.characters.end()) {
				if (request.group_type == SharedTaskRequestGroupType::Group) {
					client->MessageString(
						Chat::Red,
						SharedTaskMessage::NO_REQUEST_BECAUSE_GROUP_HAS_ONE,
						it->name.c_str());
				}
				else {
					client->MessageString(
						Chat::Red,
						SharedTaskMessage::NO_REQUEST_BECAUSE_RAID_HAS_ONE,
						it->name.c_str());
				}
			}
		}
	}

	if (!validation_failed) {
		// run type and level filters on task selections
		int task_list[MAXCHOOSERENTRIES] = {0};
		int task_list_index              = 0;

		for (int i = 0; i < count && task_list_index < MAXCHOOSERENTRIES; ++i) {
			// todo: are there non repeatable shared tasks? (would need to check all group/raid members)
			auto task = tasks[i];
			if (m_task_data[task] &&
				m_task_data[task]->type == TaskType::Shared &&
				request.lowest_level >= m_task_data[task]->min_level &&
				(m_task_data[task]->max_level == 0 || request.highest_level <= m_task_data[task]->max_level)) {
				task_list[task_list_index++] = task;
			}
		}

		// check if any tasks are left to offer after filtering
		if (task_list_index > 0) {
			SendSharedTaskSelector(client, mob, task_list_index, task_list);
		}
		else {
			client->MessageString(Chat::Red, SharedTaskMessage::YOU_DO_NOT_MEET_REQ_AVAILABLE);
		}
	}
}

// sends task selector to client
void TaskManager::SendTaskSelector(Client *client, Mob *mob, int task_count, int *task_list)
{
	LogTasks("TaskSelector for [{}] Tasks", task_count);
	int player_level = client->GetLevel();

	// Check if any of the tasks exist
	for (int i = 0; i < task_count; i++) {
		if (m_task_data[task_list[i]] != nullptr) {
			break;
		}
	}

	int      valid_tasks_count = 0;
	for (int task_index        = 0; task_index < task_count; task_index++) {
		if (!ValidateLevel(task_list[task_index], player_level)) {
			continue;
		}
		if (client->IsTaskActive(task_list[task_index])) {
			continue;
		}
		if (!IsTaskRepeatable(task_list[task_index]) && client->IsTaskCompleted(task_list[task_index])) {
			continue;
		}

		valid_tasks_count++;
	}

	if (valid_tasks_count == 0) {
		return;
	}

	SerializeBuffer buf(50 * valid_tasks_count);

	buf.WriteUInt32(valid_tasks_count);    // TaskCount
	buf.WriteUInt32(2);            // Type, valid values: 0-3. 0 = Task, 1 = Shared Task, 2 = Quest, 3 = ??? -- should fix maybe some day, but we let more than 1 type through :P
	// so I guess an NPC can only offer one type of quests or we can only open a selection with one type :P (so quest call can tell us I guess)
	// this is also sent in OP_TaskDescription
	buf.WriteUInt32(mob->GetID());    // TaskGiver

	for (int i = 0; i < task_count; i++) { // max 40
		if (!ValidateLevel(task_list[i], player_level)) {
			continue;
		}
		if (client->IsTaskActive(task_list[i])) {
			continue;
		}
		if (!IsTaskRepeatable(task_list[i]) && client->IsTaskCompleted(task_list[i])) {
			continue;
		}

		buf.WriteUInt32(task_list[i]); // task_id
		m_task_data[task_list[i]]->SerializeSelector(buf, client->ClientVersion());
	}

	auto outapp = std::make_unique<EQApplicationPacket>(OP_TaskSelectWindow, buf);
	client->QueuePacket(outapp.get());
}

void TaskManager::SendSharedTaskSelector(Client *client, Mob *mob, int task_count, int *task_list)
{
	LogTasks("SendSharedTaskSelector for [{}] Tasks", task_count);

	// request timer is only set when shared task selection shown (not for failed validations)
	client->StartTaskRequestCooldownTimer();

	SerializeBuffer buf;

	buf.WriteUInt32(task_count); // number of tasks
	// shared task selection (live doesn't mix types) makes client send shared task specific opcode for accepts
	buf.WriteUInt32(static_cast<uint32_t>(TaskType::Shared));
	buf.WriteUInt32(mob->GetID()); // task giver entity id

	for (int i = 0; i < task_count; ++i) {
		int task_id = task_list[i];
		buf.WriteUInt32(task_id);
		m_task_data[task_id]->SerializeSelector(buf, client->ClientVersion());
	}

	auto outapp = std::make_unique<EQApplicationPacket>(OP_SharedTaskSelectWindow, buf);
	client->QueuePacket(outapp.get());
}

int TaskManager::GetActivityCount(int task_id)
{
	if ((task_id > 0) && (task_id < MAXTASKS)) {
		if (m_task_data[task_id]) { return m_task_data[task_id]->activity_count; }
	}

	return 0;
}

void TaskManager::ExplainTask(Client *client, int task_id)
{

	// TODO: This method is not finished (hardly started). It was intended to
	// explain in English, what each activity_information did, conditions for step unlocking, etc.
	//
	return;

	if (!client) { return; }

	if ((task_id <= 0) || (task_id >= MAXTASKS)) {
		client->Message(Chat::White, "task_id out-of-range.");
		return;
	}

	if (m_task_data[task_id] == nullptr) {
		client->Message(Chat::White, "Task does not exist.");
		return;
	}

	char explanation[1000], *ptr;
	client->Message(Chat::White, "Task %4i: title: %s", task_id, m_task_data[task_id]->description.c_str());
	client->Message(Chat::White, "%3i Activities", m_task_data[task_id]->activity_count);
	ptr = explanation;
	for (int i = 0; i < m_task_data[task_id]->activity_count; i++) {

		sprintf(ptr, "Act: %3i: ", i);
		ptr = ptr + strlen(ptr);
		switch (m_task_data[task_id]->activity_information[i].activity_type) {
			case TaskActivityType::Deliver:
				sprintf(ptr, "Deliver");
				break;
		}

	}
}

bool TaskManager::IsTaskRepeatable(int task_id)
{
	if ((task_id <= 0) || (task_id >= MAXTASKS)) {
		return false;
	}

	TaskInformation *task_data = task_manager->m_task_data[task_id];
	if (task_data == nullptr) {
		return false;
	}

	return task_data->repeatable;
}

void TaskManager::SendCompletedTasksToClient(Client *c, ClientTaskState *client_task_state)
{
	int packet_length = 4;

	//vector<CompletedTaskInformation>::const_iterator iterator;
	// The client only display the first 50 Completed Tasks send, so send the 50 most recent
	int first_task_to_send = 0;
	int last_task_to_send  = client_task_state->m_completed_tasks.size();

	if (client_task_state->m_completed_tasks.size() > 50) {
		first_task_to_send = client_task_state->m_completed_tasks.size() - 50;
	}

	LogTasks(
		"[SendCompletedTasksToClient] completed task count [{}] first tank to send is [{}] last is [{}]",
		client_task_state->m_completed_tasks.size(),
		first_task_to_send,
		last_task_to_send
	);

	/*
	for(iterator=activity_state->CompletedTasks.begin(); iterator!=activity_state->CompletedTasks.end(); iterator++) {
		int task_id = (*iterator).task_id;
		if(Tasks[task_id] == nullptr) continue;
		PacketLength = PacketLength + 8 + strlen(Tasks[task_id]->title) + 1;
	}
	*/
	for (int i = first_task_to_send; i < last_task_to_send; i++) {
		int TaskID = client_task_state->m_completed_tasks[i].task_id;
		if (m_task_data[TaskID] == nullptr) { continue; }
		packet_length = packet_length + 8 + m_task_data[TaskID]->title.size() + 1;
	}

	auto outapp = new EQApplicationPacket(OP_CompletedTasks, packet_length);
	char *buf   = (char *) outapp->pBuffer;

	//*(uint32 *)buf = activity_state->CompletedTasks.size();
	*(uint32 *) buf = last_task_to_send - first_task_to_send;
	buf = buf + 4;
	//for(iterator=activity_state->CompletedTasks.begin(); iterator!=activity_state->CompletedTasks.end(); iterator++) {
	//	int task_id = (*iterator).task_id;
	for (int i = first_task_to_send; i < last_task_to_send; i++) {
		int task_id = client_task_state->m_completed_tasks[i].task_id;
		if (m_task_data[task_id] == nullptr) { continue; }
		*(uint32 *) buf = task_id;
		buf = buf + 4;

		sprintf(buf, "%s", m_task_data[task_id]->title.c_str());
		buf = buf + strlen(buf) + 1;
		//*(uint32 *)buf = (*iterator).CompletedTime;
		*(uint32 *) buf = client_task_state->m_completed_tasks[i].completed_time;
		buf = buf + 4;
	}

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void TaskManager::SendTaskActivityShort(Client *client, int task_id, int activity_id, int client_task_index)
{
	// This activity_information Packet is sent for activities that have not yet been unlocked and appear as ???
	// in the client.
	auto outapp = std::make_unique<EQApplicationPacket>(OP_TaskActivity, 25);
	outapp->WriteUInt32(client_task_index);
	outapp->WriteUInt32(static_cast<uint32>(m_task_data[task_id]->type));
	outapp->WriteUInt32(task_id);
	outapp->WriteUInt32(activity_id);
	outapp->WriteUInt32(0);
	outapp->WriteUInt32(0xffffffff);
	outapp->WriteUInt8(m_task_data[task_id]->activity_information[activity_id].optional ? 1 : 0);
	client->QueuePacket(outapp.get());
}

void TaskManager::SendTaskActivityLong(
	Client *client,
	int task_id,
	int activity_id,
	int client_task_index,
	bool task_complete
)
{
	SerializeBuffer buf(100);

	buf.WriteUInt32(client_task_index);    // TaskSequenceNumber
	buf.WriteUInt32(static_cast<uint32>(m_task_data[task_id]->type));        // task type
	buf.WriteUInt32(task_id);
	buf.WriteUInt32(activity_id);
	buf.WriteUInt32(0);        // unknown3

	const auto &activity = m_task_data[task_id]->activity_information[activity_id];
	int done_count = client->GetTaskActivityDoneCount(m_task_data[task_id]->type, client_task_index, activity_id);

	activity.SerializeObjective(buf, client->ClientVersion(), done_count);

	auto outapp = std::make_unique<EQApplicationPacket>(OP_TaskActivity, buf);
	client->QueuePacket(outapp.get());
}

void TaskManager::SendActiveTaskToClient(
	ClientTaskInformation *task,
	Client *client,
	int task_index,
	bool task_complete
)
{
	auto state = client->GetTaskState();
	if (!state) {
		return;
	}

	int  start_time    = task->accepted_time;
	int  task_id       = task->task_id;
	auto task_type     = m_task_data[task_id]->type;
	auto task_duration = m_task_data[task_id]->duration;

	SendActiveTaskDescription(
		client,
		task_id,
		*task,
		start_time,
		task_duration,
		false
	);

	LogTasks("[SendActiveTasksToClient] task_id [{}] activity_count [{}] task_index [{}]",
			 task_id,
			 GetActivityCount(task_id),
			 task_index);

	int      sequence    = 0;
	int      fixed_index = task_index;
	for (int activity_id = 0; activity_id < GetActivityCount(task_id); activity_id++) {
		if (client->GetTaskActivityState(task_type, fixed_index, activity_id) != ActivityHidden) {
			LogTasks(
				"[SendActiveTasksToClient] (Long Update) task_id [{}] activity_id [{}] fixed_index [{}] task_complete [{}]",
				task_id,
				activity_id,
				fixed_index,
				task_complete ? "true" : "false"
			);

			if (activity_id == GetActivityCount(task_id) - 1) {
				SendTaskActivityLong(
					client,
					task_id,
					activity_id,
					fixed_index,
					task_complete
				);
			}
			else {
				SendTaskActivityLong(
					client,
					task_id,
					activity_id,
					fixed_index,
					0
				);
			}
		}
		else {
			LogTasks(
				"[SendActiveTasksToClient] (Short Update) task_id [{}] activity_id [{}] fixed_index [{}]",
				task_id,
				activity_id,
				fixed_index
			);

			SendTaskActivityShort(client, task_id, activity_id, fixed_index);
		}
		sequence++;
	}
}

void TaskManager::SendActiveTasksToClient(Client *client, bool task_complete)
{
	auto state = client->GetTaskState();
	if (!state) {
		return;
	}

	// task
	if (state->m_active_task.task_id != TASKSLOTEMPTY) {
		SendActiveTaskToClient(&state->m_active_task, client, 0, task_complete);
	}

	// shared task
	if (state->m_active_shared_task.task_id != TASKSLOTEMPTY) {
		SendActiveTaskToClient(&state->m_active_shared_task, client, 0, task_complete);
	}

	// quests
	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++) {
		int task_id = state->m_active_quests[task_index].task_id;
		if ((task_id == 0) || (m_task_data[task_id] == nullptr)) {
			continue;
		}

		LogTasksDetail("--");
		LogTasksDetail("[SendActiveTasksToClient] Task [{}]", m_task_data[task_id]->title);

		SendActiveTaskToClient(&state->m_active_quests[task_index], client, task_index, task_complete);
	}
}

void TaskManager::SendSingleActiveTaskToClient(
	Client *client, ClientTaskInformation &task_info, bool task_complete,
	bool bring_up_task_journal
)
{
	int task_id = task_info.task_id;
	if (task_id == 0 || m_task_data[task_id] == nullptr) {
		return;
	}

	int start_time = task_info.accepted_time;
	SendActiveTaskDescription(
		client,
		task_id,
		task_info,
		start_time,
		m_task_data[task_id]->duration,
		bring_up_task_journal
	);
	Log(Logs::General,
		Logs::Tasks,
		"[UPDATE] SendSingleActiveTasksToClient: Task %i, Activities: %i",
		task_id,
		GetActivityCount(task_id));

	for (int activity_id = 0; activity_id < GetActivityCount(task_id); activity_id++) {
		if (task_info.activity[activity_id].activity_state != ActivityHidden) {
			LogTasks("[SendSingleActiveTaskToClient] Long [{}] [{}] complete [{}]",
					 task_id,
					 activity_id,
					 task_complete);
			if (activity_id == GetActivityCount(task_id) - 1) {
				SendTaskActivityLong(client, task_id, activity_id, task_info.slot, task_complete);
			}
			else {
				SendTaskActivityLong(client, task_id, activity_id, task_info.slot, 0);
			}
		}
		else {
			LogTasks("[SendSingleActiveTaskToClient] Short [{}] [{}]", task_id, activity_id);
			SendTaskActivityShort(client, task_id, activity_id, task_info.slot);
		}
	}
}

void TaskManager::SendActiveTaskDescription(
	Client *client,
	int task_id,
	ClientTaskInformation &task_info,
	int start_time,
	int duration,
	bool bring_up_task_journal
)
{
	if ((task_id < 1) || (task_id >= MAXTASKS) || !m_task_data[task_id]) {
		return;
	}

	int packet_length = sizeof(TaskDescriptionHeader_Struct) + m_task_data[task_id]->title.length() + 1
						+ sizeof(TaskDescriptionData1_Struct) + m_task_data[task_id]->description.length() + 1
						+ sizeof(TaskDescriptionData2_Struct) + 1 + sizeof(TaskDescriptionTrailer_Struct);

	// If there is an item make the reward text into a link to the item (only the first item if a list
	// is specified). I have been unable to get multiple item links to work.
	//
	if (m_task_data[task_id]->reward_id && m_task_data[task_id]->item_link.empty()) {
		int item_id = 0;
		// If the reward is a list of items, and the first entry on the list is valid
		if (m_task_data[task_id]->reward_method == METHODSINGLEID) {
			item_id = m_task_data[task_id]->reward_id;
		}
		else if (m_task_data[task_id]->reward_method == METHODLIST) {
			item_id = m_goal_list_manager.GetFirstEntry(m_task_data[task_id]->reward_id);
			if (item_id < 0) {
				item_id = 0;
			}
		}

		if (item_id) {
			const EQ::ItemData *reward_item = database.GetItem(item_id);

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkItemData);
			linker.SetItemData(reward_item);
			linker.SetTaskUse();
			m_task_data[task_id]->item_link = linker.GenerateLink();
		}
	}

	packet_length += m_task_data[task_id]->reward.length() + 1 + m_task_data[task_id]->item_link.length() + 1;

	char                          *Ptr;
	TaskDescriptionHeader_Struct  *task_description_header;
	TaskDescriptionData1_Struct   *tdd1;
	TaskDescriptionData2_Struct   *tdd2;
	TaskDescriptionTrailer_Struct *tdt;

	auto outapp = new EQApplicationPacket(OP_TaskDescription, packet_length);

	task_description_header = (TaskDescriptionHeader_Struct *) outapp->pBuffer;

	task_description_header->SequenceNumber = task_info.slot;
	task_description_header->TaskID         = task_id;
	task_description_header->open_window    = bring_up_task_journal;
	task_description_header->task_type      = static_cast<uint32>(m_task_data[task_id]->type);

	constexpr uint32_t reward_radiant_type = 4; // Radiant Crystals, anything else is Ebon for shared tasks
	task_description_header->reward_type = m_task_data[task_id]->reward_radiant_crystals > 0 ? reward_radiant_type : 0;

	Ptr = (char *) task_description_header + sizeof(TaskDescriptionHeader_Struct);

	sprintf(Ptr, "%s", m_task_data[task_id]->title.c_str());
	Ptr += m_task_data[task_id]->title.length() + 1;

	tdd1 = (TaskDescriptionData1_Struct *) Ptr;

	tdd1->Duration = duration;
	tdd1->dur_code = static_cast<uint32>(m_task_data[task_id]->duration_code);

	tdd1->StartTime = start_time;

	Ptr = (char *) tdd1 + sizeof(TaskDescriptionData1_Struct);

	sprintf(Ptr, "%s", m_task_data[task_id]->description.c_str());
	Ptr += m_task_data[task_id]->description.length() + 1;

	tdd2 = (TaskDescriptionData2_Struct *) Ptr;

	// we have this reward stuff!
	// if we ever don't hardcode this, TaskDescriptionTrailer_Struct will need to be fixed since
	// "has_reward_selection" is after this bool! Smaller packet when this is 0
	tdd2->has_rewards = 1;

	tdd2->coin_reward    = m_task_data[task_id]->cash_reward;
	tdd2->xp_reward      = m_task_data[task_id]->experience_reward ? 1 : 0; // just booled
	tdd2->faction_reward = m_task_data[task_id]->faction_reward ? 1 : 0; // faction booled

	Ptr = (char *) tdd2 + sizeof(TaskDescriptionData2_Struct);

	// we actually have 2 strings here. One is max length 96 and not parsed for item links
	// We actually skipped past that string incorrectly before, so TODO: fix item link string
	sprintf(Ptr, "%s", m_task_data[task_id]->reward.c_str());
	Ptr += m_task_data[task_id]->reward.length() + 1;

	// second string is parsed for item links
	sprintf(Ptr, "%s", m_task_data[task_id]->item_link.c_str());
	Ptr += m_task_data[task_id]->item_link.length() + 1;

	tdt = (TaskDescriptionTrailer_Struct *) Ptr;
	// shared tasks show radiant/ebon crystal reward, non-shared tasks show generic points
	tdt->Points               = m_task_data[task_id]->reward_ebon_crystals;
	if (m_task_data[task_id]->reward_radiant_crystals > 0) {
		tdt->Points = m_task_data[task_id]->reward_radiant_crystals;
	}
	tdt->has_reward_selection = 0; // TODO: new rewards window

	client->QueuePacket(outapp);
	safe_delete(outapp);
}

bool TaskManager::LoadClientState(Client *client, ClientTaskState *client_task_state)
{
	if (!client || !client_task_state) {
		return false;
	}

	client->SetSharedTaskId(0);

	int character_id = client->CharacterID();

	client_task_state->m_active_task_count = 0;

	LogTasks("[LoadClientState] for character_id [{}]", character_id);

	// in a case where a client somehow lost local state with what state exists in world - we need
	// to perform an inverse sync where we inject the task
	SyncClientSharedTaskStateToLocal(client);

	auto character_tasks = CharacterTasksRepository::GetWhere(
		database,
		fmt::format("charid = {} ORDER BY acceptedtime", character_id)
	);

	for (auto &character_task: character_tasks) {
		int task_id = character_task.taskid;
		int slot    = character_task.slot;

		// this used to be loaded from character_tasks
		// this should just load from the tasks table
		auto type = task_manager->GetTaskType(character_task.taskid);

		if ((task_id < 0) || (task_id >= MAXTASKS)) {
			LogTasks(
				"[LoadClientState] Error: task_id [{}] out of range while loading character tasks from database",
				task_id
			);
			continue;
		}

		// client data bucket pointer
		// this actually fetches the proper task type instances to be loaded with data
		// whether it be quest / task / shared task
		auto task_info = client_task_state->GetClientTaskInfo(type, slot);
		if (task_info == nullptr) {
			LogTasks(
				"[LoadClientState] Error: slot [{}] out of range while loading character tasks from database",
				slot
			);
			continue;
		}

		if (task_info->task_id != TASKSLOTEMPTY) {
			LogTasks("[LoadClientState] Error: slot [{}] for task [{}] is already occupied", slot, task_id);
			continue;
		}

		task_info->task_id       = task_id;
		task_info->current_step  = -1;
		task_info->accepted_time = character_task.acceptedtime;
		task_info->updated       = false;

		for (auto &i : task_info->activity) {
			i.activity_id = -1;
		}

		// this check keeps a lot of core task updating code from working properly (shared or otherwise)
		if (type == TaskType::Quest) {
			++client_task_state->m_active_task_count;
		}

		LogTasks(
			"[LoadClientState] character_id [{}] task_id [{}] slot [{}] accepted_time [{}]",
			character_id,
			task_id,
			slot,
			character_task.acceptedtime
		);
	}

	// Load Activities
	LogTasks("[LoadClientState] Loading activities for character_id [{}]", character_id);

	auto character_activities = CharacterActivitiesRepository::GetWhere(
		database,
		fmt::format("charid = {} ORDER BY `taskid` ASC, `activityid` ASC", character_id)
	);

	for (auto &character_activity: character_activities) {
		int task_id = character_activity.taskid;
		if ((task_id < 0) || (task_id >= MAXTASKS)) {
			LogTasks(
				"[LoadClientState] Error: task_id [{}] out of range while loading character activities from database character_id [{}]",
				task_id,
				character_id
			);
			continue;
		}

		int activity_id = character_activity.activityid;
		if ((activity_id < 0) || (activity_id >= MAXACTIVITIESPERTASK)) {
			LogTasks(
				"[LoadClientState] Error: activity_id [{}] out of range while loading character activities from database character_id [{}]",
				activity_id,
				character_id
			);

			continue;
		}

		// type: task
		ClientTaskInformation *task_info = nullptr;
		if (client_task_state->m_active_task.task_id == task_id) {
			task_info = &client_task_state->m_active_task;
		}

		// type: shared task
		if (client_task_state->m_active_shared_task.task_id == task_id) {
			task_info = &client_task_state->m_active_shared_task;
		}

		// type: quest
		if (task_info == nullptr) {
			for (auto &active_quest : client_task_state->m_active_quests) {
				if (active_quest.task_id == task_id) {
					task_info = &active_quest;
				}
			}
		}

		if (task_info == nullptr) {
			LogTasks(
				"[LoadClientState] Error: activity_id [{}] found for task_id [{}] which client does not have character_id [{}]",
				activity_id,
				task_id,
				character_id
			);

			continue;
		}

		task_info->activity[activity_id].activity_id = activity_id;
		task_info->activity[activity_id].done_count  = character_activity.donecount;
		if (character_activity.completed) {
			task_info->activity[activity_id].activity_state = ActivityCompleted;
		}
		else {
			task_info->activity[activity_id].activity_state = ActivityHidden;
		}

		task_info->activity[activity_id].updated = false;

		LogTasks(
			"[LoadClientState] character_id [{}] task_id [{}] activity_id [{}] done_count [{}] completed [{}]",
			character_id,
			task_id,
			activity_id,
			character_activity.donecount,
			character_activity.completed
		);
	}

	SyncClientSharedTaskState(client, client_task_state);

	if (RuleB(TaskSystem, RecordCompletedTasks)) {
		CompletedTaskInformation completed_task_information{};

		for (bool &i : completed_task_information.activity_done) {
			i = false;
		}

		int previous_task_id        = -1;
		int previous_completed_time = -1;

		auto character_completed_tasks = CompletedTasksRepository::GetWhere(
			database,
			fmt::format("charid = {}  ORDER BY completedtime, taskid, activityid", character_id)
		);

		for (auto &character_completed_task: character_completed_tasks) {
			int task_id = character_completed_task.taskid;
			if ((task_id <= 0) || (task_id >= MAXTASKS)) {
				LogError("[TASKS]Task ID [{}] out of range while loading completed tasks from database", task_id);
				continue;
			}

			// An activity_id of -1 means mark all the none optional activities in the
			// task as complete. If the Rule to record optional activities is enabled,
			// subsequent records for this task will flag any optional tasks that were
			// completed.
			int activity_id = character_completed_task.activityid;
			if ((activity_id < -1) || (activity_id >= MAXACTIVITIESPERTASK)) {
				LogError("[TASKS]activity_information ID [{}] out of range while loading completed tasks from database",
						 activity_id);
				continue;
			}

			int completed_time = character_completed_task.completedtime;
			if ((previous_task_id != -1) &&
				((task_id != previous_task_id) || (completed_time != previous_completed_time))) {
				client_task_state->m_completed_tasks.push_back(completed_task_information);
				for (bool &activity_done : completed_task_information.activity_done) {
					activity_done = false;
				}
			}

			completed_task_information.task_id        = previous_task_id        = task_id;
			completed_task_information.completed_time = previous_completed_time = completed_time;

			// If activity_id is -1, Mark all the non-optional tasks as completed.
			if (activity_id < 0) {
				TaskInformation *task = m_task_data[task_id];
				if (task == nullptr) {
					continue;
				}

				for (int i = 0; i < task->activity_count; i++) {
					if (!task->activity_information[i].optional) {
						completed_task_information.activity_done[i] = true;
					}
				}
			}
			else {
				completed_task_information.activity_done[activity_id] = true;
			}
		}

		if (previous_task_id != -1) {
			client_task_state->m_completed_tasks.push_back(completed_task_information);
		}

		client_task_state->m_last_completed_task_loaded = client_task_state->m_completed_tasks.size();
	}

	std::string query = StringFormat(
		"SELECT `taskid` FROM character_enabledtasks "
		"WHERE `charid` = %i AND `taskid` >0 AND `taskid` < %i "
		"ORDER BY `taskid` ASC",
		character_id, MAXTASKS
	);

	auto results = database.QueryDatabase(query);
	if (results.Success()) {
		for (auto row = results.begin(); row != results.end(); ++row) {
			int task_id = atoi(row[0]);
			client_task_state->m_enabled_tasks.push_back(task_id);
			LogTasksDetail("[LoadClientState] Adding task_id [{}] to enabled tasks", task_id);
		}
	}

	// Check that there is an entry in the client task state for every activity_information in each task
	// This should only break if a ServerOP adds or deletes activites for a task that players already
	// have active, or due to a bug.
	for (int task_index = 0; task_index < MAXACTIVEQUESTS + 1; task_index++) {
		int task_id = client_task_state->m_active_tasks[task_index].task_id;
		if (task_id == TASKSLOTEMPTY) {
			continue;
		}
		if (!m_task_data[task_id]) {
			client->Message(
				Chat::Red,
				"Active Task Slot %i, references a task (%i), that does not exist. "
				"Removing from memory. Contact a GM to resolve this.",
				task_index,
				task_id
			);

			LogError("[LoadClientState] Character [{}] has task [{}] which does not exist", character_id, task_id);
			client_task_state->m_active_tasks[task_index].task_id = TASKSLOTEMPTY;
			continue;
		}
		for (int activity_index = 0; activity_index < m_task_data[task_id]->activity_count; activity_index++) {
			if (client_task_state->m_active_tasks[task_index].activity[activity_index].activity_id != activity_index) {
				client->Message(
					Chat::Red,
					"Active Task %i, %s. activity_information count does not match expected value."
					"Removing from memory. Contact a GM to resolve this.",
					task_id, m_task_data[task_id]->title.c_str()
				);

				LogTasks(
					"[LoadClientState] Fatal error in character [{}] task state. activity_information [{}] for Task [{}] either missing from client state or from task",
					character_id,
					activity_index,
					task_id
				);
				client_task_state->m_active_tasks[task_index].task_id = TASKSLOTEMPTY;
				break;
			}
		}
	}

	LogTasksDetail(
		"[LoadClientState] m_active_task task_id is [{}] slot [{}]",
		client_task_state->m_active_task.task_id,
		client_task_state->m_active_task.slot
	);
	if (client_task_state->m_active_task.task_id != TASKSLOTEMPTY) {
		client_task_state->UnlockActivities(character_id, client_task_state->m_active_task);

		// purely debugging
		LogTasksDetail(
			"[LoadClientState] Fetching task info for character_id [{}] task [{}] slot [{}] current_step [{}] accepted_time [{}] updated [{}]",
			character_id,
			client_task_state->m_active_task.task_id,
			client_task_state->m_active_task.slot,
			client_task_state->m_active_task.current_step,
			client_task_state->m_active_task.accepted_time,
			client_task_state->m_active_task.updated
		);

		TaskInformation *p_task_data = task_manager->m_task_data[client_task_state->m_active_task.task_id];
		if (p_task_data != nullptr) {
			for (int i = 0; i < p_task_data->activity_count; i++) {
				if (client_task_state->m_active_task.activity[i].activity_id >= 0) {
					LogTasksDetail(
						"[LoadClientState] -- character_id [{}] task [{}] activity_id [{}] done_count [{}] activity_state [{}] updated [{}] sequence [{}]",
						character_id,
						client_task_state->m_active_task.task_id,
						client_task_state->m_active_task.activity[i].activity_id,
						client_task_state->m_active_task.activity[i].done_count,
						client_task_state->m_active_task.activity[i].activity_state,
						client_task_state->m_active_task.activity[i].updated,
						p_task_data->sequence_mode
					);
				}
			}
		}
	}

	// shared task
	LogTasksDetail(
		"[LoadClientState] m_active_shared_task task_id is [{}] slot [{}]",
		client_task_state->m_active_shared_task.task_id,
		client_task_state->m_active_shared_task.slot
	);
	if (client_task_state->m_active_shared_task.task_id != TASKSLOTEMPTY) {
		client_task_state->UnlockActivities(character_id, client_task_state->m_active_shared_task);
	}

	// quests (max 20 or 40 depending on client)
	for (auto &active_quest : client_task_state->m_active_quests) {
		if (active_quest.task_id != TASKSLOTEMPTY) {
			client_task_state->UnlockActivities(character_id, active_quest);
		}
	}

	LogTasksDetail("[LoadClientState] for Character ID [{}] DONE!", character_id);
	LogTasksDetail("---", character_id);

	return true;
}

void TaskManager::SyncClientSharedTaskState(Client *c, ClientTaskState *cts)
{
	LogTasksDetail(
		"[SyncClientSharedTaskState] Syncing client shared task state"
	);

	SyncClientSharedTaskWithPersistedState(c, cts);
	SyncClientSharedTaskRemoveLocalIfNotExists(c, cts);
}

void TaskManager::SyncClientSharedTaskWithPersistedState(Client *c, ClientTaskState *cts)
{
	auto character_tasks = CharacterTasksRepository::GetWhere(
		database,
		fmt::format("charid = {} ORDER BY acceptedtime", c->CharacterID())
	);

	for (auto &character_task: character_tasks) {
		if (character_task.type == TASK_TYPE_SHARED) {
			auto st = SharedTaskMembersRepository::GetWhere(
				database,
				fmt::format(
					"character_id = {}",
					c->CharacterID()
				)
			);

			if (!st.empty()) {
				int64 shared_task_id = st[0].shared_task_id;
				auto  activities     = SharedTaskActivityStateRepository::GetWhere(
					database,
					fmt::format(
						"shared_task_id = {}",
						shared_task_id
					)
				);

				ClientTaskInformation *shared_task = nullptr;
				shared_task = &cts->m_active_shared_task;

				// has active shared task
				if (cts->HasActiveSharedTask()) {

					LogTasksDetail(
						"[SyncClientSharedTaskWithPersistedState] Client [{}] has shared_task, sync with database",
						c->GetCleanName()
					);

					bool      fell_behind_state = false;
					for (auto &a: activities) {

						LogTasksDetail(
							"[LoadClientState] shared_task loop local [{}] shared [{}]",
							shared_task->activity[a.activity_id].done_count,
							a.done_count
						);

						// we're behind shared task state, update self
						if (shared_task->activity[a.activity_id].done_count < a.done_count) {

							// update done count
							shared_task->activity[a.activity_id].done_count = a.done_count;

							// activity state
							shared_task->activity[a.activity_id].activity_state =
								(a.completed_time > 0 ? ActivityCompleted : ActivityHidden);

							// set flag to persist later
							fell_behind_state = true;
						}
					}

					// fell behind, force a save of client state
					if (fell_behind_state) {
						SaveClientState(c, cts);
					}

					c->SetSharedTaskId(shared_task_id);
				}
			}
		}
	}
}

void TaskManager::SyncClientSharedTaskRemoveLocalIfNotExists(Client *c, ClientTaskState *cts)
{
	// has active shared task
	if (cts->HasActiveSharedTask()) {
		auto members = SharedTaskMembersRepository::GetWhere(
			database,
			fmt::format(
				"character_id = {}",
				c->CharacterID()
			)
		);

		// if we don't actually have a membership anywhere, remove ourself locally
		if (members.empty()) {
			LogTasksDetail(
				"[SyncClientSharedTaskRemoveLocalIfNotExists] Client [{}] Shared task [{}] doesn't exist in world, removing from local",
				c->GetCleanName(),
				cts->m_active_shared_task.task_id
			);

			std::string delete_where = fmt::format(
				"charid = {} and taskid = {}",
				c->CharacterID(),
				cts->m_active_shared_task.task_id
			);
			CharacterTasksRepository::DeleteWhere(database, delete_where);
			CharacterActivitiesRepository::DeleteWhere(database, delete_where);

			c->MessageString(Chat::Yellow, SharedTaskMessage::YOU_ARE_NO_LONGER_A_MEMBER,
				m_task_data[cts->m_active_shared_task.task_id]->title.c_str());

			// remove as active task if doesn't exist
			cts->m_active_shared_task = {};

			// persist removal from local record
			SaveClientState(c, cts);
		}
	}
}

// in a case where a client somehow lost local state with what state exists in world - we need
// to perform an inverse sync where we inject the task
void TaskManager::SyncClientSharedTaskStateToLocal(
	Client *c
)
{
	auto character_tasks = CharacterTasksRepository::GetWhere(
		database,
		fmt::format("charid = {} ORDER BY acceptedtime", c->CharacterID())
	);

	bool has_character_shared_task = false;

	for (auto &character_task: character_tasks) {
		if (character_task.type == TASK_TYPE_SHARED) {
			has_character_shared_task = true;
		}
	}

	if (!has_character_shared_task) {
		LogTasksDetail("[SyncClientSharedTaskStateToLocal] We don't have a shared character task locally");
		auto stm = SharedTaskMembersRepository::GetWhere(
			database,
			fmt::format(
				"character_id = {}",
				c->CharacterID()
			)
		);

		if (!stm.empty()) {
			LogTasksDetail("[SyncClientSharedTaskStateToLocal] We have membership in database");
			auto s = SharedTasksRepository::FindOne(
				database,
				(int) stm.front().shared_task_id
			);

			if (s.id > 0) {
				LogTasksDetail("[SyncClientSharedTaskStateToLocal] Creating entity");

				// create task locally
				auto ct = CharacterTasksRepository::NewEntity();
				ct.charid       = (int) c->CharacterID();
				ct.acceptedtime = (int) s.accepted_time;
				ct.taskid       = (int) s.task_id;
				ct.slot         = 0;
				ct.type         = TASK_TYPE_SHARED;
				character_tasks.emplace_back(ct);
				CharacterTasksRepository::InsertOne(database, ct);

				// create activities locally
				auto activities = SharedTaskActivityStateRepository::GetWhere(
					database,
					fmt::format(
						"shared_task_id = {}",
						(int) stm.front().shared_task_id
					)
				);

				std::vector<CharacterActivitiesRepository::CharacterActivities> character_activities = {};

				for (auto &a: activities) {
					auto ca = CharacterActivitiesRepository::NewEntity();
					ca.completed  = a.completed_time > 0;
					ca.charid     = (int) c->CharacterID();
					ca.donecount  = a.done_count;
					ca.taskid     = s.task_id;
					ca.activityid = a.activity_id;
					character_activities.emplace_back(ca);
				}
				CharacterActivitiesRepository::InsertMany(database, character_activities);
			}
		}
	}
}

void TaskManager::HandleUpdateTasksOnKill(Client *client, uint32 npc_type_id, std::string npc_name)
{
	for (auto &c: client->GetPartyMembers()) {
		if (!c->ClientDataLoaded() || !c->HasTaskState()) {
			continue;
		}

		LogTasksDetail("[HandleUpdateTasksOnKill] Looping through client [{}]", c->GetCleanName());

		// loop over the union of tasks and quests
		for (auto &active_task : c->GetTaskState()->m_active_tasks) {
			auto current_task = &active_task;
			if (current_task->task_id == TASKSLOTEMPTY) {
				continue;
			}

			// Check if there are any active kill activities for this p_task_data
			auto p_task_data = m_task_data[current_task->task_id];
			if (p_task_data == nullptr) {
				return;
			}

			for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
				ClientActivityInformation *client_activity = &current_task->activity[activity_id];
				ActivityInformation       *activity_info   = &p_task_data->activity_information[activity_id];

				// We are not interested in completed or hidden activities
				if (client_activity->activity_state != ActivityActive) {
					continue;
				}

				// We are only interested in Kill activities
				if (activity_info->activity_type != TaskActivityType::Kill) {
					continue;
				}

				// Is there a zone restriction on the activity_information ?
				if (!activity_info->CheckZone(zone->GetZoneID())) {
					LogTasks(
						"[HandleUpdateTasksOnKill] character [{}] task_id [{}] activity_id [{}] activity_type [{}] for NPC [{}] failed zone check",
						client->GetName(),
						current_task->task_id,
						activity_id,
						static_cast<int32_t>(TaskActivityType::Kill),
						npc_type_id
					);
					continue;
				}
				// Is the activity_information to kill this type of NPC ?
				switch (activity_info->goal_method) {
					case METHODSINGLEID:
						if (activity_info->goal_id != npc_type_id) {
							LogTasksDetail("[HandleUpdateTasksOnKill] Matched single goal");
							continue;
						}
						break;

					case METHODLIST:
						if (!m_goal_list_manager.IsInList(
							activity_info->goal_id,
							(int) npc_type_id
						) && !TaskGoalListManager::IsInMatchList(
							activity_info->goal_match_list,
							std::to_string(npc_type_id)
						) && !TaskGoalListManager::IsInMatchListPartial(
							activity_info->goal_match_list,
							npc_name
						)) {
							LogTasksDetail("[HandleUpdateTasksOnKill] Matched list goal");
							continue;
						}
						break;

					default:
						// If METHODQUEST, don't updated the activity_information here
						continue;
				}

				LogTasksDetail("[HandleUpdateTasksOnKill] passed checks");

				// handle actual update
				// legacy eqemu task update logic loops through group on kill of npc to update a single task
				if (p_task_data->type != TaskType::Shared) {
					LogTasksDetail("[HandleUpdateTasksOnKill] Non-Shared Update");
					c->GetTaskState()->IncrementDoneCount(c, p_task_data, current_task->slot, activity_id);
					continue;
				}

				LogTasksDetail("[HandleUpdateTasksOnKill] Shared update");

				// shared tasks only require one client to receive an update to propagate
				if (c == client) {
					c->GetTaskState()->IncrementDoneCount(c, p_task_data, current_task->slot, activity_id);
				}
			}
		}
	}
}
