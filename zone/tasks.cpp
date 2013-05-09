
/*  EQEMu:  Everquest Server Emulator
Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../common/debug.h"
#include "tasks.h"

#include <string.h>
#include <stdlib.h>

#ifdef _WINDOWS
#define strcasecmp _stricmp
#endif

#include "../common/MiscFunctions.h"
#include "../common/rulesys.h"
#include "masterentity.h"
#include "../common/features.h"
#include "QuestParserCollection.h"


TaskManager::TaskManager() {

	for(int i=0; i<MAXTASKS; i++)
		Tasks[i] = nullptr;


}

TaskManager::~TaskManager() {

	for(int i=0; i<MAXTASKS; i++) {
		if(Tasks[i] != nullptr) {
			for(int j=0; j<Tasks[i]->ActivityCount; j++) {
				safe_delete_array(Tasks[i]->Activity[j].Text1);
				safe_delete_array(Tasks[i]->Activity[j].Text2);
				safe_delete_array(Tasks[i]->Activity[j].Text3);
			}
			safe_delete_array(Tasks[i]->Title);
			safe_delete_array(Tasks[i]->Description);
			safe_delete_array(Tasks[i]->Reward);
			safe_delete(Tasks[i]);
		}
	}
}

bool TaskManager::LoadTaskSets() {


	const char *TaskSetQuery = "SELECT `id`, `taskid` from `tasksets` WHERE `id` > 0 AND `id` < %i "
				   "AND `taskid` >= 0 AND `taskid` < %i ORDER BY `id`, `taskid` ASC";

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::LoadTaskSets: %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	// Clear all task sets in memory. Done so we can reload them on the fly if required by just calling
	// this method again.
	for(int i=0; i<MAXTASKSETS; i++) {
		TaskSets[i].clear();
	}

	if(database.RunQuery(query,MakeAnyLenString(&query,TaskSetQuery,MAXTASKSETS,MAXTASKS),errbuf,&result)) {

		while((row = mysql_fetch_row(result))) {
			int TaskSet = atoi(row[0]);
			int TaskID = atoi(row[1]);

			TaskSets[TaskSet].push_back(TaskID);
			_log(TASKS__GLOBALLOAD, "Adding TaskID %4i to TaskSet %4i", TaskID, TaskSet);
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
		safe_delete_array(query);
		return false;
	}

	return true;

}

bool TaskManager::LoadSingleTask(int TaskID) {

	_log(TASKS__GLOBALLOAD, "TaskManager::LoadSingleTask(%i)", TaskID);

	if((TaskID <= 0) || (TaskID >= MAXTASKS)) return false;

	// If this task already exists in memory, free all the dynamically allocated strings.
	if(Tasks[TaskID]) {

		for(int j=0; j<Tasks[TaskID]->ActivityCount; j++) {
			safe_delete_array(Tasks[TaskID]->Activity[j].Text1);
			safe_delete_array(Tasks[TaskID]->Activity[j].Text2);
			safe_delete_array(Tasks[TaskID]->Activity[j].Text3);
		}
		safe_delete_array(Tasks[TaskID]->Title);
		safe_delete_array(Tasks[TaskID]->Description);
		safe_delete_array(Tasks[TaskID]->Reward);
		safe_delete(Tasks[TaskID]);
	}

	return LoadTasks(TaskID);
}
	
void TaskManager::ReloadGoalLists() {

	if(!GoalListManager.LoadLists()) 
		_log(TASKS__GLOBALLOAD,"TaskManager::LoadTasks LoadLists failed");
}

bool TaskManager::LoadTasks(int SingleTask) {

	// If TaskID !=0, then just load the task specified.

	const char *AllTaskQuery = "SELECT `id`, `duration`, `title`, `description`, `reward`, `rewardid`,"
				   "`cashreward`, `xpreward`, `rewardmethod`, `startzone`, `minlevel`, `maxlevel`, `repeatable` "
				   "from `tasks` WHERE `id` < %i";

	const char *SingleTaskQuery = "SELECT `id`, `duration`, `title`, `description`, `reward`, `rewardid`,"
				      "`cashreward`, `xpreward`, `rewardmethod`, `startzone`, `minlevel`, `maxlevel`, `repeatable` "
				      "from `tasks` WHERE `id` = %i";

	const char *AllActivityQuery = "SELECT `taskid`, `step`, `activityid`, `activitytype`, `text1`, `text2`,"
				       "`text3`, `goalid`, `goalmethod`, `goalcount`, `delivertonpc`, "
				       "`zoneid`, `optional` from `activities` WHERE "
				       "`taskid` < %i AND `activityid` < %i ORDER BY taskid, activityid ASC";

	const char *SingleTaskActivityQuery = "SELECT `taskid`, `step`, `activityid`, `activitytype`, `text1`, `text2`,"
				    	      "`text3`, `goalid`, `goalmethod`, `goalcount`, `delivertonpc`, "
				              "`zoneid`, `optional` from `activities` WHERE "
				              "`taskid` = %i AND `activityid` < %i ORDER BY taskid, activityid ASC";

	const char *ERR_TASK_OOR = "[TASKS]Task ID %i out of range while loading tasks from database";

	const char *ERR_TASK_OR_ACTIVITY_OOR = "[TASKS]Task or Activity ID (%i, %i) out of range while loading"
	                                       "activities from database";

	const char *ERR_NOTASK = "[TASKS]Activity for non-existent task (%i, %i) while loading activities from database";

	const char *ERR_SEQERR = "[TASKS]Activities for Task %i are not sequential starting at 0. Not loading task.";

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::LoadTasks: %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	int QueryLength = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	_log(TASKS__GLOBALLOAD, "TaskManager::LoadTasks Called"); 

	if(SingleTask == 0) {
		if(!GoalListManager.LoadLists()) 
			_log(TASKS__GLOBALLOAD,"TaskManager::LoadTasks LoadLists failed");

		if(!LoadTaskSets())
			_log(TASKS__GLOBALLOAD,"TaskManager::LoadTasks LoadTaskSets failed");

		QueryLength = MakeAnyLenString(&query,AllTaskQuery,MAXTASKS);
	}
	else
		QueryLength = MakeAnyLenString(&query,SingleTaskQuery,SingleTask);

	if(database.RunQuery(query,QueryLength,errbuf,&result)) {

		while((row = mysql_fetch_row(result))) {
			int TaskID = atoi(row[0]);
			if((TaskID <= 0) || (TaskID >= MAXTASKS)) {
				// This shouldn't happen, as the SELECT is bounded by MAXTASKS
				LogFile->write(EQEMuLog::Error, ERR_TASK_OOR, TaskID);
				continue;
			}
			Tasks[TaskID] = new TaskInformation;
			Tasks[TaskID]->Duration = atoi(row[1]);
			Tasks[TaskID]->Title = new char[strlen(row[2]) + 1];
			strcpy(Tasks[TaskID]->Title, row[2]);
			Tasks[TaskID]->Description = new char[strlen(row[3]) + 1];
			strcpy(Tasks[TaskID]->Description, row[3]);
			Tasks[TaskID]->Reward = new char[strlen(row[4]) + 1];
			strcpy(Tasks[TaskID]->Reward, row[4]);
			Tasks[TaskID]->RewardID = atoi(row[5]);
			Tasks[TaskID]->CashReward = atoi(row[6]);
			Tasks[TaskID]->XPReward = atoi(row[7]);
			Tasks[TaskID]->RewardMethod = (TaskMethodType)atoi(row[8]);
			Tasks[TaskID]->StartZone = atoi(row[9]);
			Tasks[TaskID]->MinLevel = atoi(row[10]);
			Tasks[TaskID]->MaxLevel = atoi(row[11]);
			Tasks[TaskID]->Repeatable = atoi(row[12]);
			Tasks[TaskID]->ActivityCount = 0;
			Tasks[TaskID]->SequenceMode = ActivitiesSequential;
			Tasks[TaskID]->LastStep = 0;

			_log(TASKS__GLOBALLOAD,"TaskID: %5i, Duration: %8i, StartZone: %3i Reward: %s MinLevel %i MaxLevel %i Repeatable: %s",
			       TaskID, Tasks[TaskID]->Duration, Tasks[TaskID]->StartZone, Tasks[TaskID]->Reward,
			       Tasks[TaskID]->MinLevel, Tasks[TaskID]->MaxLevel,
			       Tasks[TaskID]->Repeatable ? "Yes" : "No");
			_log(TASKS__GLOBALLOAD,"Title:         %s ", Tasks[TaskID]->Title);
			//_log(TASKS__GLOBALLOAD,"Description: %s ", Tasks[TaskID]->Description);

		}
		mysql_free_result(result);
		safe_delete_array(query);

	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
		safe_delete_array(query);
		return false;
	}

	if(SingleTask==0)
		QueryLength = MakeAnyLenString(&query,AllActivityQuery,MAXTASKS, MAXACTIVITIESPERTASK);
	else
		QueryLength = MakeAnyLenString(&query,SingleTaskActivityQuery, SingleTask, MAXACTIVITIESPERTASK);

	if(database.RunQuery(query,QueryLength, errbuf, &result)) {

		while((row = mysql_fetch_row(result))) {
			int TaskID = atoi(row[0]);
			int Step = atoi(row[1]);

			int ActivityID = atoi(row[2]);
			
			if((TaskID <= 0) || (TaskID >= MAXTASKS) || (ActivityID < 0) || (ActivityID >= MAXACTIVITIESPERTASK)) {
				// This shouldn't happen, as the SELECT is bounded by MAXTASKS
				LogFile->write(EQEMuLog::Error, ERR_TASK_OR_ACTIVITY_OOR, TaskID, ActivityID);
				continue;
			}
			if(Tasks[TaskID]==nullptr) {
				LogFile->write(EQEMuLog::Error, ERR_NOTASK, TaskID, ActivityID);
				continue;
			}
			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].StepNumber = Step;

			if(Step != 0)
				Tasks[TaskID]->SequenceMode = ActivitiesStepped;

			if(Step >Tasks[TaskID]->LastStep) Tasks[TaskID]->LastStep = Step;
			
			// Task Activities MUST be numbered sequentially from 0. If not, log an error
			// and set the task to nullptr. Subsequent activities for this task will raise
			// ERR_NOTASK errors.
			// Change to (ActivityID != (Tasks[TaskID]->ActivityCount + 1)) to index from 1
			if(ActivityID != Tasks[TaskID]->ActivityCount) {
				LogFile->write(EQEMuLog::Error, ERR_SEQERR, TaskID, ActivityID);
				Tasks[TaskID] = nullptr;
				continue;
			}

			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Type = atoi(row[3]);

			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text1 = new char[strlen(row[4]) + 1];

			if(strlen(row[4])>0)
				strcpy(Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text1, row[4]);
			else
				Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text1[0]=0;

			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text2 = new char[strlen(row[5]) + 1];

			if(strlen(row[5])>0)
				strcpy(Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text2, row[5]);
			else
				Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text2[0]=0;

			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text3 = new char[strlen(row[6]) + 1];

			if(strlen(row[6])>0)
				strcpy(Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text3, row[6]);
			else
				Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text3[0]=0;

			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].GoalID = atoi(row[7]);
			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].GoalMethod = (TaskMethodType)atoi(row[8]);
			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].GoalCount = atoi(row[9]);
			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].DeliverToNPC = atoi(row[10]);
			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].ZoneID = atoi(row[11]);
			Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Optional = atoi(row[12]);

			_log(TASKS__GLOBALLOAD, "Activity Slot %2i: ID %i for Task %5i. Type: %3i, GoalID: %8i, "
			       "GoalMethod: %i, GoalCount: %3i, ZoneID:%3i", 
			       Tasks[TaskID]->ActivityCount, ActivityID, TaskID,
			       Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Type,
			       Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].GoalID,
			       Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].GoalMethod,
			       Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].GoalCount,
			       Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].ZoneID);

			_log(TASKS__GLOBALLOAD, "          Text1: %s", 
			     Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text1);
			_log(TASKS__GLOBALLOAD, "          Text2: %s", 
			     Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text2);
			_log(TASKS__GLOBALLOAD, "          Text3: %s", 
			     Tasks[TaskID]->Activity[Tasks[TaskID]->ActivityCount].Text3);

			Tasks[TaskID]->ActivityCount++;

		}
		mysql_free_result(result);
		safe_delete_array(query);

	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
		safe_delete_array(query);
		return false;
	}
	return true;
}

bool TaskManager::SaveClientState(Client *c, ClientTaskState *state) {

	// I am saving the slot in the ActiveTasks table, because unless a Task is cancelled/completed, the client doesn't
	// seem to like tasks moving slots between zoning and you can end up with 'bogus' activities if the task previously
	// in that slot had more activities than the one now occupying it. Hopefully retaining the slot number for the 
	// duration of a session will overcome this.
	//
	const char *TaskQuery="REPLACE INTO character_tasks (charid, taskid, slot, acceptedtime) "
	                      "VALUES (%i, %i, %i, %i)";

	const char *ActivityQuery="REPLACE INTO character_activities (charid, taskid, activityid, donecount, completed) "
		         	  "VALUES ";

	const char *CompletedTaskQuery="REPLACE INTO completed_tasks (charid, completedtime, taskid, activityid) "
	                      "VALUES (%i, %i, %i, %i)";

	const char *ERR_MYSQLERROR = "[TASKS]Error in TaskManager::SaveClientState %s";

	if(!c || !state) return false;

	int CharacterID = c->CharacterID();

	_log(TASKS__CLIENTSAVE,"TaskManager::SaveClientState for character ID %d", CharacterID);


	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	if(state->ActiveTaskCount > 0) {
		for(int Task=0; Task<MAXACTIVETASKS; Task++) {
			int TaskID = state->ActiveTasks[Task].TaskID;
			if(TaskID==TASKSLOTEMPTY) continue;
			if(state->ActiveTasks[Task].Updated) {

				_log(TASKS__CLIENTSAVE, "TaskManager::SaveClientState for character ID %d, Updating TaskIndex %i TaskID %i", 
				          CharacterID, Task, TaskID); 

				if(!database.RunQuery(query,MakeAnyLenString(&query, TaskQuery,
						      CharacterID,
					              TaskID,
						      Task,
				        	      state->ActiveTasks[Task].AcceptedTime), errbuf)) {

					LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
				}
				else 
					state->ActiveTasks[Task].Updated = false;

				safe_delete_array(query);
			}

			int UpdatedActivityCount = 0;
			string UpdateActivityQuery = ActivityQuery;
			char *buf = 0;

			for(int Activity=0; Activity<Tasks[TaskID]->ActivityCount; Activity++) {

				if(state->ActiveTasks[Task].Activity[Activity].Updated) {

					_log(TASKS__CLIENTSAVE, "TaskManager::SaveClientSate for character ID %d, "
						  "Updating Activity %i, %i", 
					          CharacterID, Task, Activity); 
					
					if(UpdatedActivityCount==0) {
						MakeAnyLenString(&buf, "(%i, %i, %i, %i, %i)", CharacterID, TaskID,
								 Activity,
								 state->ActiveTasks[Task].Activity[Activity].DoneCount,
								 state->ActiveTasks[Task].Activity[Activity].State == 
								 ActivityCompleted);
					}
					else {
						MakeAnyLenString(&buf, ", (%i, %i, %i, %i, %i)", CharacterID, TaskID,
								 Activity,
								 state->ActiveTasks[Task].Activity[Activity].DoneCount,
								 state->ActiveTasks[Task].Activity[Activity].State == 
								 ActivityCompleted);
					}
					UpdateActivityQuery = UpdateActivityQuery + buf;
					safe_delete_array(buf);
					UpdatedActivityCount++;
				}
			}

			if(UpdatedActivityCount > 0) {
				_log(TASKS__CLIENTSAVE, "Executing query %s", UpdateActivityQuery.c_str());
				if(!database.RunQuery(query,MakeAnyLenString(&query, UpdateActivityQuery.c_str()),
					     	      errbuf)) {

					LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
				}
				else {
					state->ActiveTasks[Task].Updated=false;
					for(int Activity=0; Activity<Tasks[TaskID]->ActivityCount; Activity++) 
						state->ActiveTasks[Task].Activity[Activity].Updated=false;

				}

				safe_delete_array(query);
			}
		}

	}
	if(RuleB(TaskSystem, RecordCompletedTasks) && 
	   (state->CompletedTasks.size() > (unsigned int)state->LastCompletedTaskLoaded)) {

		for(unsigned int i=state->LastCompletedTaskLoaded; i<state->CompletedTasks.size(); i++) {

			_log(TASKS__CLIENTSAVE, "TaskManager::SaveClientState Saving Completed Task at slot %i", i);
			int TaskID = state->CompletedTasks[i].TaskID;
			if((TaskID<=0) || (TaskID>=MAXTASKS) || (Tasks[TaskID]==nullptr)) continue;

	   		// First we save a record with an ActivityID of -1.
			// This indicates this task was completed at the given time. We infer that all
			// none optional activities were completed.
			//
			if(!database.RunQuery(query,MakeAnyLenString(&query, CompletedTaskQuery,
					      CharacterID,
					      state->CompletedTasks[i].CompletedTime,
					      TaskID, -1), errbuf)) {

				LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
				continue;
			}
			safe_delete_array(query);

			// If the Rule to record non-optional task completion is not enabled, don't save it
			if(!RuleB(TaskSystem, RecordCompletedOptionalActivities)) continue;

			// Insert one record for each completed optional task.
			
			for(int j=0; j<Tasks[TaskID]->ActivityCount; j++) {
				if(Tasks[TaskID]->Activity[j].Optional && state->CompletedTasks[i].ActivityDone[j]) {

					if(!database.RunQuery(query,MakeAnyLenString(&query, CompletedTaskQuery,
							      CharacterID,
							      state->CompletedTasks[i].CompletedTime,
							      TaskID, j), errbuf)) {

						LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, errbuf);
					}
					safe_delete_array(query);
				}
			}
		}
		state->LastCompletedTaskLoaded = state->CompletedTasks.size();
	}

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
		
bool TaskManager::LoadClientState(Client *c, ClientTaskState *state) {

	const char *TaskQuery = "SELECT `taskid`, `slot`, `acceptedtime` from `character_tasks` "
	                        "WHERE `charid` = %i ORDER BY acceptedtime";

	const char *ERR_TASK_OOR1 = "[TASKS]Task ID %i out of range while loading character tasks from database";

	const char *ERR_SLOT_OOR = "[TASKS] Slot %i out of range while loading character tasks from database";

	const char *ERR_DUP_SLOT = "[TASKS] Slot %i for Task %is is already occupied.";

	const char *ERR_MYSQLERROR1 = "[TASKS]Error in TaskManager::LoadClientState load Tasks: %s";

	char		errbuf[MYSQL_ERRMSG_SIZE];
	char*		query = 0;
	MYSQL_RES 	*result;
	MYSQL_ROW 	row;

	if(!c || !state) return false;

	int CharacterID = c->CharacterID();

	state->ActiveTaskCount = 0;

	_log(TASKS__CLIENTLOAD, "TaskManager::LoadClientSate for character ID %d", CharacterID); 

	if(database.RunQuery(query,MakeAnyLenString(&query, TaskQuery, CharacterID), errbuf, &result)) {

		while((row = mysql_fetch_row(result))) {

			int TaskID = atoi(row[0]);
			int Slot = atoi(row[1]);

			if((TaskID<0) || (TaskID>=MAXTASKS)) {
				LogFile->write(EQEMuLog::Error, ERR_TASK_OOR1, TaskID);
				continue;
			}

			if((Slot<0) || (Slot>=MAXACTIVETASKS)) {
				LogFile->write(EQEMuLog::Error, ERR_SLOT_OOR, Slot);
				continue;
			}

			if(state->ActiveTasks[Slot].TaskID != TASKSLOTEMPTY) {
				LogFile->write(EQEMuLog::Error, ERR_DUP_SLOT, Slot, TaskID);
				continue;
			}

			int acceptedtime = atoi(row[2]);

			state->ActiveTasks[Slot].TaskID = TaskID;

			state->ActiveTasks[Slot].CurrentStep = -1;

			state->ActiveTasks[Slot].AcceptedTime = acceptedtime;

			state->ActiveTasks[Slot].Updated = false;

			for(int i=0; i<MAXACTIVITIESPERTASK; i++) {
				state->ActiveTasks[Slot].Activity[i].ActivityID = -1;
			}

			//LoadClientActivitiesForTask(CharacterID, &state->ActiveTasks[state->ActiveTaskCount]);
			// Calculate which activities are active based on those completed.
			//state->UnlockActivities(state->ActiveTaskCount);

			state->ActiveTaskCount++;

			_log(TASKS__CLIENTLOAD, "TaskManager::LoadClientState. Char: %i Task ID %i, "
			          "Accepted Time: %8X",
			          CharacterID, TaskID,acceptedtime);
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR1, errbuf);
		safe_delete_array(query);
		safe_delete(state);
		return false;
	}

	// Load Activities
	
	const char *ActivityQuery = "SELECT `taskid`, `activityid`, `donecount`, `completed` "
	                            " from `character_activities` WHERE `charid` = %i "
				    "ORDER BY `taskid` ASC, `activityid` ASC";

	const char *ERR_TASK_OOR2 = "[TASKS]Task ID %i out of range while loading character activities from database";

	const char *ERR_ACTIVITY_OOR = "[TASKS]Activity ID %i out of range while loading character activities from database";

	const char *ERR_NOTASK = "[TASKS]Activity %i found for task %i which client does not have.";

	const char *ERR_MYSQLERROR2 = "[TASKS]Error in TaskManager::LoadClientState load Activities: %s";

	_log(TASKS__CLIENTLOAD, "LoadClientState. Loading activities for character ID %d", CharacterID); 

	

	if(database.RunQuery(query,MakeAnyLenString(&query, ActivityQuery, 
						    CharacterID), errbuf, &result)) {


		while((row = mysql_fetch_row(result))) {
			int TaskID = atoi(row[0]);
			if((TaskID<0) || (TaskID>=MAXTASKS)) {
				LogFile->write(EQEMuLog::Error, ERR_TASK_OOR2, TaskID);
				continue;
			}
			int ActivityID = atoi(row[1]);
			if((ActivityID<0) || (ActivityID>=MAXACTIVITIESPERTASK)) {
				LogFile->write(EQEMuLog::Error, ERR_ACTIVITY_OOR, ActivityID);
				continue;
			}

			// Find Active Task Slot
			int ActiveTaskIndex = -1;

			for(int i=0; i<MAXACTIVETASKS; i++) {
				if(state->ActiveTasks[i].TaskID == TaskID) {
					ActiveTaskIndex = i;
					break;
				}
			}

			if(ActiveTaskIndex == -1) {
				LogFile->write(EQEMuLog::Error, ERR_NOTASK,  ActivityID, TaskID);
				continue;
			}

			int DoneCount = atoi(row[2]);
			bool Completed = atoi(row[3]);
			state->ActiveTasks[ActiveTaskIndex].Activity[ActivityID].ActivityID = ActivityID;
			state->ActiveTasks[ActiveTaskIndex].Activity[ActivityID].DoneCount = DoneCount;
			if(Completed) state->ActiveTasks[ActiveTaskIndex].Activity[ActivityID].State = ActivityCompleted;
			else
				state->ActiveTasks[ActiveTaskIndex].Activity[ActivityID].State = ActivityHidden;

			state->ActiveTasks[ActiveTaskIndex].Activity[ActivityID].Updated = false;

			_log(TASKS__CLIENTLOAD, "TaskManager::LoadClientState. Char: %i Task ID %i, ActivityID: %i, "
			          "DoneCount: %i, Completed: %i",
			          CharacterID, TaskID, ActivityID, DoneCount, Completed);

		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR2, errbuf);
		safe_delete_array(query);
		safe_delete(state);
		return false;
	}

	const char *CompletedTaskQuery = "SELECT `taskid`, `activityid`, `completedtime` from `completed_tasks` "
	                        	 "WHERE `charid` = %i ORDER BY completedtime, taskid, activityid";

	const char *ERR_TASK_OOR3 = "[TASKS]Task ID %i out of range while loading completed tasks from database";

	const char *ERR_ACTIVITY_OOR2 = "[TASKS]Activity ID %i out of range while loading completed tasks from database";

	const char *ERR_MYSQLERROR3 = "[TASKS]Error in TaskManager::LoadClientState load completed tasks: %s";

	

	if(RuleB(TaskSystem, RecordCompletedTasks)) {
		if(database.RunQuery(query,MakeAnyLenString(&query, CompletedTaskQuery, 
						    CharacterID), errbuf, &result)) {

			CompletedTaskInformation cti;

			for(int i=0; i<MAXACTIVITIESPERTASK; i++)
				cti.ActivityDone[i] = false;

			int PreviousTaskID = -1;
			int PreviousCompletedTime = -1;

			while((row = mysql_fetch_row(result))) {
				int TaskID = atoi(row[0]);
				if((TaskID <= 0) || (TaskID >=MAXTASKS)) {
					LogFile->write(EQEMuLog::Error, ERR_TASK_OOR3, TaskID);
					continue;
				}
				int ActivityID = atoi(row[1]);

				// An ActivityID of -1 means mark all the none optional activities in the
				// task as complete. If the Rule to record optional activities is enabled,
				// subsequent records for this task will flag any optional tasks that were
				// completed.
				if((ActivityID<-1) || (ActivityID>=MAXACTIVITIESPERTASK)) {
					LogFile->write(EQEMuLog::Error, ERR_ACTIVITY_OOR2, ActivityID);
					continue;
				}
				int CompletedTime = atoi(row[2]);

				if((PreviousTaskID != -1) && ((TaskID != PreviousTaskID) || 
				   (CompletedTime != PreviousCompletedTime))) {

					state->CompletedTasks.push_back(cti);
					for(int i=0; i<MAXACTIVITIESPERTASK; i++)
						cti.ActivityDone[i] = false;
				}
				cti.TaskID = PreviousTaskID = TaskID;
				cti.CompletedTime = PreviousCompletedTime = CompletedTime;
			
				// If ActivityID is -1, Mark all the non-optional tasks as completed.
				if(ActivityID < 0) {
					TaskInformation* Task = Tasks[TaskID];
					if(Task == nullptr) continue;

					for(int i=0; i<Task->ActivityCount; i++)
						if(!Task->Activity[i].Optional)
							cti.ActivityDone[i] = true;
				}
				else
					cti.ActivityDone[ActivityID] = true;

			}
			if(PreviousTaskID != -1)
				state->CompletedTasks.push_back(cti);

			state->LastCompletedTaskLoaded = state->CompletedTasks.size();

			mysql_free_result(result);
			safe_delete_array(query);
		}
		else {
			LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR3, errbuf);
			safe_delete_array(query);
			safe_delete(state);
			return false;
		}
	}

	const char *EnabledTaskQuery = "SELECT `taskid` FROM character_enabledtasks WHERE `charid` = %i "
				      "AND `taskid` >0 AND `taskid` < %i ORDER BY `taskid` ASC";
	
	const char *ERR_MYSQLERROR4 = "[TASKS]Error in TaskManager::LoadClientState load enabled tasks: %s";

	if(database.RunQuery(query,MakeAnyLenString(&query, EnabledTaskQuery, 
						    CharacterID, MAXTASKS), errbuf, &result)) {

		while((row = mysql_fetch_row(result))) {
			int TaskID = atoi(row[0]);
			state->EnabledTasks.push_back(TaskID);
			_log(TASKS__CLIENTLOAD, "Adding TaskID %i to enabled tasks", TaskID);
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR4, errbuf);
		safe_delete_array(query);
	}

	// Check that there is an entry in the client task state for every activity in each task
	// This should only break if a ServerOP adds or deletes activites for a task that players already
	// have active, or due to a bug.
	

	const char *ERR_NOTASK2 = "[TASKS]Character %i has task %i which does not exist.";

	const char *ERR_INCONSISTENT = "[TASKS]Fatal error in character %i task state. Activity %i for "
				       "Task %i either missing from client state or from task.";

	for(int i=0; i<MAXACTIVETASKS; i++) {
		int TaskID = state->ActiveTasks[i].TaskID;
		if(TaskID==TASKSLOTEMPTY) continue;
		if(!Tasks[TaskID]) {
			c->Message(13, "Active Task Slot %i, references a task (%i), that does not exist. "
				       "Removing from memory. Contact a GM to resolve this.",i, TaskID);

			LogFile->write(EQEMuLog::Error, ERR_NOTASK2, CharacterID, TaskID);
			state->ActiveTasks[i].TaskID=TASKSLOTEMPTY;
			continue;

		}
		for(int j=0; j<Tasks[TaskID]->ActivityCount; j++) {

			if(state->ActiveTasks[i].Activity[j].ActivityID != j) {
				c->Message(13, "Active Task %i, %s. Activity count does not match expected value."
					       "Removing from memory. Contact a GM to resolve this.",
					       TaskID, Tasks[TaskID]->Title);

				LogFile->write(EQEMuLog::Error, ERR_INCONSISTENT, CharacterID, j, TaskID);
				state->ActiveTasks[i].TaskID=TASKSLOTEMPTY;
				break;
			}
		}
	}


	for(int i=0; i<MAXACTIVETASKS; i++) 
		if(state->ActiveTasks[i].TaskID != TASKSLOTEMPTY)
			state->UnlockActivities(CharacterID, i);

	_log(TASKS__CLIENTLOAD, "LoadClientState for Character ID %d DONE!", CharacterID); 
	return true;
}

void ClientTaskState::EnableTask(int CharID, int TaskCount, int *TaskList) {

	// Check if the Task is already enabled for this client
	//
	vector<int> TasksEnabled;
	vector<int>::iterator Iterator;

	for(int i=0; i<TaskCount; i++) {
		Iterator = EnabledTasks.begin();
		bool AddTask = true;

		while(Iterator != EnabledTasks.end()) {
			// If this task is already enabled, stop looking
			if((*Iterator) == TaskList[i]) {
				AddTask = false;
				break;
			}
			// Our list of enabled tasks is sorted, so we can quit if we find a taskid higher than
			// the one we are looking for.
			if((*Iterator) > TaskList[i]) break;
			Iterator++;
		}
		if(AddTask) {
			EnabledTasks.insert(Iterator, TaskList[i]);
			// Make a note of the task we enabled, for later SQL generation
			TasksEnabled.push_back(TaskList[i]);
		}
	}

	_log(TASKS__UPDATE, "New enabled task list ");
	for(unsigned int i=0; i<EnabledTasks.size(); i++)
		_log(TASKS__UPDATE, "%i ", EnabledTasks[i]);

	if(TasksEnabled.size() == 0 ) return;

	string TaskQuery="REPLACE INTO character_enabledtasks (charid, taskid) VALUES ";

	const char *ERR_MYSQLERROR = "[TASKS]Error in ClientTaskState::EnableTask %s %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	char *buf = 0;

	for(unsigned int i=0; i<TasksEnabled.size(); i++) {
		if(i==0) 
			MakeAnyLenString(&buf, "(%i, %i)", CharID, TasksEnabled[i]);
		else
			MakeAnyLenString(&buf, ",(%i, %i)", CharID, TasksEnabled[i]);

		TaskQuery += buf;
		safe_delete_array(buf);
	}

	_log(TASKS__UPDATE, "Executing query %s", TaskQuery.c_str());

	if(!database.RunQuery(query,MakeAnyLenString(&query, TaskQuery.c_str()), errbuf)) {

		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, query, errbuf);
	}

	safe_delete_array(query);
}

void ClientTaskState::DisableTask(int CharID, int TaskCount, int *TaskList) {

	// Check if the Task is enabled for this client
	//
	vector<int> TasksDisabled;
	vector<int>::iterator Iterator;
	
	for(int i=0; i<TaskCount; i++) {
		Iterator = EnabledTasks.begin();
		bool RemoveTask = false;

		while(Iterator != EnabledTasks.end()) {
			if((*Iterator) == TaskList[i]) {
				RemoveTask = true;
				break;
			}
			if((*Iterator) > TaskList[i]) break;
			Iterator++;
		}
		if(RemoveTask) {
			EnabledTasks.erase(Iterator);
			TasksDisabled.push_back(TaskList[i]);
		}
	}

	_log(TASKS__UPDATE, "New enabled task list ");
	for(unsigned int i=0; i<EnabledTasks.size(); i++)
		_log(TASKS__UPDATE, "%i ", EnabledTasks[i]);

	if(TasksDisabled.size() == 0) return;

	string TaskQuery="DELETE FROM character_enabledtasks WHERE ";

	const char *ERR_MYSQLERROR = "[TASKS]Error in ClientTaskState::DisableTask %s %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	char *buf = 0;

	MakeAnyLenString(&buf, "charid=%i AND (", CharID);
	TaskQuery += buf;
	safe_delete_array(buf);

	for(unsigned int i=0; i<TasksDisabled.size(); i++) {
		if(i==0)
			MakeAnyLenString(&buf, "taskid=%i", TasksDisabled[i]);
		else
			MakeAnyLenString(&buf, " OR taskid=%i", TasksDisabled[i]);

		TaskQuery += buf;
		safe_delete_array(buf);
	}

	TaskQuery = TaskQuery + ")";
	_log(TASKS__UPDATE, "Executing query %s", TaskQuery.c_str());

	if(!database.RunQuery(query,MakeAnyLenString(&query, TaskQuery.c_str()), errbuf)) {

		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, query, errbuf);
	}

	safe_delete_array(query);
}

bool ClientTaskState::IsTaskEnabled(int TaskID) {

	vector<int>::iterator Iterator;

	Iterator = EnabledTasks.begin();

	while(Iterator != EnabledTasks.end()) {
		if((*Iterator) == TaskID) return true;
		if((*Iterator) > TaskID) break;
		Iterator++;
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

int TaskManager::FirstTaskInSet(int TaskSetID) {

	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return 0;

	if(TaskSets[TaskSetID].size() == 0) return 0;

	vector<int>::iterator Iterator = TaskSets[TaskSetID].begin();

	while(Iterator != TaskSets[TaskSetID].end()) {
		if((*Iterator) > 0)
			return (*Iterator);
		Iterator++;
	}

	return 0;
}

int TaskManager::LastTaskInSet(int TaskSetID) {

	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return 0;

	if(TaskSets[TaskSetID].size() == 0) return 0;

	return TaskSets[TaskSetID][TaskSets[TaskSetID].size()-1];
}

int TaskManager::NextTaskInSet(int TaskSetID, int TaskID) {

	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return 0;

	if(TaskSets[TaskSetID].size() == 0) return 0;

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

void TaskManager::TaskSetSelector(Client *c, ClientTaskState *state, Mob *mob, int TaskSetID) {

	unsigned int EnabledTaskIndex = 0;
	unsigned int TaskSetIndex = 0;
	int TaskList[MAXCHOOSERENTRIES];
	int TaskListIndex = 0;
	int PlayerLevel = c->GetLevel();

	_log(TASKS__UPDATE, "TaskSetSelector called for taskset %i. EnableTaskSize is %i", TaskSetID, 
			     state->EnabledTasks.size());
	if((TaskSetID<=0) || (TaskSetID>=MAXTASKSETS)) return;

	if(TaskSets[TaskSetID].size() > 0) {

		// A TaskID of 0 in a TaskSet indicates that all Tasks in the set are enabled for all players.
		
		if(TaskSets[TaskSetID][0] == 0) {

			_log(TASKS__UPDATE, "TaskSets[%i][0] == 0. All Tasks in Set enabled.", TaskSetID);
			vector<int>::iterator Iterator = TaskSets[TaskSetID].begin();

			while((Iterator != TaskSets[TaskSetID].end()) && (TaskListIndex < MAXCHOOSERENTRIES)) {
				if(AppropriateLevel((*Iterator), PlayerLevel) && !state->IsTaskActive((*Iterator)) &&
				   (IsTaskRepeatable((*Iterator)) || !state->IsTaskCompleted((*Iterator)))) 
					TaskList[TaskListIndex++] = (*Iterator);

				Iterator++;
			}
			if(TaskListIndex > 0) 
			{
				SendTaskSelector(c, mob, TaskListIndex, TaskList);
			}

			return;
		}
	}


	while((EnabledTaskIndex < state->EnabledTasks.size()) && (TaskSetIndex < TaskSets[TaskSetID].size()) &&
	      (TaskListIndex < MAXCHOOSERENTRIES)) {

		_log(TASKS__UPDATE, "Comparing EnabledTasks[%i] (%i) with TaskSets[%i][%i] (%i)",
			  EnabledTaskIndex, state->EnabledTasks[EnabledTaskIndex], TaskSetID, TaskSetIndex,
			  TaskSets[TaskSetID][TaskSetIndex]);

		if((TaskSets[TaskSetID][TaskSetIndex] > 0) &&
		   (state->EnabledTasks[EnabledTaskIndex] == TaskSets[TaskSetID][TaskSetIndex])) {

			if(AppropriateLevel(TaskSets[TaskSetID][TaskSetIndex], PlayerLevel) &&
			   !state->IsTaskActive(TaskSets[TaskSetID][TaskSetIndex]) &&
			   (IsTaskRepeatable(TaskSets[TaskSetID][TaskSetIndex]) ||
			    !state->IsTaskCompleted(TaskSets[TaskSetID][TaskSetIndex]))) {

				TaskList[TaskListIndex++] = TaskSets[TaskSetID][TaskSetIndex];

				EnabledTaskIndex++;
				TaskSetIndex++;
				continue;
			}
		}

		if(state->EnabledTasks[EnabledTaskIndex] < TaskSets[TaskSetID][TaskSetIndex])
			EnabledTaskIndex++;
		else
			TaskSetIndex++;
	}

	if(TaskListIndex == 0) return;

	SendTaskSelector(c, mob, TaskListIndex, TaskList);

}

void TaskManager::SendTaskSelector(Client *c, Mob *mob, int TaskCount, int *TaskList) {

	if (c->GetClientVersion() >= EQClientRoF)
	{
		SendTaskSelectorNew(c, mob, TaskCount, TaskList);
		return;
	}
	// Titanium OpCode: 0x5e7c
	_log(TASKS__UPDATE, "TaskSelector for %i Tasks", TaskCount);
	char *Ptr;
	int PlayerLevel = c->GetLevel();

	AvailableTaskHeader_Struct*	AvailableTaskHeader;
	AvailableTaskData1_Struct* 	AvailableTaskData1;
	AvailableTaskData2_Struct* 	AvailableTaskData2;
	AvailableTaskTrailer_Struct* 	AvailableTaskTrailer;

	// Check if any of the  tasks exist

	
	for(int i=0; i<TaskCount; i++) {

		if(Tasks[TaskList[i]] != nullptr) break;
	}

	// FIXME: The 10 and 5 values in this calculation are to account for the string "ABCD" we are putting in 3 times. 
	//
	// Calculate how big the packet needs to be pased on the number of tasks and the
	// size of the variable length strings.
	
	int PacketLength = sizeof(AvailableTaskHeader_Struct);

	int ValidTasks = 0;

	for(int i=0; i<TaskCount; i++) {

		if(!AppropriateLevel(TaskList[i], PlayerLevel)) continue;

		if(c->IsTaskActive(TaskList[i])) continue;

		if(!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) continue;

		ValidTasks++;

		PacketLength = PacketLength + sizeof(AvailableTaskData1_Struct) + strlen(Tasks[TaskList[i]]->Title) + 1 +
	                       strlen(Tasks[TaskList[i]]->Description) + 1 + sizeof(AvailableTaskData2_Struct) + 10 +
			       sizeof(AvailableTaskTrailer_Struct) + 5;
	}

	if(ValidTasks == 0) return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_OpenNewTasksWindow, PacketLength);

	AvailableTaskHeader = (AvailableTaskHeader_Struct*)outapp->pBuffer;

	AvailableTaskHeader->TaskCount = ValidTasks;

	// unknown1 is always 2 in the packets I have ssen. It may be a 'Task Type'. Given that the
	// task system was apparently first introduced for LDoN missions, type 1 may be for missions.
	//
	AvailableTaskHeader->unknown1 = 2;
	AvailableTaskHeader->TaskGiver = mob->GetID();

	Ptr = (char *) AvailableTaskHeader + sizeof(AvailableTaskHeader_Struct);

	for(int i=0; i<TaskCount;i++) {

		if(!AppropriateLevel(TaskList[i], PlayerLevel)) continue;

		if(c->IsTaskActive(TaskList[i])) continue;

		if(!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) continue;

		AvailableTaskData1 = (AvailableTaskData1_Struct*)Ptr;

		AvailableTaskData1->TaskID = TaskList[i];

		AvailableTaskData1->TimeLimit = Tasks[TaskList[i]]->Duration;

		AvailableTaskData1->unknown2 = 0;

		Ptr = (char *)AvailableTaskData1 + sizeof(AvailableTaskData1_Struct);
	
		sprintf(Ptr, "%s", Tasks[TaskList[i]]->Title);

		Ptr = Ptr + strlen(Ptr) + 1;

		sprintf(Ptr, "%s", Tasks[TaskList[i]]->Description);

		Ptr = Ptr + strlen(Ptr) + 1;

		AvailableTaskData2 = (AvailableTaskData2_Struct*)Ptr;

		AvailableTaskData2->unknown1 = 1;
		AvailableTaskData2->unknown2 = 0;
		AvailableTaskData2->unknown3 = 1;
		AvailableTaskData2->unknown4 = 0;

		Ptr = (char *)AvailableTaskData2 + sizeof(AvailableTaskData2_Struct);

		// FIXME: In live packets, these two strings appear to be the same as the Text1 and Text2
		// strings from the first activity in the task, however the task chooser/selector
		// does not appear to make use of them.
		sprintf(Ptr, "ABCD");				
		Ptr = Ptr + strlen(Ptr) + 1;
		sprintf(Ptr, "ABCD");	
		Ptr = Ptr + strlen(Ptr) + 1;

		AvailableTaskTrailer = (AvailableTaskTrailer_Struct*)Ptr;

		// The name of this ItemCount field may be incorrect, but 1 works.
		AvailableTaskTrailer->ItemCount = 1;
		AvailableTaskTrailer->unknown1 = 0xFFFFFFFF;
		AvailableTaskTrailer->unknown2 = 0xFFFFFFFF;
		AvailableTaskTrailer->StartZone = Tasks[TaskList[i]]->StartZone;

		Ptr = (char *)AvailableTaskTrailer + sizeof(AvailableTaskTrailer_Struct);
		
		// In some packets, this next string looks like a short task summary, however it doesn't
		// appear anywhere in the client window.
		sprintf(Ptr, "ABCD"); 
		Ptr = Ptr + strlen(Ptr) + 1;
	}

	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

void TaskManager::SendTaskSelectorNew(Client *c, Mob *mob, int TaskCount, int *TaskList) {

	_log(TASKS__UPDATE, "TaskSelector for %i Tasks", TaskCount);

	int PlayerLevel = c->GetLevel();

	// Check if any of the  tasks exist
	for(int i=0; i<TaskCount; i++)
	{
		if(Tasks[TaskList[i]] != nullptr) break;
	}
	
	int PacketLength = 12;	// Header

	int ValidTasks = 0;

	char StartZone[10];

	for(int i=0; i<TaskCount; i++) {

		if(!AppropriateLevel(TaskList[i], PlayerLevel)) continue;

		if(c->IsTaskActive(TaskList[i])) continue;

		if(!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) continue;

		ValidTasks++;

		PacketLength += 21;	// Task Data - strings
		PacketLength += strlen(Tasks[TaskList[i]]->Title) + 1 +
						strlen(Tasks[TaskList[i]]->Description) + 1;
						
		sprintf(StartZone, "%i", Tasks[TaskList[i]]->StartZone);	
		/*
		PacketLength += strlen(Tasks[TaskList[i]]->Activity[ActivityID].Text1) + 1 + 
						strlen(Tasks[TaskList[i]]->Activity[ActivityID].Text2) +
						strlen(Tasks[TaskList[i]]->Activity[ActivityID].Text3) + 1 +
						strlen(itoa(Tasks[TaskList[i]]->Activity[ActivityID].ZoneID)) + 1 +
						3 + 3 + 5;	// Other strings (Hard set for now)
		*/
		PacketLength += 11 + 11 + 11 + 3 + 3 + (strlen(StartZone) * 2) + 2;	// Other strings (Hard set for now)
		PacketLength += 28;	// Activity Data - strings (Hard set for 1 activity per task for now)
	}

	if(ValidTasks == 0) return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_OpenNewTasksWindow, PacketLength);
	
	outapp->WriteUInt32(ValidTasks);	// TaskCount
	outapp->WriteUInt32(2);			// Unknown2
	outapp->WriteUInt32(mob->GetID());	// TaskGiver

	for(int i=0; i<TaskCount;i++) {

		if(!AppropriateLevel(TaskList[i], PlayerLevel)) continue;

		if(c->IsTaskActive(TaskList[i])) continue;

		if(!IsTaskRepeatable(TaskList[i]) && c->IsTaskCompleted(TaskList[i])) continue;

		outapp->WriteUInt32(TaskList[i]);	// TaskID
		outapp->WriteFloat(1.0f);
		outapp->WriteUInt32(Tasks[TaskList[i]]->Duration);
		outapp->WriteUInt32(0);				// Unknown7
		
		outapp->WriteString(Tasks[TaskList[i]]->Title);
		outapp->WriteString(Tasks[TaskList[i]]->Description);
		
		outapp->WriteUInt8(0);				// Unknown10 - Empty string ?
		outapp->WriteUInt32(1);				// ActivityCount - Hard set to 1 for now

		// Activity stuff below - Will need to iterate through each task
		// Currently hard set for testing
	

		sprintf(StartZone, "%i", Tasks[TaskList[i]]->StartZone);

		outapp->WriteUInt32(0);				// ActivityNumber
		outapp->WriteUInt32(1);				// ActivityType
		outapp->WriteUInt32(0);				// Unknown14
		outapp->WriteString("Text1 Test");
		outapp->WriteUInt32(11);			// Text2Len
		outapp->WriteString("Text2 Test");
		outapp->WriteUInt32(1);				// GoalCount
		outapp->WriteUInt32(3);				// NumString1Len
		outapp->WriteString("-1");
		outapp->WriteUInt32(3);				// NumString2Len
		outapp->WriteString("-1");
		//outapp->WriteString(itoa(Tasks[TaskList[i]]->Activity[ActivityID].ZoneID));
		outapp->WriteString(StartZone);		// Zone number in ascii
		outapp->WriteString("Text3 Test");
		outapp->WriteString(StartZone);		// Zone number in ascii
	}
	_pkt(TASKS__PACKETS, outapp);

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
		c->Message(0, "TaskID out-of-range.");
		return;
	}

	if(Tasks[TaskID] == nullptr) {
		c->Message(0, "Task does not exist.");
		return;
	}

	char Explanation[1000], *ptr;
	c->Message(0, "Task %4i: Title: %s", TaskID, Tasks[TaskID]->Description);
	c->Message(0, "%3i Activities", Tasks[TaskID]->ActivityCount);
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

	for(int i=0; i<MAXACTIVETASKS; i++)
		ActiveTasks[i].TaskID = TASKSLOTEMPTY;
}

