#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/repositories/character_activities_repository.h"
#include "../common/repositories/character_task_timers_repository.h"
#include "../common/repositories/character_tasks_repository.h"
#include "../common/repositories/completed_tasks_repository.h"
#include "../common/rulesys.h"
#include "client.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "task_client_state.h"
#include "zonedb.h"
#include "../common/shared_tasks.h"
#include "worldserver.h"
#include "dynamic_zone.h"
#include "string_ids.h"

extern WorldServer worldserver;
extern QueryServ   *QServ;

ClientTaskState::ClientTaskState()
{
	m_active_task_count          = 0;
	m_last_completed_task_loaded = 0;
	m_checked_touch_activities   = false;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		m_active_quests[i].slot    = i;
		m_active_quests[i].task_id = TASKSLOTEMPTY;
	}

	m_active_task = {};
	m_active_task.slot    = 0;
	m_active_task.task_id = TASKSLOTEMPTY;

	m_active_shared_task = {};
	m_active_shared_task.slot    = 0;
	m_active_shared_task.task_id = TASKSLOTEMPTY;
}

ClientTaskState::~ClientTaskState()
{
}

void ClientTaskState::SendTaskHistory(Client *client, int task_index)
{

	LogTasks("[SendTaskHistory] Task history requested for completed task index [{}]", task_index);

	// We only sent the most recent 50 completed tasks, so we need to offset the Index the client sent to us.

	int adjusted_task_index = task_index;
	if (m_completed_tasks.size() > 50) {
		adjusted_task_index += (m_completed_tasks.size() - 50);
	}

	if ((adjusted_task_index < 0) || (adjusted_task_index >= (int) m_completed_tasks.size())) {
		return;
	}

	int task_id = m_completed_tasks[adjusted_task_index].task_id;
	if ((task_id < 0) || (task_id > MAXTASKS)) {
		return;
	}

	TaskInformation *p_task_data = task_manager->m_task_data[task_id];

	if (p_task_data == nullptr) {
		return;
	}

	TaskHistoryReplyHeader_Struct *task_history_reply;
	TaskHistoryReplyData1_Struct  *task_history_reply_data_1;
	TaskHistoryReplyData2_Struct  *task_history_reply_data_2;

	char *reply;

	int completed_activity_count = 0;
	int packet_length            = sizeof(TaskHistoryReplyHeader_Struct);

	for (int i = 0; i < p_task_data->activity_count; i++) {
		if (m_completed_tasks[adjusted_task_index].activity_done[i]) {
			completed_activity_count++;
			packet_length = packet_length + sizeof(TaskHistoryReplyData1_Struct) +
							p_task_data->activity_information[i].target_name.size() + 1 +
							p_task_data->activity_information[i].item_list.size() + 1 +
							sizeof(TaskHistoryReplyData2_Struct) +
							p_task_data->activity_information[i].description_override.size() + 1;
		}
	}

	auto outapp = new EQApplicationPacket(OP_TaskHistoryReply, packet_length);

	task_history_reply = (TaskHistoryReplyHeader_Struct *) outapp->pBuffer;

	// We use the TaskIndex the client sent in the request
	task_history_reply->TaskID        = task_index;
	task_history_reply->ActivityCount = completed_activity_count;

	reply = (char *) task_history_reply + sizeof(TaskHistoryReplyHeader_Struct);

	for (int i = 0; i < p_task_data->activity_count; i++) {
		if (m_completed_tasks[adjusted_task_index].activity_done[i]) {
			task_history_reply_data_1 = (TaskHistoryReplyData1_Struct *) reply;
			task_history_reply_data_1->ActivityType = static_cast<uint32_t>(p_task_data->activity_information[i].activity_type);
			reply = (char *) task_history_reply_data_1 + sizeof(TaskHistoryReplyData1_Struct);
			VARSTRUCT_ENCODE_STRING(reply, p_task_data->activity_information[i].target_name.c_str());
			VARSTRUCT_ENCODE_STRING(reply, p_task_data->activity_information[i].item_list.c_str());
			task_history_reply_data_2 = (TaskHistoryReplyData2_Struct *) reply;
			task_history_reply_data_2->GoalCount = p_task_data->activity_information[i].goal_count;
			task_history_reply_data_2->unknown04 = 0xffffffff;
			task_history_reply_data_2->unknown08 = 0xffffffff;
			task_history_reply_data_2->ZoneID    = p_task_data->activity_information[i].zone_ids.empty() ? 0
				: p_task_data->activity_information[i].zone_ids.front();
			task_history_reply_data_2->unknown16 = 0x00000000;
			reply = (char *) task_history_reply_data_2 + sizeof(TaskHistoryReplyData2_Struct);
			VARSTRUCT_ENCODE_STRING(reply, p_task_data->activity_information[i].description_override.c_str());
		}
	}

	client->QueuePacket(outapp);
	safe_delete(outapp);
}

void ClientTaskState::EnableTask(int character_id, int task_count, int *task_list)
{
	// Check if the Task is already enabled for this client
	std::vector<int> tasks_enabled;

	for (int i = 0; i < task_count; i++) {
		auto iterator = m_enabled_tasks.begin();
		bool add_task = true;

		while (iterator != m_enabled_tasks.end()) {
			// If this task is already enabled, stop looking
			if ((*iterator) == task_list[i]) {
				add_task = false;
				break;
			}
			// Our list of enabled tasks is sorted, so we can quit if we find a taskid higher than
			// the one we are looking for.
			if ((*iterator) > task_list[i]) {
				break;
			}
			++iterator;
		}

		if (add_task) {
			m_enabled_tasks.insert(iterator, task_list[i]);
			// Make a note of the task we enabled, for later SQL generation
			tasks_enabled.push_back(task_list[i]);
		}
	}

	LogTasksDetail("[EnableTask] New enabled task list");
	for (int enabled_task : m_enabled_tasks) {
		LogTasksDetail("[EnableTask] enabled [{}] character_id [{}]", enabled_task, character_id);
	}

	if (tasks_enabled.empty()) {
		return;
	}

	std::stringstream query_stream;
	query_stream << "REPLACE INTO character_enabledtasks (charid, taskid) VALUES ";
	for (unsigned int i = 0; i < tasks_enabled.size(); i++) {
		query_stream << (i ? ", " : "") << StringFormat("(%i, %i)", character_id, tasks_enabled[i]);
	}

	std::string query = query_stream.str();
	if (!tasks_enabled.empty()) {
		database.QueryDatabase(query);
	}
	else {
		LogTasks("[EnableTask] Called for character_id [{}] but, no tasks exist", character_id);
	}
}

void ClientTaskState::DisableTask(int character_id, int task_count, int *task_list)
{

	// Check if the Task is enabled for this client
	std::vector<int> tasks_disabled;

	for (int task_id = 0; task_id < task_count; task_id++) {
		auto iterator   = m_enabled_tasks.begin();
		bool removeTask = false;

		while (iterator != m_enabled_tasks.end()) {
			if ((*iterator) == task_list[task_id]) {
				removeTask = true;
				break;
			}

			if ((*iterator) > task_list[task_id]) {
				break;
			}

			++iterator;
		}

		if (removeTask) {
			m_enabled_tasks.erase(iterator);
			tasks_disabled.push_back(task_list[task_id]);
		}
	}

	LogTasks("[DisableTask] New enabled task list ");
	for (int enabled_task : m_enabled_tasks) {
		LogTasks("[DisableTask] enabled_tasks [{}]", enabled_task);
	}

	if (tasks_disabled.empty()) {
		return;
	}

	std::stringstream queryStream;
	queryStream << StringFormat("DELETE FROM character_enabledtasks WHERE charid = %i AND (", character_id);

	for (unsigned int i = 0; i < tasks_disabled.size(); i++)
		queryStream
			<< (i ? StringFormat("taskid = %i ", tasks_disabled[i]) : StringFormat(
				"OR taskid = %i ",
				tasks_disabled[i]
			));

	queryStream << ")";

	std::string query = queryStream.str();

	if (tasks_disabled.size()) {
		database.QueryDatabase(query);
	}
	else {
		LogTasks(
			"[DisableTask] DisableTask called for character_id [{}] ... but, no tasks exist",
			character_id
		);
	}
}

bool ClientTaskState::IsTaskEnabled(int task_id)
{
	std::vector<int>::iterator iterator;

	iterator = m_enabled_tasks.begin();

	while (iterator != m_enabled_tasks.end()) {
		if ((*iterator) == task_id) { return true; }
		if ((*iterator) > task_id) { break; }
		++iterator;
	}

	return false;
}

int ClientTaskState::EnabledTaskCount(int task_set_id)
{

	// Return the number of tasks in TaskSet that this character is enabled for.

	unsigned int enabled_task_index = 0;
	unsigned int task_set_index     = 0;
	int          enabled_task_count = 0;

	if ((task_set_id <= 0) || (task_set_id >= MAXTASKSETS)) { return -1; }

	while ((enabled_task_index < m_enabled_tasks.size()) &&
		   (task_set_index < task_manager->m_task_sets[task_set_id].size())) {

		if (m_enabled_tasks[enabled_task_index] == task_manager->m_task_sets[task_set_id][task_set_index]) {
			enabled_task_count++;
			enabled_task_index++;
			task_set_index++;
			continue;
		}

		if (m_enabled_tasks[enabled_task_index] < task_manager->m_task_sets[task_set_id][task_set_index]) {
			enabled_task_index++;
		}
		else {
			task_set_index++;
		}

	}

	return enabled_task_count;
}

int ClientTaskState::ActiveTasksInSet(int task_set_id)
{
	if ((task_set_id <= 0) || (task_set_id >= MAXTASKSETS)) {
		return -1;
	}

	int active_task_in_set_count = 0;

	for (int task_id : task_manager->m_task_sets[task_set_id]) {
		if (IsTaskActive(task_id)) {
			active_task_in_set_count++;
		}
	}

	return active_task_in_set_count;
}

