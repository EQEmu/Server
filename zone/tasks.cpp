
/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/global_define.h"
#include "tasks.h"

#include <string.h>

#ifdef _WINDOWS
#define strcasecmp _stricmp
#endif

#include "../common/misc_functions.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/say_link.h"
#include "zonedb.h"
#include "zone_store.h"
#include "../common/repositories/goallists_repository.h"
#include "client.h"
#include "entity.h"
#include "mob.h"
#include "string_ids.h"

#include "queryserv.h"
#include "quest_parser_collection.h"
#include "../common/repositories/completed_tasks_repository.h"

extern QueryServ *QServ;

TaskManager::TaskManager()
{
	for (auto & Task : Tasks)
		Task = nullptr;
}

TaskManager::~TaskManager()
{
	for (auto & Task : Tasks) {
		if (Task != nullptr) {
			safe_delete(Task);
		}
	}
}

bool TaskManager::LoadTaskSets()
{

	// Clear all task sets in memory. Done so we can reload them on the fly if required by just calling
	// this method again.
	for (auto & TaskSet : TaskSets)
		TaskSet.clear();

	std::string query   = StringFormat(
		"SELECT `id`, `taskid` from `tasksets` "
		"WHERE `id` > 0 AND `id` < %i "
		"AND `taskid` >= 0 AND `taskid` < %i "
		"ORDER BY `id`, `taskid` ASC",
		MAXTASKSETS, MAXTASKS
	);
	auto        results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in TaskManager::LoadTaskSets: [{}]", results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int taskSet = atoi(row[0]);
		int taskID  = atoi(row[1]);

		TaskSets[taskSet].push_back(taskID);
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Adding task_id %4i to TaskSet %4i", taskID, taskSet);
	}

	return true;
}

bool TaskManager::LoadSingleTask(int TaskID)
{

	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] TaskManager::LoadSingleTask(%i)", TaskID);

	if ((TaskID <= 0) || (TaskID >= MAXTASKS)) { return false; }

	// If this task already exists in memory, free all the dynamically allocated strings.
	if (Tasks[TaskID]) {
		safe_delete(Tasks[TaskID]);
	}

	return LoadTasks(TaskID);
}

void TaskManager::ReloadGoalLists()
{

	if (!GoalListManager.LoadLists())
		Log(Logs::Detail, Logs::Tasks, "TaskManager::LoadTasks LoadLists failed");
}

bool TaskManager::LoadTasks(int singleTask)
{
	// If task_id !=0, then just load the task specified.
	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] TaskManager::LoadTasks Called");

	std::string query;
	if (singleTask == 0) {
		if (!GoalListManager.LoadLists())
			Log(Logs::Detail, Logs::Tasks, "TaskManager::LoadTasks LoadLists failed");

		if (!LoadTaskSets())
			Log(Logs::Detail, Logs::Tasks, "TaskManager::LoadTasks LoadTaskSets failed");

		query = StringFormat(
			"SELECT `id`, `type`, `duration`, `duration_code`, `title`, `description`, "
			"`reward`, `rewardid`, `cashreward`, `xpreward`, `rewardmethod`, `faction_reward`,"
			"`minlevel`, `maxlevel`, `repeatable`, `completion_emote` FROM `tasks` WHERE `id` < %i",
			MAXTASKS
		);
	}
	else {
		query = StringFormat(
			"SELECT `id`, `type`, `duration`, `duration_code`, `title`, `description`, "
			"`reward`, `rewardid`, `cashreward`, `xpreward`, `rewardmethod`, `faction_reward`,"
			"`minlevel`, `maxlevel`, `repeatable`, `completion_emote` FROM `tasks` WHERE `id` = %i",
			singleTask
		);
	}

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::LoadTasks: %s";

	auto results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int taskID = atoi(row[0]);

		if ((taskID <= 0) || (taskID >= MAXTASKS)) {
			// This shouldn't happen, as the SELECT is bounded by MAXTASKS
			LogError("[TASKS]Task ID [{}] out of range while loading tasks from database", taskID);
			continue;
		}

		Tasks[taskID] = new TaskInformation;
		Tasks[taskID]->type              = static_cast<TaskType>(atoi(row[1]));
		Tasks[taskID]->duration          = atoi(row[2]);
		Tasks[taskID]->duration_code     = static_cast<DurationCode>(atoi(row[3]));
		Tasks[taskID]->title             = row[4];
		Tasks[taskID]->description       = row[5];
		Tasks[taskID]->reward            = row[6];
		Tasks[taskID]->reward_id         = atoi(row[7]);
		Tasks[taskID]->cash_reward       = atoi(row[8]);
		Tasks[taskID]->experience_reward = atoi(row[9]);
		Tasks[taskID]->reward_method     = (TaskMethodType) atoi(row[10]);
		Tasks[taskID]->faction_reward    = atoi(row[11]);
		Tasks[taskID]->min_level         = atoi(row[12]);
		Tasks[taskID]->max_level         = atoi(row[13]);
		Tasks[taskID]->repeatable        = atoi(row[14]);
		Tasks[taskID]->completion_emote  = row[15];
		Tasks[taskID]->activity_count    = 0;
		Tasks[taskID]->sequence_mode     = ActivitiesSequential;
		Tasks[taskID]->last_step         = 0;

		Log(Logs::General, Logs::Tasks,
			"[GLOBALLOAD] task_id: %5i, duration: %8i, reward: %s min_level %i max_level %i "
			"repeatable: %s",
			taskID, Tasks[taskID]->duration, Tasks[taskID]->reward.c_str(),
			Tasks[taskID]->min_level, Tasks[taskID]->max_level, Tasks[taskID]->repeatable ? "Yes" : "No");
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] title: %s", Tasks[taskID]->title.c_str());
	}

	if (singleTask == 0) {
		query =
			StringFormat(
				"SELECT `taskid`, `step`, `activityid`, `activitytype`, `target_name`, `item_list`, "
				"`skill_list`, `spell_list`, `description_override`, `goalid`, `goalmethod`, "
				"`goalcount`, `delivertonpc`, `zones`, `optional` FROM `task_activities` WHERE `taskid` < "
				"%i AND `activityid` < %i ORDER BY taskid, activityid ASC",
				MAXTASKS, MAXACTIVITIESPERTASK
			);
	}
	else {
		query =
			StringFormat(
				"SELECT `taskid`, `step`, `activityid`, `activitytype`, `target_name`, `item_list`, "
				"`skill_list`, `spell_list`, `description_override`, `goalid`, `goalmethod`, "
				"`goalcount`, `delivertonpc`, `zones`, `optional` FROM `task_activities` WHERE `taskid` = "
				"%i AND `activityid` < %i ORDER BY taskid, activityid ASC",
				singleTask, MAXACTIVITIESPERTASK
			);
	}
	results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int taskID = atoi(row[0]);
		int step   = atoi(row[1]);

		int activityID = atoi(row[2]);

		if ((taskID <= 0) || (taskID >= MAXTASKS) || (activityID < 0) || (activityID >= MAXACTIVITIESPERTASK)) {
			// This shouldn't happen, as the SELECT is bounded by MAXTASKS
			LogError("[TASKS]Task or activity_information ID ([{}], [{}]) out of range while loading activities from database",
					 taskID,
					 activityID);
			continue;
		}

		if (Tasks[taskID] == nullptr) {
			LogError("[TASKS]activity_information for non-existent task ([{}], [{}]) while loading activities from database",
					 taskID, activityID);
			continue;
		}

		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].StepNumber = step;

		if (step != 0) {
			Tasks[taskID]->sequence_mode = ActivitiesStepped;
		}

		if (step > Tasks[taskID]->last_step) {
			Tasks[taskID]->last_step = step;
		}

		// Task Activities MUST be numbered sequentially from 0. If not, log an error
		// and set the task to nullptr. Subsequent activities for this task will raise
		// ERR_NOTASK errors.
		// Change to (activityID != (Tasks[taskID]->activity_count + 1)) to index from 1
		if (activityID != Tasks[taskID]->activity_count) {
			LogError("[TASKS]Activities for Task [{}] are not sequential starting at 0. Not loading task", taskID,
					 activityID);
			Tasks[taskID] = nullptr;
			continue;
		}

		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].Type = atoi(row[3]);

		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].target_name   = row[4];
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].item_list     = row[5];
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].skill_list    = row[6];
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].skill_id      = atoi(row[6]); // for older clients
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].spell_list    = row[7];
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].spell_id      = atoi(row[7]); // for older clients
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].desc_override = row[8];

		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].GoalID       = atoi(row[9]);
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].GoalMethod   = (TaskMethodType) atoi(row[10]);
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].GoalCount    = atoi(row[11]);
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].DeliverToNPC = atoi(row[12]);
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].zones        = row[13];
		auto      zones = SplitString(Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].zones, ';');
		for (auto &&e : zones)
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].ZoneIDs.push_back(std::stoi(e));
		Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].Optional = atoi(row[14]);

		Log(Logs::General, Logs::Tasks,
			"[GLOBALLOAD] activity_information Slot %2i: ID %i for Task %5i. Type: %3i, GoalID: %8i, "
			"GoalMethod: %i, GoalCount: %3i, Zones:%s",
			Tasks[taskID]->activity_count, activityID, taskID,
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].Type,
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].GoalID,
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].GoalMethod,
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].GoalCount,
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].zones.c_str());

		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] target_name: %s",
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].target_name.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] item_list: %s",
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].item_list.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] skill_list: %s",
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].skill_list.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] spell_list: %s",
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].spell_list.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] description_override: %s",
			Tasks[taskID]->activity_information[Tasks[taskID]->activity_count].desc_override.c_str());

		Tasks[taskID]->activity_count++;
	}

	return true;
}

bool TaskManager::SaveClientState(Client *c, ClientTaskState *state)
{
	// I am saving the slot in the ActiveTasks table, because unless a Task is cancelled/completed, the client
	// doesn't seem to like tasks moving slots between zoning and you can end up with 'bogus' activities if the task
	// previously in that slot had more activities than the one now occupying it. Hopefully retaining the slot
	// number for the duration of a session will overcome this.
	if (!c || !state) {
		return false;
	}

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::SaveClientState %s";

	int characterID = c->CharacterID();

	Log(Logs::Detail, Logs::Tasks, "TaskManager::SaveClientState for character ID %d", characterID);

	if (state->active_task_count > 0 || state->active_task.task_id != TASKSLOTEMPTY) { // TODO: tasks
		for (auto & ActiveTask : state->ActiveTasks) {
			int taskID = ActiveTask.task_id;
			if (taskID == TASKSLOTEMPTY) {
				continue;
			}

			int slot = ActiveTask.slot;

			if (ActiveTask.updated) {

				Log(Logs::General, Logs::Tasks,
					"[CLIENTSAVE] TaskManager::SaveClientState for character ID %d, Updating TaskIndex "
					"%i task_id %i",
					characterID, slot, taskID);

				std::string query   = StringFormat(
					"REPLACE INTO character_tasks (charid, taskid, slot, type, acceptedtime) "
					"VALUES (%i, %i, %i, %i, %i)",
					characterID, taskID, slot, static_cast<int>(Tasks[taskID]->type),
					ActiveTask.accepted_time
				);
				auto        results = database.QueryDatabase(query);
				if (!results.Success()) {
					LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
				}
				else {
					ActiveTask.updated = false;
				}
			}

			std::string query =
							"REPLACE INTO character_activities (charid, taskid, activityid, donecount, completed) "
							"VALUES ";

			int      updatedActivityCount = 0;
			for (int activityIndex        = 0; activityIndex < Tasks[taskID]->activity_count; ++activityIndex) {

				if (!ActiveTask.activity[activityIndex].updated) {
					continue;
				}

				Log(Logs::General, Logs::Tasks,
					"[CLIENTSAVE] TaskManager::SaveClientSate for character ID %d, Updating activity_information "
					"%i, %i",
					characterID, slot, activityIndex);

				if (updatedActivityCount == 0) {
					query +=
						StringFormat(
							"(%i, %i, %i, %i, %i)", characterID, taskID, activityIndex,
							ActiveTask.activity[activityIndex].done_count,
							ActiveTask.activity[activityIndex].activity_state ==
							ActivityCompleted
						);
				}
				else {
					query +=
						StringFormat(
							", (%i, %i, %i, %i, %i)", characterID, taskID, activityIndex,
							ActiveTask.activity[activityIndex].done_count,
							ActiveTask.activity[activityIndex].activity_state ==
							ActivityCompleted
						);
				}

				updatedActivityCount++;
			}

			if (updatedActivityCount == 0) {
				continue;
			}

			Log(Logs::General, Logs::Tasks, "[CLIENTSAVE] Executing query %s", query.c_str());
			auto results = database.QueryDatabase(query);

			if (!results.Success()) {
				LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
				continue;
			}

			ActiveTask.updated = false;
			for (int activityIndex                                       = 0;
				activityIndex < Tasks[taskID]->activity_count;
				++activityIndex)
				ActiveTask.activity[activityIndex].updated = false;
		}
	}

	if (!RuleB(TaskSystem, RecordCompletedTasks) ||
		(state->completed_tasks.size() <= (unsigned int) state->last_completed_task_loaded)) {
		state->last_completed_task_loaded = state->completed_tasks.size();
		return true;
	}

	const char *completedTaskQuery = "REPLACE INTO completed_tasks (charid, completedtime, taskid, activityid) "
									 "VALUES (%i, %i, %i, %i)";

	for (unsigned int i = state->last_completed_task_loaded; i < state->completed_tasks.size(); i++) {

		Log(Logs::General, Logs::Tasks,
			"[CLIENTSAVE] TaskManager::SaveClientState Saving Completed Task at slot %i", i);
		int taskID = state->completed_tasks[i].task_id;

		if ((taskID <= 0) || (taskID >= MAXTASKS) || (Tasks[taskID] == nullptr)) {
			continue;
		}

		// First we save a record with an activity_id of -1.
		// This indicates this task was completed at the given time. We infer that all
		// none optional activities were completed.
		//
		std::string query   =
						StringFormat(
							completedTaskQuery,
							characterID,
							state->completed_tasks[i].completed_time,
							taskID,
							-1
						);
		auto        results = database.QueryDatabase(query);
		if (!results.Success()) {
			LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
			continue;
		}

		// If the Rule to record non-optional task completion is not enabled, don't save it
		if (!RuleB(TaskSystem, RecordCompletedOptionalActivities)) {
			continue;
		}

		// Insert one record for each completed optional task.

		for (int j = 0; j < Tasks[taskID]->activity_count; j++) {
			if (!Tasks[taskID]->activity_information[j].Optional || !state->completed_tasks[i].activity_done[j]) {
				continue;
			}

			query   = StringFormat(
				completedTaskQuery, characterID, state->completed_tasks[i].completed_time,
				taskID, j
			);
			results = database.QueryDatabase(query);
			if (!results.Success())
				LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
		}
	}

	state->last_completed_task_loaded = state->completed_tasks.size();
	return true;
}

