#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/repositories/character_activities_repository.h"
#include "../common/repositories/character_tasks_repository.h"
#include "../common/repositories/completed_tasks_repository.h"
#include "../common/rulesys.h"
#include "client.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "task_client_state.h"
#include "zonedb.h"

extern QueryServ *QServ;

ClientTaskState::ClientTaskState()
{
	m_active_task_count          = 0;
	m_last_completed_task_loaded = 0;
	m_checked_touch_activities   = false;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		m_active_quests[i].slot    = i;
		m_active_quests[i].task_id = TASKSLOTEMPTY;
	}

	m_active_task.slot    = 0;
	m_active_task.task_id = TASKSLOTEMPTY;
	// TODO: shared task
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
			task_history_reply_data_1->ActivityType = p_task_data->activity_information[i].activity_type;
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
			return false; // todo
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

	TaskInformation *p_task_data = task_manager->m_task_data[task_info.task_id];
	if (p_task_data == nullptr) {
		return true;
	}

	// On loading the client state, all activities that are not completed, are
	// marked as hidden. For Sequential (non-stepped) mode, we mark the first
	// activity_information as active if not complete.
	LogTasks(
		"character_id [{}] task_id [{}] sequence_mode [{}]",
		character_id,
		task_info.task_id,
		p_task_data->sequence_mode
	);

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

			CompletedTaskInformation completed_task_information{};
			completed_task_information.task_id        = task_info.task_id;
			completed_task_information.completed_time = time(nullptr);

			for (int i = 0; i < p_task_data->activity_count; i++) {
				completed_task_information.activity_done[i] = (task_info.activity[i].activity_state == ActivityCompleted);
			}

			m_completed_tasks.push_back(completed_task_information);
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

			CompletedTaskInformation completed_task_information{};
			completed_task_information.task_id        = task_info.task_id;
			completed_task_information.completed_time = time(nullptr);

			for (int activity_id = 0; activity_id < p_task_data->activity_count; activity_id++) {
				completed_task_information.activity_done[activity_id] =
					(task_info.activity[activity_id].activity_state == ActivityCompleted);
			}

			m_completed_tasks.push_back(completed_task_information);
		}
		return true;
	}

	// Mark all non-completed tasks in the current step as active
	for (int activity = 0; activity < p_task_data->activity_count; activity++) {
		if ((p_task_data->activity_information[activity].step_number == (int) task_info.current_step) &&
			(task_info.activity[activity].activity_state == ActivityHidden)) {
			task_info.activity[activity].activity_state = ActivityActive;
			task_info.activity[activity].updated        = true;
		}
	}

	return false;
}

void ClientTaskState::UpdateTasksOnKill(Client *client, int npc_type_id)
{
	UpdateTasksByNPC(client, ActivityKill, npc_type_id);
}

bool ClientTaskState::UpdateTasksOnSpeakWith(Client *client, int npc_type_id)
{
	return UpdateTasksByNPC(client, ActivitySpeakWith, npc_type_id);
}