int ClientTaskState::CompletedTasksInSet(int task_set_id)
{
	if ((task_set_id <= 0) || (task_set_id >= MAXTASKSETS)) {
		return -1;
	}

	int completed_tasks_count = 0;

	for (int i : task_manager->m_task_sets[task_set_id]) {
		if (IsTaskCompleted(i)) {
			completed_tasks_count++;
		}
	}

	return completed_tasks_count;
}

bool ClientTaskState::HasSlotForTask(TaskInformation *task)
{
	if (task == nullptr) {
		return false;
	}

	switch (task->type) {
		case TaskType::Task:
			return m_active_task.task_id == TASKSLOTEMPTY;
		case TaskType::Shared:
			return m_active_shared_task.task_id == TASKSLOTEMPTY;
		case TaskType::Quest:
			for (auto &active_quest : m_active_quests) {
				if (active_quest.task_id == TASKSLOTEMPTY) {
					return true;
				}
			}
		case TaskType::E:
			return false; // removed on live
	}

	return false;
}

int ClientTaskState::GetActiveTaskID(int index)
{
	// Return the task_id from the client's specified Active Task slot.
	if ((index < 0) || (index >= MAXACTIVEQUESTS)) {
		return 0;
	}

	return m_active_quests[index].task_id;
}

static void DeleteCompletedTaskFromDatabase(int character_id, int task_id)
{
	LogTasks("[DeleteCompletedTasksFromDatabase] character_id [{}], task_id [{}]", character_id, task_id);

	CompletedTasksRepository::DeleteWhere(
		database,
		fmt::format("charid = {} and taskid = {}", character_id, task_id)
	);
}

bool ClientTaskState::UnlockActivities(int character_id, ClientTaskInformation &task_info)
{
	bool all_activities_complete = true;

	LogTasksDetail(
		"[UnlockActivities] Fetching task info for character_id [{}] task [{}] slot [{}] current_step [{}] accepted_time [{}] updated [{}]",
		character_id,
		task_info.task_id,
		task_info.slot,
		task_info.current_step,
		task_info.accepted_time,
		task_info.updated
	);

	TaskInformation *p_task_data = task_manager->m_task_data[task_info.task_id];
	if (p_task_data == nullptr) {
		return true;
	}

	for (int i = 0; i < p_task_data->activity_count; i++) {
		if (task_info.activity[i].activity_id >= 0) {
			LogTasksDetail(
				"[UnlockActivities] character_id [{}] task [{}] activity_id [{}] done_count [{}] activity_state [{}] updated [{}] sequence [{}]",
				character_id,
				task_info.task_id,
				task_info.activity[i].activity_id,
				task_info.activity[i].done_count,
				task_info.activity[i].activity_state,
				task_info.activity[i].updated,
				p_task_data->sequence_mode
			);
		}
	}

	// On loading the client state, all activities that are not completed, are
	// marked as hidden. For Sequential (non-stepped) mode, we mark the first
	// activity_information as active if not complete.

	if (p_task_data->sequence_mode == ActivitiesSequential) {
		if (task_info.activity[0].activity_state != ActivityCompleted) {
			task_info.activity[0].activity_state = ActivityActive;
		}

		// Enable the next Hidden task.
		for (int i = 0; i < p_task_data->activity_count; i++) {
			if ((task_info.activity[i].activity_state == ActivityActive) &&
				(!p_task_data->activity_information[i].optional)) {
				all_activities_complete = false;
				break;
			}

			if (task_info.activity[i].activity_state == ActivityHidden) {
				task_info.activity[i].activity_state = ActivityActive;
				all_activities_complete = false;
				break;
			}
		}

		if (all_activities_complete && RuleB(TaskSystem, RecordCompletedTasks)) {
			if (RuleB(TasksSystem, KeepOneRecordPerCompletedTask)) {
				LogTasks("KeepOneRecord enabled");
				auto iterator        = m_completed_tasks.begin();
				int  erased_elements = 0;
				while (iterator != m_completed_tasks.end()) {
					int task_id = (*iterator).task_id;
					if (task_id == task_info.task_id) {
						iterator = m_completed_tasks.erase(iterator);
						erased_elements++;
					}
					else {
						++iterator;
					}
				}

				LogTasks("Erased Element count is [{}]", erased_elements);

				if (erased_elements) {
					m_last_completed_task_loaded -= erased_elements;
					DeleteCompletedTaskFromDatabase(character_id, task_info.task_id);
				}
			}

			if (p_task_data->type != TaskType::Shared) {
				CompletedTaskInformation completed_task_information{};
				completed_task_information.task_id        = task_info.task_id;
				completed_task_information.completed_time = time(nullptr);

				for (int i = 0; i < p_task_data->activity_count; i++) {
					completed_task_information.activity_done[i] = (task_info.activity[i].activity_state ==
						ActivityCompleted);
				}

				m_completed_tasks.push_back(completed_task_information);
			}
		}

		LogTasks("Returning sequential task, AllActivitiesComplete is [{}]", all_activities_complete);

		return all_activities_complete;
	}

	// Stepped Mode
	// TODO: This code is probably more complex than it needs to be

	bool current_step_complete = true;

	LogTasks(
		"[UnlockActivities] Current step [{}] last_step [{}]",
		task_info.current_step,
		p_task_data->last_step
	);

	// If current_step is -1, this is the first call to this method since loading the
	// client state. Unlock all activities with a step number of 0

	if (task_info.current_step == -1) {
		for (int i             = 0; i < p_task_data->activity_count; i++) {

			if (p_task_data->activity_information[i].step_number == 0 &&
				task_info.activity[i].activity_state == ActivityHidden) {
				task_info.activity[i].activity_state = ActivityActive;
				// task_info.activity_information[i].updated=true;
			}
		}
		task_info.current_step = 0;
	}

	for (int current_step = task_info.current_step; current_step <= p_task_data->last_step; current_step++) {
		for (int activity = 0; activity < p_task_data->activity_count; activity++) {
			if (p_task_data->activity_information[activity].step_number == (int) task_info.current_step) {
				if ((task_info.activity[activity].activity_state != ActivityCompleted) &&
					(!p_task_data->activity_information[activity].optional)) {
					current_step_complete   = false;
					all_activities_complete = false;
					break;
				}
			}
		}
		if (!current_step_complete) {
			break;
		}
		task_info.current_step++;
	}

	if (all_activities_complete) {
		if (RuleB(TaskSystem, RecordCompletedTasks)) {
			// If we are only keeping one completed record per task, and the player has done
			// the same task again, erase the previous completed entry for this task.
			if (RuleB(TasksSystem, KeepOneRecordPerCompletedTask)) {
				LogTasksDetail("[UnlockActivities] KeepOneRecord enabled");
				auto iterator        = m_completed_tasks.begin();
				int  erased_elements = 0;

				while (iterator != m_completed_tasks.end()) {
					int task_id = (*iterator).task_id;
					if (task_id == task_info.task_id) {
						iterator = m_completed_tasks.erase(iterator);
						erased_elements++;
					}
					else {
						++iterator;
					}
				}

				LogTasksDetail("[UnlockActivities] Erased Element count is [{}]", erased_elements);

				if (erased_elements) {
					m_last_completed_task_loaded -= erased_elements;
					DeleteCompletedTaskFromDatabase(character_id, task_info.task_id);
				}
			}

			if (p_task_data->type != TaskType::Shared) {
				CompletedTaskInformation completed_task_information{};
				completed_task_information.task_id        = task_info.task_id;
				completed_task_information.completed_time = time(nullptr);

				for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
					completed_task_information.activity_done[activity_id] =
						(task_info.activity[activity_id].activity_state == ActivityCompleted);
				}

				m_completed_tasks.push_back(completed_task_information);
			}
		}
		return true;
	}

	// Mark all non-completed tasks in the current step as active
	for (int activity = 0; activity < p_task_data->activity_count; activity++) {
		LogTasksDetail(
			"[UnlockActivities] - Debug task [{}] activity [{}] step_number [{}] current_step [{}]",
			task_info.task_id,
			activity,
			p_task_data->activity_information[activity].step_number,
			(int) task_info.current_step

		);

		if ((p_task_data->activity_information[activity].step_number == (int) task_info.current_step) &&
			(task_info.activity[activity].activity_state == ActivityHidden)) {

			LogTasksDetail(
				"[UnlockActivities] -- Debug task [{}] activity [{}] (ActivityActive)",
				task_info.task_id,
				activity
			);

			task_info.activity[activity].activity_state = ActivityActive;
			task_info.activity[activity].updated        = true;
		}
	}

	return false;
}

bool ClientTaskState::UpdateTasksOnSpeakWith(Client *client, int npc_type_id)
{
	return UpdateTasksByNPC(client, TaskActivityType::SpeakWith, npc_type_id);
}

bool ClientTaskState::UpdateTasksByNPC(Client *client, TaskActivityType activity_type, int npc_type_id)
{

	int is_updating = false;

	if (!HasActiveTasks()) {
		return false;
	}

	// loop over the union of tasks and quests
	for (auto &active_task : m_active_tasks) {
		auto current_task = &active_task;
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active kill activities for this p_task_data
		auto p_task_data = task_manager->m_task_data[current_task->task_id];
		if (p_task_data == nullptr) {
			return false;
		}

		for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
			ClientActivityInformation *client_activity = &current_task->activity[activity_id];
			ActivityInformation       *activity_info   = &p_task_data->activity_information[activity_id];

			// We are not interested in completed or hidden activities
			if (client_activity->activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in Kill activities
			if (activity_info->activity_type != activity_type) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!activity_info->CheckZone(zone->GetZoneID())) {
				LogTasks(
					"[UPDATE] character [{}] task_id [{}] activity_id [{}] activity_type [{}] for NPC [{}] failed zone check",
					client->GetName(),
					current_task->task_id,
					activity_id,
					static_cast<int32_t>(activity_type),
					npc_type_id
				);
				continue;
			}
			// Is the activity_information to kill this type of NPC ?
			switch (activity_info->goal_method) {
				case METHODSINGLEID:
					if (activity_info->goal_id != npc_type_id) {
						continue;
					}
					break;

				case METHODLIST:
					if (!task_manager->m_goal_list_manager.IsInList(
						activity_info->goal_id,
						npc_type_id
					) && !TaskGoalListManager::IsInMatchList(
						activity_info->goal_match_list,
						std::to_string(npc_type_id)
					)) {
						continue;
					}
					break;

				default:
					// If METHODQUEST, don't updated the activity_information here
					continue;
			}
			// We found an active p_task_data to kill this type of NPC, so increment the done count
			LogTasksDetail("Calling increment done count ByNPC");
			IncrementDoneCount(client, p_task_data, current_task->slot, activity_id);
			is_updating = true;
		}
	}

	return is_updating;
}