void Client::LoadClientTaskState()
{
	if (RuleB(TaskSystem, EnableTaskSystem) && p_task_manager) {
		if (task_state) {
			safe_delete(task_state);
		}

		task_state = new ClientTaskState;
		if (!p_task_manager->LoadClientState(this, task_state)) {
			safe_delete(task_state);
		}
		else {
			p_task_manager->SendActiveTasksToClient(this);
			p_task_manager->SendCompletedTasksToClient(this, task_state);
		}
	}
}

void Client::RemoveClientTaskState()
{
	if (task_state) {
		task_state->CancelAllTasks(this);
		safe_delete(task_state);
	}
}

bool TaskManager::LoadClientState(Client *c, ClientTaskState *state)
{
	if (!c || !state) {
		return false;
	}

	int character_id = c->CharacterID();

	state->active_task_count = 0;

	LogTasks("[LoadClientState] for character_id [{}]", character_id);

	std::string query = StringFormat(
		"SELECT `taskid`, `slot`,`type`, `acceptedtime` "
		"FROM `character_tasks` "
		"WHERE `charid` = %i ORDER BY acceptedtime",
		character_id
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int  task_id = atoi(row[0]);
		int  slot    = atoi(row[1]);
		auto type    = static_cast<TaskType>(atoi(row[2]));

		if ((task_id < 0) || (task_id >= MAXTASKS)) {
			LogError("[TASKS]Task ID [{}] out of range while loading character tasks from database", task_id);
			continue;
		}

		auto task_info = state->GetClientTaskInfo(type, slot);

		if (task_info == nullptr) {
			LogError("[TASKS] Slot [{}] out of range while loading character tasks from database", slot);
			continue;
		}

		if (task_info->task_id != TASKSLOTEMPTY) {
			LogError("[TASKS] Slot [{}] for Task [{}]s is already occupied", slot, task_id);
			continue;
		}

		int accepted_time = atoi(row[3]);

		task_info->task_id       = task_id;
		task_info->current_step  = -1;
		task_info->accepted_time = accepted_time;
		task_info->updated       = false;

		for (auto & i : task_info->activity) {
			i.activity_id = -1;
		}

		if (type == TaskType::Quest) {
			++state->active_task_count;
		}

		LogTasks("[LoadClientState] character_id [{}] task_id [{}] accepted_time [{}]", character_id, task_id, accepted_time);
	}

	// Load Activities
	LogTasks("[LoadClientState] Loading activities for character_id [{}]", character_id);

	query   = StringFormat(
		"SELECT `taskid`, `activityid`, `donecount`, `completed` "
		"FROM `character_activities` "
		"WHERE `charid` = %i "
		"ORDER BY `taskid` ASC, `activityid` ASC",
		character_id
	);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int task_id = atoi(row[0]);
		if ((task_id < 0) || (task_id >= MAXTASKS)) {
			LogTasks(
				"[LoadClientState] Error: task_id [{}] out of range while loading character activities from database character_id [{}]",
				task_id,
				character_id
			);
			continue;
		}

		int activity_id = atoi(row[1]);
		if ((activity_id < 0) || (activity_id >= MAXACTIVITIESPERTASK)) {
			LogTasks(
				"[LoadClientState] Error: activity_id [{}] out of range while loading character activities from database character_id [{}]",
				activity_id,
				character_id
			);

			continue;
		}

		ClientTaskInformation *task_info = nullptr;
		if (state->active_task.task_id == task_id) {
			task_info = &state->active_task;
		}

		// wasn't task
		if (task_info == nullptr) {
			for (auto & active_quest : state->active_quests) {
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

		int  done_count = atoi(row[2]);
		bool completed  = atoi(row[3]);
		task_info->activity[activity_id].activity_id = activity_id;
		task_info->activity[activity_id].done_count  = done_count;
		if (completed) {
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
			done_count,
			completed
		);
	}

	if (RuleB(TaskSystem, RecordCompletedTasks)) {
		query   = StringFormat(
			"SELECT `taskid`, `activityid`, `completedtime` "
			"FROM `completed_tasks` "
			"WHERE `charid` = %i ORDER BY completedtime, taskid, activityid",
			character_id
		);
		results = database.QueryDatabase(query);
		if (!results.Success()) {
			return false;
		}

		CompletedTaskInformation completed_task_information{};
		for (bool & i : completed_task_information.activity_done)
			i = false;

		int previous_task_id        = -1;
		int previous_completed_time = -1;

		for (auto row = results.begin(); row != results.end(); ++row) {

			int task_id = atoi(row[0]);
			if ((task_id <= 0) || (task_id >= MAXTASKS)) {
				LogError("[TASKS]Task ID [{}] out of range while loading completed tasks from database", task_id);
				continue;
			}

			// An activity_id of -1 means mark all the none optional activities in the
			// task as complete. If the Rule to record optional activities is enabled,
			// subsequent records for this task will flag any optional tasks that were
			// completed.
			int activity_id = atoi(row[1]);
			if ((activity_id < -1) || (activity_id >= MAXACTIVITIESPERTASK)) {
				LogError("[TASKS]activity_information ID [{}] out of range while loading completed tasks from database", activity_id);
				continue;
			}

			int completed_time = atoi(row[2]);
			if ((previous_task_id != -1) &&
				((task_id != previous_task_id) || (completed_time != previous_completed_time))) {
				state->completed_tasks.push_back(completed_task_information);
				for (bool & activity_done : completed_task_information.activity_done) {
					activity_done = false;
				}
			}

			completed_task_information.task_id        = previous_task_id        = task_id;
			completed_task_information.completed_time = previous_completed_time = completed_time;

			// If activity_id is -1, Mark all the non-optional tasks as completed.
			if (activity_id < 0) {
				TaskInformation *task = Tasks[task_id];
				if (task == nullptr) {
					continue;
				}

				for (int i = 0; i < task->activity_count; i++) {
					if (!task->activity_information[i].Optional) {
						completed_task_information.activity_done[i] = true;
					}
				}
			}
			else {
				completed_task_information.activity_done[activity_id] = true;
			}
		}

		if (previous_task_id != -1) {
			state->completed_tasks.push_back(completed_task_information);
		}

		state->last_completed_task_loaded = state->completed_tasks.size();
	}

	query   = StringFormat(
		"SELECT `taskid` FROM character_enabledtasks "
		"WHERE `charid` = %i AND `taskid` >0 AND `taskid` < %i "
		"ORDER BY `taskid` ASC",
		character_id, MAXTASKS
	);
	results = database.QueryDatabase(query);
	if (results.Success()) {
		for (auto row = results.begin(); row != results.end(); ++row) {
			int task_id = atoi(row[0]);
			state->enabled_tasks.push_back(task_id);
			LogTasksDetail("[LoadClientState] Adding task_id [{}] to enabled tasks", task_id);
		}
	}

	// Check that there is an entry in the client task state for every activity_information in each task
	// This should only break if a ServerOP adds or deletes activites for a task that players already
	// have active, or due to a bug.
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		int task_id = state->ActiveTasks[i].task_id;
		if (task_id == TASKSLOTEMPTY) {
			continue;
		}
		if (!Tasks[task_id]) {
			c->Message(
				Chat::Red,
				"Active Task Slot %i, references a task (%i), that does not exist. "
				"Removing from memory. Contact a GM to resolve this.",
				i, task_id
			);

			LogError("[LoadClientState] Character [{}] has task [{}] which does not exist", character_id, task_id);
			state->ActiveTasks[i].task_id = TASKSLOTEMPTY;
			continue;
		}
		for (int activity_index = 0; activity_index < Tasks[task_id]->activity_count; activity_index++) {
			if (state->ActiveTasks[i].activity[activity_index].activity_id != activity_index) {
				c->Message(
					Chat::Red,
					"Active Task %i, %s. activity_information count does not match expected value."
					"Removing from memory. Contact a GM to resolve this.",
					task_id, Tasks[task_id]->title.c_str()
				);

				LogTasks(
					"[LoadClientState] Fatal error in character [{}] task state. activity_information [{}] for Task [{}] either missing from client state or from task",
					character_id,
					activity_index,
					task_id
				);
				state->ActiveTasks[i].task_id = TASKSLOTEMPTY;
				break;
			}
		}
	}

	if (state->active_task.task_id != TASKSLOTEMPTY) {
		state->UnlockActivities(character_id, state->active_task);
	}

	// TODO: shared
	for (auto & active_quest : state->active_quests) {
		if (active_quest.task_id != TASKSLOTEMPTY) {
			state->UnlockActivities(character_id, active_quest);
		}
	}

	LogTasks( "[LoadClientState] for Character ID [{}}] DONE!", character_id);

	return true;
}

void ClientTaskState::EnableTask(int character_id, int task_count, int *tasks)
{
	// Check if the Task is already enabled for this client
	std::vector<int> tasks_enabled;
	for (int         i = 0; i < task_count; i++) {

		auto iterator = enabled_tasks.begin();
		bool addTask  = true;

		while (iterator != enabled_tasks.end()) {
			// If this task is already enabled, stop looking
			if ((*iterator) == tasks[i]) {
				addTask = false;
				break;
			}
			// Our list of enabled tasks is sorted, so we can quit if we find a taskid higher than
			// the one we are looking for.
			if ((*iterator) > tasks[i]) {
				break;
			}
			++iterator;
		}

		if (addTask) {
			enabled_tasks.insert(iterator, tasks[i]);
			// Make a note of the task we enabled, for later SQL generation
			tasks_enabled.push_back(tasks[i]);
		}
	}

	LogTasksDetail("[EnableTask] New enabled task list");
	for (int enabled_task : enabled_tasks) {
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
	std::vector<int> tasksDisabled;

	for (int i = 0; i < task_count; i++) {
		auto iterator   = enabled_tasks.begin();
		bool removeTask = false;

		while (iterator != enabled_tasks.end()) {
			if ((*iterator) == task_list[i]) {
				removeTask = true;
				break;
			}

			if ((*iterator) > task_list[i]) {
				break;
			}

			++iterator;
		}

		if (removeTask) {
			enabled_tasks.erase(iterator);
			tasksDisabled.push_back(task_list[i]);
		}
	}

	LogTasks("[DisableTask] New enabled task list ");
	for (int enabled_task : enabled_tasks) {
		LogTasks("[DisableTask] enabled_tasks [{}]", enabled_task);
	}

	if (tasksDisabled.empty()) {
		return;
	}

	std::stringstream queryStream;
	queryStream << StringFormat("DELETE FROM character_enabledtasks WHERE charid = %i AND (", character_id);

	for (unsigned int i = 0; i < tasksDisabled.size(); i++)
		queryStream
			<< (i ? StringFormat("taskid = %i ", tasksDisabled[i]) : StringFormat("OR taskid = %i ", tasksDisabled[i]));

	queryStream << ")";

	std::string query = queryStream.str();

	if (tasksDisabled.size()) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] Executing query %s", query.c_str());
		database.QueryDatabase(query);
	}
	else {
		Log(Logs::General,
			Logs::Tasks,
			"[UPDATE] DisableTask called for characterID: %u .. but, no tasks exist",
			character_id);
	}
}

bool ClientTaskState::IsTaskEnabled(int TaskID)
{

	std::vector<int>::iterator Iterator;

	Iterator = enabled_tasks.begin();

	while (Iterator != enabled_tasks.end()) {
		if ((*Iterator) == TaskID) { return true; }
		if ((*Iterator) > TaskID) { break; }
		++Iterator;
	}

	return false;
}

int ClientTaskState::EnabledTaskCount(int TaskSetID)
{

	// Return the number of tasks in TaskSet that this character is enabled for.

	unsigned int EnabledTaskIndex = 0;
	unsigned int TaskSetIndex     = 0;
	int          EnabledTaskCount = 0;

	if ((TaskSetID <= 0) || (TaskSetID >= MAXTASKSETS)) { return -1; }

	while ((EnabledTaskIndex < enabled_tasks.size()) && (TaskSetIndex < p_task_manager->TaskSets[TaskSetID].size())) {

		if (enabled_tasks[EnabledTaskIndex] == p_task_manager->TaskSets[TaskSetID][TaskSetIndex]) {

			EnabledTaskCount++;
			EnabledTaskIndex++;
			TaskSetIndex++;
			continue;
		}

		if (enabled_tasks[EnabledTaskIndex] < p_task_manager->TaskSets[TaskSetID][TaskSetIndex]) {
			EnabledTaskIndex++;
		}
		else {
			TaskSetIndex++;
		}

	}

	return EnabledTaskCount;
}
int ClientTaskState::ActiveTasksInSet(int TaskSetID)
{


	if ((TaskSetID <= 0) || (TaskSetID >= MAXTASKSETS)) { return -1; }

	int Count = 0;

	for (unsigned int i = 0; i < p_task_manager->TaskSets[TaskSetID].size(); i++)
		if (IsTaskActive(p_task_manager->TaskSets[TaskSetID][i])) {
			Count++;
		}

	return Count;
}

int ClientTaskState::CompletedTasksInSet(int TaskSetID)
{


	if ((TaskSetID <= 0) || (TaskSetID >= MAXTASKSETS)) { return -1; }

	int Count = 0;

	for (unsigned int i = 0; i < p_task_manager->TaskSets[TaskSetID].size(); i++)
		if (IsTaskCompleted(p_task_manager->TaskSets[TaskSetID][i])) {
			Count++;
		}

	return Count;
}

bool ClientTaskState::HasSlotForTask(TaskInformation *task)
{
	if (task == nullptr) {
		return false;
	}

	switch (task->type) {
		case TaskType::Task:
			return active_task.task_id == TASKSLOTEMPTY;
		case TaskType::Shared:
			return false; // todo
		case TaskType::Quest:
			for (int i = 0; i < MAXACTIVEQUESTS; ++i)
				if (active_quests[i].task_id == TASKSLOTEMPTY) {
					return true;
				}
		case TaskType::E:
			return false; // removed on live
	}

	return false;
}

int TaskManager::FirstTaskInSet(int TaskSetID)
{

	if ((TaskSetID <= 0) || (TaskSetID >= MAXTASKSETS)) { return 0; }

	if (TaskSets[TaskSetID].empty()) { return 0; }

	auto Iterator = TaskSets[TaskSetID].begin();

	while (Iterator != TaskSets[TaskSetID].end()) {
		if ((*Iterator) > 0) {
			return (*Iterator);
		}
		++Iterator;
	}

	return 0;
}

int TaskManager::LastTaskInSet(int TaskSetID)
{
	if ((TaskSetID <= 0) || (TaskSetID >= MAXTASKSETS)) {
		return 0;
	}

	if (TaskSets[TaskSetID].empty()) {
		return 0;
	}

	return TaskSets[TaskSetID][TaskSets[TaskSetID].size() - 1];
}

int TaskManager::NextTaskInSet(int TaskSetID, int TaskID)
{
	if ((TaskSetID <= 0) || (TaskSetID >= MAXTASKSETS)) {
		return 0;
	}

	if (TaskSets[TaskSetID].empty()) {
		return 0;
	}

	for (int i : TaskSets[TaskSetID]) {
		if (i > TaskID) {
			return i;
		}
	}

	return 0;
}