ClientTaskState::~ClientTaskState() {
}


int ClientTaskState::GetActiveTaskID(int index) {

	// Return the TaskID from the client's specified Active Task slot.

	if((index<0) || (index>=MAXACTIVETASKS)) return 0;

	return ActiveTasks[index].TaskID;
}

static void DeleteCompletedTaskFromDatabase(int CharID, int TaskID) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	const char *TaskQuery="DELETE FROM completed_tasks WHERE charid=%i AND taskid = %i";

	_log(TASKS__UPDATE, "DeleteCompletedTasksFromDatabase. CharID = %i, TaskID = %i",
		  CharID, TaskID);
		
	if(!database.RunQuery(query,MakeAnyLenString(&query, TaskQuery, CharID, TaskID), errbuf)) {

		LogFile->write(EQEMuLog::Error, "[TASKS]Error in CientTaskState::CancelTask %s, %s", query, errbuf);
		safe_delete_array(query);
		return;
	}
	_log(TASKS__UPDATE, "Delete query %s", query);
	safe_delete_array(query);
}

bool ClientTaskState::UnlockActivities(int CharID, int TaskIndex) {

	bool AllActivitiesComplete = true;

	TaskInformation* Task = taskmanager->Tasks[ActiveTasks[TaskIndex].TaskID];

	if(Task==nullptr) return true;

	// On loading the client state, all activities that are not completed, are
	// marked as hidden. For Sequential (non-stepped) mode,  we mark the first
	// activity as active if not complete.
	_log(TASKS__UPDATE, "CharID: %i Task: %i Sequence mode is %i", 
			    CharID, ActiveTasks[TaskIndex].TaskID, Task->SequenceMode);	
	if(Task->SequenceMode == ActivitiesSequential) {

		if(ActiveTasks[TaskIndex].Activity[0].State != ActivityCompleted)
			ActiveTasks[TaskIndex].Activity[0].State = ActivityActive;

		// Enable the next Hidden task.
		for(int i=0; i<Task->ActivityCount; i++) {
			if((ActiveTasks[TaskIndex].Activity[i].State == ActivityActive) &&
			   (!Task->Activity[i].Optional)) {
				AllActivitiesComplete = false;
				break;
			}
			if(ActiveTasks[TaskIndex].Activity[i].State == ActivityHidden) {
				ActiveTasks[TaskIndex].Activity[i].State = ActivityActive;
				AllActivitiesComplete = false;
				break;
			}
		}
		if(AllActivitiesComplete && RuleB(TaskSystem, RecordCompletedTasks)) {
			if(RuleB(TasksSystem, KeepOneRecordPerCompletedTask)) {
				_log(TASKS__UPDATE, "KeepOneRecord enabled");
				vector<CompletedTaskInformation>::iterator Iterator = CompletedTasks.begin();
				int ErasedElements = 0;
				while(Iterator != CompletedTasks.end()) {
					int TaskID = (*Iterator).TaskID;
					if(TaskID == ActiveTasks[TaskIndex].TaskID) {
						Iterator = CompletedTasks.erase(Iterator);
						ErasedElements++;
					}
					else
						Iterator++;
				}
				_log(TASKS__UPDATE, "Erased Element count is %i", ErasedElements);
				if(ErasedElements) {
					LastCompletedTaskLoaded -= ErasedElements;
					DeleteCompletedTaskFromDatabase(CharID, ActiveTasks[TaskIndex].TaskID);
				}

			}

			CompletedTaskInformation cti;
			cti.TaskID = ActiveTasks[TaskIndex].TaskID;
			cti.CompletedTime = time(nullptr);

			for(int i=0; i<Task->ActivityCount; i++)
				cti.ActivityDone[i] = (ActiveTasks[TaskIndex].Activity[i].State == ActivityCompleted);

			CompletedTasks.push_back(cti);
		}
		_log(TASKS__UPDATE, "Returning sequential task, AllActivitiesComplete is %i", AllActivitiesComplete);
		return AllActivitiesComplete;
	}

	// Stepped Mode
	// TODO: This code is probably more complex than it needs to be
	
	bool CurrentStepComplete = true;

	_log(TASKS__UPDATE, "Current Step is %i, Last Step is %i", ActiveTasks[TaskIndex].CurrentStep, Task->LastStep);
	// If CurrentStep is -1, this is the first call to this method since loading the
	// client state. Unlock all activities with a step number of 0
	if(ActiveTasks[TaskIndex].CurrentStep == -1) {
		for(int i=0; i<Task->ActivityCount; i++) {
	
			if((Task->Activity[i].StepNumber == 0) &&
		   	   (ActiveTasks[TaskIndex].Activity[i].State == ActivityHidden)) {
				ActiveTasks[TaskIndex].Activity[i].State = ActivityActive;
				//ActiveTasks[TaskIndex].Activity[i].Updated=true;
			}
		}
		ActiveTasks[TaskIndex].CurrentStep = 0;
	}

	for(int Step=ActiveTasks[TaskIndex].CurrentStep; Step<=Task->LastStep; Step++) {
		for(int Activity=0; Activity<Task->ActivityCount; Activity++) {
			if(Task->Activity[Activity].StepNumber == (int)ActiveTasks[TaskIndex].CurrentStep) {
				if((ActiveTasks[TaskIndex].Activity[Activity].State != ActivityCompleted) &&
				   (!Task->Activity[Activity].Optional)) {
					CurrentStepComplete = false;
					AllActivitiesComplete = false;
					break;
				}
			}
		}
		if(!CurrentStepComplete) break;
		ActiveTasks[TaskIndex].CurrentStep++;
	}

	if(AllActivitiesComplete) {
		if(RuleB(TaskSystem, RecordCompletedTasks)) {
			// If we are only keeping one completed record per task, and the player has done
			// the same task again, erase the previous completed entry for this task.
			if(RuleB(TasksSystem, KeepOneRecordPerCompletedTask)) {
				_log(TASKS__UPDATE, "KeepOneRecord enabled");
				vector<CompletedTaskInformation>::iterator Iterator = CompletedTasks.begin();
				int ErasedElements = 0;
				while(Iterator != CompletedTasks.end()) {
					int TaskID = (*Iterator).TaskID;
					if(TaskID == ActiveTasks[TaskIndex].TaskID) {
						Iterator = CompletedTasks.erase(Iterator);
						ErasedElements++;
					}
					else
						Iterator++;
				}
				_log(TASKS__UPDATE, "Erased Element count is %i", ErasedElements);
				if(ErasedElements) {
					LastCompletedTaskLoaded -= ErasedElements;
					DeleteCompletedTaskFromDatabase(CharID, ActiveTasks[TaskIndex].TaskID);
				}

			}
			CompletedTaskInformation cti;
			cti.TaskID = ActiveTasks[TaskIndex].TaskID;
			cti.CompletedTime = time(nullptr);

			for(int i=0; i<Task->ActivityCount; i++)
				cti.ActivityDone[i] = (ActiveTasks[TaskIndex].Activity[i].State == ActivityCompleted);

			CompletedTasks.push_back(cti);
		}
		return true;
	}

	// Mark all non-completed tasks in the current step as active
	//
	for(int Activity=0; Activity<Task->ActivityCount; Activity++) {
		if((Task->Activity[Activity].StepNumber == (int)ActiveTasks[TaskIndex].CurrentStep) &&
		   (ActiveTasks[TaskIndex].Activity[Activity].State == ActivityHidden)) {
		   	ActiveTasks[TaskIndex].Activity[Activity].State = ActivityActive;
			ActiveTasks[TaskIndex].Activity[Activity].Updated=true;
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

	_log(TASKS__UPDATE, "ClientTaskState::UpdateTasks for NPCTypeID: %d", NPCTypeID);

	// If the client has no tasks, there is nothing further to check.
	
	if(!taskmanager || ActiveTaskCount == 0) return false;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID == TASKSLOTEMPTY) continue;

		// Check if there are any active kill activities for this task

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) return false;

		for(int j=0; j<Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if(ActiveTasks[i].Activity[j].State != ActivityActive) continue;
			// We are only interested in Kill activities
			if(Task->Activity[j].Type != ActivityType) continue;
			// Is there a zone restriction on the activity ?
			if((Task->Activity[j].ZoneID >0) && (Task->Activity[j].ZoneID != (int)zone->GetZoneID())) {
				_log(TASKS__UPDATE, "Char: %s Task: %i, Activity %i, Activity type %i for NPC %i failed zone check",
						    c->GetName(), ActiveTasks[i].TaskID, j, ActivityType, NPCTypeID);
				continue;
			}
			// Is the activity to kill this type of NPC ?
			switch(Task->Activity[j].GoalMethod) {

				case METHODSINGLEID:
					if(Task->Activity[j].GoalID != NPCTypeID) continue;
					break;

				case METHODLIST:
					if(!taskmanager->GoalListManager.IsInList(Task->Activity[j].GoalID, 
											NPCTypeID)) continue;
					break;

				default:
					// If METHODQUEST, don't update the activity here
					continue;
			}
			// We found an active task to kill this type of NPC, so increment the done count
			_log(TASKS__UPDATE, "Calling increment done count ByNPC");
			IncrementDoneCount(c, Task, i, j);
			Ret = true;
		}
	}
	
	return Ret;
}

