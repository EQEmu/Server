#ifndef TASKS_H
#define TASKS_H

#include "../common/types.h"

#include <list>
#include <vector>
#include <string>
#include <algorithm>

#define MAXTASKS 10000
#define MAXTASKSETS 1000
// The Client has a hard cap of 19 active quests, 29 in SoD+
#define MAXACTIVEQUESTS 19
// The Max Chooser (Task Selector entries) is capped at 40 in the Titanium Client.
#define MAXCHOOSERENTRIES 40
// The Client has a hard cap of 20 activities per task.
#define MAXACTIVITIESPERTASK 20
// This is used to determine if a client's active task slot is empty.
#define TASKSLOTEMPTY 0

// Command Codes for worldserver ServerOP_ReloadTasks
#define RELOADTASKS 0
#define RELOADTASKGOALLISTS 1
#define RELOADTASKPROXIMITIES 2
#define RELOADTASKSETS 3

class Client;

class Mob;

namespace EQ {
	class ItemInstance;
}

struct TaskGoalList_Struct {
	int              ListID;
	int              Min, Max;
	std::vector<int> GoalItemEntries;
};

// This is used for handling lists, loading them from the database, searching them.
// Used for lists of NPCs to kill, items to loot, etc, as well as lists of items to
// reward the player with on completion of the task.
class TaskGoalListManager {

public:
	TaskGoalListManager();
	~TaskGoalListManager();
	bool LoadLists();
	int GetListByID(int list_id);
	bool IsInList(int list_id, int entry);
	int GetFirstEntry(int list_id);
	std::vector<int> GetListContents(int list_index);

private:
	std::vector<TaskGoalList_Struct> task_goal_lists;
	int                              goal_lists_count;
};

struct TaskProximity {
	int   explore_id;
	float min_x;
	float max_x;
	float min_y;
	float max_y;
	float min_z;
	float max_z;
};

// This class is used for managing proximities so that Quest NPC proximities don't need to be used.
class TaskProximityManager {

public:
	TaskProximityManager();
	~TaskProximityManager();
	bool LoadProximities(int zone_id);
	int CheckProximities(float x, float y, float z);

private:
	std::vector<TaskProximity> task_proximities;
};

typedef enum {
	METHODSINGLEID = 0,
	METHODLIST     = 1,
	METHODQUEST    = 2
} TaskMethodType;

struct ActivityInformation {
	int              step_number;
	int              activity_type;
	std::string      target_name; // name mob, location -- default empty
	std::string      item_list; // likely defaults to empty
	std::string      skill_list; // IDs ; separated -- default -1
	std::string      spell_list; // IDs ; separated -- default 0
	std::string      description_override; // overrides auto generated description -- default empty
	int              skill_id; // older clients, first id from above
	int              spell_id; // older clients, first id from above
	int              goal_id;
	TaskMethodType   goal_method;
	int              goal_count;
	int              deliver_to_npc;
	std::vector<int> zone_ids;
	std::string      zones; // IDs ; searated, ZoneID is the first in this list for older clients -- default empty string
	bool             optional;

	inline bool CheckZone(int zone_id)
	{
		if (zone_ids.empty()) {
			return true;
		}
		return std::find(zone_ids.begin(), zone_ids.end(), zone_id) != zone_ids.end();
	}
};

typedef enum {
	ActivitiesSequential = 0,
	ActivitiesStepped    = 1
} SequenceType;

enum class TaskType {
	Task   = 0,        // can have at max 1
	Shared = 1,        // can have at max 1
	Quest  = 2,        // can have at max 19 or 29 depending on client
	E      = 3         // can have at max 19 or 29 depending on client, not present in live anymore
};

enum class DurationCode {
	None   = 0,
	Short  = 1,
	Medium = 2,
	Long   = 3
};

struct TaskInformation {
	TaskType            type;
	int                 duration;
	DurationCode        duration_code;         // description for time investment for when duration == 0
	std::string         title;            // max length 64
	std::string         description;      // max length 4000, 2048 on Tit
	std::string         reward;
	std::string         item_link;        // max length 128 older clients, item link gets own string
	std::string         completion_emote; // emote after completing task, yellow. Maybe should make more generic ... but yellow for now!
	int                 reward_id;
	int                 cash_reward;       // Expressed in copper
	int                 experience_reward;
	int                 faction_reward;   // just a npc_faction_id
	TaskMethodType      reward_method;
	int                 activity_count;
	SequenceType        sequence_mode;
	int                 last_step;
	short               min_level;
	short               max_level;
	bool                repeatable;
	ActivityInformation activity_information[MAXACTIVITIESPERTASK];
};

