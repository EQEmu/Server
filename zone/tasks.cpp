
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

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "string_ids.h"

#include "queryserv.h"
#include "quest_parser_collection.h"

extern QueryServ* QServ;

TaskManager::TaskManager() {
	for(int i=0; i<MAXTASKS; i++)
		Tasks[i] = nullptr;
}

TaskManager::~TaskManager() {
	for(int i=0; i<MAXTASKS; i++) {
		if(Tasks[i] != nullptr) {
			safe_delete(Tasks[i]);
		}
	}
}

bool TaskManager::LoadTaskSets()
{

	// Clear all task sets in memory. Done so we can reload them on the fly if required by just calling
	// this method again.
	for (int i = 0; i < MAXTASKSETS; i++)
		TaskSets[i].clear();

	std::string query   = StringFormat(
		"SELECT `id`, `taskid` from `tasksets` "
		"WHERE `id` > 0 AND `id` < %i "
		"AND `taskid` >= 0 AND `taskid` < %i "
		"ORDER BY `id`, `taskid` ASC",
		MAXTASKSETS, MAXTASKS
	);
	auto        results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error in TaskManager::LoadTaskSets: [{}]", results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int taskSet = atoi(row[0]);
		int taskID  = atoi(row[1]);

		TaskSets[taskSet].push_back(taskID);
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Adding TaskID %4i to TaskSet %4i", taskID, taskSet);
	}

	return true;
}

bool TaskManager::LoadSingleTask(int TaskID) {

	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] TaskManager::LoadSingleTask(%i)", TaskID);

	if((TaskID <= 0) || (TaskID >= MAXTASKS)) return false;

	// If this task already exists in memory, free all the dynamically allocated strings.
	if(Tasks[TaskID]) {
		safe_delete(Tasks[TaskID]);
	}

	return LoadTasks(TaskID);
}

void TaskManager::ReloadGoalLists() {

	if(!GoalListManager.LoadLists())
		Log(Logs::Detail, Logs::Tasks,"TaskManager::LoadTasks LoadLists failed");
}

bool TaskManager::LoadTasks(int singleTask)
{
	// If TaskID !=0, then just load the task specified.
	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] TaskManager::LoadTasks Called");

	std::string query;
	if (singleTask == 0) {
		if (!GoalListManager.LoadLists())
			Log(Logs::Detail, Logs::Tasks, "TaskManager::LoadTasks LoadLists failed");

		if (!LoadTaskSets())
			Log(Logs::Detail, Logs::Tasks, "TaskManager::LoadTasks LoadTaskSets failed");

		query = StringFormat("SELECT `id`, `type`, `duration`, `duration_code`, `title`, `description`, "
				     "`reward`, `rewardid`, `cashreward`, `xpreward`, `rewardmethod`, `faction_reward`,"
				     "`minlevel`, `maxlevel`, `repeatable`, `completion_emote` FROM `tasks` WHERE `id` < %i",
				     MAXTASKS);
	} else
		query = StringFormat("SELECT `id`, `type`, `duration`, `duration_code`, `title`, `description`, "
				     "`reward`, `rewardid`, `cashreward`, `xpreward`, `rewardmethod`, `faction_reward`,"
				     "`minlevel`, `maxlevel`, `repeatable`, `completion_emote` FROM `tasks` WHERE `id` = %i",
				     singleTask);

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::LoadTasks: %s";

	auto results = database.QueryDatabase(query);
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
		Tasks[taskID]->type = static_cast<TaskType>(atoi(row[1]));
		Tasks[taskID]->Duration = atoi(row[2]);
		Tasks[taskID]->dur_code = static_cast<DurationCode>(atoi(row[3]));
		Tasks[taskID]->Title = row[4];
		Tasks[taskID]->Description = row[5];
		Tasks[taskID]->Reward = row[6];
		Tasks[taskID]->RewardID = atoi(row[7]);
		Tasks[taskID]->CashReward = atoi(row[8]);
		Tasks[taskID]->XPReward = atoi(row[9]);
		Tasks[taskID]->RewardMethod = (TaskMethodType)atoi(row[10]);
		Tasks[taskID]->faction_reward = atoi(row[11]);
		Tasks[taskID]->MinLevel = atoi(row[12]);
		Tasks[taskID]->MaxLevel = atoi(row[13]);
		Tasks[taskID]->Repeatable = atoi(row[14]);
		Tasks[taskID]->completion_emote = row[15];
		Tasks[taskID]->ActivityCount = 0;
		Tasks[taskID]->SequenceMode = ActivitiesSequential;
		Tasks[taskID]->LastStep = 0;

		Log(Logs::General, Logs::Tasks,
		    "[GLOBALLOAD] TaskID: %5i, Duration: %8i, Reward: %s MinLevel %i MaxLevel %i "
		    "Repeatable: %s",
		    taskID, Tasks[taskID]->Duration, Tasks[taskID]->Reward.c_str(),
		    Tasks[taskID]->MinLevel, Tasks[taskID]->MaxLevel, Tasks[taskID]->Repeatable ? "Yes" : "No");
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Title: %s", Tasks[taskID]->Title.c_str());
	}

	if (singleTask == 0)
		query =
		    StringFormat("SELECT `taskid`, `step`, `activityid`, `activitytype`, `target_name`, `item_list`, "
				 "`skill_list`, `spell_list`, `description_override`, `goalid`, `goalmethod`, "
				 "`goalcount`, `delivertonpc`, `zones`, `optional` FROM `task_activities` WHERE `taskid` < "
				 "%i AND `activityid` < %i ORDER BY taskid, activityid ASC",
				 MAXTASKS, MAXACTIVITIESPERTASK);
	else
		query =
		    StringFormat("SELECT `taskid`, `step`, `activityid`, `activitytype`, `target_name`, `item_list`, "
				 "`skill_list`, `spell_list`, `description_override`, `goalid`, `goalmethod`, "
				 "`goalcount`, `delivertonpc`, `zones`, `optional` FROM `task_activities` WHERE `taskid` = "
				 "%i AND `activityid` < %i ORDER BY taskid, activityid ASC",
				 singleTask, MAXACTIVITIESPERTASK);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int taskID = atoi(row[0]);
		int step = atoi(row[1]);

		int activityID = atoi(row[2]);

		if ((taskID <= 0) || (taskID >= MAXTASKS) || (activityID < 0) || (activityID >= MAXACTIVITIESPERTASK)) {
			// This shouldn't happen, as the SELECT is bounded by MAXTASKS
			LogError("[TASKS]Task or Activity ID ([{}], [{}]) out of range while loading activities from database", taskID, activityID);
			continue;
		}

		if (Tasks[taskID] == nullptr) {
			LogError("[TASKS]Activity for non-existent task ([{}], [{}]) while loading activities from database",
			    taskID, activityID);
			continue;
		}

		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].StepNumber = step;

		if (step != 0)
			Tasks[taskID]->SequenceMode = ActivitiesStepped;

		if (step > Tasks[taskID]->LastStep)
			Tasks[taskID]->LastStep = step;

		// Task Activities MUST be numbered sequentially from 0. If not, log an error
		// and set the task to nullptr. Subsequent activities for this task will raise
		// ERR_NOTASK errors.
		// Change to (activityID != (Tasks[taskID]->ActivityCount + 1)) to index from 1
		if (activityID != Tasks[taskID]->ActivityCount) {
			LogError("[TASKS]Activities for Task [{}] are not sequential starting at 0. Not loading task", taskID,
			    activityID);
			Tasks[taskID] = nullptr;
			continue;
		}

		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].Type = atoi(row[3]);

		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].target_name = row[4];
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].item_list = row[5];
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].skill_list = row[6];
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].skill_id = atoi(row[6]); // for older clients
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].spell_list = row[7];
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].spell_id = atoi(row[7]); // for older clients
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].desc_override = row[8];

		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].GoalID = atoi(row[9]);
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].GoalMethod = (TaskMethodType)atoi(row[10]);
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].GoalCount = atoi(row[11]);
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].DeliverToNPC = atoi(row[12]);
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].zones = row[13];
		auto zones = SplitString(Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].zones, ';');
		for (auto && e : zones)
			Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].ZoneIDs.push_back(std::stoi(e));
		Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].Optional = atoi(row[14]);

		Log(Logs::General, Logs::Tasks,
		    "[GLOBALLOAD] Activity Slot %2i: ID %i for Task %5i. Type: %3i, GoalID: %8i, "
		    "GoalMethod: %i, GoalCount: %3i, Zones:%s",
		    Tasks[taskID]->ActivityCount, activityID, taskID,
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].Type,
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].GoalID,
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].GoalMethod,
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].GoalCount,
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].zones.c_str());

		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] target_name: %s",
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].target_name.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] item_list: %s",
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].item_list.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] skill_list: %s",
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].skill_list.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] spell_list: %s",
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].spell_list.c_str());
		Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] description_override: %s",
		    Tasks[taskID]->Activity[Tasks[taskID]->ActivityCount].desc_override.c_str());

		Tasks[taskID]->ActivityCount++;
	}

	return true;
}

bool TaskManager::SaveClientState(Client *c, ClientTaskState *state)
{
	// I am saving the slot in the ActiveTasks table, because unless a Task is cancelled/completed, the client
	// doesn't seem to like tasks moving slots between zoning and you can end up with 'bogus' activities if the task
	// previously in that slot had more activities than the one now occupying it. Hopefully retaining the slot
	// number for the duration of a session will overcome this.
	if (!c || !state)
		return false;

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::SaveClientState %s";

	int characterID = c->CharacterID();

	Log(Logs::Detail, Logs::Tasks, "TaskManager::SaveClientState for character ID %d", characterID);

	if (state->ActiveTaskCount > 0 || state->ActiveTask.TaskID != TASKSLOTEMPTY) { // TODO: tasks
		for (int task = 0; task < MAXACTIVEQUESTS + 1; task++) {
			int taskID = state->ActiveTasks[task].TaskID;
			if (taskID == TASKSLOTEMPTY)
				continue;

			int slot = state->ActiveTasks[task].slot;

			if (state->ActiveTasks[task].Updated) {

				Log(Logs::General, Logs::Tasks,
				    "[CLIENTSAVE] TaskManager::SaveClientState for character ID %d, Updating TaskIndex "
				    "%i TaskID %i",
				    characterID, slot, taskID);

				std::string query = StringFormat(
				    "REPLACE INTO character_tasks (charid, taskid, slot, type, acceptedtime) "
				    "VALUES (%i, %i, %i, %i, %i)",
				    characterID, taskID, slot, static_cast<int>(Tasks[taskID]->type),
				    state->ActiveTasks[task].AcceptedTime);
				auto results = database.QueryDatabase(query);
				if (!results.Success()) {
					LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
				} else {
					state->ActiveTasks[task].Updated = false;
				}
			}

			std::string query =
			    "REPLACE INTO character_activities (charid, taskid, activityid, donecount, completed) "
			    "VALUES ";

			int updatedActivityCount = 0;
			for (int activityIndex = 0; activityIndex < Tasks[taskID]->ActivityCount; ++activityIndex) {

				if (!state->ActiveTasks[task].Activity[activityIndex].Updated)
					continue;

				Log(Logs::General, Logs::Tasks,
				    "[CLIENTSAVE] TaskManager::SaveClientSate for character ID %d, Updating Activity "
				    "%i, %i",
				    characterID, slot, activityIndex);

				if (updatedActivityCount == 0)
					query +=
					    StringFormat("(%i, %i, %i, %i, %i)", characterID, taskID, activityIndex,
							 state->ActiveTasks[task].Activity[activityIndex].DoneCount,
							 state->ActiveTasks[task].Activity[activityIndex].State ==
							     ActivityCompleted);
				else
					query +=
					    StringFormat(", (%i, %i, %i, %i, %i)", characterID, taskID, activityIndex,
							 state->ActiveTasks[task].Activity[activityIndex].DoneCount,
							 state->ActiveTasks[task].Activity[activityIndex].State ==
							     ActivityCompleted);

				updatedActivityCount++;
			}

			if (updatedActivityCount == 0)
				continue;

			Log(Logs::General, Logs::Tasks, "[CLIENTSAVE] Executing query %s", query.c_str());
			auto results = database.QueryDatabase(query);

			if (!results.Success()) {
				LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
				continue;
			}

			state->ActiveTasks[task].Updated = false;
			for (int activityIndex = 0; activityIndex < Tasks[taskID]->ActivityCount; ++activityIndex)
				state->ActiveTasks[task].Activity[activityIndex].Updated = false;
		}
	}

	if (!RuleB(TaskSystem, RecordCompletedTasks) ||
	    (state->CompletedTasks.size() <= (unsigned int)state->LastCompletedTaskLoaded)) {
		state->LastCompletedTaskLoaded = state->CompletedTasks.size();
		return true;
	}

	const char *completedTaskQuery = "REPLACE INTO completed_tasks (charid, completedtime, taskid, activityid) "
					 "VALUES (%i, %i, %i, %i)";

	for (unsigned int i = state->LastCompletedTaskLoaded; i < state->CompletedTasks.size(); i++) {

		Log(Logs::General, Logs::Tasks,
		    "[CLIENTSAVE] TaskManager::SaveClientState Saving Completed Task at slot %i", i);
		int taskID = state->CompletedTasks[i].TaskID;

		if ((taskID <= 0) || (taskID >= MAXTASKS) || (Tasks[taskID] == nullptr))
			continue;

		// First we save a record with an ActivityID of -1.
		// This indicates this task was completed at the given time. We infer that all
		// none optional activities were completed.
		//
		std::string query =
		    StringFormat(completedTaskQuery, characterID, state->CompletedTasks[i].CompletedTime, taskID, -1);
		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
			continue;
		}

		// If the Rule to record non-optional task completion is not enabled, don't save it
		if (!RuleB(TaskSystem, RecordCompletedOptionalActivities))
			continue;

		// Insert one record for each completed optional task.

		for (int j = 0; j < Tasks[taskID]->ActivityCount; j++) {
			if (!Tasks[taskID]->Activity[j].Optional || !state->CompletedTasks[i].ActivityDone[j])
				continue;

			query = StringFormat(completedTaskQuery, characterID, state->CompletedTasks[i].CompletedTime,
					     taskID, j);
			results = database.QueryDatabase(query);
			if (!results.Success())
				LogError(ERR_MYSQLERROR, results.ErrorMessage().c_str());
		}
	}

	state->LastCompletedTaskLoaded = state->CompletedTasks.size();
	return true;
}