int ClientTaskState::ActiveSpeakTask(int NPCTypeID) {

	// This method is to be used from Perl quests only and returns the TaskID of the first
	// active task found which has an active SpeakWith activity for this NPC.

	if(!taskmanager || ActiveTaskCount == 0) return 0;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID == TASKSLOTEMPTY) continue;

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) continue;

		for(int j=0; j<Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if(ActiveTasks[i].Activity[j].State != ActivityActive) continue;
			if(Task->Activity[j].Type != ActivitySpeakWith) continue;
			// Is there a zone restriction on the activity ?
			if((Task->Activity[j].ZoneID >0) && (Task->Activity[j].ZoneID != (int)zone->GetZoneID())) {
				continue;
			}
			// Is the activity to speak with this type of NPC ?
			if((Task->Activity[j].GoalMethod == METHODQUEST) &&
			   (Task->Activity[j].GoalID == NPCTypeID)) return ActiveTasks[i].TaskID;
		}
	}
	return 0;
}

int ClientTaskState::ActiveSpeakActivity(int NPCTypeID, int TaskID) {

	// This method is to be used from Perl quests only and returns the ActivityID of the first
	// active activity found in the specified task which is to SpeakWith this NPC.

	if(!taskmanager || ActiveTaskCount == 0) return -1;
	if((TaskID<=0) || (TaskID>=MAXTASKS)) return -1;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID != TaskID) continue;

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) continue;

		for(int j=0; j<Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if(ActiveTasks[i].Activity[j].State != ActivityActive) continue;
			if(Task->Activity[j].Type != ActivitySpeakWith) continue;
			// Is there a zone restriction on the activity ?
			if((Task->Activity[j].ZoneID >0) && (Task->Activity[j].ZoneID != (int)zone->GetZoneID())) {
				continue;
			}
			// Is the activity to speak with this type of NPC ?
			if((Task->Activity[j].GoalMethod == METHODQUEST) &&
			   (Task->Activity[j].GoalID == NPCTypeID)) return j;
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
	
	_log(TASKS__UPDATE, "ClientTaskState::UpdateTasksForItem(%d,%d)", Type, ItemID);

	if(ActiveTaskCount == 0) return;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID == TASKSLOTEMPTY) continue;

		// Check if there are any active loot activities for this task

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) return;

		for(int j=0; j<Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if(ActiveTasks[i].Activity[j].State != ActivityActive) continue;
			// We are only interested in the ActivityType we were called with
			if(Task->Activity[j].Type != (int)Type) continue;
			// Is there a zone restriction on the activity ?
			if((Task->Activity[j].ZoneID >0) && (Task->Activity[j].ZoneID != (int)zone->GetZoneID())) {
				_log(TASKS__UPDATE, "Char: %s Activity type %i for Item %i failed zone check",
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
			_log(TASKS__UPDATE, "Calling increment done count ForItem");
			IncrementDoneCount(c, Task, i, j, Count);
		}
	}
	
	return;
}

