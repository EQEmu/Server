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
	m_goal_lists_count = 0;
}

TaskGoalListManager::~TaskGoalListManager() {}

bool TaskGoalListManager::LoadLists()
{
	m_task_goal_lists.clear();
	m_goal_lists_count = 0;

	std::string query   = "SELECT `listid`, COUNT(`entry`) FROM `goallists` GROUP by `listid` ORDER BY `listid`";
	auto        results = content_db.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	m_goal_lists_count = results.RowCount();
	LogTasks("Loaded [{}] GoalLists", m_goal_lists_count);

	m_task_goal_lists.reserve(m_goal_lists_count);

	int       list_index = 0;
	for (auto row        = results.begin(); row != results.end(); ++row) {
		int list_id   = atoi(row[0]);
		int list_size = atoi(row[1]);

		m_task_goal_lists.push_back({list_id, 0, 0});

		m_task_goal_lists[list_index].GoalItemEntries.reserve(list_size);

		list_index++;
	}

	auto goal_lists = GoallistsRepository::GetWhere(content_db, "TRUE ORDER BY listid, entry ASC");
	for (list_index = 0; list_index < m_goal_lists_count; list_index++) {

		int list_id = m_task_goal_lists[list_index].ListID;

		for (auto &entry: goal_lists) {
			if (entry.listid == list_id) {
				if (entry.entry < m_task_goal_lists[list_index].Min) {
					m_task_goal_lists[list_index].Min = entry.entry;
				}

				if (entry.entry > m_task_goal_lists[list_index].Max) {
					m_task_goal_lists[list_index].Max = entry.entry;
				}

				m_task_goal_lists[list_index].GoalItemEntries.push_back(entry.entry);

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
		m_task_goal_lists.begin(),
		m_task_goal_lists.end(),
		[list_id](const TaskGoalList_Struct &t) { return t.ListID == list_id; }
	);

	if (it == m_task_goal_lists.end()) {
		return -1;
	}

	return std::distance(m_task_goal_lists.begin(), it);
}

int TaskGoalListManager::GetFirstEntry(int list_id)
{
	int list_by_id = GetListByID(list_id);

	if ((list_by_id < 0) || (list_by_id >= m_goal_lists_count)) {
		return -1;
	}

	if (m_task_goal_lists[list_by_id].GoalItemEntries.empty()) {
		return -1;
	}

	return m_task_goal_lists[list_by_id].GoalItemEntries[0];
}

std::vector<int> TaskGoalListManager::GetListContents(int list_index)
{
	std::vector<int> list_contents;
	int              list_by_id = GetListByID(list_index);

	if ((list_by_id < 0) || (list_by_id >= m_goal_lists_count)) {
		return list_contents;
	}

	list_contents = m_task_goal_lists[list_by_id].GoalItemEntries;

	return list_contents;
}

bool TaskGoalListManager::IsInList(int list_id, int entry)
{
	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i)", list_id, entry);

	int list_index = GetListByID(list_id);

	if ((list_index < 0) || (list_index >= m_goal_lists_count)) {
		return false;
	}

	if ((entry < m_task_goal_lists[list_index].Min) || (entry > m_task_goal_lists[list_index].Max)) {
		return false;
	}

	int  first_entry = 0;
	auto &task       = m_task_goal_lists[list_index];
	auto it          = std::find(task.GoalItemEntries.begin(), task.GoalItemEntries.end(), entry);

	if (it == task.GoalItemEntries.end()) {
		return false;
	}

	Log(Logs::General, Logs::Tasks, "[UPDATE] TaskGoalListManager::IsInList(%i, %i) returning true", list_index, entry);

	return true;
}

bool TaskGoalListManager::IsInMatchList(const std::string& match_list, const std::string& entry)
{
	for (auto &s: SplitString(match_list, '|')) {
		if (s == entry) {
			return true;
		}
	}

	return false;
}

bool TaskGoalListManager::IsInMatchListPartial(const std::string &match_list, const std::string &entry)
{
	std::string entry_match = str_tolower(entry);
	for (auto &s: SplitString(match_list, '|')) {
		if (entry_match.find(str_tolower(s)) != std::string::npos) {
			return true;
		}
	}

	return false;
}