void Client::LoadClientTaskState() {

	if(RuleB(TaskSystem, EnableTaskSystem) && taskmanager) {
		if(taskstate)
			safe_delete(taskstate);

		taskstate = new ClientTaskState;
		if(!taskmanager->LoadClientState(this, taskstate)) {
			safe_delete(taskstate);
		}
		else {
			taskmanager->SendActiveTasksToClient(this);
			taskmanager->SendCompletedTasksToClient(this, taskstate);
		}
	}

}

void Client::RemoveClientTaskState() {

	if(taskstate) {
		taskstate->CancelAllTasks(this);
		safe_delete(taskstate);
	}
}

bool TaskManager::LoadClientState(Client *c, ClientTaskState *state)
{
	if (!c || !state)
		return false;

	int characterID = c->CharacterID();

	state->ActiveTaskCount = 0;

	Log(Logs::General, Logs::Tasks, "[CLIENTLOAD] TaskManager::LoadClientState for character ID %d", characterID);

	std::string query = StringFormat("SELECT `taskid`, `slot`,`type`, `acceptedtime` "
					 "FROM `character_tasks` "
					 "WHERE `charid` = %i ORDER BY acceptedtime",
					 characterID);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("[TASKS]Error in TaskManager::LoadClientState load Tasks: [{}]",
		    results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int taskID = atoi(row[0]);
		int slot = atoi(row[1]);
		TaskType type = static_cast<TaskType>(atoi(row[2]));

		if ((taskID < 0) || (taskID >= MAXTASKS)) {
			LogError("[TASKS]Task ID [{}] out of range while loading character tasks from database", taskID);
			continue;
		}

		auto task_info = state->GetClientTaskInfo(type, slot);

		if (task_info == nullptr) {
			LogError("[TASKS] Slot [{}] out of range while loading character tasks from database", slot);
			continue;
		}

		if (task_info->TaskID != TASKSLOTEMPTY) {
			LogError("[TASKS] Slot [{}] for Task [{}]s is already occupied", slot,
			    taskID);
			continue;
		}

		int acceptedtime = atoi(row[3]);

		task_info->TaskID = taskID;
		task_info->CurrentStep = -1;
		task_info->AcceptedTime = acceptedtime;
		task_info->Updated = false;

		for (int i = 0; i < MAXACTIVITIESPERTASK; i++)
			task_info->Activity[i].ActivityID = -1;

		if (type == TaskType::Quest)
			++state->ActiveTaskCount;

		Log(Logs::General, Logs::Tasks,
		    "[CLIENTLOAD] TaskManager::LoadClientState. Char: %i Task ID %i, Accepted Time: %8X", characterID,
		    taskID, acceptedtime);
	}

	// Load Activities
	Log(Logs::General, Logs::Tasks, "[CLIENTLOAD] LoadClientState. Loading activities for character ID %d",
	    characterID);

	query = StringFormat("SELECT `taskid`, `activityid`, `donecount`, `completed` "
			     "FROM `character_activities` "
			     "WHERE `charid` = %i "
			     "ORDER BY `taskid` ASC, `activityid` ASC",
			     characterID);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("[TASKS]Error in TaskManager::LoadClientState load Activities: [{}]",
		    results.ErrorMessage().c_str());
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		int taskID = atoi(row[0]);
		if ((taskID < 0) || (taskID >= MAXTASKS)) {
			LogError("[TASKS]Task ID [{}] out of range while loading character activities from database", taskID);
			continue;
		}

		int activityID = atoi(row[1]);
		if ((activityID < 0) || (activityID >= MAXACTIVITIESPERTASK)) {
			LogError("[TASKS]Activity ID [{}] out of range while loading character activities from database",
			    activityID);
			continue;
		}

		ClientTaskInformation *task_info = nullptr;

		if (state->ActiveTask.TaskID == taskID)
			task_info = &state->ActiveTask;

		// wasn't task
		if (task_info == nullptr)
			for (int i = 0; i < MAXACTIVEQUESTS; i++)
				if (state->ActiveQuests[i].TaskID == taskID)
					task_info = &state->ActiveQuests[i];

		if (task_info == nullptr) {
			LogError("[TASKS]Activity [{}] found for task [{}] which client does not have", activityID, taskID);
			continue;
		}

		int doneCount = atoi(row[2]);
		bool completed = atoi(row[3]);
		task_info->Activity[activityID].ActivityID = activityID;
		task_info->Activity[activityID].DoneCount = doneCount;
		if (completed)
			task_info->Activity[activityID].State = ActivityCompleted;
		else
			task_info->Activity[activityID].State = ActivityHidden;

		task_info->Activity[activityID].Updated = false;

		Log(Logs::General, Logs::Tasks,
		    "[CLIENTLOAD] TaskManager::LoadClientState. Char: %i Task ID %i, ActivityID: %i, DoneCount: %i, "
		    "Completed: %i",
		    characterID, taskID, activityID, doneCount, completed);
	}

	if (RuleB(TaskSystem, RecordCompletedTasks)) {
		query = StringFormat("SELECT `taskid`, `activityid`, `completedtime` "
				     "FROM `completed_tasks` "
				     "WHERE `charid` = %i ORDER BY completedtime, taskid, activityid",
				     characterID);
		results = database.QueryDatabase(query);
		if (!results.Success()) {
			LogError("[TASKS]Error in TaskManager::LoadClientState load completed tasks: [{}]",
			    results.ErrorMessage().c_str());
			return false;
		}

		CompletedTaskInformation cti;

		for (int i = 0; i < MAXACTIVITIESPERTASK; i++)
			cti.ActivityDone[i] = false;

		int previousTaskID = -1;
		int previousCompletedTime = -1;

		for (auto row = results.begin(); row != results.end(); ++row) {

			int taskID = atoi(row[0]);
			if ((taskID <= 0) || (taskID >= MAXTASKS)) {
				LogError("[TASKS]Task ID [{}] out of range while loading completed tasks from database",
				    taskID);
				continue;
			}

			// An ActivityID of -1 means mark all the none optional activities in the
			// task as complete. If the Rule to record optional activities is enabled,
			// subsequent records for this task will flag any optional tasks that were
			// completed.
			int activityID = atoi(row[1]);
			if ((activityID < -1) || (activityID >= MAXACTIVITIESPERTASK)) {
				LogError("[TASKS]Activity ID [{}] out of range while loading completed tasks from database",
				    activityID);
				continue;
			}

			int completedTime = atoi(row[2]);
			if ((previousTaskID != -1) &&
			    ((taskID != previousTaskID) || (completedTime != previousCompletedTime))) {
				state->CompletedTasks.push_back(cti);
				for (int i = 0; i < MAXACTIVITIESPERTASK; i++)
					cti.ActivityDone[i] = false;
			}

			cti.TaskID = previousTaskID = taskID;
			cti.CompletedTime = previousCompletedTime = completedTime;

			// If ActivityID is -1, Mark all the non-optional tasks as completed.
			if (activityID < 0) {
				TaskInformation *task = Tasks[taskID];
				if (task == nullptr)
					continue;

				for (int i = 0; i < task->ActivityCount; i++)
					if (!task->Activity[i].Optional)
						cti.ActivityDone[i] = true;
			} else
				cti.ActivityDone[activityID] = true;
		}

		if (previousTaskID != -1)
			state->CompletedTasks.push_back(cti);

		state->LastCompletedTaskLoaded = state->CompletedTasks.size();
	}

	query = StringFormat("SELECT `taskid` FROM character_enabledtasks "
			     "WHERE `charid` = %i AND `taskid` >0 AND `taskid` < %i "
			     "ORDER BY `taskid` ASC",
			     characterID, MAXTASKS);
	results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("[TASKS]Error in TaskManager::LoadClientState load enabled tasks: [{}]",
		    results.ErrorMessage().c_str());
	} else {
		for (auto row = results.begin(); row != results.end(); ++row) {
			int taskID = atoi(row[0]);
			state->EnabledTasks.push_back(taskID);
			Log(Logs::General, Logs::Tasks, "[CLIENTLOAD] Adding TaskID %i to enabled tasks", taskID);
		}
	}

	// Check that there is an entry in the client task state for every activity in each task
	// This should only break if a ServerOP adds or deletes activites for a task that players already
	// have active, or due to a bug.
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		int taskID = state->ActiveTasks[i].TaskID;
		if (taskID == TASKSLOTEMPTY)
			continue;
		if (!Tasks[taskID]) {
			c->Message(Chat::Red,
				   "Active Task Slot %i, references a task (%i), that does not exist. "
				   "Removing from memory. Contact a GM to resolve this.",
				   i, taskID);

			LogError("[TASKS]Character [{}] has task [{}] which does not exist",
			    characterID, taskID);
			state->ActiveTasks[i].TaskID = TASKSLOTEMPTY;
			continue;
		}
		for (int j = 0; j < Tasks[taskID]->ActivityCount; j++) {

			if (state->ActiveTasks[i].Activity[j].ActivityID != j) {
				c->Message(Chat::Red,
					   "Active Task %i, %s. Activity count does not match expected value."
					   "Removing from memory. Contact a GM to resolve this.",
					   taskID, Tasks[taskID]->Title.c_str());

				LogError("[TASKS]Fatal error in character [{}] task state. Activity [{}] for Task [{}] either missing from client state or from task",
				    characterID, j, taskID);
				state->ActiveTasks[i].TaskID = TASKSLOTEMPTY;
				break;
			}
		}
	}

	if (state->ActiveTask.TaskID != TASKSLOTEMPTY)
		state->UnlockActivities(characterID, state->ActiveTask);
	// TODO: shared
	for (int i = 0; i < MAXACTIVEQUESTS; i++)
		if (state->ActiveQuests[i].TaskID != TASKSLOTEMPTY)
			state->UnlockActivities(characterID, state->ActiveQuests[i]);

	Log(Logs::General, Logs::Tasks, "[CLIENTLOAD] LoadClientState for Character ID %d DONE!", characterID);
	return true;
}

void ClientTaskState::EnableTask(int characterID, int taskCount, int *tasks) {

	// Check if the Task is already enabled for this client
	//
	std::vector<int> tasksEnabled;

	for(int i=0; i<taskCount; i++) {
		auto iterator = EnabledTasks.begin();
		bool addTask = true;

		while(iterator != EnabledTasks.end()) {
			// If this task is already enabled, stop looking
			if((*iterator) == tasks[i]) {
				addTask = false;
				break;
			}
			// Our list of enabled tasks is sorted, so we can quit if we find a taskid higher than
			// the one we are looking for.
			if((*iterator) > tasks[i])
                break;
			++iterator;
		}

		if(addTask) {
			EnabledTasks.insert(iterator, tasks[i]);
			// Make a note of the task we enabled, for later SQL generation
			tasksEnabled.push_back(tasks[i]);
		}
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] New enabled task list ");
	for(unsigned int i=0; i<EnabledTasks.size(); i++)
		Log(Logs::General, Logs::Tasks, "[UPDATE] %i", EnabledTasks[i]);

	if(tasksEnabled.empty() )
        return;

	std::stringstream queryStream;
	queryStream << "REPLACE INTO character_enabledtasks (charid, taskid) VALUES ";
	for(unsigned int i=0; i<tasksEnabled.size(); i++)
		queryStream << (i ? ", " : "") <<  StringFormat("(%i, %i)", characterID, tasksEnabled[i]);

    std::string query = queryStream.str();

	if (tasksEnabled.size()) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] Executing query %s", query.c_str());
		database.QueryDatabase(query);
	}
	else {
		Log(Logs::General, Logs::Tasks, "[UPDATE] EnableTask called for characterID: %u .. but, no tasks exist", characterID);
	}
}

void ClientTaskState::DisableTask(int charID, int taskCount, int *taskList) {

	// Check if the Task is enabled for this client
	//
	std::vector<int> tasksDisabled;

	for(int i=0; i<taskCount; i++) {
		auto iterator = EnabledTasks.begin();
		bool removeTask = false;

		while(iterator != EnabledTasks.end()) {
			if((*iterator) == taskList[i]) {
				removeTask = true;
				break;
			}

			if((*iterator) > taskList[i])
                break;

			++iterator;
		}

		if(removeTask) {
			EnabledTasks.erase(iterator);
			tasksDisabled.push_back(taskList[i]);
		}
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] New enabled task list ");
	for(unsigned int i=0; i<EnabledTasks.size(); i++)
		Log(Logs::General, Logs::Tasks, "[UPDATE] %i", EnabledTasks[i]);

	if(tasksDisabled.empty())
        return;

	std::stringstream queryStream;
	queryStream << StringFormat("DELETE FROM character_enabledtasks WHERE charid = %i AND (", charID);

	for(unsigned int i=0; i<tasksDisabled.size(); i++)
        queryStream << (i ? StringFormat("taskid = %i ", tasksDisabled[i]) : StringFormat("OR taskid = %i ", tasksDisabled[i]));

	queryStream << ")";

	std::string query = queryStream.str();

	if (tasksDisabled.size()) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] Executing query %s", query.c_str());
		database.QueryDatabase(query);
	}
	else {
		Log(Logs::General, Logs::Tasks, "[UPDATE] DisableTask called for characterID: %u .. but, no tasks exist", charID);
	}
}

bool ClientTaskState::IsTaskEnabled(int TaskID) {

	std::vector<int>::iterator Iterator;

	Iterator = EnabledTasks.begin();

	while(Iterator != EnabledTasks.end()) {
		if((*Iterator) == TaskID) return true;
		if((*Iterator) > TaskID) break;
		++Iterator;
	}

	return false;
}

int ClientTaskState::EnabledTaskCount(int TaskSetID) {

	// Return the number of tasks in TaskSet that this character is enabled for.

	unsigned int EnabledTaskIndex = 0;
	unsigned int TaskSetIndex = 0;
	int EnabledTaskCount = 0;

	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return -1;

	while((EnabledTaskIndex < EnabledTasks.size()) && (TaskSetIndex < taskmanager->TaskSets[TaskSetID].size())) {

		if(EnabledTasks[EnabledTaskIndex] == taskmanager->TaskSets[TaskSetID][TaskSetIndex]) {

			EnabledTaskCount++;
			EnabledTaskIndex++;
			TaskSetIndex++;
			continue;
		}

		if(EnabledTasks[EnabledTaskIndex] < taskmanager->TaskSets[TaskSetID][TaskSetIndex])
			EnabledTaskIndex++;
		else
			TaskSetIndex++;

	}

	return EnabledTaskCount;
}
int ClientTaskState::ActiveTasksInSet(int TaskSetID) {


	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return -1;

	int Count = 0;

	for(unsigned int i=0; i<taskmanager->TaskSets[TaskSetID].size(); i++)
		if(IsTaskActive(taskmanager->TaskSets[TaskSetID][i]))
			Count++;

	return Count;
}