bool TaskManager::ValidateLevel(int TaskID, int PlayerLevel)
{
	if (Tasks[TaskID] == nullptr) {
		return false;
	}

	if (Tasks[TaskID]->min_level && (PlayerLevel < Tasks[TaskID]->min_level)) {
		return false;
	}

	if (Tasks[TaskID]->max_level && (PlayerLevel > Tasks[TaskID]->max_level)) {
		return false;
	}

	return true;
}

std::string TaskManager::GetTaskName(uint32 task_id)
{
	if (task_id > 0 && task_id < MAXTASKS) {
		if (Tasks[task_id] != nullptr) {
			return Tasks[task_id]->title;
		}
	}

	return std::string();
}

TaskType TaskManager::GetTaskType(uint32 task_id)
{
	if (task_id > 0 && task_id < MAXTASKS) {
		if (Tasks[task_id] != nullptr) {
			return Tasks[task_id]->type;
		}
	}
	return TaskType::Task;
}

int TaskManager::GetTaskMinLevel(int TaskID)
{
	if (Tasks[TaskID]->min_level) {
		return Tasks[TaskID]->min_level;
	}

	return -1;
}

int TaskManager::GetTaskMaxLevel(int TaskID)
{
	if (Tasks[TaskID]->max_level) {
		return Tasks[TaskID]->max_level;
	}

	return -1;
}

void TaskManager::TaskSetSelector(Client *c, ClientTaskState *state, Mob *mob, int TaskSetID)
{
	int TaskList[MAXCHOOSERENTRIES];
	int TaskListIndex = 0;
	int PlayerLevel   = c->GetLevel();

	LogTasks("TaskSetSelector called for taskset [{}]. EnableTaskSize is [{}]", TaskSetID, state->enabled_tasks.size());

	if (TaskSetID <= 0 || TaskSetID >= MAXTASKSETS) {
		return;
	}

	if (TaskSets[TaskSetID].empty()) {
		mob->SayString(c, Chat::Yellow, MAX_ACTIVE_TASKS, c->GetName()); // I think this is suppose to be yellow
		return;
	}

	bool all_enabled = false;

	// A task_id of 0 in a TaskSet indicates that all Tasks in the set are enabled for all players.
	if (TaskSets[TaskSetID][0] == 0) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSets[%i][0] == 0. All Tasks in Set enabled.", TaskSetID);
		all_enabled = true;
	}

	auto Iterator = TaskSets[TaskSetID].begin();

	if (all_enabled) {
		++Iterator;
	} // skip first when all enabled since it's useless data

	while (Iterator != TaskSets[TaskSetID].end() && TaskListIndex < MAXCHOOSERENTRIES) {
		auto task = *Iterator;
		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if ((all_enabled || state->IsTaskEnabled(task)) && ValidateLevel(task, PlayerLevel) &&
			!state->IsTaskActive(task) && state->HasSlotForTask(Tasks[task]) &&
			// this slot checking is a bit silly, but we allow mixing of task types ...
			(IsTaskRepeatable(task) || !state->IsTaskCompleted(task))) {
			TaskList[TaskListIndex++] = task;
		}

		++Iterator;
	}

	if (TaskListIndex > 0) {
		SendTaskSelector(c, mob, TaskListIndex, TaskList);
	}
	else {
		// TODO: check color, I think this might be only for (Shared) Tasks, w/e -- think should be yellow
		mob->SayString(
			c,
			Chat::Yellow,
			MAX_ACTIVE_TASKS,
			c->GetName()
		);
	}
}

// unlike the non-Quest version of this function, it does not check enabled, that is assumed the responsibility of the quest to handle
// we do however still want it to check the other stuff like level, active, room, etc
void TaskManager::TaskQuestSetSelector(Client *c, ClientTaskState *state, Mob *mob, int count, int *tasks)
{
	int task_list[MAXCHOOSERENTRIES];
	int task_list_index = 0;
	int player_level    = c->GetLevel();

	LogTasks("[UPDATE] TaskQuestSetSelector called for array size [{}]", count);

	if (count <= 0) {
		return;
	}

	for (int i = 0; i < count; ++i) {
		auto task = tasks[i];
		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if (ValidateLevel(task, player_level) && !state->IsTaskActive(task) && state->HasSlotForTask(Tasks[task]) &&
			// this slot checking is a bit silly, but we allow mixing of task types ...
			(IsTaskRepeatable(task) || !state->IsTaskCompleted(task))) {
			task_list[task_list_index++] = task;
		}
	}

	if (task_list_index > 0) {
		SendTaskSelector(c, mob, task_list_index, task_list);
	}
	else {
		// TODO: check color, I think this might be only for (Shared) Tasks, w/e -- think should be yellow
		mob->SayString(
			c,
			Chat::Yellow,
			MAX_ACTIVE_TASKS,
			c->GetName()
		);
	}
}

// sends task selector to client
void TaskManager::SendTaskSelector(Client *c, Mob *mob, int TaskCount, int *TaskList)
{
	if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		SendTaskSelectorNew(c, mob, TaskCount, TaskList);
		return;
	}
	// Titanium OpCode: 0x5e7c
	LogTasks("TaskSelector for [{}] Tasks", TaskCount);
	int PlayerLevel = c->GetLevel();

	// Check if any of the tasks exist
	for (int i = 0; i < TaskCount; i++) {
		if (Tasks[TaskList[i]] != nullptr) {
			break;
		}
	}

	int ValidTasks = 0;

	for (int i = 0; i < TaskCount; i++) {
		if (!ValidateLevel(TaskList[i], PlayerLevel)) {
			continue;
		}
		if (c->IsTaskActive(TaskList[i])) {
			continue;
		}
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) {
			continue;
		}

		ValidTasks++;
	}

	if (ValidTasks == 0) {
		return;
	}

	SerializeBuffer buf(50 * ValidTasks);


	buf.WriteUInt32(ValidTasks);
	buf.WriteUInt32(2); // task type, live doesn't let you send more than one type, but we do?
	buf.WriteUInt32(mob->GetID());

	for (int i = 0; i < TaskCount; i++) {
		if (!ValidateLevel(TaskList[i], PlayerLevel)) {
			continue;
		}
		if (c->IsTaskActive(TaskList[i])) {
			continue;
		}
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) {
			continue;
		}

		buf.WriteUInt32(TaskList[i]);    // task_id

		// affects color, difficulty?
		if (c->ClientVersion() != EQ::versions::ClientVersion::Titanium) {
			buf.WriteFloat(1.0f);
		}
		buf.WriteUInt32(Tasks[TaskList[i]]->duration);
		buf.WriteUInt32(static_cast<int>(Tasks[TaskList[i]]->duration_code));

		buf.WriteString(Tasks[TaskList[i]]->title); // max 64 with null
		buf.WriteString(Tasks[TaskList[i]]->description); // max 4000 with null

		// Has reward set flag
		if (c->ClientVersion() != EQ::versions::ClientVersion::Titanium) {
			buf.WriteUInt8(0);
		}

		buf.WriteUInt32(Tasks[TaskList[i]]->activity_count);

		for (int j = 0; j < Tasks[TaskList[i]]->activity_count; ++j) {
			buf.WriteUInt32(j); // ActivityNumber
			auto &activity = Tasks[TaskList[i]]->activity_information[j];
			buf.WriteUInt32(activity.Type);
			buf.WriteUInt32(0); // solo, group, raid?
			buf.WriteString(activity.target_name); // max length 64, "target name" so like loot x foo from bar (this is bar)
			buf.WriteString(activity.item_list); // max length 64 in these clients
			buf.WriteUInt32(activity.GoalCount);
			buf.WriteInt32(activity.skill_id);
			buf.WriteInt32(activity.spell_id);
			buf.WriteInt32(activity.ZoneIDs.empty() ? 0 : activity.ZoneIDs.front());
			buf.WriteString(activity.desc_override);
		}
	}

	auto outapp = new EQApplicationPacket(OP_OpenNewTasksWindow, buf);

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void TaskManager::SendTaskSelectorNew(Client *c, Mob *mob, int TaskCount, int *TaskList)
{
	LogTasks("SendTaskSelectorNew for [{}] Tasks", TaskCount);

	int PlayerLevel = c->GetLevel();

	// Check if any of the tasks exist
	for (int i = 0; i < TaskCount; i++) {
		if (Tasks[TaskList[i]] != nullptr) {
			break;
		}
	}

	int ValidTasks = 0;

	for (int i = 0; i < TaskCount; i++) {
		if (!ValidateLevel(TaskList[i], PlayerLevel)) {
			continue;
		}
		if (c->IsTaskActive(TaskList[i])) {
			continue;
		}
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) {
			continue;
		}

		ValidTasks++;
	}

	if (ValidTasks == 0) {
		return;
	}

	SerializeBuffer buf(50 * ValidTasks);

	buf.WriteUInt32(ValidTasks);    // TaskCount
	buf.WriteUInt32(2);            // Type, valid values: 0-3. 0 = Task, 1 = Shared Task, 2 = Quest, 3 = ??? -- should fix maybe some day, but we let more than 1 type through :P
	// so I guess an NPC can only offer one type of quests or we can only open a selection with one type :P (so quest call can tell us I guess)
	// this is also sent in OP_TaskDescription
	buf.WriteUInt32(mob->GetID());    // TaskGiver

	for (int i = 0; i < TaskCount; i++) { // max 40
		if (!ValidateLevel(TaskList[i], PlayerLevel)) {
			continue;
		}
		if (c->IsTaskActive(TaskList[i])) {
			continue;
		}
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) {
			continue;
		}

		buf.WriteUInt32(TaskList[i]);    // task_id
		buf.WriteFloat(1.0f); // affects color, difficulty?
		buf.WriteUInt32(Tasks[TaskList[i]]->duration);
		buf.WriteUInt32(static_cast<int>(Tasks[TaskList[i]]->duration_code));    // 1 = Short, 2 = Medium, 3 = Long, anything else Unlimited

		buf.WriteString(Tasks[TaskList[i]]->title); // max 64 with null
		buf.WriteString(Tasks[TaskList[i]]->description); // max 4000 with null

		buf.WriteUInt8(0);                // Has reward set flag
		buf.WriteUInt32(Tasks[TaskList[i]]->activity_count);    // activity_count

		for (int j = 0; j < Tasks[TaskList[i]]->activity_count; ++j) {
			buf.WriteUInt32(j);                // ActivityNumber
			auto &activity = Tasks[TaskList[i]]->activity_information[j];
			buf.WriteUInt32(activity.Type);                // ActivityType
			buf.WriteUInt32(0);                // solo, group, raid?
			buf.WriteString(activity.target_name);    // max length 64, "target name" so like loot x foo from bar (this is bar)

			// this string is item names
			buf.WriteLengthString(activity.item_list);

			buf.WriteUInt32(activity.GoalCount);                // GoalCount

			// this string is skill IDs? probably one of the "use on" tasks
			buf.WriteLengthString(activity.skill_list);

			// this string is spell IDs? probably one of the "use on" tasks
			buf.WriteLengthString(activity.spell_list);

			//buf.WriteString(itoa(Tasks[TaskList[i]]->activity_information[activity_id].ZoneID));
			buf.WriteString(activity.zones);        // Zone number in ascii max length 64, can be multiple with separated by ;
			buf.WriteString(activity.desc_override);    // max length 128 -- overrides the automatic descriptions
			// this doesn't appear to be shown to the client at all and isn't the same as zones ... defaults to '0' though
			buf.WriteString(activity.zones);        // Zone number in ascii max length 64, probably can be separated by ; too, haven't found it used
		}
	}

	auto outapp = new EQApplicationPacket(OP_OpenNewTasksWindow, buf);

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

int TaskManager::GetActivityCount(int TaskID)
{

	// Return the total number of activities in a particular task.

	if ((TaskID > 0) && (TaskID < MAXTASKS)) {
		if (Tasks[TaskID]) { return Tasks[TaskID]->activity_count; }
	}

	return 0;

}

void TaskManager::ExplainTask(Client *c, int TaskID)
{

	// TODO: This method is not finished (hardly started). It was intended to
	// explain in English, what each activity_information did, conditions for step unlocking, etc.
	//
	return;

	if (!c) { return; }

	if ((TaskID <= 0) || (TaskID >= MAXTASKS)) {
		c->Message(Chat::White, "task_id out-of-range.");
		return;
	}

	if (Tasks[TaskID] == nullptr) {
		c->Message(Chat::White, "Task does not exist.");
		return;
	}

	char explanation[1000], *ptr;
	c->Message(Chat::White, "Task %4i: title: %s", TaskID, Tasks[TaskID]->description.c_str());
	c->Message(Chat::White, "%3i Activities", Tasks[TaskID]->activity_count);
	ptr = explanation;
	for (int i = 0; i < Tasks[TaskID]->activity_count; i++) {

		sprintf(ptr, "Act: %3i: ", i);
		ptr = ptr + strlen(ptr);
		switch (Tasks[TaskID]->activity_information[i].Type) {
			case ActivityDeliver:
				sprintf(ptr, "Deliver");
				break;
		}

	}
}

ClientTaskState::ClientTaskState()
{
	active_task_count          = 0;
	last_completed_task_loaded = 0;
	checked_touch_activities   = false;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		active_quests[i].slot   = i;
		active_quests[i].task_id = TASKSLOTEMPTY;
	}

	active_task.slot   = 0;
	active_task.task_id = TASKSLOTEMPTY;
	// TODO: shared task
}

ClientTaskState::~ClientTaskState()
{
}


