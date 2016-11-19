/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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


#ifndef TASKS_H
#define TASKS_H

#include "../common/types.h"

#include <list>
#include <vector>

#define MAXTASKS 10000
#define MAXTASKSETS 1000
// The Client has a hard cap of 19 active tasks
#define MAXACTIVETASKS 19
// The Max Chooser (Task Selector entries) is capped at 40 in the Titanium Client.
#define MAXCHOOSERENTRIES 40
// The Client has a hard cap of 20 activities per task.
#define MAXACTIVITIESPERTASK 20
// This is used to determine if a client's active task slot is empty.
#define TASKSLOTEMPTY 0

// Command Codes for worldserver ServerOP_ReloadTasks
//
#define RELOADTASKS		0
#define RELOADTASKGOALLISTS	1
#define RELOADTASKPROXIMITIES	2
#define RELOADTASKSETS		3

class Client;
class Mob;

namespace EQEmu
{
	class ItemInstance;
}

struct TaskGoalList_Struct {
	int ListID;
	int Size;
	int Min, Max;
	int *GoalItemEntries;
};

// This is used for handling lists, loading them from the database, searching them.
// Used for lists of NPCs to kill, items to loot, etc, as well as lists of items to
// reward the player with on completion of the task.
//
class TaskGoalListManager {

public:
	TaskGoalListManager();
	~TaskGoalListManager();
	bool LoadLists();
	int GetListByID(int ListID);
	bool IsInList(int ListID, int Entry);
	int GetFirstEntry(int ListID);
	std::vector<int> GetListContents(int ListIndex);

private:

	TaskGoalList_Struct *TaskGoalLists;
	int NumberOfLists;
};

typedef struct {
	int ExploreID;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
} TaskProximity;

// This class is used for managing proximities so that Quest NPC proximities don't need to be used.
class TaskProximityManager {

public:
	TaskProximityManager();
	~TaskProximityManager();
	bool LoadProximities(int ZoneID);
	int CheckProximities(float X, float Y, float Z);

private:
	std::vector<TaskProximity> TaskProximities;
};

typedef enum { METHODSINGLEID = 0, METHODLIST = 1, METHODQUEST = 2 } TaskMethodType;

struct ActivityInformation {
	int		StepNumber;
	int		Type;
	char	*Text1;
	char	*Text2;
	char	*Text3;
	int		GoalID;
	TaskMethodType GoalMethod;
	int		GoalCount;
	int		DeliverToNPC;
	int		ZoneID;
	bool	Optional;
};

typedef enum { ActivitiesSequential = 0, ActivitiesStepped = 1 } SequenceType;

struct TaskInformation {
	int	Duration;
	char	*Title;
	char	*Description;
	char	*Reward;
	int	RewardID;
	int	CashReward; // Expressed in copper
	int	XPReward;
	TaskMethodType RewardMethod;
	int	StartZone;
	int	ActivityCount;
	SequenceType SequenceMode;
	int	LastStep;
	short	MinLevel;
	short	MaxLevel;
	bool	Repeatable;
	ActivityInformation Activity[MAXACTIVITIESPERTASK];
};

typedef enum { ActivityHidden = 0, ActivityActive = 1, ActivityCompleted = 2 } ActivityState;

typedef enum { ActivityDeliver = 1, ActivityKill = 2, ActivityLoot = 3, ActivitySpeakWith = 4, ActivityExplore = 5,
			ActivityTradeSkill = 6, ActivityFish = 7, ActivityForage = 8, ActivityUse1 = 9, ActivityUse2 = 10,
			ActivityTouch = 11, ActivityGiveCash = 100 } ActivityType;


struct ClientActivityInformation {
	int ActivityID;
	int DoneCount;
	ActivityState State;
	bool Updated; // Flag so we know if we need to update the database
};

struct ClientTaskInformation {
	int TaskID;
	int CurrentStep;
	int AcceptedTime;
	bool Updated;
	ClientActivityInformation Activity[MAXACTIVITIESPERTASK];
};

struct CompletedTaskInformation {
	int TaskID;
	int CompletedTime;
	bool ActivityDone[MAXACTIVITIESPERTASK];
};