void ClientTaskState::UpdateTasksOnExplore(Client *c, int ExploreID) {

	// If the client has no tasks, there is nothing further to check.
	
	_log(TASKS__UPDATE, "ClientTaskState::UpdateTasksOnExplore(%i)", ExploreID);
	if(ActiveTaskCount == 0) return;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID == TASKSLOTEMPTY) continue;

		// Check if there are any active explore activities for this task

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) return;

		for(int j=0; j<Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if(ActiveTasks[i].Activity[j].State != ActivityActive) continue;
			// We are only interested in explore activities
			if(Task->Activity[j].Type != ActivityExplore) continue;
			if((Task->Activity[j].ZoneID >0) && (Task->Activity[j].ZoneID != (int)zone->GetZoneID())) {
				_log(TASKS__UPDATE, "Char: %s Explore exploreid %i failed zone check",
						    c->GetName(), ExploreID);
				continue;
			}
			// Is the activity to explore this area id ?
			switch(Task->Activity[j].GoalMethod) {

				case METHODSINGLEID:
					if(Task->Activity[j].GoalID != ExploreID) continue;
					break;

				case METHODLIST:
					if(!taskmanager->GoalListManager.IsInList(Task->Activity[j].GoalID, 
											ExploreID)) continue;
					break;

				default:
					// If METHODQUEST, don't update the activity here
					continue;
			}
			// We found an active task to explore this area, so set done count to goal count
			// (Only a goal count of 1 makes sense for explore activities?)
			_log(TASKS__UPDATE, "Increment on explore");
			IncrementDoneCount(c, Task, i, j, 
					   Task->Activity[j].GoalCount - ActiveTasks[i].Activity[j].DoneCount);

		}
	}
	
	return;
}

