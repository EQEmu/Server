#ifndef EQEMU_TASK_MANAGER_H
#define EQEMU_TASK_MANAGER_H

#include "tasks.h"
#include "task_client_state.h"
#include "task_proximity_manager.h"
#include "task_goal_list_manager.h"
#include "../common/types.h"
#include "../common/repositories/character_tasks_repository.h"
#include <list>
#include <vector>
#include <string>
#include <algorithm>

class Client;

class Mob;

class TaskManager {

public:
	TaskManager();
	~TaskManager();
	int GetActivityCount(int task_id);
	bool LoadTasks(int single_task = 0);
	void ReloadGoalLists();
	inline void LoadProximities(int zone_id)
	{
		m_proximity_manager.LoadProximities(zone_id);
	}
	bool LoadTaskSets();
	bool LoadClientState(Client *client, ClientTaskState *client_task_state);
	bool SaveClientState(Client *client, ClientTaskState *client_task_state);
	void SendTaskSelector(Client *client, Mob *mob, int task_count, int *task_list);
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
	void SharedTaskSelector(Client* client, Mob* mob, int count, const int* tasks);
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
		bool task_complete = false
	);
	void SendCompletedTasksToClient(Client *c, ClientTaskState *client_task_state);
	void ExplainTask(Client *client, int task_id);
	int FirstTaskInSet(int task_set);
	int LastTaskInSet(int task_set);
	int NextTaskInSet(int task_set, int task_id);
	bool IsTaskRepeatable(int task_id);

	friend class ClientTaskState;

	// shared tasks
	void SyncClientSharedTaskState(Client *c, ClientTaskState *cts);

	void HandleUpdateTasksOnKill(Client *client, uint32 npc_type_id, std::string npc_name);

private:
	TaskGoalListManager  m_goal_list_manager;
	TaskProximityManager m_proximity_manager;
	TaskInformation      *m_task_data[MAXTASKS]{};
	std::vector<int>     m_task_sets[MAXTASKSETS];
	void SendActiveTaskDescription(
		Client *client,
		int task_id,
		ClientTaskInformation &task_info,
		int start_time,
		int duration,
		bool bring_up_task_journal = false
	);

	void SendActiveTaskToClient(ClientTaskInformation *task, Client *client, int task_index, bool task_complete);

	// shared tasks
	void SyncClientSharedTaskWithPersistedState(Client *c, ClientTaskState *cts);
	void SyncClientSharedTaskRemoveLocalIfNotExists(Client *c, ClientTaskState *cts);
	void SendSharedTaskSelector(Client* client, Mob* mob, int task_count, int* task_list);
	void SyncClientSharedTaskStateToLocal(Client *c);
};


#endif //EQEMU_TASK_MANAGER_H