int ClientTaskState::ActiveSpeakTask(int npc_type_id)
{

	// This method is to be used from Perl quests only and returns the task_id of the first
	// active task found which has an active SpeakWith activity_information for this NPC.
	if (!HasActiveTasks()) {
		return 0;
	}

	// loop over the union of tasks and quests
	for (auto &active_task : m_active_tasks) {
		auto current_task = &active_task;
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		TaskInformation *p_task_data = task_manager->m_task_data[current_task->task_id];
		if (p_task_data == nullptr) {
			continue;
		}

		for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
			ClientActivityInformation *client_activity = &current_task->activity[activity_id];
			ActivityInformation       *activity_info   = &p_task_data->activity_information[activity_id];

			// We are not interested in completed or hidden activities
			if (client_activity->activity_state != ActivityActive) {
				continue;
			}
			if (activity_info->activity_type != TaskActivityType::SpeakWith) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!activity_info->CheckZone(zone->GetZoneID())) {
				continue;
			}
			// Is the activity_information to speak with this type of NPC ?
			if (activity_info->goal_method == METHODQUEST && activity_info->goal_id == npc_type_id) {
				return current_task->task_id;
			}
		}
	}

	return 0;
}

int ClientTaskState::ActiveSpeakActivity(int npc_type_id, int task_id)
{

	// This method is to be used from Perl quests only and returns the activity_id of the first
	// active activity_information found in the specified task which is to SpeakWith this NPC.
	if (!HasActiveTasks()) {
		return -1;
	}
	if (task_id <= 0 || task_id >= MAXTASKS) {
		return -1;
	}

	// loop over the union of tasks and quests
	for (auto &active_task : m_active_tasks) {
		auto current_task = &active_task;
		if (current_task->task_id != task_id) {
			continue;
		}

		TaskInformation *p_task_data = task_manager->m_task_data[current_task->task_id];
		if (p_task_data == nullptr) {
			continue;
		}

		for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
			ClientActivityInformation *client_activity = &current_task->activity[activity_id];
			ActivityInformation       *activity_info   = &p_task_data->activity_information[activity_id];

			// We are not interested in completed or hidden activities
			if (client_activity->activity_state != ActivityActive) {
				continue;
			}
			if (activity_info->activity_type != TaskActivityType::SpeakWith) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!activity_info->CheckZone(zone->GetZoneID())) {
				continue;
			}

			// Is the activity_information to speak with this type of NPC ?
			if (activity_info->goal_method == METHODQUEST && activity_info->goal_id == npc_type_id) {
				return activity_id;
			}
		}
		return 0;
	}
	return 0;
}

void ClientTaskState::UpdateTasksForItem(Client *client, TaskActivityType activity_type, int item_id, int count)
{

	// This method updates the client's task activities of the specified type which relate
	// to the specified item.
	//
	// Type should be one of ActivityLoot, ActivityTradeSkill, ActivityFish or ActivityForage

	// If the client has no tasks, there is nothing further to check.

	LogTasks(
		"[UpdateTasksForItem] activity_type [{}] item_id [{}]",
		static_cast<int32_t>(activity_type),
		item_id
	);

	if (!HasActiveTasks()) {
		return;
	}

	// loop over the union of tasks and quests
	for (auto &active_task : m_active_tasks) {
		auto current_task = &active_task;
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active loot activities for this task

		TaskInformation *p_task_data = task_manager->m_task_data[current_task->task_id];
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
			// We are only interested in the ActivityType we were called with
			if (activity_info->activity_type != activity_type) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!activity_info->CheckZone(zone->GetZoneID())) {
				LogTasks(
					"[UpdateTasksForItem] Error: Character [{}] activity_information type [{}] for Item [{}] failed zone check",
					client->GetName(),
					static_cast<int32_t>(activity_type),
					item_id
				);
				continue;
			}
			// Is the activity_information related to this item ?
			//
			switch (activity_info->goal_method) {

				case METHODSINGLEID:
					if (activity_info->goal_id != item_id) { continue; }
					break;

				case METHODLIST:
					if (!task_manager->m_goal_list_manager.IsInList(
						activity_info->goal_id,
						item_id
					) && !TaskGoalListManager::IsInMatchList(
						activity_info->goal_match_list,
						std::to_string(item_id)
					)) { continue; }
					break;

				default:
					// If METHODQUEST, don't updated the activity_information here
					continue;
			}
			// We found an active task related to this item, so increment the done count
			LogTasksDetail("[UpdateTasksForItem] Calling increment done count ForItem");
			IncrementDoneCount(client, p_task_data, current_task->slot, activity_id, count);
		}
	}
}

void ClientTaskState::UpdateTasksOnExplore(Client *client, int explore_id)
{
	LogTasks("[UpdateTasksOnExplore] explore_id [{}]", explore_id);

	if (!HasActiveTasks()) {
		return;
	}

	// loop over the union of tasks and quests
	for (auto &active_task : m_active_tasks) {
		auto current_task = &active_task;
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active explore activities for this task

		TaskInformation *task_data = task_manager->m_task_data[current_task->task_id];
		if (task_data == nullptr) {
			return;
		}

		for (int activity_id = 0; activity_id < task_data->activity_count; activity_id++) {
			ClientActivityInformation *client_activity = &current_task->activity[activity_id];
			ActivityInformation       *activity_info   = &task_data->activity_information[activity_id];

			// We are not interested in completed or hidden activities
			if (client_activity->activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in explore activities
			if (activity_info->activity_type != TaskActivityType::Explore) {
				continue;
			}
			if (!activity_info->CheckZone(zone->GetZoneID())) {
				LogTasks(
					"[UpdateTasksOnExplore] character [{}] explore_id [{}] failed zone check",
					client->GetName(),
					explore_id
				);
				continue;
			}
			// Is the activity_information to explore this area id ?
			switch (activity_info->goal_method) {

				case METHODSINGLEID:
					if (activity_info->goal_id != explore_id) {
						continue;
					}
					break;

				case METHODLIST:
					if (!task_manager->m_goal_list_manager.IsInList(
						activity_info->goal_id,
						explore_id
					) && !TaskGoalListManager::IsInMatchList(
						activity_info->goal_match_list,
						std::to_string(explore_id)
					)) {
						continue;
					}
					break;

				default:
					// If METHODQUEST, don't updated the activity_information here
					continue;
			}

			// We found an active task to explore this area, so set done count to goal count
			// (Only a goal count of 1 makes sense for explore activities?)
			LogTasks(
				"[UpdateTasksOnExplore] character [{}] explore_id [{}] increment on explore",
				client->GetName(),
				explore_id
			);

			IncrementDoneCount(
				client,
				task_data,
				current_task->slot,
				activity_id,
				activity_info->goal_count - current_task->activity[activity_id].done_count
			);
		}
	}
}

bool ClientTaskState::UpdateTasksOnDeliver(
	Client *client,
	std::list<EQ::ItemInstance *> &items,
	int cash,
	int npc_type_id
)
{
	bool is_updated = false;

	LogTasks("[UpdateTasksOnDeliver] [{}]", npc_type_id);

	if (!HasActiveTasks()) {
		return false;
	}

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto current_task = &m_active_tasks[i];
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active deliver activities for this task
		TaskInformation *p_task_data = task_manager->m_task_data[current_task->task_id];
		if (p_task_data == nullptr) {
			return false;
		}

		for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
			ClientActivityInformation *client_activity = &current_task->activity[activity_id];
			ActivityInformation       *activity_info   = &p_task_data->activity_information[activity_id];

			// We are not interested in completed or hidden activities
			if (client_activity->activity_state != ActivityActive) {
				continue;
			}

			// We are only interested in Deliver activities
			if (activity_info->activity_type != TaskActivityType::Deliver &&
				activity_info->activity_type != TaskActivityType::GiveCash) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!activity_info->CheckZone(zone->GetZoneID())) {
				Log(
					Logs::General, Logs::Tasks,
					"[UPDATE] Char: %s Deliver activity_information failed zone check (current zone %i, need zone "
					"%s",
					client->GetName(), zone->GetZoneID(), activity_info->zones.c_str());
				continue;
			}
			// Is the activity_information to deliver to this NPCTypeID ?
			if (activity_info->deliver_to_npc != npc_type_id) {
				continue;
			}
			// Is the activity_information related to these items ?
			//
			if ((activity_info->activity_type == TaskActivityType::GiveCash) && cash) {
				LogTasks("[UpdateTasksOnDeliver] Increment on GiveCash");
				IncrementDoneCount(client, p_task_data, i, activity_id, cash);
				is_updated = true;
			}
			else {
				for (auto &item : items) {
					switch (activity_info->goal_method) {
						case METHODSINGLEID:
							if (activity_info->goal_id != item->GetID()) {
								continue;
							}
							break;

						case METHODLIST:
							if (!task_manager->m_goal_list_manager.IsInList(
								activity_info->goal_id,
								item->GetID()
							) && !TaskGoalListManager::IsInMatchList(
								activity_info->goal_match_list,
								std::to_string(item->GetID())
							)) {
								continue;
							}
							break;

						default:
							// If METHODQUEST, don't updated the activity_information here
							continue;
					}
					// We found an active task related to this item, so increment the done count
					LogTasks("[UpdateTasksOnDeliver] Increment on GiveItem");
					IncrementDoneCount(
						client,
						p_task_data,
						current_task->slot,
						activity_id,
						item->GetCharges() <= 0 ? 1 : item->GetCharges()
					);
					is_updated = true;
				}
			}
		}
	}

	return is_updated;
}