class ClientTaskState {

public:
	ClientTaskState();
	~ClientTaskState();
	void ShowClientTasks(Client *c);
	inline int GetActiveTaskCount() { return ActiveTaskCount; }
	int GetActiveTaskID(int index);
	bool IsTaskActivityCompleted(int index, int ActivityID);
	int GetTaskActivityDoneCount(int index, int ActivityID);
	int GetTaskActivityDoneCountFromTaskID(int TaskID, int ActivityID);
	int GetTaskStartTime(int index);
	void AcceptNewTask(Client *c, int TaskID, int NPCID, bool enforce_level_requirement = false);
	void FailTask(Client *c, int TaskID);
	int TaskTimeLeft(int TaskID);
	int IsTaskCompleted(int TaskID);
	bool IsTaskActive(int TaskID);
	bool IsTaskActivityActive(int TaskID, int ActivityID);
	ActivityState GetTaskActivityState(int index, int ActivityID);
	void UpdateTaskActivity(Client *c, int TaskID, int ActivityID, int Count, bool ignore_quest_update = false);
	void ResetTaskActivity(Client *c, int TaskID, int ActivityID);
	void CancelTask(Client *c, int SequenceNumber, bool RemoveFromDB = true);
	void CancelAllTasks(Client *c);
	void RemoveTask(Client *c, int SequenceNumber);
	bool UpdateTasksByNPC(Client *c, int ActivityType, int NPCTypeID);
	void UpdateTasksOnKill(Client *c, int NPCTypeID);
	void UpdateTasksForItem(Client *c, ActivityType Type, int ItemID, int Count=1);
	void UpdateTasksOnExplore(Client *c, int ExploreID);
	bool UpdateTasksOnSpeakWith(Client *c, int NPCTypeID);
	bool UpdateTasksOnDeliver(Client *c, std::list<EQEmu::ItemInstance*>& Items, int Cash, int NPCTypeID);
	void UpdateTasksOnTouch(Client *c, int ZoneID);
	void ProcessTaskProximities(Client *c, float X, float Y, float Z);
	bool TaskOutOfTime(int Index);
	void TaskPeriodicChecks(Client *c);
	void SendTaskHistory(Client *c, int TaskIndex);
	void RewardTask(Client *c, TaskInformation *Task);
	void EnableTask(int CharID, int TaskCount, int *TaskList);
	void DisableTask(int CharID, int TaskCount, int *TaskList);
	bool IsTaskEnabled(int TaskID);
	int EnabledTaskCount(int TaskSetID);
	int ActiveSpeakTask(int NPCID);
	int ActiveSpeakActivity(int NPCID, int TaskID);
	int ActiveTasksInSet(int TaskSetID);
	int CompletedTasksInSet(int TaskSetID);
	friend class TaskManager;

private:
	bool UnlockActivities(int CharID, int TaskIndex);
	void IncrementDoneCount(Client *c, TaskInformation *Task, int TaskIndex, int ActivityID, int Count = 1, bool ignore_quest_update = false);
	int ActiveTaskCount;
	ClientTaskInformation ActiveTasks[MAXACTIVETASKS];
	std::vector<int>EnabledTasks;
	std::vector<CompletedTaskInformation> CompletedTasks;
	int LastCompletedTaskLoaded;
	bool CheckedTouchActivities;
};


class TaskManager {

public:
	TaskManager();
	~TaskManager();
	int GetActivityCount(int TaskID);
	bool LoadSingleTask(int TaskID);
	bool LoadTasks(int SingleTask=0);
	void ReloadGoalLists();
	inline void LoadProximities(int ZoneID) { ProximityManager.LoadProximities(ZoneID); }
	bool LoadTaskSets();
	bool LoadClientState(Client *c, ClientTaskState *state);
	bool SaveClientState(Client *c, ClientTaskState *state);
	void SendTaskSelector(Client *c, Mob *mob, int TaskCount, int *TaskList);
	void SendTaskSelectorNew(Client *c, Mob *mob, int TaskCount, int *TaskList);
	bool AppropriateLevel(int TaskID, int PlayerLevel);
	int GetTaskMinLevel(int TaskID);
	int GetTaskMaxLevel(int TaskID);
	void TaskSetSelector(Client *c, ClientTaskState *state, Mob *mob, int TaskSetID);
	void SendActiveTasksToClient(Client *c, bool TaskComplete=false);
	void SendSingleActiveTaskToClient(Client *c, int TaskIndex, bool TaskComplete, bool BringUpTaskJournal=false);
	void SendTaskActivityShort(Client *c, int TaskID, int ActivityID, int ClientTaskIndex);
	void SendTaskActivityLong(Client *c, int TaskID, int ActivityID, int ClientTaskIndex,
				bool Optional, bool TaskComplete=false);
	void SendTaskActivityNew(Client *c, int TaskID, int ActivityID, int ClientTaskIndex,
				bool Optional, bool TaskComplete=false);
	void SendCompletedTasksToClient(Client *c, ClientTaskState *state);
	void ExplainTask(Client *c, int TaskID);
	int FirstTaskInSet(int TaskSet);
	int LastTaskInSet(int TaskSet);
	int NextTaskInSet(int TaskSet, int TaskID);
	bool IsTaskRepeatable(int TaskID);
	friend class ClientTaskState;


private:
	TaskGoalListManager GoalListManager;
	TaskProximityManager ProximityManager;
	TaskInformation* Tasks[MAXTASKS];
	std::vector<int> TaskSets[MAXTASKSETS];
	void SendActiveTaskDescription(Client *c, int TaskID, int SequenceNumber, int StartTime, int Duration, bool BringUpTaskJournal=false);

};

#endif