int ClientTaskState::CompletedTasksInSet(int TaskSetID) {


	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return -1;

	int Count = 0;

	for(unsigned int i=0; i<taskmanager->TaskSets[TaskSetID].size(); i++)
		if(IsTaskCompleted(taskmanager->TaskSets[TaskSetID][i]))
			Count++;

	return Count;
}

bool ClientTaskState::HasSlotForTask(TaskInformation *task)
{
	if (task == nullptr)
		return false;

	switch (task->type) {
	case TaskType::Task:
		return ActiveTask.TaskID == TASKSLOTEMPTY;
	case TaskType::Shared:
		return false; // todo
	case TaskType::Quest:
		for (int i = 0; i < MAXACTIVEQUESTS; ++i)
			if (ActiveQuests[i].TaskID == TASKSLOTEMPTY)
				return true;
	case TaskType::E:
		return false; // removed on live
	}

	return false;
}

int TaskManager::FirstTaskInSet(int TaskSetID) {

	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return 0;

	if(TaskSets[TaskSetID].empty()) return 0;

	auto Iterator = TaskSets[TaskSetID].begin();

	while(Iterator != TaskSets[TaskSetID].end()) {
		if((*Iterator) > 0)
			return (*Iterator);
		++Iterator;
	}

	return 0;
}

int TaskManager::LastTaskInSet(int TaskSetID) {

	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return 0;

	if(TaskSets[TaskSetID].empty()) return 0;

	return TaskSets[TaskSetID][TaskSets[TaskSetID].size()-1];
}

int TaskManager::NextTaskInSet(int TaskSetID, int TaskID) {

	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return 0;

	if(TaskSets[TaskSetID].empty()) return 0;

	for(unsigned int i=0; i<TaskSets[TaskSetID].size(); i++) {
		if(TaskSets[TaskSetID][i] > TaskID) return TaskSets[TaskSetID][i];
	}

	return 0;
}

bool TaskManager::AppropriateLevel(int TaskID, int PlayerLevel) {

	if(Tasks[TaskID] == nullptr) return false;

	if(Tasks[TaskID]->MinLevel && (PlayerLevel < Tasks[TaskID]->MinLevel)) return false;

	if(Tasks[TaskID]->MaxLevel && (PlayerLevel > Tasks[TaskID]->MaxLevel)) return false;

	return true;

}

std::string TaskManager::GetTaskName(uint32 task_id)
{
	if (task_id > 0 && task_id < MAXTASKS) {
		if (Tasks[task_id] != nullptr) {
			return Tasks[task_id]->Title;
		}
	}

	return std::string();
}

int TaskManager::GetTaskMinLevel(int TaskID)
{
	if (Tasks[TaskID]->MinLevel)
	{
		return Tasks[TaskID]->MinLevel;
	}
		
	return -1;
}

int TaskManager::GetTaskMaxLevel(int TaskID)
{
	if (Tasks[TaskID]->MaxLevel)
	{
		return Tasks[TaskID]->MaxLevel;
	}

	return -1;
}

void TaskManager::TaskSetSelector(Client *c, ClientTaskState *state, Mob *mob, int TaskSetID)
{
	int TaskList[MAXCHOOSERENTRIES];
	int TaskListIndex = 0;
	int PlayerLevel = c->GetLevel();

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSetSelector called for taskset %i. EnableTaskSize is %i",
	    TaskSetID, state->EnabledTasks.size());

	if (TaskSetID <= 0 || TaskSetID >= MAXTASKSETS)
		return;

	if (TaskSets[TaskSetID].empty()) {
		mob->SayString(c, Chat::Yellow, MAX_ACTIVE_TASKS, c->GetName()); // I think this is suppose to be yellow
		return;
	}

	bool all_enabled = false;

	// A TaskID of 0 in a TaskSet indicates that all Tasks in the set are enabled for all players.
	if (TaskSets[TaskSetID][0] == 0) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSets[%i][0] == 0. All Tasks in Set enabled.", TaskSetID);
		all_enabled = true;
	}

	auto Iterator = TaskSets[TaskSetID].begin();

	if (all_enabled)
		++Iterator; // skip first when all enabled since it's useless data

	while (Iterator != TaskSets[TaskSetID].end() && TaskListIndex < MAXCHOOSERENTRIES) {
		auto task = *Iterator;
		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if ((all_enabled || state->IsTaskEnabled(task)) && AppropriateLevel(task, PlayerLevel) &&
		    !state->IsTaskActive(task) && state->HasSlotForTask(Tasks[task]) && // this slot checking is a bit silly, but we allow mixing of task types ...
		    (IsTaskRepeatable(task) || !state->IsTaskCompleted(task)))
			TaskList[TaskListIndex++] = task;

		++Iterator;
	}

	if (TaskListIndex > 0) {
		SendTaskSelector(c, mob, TaskListIndex, TaskList);
	} else {
		mob->SayString(c, Chat::Yellow, MAX_ACTIVE_TASKS, c->GetName()); // check color, I think this might be only for (Shared) Tasks, w/e -- think should be yellow
	}

	return;
}

// unlike the non-Quest version of this function, it does not check enabled, that is assumed the responsibility of the quest to handle
// we do however still want it to check the other stuff like level, active, room, etc
void TaskManager::TaskQuestSetSelector(Client *c, ClientTaskState *state, Mob *mob, int count, int *tasks)
{
	int TaskList[MAXCHOOSERENTRIES];
	int TaskListIndex = 0;
	int PlayerLevel = c->GetLevel();

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskQuestSetSelector called for array size %d", count);

	if (count <= 0)
		return;

	for (int i = 0; i < count; ++i) {
		auto task = tasks[i];
		// verify level, we're not currently on it, repeatable status, if it's a (shared) task
		// we aren't currently on another, and if it's enabled if not all_enabled
		if (AppropriateLevel(task, PlayerLevel) &&
		    !state->IsTaskActive(task) && state->HasSlotForTask(Tasks[task]) && // this slot checking is a bit silly, but we allow mixing of task types ...
		    (IsTaskRepeatable(task) || !state->IsTaskCompleted(task)))
			TaskList[TaskListIndex++] = task;
	}

	if (TaskListIndex > 0) {
		SendTaskSelector(c, mob, TaskListIndex, TaskList);
	} else {
		mob->SayString(c, Chat::Yellow, MAX_ACTIVE_TASKS, c->GetName()); // check color, I think this might be only for (Shared) Tasks, w/e -- think should be yellow
	}

	return;
}

void TaskManager::SendTaskSelector(Client *c, Mob *mob, int TaskCount, int *TaskList) {

	if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF)
	{
		SendTaskSelectorNew(c, mob, TaskCount, TaskList);
		return;
	}
	// Titanium OpCode: 0x5e7c
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSelector for %i Tasks", TaskCount);
	int PlayerLevel = c->GetLevel();

	// Check if any of the tasks exist
	for (int i = 0; i < TaskCount; i++) {
		if (Tasks[TaskList[i]] != nullptr)
			break;
	}

	int ValidTasks = 0;

	for (int i = 0; i < TaskCount; i++) {
		if (!AppropriateLevel(TaskList[i], PlayerLevel))
			continue;
		if (c->IsTaskActive(TaskList[i]))
			continue;
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i]))
			continue;

		ValidTasks++;
	}

	if (ValidTasks == 0)
		return;

	SerializeBuffer buf(50 * ValidTasks);


	buf.WriteUInt32(ValidTasks);
	buf.WriteUInt32(2); // task type, live doesn't let you send more than one type, but we do?
	buf.WriteUInt32(mob->GetID());

	for (int i = 0; i < TaskCount; i++) {
		if (!AppropriateLevel(TaskList[i], PlayerLevel))
			continue;
		if (c->IsTaskActive(TaskList[i]))
			continue;
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i]))
			continue;

		buf.WriteUInt32(TaskList[i]);	// TaskID
		if (c->ClientVersion() != EQ::versions::ClientVersion::Titanium)
			buf.WriteFloat(1.0f); // affects color, difficulty?
		buf.WriteUInt32(Tasks[TaskList[i]]->Duration);
		buf.WriteUInt32(static_cast<int>(Tasks[TaskList[i]]->dur_code));

		buf.WriteString(Tasks[TaskList[i]]->Title); // max 64 with null
		buf.WriteString(Tasks[TaskList[i]]->Description); // max 4000 with null

		if (c->ClientVersion() != EQ::versions::ClientVersion::Titanium)
			buf.WriteUInt8(0); // Has reward set flag

		buf.WriteUInt32(Tasks[TaskList[i]]->ActivityCount);

		for (int j = 0; j < Tasks[TaskList[i]]->ActivityCount; ++j) {
			buf.WriteUInt32(j);				// ActivityNumber
			auto &activity = Tasks[TaskList[i]]->Activity[j];
			buf.WriteUInt32(activity.Type);
			buf.WriteUInt32(0);				// solo, group, raid?
			buf.WriteString(activity.target_name);	// max length 64, "target name" so like loot x foo from bar (this is bar)

			buf.WriteString(activity.item_list);		// max length 64 in these clients

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
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskSelector for %i Tasks", TaskCount);

	int PlayerLevel = c->GetLevel();

	// Check if any of the tasks exist
	for (int i = 0; i < TaskCount; i++)
		if (Tasks[TaskList[i]] != nullptr)
			break;

	int ValidTasks = 0;

	for (int i = 0; i < TaskCount; i++) {
		if (!AppropriateLevel(TaskList[i], PlayerLevel))
			continue;
		if (c->IsTaskActive(TaskList[i]))
			continue;
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i]))
			continue;

		ValidTasks++;
	}

	if (ValidTasks == 0)
		return;

	SerializeBuffer buf(50 * ValidTasks);

	buf.WriteUInt32(ValidTasks);	// TaskCount
	buf.WriteUInt32(2);			// Type, valid values: 0-3. 0 = Task, 1 = Shared Task, 2 = Quest, 3 = ??? -- should fix maybe some day, but we let more than 1 type through :P
	// so I guess an NPC can only offer one type of quests or we can only open a selection with one type :P (so quest call can tell us I guess)
	// this is also sent in OP_TaskDescription
	buf.WriteUInt32(mob->GetID());	// TaskGiver

	for (int i = 0; i < TaskCount; i++) { // max 40
		if (!AppropriateLevel(TaskList[i], PlayerLevel))
			continue;
		if (c->IsTaskActive(TaskList[i]))
			continue;
		if (!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i]))
			continue;

		buf.WriteUInt32(TaskList[i]);	// TaskID
		buf.WriteFloat(1.0f); // affects color, difficulty?
		buf.WriteUInt32(Tasks[TaskList[i]]->Duration);
		buf.WriteUInt32(static_cast<int>(Tasks[TaskList[i]]->dur_code));	// 1 = Short, 2 = Medium, 3 = Long, anything else Unlimited

		buf.WriteString(Tasks[TaskList[i]]->Title); // max 64 with null
		buf.WriteString(Tasks[TaskList[i]]->Description); // max 4000 with null

		buf.WriteUInt8(0);				// Has reward set flag
		buf.WriteUInt32(Tasks[TaskList[i]]->ActivityCount);	// ActivityCount

		for (int j = 0; j < Tasks[TaskList[i]]->ActivityCount; ++j) {
			buf.WriteUInt32(j);				// ActivityNumber
			auto &activity = Tasks[TaskList[i]]->Activity[j];
			buf.WriteUInt32(activity.Type);				// ActivityType
			buf.WriteUInt32(0);				// solo, group, raid?
			buf.WriteString(activity.target_name);	// max length 64, "target name" so like loot x foo from bar (this is bar)

			// this string is item names
			buf.WriteLengthString(activity.item_list);

			buf.WriteUInt32(activity.GoalCount);				// GoalCount

			// this string is skill IDs? probably one of the "use on" tasks
			buf.WriteLengthString(activity.skill_list);

			// this string is spell IDs? probably one of the "use on" tasks
			buf.WriteLengthString(activity.spell_list);

			//buf.WriteString(itoa(Tasks[TaskList[i]]->Activity[ActivityID].ZoneID));
			buf.WriteString(activity.zones);		// Zone number in ascii max length 64, can be multiple with separated by ;
			buf.WriteString(activity.desc_override);	// max length 128 -- overrides the automatic descriptions
			// this doesn't appear to be shown to the client at all and isn't the same as zones ... defaults to '0' though
			buf.WriteString(activity.zones);		// Zone number in ascii max length 64, probably can be separated by ; too, haven't found it used
		}
	}

	auto outapp = new EQApplicationPacket(OP_OpenNewTasksWindow, buf);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

int TaskManager::GetActivityCount(int TaskID) {

	// Return the total number of activities in a particular task.

	if((TaskID>0) && (TaskID<MAXTASKS))
		if(Tasks[TaskID]) return Tasks[TaskID]->ActivityCount;

	return 0;

}

void TaskManager::ExplainTask(Client*c, int TaskID) {

	// TODO: This method is not finished (hardly started). It was intended to
	// explain in English, what each activity did, conditions for step unlocking, etc.
	//
	return;

	if(!c) return;

	if((TaskID<=0) || (TaskID>=MAXTASKS)) {
		c->Message(Chat::White, "TaskID out-of-range.");
		return;
	}

	if(Tasks[TaskID] == nullptr) {
		c->Message(Chat::White, "Task does not exist.");
		return;
	}

	char Explanation[1000], *ptr;
	c->Message(Chat::White, "Task %4i: Title: %s", TaskID, Tasks[TaskID]->Description.c_str());
	c->Message(Chat::White, "%3i Activities", Tasks[TaskID]->ActivityCount);
	ptr = Explanation;
	for(int i=0; i<Tasks[TaskID]->ActivityCount; i++) {

		sprintf(ptr, "Act: %3i: ", i); ptr = ptr + strlen(ptr);
		switch(Tasks[TaskID]->Activity[i].Type) {
			case ActivityDeliver:
				sprintf(ptr, "Deliver");
				break;
		}

	}
}

ClientTaskState::ClientTaskState() {

	ActiveTaskCount = 0;
	LastCompletedTaskLoaded = 0;
	CheckedTouchActivities = false;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		ActiveQuests[i].slot = i;
		ActiveQuests[i].TaskID = TASKSLOTEMPTY;
	}

	ActiveTask.slot = 0;
	ActiveTask.TaskID = TASKSLOTEMPTY;
	// TODO: shared task
}

ClientTaskState::~ClientTaskState() {
}


int ClientTaskState::GetActiveTaskID(int index) {

	// Return the TaskID from the client's specified Active Task slot.

	if((index<0) || (index>=MAXACTIVEQUESTS)) return 0;

	return ActiveQuests[index].TaskID;
}