int ClientTaskState::GetActiveTaskID(int index)
{

	// Return the task_id from the client's specified Active Task slot.
	if ((index < 0) || (index >= MAXACTIVEQUESTS)) {
		return 0;
	}

	return active_quests[index].task_id;
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

	TaskInformation *p_task_information = p_task_manager->Tasks[task_info.task_id];
	if (p_task_information == nullptr) {
		return true;
	}

	// On loading the client state, all activities that are not completed, are
	// marked as hidden. For Sequential (non-stepped) mode, we mark the first
	// activity_information as active if not complete.
	LogTasks(
		"character_id [{}] task_id [{}] sequence_mode [{}]",
		character_id,
		task_info.task_id,
		p_task_information->sequence_mode
	);

	if (p_task_information->sequence_mode == ActivitiesSequential) {
		if (task_info.activity[0].activity_state != ActivityCompleted) {
			task_info.activity[0].activity_state = ActivityActive;
		}

		// Enable the next Hidden task.
		for (int i = 0; i < p_task_information->activity_count; i++) {
			if ((task_info.activity[i].activity_state == ActivityActive) &&
				(!p_task_information->activity_information[i].Optional)) {
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
				auto iterator        = completed_tasks.begin();
				int  erased_elements = 0;
				while (iterator != completed_tasks.end()) {
					int task_id = (*iterator).task_id;
					if (task_id == task_info.task_id) {
						iterator = completed_tasks.erase(iterator);
						erased_elements++;
					}
					else {
						++iterator;
					}
				}

				LogTasks("Erased Element count is [{}]", erased_elements);

				if (erased_elements) {
					last_completed_task_loaded -= erased_elements;
					DeleteCompletedTaskFromDatabase(character_id, task_info.task_id);
				}
			}

			CompletedTaskInformation completed_task_information{};
			completed_task_information.task_id        = task_info.task_id;
			completed_task_information.completed_time = time(nullptr);

			for (int i = 0; i < p_task_information->activity_count; i++) {
				completed_task_information.activity_done[i] = (task_info.activity[i].activity_state == ActivityCompleted);
			}

			completed_tasks.push_back(completed_task_information);
		}

		LogTasks("Returning sequential task, AllActivitiesComplete is [{}]", all_activities_complete);

		return all_activities_complete;
	}

	// Stepped Mode
	// TODO: This code is probably more complex than it needs to be

	bool CurrentStepComplete = true;

	Log(Logs::General, Logs::Tasks, "[UPDATE] Current Step is %i, Last Step is %i", task_info.current_step,
		p_task_information->last_step);
	// If current_step is -1, this is the first call to this method since loading the
	// client state. Unlock all activities with a step number of 0

	if (task_info.current_step == -1) {
		for (int i             = 0; i < p_task_information->activity_count; i++) {
			if (p_task_information->activity_information[i].StepNumber == 0 && task_info.activity[i].activity_state == ActivityHidden) {
				task_info.activity[i].activity_state = ActivityActive;
				// task_info.activity_information[i].updated=true;
			}
		}
		task_info.current_step = 0;
	}

	for (int Step = task_info.current_step; Step <= p_task_information->last_step; Step++) {
		for (int Activity = 0; Activity < p_task_information->activity_count; Activity++) {
			if (p_task_information->activity_information[Activity].StepNumber == (int) task_info.current_step) {
				if ((task_info.activity[Activity].activity_state != ActivityCompleted) &&
					(!p_task_information->activity_information[Activity].Optional)) {
					CurrentStepComplete     = false;
					all_activities_complete = false;
					break;
				}
			}
		}
		if (!CurrentStepComplete) {
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
				auto iterator        = completed_tasks.begin();
				int  erased_elements = 0;

				while (iterator != completed_tasks.end()) {
					int task_id = (*iterator).task_id;
					if (task_id == task_info.task_id) {
						iterator = completed_tasks.erase(iterator);
						erased_elements++;
					}
					else {
						++iterator;
					}
				}

				LogTasksDetail("[UnlockActivities] Erased Element count is [{}]", erased_elements);

				if (erased_elements) {
					last_completed_task_loaded -= erased_elements;
					DeleteCompletedTaskFromDatabase(character_id, task_info.task_id);
				}
			}

			CompletedTaskInformation completed_task_information{};
			completed_task_information.task_id        = task_info.task_id;
			completed_task_information.completed_time = time(nullptr);

			for (int i = 0; i < p_task_information->activity_count; i++) {
				completed_task_information.activity_done[i] = (task_info.activity[i].activity_state == ActivityCompleted);
			}
			
			completed_tasks.push_back(completed_task_information);
		}
		return true;
	}

	// Mark all non-completed tasks in the current step as active
	for (int activity = 0; activity < p_task_information->activity_count; activity++) {
		if ((p_task_information->activity_information[activity].StepNumber == (int) task_info.current_step) &&
			(task_info.activity[activity].activity_state == ActivityHidden)) {
			task_info.activity[activity].activity_state = ActivityActive;
			task_info.activity[activity].updated        = true;
		}
	}

	return false;
}

void ClientTaskState::UpdateTasksOnKill(Client *c, int NPCTypeID)
{
	UpdateTasksByNPC(c, ActivityKill, NPCTypeID);
}

bool ClientTaskState::UpdateTasksOnSpeakWith(Client *c, int NPCTypeID)
{
	return UpdateTasksByNPC(c, ActivitySpeakWith, NPCTypeID);
}

bool ClientTaskState::UpdateTasksByNPC(Client *c, int activity_type, int npc_type_id)
{

	int is_updating = false;

	// If the client has no tasks, there is nothing further to check.
	if (!p_task_manager || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
		return false;
	}

	// loop over the union of tasks and quests
	for (auto & ActiveTask : ActiveTasks) {
		auto current_task = &ActiveTask;
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active kill activities for this p_task_information
		auto p_task_information = p_task_manager->Tasks[current_task->task_id];
		if (p_task_information == nullptr) {
			return false;
		}

		for (int activity_id = 0; activity_id < p_task_information->activity_count; activity_id++) {
			// We are not interested in completed or hidden activities
			if (current_task->activity[activity_id].activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in Kill activities
			if (p_task_information->activity_information[activity_id].Type != activity_type) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!p_task_information->activity_information[activity_id].CheckZone(zone->GetZoneID())) {
				LogTasks(
					"[UPDATE] character [{}] task_id [{}] activity_id [{}] activity_type [{}] for NPC [{}] failed zone check",
					c->GetName(),
					current_task->task_id,
					activity_id,
					activity_type,
					npc_type_id
				);
				continue;
			}
			// Is the activity_information to kill this type of NPC ?
			switch (p_task_information->activity_information[activity_id].GoalMethod) {
				case METHODSINGLEID:
					if (p_task_information->activity_information[activity_id].GoalID != npc_type_id) {
						continue;
					}
					break;

				case METHODLIST:
					if (!p_task_manager->GoalListManager.IsInList(p_task_information->activity_information[activity_id].GoalID, npc_type_id)) {
						continue;
					}
					break;

				default:
					// If METHODQUEST, don't updated the activity_information here
					continue;
			}
			// We found an active p_task_information to kill this type of NPC, so increment the done count
			LogTasksDetail("Calling increment done count ByNPC");
			IncrementDoneCount(c, p_task_information, current_task->slot, activity_id);
			is_updating = true;
		}
	}

	return is_updating;
}

int ClientTaskState::ActiveSpeakTask(int npc_type_id)
{

	// This method is to be used from Perl quests only and returns the task_id of the first
	// active task found which has an active SpeakWith activity_information for this NPC.
	if (!p_task_manager || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
		return 0;
	}

	// loop over the union of tasks and quests
	for (auto & active_task : ActiveTasks) {
		auto current_task = &active_task;
		if (current_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		TaskInformation *p_task_information = p_task_manager->Tasks[current_task->task_id];
		if (p_task_information == nullptr) {
			continue;
		}

		for (int activity_id = 0; activity_id < p_task_information->activity_count; activity_id++) {
			// We are not interested in completed or hidden activities
			if (current_task->activity[activity_id].activity_state != ActivityActive) {
				continue;
			}
			if (p_task_information->activity_information[activity_id].Type != ActivitySpeakWith) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!p_task_information->activity_information[activity_id].CheckZone(zone->GetZoneID())) {
				continue;
			}
			// Is the activity_information to speak with this type of NPC ?
			if (p_task_information->activity_information[activity_id].GoalMethod == METHODQUEST && p_task_information->activity_information[activity_id].GoalID == npc_type_id) {
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
	if (!p_task_manager || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
		return -1;
	}
	if (task_id <= 0 || task_id >= MAXTASKS) {
		return -1;
	}

	// loop over the union of tasks and quests
	for (auto & ActiveTask : ActiveTasks) {
		auto current_task = &ActiveTask;
		if (current_task->task_id != task_id) {
			continue;
		}

		TaskInformation *p_task_information = p_task_manager->Tasks[current_task->task_id];
		if (p_task_information == nullptr) {
			continue;
		}

		for (int activity_index = 0; activity_index < p_task_information->activity_count; activity_index++) {
			// We are not interested in completed or hidden activities
			if (current_task->activity[activity_index].activity_state != ActivityActive) {
				continue;
			}
			if (p_task_information->activity_information[activity_index].Type != ActivitySpeakWith) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!p_task_information->activity_information[activity_index].CheckZone(zone->GetZoneID())) {
				continue;
			}
			// Is the activity_information to speak with this type of NPC ?
			if (p_task_information->activity_information[activity_index].GoalMethod == METHODQUEST &&
				p_task_information->activity_information[activity_index].GoalID == npc_type_id) {
				return activity_index;
			}
		}
		return 0;
	}
	return 0;
}

void ClientTaskState::UpdateTasksForItem(Client *c, ActivityType Type, int ItemID, int Count)
{

	// This method updates the client's task activities of the specified type which relate
	// to the specified item.
	//
	// Type should be one of ActivityLoot, ActivityTradeSkill, ActivityFish or ActivityForage

	// If the client has no tasks, there is nothing further to check.

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState::UpdateTasksForItem(%d,%d)", Type, ItemID);

	if (!p_task_manager || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
		return;
	}

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active loot activities for this task

		TaskInformation *Task = p_task_manager->Tasks[cur_task->task_id];

		if (Task == nullptr) {
			return;
		}

		for (int j = 0; j < Task->activity_count; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->activity[j].activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in the ActivityType we were called with
			if (Task->activity_information[j].Type != (int) Type) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!Task->activity_information[j].CheckZone(zone->GetZoneID())) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] Char: %s activity_information type %i for Item %i failed zone check",
					c->GetName(), Type, ItemID);
				continue;
			}
			// Is the activity_information related to this item ?
			//
			switch (Task->activity_information[j].GoalMethod) {

				case METHODSINGLEID:
					if (Task->activity_information[j].GoalID != ItemID) { continue; }
					break;

				case METHODLIST:
					if (!p_task_manager->GoalListManager.IsInList(Task->activity_information[j].GoalID, ItemID)) { continue; }
					break;

				default:
					// If METHODQUEST, don't updated the activity_information here
					continue;
			}
			// We found an active task related to this item, so increment the done count
			Log(Logs::General, Logs::Tasks, "[UPDATE] Calling increment done count ForItem");
			IncrementDoneCount(c, Task, cur_task->slot, j, Count);
		}
	}

	return;
}

void ClientTaskState::UpdateTasksOnExplore(Client *c, int ExploreID)
{

	// If the client has no tasks, there is nothing further to check.

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState::UpdateTasksOnExplore(%i)", ExploreID);
	if (!p_task_manager || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
		return;
	}

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active explore activities for this task

		TaskInformation *Task = p_task_manager->Tasks[cur_task->task_id];

		if (Task == nullptr) {
			return;
		}

		for (int j = 0; j < Task->activity_count; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->activity[j].activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in explore activities
			if (Task->activity_information[j].Type != ActivityExplore) {
				continue;
			}
			if (!Task->activity_information[j].CheckZone(zone->GetZoneID())) {
				Log(Logs::General, Logs::Tasks,
					"[UPDATE] Char: %s Explore exploreid %i failed zone check", c->GetName(),
					ExploreID);
				continue;
			}
			// Is the activity_information to explore this area id ?
			switch (Task->activity_information[j].GoalMethod) {

				case METHODSINGLEID:
					if (Task->activity_information[j].GoalID != ExploreID) {
						continue;
					}
					break;

				case METHODLIST:
					if (!p_task_manager->GoalListManager.IsInList(Task->activity_information[j].GoalID, ExploreID)) {
						continue;
					}
					break;

				default:
					// If METHODQUEST, don't updated the activity_information here
					continue;
			}
			// We found an active task to explore this area, so set done count to goal count
			// (Only a goal count of 1 makes sense for explore activities?)
			Log(Logs::General, Logs::Tasks, "[UPDATE] Increment on explore");
			IncrementDoneCount(
				c, Task, cur_task->slot, j,
				Task->activity_information[j].GoalCount - cur_task->activity[j].done_count
			);
		}
	}

	return;
}

bool ClientTaskState::UpdateTasksOnDeliver(Client *c, std::list<EQ::ItemInstance *> &Items, int Cash, int NPCTypeID)
{
	bool Ret = false;

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState::UpdateTasksForOnDeliver(%d)", NPCTypeID);

	if (!p_task_manager || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
		return false;
	}

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active deliver activities for this task

		TaskInformation *Task = p_task_manager->Tasks[cur_task->task_id];

		if (Task == nullptr) {
			return false;
		}

		for (int j = 0; j < Task->activity_count; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->activity[j].activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in Deliver activities
			if (Task->activity_information[j].Type != ActivityDeliver && Task->activity_information[j].Type != ActivityGiveCash) {
				continue;
			}
			// Is there a zone restriction on the activity_information ?
			if (!Task->activity_information[j].CheckZone(zone->GetZoneID())) {
				Log(Logs::General, Logs::Tasks,
					"[UPDATE] Char: %s Deliver activity_information failed zone check (current zone %i, need zone "
					"%s",
					c->GetName(), zone->GetZoneID(), Task->activity_information[j].zones.c_str());
				continue;
			}
			// Is the activity_information to deliver to this NPCTypeID ?
			if (Task->activity_information[j].DeliverToNPC != NPCTypeID) {
				continue;
			}
			// Is the activity_information related to these items ?
			//
			if ((Task->activity_information[j].Type == ActivityGiveCash) && Cash) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] Increment on GiveCash");
				IncrementDoneCount(c, Task, i, j, Cash);
				Ret = true;
			}
			else {
				for (auto &k : Items) {
					switch (Task->activity_information[j].GoalMethod) {

						case METHODSINGLEID:
							if (Task->activity_information[j].GoalID != k->GetID()) {
								continue;
							}
							break;

						case METHODLIST:
							if (!p_task_manager->GoalListManager.IsInList(
								Task->activity_information[j].GoalID,
								k->GetID())) {
								continue;
							}
							break;

						default:
							// If METHODQUEST, don't updated the activity_information here
							continue;
					}
					// We found an active task related to this item, so increment the done count
					Log(Logs::General, Logs::Tasks, "[UPDATE] Increment on GiveItem");
					IncrementDoneCount(c, Task, cur_task->slot, j, k->GetCharges() <= 0 ? 1 : k->GetCharges());
					Ret = true;
				}
			}
		}
	}

	return Ret;
}

void ClientTaskState::UpdateTasksOnTouch(Client *c, int ZoneID)
{
	// If the client has no tasks, there is nothing further to check.

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState::UpdateTasksOnTouch(%i)", ZoneID);
	if (!p_task_manager || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) { // could be better ...
		return;
	}

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->task_id == TASKSLOTEMPTY) {
			continue;
		}

		// Check if there are any active explore activities for this task

		TaskInformation *Task = p_task_manager->Tasks[cur_task->task_id];

		if (Task == nullptr) {
			return;
		}

		for (int j = 0; j < Task->activity_count; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->activity[j].activity_state != ActivityActive) {
				continue;
			}
			// We are only interested in touch activities
			if (Task->activity_information[j].Type != ActivityTouch) {
				continue;
			}
			if (Task->activity_information[j].GoalMethod != METHODSINGLEID) {
				continue;
			}
			if (!Task->activity_information[j].CheckZone(ZoneID)) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] Char: %s Touch activity_information failed zone check",
					c->GetName());
				continue;
			}
			// We found an active task to zone into this zone, so set done count to goal count
			// (Only a goal count of 1 makes sense for touch activities?)
			Log(Logs::General, Logs::Tasks, "[UPDATE] Increment on Touch");
			IncrementDoneCount(
				c, Task, cur_task->slot, j,
				Task->activity_information[j].GoalCount - cur_task->activity[j].done_count
			);
		}
	}

	return;
}