typedef enum {
	ActivityHidden    = 0,
	ActivityActive    = 1,
	ActivityCompleted = 2
} ActivityState;

typedef enum {
	ActivityDeliver    = 1,
	ActivityKill       = 2,
	ActivityLoot       = 3,
	ActivitySpeakWith  = 4,
	ActivityExplore    = 5,
	ActivityTradeSkill = 6,
	ActivityFish       = 7,
	ActivityForage     = 8,
	ActivityCastOn     = 9,
	ActivitySkillOn    = 10,
	ActivityTouch      = 11,
	ActivityCollect    = 13,
	ActivityGiveCash   = 100
} ActivityType;

struct ClientActivityInformation {
	int           activity_id;
	int           done_count;
	ActivityState activity_state;
	bool          updated; // Flag so we know if we need to updated the database
};

struct ClientTaskInformation {
	int                       slot; // intrusive, but makes things easier :P
	int                       task_id;
	int                       current_step;
	int                       accepted_time;
	bool                      updated;
	ClientActivityInformation activity[MAXACTIVITIESPERTASK];
};

struct CompletedTaskInformation {
	int  task_id;
	int  completed_time;
	bool activity_done[MAXACTIVITIESPERTASK];
};

class ClientTaskState {

public:
	ClientTaskState();
	~ClientTaskState();
	void ShowClientTasks(Client *client);
	inline int GetActiveTaskCount() { return active_task_count; }
	int GetActiveTaskID(int index);
	bool IsTaskActivityCompleted(TaskType task_type, int index, int activity_id);
	int GetTaskActivityDoneCount(TaskType task_type, int index, int activity_id);
	int GetTaskActivityDoneCountFromTaskID(int task_id, int activity_id);
	int GetTaskStartTime(TaskType task_type, int index);
	void AcceptNewTask(Client *client, int task_id, int npc_type_id, bool enforce_level_requirement = false);
	void FailTask(Client *client, int task_id);
	int TaskTimeLeft(int task_id);
	int IsTaskCompleted(int task_id);
	bool IsTaskActive(int task_id);
	bool IsTaskActivityActive(int task_id, int activity_id);
	ActivityState GetTaskActivityState(TaskType task_type, int index, int activity_id);
	void UpdateTaskActivity(Client *client, int task_id, int activity_id, int count, bool ignore_quest_update = false);
	void ResetTaskActivity(Client *client, int task_id, int activity_id);
	void CancelTask(Client *client, int sequence_number, TaskType task_type, bool remove_from_db = true);
	void CancelAllTasks(Client *client);
	void RemoveTask(Client *client, int sequence_number, TaskType task_type);
	void RemoveTaskByTaskID(Client *client, uint32 task_id);
	bool UpdateTasksByNPC(Client *client, int activity_type, int npc_type_id);
	void UpdateTasksOnKill(Client *client, int npc_type_id);
	void UpdateTasksForItem(Client *client, ActivityType activity_type, int item_id, int count = 1);
	void UpdateTasksOnExplore(Client *client, int explore_id);
	bool UpdateTasksOnSpeakWith(Client *client, int npc_type_id);
	bool UpdateTasksOnDeliver(Client *client, std::list<EQ::ItemInstance *> &items, int cash, int npc_type_id);
	void UpdateTasksOnTouch(Client *client, int zone_id);
	void ProcessTaskProximities(Client *client, float x, float y, float z);
	bool TaskOutOfTime(TaskType task_type, int index);
	void TaskPeriodicChecks(Client *client);
	void SendTaskHistory(Client *client, int task_index);
	void RewardTask(Client *client, TaskInformation *task_information);
	void EnableTask(int character_id, int task_count, int *task_list);
	void DisableTask(int character_id, int task_count, int *task_list);
	bool IsTaskEnabled(int task_id);
	int EnabledTaskCount(int task_set_id);
	int ActiveSpeakTask(int npc_type_id);
	int ActiveSpeakActivity(int npc_type_id, int task_id);
	int ActiveTasksInSet(int task_set_id);
	int CompletedTasksInSet(int task_set_id);
	bool HasSlotForTask(TaskInformation *task);