bool ClientTaskState::UpdateTasksOnDeliver(Client *c, uint32 *Items, int Cash, int NPCTypeID) {

	bool Ret = false;

	_log(TASKS__UPDATE, "ClientTaskState::UpdateTasksForOnDeliver(%d)", NPCTypeID);

	if(ActiveTaskCount == 0) return false;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID == TASKSLOTEMPTY) continue;

		// Check if there are any active deliver activities for this task

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) return false;

		for(int j=0; j<Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if(ActiveTasks[i].Activity[j].State != ActivityActive) continue;
			// We are only interested in Deliver activities
			if((Task->Activity[j].Type != ActivityDeliver) &&
			   (Task->Activity[j].Type != ActivityGiveCash)) continue;
			// Is there a zone restriction on the activity ?
			if((Task->Activity[j].ZoneID >0) && (Task->Activity[j].ZoneID != (int)zone->GetZoneID())) {
				_log(TASKS__UPDATE, "Char: %s Deliver activity failed zone check (current zone %i, need zone %i",
					  c->GetName(), zone->GetZoneID(), Task->Activity[j].ZoneID);
				continue;
			}
			// Is the activity to deliver to this NPCTypeID ?
			if(Task->Activity[j].DeliverToNPC != NPCTypeID) continue;
			// Is the activity related to these items ?
			//
			if((Task->Activity[j].Type == ActivityGiveCash) && Cash) {
				_log(TASKS__UPDATE, "Increment on GiveCash");
				IncrementDoneCount(c, Task, i, j, Cash);
				Ret = true;
			}
			else {
				for(int k=0; k<4; k++) {
					if(Items[k]==0) continue;
					switch(Task->Activity[j].GoalMethod) {

						case METHODSINGLEID:
							if(Task->Activity[j].GoalID != (int)Items[k]) continue;
							break;

						case METHODLIST:
							if(!taskmanager->GoalListManager.IsInList(Task->Activity[j].GoalID,
												  Items[k]))
								continue;
							break;
	
						default:
							// If METHODQUEST, don't update the activity here
							continue;
					}
					// We found an active task related to this item, so increment the done count
					_log(TASKS__UPDATE, "Increment on GiveItem");
					IncrementDoneCount(c, Task, i, j, 1);
					Ret = true;
				}
			}
		}
	}	
	
	return Ret;
}

void ClientTaskState::UpdateTasksOnTouch(Client *c, int ZoneID) {

	// If the client has no tasks, there is nothing further to check.
	
	_log(TASKS__UPDATE, "ClientTaskState::UpdateTasksOnTouch(%i)", ZoneID);
	if(ActiveTaskCount == 0) return;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID == TASKSLOTEMPTY) continue;

		// Check if there are any active explore activities for this task

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) return;

		for(int j=0; j<Task->ActivityCount; j++) {
			// We are not interested in completed or hidden activities
			if(ActiveTasks[i].Activity[j].State != ActivityActive) continue;
			// We are only interested in touch activities
			if(Task->Activity[j].Type != ActivityTouch) continue;
			if(Task->Activity[j].GoalMethod != METHODSINGLEID) continue;
			if(Task->Activity[j].ZoneID != ZoneID) {
				_log(TASKS__UPDATE, "Char: %s Touch activity failed zone check",
						    c->GetName());
				continue;
			}
			// We found an active task to zone into this zone, so set done count to goal count
			// (Only a goal count of 1 makes sense for touch activities?)
			_log(TASKS__UPDATE, "Increment on Touch");
			IncrementDoneCount(c, Task, i, j, 
					   Task->Activity[j].GoalCount - ActiveTasks[i].Activity[j].DoneCount);
		}
	}
	
	return;
}
void ClientTaskState::IncrementDoneCount(Client *c, TaskInformation* Task, int TaskIndex, int ActivityID, int Count) {

	_log(TASKS__UPDATE, "IncrementDoneCount");
	
	ActiveTasks[TaskIndex].Activity[ActivityID].DoneCount += Count;

	if(ActiveTasks[TaskIndex].Activity[ActivityID].DoneCount > Task->Activity[ActivityID].GoalCount)
		ActiveTasks[TaskIndex].Activity[ActivityID].DoneCount = Task->Activity[ActivityID].GoalCount;

	char buf[24];
	snprintf(buf, 23, "%d %d %d", ActiveTasks[TaskIndex].Activity[ActivityID].DoneCount, ActiveTasks[TaskIndex].Activity[ActivityID].ActivityID, ActiveTasks[TaskIndex].TaskID);
	buf[23] = '\0';
	parse->EventPlayer(EVENT_TASK_UPDATE, c, buf, 0);

	ActiveTasks[TaskIndex].Activity[ActivityID].Updated=true;
	// Have we reached the goal count for this activity ?
	if(ActiveTasks[TaskIndex].Activity[ActivityID].DoneCount >= Task->Activity[ActivityID].GoalCount) {
		_log(TASKS__UPDATE, "Done (%i) = Goal (%i) for Activity %i",
			  ActiveTasks[TaskIndex].Activity[ActivityID].DoneCount,
			  Task->Activity[ActivityID].GoalCount,
			  ActivityID);

		if(Task->Activity[ActivityID].GoalMethod != METHODQUEST)
		{
			char buf[24];
			snprintf(buf, 23, "%d %d", ActiveTasks[TaskIndex].TaskID, ActiveTasks[TaskIndex].Activity[ActivityID].ActivityID);
			buf[23] = '\0';
            parse->EventPlayer(EVENT_TASK_STAGE_COMPLETE, c, buf, 0);
		}

		// Flag the activity as complete
		ActiveTasks[TaskIndex].Activity[ActivityID].State = ActivityCompleted;
		// Unlock subsequent activities for this task
		bool TaskComplete = UnlockActivities(c->CharacterID(), TaskIndex);
		_log(TASKS__UPDATE, "TaskCompleted is %i", TaskComplete);
		// and by the 'Task Stage Completed' message
		c->SendTaskActivityComplete(ActiveTasks[TaskIndex].TaskID, ActivityID, TaskIndex);
		// Send the updated task/activity list to the client
		taskmanager->SendSingleActiveTaskToClient(c, TaskIndex, TaskComplete, false);
		// Inform the client the task has been updated, both by a chat message
		c->Message(0, "Your task '%s' has been updated.", Task->Title);
		// If this task is now complete, the Completed tasks will have been
		// updated in UnlockActivities. Send the completed task list to the
		// client. This is the same sequence the packets are sent on live.
		if(TaskComplete) {
			char buf[24];
			snprintf(buf, 23, "%d %d %d", ActiveTasks[TaskIndex].Activity[ActivityID].DoneCount, ActiveTasks[TaskIndex].Activity[ActivityID].ActivityID, ActiveTasks[TaskIndex].TaskID);
			buf[23] = '\0';
			parse->EventPlayer(EVENT_TASK_COMPLETE, c, buf, 0);

			taskmanager->SendCompletedTasksToClient(c, this);
			c->SendTaskActivityComplete(ActiveTasks[TaskIndex].TaskID, 0, TaskIndex, false);
			taskmanager->SaveClientState(c, this);
			//c->SendTaskComplete(TaskIndex);
			c->CancelTask(TaskIndex);
			//if(Task->RewardMethod != METHODQUEST) RewardTask(c, Task);
			// If Experience and/or cash rewards are set, reward them from the task even if RewardMethod is METHODQUEST
			RewardTask(c, Task);
			//RemoveTask(c, TaskIndex);
		}

	}
	else
		// Send an update packet for this single activity
		taskmanager->SendTaskActivityLong(c, ActiveTasks[TaskIndex].TaskID, ActivityID, 
							TaskIndex, Task->Activity[ActivityID].Optional);
}

void ClientTaskState::RewardTask(Client *c, TaskInformation *Task) {

	if(!Task || !c) return;

	const Item_Struct* Item;
	vector<int> RewardList;

	switch(Task->RewardMethod) {

		case METHODSINGLEID:
		{
			if(Task->RewardID) {
				c->SummonItem(Task->RewardID);
				Item = database.GetItem(Task->RewardID);
				if(Item)
					c->Message(15, "You receive %s as a reward.", Item->Name);
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
					c->Message(15, "You receive %s as a reward.", Item->Name);
			}
			break;
		}
		default:
		{
			// Nothing special done for METHODQUEST
			break;
		}
	}
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

		string CashMessage;

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
		c->Message(15,CashMessage.c_str());
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

bool ClientTaskState::IsTaskActive(int TaskID) {

	if((ActiveTaskCount == 0) || (TaskID == 0)) return false;

	for(int i=0; i<MAXACTIVETASKS; i++) {

		if(ActiveTasks[i].TaskID==TaskID) return true;

	}

	return false;
}

void ClientTaskState::FailTask(Client *c, int TaskID) {

	_log(TASKS__UPDATE, "FailTask %i, ActiveTaskCount is %i", TaskID, ActiveTaskCount);
	if(ActiveTaskCount == 0) return; 

	for(int i=0; i<MAXACTIVETASKS; i++) {

		if(ActiveTasks[i].TaskID==TaskID) {
			c->SendTaskFailed(ActiveTasks[i].TaskID, i);
			// Remove the task from the client
			c->CancelTask(i);
			return;
		}

	}

}

bool ClientTaskState::IsTaskActivityActive(int TaskID, int ActivityID) {

	_log(TASKS__UPDATE, "ClientTaskState IsTaskActivityActive(%i, %i).", TaskID, ActivityID); 
	// Quick sanity check
	if(ActivityID<0) return false;
	if(ActiveTaskCount == 0) return false;

	int ActiveTaskIndex = -1;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID==TaskID) {
			ActiveTaskIndex = i;
			break;
		}
	}

	// The client does not have this task
	if(ActiveTaskIndex == -1) return false;

	TaskInformation* Task = taskmanager->Tasks[ActiveTasks[ActiveTaskIndex].TaskID];

	// The task is invalid
	if(Task==nullptr) return false;

	// The ActivityID is out of range
	if(ActivityID >= Task->ActivityCount) return false;

	_log(TASKS__UPDATE, "ClientTaskState IsTaskActivityActive(%i, %i). State is %i ", TaskID, ActivityID,
	       ActiveTasks[ActiveTaskIndex].Activity[ActivityID].State); 


	return (ActiveTasks[ActiveTaskIndex].Activity[ActivityID].State == ActivityActive);

}

void ClientTaskState::UpdateTaskActivity(Client *c, int TaskID, int ActivityID, int Count) {

	_log(TASKS__UPDATE, "ClientTaskState UpdateTaskActivity(%i, %i, %i).", TaskID, ActivityID, Count);

	// Quick sanity check
	if((ActivityID<0) || (ActiveTaskCount==0)) return;

	int ActiveTaskIndex = -1;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID==TaskID) {
			ActiveTaskIndex = i;
			break;
		}
	}

	// The client does not have this task
	if(ActiveTaskIndex == -1) return;

	TaskInformation* Task = taskmanager->Tasks[ActiveTasks[ActiveTaskIndex].TaskID];

	// The task is invalid
	if(Task==nullptr) return;

	// The ActivityID is out of range
	if(ActivityID >= Task->ActivityCount) return;

	// The Activity is not currently active
	if(ActiveTasks[ActiveTaskIndex].Activity[ActivityID].State != ActivityActive) return;
	_log(TASKS__UPDATE, "Increment done count on UpdateTaskActivity");
	IncrementDoneCount(c, Task, ActiveTaskIndex, ActivityID, Count);

}

