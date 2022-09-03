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
			"`id` > 0 AND `id` < {} AND `taskid` >= 0 ORDER BY `id`, `taskid` ASC",
			MAXTASKSETS
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

		task_query_filter = fmt::format("id > 0");
	}

	// load task level data
	auto repo_tasks = TasksRepository::GetWhere(content_db, task_query_filter);
	m_task_data.reserve(repo_tasks.size());

	for (auto &task: repo_tasks) {
		int task_id = task.id;

		if (task_id <= 0) {
			// This shouldn't happen, as the SELECT is bounded by MAXTASKS
			LogError("[TASKS]Task ID [{}] out of range while loading tasks from database", task_id);
			continue;
		}

		// load task data
		TaskInformation task_data{};
		task_data.type                    = static_cast<TaskType>(task.type);
		task_data.duration                = task.duration;
		task_data.duration_code           = static_cast<DurationCode>(task.duration_code);
		task_data.title                   = task.title;
		task_data.description             = task.description;
		task_data.reward                  = task.reward;
		task_data.reward_id               = task.rewardid;
		task_data.cash_reward             = task.cashreward;
		task_data.experience_reward       = task.xpreward;
		task_data.reward_method           = (TaskMethodType) task.rewardmethod;
		task_data.reward_points           = task.reward_points;
		task_data.reward_point_type       = static_cast<AltCurrencyType>(task.reward_point_type);
		task_data.faction_reward          = task.faction_reward;
		task_data.faction_amount          = task.faction_amount;
		task_data.min_level               = task.minlevel;
		task_data.max_level               = task.maxlevel;
		task_data.level_spread            = task.level_spread;
		task_data.min_players             = task.min_players;
		task_data.max_players             = task.max_players;
		task_data.repeatable              = task.repeatable;
		task_data.completion_emote        = task.completion_emote;
		task_data.replay_timer_group      = task.replay_timer_group;
		task_data.replay_timer_seconds    = task.replay_timer_seconds;
		task_data.request_timer_group     = task.request_timer_group;
		task_data.request_timer_seconds   = task.request_timer_seconds;
		task_data.activity_count          = 0;

		m_task_data.try_emplace(task_id, std::move(task_data));

		LogTasksDetail(
			"[LoadTasks] (Task) task_id [{}] type [{}] () duration [{}] duration_code [{}] title [{}] description [{}] "
			" reward [{}] rewardid [{}] cashreward [{}] xpreward [{}] rewardmethod [{}] faction_reward [{}] minlevel [{}] "
			" maxlevel [{}] level_spread [{}] min_players [{}] max_players [{}] repeatable [{}] completion_emote [{}]",
			" replay_group [{}] replay_timer_seconds [{}] request_group [{}] request_timer_seconds [{}]",
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
			task.replay_timer_group,
			task.replay_timer_seconds,
			task.request_timer_group,
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
			"activityid < {} ORDER BY taskid, activityid ASC",
			MAXACTIVITIESPERTASK
		);
	}

	// load activity data
	auto      task_activities = TaskActivitiesRepository::GetWhere(content_db, activities_query_filter);
	for (auto &task_activity: task_activities) {
		int task_id     = task_activity.taskid;
		int step        = task_activity.step;
		int activity_id = task_activity.activityid;

		if (task_id <= 0 || activity_id < 0 || activity_id >= MAXACTIVITIESPERTASK) {

			// This shouldn't happen, as the SELECT is bounded by MAXTASKS
			LogTasks(
				"[LoadTasks] Error: Task or activity_information ID ([{}], [{}]) out of range while loading activities from database",
				task_id,
				activity_id
			);
			continue;
		}

		auto task_data = GetTaskData(task_id);
		if (!task_data) {
			LogTasks(
				"[LoadTasks] Error: activity_information for non-existent task ([{}], [{}]) while loading activities from database",
				task_id,
				activity_id
			);
			continue;
		}

		// create pointer to activity data since declarations get unruly long
		int                 activity_index = task_data->activity_count;
		ActivityInformation *activity_data = &task_data->activity_information[activity_index];

		// Task Activities MUST be numbered sequentially from 0. If not, log an error
		// and set the task to nullptr. Subsequent activities for this task will raise
		// ERR_NOTASK errors.
		// Change to (activityID != (Tasks[taskID]->activity_count + 1)) to index from 1
		if (activity_id != task_data->activity_count) {
			LogTasks(
				"[LoadTasks] Error: Activities for Task [{}] (activity_id [{}]) are not sequential starting at 0. Not loading task ",
				task_id,
				activity_id
			);
			m_task_data.erase(task_id);
			continue;
		}

		// set activity data
		activity_data->req_activity_id      = task_activity.req_activity_id;
		activity_data->step                 = step;
		activity_data->activity_type        = static_cast<TaskActivityType>(task_activity.activitytype);
		activity_data->target_name          = task_activity.target_name;
		activity_data->item_list            = task_activity.item_list;
		activity_data->skill_list           = task_activity.skill_list;
		activity_data->skill_id             = Strings::IsNumber(task_activity.skill_list) ? std::stoi(task_activity.skill_list) : 0; // for older clients
		activity_data->spell_list           = task_activity.spell_list;
		activity_data->spell_id             = Strings::IsNumber(task_activity.spell_list) ? std::stoi(task_activity.spell_list) : 0; // for older clients
		activity_data->description_override = task_activity.description_override;
		activity_data->npc_id               = task_activity.npc_id;
		activity_data->npc_goal_id          = task_activity.npc_goal_id;
		activity_data->npc_match_list       = task_activity.npc_match_list;
		activity_data->item_id              = task_activity.item_id;
		activity_data->item_goal_id         = task_activity.item_goal_id;
		activity_data->item_id_list         = task_activity.item_id_list;
		activity_data->dz_switch_id         = task_activity.dz_switch_id;
		activity_data->goal_method          = (TaskMethodType) task_activity.goalmethod;
		activity_data->goal_count           = task_activity.goalcount;
		activity_data->min_x                = task_activity.min_x;
		activity_data->min_y                = task_activity.min_y;
		activity_data->min_z                = task_activity.min_z;
		activity_data->max_x                = task_activity.max_x;
		activity_data->max_y                = task_activity.max_y;
		activity_data->max_z                = task_activity.max_z;
		activity_data->zone_version         = task_activity.zone_version >= 0 ? task_activity.zone_version : -1;
		activity_data->has_area             = false;

		if (std::abs(task_activity.max_x - task_activity.min_x) > 0.0f &&
		    std::abs(task_activity.max_y - task_activity.min_y) > 0.0f &&
		    std::abs(task_activity.max_z - task_activity.min_z) > 0.0f)
		{
			activity_data->has_area = true;
		}

		// zones
		activity_data->zones = task_activity.zones;
		auto zones = Strings::Split(
			task_activity.zones,
			';'
		);

		for (auto &&e : zones) {
			activity_data->zone_ids.push_back(std::stoi(e));
		}

		activity_data->optional = task_activity.optional;

		LogTasksDetail(
			"[LoadTasks] (Activity) task_id [{}] activity_id [{}] slot [{}] activity_type [{}] goal_method [{}] goal_count [{}] zones [{}]"
			" target_name [{}] item_list [{}] skill_list [{}] spell_list [{}] description_override [{}]",
			task_id,
			activity_id,
			task_data->activity_count,
			static_cast<int32_t>(activity_data->activity_type),
			activity_data->goal_method,
			activity_data->goal_count,
			activity_data->zones.c_str(),
			activity_data->target_name.c_str(),
			activity_data->item_list.c_str(),
			activity_data->skill_list.c_str(),
			activity_data->spell_list.c_str(),
			activity_data->description_override.c_str()
		);

		task_data->activity_count++;
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

			const auto task_data = GetTaskData(task_id);

			int slot = active_task.slot;
			if (active_task.updated) {

				LogTasks(
					"[SaveClientState] character_id [{}] updating task_index [{}] task_id [{}]",
					character_id,
					slot,
					task_id
				);

				std::string query = StringFormat(
					"REPLACE INTO character_tasks (charid, taskid, slot, type, acceptedtime, was_rewarded) "
					"VALUES (%i, %i, %i, %i, %i, %d)",
					character_id,
					task_id,
					slot,
					static_cast<int>(task_data->type),
					active_task.accepted_time,
					active_task.was_rewarded
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

			for (int activity_index = 0; activity_index < task_data->activity_count; ++activity_index) {
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
			for (int activity_index = 0; activity_index < task_data->activity_count; ++activity_index) {
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

		const auto task_data = GetTaskData(task_id);
		if (!task_data) {
			continue;
		}

		// we don't record completed shared tasks in the task quest log
		if (task_data->type == TaskType::Shared) {
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
		for (int activity_id = 0; activity_id < task_data->activity_count; activity_id++) {
			if (!task_data->activity_information[activity_id].optional ||
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
	const auto task_data = GetTaskData(task_id);
	if (!task_data) {
		return false;
	}

	if (task_data->min_level && (player_level < task_data->min_level)) {
		return false;
	}

	if (task_data->max_level && (player_level > task_data->max_level)) {
		return false;
	}

	return true;
}

std::string TaskManager::GetTaskName(uint32 task_id)
{
	if (task_id > 0) {
		const auto task_data = GetTaskData(task_id);
		if (task_data) {
			return task_data->title;
		}
	}

	return std::string();
}

TaskType TaskManager::GetTaskType(uint32 task_id)
{
	if (task_id > 0) {
		const auto task_data = GetTaskData(task_id);
		if (task_data) {
			return task_data->type;
		}
	}
	return TaskType::Task;
}

void TaskManager::TaskSetSelector(Client* client, Mob* mob, int task_set_id, bool ignore_cooldown)
{
	int player_level    = client->GetLevel();
	ClientTaskState* client_task_state = client->GetTaskState();

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
		const auto task_data = GetTaskData(task_id);
		if (task_data && task_data->type == TaskType::Shared) {
			SharedTaskSelector(client, mob, m_task_sets[task_set_id], ignore_cooldown);
			return;
		}
	}

	if (!ignore_cooldown && client->HasTaskRequestCooldownTimer()) {
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

	std::vector<int> task_list;
	while (iterator != m_task_sets[task_set_id].end() && task_list.size() < MAXCHOOSERENTRIES) {
		auto task = *iterator;
		const auto task_data = GetTaskData(task);

		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if ((all_enabled || client_task_state->IsTaskEnabled(task)) && ValidateLevel(task, player_level) &&
			!client_task_state->IsTaskActive(task) && client_task_state->HasSlotForTask(task_data) &&
			// this slot checking is a bit silly, but we allow mixing of task types ...
			(IsTaskRepeatable(task) || !client_task_state->IsTaskCompleted(task))) {
			task_list.push_back(task);
		}

		++iterator;
	}

	if (!task_list.empty()) {
		SendTaskSelector(client, mob, task_list);
	}
	else {
		client->MessageString(Chat::Yellow, NO_TASK_OFFERS, ".", ".", client->GetName());
	}
}

// unlike the non-Quest version of this function, it does not check enabled, that is assumed the responsibility of the quest to handle
// we do however still want it to check the other stuff like level, active, room, etc
void TaskManager::TaskQuestSetSelector(Client* client, Mob* mob, const std::vector<int>& tasks, bool ignore_cooldown)
{
	std::vector<int> task_list;
	int player_level    = client->GetLevel();
	ClientTaskState* client_task_state = client->GetTaskState();

	LogTasks("[UPDATE] TaskQuestSetSelector called with size [{}]", tasks.size());

	if (tasks.empty()) {
		return;
	}

	// live prevents mixing selection types (also uses diff opcodes for solo vs shared tasks)
	// to keep shared task validation live-like (and simple), any shared task will
	// forward this to shared task validation and non-shared tasks will be dropped
	for (int i = 0; i < tasks.size(); ++i) {
		auto task = tasks[i];
		const auto task_data = GetTaskData(task);
		if (task_data && task_data->type == TaskType::Shared) {
			SharedTaskSelector(client, mob, tasks, ignore_cooldown);
			return;
		}
	}

	if (!ignore_cooldown && client->HasTaskRequestCooldownTimer()) {
		client->SendTaskRequestCooldownTimerMessage();
		return;
	}

	for (int i = 0; i < tasks.size() && task_list.size() < MAXCHOOSERENTRIES; ++i) {
		auto task = tasks[i];
		const auto task_data = GetTaskData(task);
		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if (ValidateLevel(task, player_level) && !client_task_state->IsTaskActive(task) &&
			client_task_state->HasSlotForTask(task_data) &&
			// this slot checking is a bit silly, but we allow mixing of task types ...
			(IsTaskRepeatable(task) || !client_task_state->IsTaskCompleted(task))) {
			task_list.push_back(task);
		}
	}

	if (!task_list.empty()) {
		SendTaskSelector(client, mob, task_list);
	}
	else {
		client->MessageString(Chat::Yellow, NO_TASK_OFFERS, ".", ".", client->GetName());
	}
}

void TaskManager::SharedTaskSelector(Client* client, Mob* mob, const std::vector<int>& tasks, bool ignore_cooldown)
{
	LogTasks("[UPDATE] SharedTaskSelector called with size [{}]", tasks.size());

	if (tasks.empty()) {
		return;
	}

	if (!ignore_cooldown && client->HasTaskRequestCooldownTimer()) {
		client->SendTaskRequestCooldownTimerMessage();
		return;
	}

	// check if requester already has a shared task (no need to query group/raid members if so)
	if (client->GetTaskState()->HasActiveSharedTask()) {
		client->MessageString(Chat::Red, TaskStr::REQUEST_HAVE);
		return;
	}

	// get group/raid member character data from db (need to query for character ids)
	// todo: group/raids need refactored to avoid queries and ignore offline members (through world)
	auto request = SharedTask::GetRequestCharacters(database, client->CharacterID());

	// check if any group/raid member already has a shared task (already checked solo character)
	bool validation_failed = false;
	if (request.group_type != SharedTaskRequestGroupType::Solo) {
		auto shared_task_members = SharedTaskMembersRepository::GetWhere(
			database,
			fmt::format("character_id IN ({}) LIMIT 1", fmt::join(request.character_ids, ",")));

		if (!shared_task_members.empty()) {
			validation_failed = true;

			auto it = std::find_if(request.members.begin(), request.members.end(),
				[&](const SharedTaskMember& member) {
					return member.character_id == shared_task_members.front().character_id;
				});

			if (it != request.members.end()) {
				if (request.group_type == SharedTaskRequestGroupType::Group) {
					client->MessageString(Chat::Red, TaskStr::REQUEST_GROUP_HAS, it->character_name.c_str());
				}
				else {
					client->MessageString(Chat::Red, TaskStr::REQUEST_RAID_HAS, it->character_name.c_str());
				}
			}
		}
	}

	if (!validation_failed) {
		// run type and level filters on task selections
		std::vector<int> task_list;

		for (int i = 0; i < tasks.size() && task_list.size() < MAXCHOOSERENTRIES; ++i) {
			// todo: are there non repeatable shared tasks? (would need to check all group/raid members)
			auto task = tasks[i];
			const auto task_data = GetTaskData(task);
			if (task_data &&
			    task_data->type == TaskType::Shared &&
			    request.lowest_level >= task_data->min_level &&
			    (task_data->max_level == 0 || request.highest_level <= task_data->max_level))
			{
				task_list.push_back(task);
			}
		}

		// check if any tasks are left to offer after filtering
		if (!task_list.empty()) {
			SendSharedTaskSelector(client, mob, task_list);
		}
		else {
			client->MessageString(Chat::Red, TaskStr::NOT_MEET_REQ);
		}
	}
}

// sends task selector to client
void TaskManager::SendTaskSelector(Client* client, Mob* mob, const std::vector<int>& task_list)
{
	LogTasks("TaskSelector for [{}] Tasks", task_list.size());
	int player_level = client->GetLevel();
	client->GetTaskState()->ClearLastOffers();

	int      valid_tasks_count = 0;
	for (int task_index = 0; task_index < task_list.size(); task_index++) {
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

	for (int i = 0; i < task_list.size(); i++) { // max 40
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
		m_task_data[task_list[i]].SerializeSelector(buf, client->ClientVersion());
		client->GetTaskState()->AddOffer(task_list[i], mob->GetID());
	}

	auto outapp = std::make_unique<EQApplicationPacket>(OP_TaskSelectWindow, buf);
	client->QueuePacket(outapp.get());
}

void TaskManager::SendSharedTaskSelector(Client* client, Mob* mob, const std::vector<int>& task_list)
{
	LogTasks("SendSharedTaskSelector for [{}] Tasks", task_list.size());

	// request timer is only set when shared task selection shown (not for failed validations)
	client->StartTaskRequestCooldownTimer();
	client->GetTaskState()->ClearLastOffers();

	SerializeBuffer buf;

	buf.WriteUInt32(static_cast<uint32_t>(task_list.size())); // number of tasks
	// shared task selection (live doesn't mix types) makes client send shared task specific opcode for accepts
	buf.WriteUInt32(static_cast<uint32_t>(TaskType::Shared));
	buf.WriteUInt32(mob->GetID()); // task giver entity id

	for (int i = 0; i < task_list.size(); ++i) {
		int task_id = task_list[i];
		buf.WriteUInt32(task_id);
		m_task_data[task_id].SerializeSelector(buf, client->ClientVersion());
		client->GetTaskState()->AddOffer(task_id, mob->GetID());
	}

	auto outapp = std::make_unique<EQApplicationPacket>(OP_SharedTaskSelectWindow, buf);
	client->QueuePacket(outapp.get());
}

int TaskManager::GetActivityCount(int task_id)
{
	if (task_id > 0) {
		const auto task_data = GetTaskData(task_id);
		if (task_data) {
			return task_data->activity_count;
		}
	}

	return 0;
}

bool TaskManager::IsTaskRepeatable(int task_id)
{
	const auto task_data = GetTaskData(task_id);
	if (!task_data) {
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
		int task_id = client_task_state->m_completed_tasks[i].task_id;
		const auto task_data = GetTaskData(task_id);
		if (!task_data) { continue; }
		packet_length = packet_length + 8 + task_data->title.size() + 1;
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
		const auto task_data = GetTaskData(task_id);
		if (!task_data) { continue; }
		*(uint32 *) buf = task_id;
		buf = buf + 4;

		sprintf(buf, "%s", task_data->title.c_str());
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
	const auto task_data = GetTaskData(task_id);

	auto outapp = std::make_unique<EQApplicationPacket>(OP_TaskActivity, 25);
	outapp->WriteUInt32(client_task_index);
	outapp->WriteUInt32(static_cast<uint32>(task_data->type));
	outapp->WriteUInt32(task_id);
	outapp->WriteUInt32(activity_id);
	outapp->WriteUInt32(0);
	outapp->WriteUInt32(0xffffffff);
	outapp->WriteUInt8(task_data->activity_information[activity_id].optional ? 1 : 0);
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
	const auto task_data = GetTaskData(task_id);

	SerializeBuffer buf(100);

	buf.WriteUInt32(client_task_index);    // TaskSequenceNumber
	buf.WriteUInt32(static_cast<uint32>(task_data->type)); // task type
	buf.WriteUInt32(task_id);
	buf.WriteUInt32(activity_id);
	buf.WriteUInt32(0);        // unknown3

	const auto& activity = task_data->activity_information[activity_id];
	int done_count = client->GetTaskActivityDoneCount(task_data->type, client_task_index, activity_id);

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

	const auto task_data = GetTaskData(task->task_id);

	int  start_time    = task->accepted_time;
	int  task_id       = task->task_id;
	auto task_type     = task_data->type;
	auto task_duration = task_data->duration;

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
		const auto task_data = GetTaskData(task_id);
		if (!task_data) {
			continue;
		}

		LogTasksDetail("--");
		LogTasksDetail("[SendActiveTasksToClient] Task [{}]", task_data->title);

		SendActiveTaskToClient(&state->m_active_quests[task_index], client, task_index, task_complete);
	}
}

void TaskManager::SendSingleActiveTaskToClient(
	Client *client, ClientTaskInformation &task_info, bool task_complete,
	bool bring_up_task_journal
)
{
	int task_id = task_info.task_id;
	const auto task_data = GetTaskData(task_id);
	if (!task_data) {
		return;
	}

	int start_time = task_info.accepted_time;
	SendActiveTaskDescription(
		client,
		task_id,
		task_info,
		start_time,
		task_data->duration,
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
	auto task_data = GetTaskData(task_id);
	if (!task_data) {
		return;
	}

	int packet_length = sizeof(TaskDescriptionHeader_Struct) + task_data->title.length() + 1
						+ sizeof(TaskDescriptionData1_Struct) + task_data->description.length() + 1
						+ sizeof(TaskDescriptionData2_Struct) + 1 + sizeof(TaskDescriptionTrailer_Struct);

	// If there is an item make the reward text into a link to the item (only the first item if a list
	// is specified). I have been unable to get multiple item links to work.
	//
	if (task_data->reward_id && task_data->item_link.empty()) {
		int item_id = 0;
		// If the reward is a list of items, and the first entry on the list is valid
		if (task_data->reward_method == METHODSINGLEID) {
			item_id = task_data->reward_id;
		}
		else if (task_data->reward_method == METHODLIST) {
			item_id = m_goal_list_manager.GetFirstEntry(task_data->reward_id);
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
			task_data->item_link = linker.GenerateLink();
		}
	}

	packet_length += task_data->reward.length() + 1 + task_data->item_link.length() + 1;

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
	task_description_header->task_type      = static_cast<uint32>(task_data->type);

	task_description_header->reward_type = static_cast<int>(task_data->reward_point_type);

	Ptr = (char *) task_description_header + sizeof(TaskDescriptionHeader_Struct);

	sprintf(Ptr, "%s", task_data->title.c_str());
	Ptr += task_data->title.length() + 1;

	tdd1 = (TaskDescriptionData1_Struct *) Ptr;

	tdd1->Duration = duration;
	tdd1->dur_code = static_cast<uint32>(task_data->duration_code);

	tdd1->StartTime = start_time;

	Ptr = (char *) tdd1 + sizeof(TaskDescriptionData1_Struct);

	sprintf(Ptr, "%s", task_data->description.c_str());
	Ptr += task_data->description.length() + 1;

	tdd2 = (TaskDescriptionData2_Struct *) Ptr;

	// we have this reward stuff!
	// if we ever don't hardcode this, TaskDescriptionTrailer_Struct will need to be fixed since
	// "has_reward_selection" is after this bool! Smaller packet when this is 0
	tdd2->has_rewards = 1;

	tdd2->coin_reward    = task_data->cash_reward;
	tdd2->xp_reward      = task_data->experience_reward ? 1 : 0; // just booled
	tdd2->faction_reward = task_data->faction_reward ? 1 : 0; // faction booled

	Ptr = (char *) tdd2 + sizeof(TaskDescriptionData2_Struct);

	// we actually have 2 strings here. One is max length 96 and not parsed for item links
	// We actually skipped past that string incorrectly before, so TODO: fix item link string
	sprintf(Ptr, "%s", task_data->reward.c_str());
	Ptr += task_data->reward.length() + 1;

	// second string is parsed for item links
	sprintf(Ptr, "%s", task_data->item_link.c_str());
	Ptr += task_data->item_link.length() + 1;

	tdt = (TaskDescriptionTrailer_Struct *) Ptr;
	// shared tasks show radiant/ebon crystal reward, non-shared tasks show generic points
	tdt->Points = task_data->reward_points;

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

		if (task_id < 0) {
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
		task_info->accepted_time = character_task.acceptedtime;
		task_info->updated       = false;
		task_info->was_rewarded  = character_task.was_rewarded;

		for (auto &i : task_info->activity) {
			i.activity_id = -1;
		}

		// this check keeps a lot of core task updating code from working properly (shared or otherwise)
		if (type == TaskType::Quest) {
			++client_task_state->m_active_task_count;
		}

		LogTasks(
			"[LoadClientState] character_id [{}] task_id [{}] slot [{}] accepted_time [{}] was_rewarded [{}]",
			character_id,
			task_id,
			slot,
			character_task.acceptedtime,
			character_task.was_rewarded
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
		if (task_id < 0) {
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
			if (task_id <= 0) {
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
				const auto task_data = GetTaskData(task_id);
				if (!task_data) {
					continue;
				}

				for (int i = 0; i < task_data->activity_count; i++) {
					if (!task_data->activity_information[i].optional) {
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
		"WHERE `charid` = %i AND `taskid` > 0 "
		"ORDER BY `taskid` ASC",
		character_id
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
		const auto task_data = GetTaskData(task_id);
		if (!task_data) {
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
		for (int activity_index = 0; activity_index < task_data->activity_count; activity_index++) {
			if (client_task_state->m_active_tasks[task_index].activity[activity_index].activity_id != activity_index) {
				client->Message(
					Chat::Red,
					"Active Task %i, %s. activity_information count does not match expected value."
					"Removing from memory. Contact a GM to resolve this.",
					task_id, task_data->title.c_str()
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
		client_task_state->UnlockActivities(client, client_task_state->m_active_task);

		// purely debugging
		LogTasksDetail(
			"[LoadClientState] Fetching task info for character_id [{}] task [{}] slot [{}] accepted_time [{}] updated [{}]",
			character_id,
			client_task_state->m_active_task.task_id,
			client_task_state->m_active_task.slot,
			client_task_state->m_active_task.accepted_time,
			client_task_state->m_active_task.updated
		);

		const auto task_data = GetTaskData(client_task_state->m_active_task.task_id);
		if (task_data) {
			for (int i = 0; i < task_data->activity_count; i++) {
				if (client_task_state->m_active_task.activity[i].activity_id >= 0) {
					LogTasksDetail(
						"[LoadClientState] -- character_id [{}] task [{}] activity_id [{}] done_count [{}] activity_state [{}] updated [{}]",
						character_id,
						client_task_state->m_active_task.task_id,
						client_task_state->m_active_task.activity[i].activity_id,
						client_task_state->m_active_task.activity[i].done_count,
						client_task_state->m_active_task.activity[i].activity_state,
						client_task_state->m_active_task.activity[i].updated
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
		client_task_state->UnlockActivities(client, client_task_state->m_active_shared_task);
	}

	// quests (max 20 or 40 depending on client)
	for (auto &active_quest : client_task_state->m_active_quests) {
		if (active_quest.task_id != TASKSLOTEMPTY) {
			client_task_state->UnlockActivities(client, active_quest);
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

							// flag to update character_activities table entry on save
							shared_task->activity[a.activity_id].updated = true;

							// set flag to persist later
							fell_behind_state = true;
						}
					}

					// fell behind, force a save of client state
					if (fell_behind_state) {
						// give reward if member was offline for shared task completion
						// live does this as long as the shared task is still active when entering game
						if (!shared_task->was_rewarded && IsActiveTaskComplete(*shared_task))
						{
							LogTasksDetail("[LoadClientState] Syncing shared task completion for client [{}]", c->GetName());
							const auto task_data = GetTaskData(shared_task->task_id);
							cts->AddReplayTimer(c, *shared_task, *task_data); // live updates a fresh timer
							cts->DispatchEventTaskComplete(c, *shared_task, task_data->activity_count - 1);
							cts->RewardTask(c, task_data, *shared_task);
						}

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

			c->MessageString(Chat::Yellow, TaskStr::NO_LONGER_MEMBER_TITLE,
				m_task_data[cts->m_active_shared_task.task_id].title.c_str());

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

void TaskManager::HandleUpdateTasksOnKill(Client* client, NPC* npc)
{
	for (auto &c: client->GetPartyMembers()) {
		if (!c->ClientDataLoaded() || !c->HasTaskState()) {
			continue;
		}

		LogTasksDetail("[HandleUpdateTasksOnKill] Looping through client [{}]", c->GetCleanName());

		c->GetTaskState()->UpdateTasksOnKill(c, client, npc);
	}
}

bool TaskManager::IsActiveTaskComplete(ClientTaskInformation& client_task)
{
	const auto task_data = GetTaskData(client_task.task_id);
	if (!task_data)
	{
		return false;
	}

	for (int i = 0; i < task_data->activity_count; ++i)
	{
		if (client_task.activity[i].activity_state != ActivityCompleted &&
		    !task_data->activity_information[i].optional)
		{
			return false;
		}
	}
	return true;
}
