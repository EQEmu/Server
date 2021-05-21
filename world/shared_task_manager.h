#ifndef EQEMU_SHARED_TASK_MANAGER_H
#define EQEMU_SHARED_TASK_MANAGER_H

#include "../common/database.h"
#include "../common/shared_tasks.h"

class SharedTaskManager {
public:
	SharedTaskManager *SetDatabase(Database *db);
	SharedTaskManager *SetContentDatabase(Database *db);
	Database *GetDatabase() const;

	// gets group / raid members belonging to requested character
	// this may change later depending on how shared tasks develop
	std::vector<SharedTaskMember> GetRequestMembers(uint32 requestor_character_id);

	// client attempting to create a shared task
	void AttemptSharedTaskCreation(uint32 requested_task_id, uint32 requested_character_id);

protected:
	// reference to database
	Database *m_database;
	Database *m_content_database;

	// internal shared tasks list
	std::vector<SharedTask> m_shared_tasks;
};

#endif //EQEMU_SHARED_TASK_MANAGER_H