void ClientTaskState::UpdateTasksOnTouch(Client *client, int zone_id)
{
	// If the client has no tasks, there is nothing further to check.

	LogTasks("[UpdateTasksOnTouch] [{}] ", zone_id);

	if (!HasActiveTasks()) {
		return;
	}

	// loop over the union of tasks and quests
	for (auto &active_task : m_active_tasks) {
		auto current_task = &active_task;
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active explore activities for this task
		TaskInformation *p_task_data = task_manager->m_task_data[current_task->task_id];
		if (p_task_data == nullptr) {
			return;
		}

		for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
			ClientActivityInformation *client_activity = &current_task->activity[activity_id];
			ActivityInformation       *activity_info   = &p_task_data->activity_information[activity_id];

			// We are not interested in completed or hidden activities
			if (current_task->activity[activity_id].activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in touch activities
			if (activity_info->activity_type != TaskActivityType::Touch) {
				continue;
			}
			if (activity_info->goal_method != METHODSINGLEID) {
				continue;
			}
			if (!activity_info->CheckZone(zone_id)) {
				LogTasks(
					"[UpdateTasksOnTouch] character [{}] Touch activity_information failed zone check",
					client->GetName()
				);
				continue;
			}

			// We found an active task to zone into this zone, so set done count to goal count
			// (Only a goal count of 1 makes sense for touch activities?)
			LogTasks("[UpdateTasksOnTouch] Increment on Touch");
			IncrementDoneCount(
				client,
				p_task_data,
				current_task->slot,
				activity_id,
				activity_info->goal_count - current_task->activity[activity_id].done_count
			);
		}
	}
}

void ClientTaskState::IncrementDoneCount(
	Client *client,
	TaskInformation *task_information,
	int task_index,
	int activity_id,
	int count,
	bool ignore_quest_update
)
{
	auto info = GetClientTaskInfo(task_information->type, task_index);
	if (info == nullptr) {
		return;
	}

	LogTasks(
		"[IncrementDoneCount] client [{}] task_id [{}] activity_id [{}] count [{}]",
		client->GetCleanName(),
		info->task_id,
		activity_id,
		count
	);

	// shared task shim
	// intercept and pass to world first before processing normally
	if (!client->m_shared_task_update && task_information->type == TaskType::Shared) {

		// struct
		auto pack = new ServerPacket(ServerOP_SharedTaskUpdate, sizeof(ServerSharedTaskActivityUpdate_Struct));
		auto *r   = (ServerSharedTaskActivityUpdate_Struct *) pack->pBuffer;

		// fill
		r->source_character_id = client->CharacterID();
		r->task_id             = info->task_id;
		r->activity_id         = activity_id;
		r->done_count          = info->activity[activity_id].done_count + count;
		r->ignore_quest_update = ignore_quest_update;

		LogTasksDetail(
			"[IncrementDoneCount] shared_task sending client [{}] task_id [{}] activity_id [{}] count [{}] ignore_quest_update [{}]",
			r->source_character_id,
			r->task_id,
			r->activity_id,
			r->done_count,
			(ignore_quest_update ? "true" : "false")
		);

		SyncSharedTaskZoneClientDoneCountState(
			client,
			task_information,
			task_index,
			activity_id,
			r->done_count
		);

		// send
		worldserver.SendPacket(pack);
		safe_delete(pack);

		return;
	}

	info->activity[activity_id].done_count += count;

	if (info->activity[activity_id].done_count > task_information->activity_information[activity_id].goal_count) {
		info->activity[activity_id].done_count = task_information->activity_information[activity_id].goal_count;
	}

	if (!ignore_quest_update) {
		std::string export_string = fmt::format(
			"{} {} {}",
			info->activity[activity_id].done_count,
			info->activity[activity_id].activity_id,
			info->task_id
		);
		parse->EventPlayer(EVENT_TASK_UPDATE, client, export_string, 0);
	}

	info->activity[activity_id].updated = true;
	// Have we reached the goal count for this activity_information ?
	if (info->activity[activity_id].done_count >= task_information->activity_information[activity_id].goal_count) {
		LogTasks("[IncrementDoneCount] done_count [{}] goal_count [{}] activity_id [{}]",
			info->activity[activity_id].done_count,
			task_information->activity_information[activity_id].goal_count,
			activity_id
		);

		// Flag the activity_information as complete
		info->activity[activity_id].activity_state = ActivityCompleted;
		// Unlock subsequent activities for this task
		bool task_complete = UnlockActivities(client->CharacterID(), *info);
		LogTasks("[IncrementDoneCount] task_complete is [{}]", task_complete);
		// and by the 'Task Stage Completed' message
		client->SendTaskActivityComplete(info->task_id, activity_id, task_index, task_information->type);
		// Send the updated task/activity_information list to the client
		task_manager->SendSingleActiveTaskToClient(client, *info, task_complete, false);
		// Inform the client the task has been updated, both by a chat message
		client->MessageString(Chat::White, TASK_UPDATED, task_information->title.c_str());

		if (!ignore_quest_update) {
			std::string export_string = fmt::format(
				"{} {}",
				info->task_id,
				info->activity[activity_id].activity_id
			);
			parse->EventPlayer(EVENT_TASK_STAGE_COMPLETE, client, export_string, 0);
		}
		/* QS: PlayerLogTaskUpdates :: Update */
		if (RuleB(QueryServ, PlayerLogTaskUpdates)) {
			std::string event_desc = StringFormat(
				"Task Stage Complete :: taskid:%i activityid:%i donecount:%i in zoneid:%i instid:%i",
				info->task_id,
				info->activity[activity_id].activity_id,
				info->activity[activity_id].done_count,
				client->GetZoneID(),
				client->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Task_Updates, client->CharacterID(), event_desc);
		}

		// If this task is now complete, the Completed tasks will have been
		// updated in UnlockActivities. Send the completed task list to the
		// client. This is the same sequence the packets are sent on live.
		if (task_complete) {
			std::string export_string = fmt::format(
				"{} {} {}",
				info->activity[activity_id].done_count,
				info->activity[activity_id].activity_id,
				info->task_id
			);
			parse->EventPlayer(EVENT_TASK_COMPLETE, client, export_string, 0);

			/* QS: PlayerLogTaskUpdates :: Complete */
			if (RuleB(QueryServ, PlayerLogTaskUpdates)) {
				std::string event_desc = StringFormat(
					"Task Complete :: taskid:%i activityid:%i donecount:%i in zoneid:%i instid:%i",
					info->task_id,
					info->activity[activity_id].activity_id,
					info->activity[activity_id].done_count,
					client->GetZoneID(),
					client->GetInstanceID());
				QServ->PlayerLogEvent(Player_Log_Task_Updates, client->CharacterID(), event_desc);
			}

			client->SendTaskActivityComplete(info->task_id, 0, task_index, task_information->type, 0);
			task_manager->SaveClientState(client, this);

			// If Experience and/or cash rewards are set, reward them from the task even if reward_method is METHODQUEST
			RewardTask(client, task_information);
			//RemoveTask(c, TaskIndex);

			// add replay timer (world adds timers to shared task members)
			AddReplayTimer(client, *info, *task_information);

			// shared tasks linger at the completion step and do not get removed from the task window unlike quests/task
			if (task_information->type == TaskType::Shared) {
				return;
			}

			task_manager->SendCompletedTasksToClient(client, this);

			client->CancelTask(task_index, task_information->type);
		}

	}
	else {
		// Send an updated packet for this single activity_information
		task_manager->SendTaskActivityLong(
			client,
			info->task_id,
			activity_id,
			task_index
		);
		task_manager->SaveClientState(client, this);
	}
}

void ClientTaskState::RewardTask(Client *client, TaskInformation *task_information)
{

	if (!task_information || !client) {
		return;
	}

	if (!task_information->completion_emote.empty()) {
		client->Message(Chat::Yellow, task_information->completion_emote.c_str());
	}

	const EQ::ItemData *item_data;
	std::vector<int>   reward_list;

	switch (task_information->reward_method) {
		case METHODSINGLEID: {
			if (task_information->reward_id) {
				client->SummonItem(task_information->reward_id);
				item_data = database.GetItem(task_information->reward_id);
				if (item_data) {
					client->MessageString(Chat::Yellow, YOU_HAVE_BEEN_GIVEN, item_data->Name);
				}
			}
			break;
		}
		case METHODLIST: {
			reward_list = task_manager->m_goal_list_manager.GetListContents(task_information->reward_id);
			for (int item_id : reward_list) {
				client->SummonItem(item_id);
				item_data = database.GetItem(item_id);
				if (item_data) {
					client->MessageString(Chat::Yellow, YOU_HAVE_BEEN_GIVEN, item_data->Name);
				}
			}
			break;
		}
		default: {
			// Nothing special done for METHODQUEST
			break;
		}
	}

	// just use normal NPC faction ID stuff
	if (task_information->faction_reward) {
		client->SetFactionLevel(
			client->CharacterID(),
			task_information->faction_reward,
			client->GetBaseClass(),
			client->GetBaseRace(),
			client->GetDeity()
		);
	}

	if (task_information->cash_reward) {
		int platinum, gold, silver, copper;

		copper = task_information->cash_reward;
		client->AddMoneyToPP(copper, true);

		platinum = copper / 1000;
		copper   = copper - (platinum * 1000);
		gold     = copper / 100;
		copper   = copper - (gold * 100);
		silver   = copper / 10;
		copper   = copper - (silver * 10);

		if (
			copper ||
			silver ||
			gold ||
			platinum
		) {
			client->MessageString(
				Chat::Yellow,
				YOU_RECEIVE,
				ConvertMoneyToString(
					platinum,
					gold,
					silver,
					copper
				).c_str()
			);
		}
	}
	int32 experience_reward = task_information->experience_reward;
	if (experience_reward > 0) {
		client->AddEXP(experience_reward);
	}
	if (experience_reward < 0) {
		uint32 pos_reward = experience_reward * -1;
		// Minimal Level Based Exp reward Setting is 101 (1% exp at level 1)
		if (pos_reward > 100 && pos_reward < 25700) {
			uint8 max_level   = pos_reward / 100;
			uint8 exp_percent = pos_reward - (max_level * 100);
			client->AddLevelBasedExp(exp_percent, max_level);
		}
	}

	if (task_information->reward_radiant_crystals > 0 || task_information->reward_ebon_crystals > 0)
	{
		client->AddCrystals(task_information->reward_radiant_crystals, task_information->reward_ebon_crystals);
	}

	client->SendSound();
}