static void DeleteCompletedTaskFromDatabase(int charID, int taskID) {

    Log(Logs::General, Logs::Tasks, "[UPDATE] DeleteCompletedTasksFromDatabase. CharID = %i, TaskID = %i", charID, taskID);

    const std::string query = StringFormat("DELETE FROM completed_tasks WHERE charid=%i AND taskid = %i", charID, taskID);
    auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		return;
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] Delete query %s", query.c_str());
}

bool ClientTaskState::UnlockActivities(int CharID, ClientTaskInformation &task_info)
{
	bool AllActivitiesComplete = true;

	TaskInformation* Task = taskmanager->Tasks[task_info.TaskID];

	if (Task == nullptr)
		return true;

	// On loading the client state, all activities that are not completed, are
	// marked as hidden. For Sequential (non-stepped) mode, we mark the first
	// activity as active if not complete.
	Log(Logs::General, Logs::Tasks, "[UPDATE] CharID: %i Task: %i Sequence mode is %i",
				CharID, task_info.TaskID, Task->SequenceMode);

	if (Task->SequenceMode == ActivitiesSequential) {
		if (task_info.Activity[0].State != ActivityCompleted)
			task_info.Activity[0].State = ActivityActive;

		// Enable the next Hidden task.
		for (int i = 0; i < Task->ActivityCount; i++) {
			if ((task_info.Activity[i].State == ActivityActive) &&
			    (!Task->Activity[i].Optional)) {
				AllActivitiesComplete = false;
				break;
			}

			if (task_info.Activity[i].State == ActivityHidden) {
				task_info.Activity[i].State = ActivityActive;
				AllActivitiesComplete = false;
				break;
			}
		}

		if (AllActivitiesComplete && RuleB(TaskSystem, RecordCompletedTasks)) {
			if (RuleB(TasksSystem, KeepOneRecordPerCompletedTask)) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] KeepOneRecord enabled");
				auto Iterator = CompletedTasks.begin();
				int ErasedElements = 0;
				while (Iterator != CompletedTasks.end()) {
					int TaskID = (*Iterator).TaskID;
					if (TaskID == task_info.TaskID) {
						Iterator = CompletedTasks.erase(Iterator);
						ErasedElements++;
					} else
						++Iterator;
				}

				Log(Logs::General, Logs::Tasks, "[UPDATE] Erased Element count is %i", ErasedElements);

				if (ErasedElements) {
					LastCompletedTaskLoaded -= ErasedElements;
					DeleteCompletedTaskFromDatabase(CharID, task_info.TaskID);
				}
			}

			CompletedTaskInformation cti;
			cti.TaskID = task_info.TaskID;
			cti.CompletedTime = time(nullptr);

			for (int i = 0; i < Task->ActivityCount; i++)
				cti.ActivityDone[i] = (task_info.Activity[i].State == ActivityCompleted);

			CompletedTasks.push_back(cti);
		}

		Log(Logs::General, Logs::Tasks, "[UPDATE] Returning sequential task, AllActivitiesComplete is %i",
		    AllActivitiesComplete);

		return AllActivitiesComplete;
	}

	// Stepped Mode
	// TODO: This code is probably more complex than it needs to be

	bool CurrentStepComplete = true;

	Log(Logs::General, Logs::Tasks, "[UPDATE] Current Step is %i, Last Step is %i", task_info.CurrentStep,
	    Task->LastStep);
	// If CurrentStep is -1, this is the first call to this method since loading the
	// client state. Unlock all activities with a step number of 0

	if (task_info.CurrentStep == -1) {
		for (int i = 0; i < Task->ActivityCount; i++) {
			if (Task->Activity[i].StepNumber == 0 && task_info.Activity[i].State == ActivityHidden) {
				task_info.Activity[i].State = ActivityActive;
				// task_info.Activity[i].Updated=true;
			}
		}
		task_info.CurrentStep = 0;
	}

	for (int Step = task_info.CurrentStep; Step <= Task->LastStep; Step++) {
		for (int Activity = 0; Activity < Task->ActivityCount; Activity++) {
			if (Task->Activity[Activity].StepNumber == (int)task_info.CurrentStep) {
				if ((task_info.Activity[Activity].State != ActivityCompleted) &&
				    (!Task->Activity[Activity].Optional)) {
					CurrentStepComplete = false;
					AllActivitiesComplete = false;
					break;
				}
			}
		}
		if (!CurrentStepComplete)
			break;
		task_info.CurrentStep++;
	}

	if (AllActivitiesComplete) {
		if (RuleB(TaskSystem, RecordCompletedTasks)) {
			// If we are only keeping one completed record per task, and the player has done
			// the same task again, erase the previous completed entry for this task.
			if (RuleB(TasksSystem, KeepOneRecordPerCompletedTask)) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] KeepOneRecord enabled");
				auto Iterator = CompletedTasks.begin();
				int ErasedElements = 0;

				while (Iterator != CompletedTasks.end()) {
					int TaskID = (*Iterator).TaskID;
					if (TaskID == task_info.TaskID) {
						Iterator = CompletedTasks.erase(Iterator);
						ErasedElements++;
					} else
						++Iterator;
				}

				Log(Logs::General, Logs::Tasks, "[UPDATE] Erased Element count is %i", ErasedElements);

				if (ErasedElements) {
					LastCompletedTaskLoaded -= ErasedElements;
					DeleteCompletedTaskFromDatabase(CharID, task_info.TaskID);
				}
			}

			CompletedTaskInformation cti;
			cti.TaskID = task_info.TaskID;
			cti.CompletedTime = time(nullptr);

			for (int i = 0; i < Task->ActivityCount; i++)
				cti.ActivityDone[i] = (task_info.Activity[i].State == ActivityCompleted);

			CompletedTasks.push_back(cti);
		}
		return true;
	}

	// Mark all non-completed tasks in the current step as active
	//
	for (int Activity = 0; Activity < Task->ActivityCount; Activity++) {
		if ((Task->Activity[Activity].StepNumber == (int)task_info.CurrentStep) &&
		    (task_info.Activity[Activity].State == ActivityHidden)) {
			task_info.Activity[Activity].State = ActivityActive;
			task_info.Activity[Activity].Updated = true;
		}
	}

	return false;
}

void ClientTaskState::UpdateTasksOnKill(Client *c, int NPCTypeID) {

	UpdateTasksByNPC(c, ActivityKill, NPCTypeID);

}

bool ClientTaskState::UpdateTasksOnSpeakWith(Client *c, int NPCTypeID) {

	return UpdateTasksByNPC(c, ActivitySpeakWith, NPCTypeID);

}

bool ClientTaskState::UpdateTasksByNPC(Client *c, int ActivityType, int NPCTypeID) {

	int Ret = false;

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState::UpdateTasks for NPCTypeID: %d", NPCTypeID);

	// If the client has no tasks, there is nothing further to check.

	if (!taskmanager || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)) // could be better ...
		return false;

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->TaskID == TASKSLOTEMPTY)
			continue;

		// Check if there are any active kill activities for this task

		auto Task = taskmanager->Tasks[cur_task->TaskID];

		if (Task == nullptr)
			return false;

		for (int j = 0; j < Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->Activity[j].State != ActivityActive)
				continue;
			// We are only interested in Kill activities
			if (Task->Activity[j].Type != ActivityType)
				continue;
			// Is there a zone restriction on the activity ?
			if (!Task->Activity[j].CheckZone(zone->GetZoneID())) {
				Log(Logs::General, Logs::Tasks,
					"[UPDATE] Char: %s Task: %i, Activity %i, Activity type %i for NPC %i failed zone "
					"check",
					c->GetName(), cur_task->TaskID, j, ActivityType, NPCTypeID);
				continue;
			}
			// Is the activity to kill this type of NPC ?
			switch (Task->Activity[j].GoalMethod) {

			case METHODSINGLEID:
				if (Task->Activity[j].GoalID != NPCTypeID)
					continue;
				break;

			case METHODLIST:
				if (!taskmanager->GoalListManager.IsInList(Task->Activity[j].GoalID, NPCTypeID))
					continue;
				break;

			default:
				// If METHODQUEST, don't update the activity here
				continue;
			}
			// We found an active task to kill this type of NPC, so increment the done count
			Log(Logs::General, Logs::Tasks, "[UPDATE] Calling increment done count ByNPC");
			IncrementDoneCount(c, Task, cur_task->slot, j);
			Ret = true;
		}
	}

	return Ret;
}

int ClientTaskState::ActiveSpeakTask(int NPCTypeID) {

	// This method is to be used from Perl quests only and returns the TaskID of the first
	// active task found which has an active SpeakWith activity for this NPC.

	if (!taskmanager || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)) // could be better ...
		return 0;

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->TaskID == TASKSLOTEMPTY)
			continue;

		TaskInformation* Task = taskmanager->Tasks[cur_task->TaskID];

		if (Task == nullptr)
			continue;

		for (int j = 0; j < Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->Activity[j].State != ActivityActive)
				continue;
			if (Task->Activity[j].Type != ActivitySpeakWith)
				continue;
			// Is there a zone restriction on the activity ?
			if (!Task->Activity[j].CheckZone(zone->GetZoneID()))
				continue;
			// Is the activity to speak with this type of NPC ?
			if (Task->Activity[j].GoalMethod == METHODQUEST && Task->Activity[j].GoalID == NPCTypeID)
				return cur_task->TaskID;
		}
	}
	return 0;
}

int ClientTaskState::ActiveSpeakActivity(int NPCTypeID, int TaskID) {

	// This method is to be used from Perl quests only and returns the ActivityID of the first
	// active activity found in the specified task which is to SpeakWith this NPC.

	if (!taskmanager || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)) // could be better ...
		return -1;
	if (TaskID <= 0 || TaskID >= MAXTASKS)
		return -1;

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->TaskID != TaskID)
			continue;

		TaskInformation* Task = taskmanager->Tasks[cur_task->TaskID];

		if (Task == nullptr)
			continue;

		for (int j = 0; j < Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->Activity[j].State != ActivityActive)
				continue;
			if (Task->Activity[j].Type != ActivitySpeakWith)
				continue;
			// Is there a zone restriction on the activity ?
			if (!Task->Activity[j].CheckZone(zone->GetZoneID()))
				continue;
			// Is the activity to speak with this type of NPC ?
			if (Task->Activity[j].GoalMethod == METHODQUEST && Task->Activity[j].GoalID == NPCTypeID)
				return j;
		}
		return 0;
	}
	return 0;
}

void ClientTaskState::UpdateTasksForItem(Client *c, ActivityType Type, int ItemID, int Count) {

	// This method updates the client's task activities of the specified type which relate
	// to the specified item.
	//
	// Type should be one of ActivityLoot, ActivityTradeSkill, ActivityFish or ActivityForage

	// If the client has no tasks, there is nothing further to check.

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState::UpdateTasksForItem(%d,%d)", Type, ItemID);

	if (!taskmanager || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)) // could be better ...
		return;

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->TaskID == TASKSLOTEMPTY)
			continue;

		// Check if there are any active loot activities for this task

		TaskInformation* Task = taskmanager->Tasks[cur_task->TaskID];

		if (Task == nullptr)
			return;

		for (int j = 0; j < Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->Activity[j].State != ActivityActive)
				continue;
			// We are only interested in the ActivityType we were called with
			if (Task->Activity[j].Type != (int)Type)
				continue;
			// Is there a zone restriction on the activity ?
			if (!Task->Activity[j].CheckZone(zone->GetZoneID())) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] Char: %s Activity type %i for Item %i failed zone check",
							c->GetName(), Type, ItemID);
				continue;
			}
			// Is the activity related to this item ?
			//
			switch(Task->Activity[j].GoalMethod) {

				case METHODSINGLEID:
					if(Task->Activity[j].GoalID != ItemID) continue;
					break;

				case METHODLIST:
					if(!taskmanager->GoalListManager.IsInList(Task->Activity[j].GoalID, ItemID)) continue;
					break;

				default:
					// If METHODQUEST, don't update the activity here
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
	if (!taskmanager || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)) // could be better ...
		return;

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->TaskID == TASKSLOTEMPTY)
			continue;

		// Check if there are any active explore activities for this task

		TaskInformation *Task = taskmanager->Tasks[cur_task->TaskID];

		if (Task == nullptr)
			return;

		for (int j = 0; j < Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->Activity[j].State != ActivityActive)
				continue;
			// We are only interested in explore activities
			if (Task->Activity[j].Type != ActivityExplore)
				continue;
			if (!Task->Activity[j].CheckZone(zone->GetZoneID())) {
				Log(Logs::General, Logs::Tasks,
				    "[UPDATE] Char: %s Explore exploreid %i failed zone check", c->GetName(),
				    ExploreID);
				continue;
			}
			// Is the activity to explore this area id ?
			switch (Task->Activity[j].GoalMethod) {

			case METHODSINGLEID:
				if (Task->Activity[j].GoalID != ExploreID)
					continue;
				break;

			case METHODLIST:
				if (!taskmanager->GoalListManager.IsInList(Task->Activity[j].GoalID, ExploreID))
					continue;
				break;

			default:
				// If METHODQUEST, don't update the activity here
				continue;
			}
			// We found an active task to explore this area, so set done count to goal count
			// (Only a goal count of 1 makes sense for explore activities?)
			Log(Logs::General, Logs::Tasks, "[UPDATE] Increment on explore");
			IncrementDoneCount(c, Task, cur_task->slot, j,
					   Task->Activity[j].GoalCount - cur_task->Activity[j].DoneCount);
		}
	}

	return;
}