void ClientTaskState::ResetTaskActivity(Client *c, int TaskID, int ActivityID) {

	_log(TASKS__UPDATE, "ClientTaskState UpdateTaskActivity(%i, %i, 0).", TaskID, ActivityID);

	// Quick sanity check
	if((ActivityID<0) || (ActiveTaskCount==0)) return;

	int ActiveTaskIndex = -1;

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID==TaskID) {
			ActiveTaskIndex = i;
			break;
		}
	}

	// The client does not have this task
	if(ActiveTaskIndex == -1) return;

	TaskInformation* Task = taskmanager->Tasks[ActiveTasks[ActiveTaskIndex].TaskID];

	// The task is invalid
	if(Task==nullptr) return;

	// The ActivityID is out of range
	if(ActivityID >= Task->ActivityCount) return;

	// The Activity is not currently active
	if(ActiveTasks[ActiveTaskIndex].Activity[ActivityID].State != ActivityActive) return;

	_log(TASKS__UPDATE, "ResetTaskActivityCount");

	ActiveTasks[ActiveTaskIndex].Activity[ActivityID].DoneCount = 0;

	ActiveTasks[ActiveTaskIndex].Activity[ActivityID].Updated=true;

	// Send an update packet for this single activity
	taskmanager->SendTaskActivityLong(c, ActiveTasks[ActiveTaskIndex].TaskID, ActivityID, 
						ActiveTaskIndex, Task->Activity[ActivityID].Optional);
}

void  ClientTaskState::ShowClientTasks(Client *c) {
	
	c->Message(0, "Task Information:");
	//for(int i=0; i<ActiveTaskCount; i++) {
	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID==TASKSLOTEMPTY) 
			continue;

		c->Message(0, "Task: %i %s", ActiveTasks[i].TaskID, taskmanager->Tasks[ActiveTasks[i].TaskID]->Title);
		c->Message(0, "  Description: [%s]\n", taskmanager->Tasks[ActiveTasks[i].TaskID]->Description);
		for(int j=0; j<taskmanager->GetActivityCount(ActiveTasks[i].TaskID); j++) {
			c->Message(0, "  Activity: %2d, DoneCount: %2d, Status: %d (0=Hidden, 1=Active, 2=Complete)",
			        ActiveTasks[i].Activity[j].ActivityID,
			        ActiveTasks[i].Activity[j].DoneCount,
			        ActiveTasks[i].Activity[j].State);
		}
	}

}

int ClientTaskState::TaskTimeLeft(int TaskID) {

	if(ActiveTaskCount == 0) return -1;

	for(int i=0; i<MAXACTIVETASKS; i++) {

		if(ActiveTasks[i].TaskID != TaskID) continue;

		int Now = time(nullptr);

		TaskInformation* Task = taskmanager->Tasks[ActiveTasks[i].TaskID];

		if(Task == nullptr) return -1;

		if(!Task->Duration) return -1;

		int TimeLeft = (ActiveTasks[i].AcceptedTime + Task->Duration - Now);
		
		// If Timeleft is negative, return 0, else return the number of seconds left
	
		return (TimeLeft>0 ? TimeLeft : 0);
	}

	return -1;
}

int ClientTaskState::IsTaskCompleted(int TaskID) {

	// Returns:	-1 if RecordCompletedTasks is not true
	// 		+1 if the task has been completed
	// 		 0 if the task has not been completed

	if(!(RuleB(TaskSystem, RecordCompletedTasks))) return -1;

	for(unsigned int i=0; i<CompletedTasks.size(); i++) {
		_log(TASKS__UPDATE, "Comparing completed task %i with %i", CompletedTasks[i].TaskID, TaskID);
		if(CompletedTasks[i].TaskID == TaskID) return 1;
	}

	return 0;
}

bool  TaskManager::IsTaskRepeatable(int TaskID) {

	if((TaskID <= 0) || (TaskID >= MAXTASKS)) return false;

	TaskInformation* Task = taskmanager->Tasks[TaskID];

	if(Task == nullptr) return false;

	return Task->Repeatable;
}

bool ClientTaskState::TaskOutOfTime(int Index) {

	// Returns true if the Task in the specified slot has a time limit that has been exceeded.

	if((Index < 0) || (Index>=MAXACTIVETASKS)) return false;

	if((ActiveTasks[Index].TaskID <= 0) || (ActiveTasks[Index].TaskID >= MAXTASKS)) return false;

	int Now = time(nullptr);

	TaskInformation* Task = taskmanager->Tasks[ActiveTasks[Index].TaskID];

	if(Task == nullptr) return false;

	return (Task->Duration && (ActiveTasks[Index].AcceptedTime + Task->Duration <= Now));

}

void ClientTaskState::TaskPeriodicChecks(Client *c) {

	if(ActiveTaskCount == 0) return;

	// Check for tasks that have failed because they have not been completed in the specified time
	//
	for(int i=0; i<MAXACTIVETASKS; i++) {

		if(ActiveTasks[i].TaskID==TASKSLOTEMPTY) continue;

		if(TaskOutOfTime(i)) {
			// Send Red Task Failed Message
			c->SendTaskFailed(ActiveTasks[i].TaskID, i);
			// Remove the task from the client
			c->CancelTask(i);
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
	if(!CheckedTouchActivities) {
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

	_log("[TASKS]SendTasksComplete");
	DumpPacket(outapp); fflush(stdout);

	QueuePacket(outapp);
	safe_delete(outapp);



}
#endif

void ClientTaskState::SendTaskHistory(Client *c, int TaskIndex) {

	_log(TASKS__UPDATE, "Task History Requested for Completed Task Index %i", TaskIndex);

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
						      strlen(Task->Activity[i].Text1) + 1 +
						      strlen(Task->Activity[i].Text2) + 1 +
						      sizeof(TaskHistoryReplyData2_Struct) +
						      strlen(Task->Activity[i].Text3) + 1;
		}
	}
					      


	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskHistoryReply, PacketLength);

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
			sprintf(Ptr, Task->Activity[i].Text1);
			Ptr = Ptr + strlen(Ptr) + 1;
			sprintf(Ptr, Task->Activity[i].Text2);
			Ptr = Ptr + strlen(Ptr) + 1;
			thd2 = (TaskHistoryReplyData2_Struct*)Ptr;
			thd2->GoalCount = Task->Activity[i].GoalCount;
			thd2->unknown04 = 0xffffffff;
			thd2->unknown08 = 0xffffffff;
			thd2->ZoneID = Task->Activity[i].ZoneID;
			thd2->unknown16 = 0x00000000;
			Ptr = (char *)thd2 + sizeof(TaskHistoryReplyData2_Struct);
			sprintf(Ptr, Task->Activity[i].Text3);
			Ptr = Ptr + strlen(Ptr) + 1;
		}
	}

	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);



}

void Client::SendTaskActivityComplete(int TaskID, int ActivityID, int TaskIndex, int TaskIncomplete) {

	// 0x54eb

	TaskActivityComplete_Struct* tac;
	
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	tac = (TaskActivityComplete_Struct*)outapp->pBuffer;

	//tac->unknown1 = 0x00000000;
	tac->TaskIndex = TaskIndex;
	tac->unknown2 = 0x00000002;
	//tac->unknown3 = 0x00000000;
	tac->unknown3 = TaskID; // Correct ?
	tac->ActivityID = ActivityID;
	tac->unknown4 = 0x00000001;
	//tac->unknown5 = 0x00000001;
	tac->unknown5 = TaskIncomplete;

	_pkt(TASKS__PACKETS, outapp);

	QueuePacket(outapp);
	safe_delete(outapp);
}


void Client::SendTaskFailed(int TaskID, int TaskIndex) {

	// 0x54eb
	char buf[24];
	snprintf(buf, 23, "%d", TaskID);
	buf[23] = '\0';
	parse->EventPlayer(EVENT_TASK_FAIL, this, buf, 0);

	TaskActivityComplete_Struct* tac;
	
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskActivityComplete, sizeof(TaskActivityComplete_Struct));

	tac = (TaskActivityComplete_Struct*)outapp->pBuffer;

	//tac->unknown1 = 0x00000000;
	tac->TaskIndex = TaskIndex;
	tac->unknown2 = 0x00000002;
	//tac->unknown3 = 0x00000000;
	tac->unknown3 = TaskID; // Correct ?
	tac->ActivityID = 0;
	tac->unknown4 = 0x00000000; //Fail
	//tac->unknown5 = 0x00000001;
	tac->unknown5 = 0; // 0 for task complete or failed.

	_log(TASKS__UPDATE, "TaskFailed");
	_pkt(TASKS__PACKETS, outapp);

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

	_log(TASKS__UPDATE, "Completed Task Count: %i, First Task to send is %i, Last is %i", 
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
		PacketLength = PacketLength + 8 + strlen(Tasks[TaskID]->Title) + 1;
	}

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_CompletedTasks, PacketLength);
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

		sprintf(buf, "%s", Tasks[TaskID]->Title);
		buf = buf + strlen(buf) + 1;
		//*(uint32 *)buf = (*iterator).CompletedTime;
		*(uint32 *)buf = State->CompletedTasks[i].CompletedTime;
		buf = buf + 4;
	}

	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);
}


	
void TaskManager::SendTaskActivityShort(Client *c, int TaskID, int ActivityID, int ClientTaskIndex)
{
	// This Activity Packet is sent for activities that have not yet been unlocked and appear as ???
	// in the client.

	TaskActivityShort_Struct* tass;

	if(c->GetClientVersionBit() & BIT_RoFAndLater)
	{
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskActivity, 25);
		outapp->WriteUInt32(ClientTaskIndex);
		outapp->WriteUInt32(2);
		outapp->WriteUInt32(TaskID);
		outapp->WriteUInt32(ActivityID);
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(0xffffffff);
		outapp->WriteUInt8(0);
		c->FastQueuePacket(&outapp);
		
		return;
	}

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskActivity, sizeof(TaskActivityShort_Struct));

	tass = (TaskActivityShort_Struct*)outapp->pBuffer;

	tass->TaskSequenceNumber = ClientTaskIndex;
	tass->unknown2 = 0x00000002;
	tass->TaskID = TaskID;
	tass->ActivityID = ActivityID;
	tass->unknown3 = 0x000000;
	tass->ActivityType = 0xffffffff;
	tass->unknown4 = 0x00000000;

	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);
}



void TaskManager::SendTaskActivityLong(Client *c, int TaskID, int ActivityID, int ClientTaskIndex, bool Optional, bool TaskComplete) {

	if (c->GetClientVersion() >= EQClientRoF)
	{
		SendTaskActivityNew(c, TaskID, ActivityID, ClientTaskIndex, Optional, TaskComplete);
		return;
	}

	char *Ptr;

	TaskActivityHeader_Struct* tah;
	TaskActivityData1_Struct* tad1;
	TaskActivityTrailer_Struct* tat;

	long PacketLength = sizeof(TaskActivityHeader_Struct) + +sizeof(TaskActivityData1_Struct) + sizeof(TaskActivityTrailer_Struct);
	PacketLength = PacketLength + strlen(Tasks[TaskID]->Activity[ActivityID].Text1) + 1 + 
		       strlen(Tasks[TaskID]->Activity[ActivityID].Text2) + 1 +
		       strlen(Tasks[TaskID]->Activity[ActivityID].Text3) + 1;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskActivity, PacketLength);

	tah = (TaskActivityHeader_Struct*)outapp->pBuffer;

	tah->TaskSequenceNumber = ClientTaskIndex;
	tah->unknown2 = 0x00000002;
	tah->TaskID = TaskID;
	tah->ActivityID = ActivityID;
	tah->unknown3 = 0x00000000;

	// We send our 'internal' types as ActivityUse1. text3 should be set to the activity description, so it makes
	// no difference to the client. All activity updates will be done based on our interal activity types.
	if((Tasks[TaskID]->Activity[ActivityID].Type > 0) &&  Tasks[TaskID]->Activity[ActivityID].Type < 100)
		tah->ActivityType = Tasks[TaskID]->Activity[ActivityID].Type;
	else
		tah->ActivityType = ActivityUse1;

	tah->Optional = Optional;
	tah->unknown5 = 0x00000000;
	// One of these unknown fields maybe related to the 'Use On' activity types
	Ptr = (char *) tah + sizeof(TaskActivityHeader_Struct);
	sprintf(Ptr, "%s", Tasks[TaskID]->Activity[ActivityID].Text1);
	Ptr = Ptr + strlen(Ptr) + 1;

	sprintf(Ptr, "%s", Tasks[TaskID]->Activity[ActivityID].Text2);
	Ptr = Ptr + strlen(Ptr) + 1;

	tad1 = (TaskActivityData1_Struct*)Ptr;
	tat = (TaskActivityTrailer_Struct*)Ptr;

	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		tad1->GoalCount = Tasks[TaskID]->Activity[ActivityID].GoalCount;
	else
		// For our internal type GiveCash, where the goal count has the amount of cash that must be given,
		// we don't want the donecount and goalcount fields cluttered up with potentially large numbers, so we just
		// send a goalcount of 1, and a bit further down, a donecount of 1 if the activity is complete, 0 otherwise.
		// The text3 field should decribe the exact activity goal, e.g. give 3500gp to Hasten Bootstrutter.
		tad1->GoalCount = 1;

	tad1->unknown1 = 0xffffffff;
	if(!TaskComplete) tad1->unknown2 = 0xffffffff;
	else
		tad1->unknown2 = 0xcaffffff;

	tad1->ZoneID = Tasks[TaskID]->Activity[ActivityID].ZoneID;
	tad1->unknown3 = 0x00000000;

	Ptr = (char *) tad1 + sizeof(TaskActivityData1_Struct);
	sprintf(Ptr, "%s", Tasks[TaskID]->Activity[ActivityID].Text3);
	Ptr = Ptr + strlen(Ptr) + 1;

	tat = (TaskActivityTrailer_Struct*)Ptr;

	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		tat->DoneCount = c->GetTaskActivityDoneCount(ClientTaskIndex, ActivityID);
	else
		// For internal activity types, DoneCount is either 1 if the activity is complete, 0 otherwise.
		tat->DoneCount = (c->GetTaskActivityDoneCount(ClientTaskIndex, ActivityID) >= Tasks[TaskID]->Activity[ActivityID].GoalCount);

	tat->unknown1 = 0x00000001;

	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

