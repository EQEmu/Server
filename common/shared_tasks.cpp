#include "shared_tasks.h"
#include "repositories/character_data_repository.h"

std::vector<SharedTaskActivityStateEntry> SharedTask::GetActivityState() const
{
	return shared_task_activity_state;
}

std::vector<SharedTaskMember> SharedTask::GetMembers() const
{
	return members;
}

void SharedTask::SetSharedTaskActivityState(const std::vector<SharedTaskActivityStateEntry> &activity_state)
{
	SharedTask::shared_task_activity_state = activity_state;
}

void SharedTask::SetTaskData(const TasksRepository::Tasks &task_data)
{
	SharedTask::m_task_data = task_data;
}

void SharedTask::SetTaskActivityData(const std::vector<TaskActivitiesRepository::TaskActivities> &task_activity_data)
{
	SharedTask::m_task_activity_data = task_activity_data;
}

const TasksRepository::Tasks &SharedTask::GetTaskData() const
{
	return m_task_data;
}

const std::vector<TaskActivitiesRepository::TaskActivities> &SharedTask::GetTaskActivityData() const
{
	return m_task_activity_data;
}