bool ClientTaskState::UpdateTasksOnDeliver(Client *c, std::list<EQ::ItemInstance *> &Items, int Cash, int NPCTypeID)
{
	bool Ret = false;

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState::UpdateTasksForOnDeliver(%d)", NPCTypeID);

	if (!taskmanager || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)) // could be better ...
		return false;

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->TaskID == TASKSLOTEMPTY)
			continue;

		// Check if there are any active deliver activities for this task

		TaskInformation *Task = taskmanager->Tasks[cur_task->TaskID];

		if (Task == nullptr)
			return false;

		for (int j = 0; j < Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->Activity[j].State != ActivityActive)
				continue;
			// We are only interested in Deliver activities
			if (Task->Activity[j].Type != ActivityDeliver && Task->Activity[j].Type != ActivityGiveCash)
				continue;
			// Is there a zone restriction on the activity ?
			if (!Task->Activity[j].CheckZone(zone->GetZoneID())) {
				Log(Logs::General, Logs::Tasks,
				    "[UPDATE] Char: %s Deliver activity failed zone check (current zone %i, need zone "
				    "%s",
				    c->GetName(), zone->GetZoneID(), Task->Activity[j].zones.c_str());
				continue;
			}
			// Is the activity to deliver to this NPCTypeID ?
			if (Task->Activity[j].DeliverToNPC != NPCTypeID)
				continue;
			// Is the activity related to these items ?
			//
			if ((Task->Activity[j].Type == ActivityGiveCash) && Cash) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] Increment on GiveCash");
				IncrementDoneCount(c, Task, i, j, Cash);
				Ret = true;
			} else {
				for (auto &k : Items) {
					switch (Task->Activity[j].GoalMethod) {

					case METHODSINGLEID:
						if (Task->Activity[j].GoalID != k->GetID())
							continue;
						break;

					case METHODLIST:
						if (!taskmanager->GoalListManager.IsInList(Task->Activity[j].GoalID,
											   k->GetID()))
							continue;
						break;

					default:
						// If METHODQUEST, don't update the activity here
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
	if (!taskmanager || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)) // could be better ...
		return;

	// loop over the union of tasks and quests
	for (int i = 0; i < MAXACTIVEQUESTS + 1; i++) {
		auto cur_task = &ActiveTasks[i];
		if (cur_task->TaskID == TASKSLOTEMPTY)
			continue;

		// Check if there are any active explore activities for this task

		TaskInformation *Task = taskmanager->Tasks[cur_task->TaskID];

		if (Task == nullptr)
			return;

		for (int j = 0; j < Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if (cur_task->Activity[j].State != ActivityActive)
				continue;
			// We are only interested in touch activities
			if (Task->Activity[j].Type != ActivityTouch)
				continue;
			if (Task->Activity[j].GoalMethod != METHODSINGLEID)
				continue;
			if (!Task->Activity[j].CheckZone(ZoneID)) {
				Log(Logs::General, Logs::Tasks, "[UPDATE] Char: %s Touch activity failed zone check",
				    c->GetName());
				continue;
			}
			// We found an active task to zone into this zone, so set done count to goal count
			// (Only a goal count of 1 makes sense for touch activities?)
			Log(Logs::General, Logs::Tasks, "[UPDATE] Increment on Touch");
			IncrementDoneCount(c, Task, cur_task->slot, j,
					   Task->Activity[j].GoalCount - cur_task->Activity[j].DoneCount);
		}
	}

	return;
}

void ClientTaskState::IncrementDoneCount(Client *c, TaskInformation *Task, int TaskIndex, int ActivityID, int Count,
					 bool ignore_quest_update)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] IncrementDoneCount");

	auto info = GetClientTaskInfo(Task->type, TaskIndex);

	if (info == nullptr)
		return;

	info->Activity[ActivityID].DoneCount += Count;

	if(info->Activity[ActivityID].DoneCount > Task->Activity[ActivityID].GoalCount)
		info->Activity[ActivityID].DoneCount = Task->Activity[ActivityID].GoalCount;

	if (!ignore_quest_update){
		char buf[24];
		snprintf(buf, 23, "%d %d %d", info->Activity[ActivityID].DoneCount, info->Activity[ActivityID].ActivityID, info->TaskID);
		buf[23] = '\0';
		parse->EventPlayer(EVENT_TASK_UPDATE, c, buf, 0);
	}

	info->Activity[ActivityID].Updated=true;
	// Have we reached the goal count for this activity ?
	if(info->Activity[ActivityID].DoneCount >= Task->Activity[ActivityID].GoalCount) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] Done (%i) = Goal (%i) for Activity %i",
				info->Activity[ActivityID].DoneCount,
				Task->Activity[ActivityID].GoalCount,
				ActivityID);

		// Flag the activity as complete
		info->Activity[ActivityID].State = ActivityCompleted;
		// Unlock subsequent activities for this task
		bool TaskComplete = UnlockActivities(c->CharacterID(), *info);
		Log(Logs::General, Logs::Tasks, "[UPDATE] TaskCompleted is %i", TaskComplete);
		// and by the 'Task Stage Completed' message
		c->SendTaskActivityComplete(info->TaskID, ActivityID, TaskIndex, Task->type);
		// Send the updated task/activity list to the client
		taskmanager->SendSingleActiveTaskToClient(c, *info, TaskComplete, false);
		// Inform the client the task has been updated, both by a chat message
		c->Message(Chat::White, "Your task '%s' has been updated.", Task->Title.c_str());

		if(Task->Activity[ActivityID].GoalMethod != METHODQUEST) {
			if (!ignore_quest_update){
				char buf[24];
				snprintf(buf, 23, "%d %d", info->TaskID, info->Activity[ActivityID].ActivityID);
				buf[23] = '\0';
				parse->EventPlayer(EVENT_TASK_STAGE_COMPLETE, c, buf, 0);
			}
			/* QS: PlayerLogTaskUpdates :: Update */
			if (RuleB(QueryServ, PlayerLogTaskUpdates)){
				std::string event_desc = StringFormat("Task Stage Complete :: taskid:%i activityid:%i donecount:%i in zoneid:%i instid:%i", info->TaskID, info->Activity[ActivityID].ActivityID, info->Activity[ActivityID].DoneCount, c->GetZoneID(), c->GetInstanceID());
				QServ->PlayerLogEvent(Player_Log_Task_Updates, c->CharacterID(), event_desc);
			}
		}

		// If this task is now complete, the Completed tasks will have been
		// updated in UnlockActivities. Send the completed task list to the
		// client. This is the same sequence the packets are sent on live.
		if(TaskComplete) {
			char buf[24];
			snprintf(buf, 23, "%d %d %d", info->Activity[ActivityID].DoneCount, info->Activity[ActivityID].ActivityID, info->TaskID);
			buf[23] = '\0';
			parse->EventPlayer(EVENT_TASK_COMPLETE, c, buf, 0);

			/* QS: PlayerLogTaskUpdates :: Complete */
			if (RuleB(QueryServ, PlayerLogTaskUpdates)){
				std::string event_desc = StringFormat("Task Complete :: taskid:%i activityid:%i donecount:%i in zoneid:%i instid:%i", info->TaskID, info->Activity[ActivityID].ActivityID, info->Activity[ActivityID].DoneCount, c->GetZoneID(), c->GetInstanceID());
				QServ->PlayerLogEvent(Player_Log_Task_Updates, c->CharacterID(), event_desc);
			}

			taskmanager->SendCompletedTasksToClient(c, this);
			c->SendTaskActivityComplete(info->TaskID, 0, TaskIndex, Task->type, 0);
			taskmanager->SaveClientState(c, this);
			//c->SendTaskComplete(TaskIndex);
			c->CancelTask(TaskIndex, Task->type);
			//if(Task->RewardMethod != METHODQUEST) RewardTask(c, Task);
			// If Experience and/or cash rewards are set, reward them from the task even if RewardMethod is METHODQUEST
			RewardTask(c, Task);
			//RemoveTask(c, TaskIndex);

		}

	} else {
		// Send an update packet for this single activity
		taskmanager->SendTaskActivityLong(c, info->TaskID, ActivityID, TaskIndex,
						  Task->Activity[ActivityID].Optional);
	}
}

void ClientTaskState::RewardTask(Client *c, TaskInformation *Task) {

	if(!Task || !c) return;

	const EQ::ItemData* Item;
	std::vector<int> RewardList;

	switch(Task->RewardMethod) {

		case METHODSINGLEID:
		{
			if(Task->RewardID) {
				c->SummonItem(Task->RewardID);
				Item = database.GetItem(Task->RewardID);
				if(Item)
					c->Message(Chat::Yellow, "You receive %s as a reward.", Item->Name);
			}
			break;
		}
		case METHODLIST:
		{
			RewardList = taskmanager->GoalListManager.GetListContents(Task->RewardID);
			for(unsigned int i=0; i<RewardList.size(); i++) {
				c->SummonItem(RewardList[i]);
				Item = database.GetItem(RewardList[i]);
				if(Item)
					c->Message(Chat::Yellow, "You receive %s as a reward.", Item->Name);
			}
			break;
		}
		default:
		{
			// Nothing special done for METHODQUEST
			break;
		}
	}

	if (!Task->completion_emote.empty())
		c->SendColoredText(Chat::Yellow, Task->completion_emote); // unsure if they use this packet or color, should work

	// just use normal NPC faction ID stuff
	if (Task->faction_reward)
		c->SetFactionLevel(c->CharacterID(), Task->faction_reward, c->GetBaseClass(), c->GetBaseRace(), c->GetDeity());

	if(Task->CashReward) {
		int Plat, Gold, Silver, Copper;

		Copper = Task->CashReward;
		c->AddMoneyToPP(Copper, true);

		Plat = Copper / 1000;
		Copper = Copper - (Plat * 1000);
		Gold = Copper / 100;
		Copper = Copper - (Gold * 100);
		Silver = Copper / 10;
		Copper = Copper - (Silver * 10);

		std::string CashMessage;

		if (Plat>0){
			CashMessage = "You receive ";
			CashMessage += itoa(Plat);
			CashMessage += " platinum";
		}
		if (Gold>0){
			if (CashMessage.length()==0){
				CashMessage = "You receive ";
			}
			else{
				CashMessage += ",";
			}
			CashMessage += itoa(Gold);
			CashMessage += " gold";
		}
		if(Silver>0){
			if (CashMessage.length()==0){
				CashMessage = "You receive ";
			}
			else{
				CashMessage += ",";
			}
			CashMessage += itoa(Silver);
			CashMessage += " silver";
		}
		if(Copper>0){
			if (CashMessage.length()==0){
				CashMessage = "You receive ";
			}
			else{
				CashMessage += ",";
			}
			CashMessage += itoa(Copper);
			CashMessage += " copper";
		}
		CashMessage += " pieces.";
		c->Message(Chat::Yellow,CashMessage.c_str());
	}
	int32 EXPReward = Task->XPReward;
	if(EXPReward > 0) {
		c->AddEXP(EXPReward);
	}
	if(EXPReward < 0) {
		uint32 PosReward = EXPReward * -1;
		// Minimal Level Based Exp Reward Setting is 101 (1% exp at level 1)
		if (PosReward > 100 && PosReward < 25700) {
			uint8 MaxLevel = PosReward / 100;
			uint8 ExpPercent = PosReward - (MaxLevel * 100);
			c->AddLevelBasedExp(ExpPercent, MaxLevel);
		}
	}

	c->SendSound();
}

bool ClientTaskState::IsTaskActive(int TaskID)
{
	if (ActiveTask.TaskID == TaskID)
		return true;

	if (ActiveTaskCount == 0 || TaskID == 0)
		return false;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (ActiveQuests[i].TaskID == TaskID)
			return true;
	}

	return false;
}

void ClientTaskState::FailTask(Client *c, int TaskID)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] FailTask %i, ActiveTaskCount is %i", TaskID, ActiveTaskCount);

	if (ActiveTask.TaskID == TaskID) {
		c->SendTaskFailed(TaskID, 0, TaskType::Task);
		// Remove the task from the client
		c->CancelTask(0, TaskType::Task);
		return;
	}

	// TODO: shared tasks

	if (ActiveTaskCount == 0)
		return;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (ActiveQuests[i].TaskID == TaskID) {
			c->SendTaskFailed(ActiveQuests[i].TaskID, i, TaskType::Quest);
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
	if (ActivityID < 0)
		return false;
	if (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY)
		return false;

	int ActiveTaskIndex = -1;
	auto type = TaskType::Task;

	if (ActiveTask.TaskID == TaskID)
		ActiveTaskIndex = 0;

	if (ActiveTaskIndex == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (ActiveQuests[i].TaskID == TaskID) {
				ActiveTaskIndex = i;
				type = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (ActiveTaskIndex == -1)
		return false;

	auto info = GetClientTaskInfo(type, ActiveTaskIndex);

	if (info == nullptr)
		return false;

	TaskInformation *Task = taskmanager->Tasks[info->TaskID];

	// The task is invalid
	if (Task == nullptr)
		return false;

	// The ActivityID is out of range
	if (ActivityID >= Task->ActivityCount)
		return false;

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState IsTaskActivityActive(%i, %i). State is %i ", TaskID,
	    ActivityID, info->Activity[ActivityID].State);

	return (info->Activity[ActivityID].State == ActivityActive);
}

void ClientTaskState::UpdateTaskActivity(Client *c, int TaskID, int ActivityID, int Count, bool ignore_quest_update /*= false*/)
{

	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState UpdateTaskActivity(%i, %i, %i).", TaskID, ActivityID,
	    Count);

	// Quick sanity check
	if (ActivityID < 0 || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY))
		return;

	int ActiveTaskIndex = -1;
	auto type = TaskType::Task;

	if (ActiveTask.TaskID == TaskID)
		ActiveTaskIndex = 0;

	if (ActiveTaskIndex == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (ActiveQuests[i].TaskID == TaskID) {
				ActiveTaskIndex = i;
				type = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (ActiveTaskIndex == -1)
		return;

	auto info = GetClientTaskInfo(type, ActiveTaskIndex);

	if (info == nullptr)
		return;

	TaskInformation *Task = taskmanager->Tasks[info->TaskID];

	// The task is invalid
	if (Task == nullptr)
		return;

	// The ActivityID is out of range
	if (ActivityID >= Task->ActivityCount)
		return;

	// The Activity is not currently active
	if (info->Activity[ActivityID].State != ActivityActive)
		return;
	Log(Logs::General, Logs::Tasks, "[UPDATE] Increment done count on UpdateTaskActivity");
	IncrementDoneCount(c, Task, ActiveTaskIndex, ActivityID, Count, ignore_quest_update);
}

void ClientTaskState::ResetTaskActivity(Client *c, int TaskID, int ActivityID)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState UpdateTaskActivity(%i, %i, 0).", TaskID, ActivityID);

	// Quick sanity check
	if (ActivityID < 0 || (ActiveTaskCount == 0 && ActiveTask.TaskID == TASKSLOTEMPTY))
		return;

	int ActiveTaskIndex = -1;
	auto type = TaskType::Task;

	if (ActiveTask.TaskID == TaskID)
		ActiveTaskIndex = 0;

	if (ActiveTaskIndex == -1) {
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			if (ActiveQuests[i].TaskID == TaskID) {
				ActiveTaskIndex = i;
				type = TaskType::Quest;
				break;
			}
		}
	}

	// The client does not have this task
	if (ActiveTaskIndex == -1)
		return;

	auto info = GetClientTaskInfo(type, ActiveTaskIndex);

	if (info == nullptr)
		return;

	TaskInformation *Task = taskmanager->Tasks[info->TaskID];

	// The task is invalid
	if (Task == nullptr)
		return;

	// The ActivityID is out of range
	if (ActivityID >= Task->ActivityCount)
		return;

	// The Activity is not currently active
	if (info->Activity[ActivityID].State != ActivityActive)
		return;

	Log(Logs::General, Logs::Tasks, "[UPDATE] ResetTaskActivityCount");

	info->Activity[ActivityID].DoneCount = 0;

	info->Activity[ActivityID].Updated = true;

	// Send an update packet for this single activity
	taskmanager->SendTaskActivityLong(c, info->TaskID, ActivityID, ActiveTaskIndex,
					  Task->Activity[ActivityID].Optional);
}