void ClientTaskState::IncrementDoneCount(
	Client *c, TaskInformation *Task, int TaskIndex, int ActivityID, int Count,
	bool ignore_quest_update
)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] IncrementDoneCount");

	auto info = GetClientTaskInfo(Task->type, TaskIndex);

	if (info == nullptr) {
		return;
	}

	info->activity[ActivityID].done_count += Count;

	if (info->activity[ActivityID].done_count > Task->activity_information[ActivityID].GoalCount) {
		info->activity[ActivityID].done_count = Task->activity_information[ActivityID].GoalCount;
	}

	if (!ignore_quest_update) {
		char buf[24];
		snprintf(
			buf,
			23,
			"%d %d %d",
			info->activity[ActivityID].done_count,
			info->activity[ActivityID].activity_id,
			info->task_id
		);
		buf[23] = '\0';
		parse->EventPlayer(EVENT_TASK_UPDATE, c, buf, 0);
	}

	info->activity[ActivityID].updated = true;
	// Have we reached the goal count for this activity_information ?
	if (info->activity[ActivityID].done_count >= Task->activity_information[ActivityID].GoalCount) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] Done (%i) = Goal (%i) for activity_information %i",
			info->activity[ActivityID].done_count,
			Task->activity_information[ActivityID].GoalCount,
			ActivityID);

		// Flag the activity_information as complete
		info->activity[ActivityID].activity_state = ActivityCompleted;
		// Unlock subsequent activities for this task
		bool TaskComplete = UnlockActivities(c->CharacterID(), *info);
		Log(Logs::General, Logs::Tasks, "[UPDATE] TaskCompleted is %i", TaskComplete);
		// and by the 'Task Stage Completed' message
		c->SendTaskActivityComplete(info->task_id, ActivityID, TaskIndex, Task->type);
		// Send the updated task/activity_information list to the client
		p_task_manager->SendSingleActiveTaskToClient(c, *info, TaskComplete, false);
		// Inform the client the task has been updated, both by a chat message
		c->Message(Chat::White, "Your task '%s' has been updated.", Task->title.c_str());

		if (Task->activity_information[ActivityID].GoalMethod != METHODQUEST) {
			if (!ignore_quest_update) {
				char buf[24];
				snprintf(buf, 23, "%d %d", info->task_id, info->activity[ActivityID].activity_id);
				buf[23] = '\0';
				parse->EventPlayer(EVENT_TASK_STAGE_COMPLETE, c, buf, 0);
			}
			/* QS: PlayerLogTaskUpdates :: Update */
			if (RuleB(QueryServ, PlayerLogTaskUpdates)) {
				std::string event_desc = StringFormat(
					"Task Stage Complete :: taskid:%i activityid:%i donecount:%i in zoneid:%i instid:%i",
					info->task_id,
					info->activity[ActivityID].activity_id,
					info->activity[ActivityID].done_count,
					c->GetZoneID(),
					c->GetInstanceID());
				QServ->PlayerLogEvent(Player_Log_Task_Updates, c->CharacterID(), event_desc);
			}
		}

		// If this task is now complete, the Completed tasks will have been
		// updated in UnlockActivities. Send the completed task list to the
		// client. This is the same sequence the packets are sent on live.
		if (TaskComplete) {
			char buf[24];
			snprintf(
				buf,
				23,
				"%d %d %d",
				info->activity[ActivityID].done_count,
				info->activity[ActivityID].activity_id,
				info->task_id
			);
			buf[23] = '\0';
			parse->EventPlayer(EVENT_TASK_COMPLETE, c, buf, 0);

			/* QS: PlayerLogTaskUpdates :: Complete */
			if (RuleB(QueryServ, PlayerLogTaskUpdates)) {
				std::string event_desc = StringFormat(
					"Task Complete :: taskid:%i activityid:%i donecount:%i in zoneid:%i instid:%i",
					info->task_id,
					info->activity[ActivityID].activity_id,
					info->activity[ActivityID].done_count,
					c->GetZoneID(),
					c->GetInstanceID());
				QServ->PlayerLogEvent(Player_Log_Task_Updates, c->CharacterID(), event_desc);
			}

			p_task_manager->SendCompletedTasksToClient(c, this);
			c->SendTaskActivityComplete(info->task_id, 0, TaskIndex, Task->type, 0);
			p_task_manager->SaveClientState(c, this);
			//c->SendTaskComplete(TaskIndex);
			c->CancelTask(TaskIndex, Task->type);
			//if(Task->reward_method != METHODQUEST) RewardTask(c, Task);
			// If Experience and/or cash rewards are set, reward them from the task even if reward_method is METHODQUEST
			RewardTask(c, Task);
			//RemoveTask(c, TaskIndex);

		}

	}
	else {
		// Send an updated packet for this single activity_information
		p_task_manager->SendTaskActivityLong(
			c, info->task_id, ActivityID, TaskIndex,
			Task->activity_information[ActivityID].Optional
		);
		p_task_manager->SaveClientState(c, this);
	}
}

void ClientTaskState::RewardTask(Client *c, TaskInformation *Task)
{

	if (!Task || !c) { return; }

	const EQ::ItemData *Item;
	std::vector<int>   RewardList;

	switch (Task->reward_method) {

		case METHODSINGLEID: {
			if (Task->reward_id) {
				c->SummonItem(Task->reward_id);
				Item = database.GetItem(Task->reward_id);
				if (Item) {
					c->Message(Chat::Yellow, "You receive %s as a reward.", Item->Name);
				}
			}
			break;
		}
		case METHODLIST: {
			RewardList = p_task_manager->GoalListManager.GetListContents(Task->reward_id);
			for (unsigned int i = 0; i < RewardList.size(); i++) {
				c->SummonItem(RewardList[i]);
				Item = database.GetItem(RewardList[i]);
				if (Item) {
					c->Message(Chat::Yellow, "You receive %s as a reward.", Item->Name);
				}
			}
			break;
		}
		default: {
			// Nothing special done for METHODQUEST
			break;
		}
	}

	if (!Task->completion_emote.empty()) {
		c->SendColoredText(
			Chat::Yellow,
			Task->completion_emote
		);
	} // unsure if they use this packet or color, should work

	// just use normal NPC faction ID stuff
	if (Task->faction_reward)
		c->SetFactionLevel(c->CharacterID(), Task->faction_reward, c->GetBaseClass(), c->GetBaseRace(), c->GetDeity());

	if (Task->cash_reward) {
		int Plat, Gold, Silver, Copper;

		Copper = Task->cash_reward;
		c->AddMoneyToPP(Copper, true);

		Plat   = Copper / 1000;
		Copper = Copper - (Plat * 1000);
		Gold   = Copper / 100;
		Copper = Copper - (Gold * 100);
		Silver = Copper / 10;
		Copper = Copper - (Silver * 10);

		std::string CashMessage;

		if (Plat > 0) {
			CashMessage = "You receive ";
			CashMessage += itoa(Plat);
			CashMessage += " platinum";
		}
		if (Gold > 0) {
			if (CashMessage.length() == 0) {
				CashMessage = "You receive ";
			}
			else {
				CashMessage += ",";
			}
			CashMessage += itoa(Gold);
			CashMessage += " gold";
		}
		if (Silver > 0) {
			if (CashMessage.length() == 0) {
				CashMessage = "You receive ";
			}
			else {
				CashMessage += ",";
			}
			CashMessage += itoa(Silver);
			CashMessage += " silver";
		}
		if (Copper > 0) {
			if (CashMessage.length() == 0) {
				CashMessage = "You receive ";
			}
			else {
				CashMessage += ",";
			}
			CashMessage += itoa(Copper);
			CashMessage += " copper";
		}
		CashMessage += " pieces.";
		c->Message(Chat::Yellow, CashMessage.c_str());
	}
	int32 EXPReward = Task->experience_reward;
	if (EXPReward > 0) {
		c->AddEXP(EXPReward);
	}
	if (EXPReward < 0) {
		uint32 PosReward = EXPReward * -1;
		// Minimal Level Based Exp reward Setting is 101 (1% exp at level 1)
		if (PosReward > 100 && PosReward < 25700) {
			uint8 MaxLevel   = PosReward / 100;
			uint8 ExpPercent = PosReward - (MaxLevel * 100);
			c->AddLevelBasedExp(ExpPercent, MaxLevel);
		}
	}

	c->SendSound();
}

bool ClientTaskState::IsTaskActive(int TaskID)
{
	if (active_task.task_id == TaskID) {
		return true;
	}

	if (active_task_count == 0 || TaskID == 0) {
		return false;
	}

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (active_quests[i].task_id == TaskID) {
			return true;
		}
	}

	return false;
}

void ClientTaskState::FailTask(Client *c, int TaskID)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] FailTask %i, ActiveTaskCount is %i", TaskID, active_task_count);

	if (active_task.task_id == TaskID) {
		c->SendTaskFailed(TaskID, 0, TaskType::Task);
		// Remove the task from the client
		c->CancelTask(0, TaskType::Task);
		return;
	}

	// TODO: shared tasks

	if (active_task_count == 0) {
		return;
	}

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (active_quests[i].task_id == TaskID) {
			c->SendTaskFailed(active_quests[i].task_id, i, TaskType::Quest);
			// Remove the task from the client
			c->CancelTask(i, TaskType::Quest);
			return;
		}
	}
}