bool ClientTaskState::UpdateTasksByNPC(Client *client, int activity_type, int npc_type_id)
{

	int is_updating = false;

	// If the client has no tasks, there is nothing further to check.
	if (!task_manager || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
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
					activity_type,
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
	if (!task_manager || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
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
			if (activity_info->activity_type != ActivitySpeakWith) {
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
	if (!task_manager || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
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
			if (activity_info->activity_type != ActivitySpeakWith) {
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

void ClientTaskState::UpdateTasksForItem(Client *client, ActivityType activity_type, int item_id, int count)
{

	// This method updates the client's task activities of the specified type which relate
	// to the specified item.
	//
	// Type should be one of ActivityLoot, ActivityTradeSkill, ActivityFish or ActivityForage

	// If the client has no tasks, there is nothing further to check.

	LogTasks(
		"[UpdateTasksForItem] activity_type [{}] item_id [{}]",
		activity_type,
		item_id
	);

	if (!task_manager || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
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
			if (activity_info->activity_type != (int) activity_type) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!activity_info->CheckZone(zone->GetZoneID())) {
				LogTasks(
					"[UpdateTasksForItem] Error: Character [{}] activity_information type [{}] for Item [{}] failed zone check",
					client->GetName(),
					activity_type,
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
	if (!task_manager || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
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
			if (activity_info->activity_type != ActivityExplore) {
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
	if (!task_manager || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
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
			if (activity_info->activity_type != ActivityDeliver &&
				activity_info->activity_type != ActivityGiveCash) {
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
			if ((activity_info->activity_type == ActivityGiveCash) && cash) {
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
								item->GetID())) {
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
	if (!task_manager || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
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
			if (activity_info->activity_type != ActivityTouch) {
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
	Log(Logs::General, Logs::Tasks, "[UPDATE] IncrementDoneCount");

	auto info = GetClientTaskInfo(task_information->type, task_index);
	if (info == nullptr) {
		return;
	}

	info->activity[activity_id].done_count += count;

	if (info->activity[activity_id].done_count > task_information->activity_information[activity_id].goal_count) {
		info->activity[activity_id].done_count = task_information->activity_information[activity_id].goal_count;
	}

	if (!ignore_quest_update) {
		char buf[24];
		snprintf(
			buf,
			23,
			"%d %d %d",
			info->activity[activity_id].done_count,
			info->activity[activity_id].activity_id,
			info->task_id
		);
		buf[23] = '\0';
		parse->EventPlayer(EVENT_TASK_UPDATE, client, buf, 0);
	}

	info->activity[activity_id].updated = true;
	// Have we reached the goal count for this activity_information ?
	if (info->activity[activity_id].done_count >= task_information->activity_information[activity_id].goal_count) {
		Log(
			Logs::General, Logs::Tasks, "[UPDATE] Done (%i) = Goal (%i) for activity_information %i",
			info->activity[activity_id].done_count,
			task_information->activity_information[activity_id].goal_count,
			activity_id
		);

		// Flag the activity_information as complete
		info->activity[activity_id].activity_state = ActivityCompleted;
		// Unlock subsequent activities for this task
		bool task_complete = UnlockActivities(client->CharacterID(), *info);
		Log(Logs::General, Logs::Tasks, "[UPDATE] TaskCompleted is %i", task_complete);
		// and by the 'Task Stage Completed' message
		client->SendTaskActivityComplete(info->task_id, activity_id, task_index, task_information->type);
		// Send the updated task/activity_information list to the client
		task_manager->SendSingleActiveTaskToClient(client, *info, task_complete, false);
		// Inform the client the task has been updated, both by a chat message
		client->Message(Chat::White, "Your task '%s' has been updated.", task_information->title.c_str());

		if (task_information->activity_information[activity_id].goal_method != METHODQUEST) {
			if (!ignore_quest_update) {
				char buf[24];
				snprintf(buf, 23, "%d %d", info->task_id, info->activity[activity_id].activity_id);
				buf[23] = '\0';
				parse->EventPlayer(EVENT_TASK_STAGE_COMPLETE, client, buf, 0);
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
		}

		// If this task is now complete, the Completed tasks will have been
		// updated in UnlockActivities. Send the completed task list to the
		// client. This is the same sequence the packets are sent on live.
		if (task_complete) {
			char buf[24];
			snprintf(
				buf,
				23,
				"%d %d %d",
				info->activity[activity_id].done_count,
				info->activity[activity_id].activity_id,
				info->task_id
			);
			buf[23] = '\0';
			parse->EventPlayer(EVENT_TASK_COMPLETE, client, buf, 0);

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

			task_manager->SendCompletedTasksToClient(client, this);
			client->SendTaskActivityComplete(info->task_id, 0, task_index, task_information->type, 0);
			task_manager->SaveClientState(client, this);
			//c->SendTaskComplete(TaskIndex);
			client->CancelTask(task_index, task_information->type);
			//if(Task->reward_method != METHODQUEST) RewardTask(c, Task);
			// If Experience and/or cash rewards are set, reward them from the task even if reward_method is METHODQUEST
			RewardTask(client, task_information);
			//RemoveTask(c, TaskIndex);

		}

	}
	else {
		// Send an updated packet for this single activity_information
		task_manager->SendTaskActivityLong(
			client,
			info->task_id,
			activity_id,
			task_index,
			task_information->activity_information[activity_id].optional
		);
		task_manager->SaveClientState(client, this);
	}
}

void ClientTaskState::RewardTask(Client *client, TaskInformation *task_information)
{

	if (!task_information || !client) {
		return;
	}

	const EQ::ItemData *item_data;
	std::vector<int>   reward_list;

	switch (task_information->reward_method) {
		case METHODSINGLEID: {
			if (task_information->reward_id) {
				client->SummonItem(task_information->reward_id);
				item_data = database.GetItem(task_information->reward_id);
				if (item_data) {
					client->Message(Chat::Yellow, "You receive %s as a reward.", item_data->Name);
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
					client->Message(Chat::Yellow, "You receive %s as a reward.", item_data->Name);
				}
			}
			break;
		}
		default: {
			// Nothing special done for METHODQUEST
			break;
		}
	}

	if (!task_information->completion_emote.empty()) {
		client->SendColoredText(
			Chat::Yellow,
			task_information->completion_emote
		);
	} // unsure if they use this packet or color, should work

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

		std::string cash_message;

		if (platinum > 0) {
			cash_message = "You receive ";
			cash_message += itoa(platinum);
			cash_message += " platinum";
		}
		if (gold > 0) {
			if (cash_message.length() == 0) {
				cash_message = "You receive ";
			}
			else {
				cash_message += ",";
			}
			cash_message += itoa(gold);
			cash_message += " gold";
		}
		if (silver > 0) {
			if (cash_message.length() == 0) {
				cash_message = "You receive ";
			}
			else {
				cash_message += ",";
			}
			cash_message += itoa(silver);
			cash_message += " silver";
		}
		if (copper > 0) {
			if (cash_message.length() == 0) {
				cash_message = "You receive ";
			}
			else {
				cash_message += ",";
			}
			cash_message += itoa(copper);
			cash_message += " copper";
		}
		cash_message += " pieces.";
		client->Message(Chat::Yellow, cash_message.c_str());
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

	client->SendSound();
}

bool ClientTaskState::IsTaskActive(int task_id)
{
	if (m_active_task.task_id == task_id) {
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

	if (m_active_task.task_id == task_id) {
		client->SendTaskFailed(task_id, 0, TaskType::Task);
		// Remove the task from the client
		client->CancelTask(0, TaskType::Task);
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
	if (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY) {
		return false;
	}

	int  active_task_index = -1;
	auto task_type         = TaskType::Task;

	if (m_active_task.task_id == task_id) {
		active_task_index = 0;
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
	if (activity_id < 0 || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  active_task_index = -1;
	auto type              = TaskType::Task;

	if (m_active_task.task_id == task_id) {
		active_task_index = 0;
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
	if (activity_id < 0 || (m_active_task_count == 0 && m_active_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  active_task_index = -1;
	auto type              = TaskType::Task;
	if (m_active_task.task_id == task_id) {
		active_task_index = 0;
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

void ClientTaskState::ShowClientTasks(Client *client)
{
	client->Message(Chat::White, "------------------------------------------------");
	client->Message(Chat::White, "# Task Information | Client [%s]", client->GetCleanName());
//	client->Message(Chat::White, "------------------------------------------------");
	if (m_active_task.task_id != TASKSLOTEMPTY) {
		client->Message(
			Chat::White,
			"Task: %i %s",
			m_active_task.task_id,
			task_manager->m_task_data[m_active_task.task_id]->title.c_str()
		);
		client->Message(
			Chat::White,
			"  description: [%s]\n",
			task_manager->m_task_data[m_active_task.task_id]->description.c_str()
		);
		for (int activity_id = 0; activity_id < task_manager->GetActivityCount(m_active_task.task_id); activity_id++) {
			client->Message(
				Chat::White,
				"  activity_information: %2d, done_count: %2d, Status: %d (0=Hidden, 1=Active, 2=Complete)",
				m_active_task.activity[activity_id].activity_id,
				m_active_task.activity[activity_id].done_count,
				m_active_task.activity[activity_id].activity_state
			);
		}
	}

	for (auto &active_quest : m_active_quests) {
		if (active_quest.task_id == TASKSLOTEMPTY) {
			continue;
		}

		client->Message(Chat::White, "------------------------------------------------");
		client->Message(
			Chat::White, "# Quest | task_id [%i] title [%s]",
			active_quest.task_id,
			task_manager->m_task_data[active_quest.task_id]->title.c_str()
		);
		client->Message(Chat::White, "------------------------------------------------");

		client->Message(
			Chat::White,
			" -- Description [%s]\n",
			task_manager->m_task_data[active_quest.task_id]->description.c_str()
		);

		for (int activity_id = 0; activity_id < task_manager->GetActivityCount(active_quest.task_id); activity_id++) {
			std::vector<std::string> update_increments = {"1", "5", "50"};
			std::string              update_saylinks;

			for (auto &increment: update_increments) {
				auto task_update_saylink = EQ::SayLinkEngine::GenerateQuestSaylink(
					fmt::format(
						"#task update {} {} {}",
						active_quest.task_id,
						active_quest.activity[activity_id].activity_id,
						increment
					),
					false,
					increment
				);

				update_saylinks += "[" + task_update_saylink + "] ";
			}

			client->Message(
				Chat::White,
				" --- activity_id [%i] done_count [%i] state [%d] (0=hidden 1=active 2=complete) | Update %s",
				active_quest.activity[activity_id].activity_id,
				active_quest.activity[activity_id].done_count,
				active_quest.activity[activity_id].activity_state,
				update_saylinks.c_str()
			);
		}
	}

	client->Message(Chat::White, "------------------------------------------------");
}

// TODO: Shared Task
int ClientTaskState::TaskTimeLeft(int task_id)
{
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
	if (m_active_task.task_id != TASKSLOTEMPTY) {
		if (TaskOutOfTime(TaskType::Task, 0)) {
			// Send Red Task Failed Message
			client->SendTaskFailed(m_active_task.task_id, 0, TaskType::Task);
			// Remove the task from the client
			client->CancelTask(0, TaskType::Task);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			return;
		}
	}

	// TODO: shared tasks -- although that will probably be manager in world checking and telling zones to fail us

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
			if (index != 0) {
				return false;
			}
			return m_active_task.activity[activity_id].activity_state == ActivityCompleted;
		case TaskType::Shared:
			return false; // TODO: shared tasks
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
	switch (task_type) {
		case TaskType::Task:
			if (index != 0) {
				return ActivityHidden;
			}
			return m_active_task.activity[activity_id].activity_state;
		case TaskType::Shared:
			return ActivityHidden; // TODO: shared tasks
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return m_active_quests[index].activity[activity_id].activity_state;
			}
		default:
			return ActivityHidden;
	}
}

int ClientTaskState::GetTaskActivityDoneCount(TaskType task_type, int index, int activity_id)
{
	switch (task_type) {
		case TaskType::Task:
			if (index != 0) {
				return 0;
			}
			return m_active_task.activity[activity_id].done_count;
		case TaskType::Shared:
			return 0; // TODO: shared tasks
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
	if (m_active_task.task_id == task_id) {
		return m_active_task.activity[activity_id].done_count;
	}

	// TODO: shared tasks

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
		case TaskType::Shared: // TODO
		default:
			return -1;
	}
}

void ClientTaskState::CancelAllTasks(Client *client)
{

	// This method exists solely to be called during #task reloadall
	// It removes tasks from the in-game client state ready for them to be
	// resent to the client, in case an updated task fails to load

	CancelTask(client, 0, TaskType::Task, false);
	m_active_task.task_id = TASKSLOTEMPTY;

	for (int task_index = 0; task_index < MAXACTIVEQUESTS; task_index++)
		if (m_active_quests[task_index].task_id != TASKSLOTEMPTY) {
			CancelTask(client, task_index, TaskType::Quest, false);
			m_active_quests[task_index].task_id = TASKSLOTEMPTY;
		}

	// TODO: shared
}

void ClientTaskState::CancelTask(Client *client, int sequence_number, TaskType task_type, bool remove_from_db)
{
	auto outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));

	CancelTask_Struct *cts = (CancelTask_Struct *) outapp->pBuffer;
	cts->SequenceNumber = sequence_number;
	cts->type           = static_cast<uint32>(task_type);

	Log(Logs::General, Logs::Tasks, "[UPDATE] CancelTask");

	client->QueuePacket(outapp);
	safe_delete(outapp);

	if (remove_from_db) {
		RemoveTask(client, sequence_number, task_type);
	}
}

void ClientTaskState::RemoveTask(Client *client, int sequence_number, TaskType task_type)
{
	int character_id = client->CharacterID();
	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState Cancel Task %i ", sequence_number);

	int task_id = -1;
	switch (task_type) {
		case TaskType::Task:
			if (sequence_number == 0) {
				task_id = m_active_task.task_id;
			}
			break;
		case TaskType::Quest:
			if (sequence_number < MAXACTIVEQUESTS) {
				task_id = m_active_quests[sequence_number].task_id;
			}
			break;
		case TaskType::Shared: // TODO:
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
			break; // TODO: shared tasks
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
	auto task_type    = task_manager->GetTaskType(task_id);
	int  character_id = client->CharacterID();

	CharacterActivitiesRepository::DeleteWhere(
		database,
		fmt::format("charid = {} AND taskid = {}", character_id, task_id)
	);

	CharacterTasksRepository::DeleteWhere(
		database,
		fmt::format("charid = {} AND taskid = {} AND type = {}", character_id, task_id, (int) task_type)
	);

	switch (task_type) {
		case TaskType::Task: {
			if (m_active_task.task_id == task_id) {
				auto              outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));
				CancelTask_Struct *cts   = (CancelTask_Struct *) outapp->pBuffer;
				cts->SequenceNumber = 0;
				cts->type           = static_cast<uint32>(task_type);
				LogTasks("[UPDATE] RemoveTaskByTaskID found Task [{}]", task_id);
				client->QueuePacket(outapp);
				safe_delete(outapp);
				m_active_task.task_id = TASKSLOTEMPTY;
			}
			break;
		}
		case TaskType::Shared: {
			break; // TODO: shared tasks
		}
		case TaskType::Quest: {
			for (int active_quest = 0; active_quest < MAXACTIVEQUESTS; active_quest++) {
				if (m_active_quests[active_quest].task_id == task_id) {
					auto              outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));
					CancelTask_Struct *cts   = (CancelTask_Struct *) outapp->pBuffer;
					cts->SequenceNumber = active_quest;
					cts->type           = static_cast<uint32>(task_type);
					LogTasks("[UPDATE] RemoveTaskByTaskID found Quest [{}] at index [{}]", task_id, active_quest);
					m_active_quests[active_quest].task_id = TASKSLOTEMPTY;
					m_active_task_count--;
					client->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
		}
		default: {
			break;
		}
	}
}

void ClientTaskState::AcceptNewTask(Client *client, int task_id, int npc_type_id, bool enforce_level_requirement)
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

	bool max_tasks = false;

	switch (task->type) {
		case TaskType::Task:
			if (m_active_task.task_id != TASKSLOTEMPTY) {
				max_tasks = true;
			}
			break;
		case TaskType::Shared: // TODO: shared tasks
			// if (something)
			max_tasks = true;
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
		client->Message(
			Chat::Red,
			"You already have the maximum allowable number of active tasks (%i)",
			MAXACTIVEQUESTS
		);
		return;
	}

	// only Quests can have more than one, so don't need to check others
	if (task->type == TaskType::Quest) {
		for (auto &active_quest : m_active_quests) {
			if (active_quest.task_id == task_id) {
				client->Message(Chat::Red, "You have already been assigned this task.");
				return;
			}
		}
	}

	if (enforce_level_requirement && !task_manager->ValidateLevel(task_id, client->GetLevel())) {
		client->Message(Chat::Red, "You are outside the level range of this task.");
		return;
	}

	if (!task_manager->IsTaskRepeatable(task_id) && IsTaskCompleted(task_id)) {
		return;
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
		case TaskType::Shared: // TODO: shared
			active_slot         = nullptr;
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
		client->Message(
			Chat::Red,
			"You already have the maximum allowable number of active tasks (%i)",
			MAXACTIVEQUESTS
		);
		return;
	}

	active_slot->task_id       = task_id;
	active_slot->accepted_time = time(nullptr);
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

	task_manager->SendSingleActiveTaskToClient(client, *active_slot, false, true);
	client->Message(
		Chat::White,
		"You have been assigned the task '%s'.",
		task_manager->m_task_data[task_id]->title.c_str()
	);
	task_manager->SaveClientState(client, this);
	std::string buf = std::to_string(task_id);

	NPC *npc = entity_list.GetID(npc_type_id)->CastToNPC();
	if (npc) {
		parse->EventNPC(EVENT_TASK_ACCEPTED, npc, client, buf.c_str(), 0);
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
