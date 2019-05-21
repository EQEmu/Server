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
#include "../common/global_tasks.h"

#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <map>

class Client;
class Mob;

namespace EQEmu
{
	class ItemInstance;
}

struct TaskGoalList_Struct {
	int ListID;
	int Min, Max;
	std::vector<int> GoalItemEntries;
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

	std::vector<TaskGoalList_Struct> TaskGoalLists;
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


struct ClientActivityInformation {
	int ActivityID;
	int DoneCount;
	ActivityState State;
	bool Updated; // Flag so we know if we need to update the database
};

struct ClientTaskInformation {
	int slot; // intrusive, but makes things easier :P
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

struct SharedTaskMember {
	std::string name;
	Mob *entity; // needs to be managed
	bool leader;
	SharedTaskMember() : entity(nullptr), leader(false) {}
};

class SharedTaskState {
public:
	SharedTaskState() : locked(false) {}
//	~SharedTaskState();

	inline const bool IsLocked() const { return locked; }
	inline void SetLocked(bool v) { locked = v; }
	void LockTask(); // notified clients (if they are etc)

	void MemberZoned(Mob *player); // player left zone, update their pointer
	void MemberEnterZone(Mob *player); // player entered zone, update their pointer

	ClientTaskInformation *GetActivity() { return &activity; }

	friend class TaskManager;

private:
	std::vector<SharedTaskMember> members;
	ClientTaskInformation activity;
	bool locked;
};

class ClientTaskState {

public:
	ClientTaskState();
	~ClientTaskState();
	void ShowClientTasks(Client *c);
	inline int GetActiveTaskCount() { return ActiveTaskCount; }
	int GetActiveTaskID(int index);
	bool IsTaskActivityCompleted(TaskType type, int index, int ActivityID);
	int GetTaskActivityDoneCount(TaskType type, int index, int ActivityID);
	int GetTaskActivityDoneCountFromTaskID(int TaskID, int ActivityID);
	int GetTaskStartTime(TaskType type, int index);
	void AcceptNewTask(Client *c, int TaskID, int NPCID, bool enforce_level_requirement = false);
	void AcceptNewSharedTask(Client *c, int TaskID, int NPCID, int id, std::vector<std::string> &members);
	void PendSharedTask(Client *c, int TaskID, int NPCID, bool enforce_level_requirement = false);
	void FailTask(Client *c, int TaskID);
	int TaskTimeLeft(int TaskID);
	int IsTaskCompleted(int TaskID);
	bool IsTaskActive(int TaskID);
	bool IsTaskActivityActive(int TaskID, int ActivityID);
	ActivityState GetTaskActivityState(TaskType type, int index, int ActivityID);
	void UpdateTaskActivity(Client *c, int TaskID, int ActivityID, int Count, bool ignore_quest_update = false);
	void ResetTaskActivity(Client *c, int TaskID, int ActivityID);
	void CancelTask(Client *c, int SequenceNumber, TaskType type, bool RemoveFromDB = true);
	void CancelAllTasks(Client *c);
	void RemoveTask(Client *c, int SequenceNumber, TaskType type);
	bool UpdateTasksByNPC(Client *c, int ActivityType, int NPCTypeID);
	void UpdateTasksOnKill(Client *c, int NPCTypeID);
	void UpdateTasksForItem(Client *c, ActivityType Type, int ItemID, int Count=1);
	void UpdateTasksOnExplore(Client *c, int ExploreID);
	bool UpdateTasksOnSpeakWith(Client *c, int NPCTypeID);
	bool UpdateTasksOnDeliver(Client *c, std::list<EQEmu::ItemInstance*>& Items, int Cash, int NPCTypeID);
	void UpdateTasksOnTouch(Client *c, int ZoneID);
	void ProcessTaskProximities(Client *c, float X, float Y, float Z);
	bool TaskOutOfTime(TaskType type, int Index);
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
	bool HasSlotForTask(TaskInformation *task);

	inline bool HasFreeTaskSlot() { return ActiveTask.TaskID == TASKSLOTEMPTY; }

	friend class TaskManager;

private:
	bool UnlockActivities(int CharID, ClientTaskInformation &task_info);
	void IncrementDoneCount(Client *c, TaskInformation *Task, int TaskIndex, int ActivityID, int Count = 1, bool ignore_quest_update = false);
	inline ClientTaskInformation *GetClientTaskInfo(TaskType type, int index)
	{
		ClientTaskInformation *info = nullptr;
		switch (type) {
		case TaskType::Task:
			if (index == 0)
				info = &ActiveTask;
			break;
		case TaskType::Shared:
			if (index == 0 && ActiveSharedTask)
				info = ActiveSharedTask->GetActivity();
			break;
		case TaskType::Quest:
			if (index < MAXACTIVEQUESTS)
				info = &ActiveQuests[index];
			break;
		default:
			break;
		}
		return info;
	}
	int ActiveTaskCount;
	union { // easier to loop over
		struct {
			ClientTaskInformation ActiveTask; // only one
			ClientTaskInformation ActiveQuests[MAXACTIVEQUESTS];
		};
		ClientTaskInformation ActiveTasks[MAXACTIVEQUESTS + 1];
	};
	SharedTaskState *ActiveSharedTask; // pointer to our shared task managed by TaskManager
	// Shared tasks should be limited to 1 as well
	std::vector<int> EnabledTasks;
	std::vector<CompletedTaskInformation> CompletedTasks;
	int LastCompletedTaskLoaded;
	bool CheckedTouchActivities;
};

struct TaskReplayGroups {
	std::string name;
	int duration;
};

class TaskManager {

public:
	TaskManager();
	~TaskManager();
	int GetActivityCount(int TaskID);
	bool LoadSingleTask(int TaskID);
	bool LoadTasks(int SingleTask=0);
	bool LoadReplayGroups();
	void ReloadGoalLists();
	inline void LoadProximities(int ZoneID) { ProximityManager.LoadProximities(ZoneID); }
	bool LoadTaskSets();
	bool LoadClientState(Client *c, ClientTaskState *state);
	bool SaveClientState(Client *c, ClientTaskState *state);
	void SendTaskSelector(Client *c, Mob *mob, int TaskCount, int *TaskList, bool shared = false); // dumb hack cuz we do dumb things
	void SendTaskSelectorNew(Client *c, Mob *mob, int TaskCount, int *TaskList, bool shared = false);
	bool AppropriateLevel(int TaskID, int PlayerLevel);
	int GetTaskMinLevel(int TaskID);
	int GetTaskMaxLevel(int TaskID);
	void TaskSetSelector(Client *c, ClientTaskState *state, Mob *mob, int TaskSetID, bool shared = false);
	void TaskQuestSetSelector(Client *c, ClientTaskState *state, Mob *mob, int count, int *tasks, bool shared = false); // task list provided by QuestManager (perl/lua)
	void SendActiveTasksToClient(Client *c, bool TaskComplete=false);
	void SendSingleActiveTaskToClient(Client *c, ClientTaskInformation &task_info, bool TaskComplete, bool BringUpTaskJournal = false);
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

	void LoadSharedTask(int id); // loads the shared task state

private:
	TaskGoalListManager GoalListManager;
	TaskProximityManager ProximityManager;
	TaskInformation* Tasks[MAXTASKS];
	std::vector<int> TaskSets[MAXTASKSETS];
	void SendActiveTaskDescription(Client *c, int TaskID, ClientTaskInformation &task_info, int StartTime, int Duration, bool BringUpTaskJournal=false);
	std::unordered_map<int, SharedTaskState> SharedTasks;
	std::map<int, TaskReplayGroups> replay_groups;

};

#endif