void ClientTaskState::ShowClientTasks(Client *c)
{
	c->Message(Chat::White, "Task Information:");
	if (ActiveTask.TaskID != TASKSLOTEMPTY) {
		c->Message(Chat::White, "Task: %i %s", ActiveTask.TaskID, taskmanager->Tasks[ActiveTask.TaskID]->Title.c_str());
		c->Message(Chat::White, "  Description: [%s]\n", taskmanager->Tasks[ActiveTask.TaskID]->Description.c_str());
		for (int j = 0; j < taskmanager->GetActivityCount(ActiveTask.TaskID); j++) {
			c->Message(Chat::White, "  Activity: %2d, DoneCount: %2d, Status: %d (0=Hidden, 1=Active, 2=Complete)",
				   ActiveTask.Activity[j].ActivityID, ActiveTask.Activity[j].DoneCount,
				   ActiveTask.Activity[j].State);
		}
	}

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (ActiveQuests[i].TaskID == TASKSLOTEMPTY)
			continue;

		c->Message(Chat::White, "Quest: %i %s", ActiveQuests[i].TaskID,
			   taskmanager->Tasks[ActiveQuests[i].TaskID]->Title.c_str());
		c->Message(Chat::White, "  Description: [%s]\n", taskmanager->Tasks[ActiveQuests[i].TaskID]->Description.c_str());
		for (int j = 0; j < taskmanager->GetActivityCount(ActiveQuests[i].TaskID); j++) {
			c->Message(Chat::White, "  Activity: %2d, DoneCount: %2d, Status: %d (0=Hidden, 1=Active, 2=Complete)",
				   ActiveQuests[i].Activity[j].ActivityID, ActiveQuests[i].Activity[j].DoneCount,
				   ActiveQuests[i].Activity[j].State);
		}
	}
}

// TODO: Shared Task
int ClientTaskState::TaskTimeLeft(int TaskID)
{
	if (ActiveTask.TaskID == TaskID) {
		int Now = time(nullptr);

		TaskInformation *Task = taskmanager->Tasks[TaskID];

		if (Task == nullptr)
			return -1;

		if (!Task->Duration)
			return -1;

		int TimeLeft = (ActiveTask.AcceptedTime + Task->Duration - Now);

		return (TimeLeft > 0 ? TimeLeft : 0);
	}

	if (ActiveTaskCount == 0)
		return -1;

	for (int i = 0; i < MAXACTIVEQUESTS; i++) {

		if (ActiveQuests[i].TaskID != TaskID)
			continue;

		int Now = time(nullptr);

		TaskInformation *Task = taskmanager->Tasks[ActiveQuests[i].TaskID];

		if (Task == nullptr)
			return -1;

		if (!Task->Duration)
			return -1;

		int TimeLeft = (ActiveQuests[i].AcceptedTime + Task->Duration - Now);

		// If Timeleft is negative, return 0, else return the number of seconds left

		return (TimeLeft > 0 ? TimeLeft : 0);
	}

	return -1;
}

int ClientTaskState::IsTaskCompleted(int TaskID) {

	// Returns:	-1 if RecordCompletedTasks is not true
	//		+1 if the task has been completed
	//		0 if the task has not been completed

	if(!(RuleB(TaskSystem, RecordCompletedTasks))) return -1;

	for(unsigned int i=0; i<CompletedTasks.size(); i++) {
		Log(Logs::General, Logs::Tasks, "[UPDATE] Comparing completed task %i with %i", CompletedTasks[i].TaskID, TaskID);
		if(CompletedTasks[i].TaskID == TaskID) return 1;
	}

	return 0;
}

bool TaskManager::IsTaskRepeatable(int TaskID) {

	if((TaskID <= 0) || (TaskID >= MAXTASKS)) return false;

	TaskInformation* Task = taskmanager->Tasks[TaskID];

	if(Task == nullptr) return false;

	return Task->Repeatable;
}

bool ClientTaskState::TaskOutOfTime(TaskType type, int Index)
{
	// Returns true if the Task in the specified slot has a time limit that has been exceeded.
	auto info = GetClientTaskInfo(type, Index);

	if (info == nullptr)
		return false;

	// make sure the TaskID is at least maybe in our array
	if (info->TaskID <= 0 || info->TaskID >= MAXTASKS)
		return false;

	int Now = time(nullptr);

	TaskInformation *Task = taskmanager->Tasks[info->TaskID];

	if (Task == nullptr)
		return false;

	return (Task->Duration && (info->AcceptedTime + Task->Duration <= Now));
}

void ClientTaskState::TaskPeriodicChecks(Client *c)
{
	if (ActiveTask.TaskID != TASKSLOTEMPTY) {
		if (TaskOutOfTime(TaskType::Task, 0)) {
			// Send Red Task Failed Message
			c->SendTaskFailed(ActiveTask.TaskID, 0, TaskType::Task);
			// Remove the task from the client
			c->CancelTask(0, TaskType::Task);
			// It is a conscious decision to only fail one task per call to this method,
			// otherwise the player will not see all the failed messages where multiple
			// tasks fail at the same time.
			return;
		}
	}

	// TODO: shared tasks -- although that will probably be manager in world checking and telling zones to fail us

	if (ActiveTaskCount == 0)
		return;

	// Check for tasks that have failed because they have not been completed in the specified time
	//
	for (int i = 0; i < MAXACTIVEQUESTS; i++) {
		if (ActiveQuests[i].TaskID == TASKSLOTEMPTY)
			continue;

		if (TaskOutOfTime(TaskType::Quest, i)) {
			// Send Red Task Failed Message
			c->SendTaskFailed(ActiveQuests[i].TaskID, i, TaskType::Quest);
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
	// the zone before we send the 'Task Activity Completed' message.
	//
	if (!CheckedTouchActivities) {
		UpdateTasksOnTouch(c, zone->GetZoneID());
		CheckedTouchActivities = true;
	}
}

#if 0
void Client::SendTaskComplete(int TaskIndex) {

	// 0x4c8c

	TaskComplete_Struct* tcs;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskComplete, sizeof(TaskComplete_Struct));

	tcs = (TaskComplete_Struct*)outapp->pBuffer;

	// I have seen unknown0 as non-zero. It always seems to match the value in the first word of the
	// Task Activity Complete packet sent immediately prior to it.
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

void ClientTaskState::SendTaskHistory(Client *c, int TaskIndex) {

	Log(Logs::General, Logs::Tasks, "[UPDATE] Task History Requested for Completed Task Index %i", TaskIndex);

	// We only sent the most recent 50 completed tasks, so we need to offset the Index the client sent to us.

	int AdjustedTaskIndex = TaskIndex;

	if(CompletedTasks.size() > 50)
		AdjustedTaskIndex += (CompletedTasks.size() - 50);

	if((AdjustedTaskIndex < 0) || (AdjustedTaskIndex >= (int)CompletedTasks.size())) return;

	int TaskID = CompletedTasks[AdjustedTaskIndex].TaskID;

	if((TaskID < 0) || (TaskID > MAXTASKS)) return;

	TaskInformation* Task = taskmanager->Tasks[TaskID];

	if(Task == nullptr) return;

	TaskHistoryReplyHeader_Struct* ths;
	TaskHistoryReplyData1_Struct* thd1;
	TaskHistoryReplyData2_Struct* thd2;

	char *Ptr;

	int CompletedActivityCount = 0;;

	int PacketLength = sizeof(TaskHistoryReplyHeader_Struct);

	for(int i=0; i<Task->ActivityCount; i++) {
		if(CompletedTasks[AdjustedTaskIndex].ActivityDone[i]) {
			CompletedActivityCount++;
			PacketLength = PacketLength + sizeof(TaskHistoryReplyData1_Struct) +
								Task->Activity[i].target_name.size() + 1 +
								Task->Activity[i].item_list.size() + 1 +
								sizeof(TaskHistoryReplyData2_Struct) +
								Task->Activity[i].desc_override.size() + 1;
		}
	}

	auto outapp = new EQApplicationPacket(OP_TaskHistoryReply, PacketLength);

	ths = (TaskHistoryReplyHeader_Struct*)outapp->pBuffer;

	// We use the TaskIndex the client sent in the request
	ths->TaskID = TaskIndex;

	ths->ActivityCount = CompletedActivityCount;

	Ptr = (char *) ths + sizeof(TaskHistoryReplyHeader_Struct);

	for(int i=0; i<Task->ActivityCount; i++) {
		if(CompletedTasks[AdjustedTaskIndex].ActivityDone[i]) {
			thd1 = (TaskHistoryReplyData1_Struct*)Ptr;
			thd1->ActivityType = Task->Activity[i].Type;
			Ptr = (char *)thd1 + sizeof(TaskHistoryReplyData1_Struct);
			VARSTRUCT_ENCODE_STRING(Ptr, Task->Activity[i].target_name.c_str());
			VARSTRUCT_ENCODE_STRING(Ptr, Task->Activity[i].item_list.c_str());
			thd2 = (TaskHistoryReplyData2_Struct*)Ptr;
			thd2->GoalCount = Task->Activity[i].GoalCount;
			thd2->unknown04 = 0xffffffff;
			thd2->unknown08 = 0xffffffff;
			thd2->ZoneID = Task->Activity[i].ZoneIDs.empty() ? 0 : Task->Activity[i].ZoneIDs.front();
			thd2->unknown16 = 0x00000000;
			Ptr = (char *)thd2 + sizeof(TaskHistoryReplyData2_Struct);
			VARSTRUCT_ENCODE_STRING(Ptr, Task->Activity[i].desc_override.c_str());
		}
	}


	c->QueuePacket(outapp);
	safe_delete(outapp);



}

void Client::SendTaskActivityComplete(int TaskID, int ActivityID, int TaskIndex, TaskType type, int TaskIncomplete)
{
	// 0x54eb

	TaskActivityComplete_Struct* tac;

	auto outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	tac = (TaskActivityComplete_Struct*)outapp->pBuffer;

	tac->TaskIndex = TaskIndex;
	tac->TaskType = static_cast<uint32>(type);
	tac->TaskID = TaskID;
	tac->ActivityID = ActivityID;
	tac->task_completed = 0x00000001;
	tac->stage_complete = TaskIncomplete;


	QueuePacket(outapp);
	safe_delete(outapp);
}


void Client::SendTaskFailed(int TaskID, int TaskIndex, TaskType type)
{
	// 0x54eb
	char buf[24];
	snprintf(buf, 23, "%d", TaskID);
	buf[23] = '\0';
	parse->EventPlayer(EVENT_TASK_FAIL, this, buf, 0);

	TaskActivityComplete_Struct* tac;

	auto outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	tac = (TaskActivityComplete_Struct*)outapp->pBuffer;

	tac->TaskIndex = TaskIndex;
	tac->TaskType = static_cast<uint32>(type);
	tac->TaskID = TaskID;
	tac->ActivityID = 0;
	tac->task_completed = 0; //Fail
	tac->stage_complete = 0; // 0 for task complete or failed.

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskFailed");

	QueuePacket(outapp);
	safe_delete(outapp);
}

void TaskManager::SendCompletedTasksToClient(Client *c, ClientTaskState *State) {

	int PacketLength = 4;



	//vector<CompletedTaskInformation>::const_iterator iterator;

	// The client only display the first 50 Completed Tasks send, so send the 50 most recent
	int FirstTaskToSend = 0;
	int LastTaskToSend = State->CompletedTasks.size();

	if(State->CompletedTasks.size() > 50)
		FirstTaskToSend = State->CompletedTasks.size() - 50;

	Log(Logs::General, Logs::Tasks, "[UPDATE] Completed Task Count: %i, First Task to send is %i, Last is %i",
				State->CompletedTasks.size(), FirstTaskToSend, LastTaskToSend);
	/*
	for(iterator=State->CompletedTasks.begin(); iterator!=State->CompletedTasks.end(); iterator++) {
		int TaskID = (*iterator).TaskID;
		if(Tasks[TaskID] == nullptr) continue;
		PacketLength = PacketLength + 8 + strlen(Tasks[TaskID]->Title) + 1;
	}
	*/
	for(int i = FirstTaskToSend; i<LastTaskToSend; i++) {
		int TaskID = State->CompletedTasks[i].TaskID;
		if(Tasks[TaskID] == nullptr) continue;
		PacketLength = PacketLength + 8 + Tasks[TaskID]->Title.size() + 1;
	}

	auto outapp = new EQApplicationPacket(OP_CompletedTasks, PacketLength);
	char *buf = (char*)outapp->pBuffer;

	//*(uint32 *)buf = State->CompletedTasks.size();
	*(uint32 *)buf = LastTaskToSend - FirstTaskToSend;
	buf = buf + 4;
	//for(iterator=State->CompletedTasks.begin(); iterator!=State->CompletedTasks.end(); iterator++) {
	//	int TaskID = (*iterator).TaskID;
	for(int i = FirstTaskToSend; i<LastTaskToSend; i++) {
		int TaskID = State->CompletedTasks[i].TaskID;
		if(Tasks[TaskID] == nullptr) continue;
		*(uint32 *)buf = TaskID;
		buf = buf + 4;

		sprintf(buf, "%s", Tasks[TaskID]->Title.c_str());
		buf = buf + strlen(buf) + 1;
		//*(uint32 *)buf = (*iterator).CompletedTime;
		*(uint32 *)buf = State->CompletedTasks[i].CompletedTime;
		buf = buf + 4;
	}


	c->QueuePacket(outapp);
	safe_delete(outapp);
}



void TaskManager::SendTaskActivityShort(Client *c, int TaskID, int ActivityID, int ClientTaskIndex)
{
	// This Activity Packet is sent for activities that have not yet been unlocked and appear as ???
	// in the client.

	TaskActivityShort_Struct* tass;

	if (c->ClientVersionBit() & EQ::versions::maskRoFAndLater)
	{
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

	tass = (TaskActivityShort_Struct*)outapp->pBuffer;

	tass->TaskSequenceNumber = ClientTaskIndex;
	tass->unknown2 = static_cast<uint32>(Tasks[TaskID]->type);
	tass->TaskID = TaskID;
	tass->ActivityID = ActivityID;
	tass->unknown3 = 0x000000;
	tass->ActivityType = 0xffffffff;
	tass->unknown4 = 0x00000000;


	c->QueuePacket(outapp);
	safe_delete(outapp);
}



void TaskManager::SendTaskActivityLong(Client *c, int TaskID, int ActivityID, int ClientTaskIndex, bool Optional, bool TaskComplete) {

	if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF)
	{
		SendTaskActivityNew(c, TaskID, ActivityID, ClientTaskIndex, Optional, TaskComplete);
		return;
	}

	SerializeBuffer buf(100);

	buf.WriteUInt32(ClientTaskIndex);
	buf.WriteUInt32(static_cast<uint32>(Tasks[TaskID]->type));
	buf.WriteUInt32(TaskID);
	buf.WriteUInt32(ActivityID);
	buf.WriteUInt32(0); // unknown3

	// We send our 'internal' types as ActivityCastOn. text3 should be set to the activity description, so it makes
	// no difference to the client. All activity updates will be done based on our interal activity types.
	if((Tasks[TaskID]->Activity[ActivityID].Type > 0) && Tasks[TaskID]->Activity[ActivityID].Type < 100)
		buf.WriteUInt32(Tasks[TaskID]->Activity[ActivityID].Type);
	else
		buf.WriteUInt32(ActivityCastOn); // w/e!

	buf.WriteUInt32(Optional);
	buf.WriteUInt32(0);		// solo, group, raid

	buf.WriteString(Tasks[TaskID]->Activity[ActivityID].target_name); // target name string
	buf.WriteString(Tasks[TaskID]->Activity[ActivityID].item_list); // item name list

	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(Tasks[TaskID]->Activity[ActivityID].GoalCount);
	else
		// For our internal type GiveCash, where the goal count has the amount of cash that must be given,
		// we don't want the donecount and goalcount fields cluttered up with potentially large numbers, so we just
		// send a goalcount of 1, and a bit further down, a donecount of 1 if the activity is complete, 0 otherwise.
		// The text3 field should decribe the exact activity goal, e.g. give 3500gp to Hasten Bootstrutter.
		buf.WriteUInt32(1);

	buf.WriteUInt32(Tasks[TaskID]->Activity[ActivityID].skill_id);
	buf.WriteUInt32(Tasks[TaskID]->Activity[ActivityID].spell_id);
	buf.WriteUInt32(Tasks[TaskID]->Activity[ActivityID].ZoneIDs.empty() ? 0 : Tasks[TaskID]->Activity[ActivityID].ZoneIDs.front());
	buf.WriteUInt32(0);

	buf.WriteString(Tasks[TaskID]->Activity[ActivityID].desc_override);

	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID));
	else
		// For internal activity types, DoneCount is either 1 if the activity is complete, 0 otherwise.
		buf.WriteUInt32((c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID) >= Tasks[TaskID]->Activity[ActivityID].GoalCount));

	buf.WriteUInt32(1); // unknown

	auto outapp = new EQApplicationPacket(OP_TaskActivity, buf);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