// TODO: Shared tasks
bool ClientTaskState::IsTaskActivityActive(int TaskID, int ActivityID)
{

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState IsTaskActivityActive(%i, %i).", TaskID, ActivityID);
	// Quick sanity check
	if (ActivityID < 0) {
		return false;
	}
	if (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY) {
		return false;
	}

	int  ActiveTaskIndex = -1;
	auto type            = TaskType::Task;

	if (active_task.task_id == TaskID) {
		ActiveTaskIndex = 0;
	}

	if (ActiveTaskIndex == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (active_quests[i].task_id == TaskID) {
				ActiveTaskIndex = i;
				type            = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (ActiveTaskIndex == -1) {
		return false;
	}

	auto info = GetClientTaskInfo(type, ActiveTaskIndex);

	if (info == nullptr) {
		return false;
	}

	TaskInformation *Task = p_task_manager->Tasks[info->task_id];

	// The task is invalid
	if (Task == nullptr) {
		return false;
	}

	// The activity_id is out of range
	if (ActivityID >= Task->activity_count) {
		return false;
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState IsTaskActivityActive(%i, %i). activity_state is %i ", TaskID,
		ActivityID, info->activity[ActivityID].activity_state);

	return (info->activity[ActivityID].activity_state == ActivityActive);
}

void ClientTaskState::UpdateTaskActivity(
	Client *c,
	int TaskID,
	int ActivityID,
	int Count,
	bool ignore_quest_update /*= false*/)
{

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState UpdateTaskActivity(%i, %i, %i).", TaskID, ActivityID,
		Count);

	// Quick sanity check
	if (ActivityID < 0 || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  ActiveTaskIndex = -1;
	auto type            = TaskType::Task;

	if (active_task.task_id == TaskID) {
		ActiveTaskIndex = 0;
	}

	if (ActiveTaskIndex == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (active_quests[i].task_id == TaskID) {
				ActiveTaskIndex = i;
				type            = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (ActiveTaskIndex == -1) {
		return;
	}

	auto info = GetClientTaskInfo(type, ActiveTaskIndex);

	if (info == nullptr) {
		return;
	}

	TaskInformation *Task = p_task_manager->Tasks[info->task_id];

	// The task is invalid
	if (Task == nullptr) {
		return;
	}

	// The activity_id is out of range
	if (ActivityID >= Task->activity_count) {
		return;
	}

	// The activity_information is not currently active
	if (info->activity[ActivityID].activity_state == ActivityHidden) {
		return;
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] Increment done count on UpdateTaskActivity %d %d", ActivityID, Count);
	IncrementDoneCount(c, Task, ActiveTaskIndex, ActivityID, Count, ignore_quest_update);
}

void ClientTaskState::ResetTaskActivity(Client *c, int TaskID, int ActivityID)
{
	Log(Logs::General, Logs::Tasks, "[RESET] ClientTaskState ResetTaskActivity(%i, %i).", TaskID, ActivityID);

	// Quick sanity check
	if (ActivityID < 0 || (active_task_count == 0 && active_task.task_id == TASKSLOTEMPTY)) {
		return;
	}

	int  ActiveTaskIndex = -1;
	auto type            = TaskType::Task;

	if (active_task.task_id == TaskID) {
		ActiveTaskIndex = 0;
	}

	if (ActiveTaskIndex == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (active_quests[i].task_id == TaskID) {
				ActiveTaskIndex = i;
				type            = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (ActiveTaskIndex == -1) {
		return;
	}

	auto info = GetClientTaskInfo(type, ActiveTaskIndex);

	if (info == nullptr) {
		return;
	}

	TaskInformation *Task = p_task_manager->Tasks[info->task_id];

	// The task is invalid
	if (Task == nullptr) {
		return;
	}

	// The activity_id is out of range
	if (ActivityID >= Task->activity_count) {
		return;
	}

	// The activity_information is not currently active
	if (info->activity[ActivityID].activity_state == ActivityHidden) {
		return;
	}

	Log(Logs::General, Logs::Tasks, "[RESET] Increment done count on ResetTaskActivity");
	IncrementDoneCount(c, Task, ActiveTaskIndex, ActivityID, (info->activity[ActivityID].done_count * -1), false);
}

void ClientTaskState::ShowClientTasks(Client *c)
{
	c->Message(Chat::White, "Task Information:");
	if (active_task.task_id != TASKSLOTEMPTY) {
		c->Message(Chat::White, "Task: %i %s", active_task.task_id, p_task_manager->Tasks[active_task.task_id]->title.c_str());
		c->Message(Chat::White, "  description: [%s]\n", p_task_manager->Tasks[active_task.task_id]->description.c_str());
		for (int j = 0; j < p_task_manager->GetActivityCount(active_task.task_id); j++) {
			c->Message(
				Chat::White, "  activity_information: %2d, done_count: %2d, Status: %d (0=Hidden, 1=Active, 2=Complete)",
				active_task.activity[j].activity_id, active_task.activity[j].done_count,
				active_task.activity[j].activity_state
			);
		}
	}

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (active_quests[i].task_id == TASKSLOTEMPTY) {
			continue;
		}

		c->Message(
			Chat::White, "Quest: %i %s", active_quests[i].task_id,
			p_task_manager->Tasks[active_quests[i].task_id]->title.c_str());
		c->Message(
			Chat::White,
			"  description: [%s]\n",
			p_task_manager->Tasks[active_quests[i].task_id]->description.c_str());
		for (int j = 0; j < p_task_manager->GetActivityCount(active_quests[i].task_id); j++) {
			c->Message(
				Chat::White, "  activity_information: %2d, done_count: %2d, Status: %d (0=Hidden, 1=Active, 2=Complete)",
				active_quests[i].activity[j].activity_id, active_quests[i].activity[j].done_count,
				active_quests[i].activity[j].activity_state
			);
		}
	}
}

// TODO: Shared Task
int ClientTaskState::TaskTimeLeft(int TaskID)
{
	if (active_task.task_id == TaskID) {
		int Now = time(nullptr);

		TaskInformation *Task = p_task_manager->Tasks[TaskID];

		if (Task == nullptr) {
			return -1;
		}

		if (!Task->duration) {
			return -1;
		}

		int TimeLeft = (active_task.accepted_time + Task->duration - Now);

		return (TimeLeft > 0 ? TimeLeft : 0);
	}

	if (active_task_count == 0) {
		return -1;
	}

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {

		if (active_quests[i].task_id != TaskID) {
			continue;
		}

		int Now = time(nullptr);

		TaskInformation *Task = p_task_manager->Tasks[active_quests[i].task_id];

		if (Task == nullptr) {
			return -1;
		}

		if (!Task->duration) {
			return -1;
		}

		int TimeLeft = (active_quests[i].accepted_time + Task->duration - Now);

		// If Timeleft is negative, return 0, else return the number of seconds left

		return (TimeLeft > 0 ? TimeLeft : 0);
	}

	return -1;
}

int ClientTaskState::IsTaskCompleted(int TaskID)
{

	// Returns:	-1 if RecordCompletedTasks is not true
	//		+1 if the task has been completed
	//		0 if the task has not been completed

	if (!(RuleB(TaskSystem, RecordCompletedTasks))) { return -1; }

	for (unsigned int i = 0; i < completed_tasks.size(); i++) {
		Log(Logs::General,
			Logs::Tasks,
			"[UPDATE] Comparing completed task %i with %i",
			completed_tasks[i].task_id,
			TaskID);
		if (completed_tasks[i].task_id == TaskID) { return 1; }
	}

	return 0;
}

bool TaskManager::IsTaskRepeatable(int TaskID)
{

	if ((TaskID <= 0) || (TaskID >= MAXTASKS)) { return false; }

	TaskInformation *Task = p_task_manager->Tasks[TaskID];

	if (Task == nullptr) { return false; }

	return Task->repeatable;
}

bool ClientTaskState::TaskOutOfTime(TaskType type, int Index)
{
	// Returns true if the Task in the specified slot has a time limit that has been exceeded.
	auto info = GetClientTaskInfo(type, Index);

	if (info == nullptr) {
		return false;
	}

	// make sure the task_id is at least maybe in our array
	if (info->task_id <= 0 || info->task_id >= MAXTASKS) {
		return false;
	}

	int Now = time(nullptr);

	TaskInformation *Task = p_task_manager->Tasks[info->task_id];

	if (Task == nullptr) {
		return false;
	}

	return (Task->duration && (info->accepted_time + Task->duration <= Now));
}

void ClientTaskState::TaskPeriodicChecks(Client *c)
{
	if (active_task.task_id != TASKSLOTEMPTY) {
		if (TaskOutOfTime(TaskType::Task, 0)) {
			// Send Red Task Failed Message
			c->SendTaskFailed(active_task.task_id, 0, TaskType::Task);
			// Remove the task from the client
			c->CancelTask(0, TaskType::Task);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			return;
		}
	}

	// TODO: shared tasks -- although that will probably be manager in world checking and telling zones to fail us

	if (active_task_count == 0) {
		return;
	}

	// Check for tasks that have failed because they have not been completed in the specified time
	//
	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (active_quests[i].task_id == TASKSLOTEMPTY) {
			continue;
		}

		if (TaskOutOfTime(TaskType::Quest, i)) {
			// Send Red Task Failed Message
			c->SendTaskFailed(active_quests[i].task_id, i, TaskType::Quest);
			// Remove the task from the client
			c->CancelTask(i, TaskType::Quest);
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
	if (!checked_touch_activities) {
		UpdateTasksOnTouch(c, zone->GetZoneID());
		checked_touch_activities = true;
	}
}

#if 0
void Client::SendTaskComplete(int TaskIndex) {

	// 0x4c8c

	TaskComplete_Struct* tcs;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskComplete, sizeof(TaskComplete_Struct));

	tcs = (TaskComplete_Struct*)outapp->pBuffer;

	// I have seen unknown0 as non-zero. It always seems to match the value in the first word of the
	// Task activity_information Complete packet sent immediately prior to it.
	//tcs->unknown00 = 0x00000000;
	tcs->unknown00 = TaskIndex;
	// I have only seen 0x00000002 in the next field. This is a common 'unknown' value in the task packets.
	// I suspect this is the type field to indicate this is a quest task, as opposed to other types.
	tcs->unknown04 = 0x00000002;

	Log.LogDebugType(Logs::Detail, Logs::Tasks, "SendTasksComplete");
	DumpPacket(outapp); fflush(stdout);

	QueuePacket(outapp);
	safe_delete(outapp);



}
#endif

void ClientTaskState::SendTaskHistory(Client *c, int TaskIndex)
{

	Log(Logs::General, Logs::Tasks, "[UPDATE] Task History Requested for Completed Task Index %i", TaskIndex);

	// We only sent the most recent 50 completed tasks, so we need to offset the Index the client sent to us.

	int AdjustedTaskIndex = TaskIndex;

	if (completed_tasks.size() > 50) {
		AdjustedTaskIndex += (completed_tasks.size() - 50);
	}

	if ((AdjustedTaskIndex < 0) || (AdjustedTaskIndex >= (int) completed_tasks.size())) { return; }

	int TaskID = completed_tasks[AdjustedTaskIndex].task_id;

	if ((TaskID < 0) || (TaskID > MAXTASKS)) { return; }

	TaskInformation *Task = p_task_manager->Tasks[TaskID];

	if (Task == nullptr) { return; }

	TaskHistoryReplyHeader_Struct *ths;
	TaskHistoryReplyData1_Struct  *thd1;
	TaskHistoryReplyData2_Struct  *thd2;

	char *Ptr;

	int CompletedActivityCount = 0;;

	int PacketLength = sizeof(TaskHistoryReplyHeader_Struct);

	for (int i = 0; i < Task->activity_count; i++) {
		if (completed_tasks[AdjustedTaskIndex].activity_done[i]) {
			CompletedActivityCount++;
			PacketLength = PacketLength + sizeof(TaskHistoryReplyData1_Struct) +
						   Task->activity_information[i].target_name.size() + 1 +
						   Task->activity_information[i].item_list.size() + 1 +
						   sizeof(TaskHistoryReplyData2_Struct) +
						   Task->activity_information[i].desc_override.size() + 1;
		}
	}

	auto outapp = new EQApplicationPacket(OP_TaskHistoryReply, PacketLength);

	ths = (TaskHistoryReplyHeader_Struct *) outapp->pBuffer;

	// We use the TaskIndex the client sent in the request
	ths->TaskID = TaskIndex;

	ths->ActivityCount = CompletedActivityCount;

	Ptr = (char *) ths + sizeof(TaskHistoryReplyHeader_Struct);

	for (int i = 0; i < Task->activity_count; i++) {
		if (completed_tasks[AdjustedTaskIndex].activity_done[i]) {
			thd1 = (TaskHistoryReplyData1_Struct *) Ptr;
			thd1->ActivityType = Task->activity_information[i].Type;
			Ptr = (char *) thd1 + sizeof(TaskHistoryReplyData1_Struct);
			VARSTRUCT_ENCODE_STRING(Ptr, Task->activity_information[i].target_name.c_str());
			VARSTRUCT_ENCODE_STRING(Ptr, Task->activity_information[i].item_list.c_str());
			thd2 = (TaskHistoryReplyData2_Struct *) Ptr;
			thd2->GoalCount = Task->activity_information[i].GoalCount;
			thd2->unknown04 = 0xffffffff;
			thd2->unknown08 = 0xffffffff;
			thd2->ZoneID    = Task->activity_information[i].ZoneIDs.empty() ? 0 : Task->activity_information[i].ZoneIDs.front();
			thd2->unknown16 = 0x00000000;
			Ptr = (char *) thd2 + sizeof(TaskHistoryReplyData2_Struct);
			VARSTRUCT_ENCODE_STRING(Ptr, Task->activity_information[i].desc_override.c_str());
		}
	}


	c->QueuePacket(outapp);
	safe_delete(outapp);


}

void Client::SendTaskActivityComplete(int task_id, int activity_id, int task_index, TaskType task_type, int task_incomplete)
{
	// 0x54eb

	TaskActivityComplete_Struct *tac;

	auto outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	tac = (TaskActivityComplete_Struct *) outapp->pBuffer;

	tac->TaskIndex      = task_index;
	tac->TaskType       = static_cast<uint32>(task_type);
	tac->TaskID         = task_id;
	tac->ActivityID     = activity_id;
	tac->task_completed = 0x00000001;
	tac->stage_complete = task_incomplete;


	QueuePacket(outapp);
	safe_delete(outapp);
}


void Client::SendTaskFailed(int task_id, int task_index, TaskType task_type)
{
	// 0x54eb
	char buf[24];
	snprintf(buf, 23, "%d", task_id);
	buf[23] = '\0';
	parse->EventPlayer(EVENT_TASK_FAIL, this, buf, 0);

	TaskActivityComplete_Struct *tac;

	auto outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	tac = (TaskActivityComplete_Struct *) outapp->pBuffer;

	tac->TaskIndex      = task_index;
	tac->TaskType       = static_cast<uint32>(task_type);
	tac->TaskID         = task_id;
	tac->ActivityID     = 0;
	tac->task_completed = 0; //Fail
	tac->stage_complete = 0; // 0 for task complete or failed.

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskFailed");

	QueuePacket(outapp);
	safe_delete(outapp);
}

void TaskManager::SendCompletedTasksToClient(Client *c, ClientTaskState *State)
{

	int PacketLength = 4;



	//vector<CompletedTaskInformation>::const_iterator iterator;

	// The client only display the first 50 Completed Tasks send, so send the 50 most recent
	int FirstTaskToSend = 0;
	int LastTaskToSend  = State->completed_tasks.size();

	if (State->completed_tasks.size() > 50) {
		FirstTaskToSend = State->completed_tasks.size() - 50;
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] Completed Task Count: %i, First Task to send is %i, Last is %i",
		State->completed_tasks.size(), FirstTaskToSend, LastTaskToSend);
	/*
	for(iterator=activity_state->CompletedTasks.begin(); iterator!=activity_state->CompletedTasks.end(); iterator++) {
		int task_id = (*iterator).task_id;
		if(Tasks[task_id] == nullptr) continue;
		PacketLength = PacketLength + 8 + strlen(Tasks[task_id]->title) + 1;
	}
	*/
	for (int i = FirstTaskToSend; i < LastTaskToSend; i++) {
		int TaskID = State->completed_tasks[i].task_id;
		if (Tasks[TaskID] == nullptr) { continue; }
		PacketLength = PacketLength + 8 + Tasks[TaskID]->title.size() + 1;
	}

	auto outapp = new EQApplicationPacket(OP_CompletedTasks, PacketLength);
	char *buf   = (char *) outapp->pBuffer;

	//*(uint32 *)buf = activity_state->CompletedTasks.size();
	*(uint32 *) buf = LastTaskToSend - FirstTaskToSend;
	buf = buf + 4;
	//for(iterator=activity_state->CompletedTasks.begin(); iterator!=activity_state->CompletedTasks.end(); iterator++) {
	//	int task_id = (*iterator).task_id;
	for (int i = FirstTaskToSend; i < LastTaskToSend; i++) {
		int TaskID = State->completed_tasks[i].task_id;
		if (Tasks[TaskID] == nullptr) { continue; }
		*(uint32 *) buf = TaskID;
		buf = buf + 4;

		sprintf(buf, "%s", Tasks[TaskID]->title.c_str());
		buf = buf + strlen(buf) + 1;
		//*(uint32 *)buf = (*iterator).CompletedTime;
		*(uint32 *) buf = State->completed_tasks[i].completed_time;
		buf = buf + 4;
	}


	c->QueuePacket(outapp);
	safe_delete(outapp);
}


void TaskManager::SendTaskActivityShort(Client *c, int TaskID, int ActivityID, int ClientTaskIndex)
{
	// This activity_information Packet is sent for activities that have not yet been unlocked and appear as ???
	// in the client.

	TaskActivityShort_Struct *tass;

	if (c->ClientVersionBit() & EQ::versions::maskRoFAndLater) {
		auto outapp = new EQApplicationPacket(OP_TaskActivity, 25);
		outapp->WriteUInt32(ClientTaskIndex);
		outapp->WriteUInt32(static_cast<uint32>(Tasks[TaskID]->type));
		outapp->WriteUInt32(TaskID);
		outapp->WriteUInt32(ActivityID);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0xffffffff);
		outapp->WriteUInt8(0);
		c->FastQueuePacket(&outapp);

		return;
	}

	auto outapp = new EQApplicationPacket(OP_TaskActivity, sizeof(TaskActivityShort_Struct));

	tass = (TaskActivityShort_Struct *) outapp->pBuffer;

	tass->TaskSequenceNumber = ClientTaskIndex;
	tass->unknown2           = static_cast<uint32>(Tasks[TaskID]->type);
	tass->TaskID             = TaskID;
	tass->ActivityID         = ActivityID;
	tass->unknown3           = 0x000000;
	tass->ActivityType       = 0xffffffff;
	tass->unknown4           = 0x00000000;


	c->QueuePacket(outapp);
	safe_delete(outapp);
}


void TaskManager::SendTaskActivityLong(
	Client *c,
	int TaskID,
	int ActivityID,
	int ClientTaskIndex,
	bool Optional,
	bool TaskComplete
)
{

	if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF) {
		SendTaskActivityNew(c, TaskID, ActivityID, ClientTaskIndex, Optional, TaskComplete);
		return;
	}

	SerializeBuffer buf(100);

	buf.WriteUInt32(ClientTaskIndex);
	buf.WriteUInt32(static_cast<uint32>(Tasks[TaskID]->type));
	buf.WriteUInt32(TaskID);
	buf.WriteUInt32(ActivityID);
	buf.WriteUInt32(0); // unknown3

	// We send our 'internal' types as ActivityCastOn. text3 should be set to the activity_information description, so it makes
	// no difference to the client. All activity_information updates will be done based on our interal activity_information types.
	if ((Tasks[TaskID]->activity_information[ActivityID].Type > 0) && Tasks[TaskID]->activity_information[ActivityID].Type < 100) {
		buf.WriteUInt32(Tasks[TaskID]->activity_information[ActivityID].Type);
	}
	else {
		buf.WriteUInt32(ActivityCastOn);
	} // w/e!

	buf.WriteUInt32(Optional);
	buf.WriteUInt32(0);        // solo, group, raid

	buf.WriteString(Tasks[TaskID]->activity_information[ActivityID].target_name); // target name string
	buf.WriteString(Tasks[TaskID]->activity_information[ActivityID].item_list); // item name list

	if (Tasks[TaskID]->activity_information[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(Tasks[TaskID]->activity_information[ActivityID].GoalCount);
	else
		// For our internal type GiveCash, where the goal count has the amount of cash that must be given,
		// we don't want the donecount and goalcount fields cluttered up with potentially large numbers, so we just
		// send a goalcount of 1, and a bit further down, a donecount of 1 if the activity_information is complete, 0 otherwise.
		// The text3 field should decribe the exact activity_information goal, e.g. give 3500gp to Hasten Bootstrutter.
		buf.WriteUInt32(1);

	buf.WriteUInt32(Tasks[TaskID]->activity_information[ActivityID].skill_id);
	buf.WriteUInt32(Tasks[TaskID]->activity_information[ActivityID].spell_id);
	buf.WriteUInt32(
		Tasks[TaskID]->activity_information[ActivityID].ZoneIDs.empty() ? 0 : Tasks[TaskID]->activity_information[ActivityID].ZoneIDs.front());
	buf.WriteUInt32(0);

	buf.WriteString(Tasks[TaskID]->activity_information[ActivityID].desc_override);

	if (Tasks[TaskID]->activity_information[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID));
	else
		// For internal activity_information types, done_count is either 1 if the activity_information is complete, 0 otherwise.
		buf.WriteUInt32((c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID) >=
						 Tasks[TaskID]->activity_information[ActivityID].GoalCount));

	buf.WriteUInt32(1); // unknown

	auto outapp = new EQApplicationPacket(OP_TaskActivity, buf);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

// Used only by RoF+ Clients
void TaskManager::SendTaskActivityNew(
	Client *c,
	int TaskID,
	int ActivityID,
	int ClientTaskIndex,
	bool Optional,
	bool TaskComplete
)
{
	SerializeBuffer buf(100);

	buf.WriteUInt32(ClientTaskIndex);    // TaskSequenceNumber
	buf.WriteUInt32(static_cast<uint32>(Tasks[TaskID]->type));        // task type
	buf.WriteUInt32(TaskID);
	buf.WriteUInt32(ActivityID);
	buf.WriteUInt32(0);        // unknown3

	// We send our 'internal' types as ActivityCastOn. text3 should be set to the activity_information description, so it makes
	// no difference to the client. All activity_information updates will be done based on our interal activity_information types.
	if ((Tasks[TaskID]->activity_information[ActivityID].Type > 0) && Tasks[TaskID]->activity_information[ActivityID].Type < 100) {
		buf.WriteUInt32(Tasks[TaskID]->activity_information[ActivityID].Type);
	}
	else {
		buf.WriteUInt32(ActivityCastOn);
	} // w/e!

	buf.WriteUInt8(Optional);
	buf.WriteUInt32(0);        // solo, group, raid

	// One of these unknown fields maybe related to the 'Use On' activity_information types
	buf.WriteString(Tasks[TaskID]->activity_information[ActivityID].target_name); // target name string

	buf.WriteLengthString(Tasks[TaskID]->activity_information[ActivityID].item_list); // item name list

	// Goal Count
	if (Tasks[TaskID]->activity_information[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(Tasks[TaskID]->activity_information[ActivityID].GoalCount);
	else
		buf.WriteUInt32(1);    // GoalCount

	// skill ID list ; separated
	buf.WriteLengthString(Tasks[TaskID]->activity_information[ActivityID].skill_list);

	// spelll ID list ; separated -- unsure wtf we're doing here
	buf.WriteLengthString(Tasks[TaskID]->activity_information[ActivityID].spell_list);

	buf.WriteString(Tasks[TaskID]->activity_information[ActivityID].zones);
	buf.WriteUInt32(0);        // unknown7

	buf.WriteString(Tasks[TaskID]->activity_information[ActivityID].desc_override); // description override

	if (Tasks[TaskID]->activity_information[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID));    // done_count
	else
		// For internal activity_information types, done_count is either 1 if the activity_information is complete, 0 otherwise.
		buf.WriteUInt32((c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID) >=
						 Tasks[TaskID]->activity_information[ActivityID].GoalCount));

	buf.WriteUInt8(1);    // unknown9

	buf.WriteString(Tasks[TaskID]->activity_information[ActivityID].zones);

	auto outapp = new EQApplicationPacket(OP_TaskActivity, buf);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

void TaskManager::SendActiveTasksToClient(Client *c, bool TaskComplete)
{
	auto state = c->GetTaskState();
	if (!state) {
		return;
	}

	for (int TaskIndex = 0; TaskIndex < MAXACTIVEQUESTS + 1; TaskIndex++) {
		int TaskID = state->ActiveTasks[TaskIndex].task_id;
		if ((TaskID == 0) || (Tasks[TaskID] == 0)) {
			continue;
		}
		int StartTime = state->ActiveTasks[TaskIndex].accepted_time;

		SendActiveTaskDescription(
			c, TaskID, state->ActiveTasks[TaskIndex], StartTime, Tasks[TaskID]->duration,
			false
		);
		Log(Logs::General, Logs::Tasks, "[UPDATE] SendActiveTasksToClient: Task %i, Activities: %i", TaskID,
			GetActivityCount(TaskID));

		int      Sequence    = 0;
		int      fixed_index = Tasks[TaskID]->type == TaskType::Task ? 0 : TaskIndex - 1; // hmmm fuck
		for (int Activity    = 0; Activity < GetActivityCount(TaskID); Activity++) {
			if (c->GetTaskActivityState(Tasks[TaskID]->type, fixed_index, Activity) != ActivityHidden) {
				Log(Logs::General, Logs::Tasks, "[UPDATE]   Long: %i, %i, %i Complete=%i", TaskID,
					Activity, fixed_index, TaskComplete);
				if (Activity == GetActivityCount(TaskID) - 1) {
					SendTaskActivityLong(
						c, TaskID, Activity, fixed_index,
						Tasks[TaskID]->activity_information[Activity].Optional, TaskComplete
					);
				}
				else {
					SendTaskActivityLong(
						c, TaskID, Activity, fixed_index,
						Tasks[TaskID]->activity_information[Activity].Optional, 0
					);
				}
			}
			else {
				Log(Logs::General, Logs::Tasks, "[UPDATE]   Short: %i, %i, %i", TaskID, Activity,
					fixed_index);
				SendTaskActivityShort(c, TaskID, Activity, fixed_index);
			}
			Sequence++;
		}
	}
}

void TaskManager::SendSingleActiveTaskToClient(
	Client *c, ClientTaskInformation &task_info, bool TaskComplete,
	bool BringUpTaskJournal
)
{
	int TaskID = task_info.task_id;

	if (TaskID == 0 || Tasks[TaskID] == nullptr) {
		return;
	}

	int StartTime = task_info.accepted_time;
	SendActiveTaskDescription(c, TaskID, task_info, StartTime, Tasks[TaskID]->duration, BringUpTaskJournal);
	Log(Logs::General,
		Logs::Tasks,
		"[UPDATE] SendSingleActiveTasksToClient: Task %i, Activities: %i",
		TaskID,
		GetActivityCount(TaskID));

	for (int Activity = 0; Activity < GetActivityCount(TaskID); Activity++) {
		if (task_info.activity[Activity].activity_state != ActivityHidden) {
			Log(Logs::General, Logs::Tasks, "[UPDATE]   Long: %i, %i Complete=%i", TaskID, Activity, TaskComplete);
			if (Activity == GetActivityCount(TaskID) - 1) {
				SendTaskActivityLong(
					c, TaskID, Activity, task_info.slot,
					Tasks[TaskID]->activity_information[Activity].Optional, TaskComplete
				);
			}
			else {
				SendTaskActivityLong(
					c, TaskID, Activity, task_info.slot,
					Tasks[TaskID]->activity_information[Activity].Optional, 0
				);
			}
		}
		else {
			Log(Logs::General, Logs::Tasks, "[UPDATE]   Short: %i, %i", TaskID, Activity);
			SendTaskActivityShort(c, TaskID, Activity, task_info.slot);
		}
	}
}

void TaskManager::SendActiveTaskDescription(
	Client *c,
	int TaskID,
	ClientTaskInformation &task_info,
	int StartTime,
	int Duration,
	bool BringUpTaskJournal
)
{
	if ((TaskID < 1) || (TaskID >= MAXTASKS) || !Tasks[TaskID]) {
		return;
	}

	int PacketLength = sizeof(TaskDescriptionHeader_Struct) + Tasks[TaskID]->title.length() + 1
					   + sizeof(TaskDescriptionData1_Struct) + Tasks[TaskID]->description.length() + 1
					   + sizeof(TaskDescriptionData2_Struct) + 1 + sizeof(TaskDescriptionTrailer_Struct);

	// If there is an item make the reward text into a link to the item (only the first item if a list
	// is specified). I have been unable to get multiple item links to work.
	//
	if (Tasks[TaskID]->reward_id && Tasks[TaskID]->item_link.empty()) {
		int ItemID = 0;
		// If the reward is a list of items, and the first entry on the list is valid
		if (Tasks[TaskID]->reward_method == METHODSINGLEID) {
			ItemID = Tasks[TaskID]->reward_id;
		}
		else if (Tasks[TaskID]->reward_method == METHODLIST) {
			ItemID = GoalListManager.GetFirstEntry(Tasks[TaskID]->reward_id);
			if (ItemID < 0) {
				ItemID = 0;
			}
		}

		if (ItemID) {
			const EQ::ItemData *reward_item = database.GetItem(ItemID);

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkItemData);
			linker.SetItemData(reward_item);
			linker.SetTaskUse();
			Tasks[TaskID]->item_link = linker.GenerateLink();
		}
	}

	PacketLength += Tasks[TaskID]->reward.length() + 1 + Tasks[TaskID]->item_link.length() + 1;

	char                          *Ptr;
	TaskDescriptionHeader_Struct  *tdh;
	TaskDescriptionData1_Struct   *tdd1;
	TaskDescriptionData2_Struct   *tdd2;
	TaskDescriptionTrailer_Struct *tdt;

	auto outapp = new EQApplicationPacket(OP_TaskDescription, PacketLength);

	tdh = (TaskDescriptionHeader_Struct *) outapp->pBuffer;

	tdh->SequenceNumber = task_info.slot;
	tdh->TaskID         = TaskID;
	tdh->open_window    = BringUpTaskJournal;
	tdh->task_type      = static_cast<uint32>(Tasks[TaskID]->type);
	tdh->reward_type    = 0; // TODO: 4 says Radiant Crystals else Ebon Crystals when shared task

	Ptr = (char *) tdh + sizeof(TaskDescriptionHeader_Struct);

	sprintf(Ptr, "%s", Tasks[TaskID]->title.c_str());
	Ptr += Tasks[TaskID]->title.length() + 1;

	tdd1 = (TaskDescriptionData1_Struct *) Ptr;

	tdd1->Duration = Duration;
	tdd1->dur_code = static_cast<uint32>(Tasks[TaskID]->duration_code);

	tdd1->StartTime = StartTime;

	Ptr = (char *) tdd1 + sizeof(TaskDescriptionData1_Struct);

	sprintf(Ptr, "%s", Tasks[TaskID]->description.c_str());
	Ptr += Tasks[TaskID]->description.length() + 1;

	tdd2 = (TaskDescriptionData2_Struct *) Ptr;

	// we have this reward stuff!
	// if we ever don't hardcode this, TaskDescriptionTrailer_Struct will need to be fixed since
	// "has_reward_selection" is after this bool! Smaller packet when this is 0
	tdd2->has_rewards = 1;

	tdd2->coin_reward    = Tasks[TaskID]->cash_reward;
	tdd2->xp_reward      = Tasks[TaskID]->experience_reward ? 1 : 0; // just booled
	tdd2->faction_reward = Tasks[TaskID]->faction_reward ? 1 : 0; // faction booled

	Ptr = (char *) tdd2 + sizeof(TaskDescriptionData2_Struct);

	// we actually have 2 strings here. One is max length 96 and not parsed for item links
	// We actually skipped past that string incorrectly before, so TODO: fix item link string
	sprintf(Ptr, "%s", Tasks[TaskID]->reward.c_str());
	Ptr += Tasks[TaskID]->reward.length() + 1;

	// second string is parsed for item links
	sprintf(Ptr, "%s", Tasks[TaskID]->item_link.c_str());
	Ptr += Tasks[TaskID]->item_link.length() + 1;

	tdt = (TaskDescriptionTrailer_Struct *) Ptr;
	tdt->Points               = 0x00000000; // Points Count TODO: this does have a visible affect on the client ...
	tdt->has_reward_selection = 0; // TODO: new rewards window

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

bool ClientTaskState::IsTaskActivityCompleted(TaskType type, int index, int ActivityID)
{
	switch (type) {
		case TaskType::Task:
			if (index != 0) {
				return false;
			}
			return active_task.activity[ActivityID].activity_state == ActivityCompleted;
		case TaskType::Shared:
			return false; // TODO: shared tasks
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return active_quests[index].activity[ActivityID].activity_state == ActivityCompleted;
			}
		default:
			return false;
	}

}

// should we be defaulting to hidden?
ActivityState ClientTaskState::GetTaskActivityState(TaskType type, int index, int ActivityID)
{
	switch (type) {
		case TaskType::Task:
			if (index != 0) {
				return ActivityHidden;
			}
			return active_task.activity[ActivityID].activity_state;
		case TaskType::Shared:
			return ActivityHidden; // TODO: shared tasks
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return active_quests[index].activity[ActivityID].activity_state;
			}
		default:
			return ActivityHidden;
	}
}

int ClientTaskState::GetTaskActivityDoneCount(TaskType type, int index, int ActivityID)
{
	switch (type) {
		case TaskType::Task:
			if (index != 0) {
				return 0;
			}
			return active_task.activity[ActivityID].done_count;
		case TaskType::Shared:
			return 0; // TODO: shared tasks
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS) {
				return active_quests[index].activity[ActivityID].done_count;
			}
		default:
			return 0;
	}
}

int ClientTaskState::GetTaskActivityDoneCountFromTaskID(int TaskID, int ActivityID)
{
	if (active_task.task_id == TaskID) {
		return active_task.activity[ActivityID].done_count;
	}

	// TODO: shared tasks

	int      ActiveTaskIndex = -1;
	for (int i               = 0; i < MAXACTIVEQUESTS; i++) {
		if (active_quests[i].task_id == TaskID) {
			ActiveTaskIndex = i;
			break;
		}
	}

	if (ActiveTaskIndex == -1) {
		return 0;
	}

	if (active_quests[ActiveTaskIndex].activity[ActivityID].done_count) {
		return active_quests[ActiveTaskIndex].activity[ActivityID].done_count;
	}
	else {
		return 0;
	}
}

int ClientTaskState::GetTaskStartTime(TaskType type, int index)
{
	switch (type) {
		case TaskType::Task:
			return active_task.accepted_time;
		case TaskType::Quest:
			return active_quests[index].accepted_time;
		case TaskType::Shared: // TODO
		default:
			return -1;
	}
}

void ClientTaskState::CancelAllTasks(Client *c)
{

	// This method exists solely to be called during #task reloadall
	// It removes tasks from the in-game client state ready for them to be
	// resent to the client, in case an updated task fails to load

	CancelTask(c, 0, TaskType::Task, false);
	active_task.task_id = TASKSLOTEMPTY;

	for (int i = 0; i < MAXACTIVEQUESTS; i++)
		if (active_quests[i].task_id != TASKSLOTEMPTY) {
			CancelTask(c, i, TaskType::Quest, false);
			active_quests[i].task_id = TASKSLOTEMPTY;
		}

	// TODO: shared
}

void ClientTaskState::CancelTask(Client *c, int SequenceNumber, TaskType type, bool RemoveFromDB)
{
	auto outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));

	CancelTask_Struct *cts = (CancelTask_Struct *) outapp->pBuffer;
	cts->SequenceNumber = SequenceNumber;
	cts->type           = static_cast<uint32>(type);

	Log(Logs::General, Logs::Tasks, "[UPDATE] CancelTask");

	c->QueuePacket(outapp);
	safe_delete(outapp);

	if (RemoveFromDB) {
		RemoveTask(c, SequenceNumber, type);
	}
}

void ClientTaskState::RemoveTask(Client *c, int sequenceNumber, TaskType type)
{
	int characterID = c->CharacterID();
	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState Cancel Task %i ", sequenceNumber);

	int task_id = -1;
	switch (type) {
		case TaskType::Task:
			if (sequenceNumber == 0) {
				task_id = active_task.task_id;
			}
			break;
		case TaskType::Quest:
			if (sequenceNumber < MAXACTIVEQUESTS) {
				task_id = active_quests[sequenceNumber].task_id;
			}
			break;
		case TaskType::Shared: // TODO:
		default:
			break;
	}

	std::string query   = StringFormat(
		"DELETE FROM character_activities WHERE charid=%i AND taskid = %i",
		characterID, task_id
	);
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("[TASKS] Error in CientTaskState::CancelTask [{}]",
				 results.ErrorMessage().c_str());
		return;
	}
	Log(Logs::General, Logs::Tasks, "[UPDATE] CancelTask: %s", query.c_str());

	query   = StringFormat(
		"DELETE FROM character_tasks WHERE charid=%i AND taskid = %i AND type=%i", characterID,
		task_id, static_cast<int>(type));
	results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("[TASKS] Error in CientTaskState::CancelTask [{}]",
				 results.ErrorMessage().c_str());

	Log(Logs::General, Logs::Tasks, "[UPDATE] CancelTask: %s", query.c_str());

	switch (type) {
		case TaskType::Task:
			active_task.task_id = TASKSLOTEMPTY;
			break;
		case TaskType::Shared:
			break; // TODO: shared tasks
		case TaskType::Quest:
			active_quests[sequenceNumber].task_id = TASKSLOTEMPTY;
			active_task_count--;
			break;
		default:
			break;
	}
}