bool ClientTaskState::IsTaskActive(int task_id)
{
	if (m_active_task.task_id == task_id) {
		return true;
	}

	if (m_active_shared_task.task_id == task_id) {
		return true;
	}

	if (m_active_task_count == 0 || task_id == 0) {
		return false;
	}

	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id == task_id) {
			return true;
		}
	}

	return false;
}

void ClientTaskState::FailTask(Client *client, int task_id)
{
	LogTasks(
		"[FailTask] Failing task for character [{}] task_id [{}] task_count [{}]",
		client->GetCleanName(),
		task_id,
		m_active_task_count
	);


	// type: Task
	if (m_active_task.task_id == task_id) {
		client->SendTaskFailed(task_id, TASKSLOTTASK, TaskType::Task);
		// Remove the task from the client
		client->CancelTask(TASKSLOTTASK, TaskType::Task);
		return;
	}

	// type: Shared Task
	if (m_active_shared_task.task_id == task_id) {
		client->SendTaskFailed(task_id, TASKSLOTSHAREDTASK, TaskType::Shared);
		// Remove the task from the client
		client->CancelTask(TASKSLOTSHAREDTASK, TaskType::Shared);
		return;
	}

	// TODO: shared tasks

	if (m_active_task_count == 0) {
		return;
	}

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (m_active_quests[i].task_id == task_id) {
			client->SendTaskFailed(m_active_quests[i].task_id, i, TaskType::Quest);
			// Remove the task from the client
			client->CancelTask(i, TaskType::Quest);
			return;
		}
	}
}

// TODO: Shared tasks
bool ClientTaskState::IsTaskActivityActive(int task_id, int activity_id)
{
	LogTasks("[IsTaskActivityActive] task_id [{}] activity_id [{}]", task_id, activity_id);

	// Quick sanity check
	if (activity_id < 0) {
		return false;
	}
	if (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY &&
		m_active_shared_task.task_id == TASKSLOTEMPTY) {
		return false;
	}

	int  active_task_index = -1;
	auto task_type         = TaskType::Task;
	if (m_active_task.task_id == task_id) {
		active_task_index = TASKSLOTTASK;
	}
	if (m_active_shared_task.task_id == task_id) {
		task_type         = TaskType::Shared;
		active_task_index = TASKSLOTSHAREDTASK;
	}

	if (active_task_index == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (m_active_quests[i].task_id == task_id) {
				active_task_index = i;
				task_type         = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (active_task_index == -1) {
		return false;
	}

	auto info = GetClientTaskInfo(task_type, active_task_index);

	if (info == nullptr) {
		return false;
	}

	TaskInformation *p_task_data = task_manager->m_task_data[info->task_id];

	// The task is invalid
	if (p_task_data == nullptr) {
		return false;
	}

	// The activity_id is out of range
	if (activity_id >= p_task_data->activity_count) {
		return false;
	}

	LogTasks(
		"[IsTaskActivityActive] (Update) task_id [{}] activity_id [{}] activity_state [{}]",
		task_id,
		activity_id,
		info->activity[activity_id].activity_state
	);

	return (info->activity[activity_id].activity_state == ActivityActive);
}

void ClientTaskState::UpdateTaskActivity(
	Client *client,
	int task_id,
	int activity_id,
	int count,
	bool ignore_quest_update /*= false*/)
{
	LogTasks(
		"[UpdateTaskActivity] Increment done count (pre) on UpdateTaskActivity task_id [{}] activity_id [{}] count [{}]",
		task_id,
		activity_id,
		count
	);

	// Quick sanity check
	if (activity_id < 0 || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY &&
							m_active_shared_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  active_task_index = -1;
	auto type              = TaskType::Task;

	if (m_active_task.task_id == task_id) {
		active_task_index = TASKSLOTTASK;
	}
	if (m_active_shared_task.task_id == task_id) {
		type              = TaskType::Shared;
		active_task_index = TASKSLOTSHAREDTASK;
	}

	if (active_task_index == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (m_active_quests[i].task_id == task_id) {
				active_task_index = i;
				type              = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (active_task_index == -1) {
		return;
	}

	auto info = GetClientTaskInfo(type, active_task_index);
	if (info == nullptr) {
		return;
	}

	TaskInformation *p_task_data = task_manager->m_task_data[info->task_id];

	// The task is invalid
	if (p_task_data == nullptr) {
		return;
	}

	// The activity_id is out of range
	if (activity_id >= p_task_data->activity_count) {
		return;
	}

	// The activity_information is not currently active
	if (info->activity[activity_id].activity_state == ActivityHidden) {
		return;
	}

	LogTasks(
		"[UpdateTaskActivity] Increment done count (done) on UpdateTaskActivity task_id [{}] activity_id [{}] count [{}]",
		task_id,
		activity_id,
		count
	);

	IncrementDoneCount(client, p_task_data, active_task_index, activity_id, count, ignore_quest_update);
}

void ClientTaskState::ResetTaskActivity(Client *client, int task_id, int activity_id)
{
	LogTasks(
		"[ResetTaskActivity] (pre) client [{}] task_id [{}] activity_id [{}]",
		client->GetCleanName(),
		task_id,
		activity_id
	);

	// Quick sanity check
	if (activity_id < 0 || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY &&
							m_active_shared_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  active_task_index = -1;
	auto type              = TaskType::Task;
	if (m_active_task.task_id == task_id) {
		active_task_index = TASKSLOTTASK;
	}
	if (m_active_shared_task.task_id == task_id) {
		type              = TaskType::Shared;
		active_task_index = TASKSLOTSHAREDTASK;
	}

	if (active_task_index == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (m_active_quests[i].task_id == task_id) {
				active_task_index = i;
				type              = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (active_task_index == -1) {
		return;
	}

	auto info = GetClientTaskInfo(type, active_task_index);
	if (info == nullptr) {
		return;
	}

	TaskInformation *p_task_data = task_manager->m_task_data[info->task_id];
	if (p_task_data == nullptr) {
		return;
	}

	// The activity_id is out of range
	if (activity_id >= p_task_data->activity_count) {
		return;
	}

	// The activity_information is not currently active
	if (info->activity[activity_id].activity_state == ActivityHidden) {
		return;
	}

	LogTasks(
		"[ResetTaskActivity] (IncrementDoneCount) client [{}] task_id [{}] activity_id [{}]",
		client->GetCleanName(),
		task_id,
		activity_id
	);

	IncrementDoneCount(
		client,
		p_task_data,
		active_task_index,
		activity_id,
		(info->activity[activity_id].done_count * -1),
		false
	);
}

void ClientTaskState::ShowClientTaskInfoMessage(ClientTaskInformation *task, Client *c)
{
	auto task_data = task_manager->m_task_data[task->task_id];

	c->Message(Chat::White, "------------------------------------------------");
	c->Message(
		Chat::White,
		fmt::format(
			"Task {} | Title: {} ID: {} Type: {}",
			task->slot,
			task_data->title,
			task->task_id,
			Tasks::GetTaskTypeDescription(task_data->type)
		).c_str()
	);
	c->Message(Chat::White, "------------------------------------------------");
	c->Message(
		Chat::White,
		fmt::format(
			"Description | {}",
			task_data->description
		).c_str()
	);

	for (int activity_id = 0; activity_id < task_manager->GetActivityCount(task->task_id); activity_id++) {
		std::vector<std::string> update_increments = { "1", "5", "10", "20", "50" };
		std::vector<std::string> update_saylinks;

		for (auto &increment: update_increments) {
			auto task_update_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
				fmt::format(
					"#task update {} {} {}",
					task->task_id,
					task->activity[activity_id].activity_id,
					increment
				),
				false,
				increment
			);

			update_saylinks.push_back(task_update_saylink);
		}

		c->Message(
			Chat::White,
			fmt::format(
				"Activity {} | Count: {} State: {} ({}) [{}]",
				task->activity[activity_id].activity_id,
				task->activity[activity_id].done_count,
				task->activity[activity_id].activity_state,
				Tasks::GetActivityStateDescription(task->activity[activity_id].activity_state),
				implode(" | ", update_saylinks)
			).c_str()
		);
	}
}

void ClientTaskState::ShowClientTasks(Client* who, Client *to)
{
	to->SendChatLineBreak();

	to->Message(
		Chat::White,
		fmt::format(
			"Task Information for {}",
			to->GetTargetDescription(who, TargetDescriptionType::UCSelf)
		).c_str()
	);

	if (m_active_task.task_id != TASKSLOTEMPTY) {
		ShowClientTaskInfoMessage(&m_active_task, to);
	}

	if (m_active_shared_task.task_id != TASKSLOTEMPTY) {
		ShowClientTaskInfoMessage(&m_active_shared_task, to);
	}

	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id == TASKSLOTEMPTY) {
			continue;
		}

		ShowClientTaskInfoMessage(&active_quest, to);
	}

	to->SendChatLineBreak();
}

// TODO: Shared Task
int ClientTaskState::TaskTimeLeft(int task_id)
{

	// type "task"
	if (m_active_task.task_id == task_id) {
		int time_now = time(nullptr);

		TaskInformation *p_task_data = task_manager->m_task_data[task_id];
		if (p_task_data == nullptr) {
			return -1;
		}

		if (!p_task_data->duration) {
			return -1;
		}

		int time_left = (m_active_task.accepted_time + p_task_data->duration - time_now);

		return (time_left > 0 ? time_left : 0);
	}

	// type "shared task"
	if (m_active_shared_task.task_id == task_id) {
		int time_now = time(nullptr);

		TaskInformation *p_task_data = task_manager->m_task_data[task_id];
		if (p_task_data == nullptr) {
			return -1;
		}

		if (!p_task_data->duration) {
			return -1;
		}

		int time_left = (m_active_shared_task.accepted_time + p_task_data->duration - time_now);

		return (time_left > 0 ? time_left : 0);
	}

	if (m_active_task_count == 0) {
		return -1;
	}

	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id != task_id) {
			continue;
		}

		int time_now = time(nullptr);

		TaskInformation *p_task_data = task_manager->m_task_data[active_quest.task_id];
		if (p_task_data == nullptr) {
			return -1;
		}

		if (!p_task_data->duration) {
			return -1;
		}

		int time_left = (active_quest.accepted_time + p_task_data->duration - time_now);

		// If Timeleft is negative, return 0, else return the number of seconds left

		return (time_left > 0 ? time_left : 0);
	}

	return -1;
}

int ClientTaskState::IsTaskCompleted(int task_id)
{

	// Returns:	-1 if RecordCompletedTasks is not true
	//		+1 if the task has been completed
	//		0 if the task has not been completed

	if (!(RuleB(TaskSystem, RecordCompletedTasks))) {
		return -1;
	}

	for (auto &completed_task : m_completed_tasks) {
		LogTasks("[IsTaskCompleted] Comparing compelted task [{}] with [{}]", completed_task.task_id, task_id);
		if (completed_task.task_id == task_id) {
			return 1;
		}
	}

	return 0;
}

bool ClientTaskState::TaskOutOfTime(TaskType task_type, int index)
{
	// Returns true if the Task in the specified slot has a time limit that has been exceeded.
	auto info = GetClientTaskInfo(task_type, index);
	if (info == nullptr) {
		return false;
	}

	// make sure the task_id is at least maybe in our array
	if (info->task_id <= 0 || info->task_id >= MAXTASKS) {
		return false;
	}

	int             time_now   = time(nullptr);
	TaskInformation *task_data = task_manager->m_task_data[info->task_id];
	if (task_data == nullptr) {
		return false;
	}

	return (task_data->duration && (info->accepted_time + task_data->duration <= time_now));
}

void ClientTaskState::TaskPeriodicChecks(Client *client)
{

	// type "task"
	if (m_active_task.task_id != TASKSLOTEMPTY) {
		if (TaskOutOfTime(TaskType::Task, TASKSLOTTASK)) {
			// Send Red Task Failed Message
			client->SendTaskFailed(m_active_task.task_id, TASKSLOTTASK, TaskType::Task);
			// Remove the task from the client
			client->CancelTask(TASKSLOTTASK, TaskType::Task);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			return;
		}
	}

	// type "shared"
	if (m_active_shared_task.task_id != TASKSLOTEMPTY) {
		if (TaskOutOfTime(TaskType::Shared, TASKSLOTSHAREDTASK)) {
			// Send Red Task Failed Message
			client->SendTaskFailed(m_active_shared_task.task_id, TASKSLOTSHAREDTASK, TaskType::Shared);
			// Remove the task from the client
			client->CancelTask(TASKSLOTSHAREDTASK, TaskType::Shared);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			return;
		}
	}

	if (m_active_task_count == 0) {
		return;
	}

	// Check for tasks that have failed because they have not been completed in the specified time
	//
	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++) {
		if (m_active_quests[task_index].task_id == TASKSLOTEMPTY) {
			continue;
		}

		if (TaskOutOfTime(TaskType::Quest, task_index)) {
			// Send Red Task Failed Message
			client->SendTaskFailed(m_active_quests[task_index].task_id, task_index, TaskType::Quest);
			// Remove the task from the client
			client->CancelTask(task_index, TaskType::Quest);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			break;
		}
	}

	// Check for activities that require zoning into a specific zone.
	// This is done in this method because it gives an extra few seconds for the client screen to display
	// the zone before we send the 'Task activity_information Completed' message.
	//
	if (!m_checked_touch_activities) {
		UpdateTasksOnTouch(client, zone->GetZoneID());
		m_checked_touch_activities = true;
	}
}

bool ClientTaskState::IsTaskActivityCompleted(TaskType task_type, int index, int activity_id)
{
	switch (task_type) {
		case TaskType::Task:
			if (index != TASKSLOTTASK) {
				return false;
			}
			return m_active_task.activity[activity_id].activity_state == ActivityCompleted;
		case TaskType::Shared:
			if (index != TASKSLOTSHAREDTASK) {
				return false;
			}
			return m_active_shared_task.activity[activity_id].activity_state == ActivityCompleted;
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return m_active_quests[index].activity[activity_id].activity_state == ActivityCompleted;
			}
		default:
			return false;
	}

}

// should we be defaulting to hidden?
ActivityState ClientTaskState::GetTaskActivityState(TaskType task_type, int index, int activity_id)
{
	ActivityState return_state = ActivityHidden;
	switch (task_type) {
		case TaskType::Task:
			if (index != TASKSLOTTASK) {
				return_state = ActivityHidden;
				break;
			}
			return_state = m_active_task.activity[activity_id].activity_state;
			break;
		case TaskType::Shared:
			if (index != TASKSLOTSHAREDTASK) {
				return_state = ActivityHidden;
				break;
			}
			return_state = m_active_shared_task.activity[activity_id].activity_state;
			break;
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return_state = m_active_quests[index].activity[activity_id].activity_state;
				break;
			}
			break;
		default:
			return_state = ActivityHidden;
	}

	LogTasksDetail(
		"-- [GetTaskActivityState] task_type [{}] ({}) index [{}] activity_id [{}] activity_state [{}] ({})",
		Tasks::GetTaskTypeIdentifier(task_type),
		Tasks::GetTaskTypeDescription(task_type),
		index,
		activity_id,
		Tasks::GetActivityStateIdentifier(return_state),
		Tasks::GetActivityStateDescription(return_state)
	);

	return return_state;
}