// Used only by RoF+ Clients
void TaskManager::SendTaskActivityNew(Client *c, int TaskID, int ActivityID, int ClientTaskIndex, bool Optional, bool TaskComplete) {
	
	uint32 String2Len = 3;
	if(TaskComplete)
		String2Len = 4;
		
	long PacketLength = 29 + 4 + 8 + 4 + 4 + 5;
	PacketLength = PacketLength + strlen(Tasks[TaskID]->Activity[ActivityID].Text1) + 1 + 
		       strlen(Tasks[TaskID]->Activity[ActivityID].Text2) + 1 +
		       strlen(Tasks[TaskID]->Activity[ActivityID].Text3) + 1 +
			   ((strlen(itoa(Tasks[TaskID]->Activity[ActivityID].ZoneID)) + 1) * 2) +
			   3 + String2Len;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskActivity, PacketLength);

	outapp->WriteUInt32(ClientTaskIndex);	// TaskSequenceNumber
	outapp->WriteUInt32(2);		// unknown2
	outapp->WriteUInt32(TaskID);
	outapp->WriteUInt32(ActivityID);
	outapp->WriteUInt32(0);		// unknown3

	// We send our 'internal' types as ActivityUse1. text3 should be set to the activity description, so it makes
	// no difference to the client. All activity updates will be done based on our interal activity types.
	if((Tasks[TaskID]->Activity[ActivityID].Type > 0) &&  Tasks[TaskID]->Activity[ActivityID].Type < 100)
		outapp->WriteUInt32(Tasks[TaskID]->Activity[ActivityID].Type);
	else
		outapp->WriteUInt32(ActivityUse1);

	outapp->WriteUInt32(Optional);
	outapp->WriteUInt8(0);		// unknown5

	// One of these unknown fields maybe related to the 'Use On' activity types
	outapp->WriteString(Tasks[TaskID]->Activity[ActivityID].Text1);

	outapp->WriteUInt32((strlen(Tasks[TaskID]->Activity[ActivityID].Text2) + 1));	// String Length - Add in null terminator
	outapp->WriteString(Tasks[TaskID]->Activity[ActivityID].Text2);

	// Goal Count
	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		outapp->WriteUInt32(Tasks[TaskID]->Activity[ActivityID].GoalCount);
	else
		outapp->WriteUInt32(1);	// GoalCount
		
	outapp->WriteUInt32(3);		// String Length - Add in null terminator
	outapp->WriteString("-1");

	if(!TaskComplete) {
		outapp->WriteUInt32(3);	// String Length - Add in null terminator
		outapp->WriteString("-1");
	}
	else
	{
		outapp->WriteUInt32(4);	// String Length - Add in null terminator
		outapp->WriteString("-54");
	}
	
	outapp->WriteString(itoa(Tasks[TaskID]->Activity[ActivityID].ZoneID));
	outapp->WriteUInt32(0);		// unknown7

	outapp->WriteString(Tasks[TaskID]->Activity[ActivityID].Text3);
	
	if(Tasks[TaskID]->Activity[ActivityID].Type != ActivityGiveCash)
		outapp->WriteUInt32(c->GetTaskActivityDoneCount(ClientTaskIndex, ActivityID));	// DoneCount
	else
		// For internal activity types, DoneCount is either 1 if the activity is complete, 0 otherwise.
		outapp->WriteUInt32((c->GetTaskActivityDoneCount(ClientTaskIndex, ActivityID) >= Tasks[TaskID]->Activity[ActivityID].GoalCount));

	outapp->WriteUInt8(1);	// unknown9
	
	outapp->WriteString(itoa(Tasks[TaskID]->Activity[ActivityID].ZoneID));

	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

void TaskManager::SendActiveTasksToClient(Client *c, bool TaskComplete) {

	//for(int TaskIndex=0; TaskIndex<c->GetActiveTaskCount(); TaskIndex++) {
	for(int TaskIndex=0; TaskIndex<MAXACTIVETASKS; TaskIndex++) {
		int TaskID = c->GetActiveTaskID(TaskIndex);
		if((TaskID==0) || (Tasks[TaskID] ==0))  continue;
		int StartTime = c->GetTaskStartTime(TaskIndex);
		
		SendActiveTaskDescription(c, TaskID, TaskIndex, StartTime, Tasks[TaskID]->Duration, false);
		_log(TASKS__UPDATE, "SendActiveTasksToClient: Task %i, Activities: %i", TaskID, GetActivityCount(TaskID));

		int Sequence = 0;
		for(int Activity=0; Activity<GetActivityCount(TaskID); Activity++) {
			if(c->GetTaskActivityState(TaskIndex, Activity) != ActivityHidden) {
				_log(TASKS__UPDATE, "  Long: %i, %i, %i Complete=%i", TaskID, Activity, TaskIndex, TaskComplete);
				if(Activity==GetActivityCount(TaskID)-1)
					SendTaskActivityLong(c, TaskID, Activity, TaskIndex, 
							     Tasks[TaskID]->Activity[Activity].Optional,
							     TaskComplete);
				else
					SendTaskActivityLong(c, TaskID, Activity, TaskIndex, 
							     Tasks[TaskID]->Activity[Activity].Optional, 0);
			}
			else {
				_log(TASKS__UPDATE, "  Short: %i, %i, %i", TaskID, Activity, TaskIndex);
				SendTaskActivityShort(c, TaskID, Activity, TaskIndex);
			}
			Sequence++;
		}
			

	}
}


void TaskManager::SendSingleActiveTaskToClient(Client *c, int TaskIndex, bool TaskComplete, bool BringUpTaskJournal) {

	if((TaskIndex < 0) || (TaskIndex >= MAXACTIVETASKS)) return;

	int TaskID = c->GetActiveTaskID(TaskIndex);

	if((TaskID==0) || (Tasks[TaskID] ==0))  return;

	int StartTime = c->GetTaskStartTime(TaskIndex);
	SendActiveTaskDescription(c, TaskID, TaskIndex, StartTime, Tasks[TaskID]->Duration, BringUpTaskJournal);
	_log(TASKS__UPDATE, "SendSingleActiveTasksToClient: Task %i, Activities: %i", TaskID, GetActivityCount(TaskID));

	int Sequence = 0;

	for(int Activity=0; Activity<GetActivityCount(TaskID); Activity++) {
		if(c->GetTaskActivityState(TaskIndex, Activity) != ActivityHidden) {
			_log(TASKS__UPDATE, "  Long: %i, %i, %i Complete=%i", TaskID, Activity, TaskIndex, TaskComplete);
			if(Activity==GetActivityCount(TaskID)-1)
				SendTaskActivityLong(c, TaskID, Activity, TaskIndex, 
						     Tasks[TaskID]->Activity[Activity].Optional, TaskComplete);
			else
				SendTaskActivityLong(c, TaskID, Activity, TaskIndex, 
						     Tasks[TaskID]->Activity[Activity].Optional, 0);
		}
		else {
			_log(TASKS__UPDATE, "  Short: %i, %i, %i", TaskID, Activity, TaskIndex);
			SendTaskActivityShort(c, TaskID, Activity, TaskIndex);
		}
		Sequence++;
	}
}

void TaskManager::SendActiveTaskDescription(Client *c, int TaskID, int SequenceNumber, int StartTime, int Duration, bool BringUpTaskJournal) {


	if((TaskID<1) || (TaskID>=MAXTASKS) || !Tasks[TaskID]) return;

	int PacketLength = sizeof(TaskDescriptionHeader_Struct) + strlen(Tasks[TaskID]->Title) + 1
	                   + sizeof(TaskDescriptionData1_Struct) + strlen(Tasks[TaskID]->Description) + 1
			   + sizeof(TaskDescriptionData2_Struct) + 1 + sizeof(TaskDescriptionTrailer_Struct);

	string RewardText;
	int ItemID = 0;

	// If there is an item make the Reward text into a link to the item (only the first item if a list
	// is specified). I have been unable to get multiple item links to work.
	//
	if(Tasks[TaskID]->RewardID) {
		// If the reward is a list of items, and the first entry on the list is valid
		if(Tasks[TaskID]->RewardMethod==METHODSINGLEID) {
			ItemID = Tasks[TaskID]->RewardID;
		}
		else if(Tasks[TaskID]->RewardMethod==METHODLIST) {
			ItemID = GoalListManager.GetFirstEntry(Tasks[TaskID]->RewardID);
			if(ItemID < 0)
				ItemID = 0;
		}
		if(ItemID) {
			char *RewardTmp = 0;
			if(strlen(Tasks[TaskID]->Reward) != 0) {

				switch(c->GetClientVersion()) {

					case EQClient62:
					{
						MakeAnyLenString(&RewardTmp, "%c%07i-00001-00001-00001-00001-000013E0ABA6B%s%c", 
								 0x12, ItemID, Tasks[TaskID]->Reward,0x12);
						break;
					}
					case EQClientTitanium:
					{
						MakeAnyLenString(&RewardTmp, "%c%06X000000000000000000000000000000014505DC2%s%c", 
								 0x12, ItemID, Tasks[TaskID]->Reward,0x12);
						break;
					}
					case EQClientRoF:
					{
						MakeAnyLenString(&RewardTmp, "%c%06X0000000000000000000000000000000000000000014505DC2%s%c", 
								 0x12, ItemID, Tasks[TaskID]->Reward,0x12);
						break;
					}
					default:
					{
						// All clients after Titanium
						MakeAnyLenString(&RewardTmp, "%c%06X00000000000000000000000000000000000014505DC2%s%c", 
								 0x12, ItemID, Tasks[TaskID]->Reward,0x12);
					}
				}

			}
			else {
				const Item_Struct *Item = database.GetItem(ItemID);

				if(Item) {

					switch(c->GetClientVersion()) {

						case EQClient62:
						{
							MakeAnyLenString(&RewardTmp, "%c%07i-00001-00001-00001-00001-000013E0ABA6B%s%c", 
									 0x12, ItemID, Item->Name,0x12);
							break;
						}
						case EQClientTitanium:
						{
							MakeAnyLenString(&RewardTmp, "%c%06X000000000000000000000000000000014505DC2%s%c", 
									 0x12, ItemID, Item->Name ,0x12);
							break;
						}
						case EQClientRoF:
						{
							MakeAnyLenString(&RewardTmp, "%c%06X0000000000000000000000000000000000000000014505DC2%s%c", 
									 0x12, ItemID, Item->Name ,0x12);
							break;
						}
						default:
						{
							// All clients after Titanium
							MakeAnyLenString(&RewardTmp, "%c%06X00000000000000000000000000000000000014505DC2%s%c", 
									 0x12, ItemID, Item->Name ,0x12);
						}
					}
				}
			}

			if(RewardTmp) RewardText += RewardTmp;
			safe_delete_array(RewardTmp);
		}
		else {
			RewardText += Tasks[TaskID]->Reward;	
		}

	}
	else {
		RewardText += Tasks[TaskID]->Reward;	
	}
	PacketLength += strlen(RewardText.c_str()) + 1;

	char *Ptr;
	TaskDescriptionHeader_Struct* tdh;
	TaskDescriptionData1_Struct* tdd1;
	TaskDescriptionData2_Struct* tdd2;
	TaskDescriptionTrailer_Struct* tdt;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TaskDescription, PacketLength);

	tdh = (TaskDescriptionHeader_Struct*)outapp->pBuffer;

	tdh->SequenceNumber = SequenceNumber;
	tdh->TaskID = TaskID;

	if(BringUpTaskJournal)
		tdh->unknown2 = 0x00000201;
	else
		tdh->unknown2 = 0x00000200;
		//tdh->unknown2 = 0x00000100; // This makes the Task Description have an S instead of Q, but the description doesn't show

	tdh->unknown3 = 0x00000000;
	tdh->unknown4 = 0x00;

	Ptr = (char *) tdh + sizeof(TaskDescriptionHeader_Struct);

	sprintf(Ptr, "%s", Tasks[TaskID]->Title);
	Ptr = Ptr + strlen(Ptr) + 1;

	tdd1 = (TaskDescriptionData1_Struct*)Ptr;

	tdd1->Duration = Duration;
	tdd1->unknown2 = 0x00000000;
	
	tdd1->StartTime = StartTime;

	Ptr = (char *) tdd1 + sizeof(TaskDescriptionData1_Struct);

	sprintf(Ptr, "%s", Tasks[TaskID]->Description);
	Ptr = Ptr + strlen(Ptr) + 1;

	tdd2 = (TaskDescriptionData2_Struct*)Ptr;

	// This next field may not be a reward count. It is always 1 in the packets I have seen. Setting it to 2 and trying
	// to include multiple item links has so far proven futile.  Setting it to 0 ends the packet after the next byte.
	tdd2->RewardCount = 1;  
	
	if(Tasks[TaskID]->XPReward)
		tdd2->unknown1 = 0x00000100;
	else
		tdd2->unknown1 = 0x00000000;

	tdd2->unknown2 = 0x00000000;
	tdd2->unknown3 = 0x0000;
	Ptr = (char *) tdd2 + sizeof(TaskDescriptionData2_Struct);

	sprintf(Ptr, "%s", RewardText.c_str());
	Ptr = Ptr + strlen(Ptr) + 1;

	tdt = (TaskDescriptionTrailer_Struct*)Ptr;
	tdt->Points = 0x00000000; // Points Count 

	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);

}

bool ClientTaskState::IsTaskActivityCompleted(int index, int ActivityID) {
	
	return (ActiveTasks[index].Activity[ActivityID].State == ActivityCompleted);
}

ActivityState ClientTaskState::GetTaskActivityState(int index, int ActivityID) {


	return ActiveTasks[index].Activity[ActivityID].State;
}

int ClientTaskState::GetTaskActivityDoneCount(int index, int ActivityID) {

	return ActiveTasks[index].Activity[ActivityID].DoneCount;

}

int ClientTaskState::GetTaskActivityDoneCountFromTaskID(int TaskID, int ActivityID){
	int ActiveTaskIndex = -1;
	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID==TaskID) {
			ActiveTaskIndex = i;
			break;
		}
	}
	return ActiveTasks[ActiveTaskIndex].Activity[ActivityID].DoneCount;
}

int ClientTaskState::GetTaskStartTime(int index) {

	return ActiveTasks[index].AcceptedTime;

}

