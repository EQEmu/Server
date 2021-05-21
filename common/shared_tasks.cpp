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