	inline bool HasFreeTaskSlot() { return active_task.task_id == TASKSLOTEMPTY; }

	friend class TaskManager;

private:
	bool UnlockActivities(int character_id, ClientTaskInformation &task_info);
	void IncrementDoneCount(
		Client *client,
		TaskInformation *task_information,
		int task_index,
		int activity_id,
		int count = 1,
		bool ignore_quest_update = false
	);
	inline ClientTaskInformation *GetClientTaskInfo(TaskType task_type, int index)
	{
		ClientTaskInformation *info = nullptr;
		switch (task_type) {
			case TaskType::Task:
				if (index == 0) {
					info = &active_task;
				}
				break;
			case TaskType::Shared:
				break;
			case TaskType::Quest:
				if (index < MAXACTIVEQUESTS) {
					info = &active_quests[index];
				}
				break;
			default:
				break;
		}
		return info;
	}
	int                                   active_task_count;

	union { // easier to loop over
		struct {
			ClientTaskInformation active_task; // only one
			ClientTaskInformation active_quests[MAXACTIVEQUESTS];
		};
		ClientTaskInformation active_tasks[MAXACTIVEQUESTS + 1];
	};
	// Shared tasks should be limited to 1 as well
	std::vector<int>                      enabled_tasks;
	std::vector<CompletedTaskInformation> completed_tasks;
	int                                   last_completed_task_loaded;
	bool                                  checked_touch_activities;
};


class TaskManager {

public:
	TaskManager();
	~TaskManager();
	int GetActivityCount(int task_id);
	bool LoadTasks(int single_task = 0);
	void ReloadGoalLists();
	inline void LoadProximities(int zone_id)
	{
		proximity_manager.LoadProximities(zone_id);
	}
	bool LoadTaskSets();
	bool LoadClientState(Client *client, ClientTaskState *client_task_state);
	bool SaveClientState(Client *client, ClientTaskState *client_task_state);
	void SendTaskSelector(Client *client, Mob *mob, int task_count, int *task_list);
	void SendTaskSelectorNew(Client *client, Mob *mob, int task_count, int *task_list);
	bool ValidateLevel(int task_id, int player_level);
	std::string GetTaskName(uint32 task_id);
	TaskType GetTaskType(uint32 task_id);
	void TaskSetSelector(Client *client, ClientTaskState *client_task_state, Mob *mob, int task_set_id);
	// task list provided by QuestManager (perl/lua)
	void TaskQuestSetSelector(
		Client *client,
		ClientTaskState *client_task_state,
		Mob *mob,
		int count,
		int *tasks
	);
	void SendActiveTasksToClient(Client *client, bool task_complete = false);
	void SendSingleActiveTaskToClient(
		Client *client,
		ClientTaskInformation &task_info,
		bool task_complete,
		bool bring_up_task_journal = false
	);
	void SendTaskActivityShort(Client *client, int task_id, int activity_id, int client_task_index);
	void SendTaskActivityLong(
		Client *client,
		int task_id,
		int activity_id,
		int client_task_index,
		bool optional,
		bool task_complete = false
	);
	void SendTaskActivityNew(
		Client *client,
		int task_id,
		int activity_id,
		int client_task_index,
		bool optional,
		bool task_complete = false
	);
	void SendCompletedTasksToClient(Client *c, ClientTaskState *client_task_state);
	void ExplainTask(Client *client, int task_id);
	int FirstTaskInSet(int task_set);
	int LastTaskInSet(int task_set);
	int NextTaskInSet(int task_set, int task_id);
	bool IsTaskRepeatable(int task_id);

	friend class ClientTaskState;


private:
	TaskGoalListManager  goal_list_manager;
	TaskProximityManager proximity_manager;
	TaskInformation      *p_task_data[MAXTASKS]{};
	std::vector<int>     task_sets[MAXTASKSETS];
	void SendActiveTaskDescription(
		Client *client,
		int task_id,
		ClientTaskInformation &task_info,
		int start_time,
		int duration,
		bool bring_up_task_journal = false
	);

};

#endif