void ClientTaskState::RemoveTaskByTaskID(Client *c, uint32 task_id)
{
	auto task_type    = p_task_manager->GetTaskType(task_id);
	int  character_id = c->CharacterID();
	Log(Logs::General, Logs::Tasks, "[UPDATE] RemoveTaskByTaskID: %d", task_id);
	std::string query   = fmt::format(
		"DELETE FROM character_activities WHERE charid = {} AND taskid = {}",
		character_id,
		task_id
	);
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("[TASKS] Error in CientTaskState::RemoveTaskByTaskID [{}]", results.ErrorMessage().c_str());
		return;
	}
	LogTasks("[UPDATE] RemoveTaskByTaskID: {}", query.c_str());

	query   = fmt::format(
		"DELETE FROM character_tasks WHERE charid = {} AND taskid = {} AND type = {}",
		character_id,
		task_id,
		(int) task_type
	);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("[TASKS] Error in ClientTaskState::RemoveTaskByTaskID [{}]", results.ErrorMessage().c_str());
	}

	LogTasks("[UPDATE] RemoveTaskByTaskID: {}", query.c_str());

	switch (task_type) {
		case TaskType::Task: {
			if (active_task.task_id == task_id) {
				auto              outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));
				CancelTask_Struct *cts   = (CancelTask_Struct *) outapp->pBuffer;
				cts->SequenceNumber = 0;
				cts->type           = static_cast<uint32>(task_type);
				LogTasks("[UPDATE] RemoveTaskByTaskID found Task [{}]", task_id);
				c->QueuePacket(outapp);
				safe_delete(outapp);
				active_task.task_id = TASKSLOTEMPTY;
			}
			break;
		}
		case TaskType::Shared: {
			break; // TODO: shared tasks
		}
		case TaskType::Quest: {
			for (int active_quest = 0; active_quest < MAXACTIVEQUESTS; active_quest++) {
				if (active_quests[active_quest].task_id == task_id) {
					auto              outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));
					CancelTask_Struct *cts   = (CancelTask_Struct *) outapp->pBuffer;
					cts->SequenceNumber = active_quest;
					cts->type           = static_cast<uint32>(task_type);
					LogTasks("[UPDATE] RemoveTaskByTaskID found Quest [{}] at index [{}]", task_id, active_quest);
					active_quests[active_quest].task_id = TASKSLOTEMPTY;
					active_task_count--;
					c->QueuePacket(outapp);
					safe_delete(outapp);
				}
			}
		}
		default: {
			break;
		}
	}
}