// Used only by RoF+ Clients
void TaskManager::SendTaskActivityNew(Client *c, int TaskID, int ActivityID, int ClientTaskIndex, bool Optional, bool TaskComplete)
{
	SerializeBuffer buf(100);

	buf.WriteUInt32(ClientTaskIndex);	// TaskSequenceNumber
	buf.WriteUInt32(static_cast<uint32>(Tasks[TaskID]->type));		// task type
	buf.WriteUInt32(TaskID);
	buf.WriteUInt32(ActivityID);
	buf.WriteUInt32(0);		// unknown3

	// We send our 'internal' types as ActivityCastOn. text3 should be set to the activity description, so it makes
	// no difference to the client. All activity updates will be done based on our interal activity types.
	if((Tasks[TaskID]->Activity[ActivityID].Type > 0) && Tasks[TaskID]->Activity[ActivityID].Type < 100)
		buf.WriteUInt32(Tasks[TaskID]->Activity[ActivityID].Type);
	else
		buf.WriteUInt32(ActivityCastOn); // w/e!

	buf.WriteUInt8(Optional);
	buf.WriteUInt32(0);		// solo, group, raid

	// One of these unknown fields maybe related to the 'Use On' activity types
	buf.WriteString(Tasks[TaskID]->Activity[ActivityID].target_name); // target name string

	buf.WriteLengthString(Tasks[TaskID]->Activity[ActivityID].item_list); // item name list

	// Goal Count
	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(Tasks[TaskID]->Activity[ActivityID].GoalCount);
	else
		buf.WriteUInt32(1);	// GoalCount

	// skill ID list ; separated
	buf.WriteLengthString(Tasks[TaskID]->Activity[ActivityID].skill_list);

	// spelll ID list ; separated -- unsure wtf we're doing here
	buf.WriteLengthString(Tasks[TaskID]->Activity[ActivityID].spell_list);

	buf.WriteString(Tasks[TaskID]->Activity[ActivityID].zones);
	buf.WriteUInt32(0);		// unknown7

	buf.WriteString(Tasks[TaskID]->Activity[ActivityID].desc_override); // description override

	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		buf.WriteUInt32(c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID));	// DoneCount
	else
		// For internal activity types, DoneCount is either 1 if the activity is complete, 0 otherwise.
		buf.WriteUInt32((c->GetTaskActivityDoneCount(Tasks[TaskID]->type, ClientTaskIndex, ActivityID) >= Tasks[TaskID]->Activity[ActivityID].GoalCount));

	buf.WriteUInt8(1);	// unknown9

	buf.WriteString(Tasks[TaskID]->Activity[ActivityID].zones);

	auto outapp = new EQApplicationPacket(OP_TaskActivity, buf);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

void TaskManager::SendActiveTasksToClient(Client *c, bool TaskComplete)
{
	auto state = c->GetTaskState();
	if (!state)
		return;

	for (int TaskIndex = 0; TaskIndex < MAXACTIVEQUESTS + 1; TaskIndex++) {
		int TaskID = state->ActiveTasks[TaskIndex].TaskID;
		if ((TaskID == 0) || (Tasks[TaskID] == 0))
			continue;
		int StartTime = state->ActiveTasks[TaskIndex].AcceptedTime;

		SendActiveTaskDescription(c, TaskID, state->ActiveTasks[TaskIndex], StartTime, Tasks[TaskID]->Duration,
					  false);
		Log(Logs::General, Logs::Tasks, "[UPDATE] SendActiveTasksToClient: Task %i, Activities: %i", TaskID,
		    GetActivityCount(TaskID));

		int Sequence = 0;
		int fixed_index = Tasks[TaskID]->type == TaskType::Task ? 0 : TaskIndex - 1; // hmmm fuck
		for (int Activity = 0; Activity < GetActivityCount(TaskID); Activity++) {
			if (c->GetTaskActivityState(Tasks[TaskID]->type, fixed_index, Activity) != ActivityHidden) {
				Log(Logs::General, Logs::Tasks, "[UPDATE]   Long: %i, %i, %i Complete=%i", TaskID,
				    Activity, fixed_index, TaskComplete);
				if (Activity == GetActivityCount(TaskID) - 1)
					SendTaskActivityLong(c, TaskID, Activity, fixed_index,
							     Tasks[TaskID]->Activity[Activity].Optional, TaskComplete);
				else
					SendTaskActivityLong(c, TaskID, Activity, fixed_index,
							     Tasks[TaskID]->Activity[Activity].Optional, 0);
			} else {
				Log(Logs::General, Logs::Tasks, "[UPDATE]   Short: %i, %i, %i", TaskID, Activity,
				    fixed_index);
				SendTaskActivityShort(c, TaskID, Activity, fixed_index);
			}
			Sequence++;
		}
	}
}

void TaskManager::SendSingleActiveTaskToClient(Client *c, ClientTaskInformation &task_info, bool TaskComplete,
					       bool BringUpTaskJournal)
{
	int TaskID = task_info.TaskID;

	if (TaskID == 0 || Tasks[TaskID] == nullptr)
		return;

	int StartTime = task_info.AcceptedTime;
	SendActiveTaskDescription(c, TaskID, task_info, StartTime, Tasks[TaskID]->Duration, BringUpTaskJournal);
	Log(Logs::General, Logs::Tasks, "[UPDATE] SendSingleActiveTasksToClient: Task %i, Activities: %i", TaskID, GetActivityCount(TaskID));

	for (int Activity = 0; Activity < GetActivityCount(TaskID); Activity++) {
		if(task_info.Activity[Activity].State != ActivityHidden) {
			Log(Logs::General, Logs::Tasks, "[UPDATE]   Long: %i, %i Complete=%i", TaskID, Activity, TaskComplete);
			if (Activity == GetActivityCount(TaskID) - 1)
				SendTaskActivityLong(c, TaskID, Activity, task_info.slot,
							Tasks[TaskID]->Activity[Activity].Optional, TaskComplete);
			else
				SendTaskActivityLong(c, TaskID, Activity, task_info.slot,
							Tasks[TaskID]->Activity[Activity].Optional, 0);
		} else {
			Log(Logs::General, Logs::Tasks, "[UPDATE]   Short: %i, %i", TaskID, Activity);
			SendTaskActivityShort(c, TaskID, Activity, task_info.slot);
		}
	}
}

void TaskManager::SendActiveTaskDescription(Client *c, int TaskID, ClientTaskInformation &task_info, int StartTime, int Duration, bool BringUpTaskJournal)
{
	if ((TaskID < 1) || (TaskID >= MAXTASKS) || !Tasks[TaskID])
		return;

	int PacketLength = sizeof(TaskDescriptionHeader_Struct) + Tasks[TaskID]->Title.length() + 1
				+ sizeof(TaskDescriptionData1_Struct) + Tasks[TaskID]->Description.length() + 1
				+ sizeof(TaskDescriptionData2_Struct) + 1 + sizeof(TaskDescriptionTrailer_Struct);

	// If there is an item make the Reward text into a link to the item (only the first item if a list
	// is specified). I have been unable to get multiple item links to work.
	//
	if(Tasks[TaskID]->RewardID && Tasks[TaskID]->item_link.empty()) {
		int ItemID = 0;
		// If the reward is a list of items, and the first entry on the list is valid
		if (Tasks[TaskID]->RewardMethod == METHODSINGLEID) {
			ItemID = Tasks[TaskID]->RewardID;
		} else if (Tasks[TaskID]->RewardMethod == METHODLIST) {
			ItemID = GoalListManager.GetFirstEntry(Tasks[TaskID]->RewardID);
			if (ItemID < 0)
				ItemID = 0;
		}

		if(ItemID) {
			const EQ::ItemData* reward_item = database.GetItem(ItemID);

			EQ::SayLinkEngine linker;
			linker.SetLinkType(EQ::saylink::SayLinkItemData);
			linker.SetItemData(reward_item);
			linker.SetTaskUse();
			Tasks[TaskID]->item_link = linker.GenerateLink();
		}
	}

	PacketLength += Tasks[TaskID]->Reward.length() + 1 + Tasks[TaskID]->item_link.length() + 1;

	char *Ptr;
	TaskDescriptionHeader_Struct* tdh;
	TaskDescriptionData1_Struct* tdd1;
	TaskDescriptionData2_Struct* tdd2;
	TaskDescriptionTrailer_Struct* tdt;

	auto outapp = new EQApplicationPacket(OP_TaskDescription, PacketLength);

	tdh = (TaskDescriptionHeader_Struct*)outapp->pBuffer;

	tdh->SequenceNumber = task_info.slot;
	tdh->TaskID = TaskID;
	tdh->open_window = BringUpTaskJournal;
	tdh->task_type = static_cast<uint32>(Tasks[TaskID]->type);
	tdh->reward_type = 0; // TODO: 4 says Radiant Crystals else Ebon Crystals when shared task

	Ptr = (char *) tdh + sizeof(TaskDescriptionHeader_Struct);

	sprintf(Ptr, "%s", Tasks[TaskID]->Title.c_str());
	Ptr += Tasks[TaskID]->Title.length() + 1;

	tdd1 = (TaskDescriptionData1_Struct*)Ptr;

	tdd1->Duration = Duration;
	tdd1->dur_code = static_cast<uint32>(Tasks[TaskID]->dur_code);

	tdd1->StartTime = StartTime;

	Ptr = (char *) tdd1 + sizeof(TaskDescriptionData1_Struct);

	sprintf(Ptr, "%s", Tasks[TaskID]->Description.c_str());
	Ptr += Tasks[TaskID]->Description.length() + 1;

	tdd2 = (TaskDescriptionData2_Struct*)Ptr;

	// we have this reward stuff!
	// if we ever don't hardcode this, TaskDescriptionTrailer_Struct will need to be fixed since
	// "has_reward_selection" is after this bool! Smaller packet when this is 0
	tdd2->has_rewards = 1;

	tdd2->coin_reward = Tasks[TaskID]->CashReward;
	tdd2->xp_reward = Tasks[TaskID]->XPReward ? 1 : 0; // just booled
	tdd2->faction_reward = Tasks[TaskID]->faction_reward ? 1 : 0; // faction booled

	Ptr = (char *) tdd2 + sizeof(TaskDescriptionData2_Struct);

	// we actually have 2 strings here. One is max length 96 and not parsed for item links
	// We actually skipped past that string incorrectly before, so TODO: fix item link string
	sprintf(Ptr, "%s", Tasks[TaskID]->Reward.c_str());
	Ptr += Tasks[TaskID]->Reward.length() + 1;

	// second string is parsed for item links
	sprintf(Ptr, "%s", Tasks[TaskID]->item_link.c_str());
	Ptr += Tasks[TaskID]->item_link.length() + 1;

	tdt = (TaskDescriptionTrailer_Struct*)Ptr;
	tdt->Points = 0x00000000; // Points Count TODO: this does have a visible affect on the client ...
	tdt->has_reward_selection = 0; // TODO: new rewards window

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

bool ClientTaskState::IsTaskActivityCompleted(TaskType type, int index, int ActivityID)
{
	switch (type) {
	case TaskType::Task:
		if (index != 0)
			return false;
		return ActiveTask.Activity[ActivityID].State == ActivityCompleted;
	case TaskType::Shared:
		return false; // TODO: shared tasks
	case TaskType::Quest:
		if (index < MAXACTIVEQUESTS)
			return ActiveQuests[index].Activity[ActivityID].State == ActivityCompleted;
	default:
		return false;
	}

}

// should we be defaulting to hidden?
ActivityState ClientTaskState::GetTaskActivityState(TaskType type, int index, int ActivityID)
{
	switch (type) {
	case TaskType::Task:
		if (index != 0)
			return ActivityHidden;
		return ActiveTask.Activity[ActivityID].State;
	case TaskType::Shared:
		return ActivityHidden; // TODO: shared tasks
	case TaskType::Quest:
		if (index < MAXACTIVEQUESTS)
			return ActiveQuests[index].Activity[ActivityID].State;
	default:
		return ActivityHidden;
	}
}

int ClientTaskState::GetTaskActivityDoneCount(TaskType type, int index, int ActivityID)
{
	switch (type) {
	case TaskType::Task:
		if (index != 0)
			return 0;
		return ActiveTask.Activity[ActivityID].DoneCount;
	case TaskType::Shared:
		return 0; // TODO: shared tasks
	case TaskType::Quest:
		if (index < MAXACTIVEQUESTS)
			return ActiveQuests[index].Activity[ActivityID].DoneCount;
	default:
		return 0;
	}
}

int ClientTaskState::GetTaskActivityDoneCountFromTaskID(int TaskID, int ActivityID)
{
	if (ActiveTask.TaskID == TaskID)
		return ActiveTask.Activity[ActivityID].DoneCount;

	// TODO: shared tasks

	int ActiveTaskIndex = -1;
	for(int i=0; i<MAXACTIVEQUESTS; i++) {
		if(ActiveQuests[i].TaskID==TaskID) {
			ActiveTaskIndex = i;
			break;
		}
	}

	if (ActiveTaskIndex == -1)
		return 0;

	if (ActiveQuests[ActiveTaskIndex].Activity[ActivityID].DoneCount){
		return ActiveQuests[ActiveTaskIndex].Activity[ActivityID].DoneCount;
	}
	else{
		return 0;
	}
}

int ClientTaskState::GetTaskStartTime(TaskType type, int index)
{
	switch (type) {
	case TaskType::Task:
		return ActiveTask.AcceptedTime;
	case TaskType::Quest:
		return ActiveQuests[index].AcceptedTime;
	case TaskType::Shared: // TODO
	default:
		return -1;
	}
}

void ClientTaskState::CancelAllTasks(Client *c) {

	// This method exists solely to be called during #task reloadall
	// It removes tasks from the in-game client state ready for them to be
	// resent to the client, in case an updated task fails to load

	CancelTask(c, 0, TaskType::Task, false);
	ActiveTask.TaskID = TASKSLOTEMPTY;

	for(int i=0; i<MAXACTIVEQUESTS; i++)
		if(ActiveQuests[i].TaskID != TASKSLOTEMPTY) {
			CancelTask(c, i, TaskType::Quest, false);
			ActiveQuests[i].TaskID = TASKSLOTEMPTY;
		}

	// TODO: shared
}

void ClientTaskState::CancelTask(Client *c, int SequenceNumber, TaskType type, bool RemoveFromDB)
{
	auto outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));

	CancelTask_Struct* cts = (CancelTask_Struct*)outapp->pBuffer;
	cts->SequenceNumber = SequenceNumber;
	cts->type = static_cast<uint32>(type);

	Log(Logs::General, Logs::Tasks, "[UPDATE] CancelTask");

	c->QueuePacket(outapp);
	safe_delete(outapp);

	if(RemoveFromDB)
		RemoveTask(c, SequenceNumber, type);
}

