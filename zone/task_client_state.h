#ifndef EQEMU_TASK_CLIENT_STATE_H
#define EQEMU_TASK_CLIENT_STATE_H

#include "tasks.h"
#include "../common/types.h"
#include <list>
#include <vector>
#include <string>
#include <algorithm>

class ClientTaskState {

public:
	ClientTaskState();
	~ClientTaskState();
	void ShowClientTasks(Client *client);
	inline int GetActiveTaskCount() { return m_active_task_count; }
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

	inline bool HasFreeTaskSlot() { return m_active_task.task_id == TASKSLOTEMPTY; }

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
					info = &m_active_task;
				}
				break;
			case TaskType::Shared:
				break;
			case TaskType::Quest:
				if (index < MAXACTIVEQUESTS) {
					info = &m_active_quests[index];
				}
				break;
			default:
				break;
		}
		return info;
	}

	union { // easier to loop over
		struct {
			ClientTaskInformation m_active_task; // only one
			ClientTaskInformation m_active_quests[MAXACTIVEQUESTS];
		};
		ClientTaskInformation m_active_tasks[MAXACTIVEQUESTS + 1];
	};
	// Shared tasks should be limited to 1 as well
	int                                   m_active_task_count;
	std::vector<int>                      m_enabled_tasks;
	std::vector<CompletedTaskInformation> m_completed_tasks;
	int                                   m_last_completed_task_loaded;
	bool                                  m_checked_touch_activities;
};


#endif //EQEMU_TASK_CLIENT_STATE_H