void ClientTaskState::CancelAllTasks(Client *c) {

	// This method exists solely to be called during #task reloadall
	// It removes tasks from the in-game client state ready for them to be 
	// resent to the client, in case an updated task fails to load

	for(int i=0; i<MAXACTIVETASKS; i++) 
		if(ActiveTasks[i].TaskID != TASKSLOTEMPTY) {
			CancelTask(c, i, false);
			ActiveTasks[i].TaskID = TASKSLOTEMPTY;
		}
	

}
void ClientTaskState::CancelTask(Client *c, int SequenceNumber, bool RemoveFromDB) {

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_CancelTask, sizeof(CancelTask_Struct));

	CancelTask_Struct* cts = (CancelTask_Struct*)outapp->pBuffer;
	cts->SequenceNumber = SequenceNumber;
	cts->unknown4 = 0x00000002;

	_log(TASKS__UPDATE, "CancelTask");
	_pkt(TASKS__PACKETS, outapp);

	c->QueuePacket(outapp);
	safe_delete(outapp);

	if(RemoveFromDB)
		RemoveTask(c, SequenceNumber);
}

void ClientTaskState::RemoveTask(Client *c, int SequenceNumber) {

	int CharacterID = c->CharacterID();

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	const char *TaskQuery="DELETE FROM character_tasks WHERE charid=%i AND taskid = %i";

	const char *ActivityQuery="DELETE FROM character_activities WHERE charid=%i AND taskid = %i";

	_log(TASKS__UPDATE, "ClientTaskState Cancel Task %i ", SequenceNumber); 
		
	if(!database.RunQuery(query,MakeAnyLenString(&query, ActivityQuery,
						      CharacterID,
					              ActiveTasks[SequenceNumber].TaskID), errbuf)) {

		LogFile->write(EQEMuLog::Error, "[TASKS]Error in CientTaskState::CancelTask %s", errbuf);
		safe_delete_array(query);
		return;
	}
	_log(TASKS__UPDATE, "CancelTask: %s", query);
	safe_delete_array(query);
	if(!database.RunQuery(query,MakeAnyLenString(&query, TaskQuery,
						      CharacterID,
					              ActiveTasks[SequenceNumber].TaskID), errbuf)) {

		LogFile->write(EQEMuLog::Error, "[TASKS]Error in CientTaskState::CancelTask %s", errbuf);
	}

	_log(TASKS__UPDATE, "CancelTask: %s", query);
	safe_delete_array(query);

	ActiveTasks[SequenceNumber].TaskID = TASKSLOTEMPTY;
	ActiveTaskCount--;
}


void ClientTaskState::AcceptNewTask(Client *c, int TaskID, int NPCID) {
	
	if(!taskmanager || TaskID<0 || TaskID>=MAXTASKS) {
		c->Message(13, "Task system not functioning, or TaskID %i out of range.", TaskID);
		return;

	}

	if(taskmanager->Tasks[TaskID] == nullptr) {
		c->Message(13, "Invalid TaskID %i", TaskID);
		return;
	}

	if(ActiveTaskCount==MAXACTIVETASKS) {
		c->Message(13, "You already have the maximum allowable number of active tasks (%i)", MAXACTIVETASKS);
		return;
	}

	for(int i=0; i<MAXACTIVETASKS; i++) {
		if(ActiveTasks[i].TaskID==TaskID) {
			c->Message(13, "You have already been assigned this task.");
			return;
		}
	}

	if(!taskmanager->IsTaskRepeatable(TaskID) && IsTaskCompleted(TaskID)) return;

	// We do it this way, because when the Client cancels a task, it retains the sequence number of the remaining
	// tasks in it's window, until something causes the TaskDescription packets to be sent again. We could just
	// resend all the active task data to the client when it cancels a task, but that could be construed as a
	// waste of bandwidth.
	//
	int FreeSlot = -1;
	for(int i=0; i<MAXACTIVETASKS; i++) {
		_log(TASKS__UPDATE, "ClientTaskState Looking for free slot in slot %i, found TaskID of %i ",
		       i, ActiveTasks[i].TaskID);
		if(ActiveTasks[i].TaskID == 0) {
			FreeSlot = i;
			break;
		}
	}

	// This shouldn't happen unless there is a bug in the handling of ActiveTaskCount somewhere
	if(FreeSlot == -1) {
		c->Message(13, "You already have the maximum allowable number of active tasks (%i)", MAXACTIVETASKS);
		return;
	}
	

	ActiveTasks[FreeSlot].TaskID = TaskID;
	ActiveTasks[FreeSlot].AcceptedTime = time(nullptr);
	ActiveTasks[FreeSlot].Updated = true;
	ActiveTasks[FreeSlot].CurrentStep = -1;
	
	for(int i=0; i<taskmanager->Tasks[TaskID]->ActivityCount; i++) {
		ActiveTasks[FreeSlot].Activity[i].ActivityID = i;
		ActiveTasks[FreeSlot].Activity[i].DoneCount = 0;
		ActiveTasks[FreeSlot].Activity[i].State = ActivityHidden;
		ActiveTasks[FreeSlot].Activity[i].Updated = true;
	}
	UnlockActivities(c->CharacterID(), FreeSlot);
	ActiveTaskCount++;
	taskmanager->SendSingleActiveTaskToClient(c, FreeSlot, false, true);
	c->Message(0, "You have been assigned the task '%s'.", taskmanager->Tasks[TaskID]->Title);

	char *buf = 0;
	MakeAnyLenString(&buf, "%d", TaskID);

	NPC *npc = entity_list.GetID(NPCID)->CastToNPC();
	if(!npc) {
		c->Message(clientMessageYellow, "Task Giver ID is %i", NPCID);
		c->Message(clientMessageError, "Unable to find NPC to send EVENT_TASKACCEPTD to. Report this bug.");
		safe_delete_array(buf);
		return;
	}
	taskmanager->SaveClientState(c, this);
    parse->EventNPC(EVENT_TASKACCEPTED, npc, c, buf, 0);
	safe_delete_array(buf);

}

void ClientTaskState::ProcessTaskProximities(Client *c, float X, float Y, float Z) {

	float LastX = c->ProximityX();
	float LastY = c->ProximityY();
	float LastZ = c->ProximityZ();

	//_log(TASKS__PROXIMITY, "Checing proximities for Position %8.3f, %8.3f, %8.3f Last: %8.3f, %8.3f, %8.3f\n", X, Y, Z, LastX, LastY, LastZ);
	if((LastX==X) && (LastY==Y) && (LastZ==Z)) return;

	_log(TASKS__PROXIMITY, "Checing proximities for Position %8.3f, %8.3f, %8.3f\n", X, Y, Z);
	int ExploreID = taskmanager->ProximityManager.CheckProximities(X, Y, Z);

	if(ExploreID>0) {
		_log(TASKS__PROXIMITY, "Position %8.3f, %8.3f, %8.3f is within proximity %i\n", X, Y, Z, ExploreID);
		UpdateTasksOnExplore(c, ExploreID);
	}
}


	

TaskGoalListManager::TaskGoalListManager() {

	TaskGoalLists = nullptr;
	NumberOfLists = 0;

}

TaskGoalListManager::~TaskGoalListManager() {

	for(int i=0; i< NumberOfLists; i++) {

		safe_delete_array(TaskGoalLists[i].GoalItemEntries);

	}
	safe_delete_array(TaskGoalLists);
}

bool TaskGoalListManager::LoadLists() {


	const char *CountQuery = "SELECT `listid`, COUNT(`entry`) FROM `goallists` GROUP by `listid` "
	                         "ORDER BY `listid`";

	const char *ListQuery = "SELECT `entry` from `goallists` WHERE `listid`=%i "
	                         "ORDER BY `entry` ASC LIMIT %i";

	const char *ERR_MYSQLERROR = "Error in TaskGoalListManager::LoadLists: %s %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	_log(TASKS__GLOBALLOAD, "TaskGoalListManager::LoadLists Called"); 

	for(int i=0; i< NumberOfLists; i++) {

		safe_delete_array(TaskGoalLists[i].GoalItemEntries);

	}
	safe_delete_array(TaskGoalLists);

	NumberOfLists = 0;

	if(database.RunQuery(query,MakeAnyLenString(&query,CountQuery),errbuf,&result)) {

		NumberOfLists = mysql_num_rows(result);
		_log(TASKS__GLOBALLOAD, "Database returned a count of %i lists", NumberOfLists);

		TaskGoalLists = new TaskGoalList_Struct[NumberOfLists];
	
		int ListIndex = 0;

		while((row = mysql_fetch_row(result))) {
			int ListID   = atoi(row[0]);
			int ListSize = atoi(row[1]);

			TaskGoalLists[ListIndex].ListID = ListID;
			TaskGoalLists[ListIndex].Size = ListSize;
			TaskGoalLists[ListIndex].Min = 0;
			TaskGoalLists[ListIndex].Max = 0;
			TaskGoalLists[ListIndex].GoalItemEntries = new int[ListSize];

			ListIndex++;
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, query, errbuf);
		safe_delete_array(query);
		return false;
	}

	for(int ListIndex = 0; ListIndex < NumberOfLists; ListIndex++) {

		int ListID = TaskGoalLists[ListIndex].ListID;
		unsigned int Size = TaskGoalLists[ListIndex].Size;

		if(database.RunQuery(query,MakeAnyLenString(&query,ListQuery,ListID,Size),errbuf,&result)) {
			// This should only happen if a row is deleted in between us retrieving the counts
			// at the start of this method and getting to here. It should not be possible for
			// an INSERT to cause a problem, as the SELECT is used with a LIMIT
			if(mysql_num_rows(result) < Size) 
				TaskGoalLists[ListIndex].Size = mysql_num_rows(result);

			int EntryIndex = 0;

			while((row = mysql_fetch_row(result))) {

				int Entry = atoi(row[0]);

				if(Entry < TaskGoalLists[ListIndex].Min)
					TaskGoalLists[ListIndex].Min = Entry;

				if(Entry > TaskGoalLists[ListIndex].Max)
					TaskGoalLists[ListIndex].Max = Entry;

				TaskGoalLists[ListIndex].GoalItemEntries[EntryIndex++] = Entry;

			}

			mysql_free_result(result);
			safe_delete_array(query);
		}
		else {
			LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, query, errbuf);
			TaskGoalLists[ListIndex].Size = 0;
			safe_delete_array(query);
		}
	}
	return true;

}

int TaskGoalListManager::GetListByID(int ListID) {

	// Find the list with the specified ListID and return the index
	
	int FirstEntry = 0;
	int LastEntry = NumberOfLists - 1;

	while(FirstEntry <= LastEntry) {
		int MiddleEntry = (FirstEntry + LastEntry) / 2;

		if(ListID > TaskGoalLists[MiddleEntry].ListID)
			FirstEntry = MiddleEntry + 1;
		else if(ListID < TaskGoalLists[MiddleEntry].ListID)
			LastEntry = MiddleEntry - 1;
		else
			return MiddleEntry;

	}

	return -1;

}

int TaskGoalListManager::GetFirstEntry(int ListID) {

	int ListIndex = GetListByID(ListID);

	if((ListIndex < 0) || (ListIndex >= NumberOfLists)) return -1;

	if(TaskGoalLists[ListIndex].Size == 0) return -1;

	return TaskGoalLists[ListIndex].GoalItemEntries[0];
}

vector<int> TaskGoalListManager::GetListContents(int ListID) {

	vector<int> ListContents;

	int ListIndex = GetListByID(ListID);

	if((ListIndex < 0) || (ListIndex >= NumberOfLists)) return ListContents;

	for(int i=0; i<TaskGoalLists[ListIndex].Size; i++) 
		ListContents.push_back(TaskGoalLists[ListIndex].GoalItemEntries[i]);

	return ListContents;

}



bool TaskGoalListManager::IsInList(int ListID, int Entry) {

	_log(TASKS__UPDATE, "TaskGoalListManager::IsInList(%i, %i)", ListID, Entry);

	int ListIndex = GetListByID(ListID);

	if((ListIndex<0) || (ListIndex >= NumberOfLists)) return false;

	if((Entry < TaskGoalLists[ListIndex].Min) ||
	   (Entry > TaskGoalLists[ListIndex].Max))
	   	return false;

	int FirstEntry = 0;
	int LastEntry = TaskGoalLists[ListIndex].Size - 1;

	while(FirstEntry <= LastEntry) {
		int MiddleEntry = (FirstEntry + LastEntry) / 2;

		if(Entry > TaskGoalLists[ListIndex].GoalItemEntries[MiddleEntry])
			FirstEntry = MiddleEntry + 1;
		else if(Entry < TaskGoalLists[ListIndex].GoalItemEntries[MiddleEntry])
			LastEntry = MiddleEntry - 1;
		else {
			_log(TASKS__UPDATE, "TaskGoalListManager::IsInList(%i, %i) returning true", ListIndex, Entry);
			return true;
		}

	}

	return false;

}

TaskProximityManager::TaskProximityManager() {


}

TaskProximityManager::~TaskProximityManager() {


}

bool TaskProximityManager::LoadProximities(int ZoneID) {

	const char *ProximityQuery = "SELECT `exploreid`, `minx`, `maxx`, `miny`, `maxy`, "
				     "`minz`, `maxz` from `proximities` WHERE `zoneid`=%i "
				     "ORDER BY `zoneid` ASC";

	const char *ERR_MYSQLERROR = "Error in TaskProximityManager::LoadProximities %s %s";

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;


	TaskProximity Proximity;

	_log(TASKS__GLOBALLOAD, "TaskProximityManager::LoadProximities Called for zone %i", ZoneID); 
	TaskProximities.clear();

	if(database.RunQuery(query,MakeAnyLenString(&query,ProximityQuery, ZoneID),errbuf,&result)) {

		while((row = mysql_fetch_row(result))) {
			Proximity.ExploreID = atoi(row[0]);
			Proximity.MinX = atof(row[1]);
			Proximity.MaxX = atof(row[2]);
			Proximity.MinY = atof(row[3]);
			Proximity.MaxY = atof(row[4]);
			Proximity.MinZ = atof(row[5]);
			Proximity.MaxZ = atof(row[6]);

			TaskProximities.push_back(Proximity);

		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, ERR_MYSQLERROR, query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	return true;

}

int TaskProximityManager::CheckProximities(float X, float Y, float Z) {

	for(unsigned int i=0; i<TaskProximities.size(); i++) {

		TaskProximity* P = &TaskProximities[i];

		_log(TASKS__PROXIMITY, "Checking %8.3f, %8.3f, %8.3f against %8.3f, %8.3f, %8.3f, %8.3f, %8.3f, %8.3f",
			   X, Y, Z, P->MinX, P->MaxX, P->MinY, P->MaxY, P->MinZ, P->MaxZ);

		if(X < P->MinX || X > P->MaxX || Y < P->MinY || Y > P->MaxY ||
		   Z < P->MinZ || Z > P->MaxZ) continue;

		return P->ExploreID;

	}

	return 0;
}