int ClientTaskState::GetTaskActivityDoneCount(TaskType task_type, int index, int activity_id)
{
	switch (task_type) {
		case TaskType::Task:
			if (index != TASKSLOTTASK) {
				return 0;
			}
			return m_active_task.activity[activity_id].done_count;
		case TaskType::Shared:
			if (index != TASKSLOTSHAREDTASK) {
				return 0;
			}
			return m_active_shared_task.activity[activity_id].done_count;
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return m_active_quests[index].activity[activity_id].done_count;
			}
		default:
			return 0;
	}
}

int ClientTaskState::GetTaskActivityDoneCountFromTaskID(int task_id, int activity_id)
{

	// type "task"
	if (m_active_task.task_id == task_id) {
		return m_active_task.activity[activity_id].done_count;
	}

	// type "shared"
	if (m_active_shared_task.task_id == task_id) {
		return m_active_shared_task.activity[activity_id].done_count;
	}

	int active_task_index = -1;

	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++) {
		if (m_active_quests[task_index].task_id == task_id) {
			active_task_index = task_index;
			break;
		}
	}

	if (active_task_index == -1) {
		return 0;
	}

	if (m_active_quests[active_task_index].activity[activity_id].done_count) {
		return m_active_quests[active_task_index].activity[activity_id].done_count;
	}
	else {
		return 0;
	}
}

int ClientTaskState::GetTaskStartTime(TaskType task_type, int index)
{
	switch (task_type) {
		case TaskType::Task:
			return m_active_task.accepted_time;
		case TaskType::Quest:
			return m_active_quests[index].accepted_time;
		case TaskType::Shared:
			return m_active_shared_task.accepted_time;
		default:
			return -1;
	}
}

void ClientTaskState::CancelAllTasks(Client *client)
{

	// This method exists solely to be called during #task reloadall
	// It removes tasks from the in-game client state ready for them to be
	// resent to the client, in case an updated task fails to load

	// task
	// these cancels lock up the client for some reason
	CancelTask(client, TASKSLOTTASK, TaskType::Task, false);
	m_active_task.task_id = TASKSLOTEMPTY;

	// shared task
	CancelTask(client, TASKSLOTSHAREDTASK, TaskType::Shared, false);
	m_active_shared_task.task_id = TASKSLOTEMPTY;

	// "quests"
	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++)
		if (m_active_quests[task_index].task_id != TASKSLOTEMPTY) {
			CancelTask(client, task_index, TaskType::Quest, false);
			m_active_quests[task_index].task_id = TASKSLOTEMPTY;
		}

	// TODO: shared
}

void ClientTaskState::CancelTask(Client *c, int sequence_number, TaskType task_type, bool remove_from_db)
{
	LogTasks("CancelTask");

	// shared task middleware
	// intercept and pass to world first before processing normally
	if (!c->m_requested_shared_task_removal && task_type == TaskType::Shared && m_active_shared_task.task_id != 0) {

		// struct
		auto pack = new ServerPacket(ServerOP_SharedTaskAttemptRemove, sizeof(ServerSharedTaskAttemptRemove_Struct));
		auto *r   = (ServerSharedTaskAttemptRemove_Struct *) pack->pBuffer;

		// fill
		r->requested_character_id = c->CharacterID();
		r->requested_task_id      = m_active_shared_task.task_id;
		r->remove_from_db         = remove_from_db;

		// send
		worldserver.SendPacket(pack);
		safe_delete(pack);

		return;
	}

	// packet
	auto outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));

	// fill
	auto *cts = (CancelTask_Struct *) outapp->pBuffer;
	cts->SequenceNumber = sequence_number;
	cts->type           = static_cast<uint32>(task_type);

	// send
	c->QueuePacket(outapp);
	safe_delete(outapp);

	// persistence
	if (remove_from_db) {
		RemoveTask(c, sequence_number, task_type);
	}
}

