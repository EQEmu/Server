#ifndef EQEMU_TASK_GOAL_LIST_MANAGER_H
#define EQEMU_TASK_GOAL_LIST_MANAGER_H

#include "tasks.h"
#include "../common/types.h"
#include <list>
#include <vector>
#include <string>
#include <algorithm>

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
	static bool IsInMatchList(const std::string& match_list, const std::string& entry);
	static bool IsInMatchListPartial(const std::string& match_list, const std::string& entry);

private:
	std::vector<TaskGoalList_Struct> m_task_goal_lists;
	int                              m_goal_lists_count;
};


#endif //EQEMU_TASK_GOAL_LIST_MANAGER_H