void ClientTaskState::AcceptNewTask(Client *c, int TaskID, int NPCID, bool enforce_level_requirement)
{
	if (!p_task_manager || TaskID < 0 || TaskID >= MAXTASKS) {
		c->Message(Chat::Red, "Task system not functioning, or task_id %i out of range.", TaskID);
		return;
	}

	auto task = p_task_manager->Tasks[TaskID];

	if (task == nullptr) {
		c->Message(Chat::Red, "Invalid task_id %i", TaskID);
		return;
	}

	bool max_tasks = false;

	switch (task->type) {
		case TaskType::Task:
			if (active_task.task_id != TASKSLOTEMPTY) {
				max_tasks = true;
			}
			break;
		case TaskType::Shared: // TODO: shared tasks
			// if (something)
			max_tasks = true;
			break;
		case TaskType::Quest:
			if (active_task_count == MAXACTIVEQUESTS) {
				max_tasks = true;
			}
			break;
		default:
			break;
	}

	if (max_tasks) {
		c->Message(Chat::Red, "You already have the maximum allowable number of active tasks (%i)", MAXACTIVEQUESTS);
		return;
	}

	// only Quests can have more than one, so don't need to check others
	if (task->type == TaskType::Quest) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (active_quests[i].task_id == TaskID) {
				c->Message(Chat::Red, "You have already been assigned this task.");
				return;
			}
		}
	}

	if (enforce_level_requirement && !p_task_manager->ValidateLevel(TaskID, c->GetLevel())) {
		c->Message(Chat::Red, "You are outside the level range of this task.");
		return;
	}

	if (!p_task_manager->IsTaskRepeatable(TaskID) && IsTaskCompleted(TaskID)) {
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
			active_slot = &active_task;
			break;
		case TaskType::Shared: // TODO: shared
			active_slot = nullptr;
			break;
		case TaskType::Quest:
			for (int i  = 0; i < MAXACTIVEQUESTS; i++) {
				Log(Logs::General, Logs::Tasks,
					"[UPDATE] ClientTaskState Looking for free slot in slot %i, found task_id of %i", i,
					active_quests[i].task_id);
				if (active_quests[i].task_id == 0) {
					active_slot = &active_quests[i];
					break;
				}
			}
			break;
		default:
			break;
	}

	// This shouldn't happen unless there is a bug in the handling of ActiveTaskCount somewhere
	if (active_slot == nullptr) {
		c->Message(Chat::Red, "You already have the maximum allowable number of active tasks (%i)", MAXACTIVEQUESTS);
		return;
	}

	active_slot->task_id       = TaskID;
	active_slot->accepted_time = time(nullptr);
	active_slot->updated       = true;
	active_slot->current_step  = -1;

	for (int i = 0; i < p_task_manager->Tasks[TaskID]->activity_count; i++) {
		active_slot->activity[i].activity_id    = i;
		active_slot->activity[i].done_count     = 0;
		active_slot->activity[i].activity_state = ActivityHidden;
		active_slot->activity[i].updated        = true;
	}

	UnlockActivities(c->CharacterID(), *active_slot);

	if (task->type == TaskType::Quest) {
		active_task_count++;
	}

	p_task_manager->SendSingleActiveTaskToClient(c, *active_slot, false, true);
	c->Message(Chat::White, "You have been assigned the task '%s'.", p_task_manager->Tasks[TaskID]->title.c_str());
	p_task_manager->SaveClientState(c, this);
	std::string buf = std::to_string(TaskID);

	NPC *npc = entity_list.GetID(NPCID)->CastToNPC();
	if (npc) {
		parse->EventNPC(EVENT_TASK_ACCEPTED, npc, c, buf.c_str(), 0);
	}
}

void ClientTaskState::ProcessTaskProximities(Client *c, float X, float Y, float Z)
{

	float LastX = c->ProximityX();
	float LastY = c->ProximityY();
	float LastZ = c->ProximityZ();

	if ((LastX == X) && (LastY == Y) && (LastZ == Z)) { return; }

	Log(Logs::General, Logs::Tasks, "[PROXIMITY] Checking proximities for Position %8.3f, %8.3f, %8.3f", X, Y, Z);
	int ExploreID = p_task_manager->ProximityManager.CheckProximities(X, Y, Z);

	if (ExploreID > 0) {
		Log(Logs::General,
			Logs::Tasks,
			"[PROXIMITY] Position %8.3f, %8.3f, %8.3f is within proximity %i",
			X,
			Y,
			Z,
			ExploreID);
		UpdateTasksOnExplore(c, ExploreID);
	}
}

TaskGoalListManager::TaskGoalListManager()
{
	NumberOfLists = 0;
}

TaskGoalListManager::~TaskGoalListManager() {}

bool TaskGoalListManager::LoadLists()
{

	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] TaskGoalListManager::LoadLists Called");

	TaskGoalLists.clear();

	const char *ERR_MYSQLERROR = "Error in TaskGoalListManager::LoadLists: %s %s";

	NumberOfLists = 0;

	std::string query   = "SELECT `listid`, COUNT(`entry`) "
						  "FROM `goallists` GROUP by `listid` "
						  "ORDER BY `listid`";
	auto        results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	NumberOfLists = results.RowCount();
	LogTasks("Loading GoalLists [{}] lists", NumberOfLists);

	TaskGoalLists.reserve(NumberOfLists);

	int       list_index = 0;
	for (auto row        = results.begin(); row != results.end(); ++row) {
		int listID   = atoi(row[0]);
		int listSize = atoi(row[1]);

		TaskGoalLists.push_back({listID, 0, 0});

		TaskGoalLists[list_index].GoalItemEntries.reserve(listSize);

		list_index++;
	}

	auto goal_lists = GoallistsRepository::GetWhere(content_db, "TRUE ORDER BY listid, entry ASC");
	for (list_index = 0; list_index < NumberOfLists; list_index++) {

		int list_id = TaskGoalLists[list_index].ListID;

		for (auto &entry: goal_lists) {
			if (entry.listid == list_id) {
				if (entry.entry < TaskGoalLists[list_index].Min) {
					TaskGoalLists[list_index].Min = entry.entry;
				}

				if (entry.entry > TaskGoalLists[list_index].Max) {
					TaskGoalLists[list_index].Max = entry.entry;
				}

				TaskGoalLists[list_index].GoalItemEntries.push_back(entry.entry);

				LogTasksDetail(
					"Goal list index [{}] loading list [{}] entry [{}]",
					list_index,
					list_id,
					entry.entry
				);
			}
		}
	}

	return true;

}

int TaskGoalListManager::GetListByID(int ListID)
{

	// Find the list with the specified ListID and return the index
	auto it = std::find_if(
		TaskGoalLists.begin(),
		TaskGoalLists.end(),
		[ListID](const TaskGoalList_Struct &t) { return t.ListID == ListID; }
	);

	if (it == TaskGoalLists.end()) {
		return -1;
	}

	return std::distance(TaskGoalLists.begin(), it);
}

int TaskGoalListManager::GetFirstEntry(int ListID)
{
	int ListIndex = GetListByID(ListID);

	if ((ListIndex < 0) || (ListIndex >= NumberOfLists)) { return -1; }

	if (TaskGoalLists[ListIndex].GoalItemEntries.empty()) { return -1; }

	return TaskGoalLists[ListIndex].GoalItemEntries[0];
}

std::vector<int> TaskGoalListManager::GetListContents(int ListID)
{
	std::vector<int> ListContents;
	int              ListIndex = GetListByID(ListID);

	if ((ListIndex < 0) || (ListIndex >= NumberOfLists)) { return ListContents; }

	ListContents = TaskGoalLists[ListIndex].GoalItemEntries;

	return ListContents;
}

bool TaskGoalListManager::IsInList(int ListID, int Entry)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i)", ListID, Entry);

	int ListIndex = GetListByID(ListID);

	if ((ListIndex < 0) || (ListIndex >= NumberOfLists)) {
		return false;
	}

	if ((Entry < TaskGoalLists[ListIndex].Min) || (Entry > TaskGoalLists[ListIndex].Max)) {
		return false;
	}

	int  FirstEntry = 0;
	auto &task      = TaskGoalLists[ListIndex];

	auto it = std::find(task.GoalItemEntries.begin(), task.GoalItemEntries.end(), Entry);

	if (it == task.GoalItemEntries.end()) {
		return false;
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i) returning true", ListIndex,
		Entry);
	return true;
}

TaskProximityManager::TaskProximityManager()
{


}

TaskProximityManager::~TaskProximityManager()
{


}

bool TaskProximityManager::LoadProximities(int zoneID)
{
	TaskProximity proximity;

	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] TaskProximityManager::LoadProximities Called for zone %i", zoneID);
	TaskProximities.clear();

	std::string query   = StringFormat(
		"SELECT `exploreid`, `minx`, `maxx`, "
		"`miny`, `maxy`, `minz`, `maxz` "
		"FROM `proximities` WHERE `zoneid` = %i "
		"ORDER BY `zoneid` ASC", zoneID
	);
	auto        results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		proximity.ExploreID = atoi(row[0]);
		proximity.MinX      = atof(row[1]);
		proximity.MaxX      = atof(row[2]);
		proximity.MinY      = atof(row[3]);
		proximity.MaxY      = atof(row[4]);
		proximity.MinZ      = atof(row[5]);
		proximity.MaxZ      = atof(row[6]);

		TaskProximities.push_back(proximity);
	}

	return true;

}

int TaskProximityManager::CheckProximities(float X, float Y, float Z)
{

	for (unsigned int i = 0; i < TaskProximities.size(); i++) {

		TaskProximity *P = &TaskProximities[i];

		Log(Logs::General,
			Logs::Tasks,
			"[PROXIMITY] Checking %8.3f, %8.3f, %8.3f against %8.3f, %8.3f, %8.3f, %8.3f, %8.3f, %8.3f",
			X,
			Y,
			Z,
			P->MinX,
			P->MaxX,
			P->MinY,
			P->MaxY,
			P->MinZ,
			P->MaxZ);

		if (X < P->MinX || X > P->MaxX || Y < P->MinY || Y > P->MaxY ||
			Z < P->MinZ || Z > P->MaxZ) {
			continue;
		}

		return P->ExploreID;

	}

	return 0;
}