void ClientTaskState::KickPlayersSharedTask(Client* client)
{
	uint32_t pack_size = sizeof(ServerSharedTaskKickPlayers_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_SharedTaskKickPlayers, pack_size);
	auto buf = reinterpret_cast<ServerSharedTaskKickPlayers_Struct*>(pack->pBuffer);

	buf->source_character_id = client->CharacterID();
	buf->task_id             = m_active_shared_task.task_id;

	worldserver.SendPacket(pack.get());
}

void ClientTaskState::RemoveTask(Client *client, int sequence_number, TaskType task_type)
{
	int character_id = client->CharacterID();
	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState Cancel Task %i ", sequence_number);

	int task_id = -1;
	switch (task_type) {
		case TaskType::Task:
			if (sequence_number == TASKSLOTTASK) {
				task_id = m_active_task.task_id;
			}
			break;
		case TaskType::Quest:
			if (sequence_number < MAXACTIVEQUESTS) {
				task_id = m_active_quests[sequence_number].task_id;
			}
			break;
		case TaskType::Shared:
			if (sequence_number == TASKSLOTSHAREDTASK) {
				task_id = m_active_shared_task.task_id;
			}
			break;
		default:
			break;
	}

	CharacterActivitiesRepository::DeleteWhere(
		database,
		fmt::format("charid = {} AND taskid = {}", character_id, task_id)
	);

	CharacterTasksRepository::DeleteWhere(
		database,
		fmt::format("charid = {} AND taskid = {} AND type = {}", character_id, task_id, static_cast<int>(task_type))
	);

	switch (task_type) {
		case TaskType::Task:
			m_active_task.task_id = TASKSLOTEMPTY;
			break;
		case TaskType::Shared:
			m_active_shared_task.task_id = TASKSLOTEMPTY;
			break;
		case TaskType::Quest:
			m_active_quests[sequence_number].task_id = TASKSLOTEMPTY;
			m_active_task_count--;
			break;
		default:
			break;
	}
}

void ClientTaskState::RemoveTaskByTaskID(Client *client, uint32 task_id)
{
	switch (task_manager->GetTaskType(task_id)) {
		case TaskType::Task: {
			if (m_active_task.task_id == task_id) {
				LogTasks("[UPDATE] RemoveTaskByTaskID found Task [{}]", task_id);
				CancelTask(client, TASKSLOTTASK, TaskType::Task, true);
			}
			break;
		}
		case TaskType::Shared: {
			if (m_active_shared_task.task_id == task_id) {
				LogTasks("[UPDATE] RemoveTaskByTaskID found Shared Task [{}]", task_id);
				CancelTask(client, TASKSLOTSHAREDTASK, TaskType::Shared, true);
			}
			break;
		}
		case TaskType::Quest: {
			for (int active_quest = 0; active_quest < MAXACTIVEQUESTS; active_quest++) {
				if (m_active_quests[active_quest].task_id == task_id) {
					LogTasks("[UPDATE] RemoveTaskByTaskID found Quest [{}] at index [{}]", task_id, active_quest);
					CancelTask(client, active_quest, TaskType::Quest, true);
				}
			}
		}
		default: {
			break;
		}
	}
}

void ClientTaskState::AcceptNewTask(
	Client *client,
	int task_id,
	int npc_type_id,
	time_t accept_time,
	bool enforce_level_requirement
)
{
	if (!task_manager || task_id < 0 || task_id >= MAXTASKS) {
		client->Message(Chat::Red, "Task system not functioning, or task_id %i out of range.", task_id);
		return;
	}

	auto task = task_manager->m_task_data[task_id];
	if (task == nullptr) {
		client->Message(Chat::Red, "Invalid task_id %i", task_id);
		return;
	}

	// shared task
	// intercept and pass to world first before processing normally
	if (!client->m_requesting_shared_task && task->type == TaskType::Shared) {
		LogTasksDetail(
			"[AcceptNewTask] Initiating shared_task request | task_id [{}] character_id [{}] name [{}]",
			task_id,
			client->CharacterID(),
			client->GetCleanName()
		);

		// struct
		auto pack = new ServerPacket(ServerOP_SharedTaskRequest, sizeof(ServerSharedTaskRequest_Struct));
		auto *r   = (ServerSharedTaskRequest_Struct *) pack->pBuffer;

		// fill
		r->requested_character_id = client->CharacterID();
		r->requested_task_id      = task_id;
		r->requested_npc_type_id  = npc_type_id;

		// send
		worldserver.SendPacket(pack);
		safe_delete(pack);

		return;
	}

	bool max_tasks = false;

	switch (task->type) {
		case TaskType::Task:
			if (m_active_task.task_id != TASKSLOTEMPTY) {
				max_tasks = true;
			}
			break;
		case TaskType::Shared: // TODO: shared tasks
			if (m_active_shared_task.task_id != TASKSLOTEMPTY) {
				max_tasks = true;
			}
			break;
		case TaskType::Quest:
			if (m_active_task_count == MAXACTIVEQUESTS) {
				max_tasks = true;
			}
			break;
		default:
			break;
	}

	if (max_tasks) {
		client->MessageString(Chat::Yellow, MAX_ACTIVE_TASKS, ".", ".", client->GetName());
		return;
	}

	// only Quests can have more than one, so don't need to check others
	if (task->type == TaskType::Quest) {
		for (auto &active_quest : m_active_quests) {
			if (active_quest.task_id == task_id) {
				// live doesn't have an eqstr for it but this seems to be the string used for this scenario
				client->Message(Chat::Yellow, "You are already working on a task for this person, you must finish it before asking for another.");
				return;
			}
		}
	}

	if (enforce_level_requirement && !task_manager->ValidateLevel(task_id, client->GetLevel())) {
		client->MessageString(Chat::Yellow, TASK_NOT_RIGHT_LEVEL);
		return;
	}

	if (!task_manager->IsTaskRepeatable(task_id) && IsTaskCompleted(task_id)) {
		return;
	}

	// solo task timer lockout validation
	if (task->type != TaskType::Shared)
	{
		auto task_timers = CharacterTaskTimersRepository::GetWhere(database, fmt::format(
			"character_id = {} AND task_id = {} AND expire_time > NOW() ORDER BY timer_type ASC LIMIT 1",
			client->CharacterID(), task_id
		));

		if (!task_timers.empty())
		{
			auto timer_type = static_cast<TaskTimerType>(task_timers.front().timer_type);
			auto seconds = task_timers.front().expire_time - std::time(nullptr);
			auto days  = fmt::format_int(seconds / 86400).str();
			auto hours = fmt::format_int((seconds / 3600) % 24).str();
			auto mins  = fmt::format_int((seconds / 60) % 60).str();

			// these solo task messages are in SharedTaskMessage for convenience
			namespace EQStr = SharedTaskMessage;
			if (timer_type == TaskTimerType::Replay)
			{
				int eqstr_id = EQStr::TASK_ASSIGN_WAIT_REPLAY_TIMER;
				client->MessageString(Chat::Red, eqstr_id, days.c_str(), hours.c_str(), mins.c_str());
			}
			else if (timer_type == TaskTimerType::Request)
			{
				int eqstr_id = EQStr::TASK_ASSIGN_WAIT_REQUEST_TIMER;
				client->Message(Chat::Red, fmt::format(EQStr::GetEQStr(eqstr_id), days, hours, mins).c_str());
			}

			return;
		}
	}

	// We do it this way, because when the Client cancels a task, it retains the sequence number of the remaining
	// tasks in it's window, until something causes the TaskDescription packets to be sent again. We could just
	// resend all the active task data to the client when it cancels a task, but that could be construed as a
	// waste of bandwidth.
	//
	ClientTaskInformation *active_slot = nullptr;
	switch (task->type) {
		case TaskType::Task:
			active_slot = &m_active_task;
			break;

		case TaskType::Shared:
			active_slot = &m_active_shared_task;
			break;

		case TaskType::Quest:
			for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++) {
				Log(Logs::General, Logs::Tasks,
					"[UPDATE] ClientTaskState Looking for free slot in slot %i, found task_id of %i", task_index,
					m_active_quests[task_index].task_id);
				if (m_active_quests[task_index].task_id == 0) {
					active_slot = &m_active_quests[task_index];
					break;
				}
			}
			break;

		default:
			break;
	}

	// This shouldn't happen unless there is a bug in the handling of ActiveTaskCount somewhere
	if (active_slot == nullptr) {
		client->MessageString(Chat::Yellow, MAX_ACTIVE_TASKS, ".", ".", client->GetName());
		return;
	}

	active_slot->task_id       = task_id;
	active_slot->accepted_time = static_cast<int>(accept_time);
	active_slot->updated       = true;
	active_slot->current_step  = -1;

	for (int activity_id = 0; activity_id < task_manager->m_task_data[task_id]->activity_count; activity_id++) {
		active_slot->activity[activity_id].activity_id    = activity_id;
		active_slot->activity[activity_id].done_count     = 0;
		active_slot->activity[activity_id].activity_state = ActivityHidden;
		active_slot->activity[activity_id].updated        = true;
	}

	UnlockActivities(client->CharacterID(), *active_slot);

	if (task->type == TaskType::Quest) {
		m_active_task_count++;
	}

	// add request timer (shared task timers are added to members by world)
	if (task->request_timer_seconds > 0)
	{
		auto expire_time = active_slot->accepted_time + task->request_timer_seconds;

		auto seconds = expire_time - std::time(nullptr);
		if (seconds > 0) // not already expired
		{
			if (task->type != TaskType::Shared)
			{
				auto timer = CharacterTaskTimersRepository::NewEntity();
				timer.character_id = client->CharacterID();
				timer.task_id      = task_id;
				timer.timer_type   = static_cast<int>(TaskTimerType::Request);
				timer.expire_time  = expire_time;

				CharacterTaskTimersRepository::InsertOne(database, timer);
			}

			client->Message(Chat::Yellow, fmt::format(
				SharedTaskMessage::GetEQStr(SharedTaskMessage::RECEIVED_REQUEST_TIMER),
				task->title,
				fmt::format_int(seconds / 86400).c_str(),       // days
				fmt::format_int((seconds / 3600) % 24).c_str(), // hours
				fmt::format_int((seconds / 60) % 60).c_str()    // minutes
			).c_str());
		}
	}

	task_manager->SendSingleActiveTaskToClient(client, *active_slot, false, true);
	client->StartTaskRequestCooldownTimer();
	client->MessageString(Chat::White, YOU_ASSIGNED_TASK, task->title.c_str());

	task_manager->SaveClientState(client, this);
	std::string export_string = std::to_string(task_id);

	NPC *npc = entity_list.GetID(npc_type_id)->CastToNPC();
	if (npc) {
		parse->EventNPC(EVENT_TASK_ACCEPTED, npc, client, export_string, 0);
	}
}