void ClientTaskState::RemoveTask(Client *c, int sequenceNumber, TaskType type)
{
	int characterID = c->CharacterID();
	Log(Logs::General, Logs::Tasks, "[UPDATE] ClientTaskState Cancel Task %i ", sequenceNumber);

	int task_id = -1;
	switch (type) {
	case TaskType::Task:
		if (sequenceNumber == 0)
			task_id = ActiveTask.TaskID;
		break;
	case TaskType::Quest:
		if (sequenceNumber < MAXACTIVEQUESTS)
			task_id = ActiveQuests[sequenceNumber].TaskID;
		break;
	case TaskType::Shared: // TODO:
	default:
		break;
	}

	std::string query = StringFormat("DELETE FROM character_activities WHERE charid=%i AND taskid = %i",
					 characterID, task_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("[TASKS] Error in CientTaskState::CancelTask [{}]",
		    results.ErrorMessage().c_str());
		return;
	}
	Log(Logs::General, Logs::Tasks, "[UPDATE] CancelTask: %s", query.c_str());

	query = StringFormat("DELETE FROM character_tasks WHERE charid=%i AND taskid = %i AND type=%i", characterID,
			     task_id, static_cast<int>(type));
	results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("[TASKS] Error in CientTaskState::CancelTask [{}]",
		    results.ErrorMessage().c_str());

	Log(Logs::General, Logs::Tasks, "[UPDATE] CancelTask: %s", query.c_str());

	switch (type) {
	case TaskType::Task:
		ActiveTask.TaskID = TASKSLOTEMPTY;
		break;
	case TaskType::Shared:
		break; // TODO: shared tasks
	case TaskType::Quest:
		ActiveQuests[sequenceNumber].TaskID = TASKSLOTEMPTY;
		ActiveTaskCount--;
		break;
	default:
		break;
	}
}

void ClientTaskState::AcceptNewTask(Client *c, int TaskID, int NPCID, bool enforce_level_requirement)
{
	if (!taskmanager || TaskID < 0 || TaskID >= MAXTASKS) {
		c->Message(Chat::Red, "Task system not functioning, or TaskID %i out of range.", TaskID);
		return;
	}

	auto task = taskmanager->Tasks[TaskID];

	if (task == nullptr) {
		c->Message(Chat::Red, "Invalid TaskID %i", TaskID);
		return;
	}

	bool max_tasks = false;

	switch (task->type) {
	case TaskType::Task:
		if (ActiveTask.TaskID != TASKSLOTEMPTY)
			max_tasks = true;
		break;
	case TaskType::Shared: // TODO: shared tasks
		// if (something)
			max_tasks = true;
		break;
	case TaskType::Quest:
		if (ActiveTaskCount == MAXACTIVEQUESTS)
			max_tasks = true;
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
			if (ActiveQuests[i].TaskID == TaskID) {
				c->Message(Chat::Red, "You have already been assigned this task.");
				return;
			}
		}
	}

	if (enforce_level_requirement && !taskmanager->AppropriateLevel(TaskID, c->GetLevel())) {
		c->Message(Chat::Red, "You are outside the level range of this task.");
		return;
	}

	if (!taskmanager->IsTaskRepeatable(TaskID) && IsTaskCompleted(TaskID))
		return;

	// We do it this way, because when the Client cancels a task, it retains the sequence number of the remaining
	// tasks in it's window, until something causes the TaskDescription packets to be sent again. We could just
	// resend all the active task data to the client when it cancels a task, but that could be construed as a
	// waste of bandwidth.
	//
	ClientTaskInformation *active_slot = nullptr;
	switch (task->type) {
	case TaskType::Task:
		active_slot = &ActiveTask;
		break;
	case TaskType::Shared: // TODO: shared
		active_slot = nullptr;
		break;
	case TaskType::Quest:
		for (int i = 0; i < MAXACTIVEQUESTS; i++) {
			Log(Logs::General, Logs::Tasks,
			    "[UPDATE] ClientTaskState Looking for free slot in slot %i, found TaskID of %i", i,
			    ActiveQuests[i].TaskID);
			if (ActiveQuests[i].TaskID == 0) {
				active_slot = &ActiveQuests[i];
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

	active_slot->TaskID = TaskID;
	active_slot->AcceptedTime = time(nullptr);
	active_slot->Updated = true;
	active_slot->CurrentStep = -1;

	for (int i = 0; i < taskmanager->Tasks[TaskID]->ActivityCount; i++) {
		active_slot->Activity[i].ActivityID = i;
		active_slot->Activity[i].DoneCount = 0;
		active_slot->Activity[i].State = ActivityHidden;
		active_slot->Activity[i].Updated = true;
	}

	UnlockActivities(c->CharacterID(), *active_slot);

	if (task->type == TaskType::Quest)
		ActiveTaskCount++;

	taskmanager->SendSingleActiveTaskToClient(c, *active_slot, false, true);
	c->Message(Chat::White, "You have been assigned the task '%s'.", taskmanager->Tasks[TaskID]->Title.c_str());

	std::string buf = std::to_string(TaskID);

	NPC *npc = entity_list.GetID(NPCID)->CastToNPC();
	if(!npc) {
		c->Message(Chat::Yellow, "Task Giver ID is %i", NPCID);
		c->Message(Chat::Red, "Unable to find NPC to send EVENT_TASKACCEPTED to. Report this bug.");
		return;
	}

	taskmanager->SaveClientState(c, this);
	parse->EventNPC(EVENT_TASK_ACCEPTED, npc, c, buf.c_str(), 0);
}

void ClientTaskState::ProcessTaskProximities(Client *c, float X, float Y, float Z) {

	float LastX = c->ProximityX();
	float LastY = c->ProximityY();
	float LastZ = c->ProximityZ();

	if((LastX==X) && (LastY==Y) && (LastZ==Z)) return;

	Log(Logs::General, Logs::Tasks, "[PROXIMITY] Checking proximities for Position %8.3f, %8.3f, %8.3f", X, Y, Z);
	int ExploreID = taskmanager->ProximityManager.CheckProximities(X, Y, Z);

	if(ExploreID > 0) {
		Log(Logs::General, Logs::Tasks, "[PROXIMITY] Position %8.3f, %8.3f, %8.3f is within proximity %i", X, Y, Z, ExploreID);
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

	std::string query = "SELECT `listid`, COUNT(`entry`) "
			    "FROM `goallists` GROUP by `listid` "
			    "ORDER BY `listid`";
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	NumberOfLists = results.RowCount();
	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] Database returned a count of %i lists", NumberOfLists);

	TaskGoalLists.reserve(NumberOfLists);

	int listIndex = 0;

	for (auto row = results.begin(); row != results.end(); ++row) {
		int listID = atoi(row[0]);
		int listSize = atoi(row[1]);
		TaskGoalLists.push_back({listID, 0, 0});

		TaskGoalLists[listIndex].GoalItemEntries.reserve(listSize);

		listIndex++;
	}

	for (int listIndex = 0; listIndex < NumberOfLists; listIndex++) {

		int listID = TaskGoalLists[listIndex].ListID;
		auto size = TaskGoalLists[listIndex].GoalItemEntries.capacity(); // this was only done for manual memory management, shouldn't need to do this
		query = StringFormat("SELECT `entry` from `goallists` "
				     "WHERE `listid` = %i "
				     "ORDER BY `entry` ASC LIMIT %i",
				     listID, size);
		results = database.QueryDatabase(query);
		if (!results.Success()) {
			continue;
		}

		for (auto row = results.begin(); row != results.end(); ++row) {

			int entry = atoi(row[0]);

			if (entry < TaskGoalLists[listIndex].Min)
				TaskGoalLists[listIndex].Min = entry;

			if (entry > TaskGoalLists[listIndex].Max)
				TaskGoalLists[listIndex].Max = entry;

			TaskGoalLists[listIndex].GoalItemEntries.push_back(entry);
		}
	}

	return true;

}

int TaskGoalListManager::GetListByID(int ListID) {

	// Find the list with the specified ListID and return the index
	auto it = std::find_if(TaskGoalLists.begin(), TaskGoalLists.end(),
			       [ListID](const TaskGoalList_Struct &t) { return t.ListID == ListID; });

	if (it == TaskGoalLists.end())
		return -1;

	return std::distance(TaskGoalLists.begin(), it);
}

int TaskGoalListManager::GetFirstEntry(int ListID) {

	int ListIndex = GetListByID(ListID);

	if((ListIndex < 0) || (ListIndex >= NumberOfLists)) return -1;

	if(TaskGoalLists[ListIndex].GoalItemEntries.empty()) return -1;

	return TaskGoalLists[ListIndex].GoalItemEntries[0];
}

std::vector<int> TaskGoalListManager::GetListContents(int ListID) {

	std::vector<int> ListContents;

	int ListIndex = GetListByID(ListID);

	if((ListIndex < 0) || (ListIndex >= NumberOfLists)) return ListContents;

	ListContents = TaskGoalLists[ListIndex].GoalItemEntries;

	return ListContents;

}

bool TaskGoalListManager::IsInList(int ListID, int Entry)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i)", ListID, Entry);

	int ListIndex = GetListByID(ListID);

	if ((ListIndex < 0) || (ListIndex >= NumberOfLists))
		return false;

	if ((Entry < TaskGoalLists[ListIndex].Min) || (Entry > TaskGoalLists[ListIndex].Max))
		return false;

	int FirstEntry = 0;
	auto &task = TaskGoalLists[ListIndex];

	auto it = std::find(task.GoalItemEntries.begin(), task.GoalItemEntries.end(), Entry);

	if (it == task.GoalItemEntries.end())
		return false;

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i) returning true", ListIndex,
	    Entry);
	return true;
}

TaskProximityManager::TaskProximityManager() {


}

TaskProximityManager::~TaskProximityManager() {


}

bool TaskProximityManager::LoadProximities(int zoneID) {
	TaskProximity proximity;

	Log(Logs::General, Logs::Tasks, "[GLOBALLOAD] TaskProximityManager::LoadProximities Called for zone %i", zoneID);
	TaskProximities.clear();

    std::string query = StringFormat("SELECT `exploreid`, `minx`, `maxx`, "
                                    "`miny`, `maxy`, `minz`, `maxz` "
                                    "FROM `proximities` WHERE `zoneid` = %i "
                                    "ORDER BY `zoneid` ASC", zoneID);
    auto results = database.QueryDatabase(query);
    if (!results.Success()) {
		return false;
    }

	for( auto row = results.begin(); row != results.end(); ++row) {
        proximity.ExploreID = atoi(row[0]);
        proximity.MinX = atof(row[1]);
        proximity.MaxX = atof(row[2]);
        proximity.MinY = atof(row[3]);
        proximity.MaxY = atof(row[4]);
        proximity.MinZ = atof(row[5]);
        proximity.MaxZ = atof(row[6]);

        TaskProximities.push_back(proximity);
    }

	return true;

}

int TaskProximityManager::CheckProximities(float X, float Y, float Z) {

	for(unsigned int i=0; i<TaskProximities.size(); i++) {

		TaskProximity* P = &TaskProximities[i];

		Log(Logs::General, Logs::Tasks, "[PROXIMITY] Checking %8.3f, %8.3f, %8.3f against %8.3f, %8.3f, %8.3f, %8.3f, %8.3f, %8.3f",
				X, Y, Z, P->MinX, P->MaxX, P->MinY, P->MaxY, P->MinZ, P->MaxZ);

		if(X < P->MinX || X > P->MaxX || Y < P->MinY || Y > P->MaxY ||
			Z < P->MinZ || Z > P->MaxZ) continue;

		return P->ExploreID;

	}

	return 0;
}

