#include "../common/global_define.h"
#include "../common/misc_functions.h"
#include "../common/repositories/goallists_repository.h"
#include "../common/rulesys.h"
#include "client.h"
#include "mob.h"
#include "quest_parser_collection.h"
#include "task_goal_list_manager.h"
#include "tasks.h"
#include "zonedb.h"

TaskGoalListManager::TaskGoalListManager()
{
	goal_lists_count = 0;
}

TaskGoalListManager::~TaskGoalListManager() {}

bool TaskGoalListManager::LoadLists()
{
	task_goal_lists.clear();
	goal_lists_count = 0;

	std::string query   = "SELECT `listid`, COUNT(`entry`) FROM `goallists` GROUP by `listid` ORDER BY `listid`";
	auto        results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	goal_lists_count = results.RowCount();
	LogTasks("Loaded [{}] GoalLists", goal_lists_count);

	task_goal_lists.reserve(goal_lists_count);

	int       list_index = 0;
	for (auto row        = results.begin(); row != results.end(); ++row) {
		int list_id   = atoi(row[0]);
		int list_size = atoi(row[1]);

		task_goal_lists.push_back({list_id, 0, 0});

		task_goal_lists[list_index].GoalItemEntries.reserve(list_size);

		list_index++;
	}

	auto goal_lists = GoallistsRepository::GetWhere(content_db, "TRUE ORDER BY listid, entry ASC");
	for (list_index = 0; list_index < goal_lists_count; list_index++) {

		int list_id = task_goal_lists[list_index].ListID;

		for (auto &entry: goal_lists) {
			if (entry.listid == list_id) {
				if (entry.entry < task_goal_lists[list_index].Min) {
					task_goal_lists[list_index].Min = entry.entry;
				}

				if (entry.entry > task_goal_lists[list_index].Max) {
					task_goal_lists[list_index].Max = entry.entry;
				}

				task_goal_lists[list_index].GoalItemEntries.push_back(entry.entry);

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

int TaskGoalListManager::GetListByID(int list_id)
{

	// Find the list with the specified ListID and return the index
	auto it = std::find_if(
		task_goal_lists.begin(),
		task_goal_lists.end(),
		[list_id](const TaskGoalList_Struct &t) { return t.ListID == list_id; }
	);

	if (it == task_goal_lists.end()) {
		return -1;
	}

	return std::distance(task_goal_lists.begin(), it);
}

int TaskGoalListManager::GetFirstEntry(int list_id)
{
	int list_by_id = GetListByID(list_id);

	if ((list_by_id < 0) || (list_by_id >= goal_lists_count)) {
		return -1;
	}

	if (task_goal_lists[list_by_id].GoalItemEntries.empty()) {
		return -1;
	}

	return task_goal_lists[list_by_id].GoalItemEntries[0];
}

std::vector<int> TaskGoalListManager::GetListContents(int list_index)
{
	std::vector<int> list_contents;
	int              list_by_id = GetListByID(list_index);

	if ((list_by_id < 0) || (list_by_id >= goal_lists_count)) {
		return list_contents;
	}

	list_contents = task_goal_lists[list_by_id].GoalItemEntries;

	return list_contents;
}

bool TaskGoalListManager::IsInList(int list_id, int entry)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i)", list_id, entry);

	int list_index = GetListByID(list_id);

	if ((list_index < 0) || (list_index >= goal_lists_count)) {
		return false;
	}

	if ((entry < task_goal_lists[list_index].Min) || (entry > task_goal_lists[list_index].Max)) {
		return false;
	}

	int  first_entry = 0;
	auto &task       = task_goal_lists[list_index];
	auto it          = std::find(task.GoalItemEntries.begin(), task.GoalItemEntries.end(), entry);

	if (it == task.GoalItemEntries.end()) {
		return false;
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i) returning true", list_index, entry);

	return true;
}