void ClientTaskState::ProcessTaskProximities(Client *client, float x, float y, float z)
{
	float last_x = client->ProximityX();
	float last_y = client->ProximityY();
	float last_z = client->ProximityZ();

	if ((last_x == x) && (last_y == y) && (last_z == z)) {
		return;
	}

	LogTasksDetail("[ProcessTaskProximities] Checking proximities for Position x[{}] y[{}] z[{}]", x, y, z);
	int explore_id = task_manager->m_proximity_manager.CheckProximities(x, y, z);
	if (explore_id > 0) {
		LogTasksDetail(
			"[ProcessTaskProximities] Position x[{}] y[{}] z[{}] is within proximity explore_id [{}]",
			x,
			y,
			z,
			explore_id
		);

		UpdateTasksOnExplore(client, explore_id);
	}
}

void ClientTaskState::SharedTaskIncrementDoneCount(
	Client *client,
	int task_id,
	int activity_id,
	int done_count,
	bool ignore_quest_update
)
{
	TaskInformation *t = task_manager->m_task_data[task_id];

	auto info = GetClientTaskInfo(t->type, TASKSLOTSHAREDTASK);
	if (info == nullptr) {
		return;
	}

	// absolute value update
	info->activity[activity_id].done_count = done_count;

	LogTasksDetail(
		"[SharedTaskIncrementDoneCount] Setting task_id [{}] to absolute done_count value of [{}] via increment [{}]",
		task_id,
		info->activity[activity_id].done_count,
		done_count
	);

	IncrementDoneCount(
		client,
		t,
		TASKSLOTSHAREDTASK,
		activity_id,
		0, // no op
		ignore_quest_update
	);
}

const ClientTaskInformation &ClientTaskState::GetActiveSharedTask() const
{
	return m_active_shared_task;
}

bool ClientTaskState::HasActiveSharedTask()
{
	return GetActiveSharedTask().task_id != 0;
}

void ClientTaskState::CreateTaskDynamicZone(Client* client, int task_id, DynamicZone& dz_request)
{
	auto task = task_manager->m_task_data[task_id];
	if (!task)
	{
		return;
	}

	// dz should be named the version-based zone name (used in choose zone window and dz window on live)
	auto zone_info = zone_store.GetZone(dz_request.GetZoneID(), dz_request.GetZoneVersion());
	dz_request.SetName(zone_info.long_name.empty() ? task->title : zone_info.long_name);
	dz_request.SetMinPlayers(task->min_players);
	dz_request.SetMaxPlayers(task->max_players);

	// a task might create a dz from an objective so override dz duration to time remaining
	// live probably creates the dz with the shared task and just adds members for objectives
	std::chrono::seconds seconds(TaskTimeLeft(task_id));
	if (task->duration == 0 || seconds.count() < 0)
	{
		// todo: maybe add a rule for duration
		// cap unlimited duration tasks so instance isn't held indefinitely
		// expected behavior is to re-acquire any unlimited tasks that have an expired dz
		seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours(24));
	}

	dz_request.SetDuration(static_cast<uint32_t>(seconds.count()));

	if (task->type == TaskType::Task || task->type == TaskType::Quest)
	{
		if (task->type == TaskType::Task) {
			dz_request.SetType(DynamicZoneType::Task);
		} else {
			dz_request.SetType(DynamicZoneType::Quest);
		}

		// todo: can enable non-shared task dz creation when dz ids are persisted for them (db also)
		//DynamicZoneMember solo_member{ client->CharacterID(), client->GetCleanName() };
		//DynamicZone::CreateNew(dz_request, { solo_member });
	}
	else if (task->type == TaskType::Shared)
	{
		dz_request.SetType(DynamicZoneType::Mission);

		// shared task missions are created in world
		EQ::Net::DynamicPacket dyn_pack = dz_request.GetSerializedDzPacket();

		auto pack_size = sizeof(ServerSharedTaskCreateDynamicZone_Struct) + dyn_pack.Length();
		auto pack = std::make_unique<ServerPacket>(ServerOP_SharedTaskCreateDynamicZone, static_cast<uint32_t>(pack_size));
		auto buf = reinterpret_cast<ServerSharedTaskCreateDynamicZone_Struct*>(pack->pBuffer);
		buf->source_character_id = client->CharacterID();
		buf->task_id = task_id;
		buf->cereal_size = static_cast<uint32_t>(dyn_pack.Length());
		memcpy(buf->cereal_data, dyn_pack.Data(), dyn_pack.Length());

		worldserver.SendPacket(pack.get());
	}

}

void ClientTaskState::ListTaskTimers(Client* client)
{
	LogTasksDetail("[ListTaskTimers] Client [{}]", client->GetCleanName());

	// this isn't live-like but we need to throttle query (alternative is caching timers)
	if (!client->m_list_task_timers_rate_limit.Check()) {
		client->Message(Chat::White, "Sending messages too fast");
		return;
	}

	auto character_task_timers = CharacterTaskTimersRepository::GetWhere(
		database, fmt::format(
			"character_id = {} AND expire_time > NOW()",
			client->CharacterID()
		)
	);

	for (const auto& task_timer : character_task_timers)
	{
		auto task = task_manager->m_task_data[task_timer.task_id];
		if (task)
		{
			auto timer_type = static_cast<TaskTimerType>(task_timer.timer_type);
			auto seconds = task_timer.expire_time - std::time(nullptr);
			auto days  = fmt::format_int(seconds / 86400).str();
			auto hours = fmt::format_int((seconds / 3600) % 24).str();
			auto mins  = fmt::format_int((seconds / 60) % 60).str();

			if (timer_type == TaskTimerType::Replay)
			{
				client->MessageString(Chat::Yellow, SharedTaskMessage::REPLAY_TIMER_REMAINING,
					task->title.c_str(), days.c_str(), hours.c_str(), mins.c_str());
			}
			else
			{
				auto eqstr = SharedTaskMessage::GetEQStr(SharedTaskMessage::REQUEST_TIMER_REMAINING);
				client->Message(Chat::Yellow, fmt::format(eqstr, task->title, days, hours, mins).c_str());
			}
		}
	}

	if (character_task_timers.empty()) {
		client->MessageString(Chat::Yellow, SharedTaskMessage::YOU_NO_CURRENT_REPLAY_TIMERS);
	}
}

void ClientTaskState::AddReplayTimer(Client* client, ClientTaskInformation& client_task, TaskInformation& task)
{
	// world adds timers for shared tasks and handles messages
	if (task.type != TaskType::Shared && task.replay_timer_seconds > 0)
	{
		// solo task replay timers are based on completion time
		auto expire_time = std::time(nullptr) + task.replay_timer_seconds;

		auto timer = CharacterTaskTimersRepository::NewEntity();
		timer.character_id = client->CharacterID();
		timer.task_id      = client_task.task_id;
		timer.expire_time  = expire_time;
		timer.timer_type   = static_cast<int>(TaskTimerType::Replay);

		CharacterTaskTimersRepository::InsertOne(database, timer);

		client->Message(Chat::Yellow, fmt::format(
			SharedTaskMessage::GetEQStr(SharedTaskMessage::RECEIVED_REPLAY_TIMER),
			task.title,
			fmt::format_int(task.replay_timer_seconds / 86400).c_str(),       // days
			fmt::format_int((task.replay_timer_seconds / 3600) % 24).c_str(), // hours
			fmt::format_int((task.replay_timer_seconds / 60) % 60).c_str()    // minutes
		).c_str());
	}
}

// Sync zone client donecount state
//
// World is always authoritative, but we still need to keep zone state in sync with reality and in this case we need
// to update zone since world hasn't had a chance to let clients at the zone level know it's ok to progress to the next
// donecount
//
// If we send updates too fast and world has not had a chance to confirm and then inform clients to set their
// absolute state we will miss and discard updates because each update sets the same donecount
//
// Example: say we have a 100 kill task and we kill 10 mobs in an AOE, world gets 10 updates at once but they are
// all from the same donecount so world only confirms 1 was actually killed and the task updates only 1 which is not
// intended behavior.
//
// In this case we need to ensure that clients at the zone level increment internally even if they aren't
// necessarily confirmed by world yet because any of them could inform world of the next donecount so we need to sync
// zone-level before sending updates to world
void ClientTaskState::SyncSharedTaskZoneClientDoneCountState(
	Client *p_client,
	TaskInformation *p_information,
	int task_index,
	int activity_id,
	uint32 done_count
)
{
	for (auto &e : entity_list.GetClientList()) {
		auto c = e.second;
		if (c->GetSharedTaskId() == p_client->GetSharedTaskId()) {
			auto t = c->GetTaskState()->GetClientTaskInfo(p_information->type, task_index);
			if (t == nullptr) {
				continue;
			}

			LogTasksDetail(
				"[IncrementDoneCount] Setting internally client [{}] to donecount [{}]",
				c->GetCleanName(),
				done_count
			);

			t->activity[activity_id].done_count = (int) done_count;
		}
	}
}

bool ClientTaskState::HasActiveTasks()
{
	if (!task_manager) {
		return false;
	}

	if (m_active_task.task_id != TASKSLOTEMPTY) {
		return true;
	}

	if (m_active_shared_task.task_id != TASKSLOTEMPTY) {
		return true;
	}

	bool has_active_quest = false;
	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id == TASKSLOTEMPTY) {
			continue;
		}

		return true;
	}

	return false;
}
