#ifndef EQEMU_SHARED_TASKS_H
#define EQEMU_SHARED_TASKS_H

#include "database.h"
#include "types.h"
#include "repositories/tasks_repository.h"
#include "repositories/task_activities_repository.h"
#include "repositories/shared_tasks_repository.h"
#include <vector>
#include <string>
#include <iostream>

// shared tasks
//#define ServerOP_SharedTaskGrant          0x0301 // world -> zone. World verified everything is good
//#define ServerOP_SharedTaskReject         0x0302 // world -> zone. Something failed ABORT
//#define ServerOP_SharedTaskRemovePlayer   0x0304 // .. /taskremoveplayer ..
//#define ServerOP_SharedTaskZoneCreated    0x0305 // zone -> world. Something didn't go wrong creating the new task! Now World needs to tell other players to join world -> zone response to tell someone to join
//#define ServerOP_SharedTaskZoneFailed     0x0306 // zone -> world. Something went wrong above ABORT
//#define ServerOP_SharedTaskActivityUpdate 0x0307 // zone -> world. Is this valid update? world -> zone update activity
//#define ServerOP_SharedTaskCompleted      0x0308 // world -> zone. We completed! Do stuff zone!

// handled
#define ServerOP_SharedTaskRequest                  0x0300 // zone -> world. Player trying to get task. Relayed world -> zone on confirmation
#define ServerOP_SharedTaskAddPlayer                0x0301 // bidirectional. /taskaddplayer request zone -> world. success world -> zone
#define ServerOP_SharedTaskMakeLeader               0x0302 // zone -> world -> zone
#define ServerOP_SharedTaskRemovePlayer             0x0303 // zone -> world -> zone
#define ServerOP_SharedTaskAttemptRemove            0x0304 // zone -> world. Player trying to delete task
#define ServerOP_SharedTaskUpdate                   0x0305 // zone -> world. Client sending task update to world. Relayed world -> zone on confirmation
#define ServerOP_SharedTaskMemberlist               0x0306 // world -> zone. Send shared task memberlist
#define ServerOP_SharedTaskRequestMemberlist        0x0307 // zone -> world. Send shared task memberlist (zone in initial for now, could change)
#define ServerOP_SharedTaskAcceptNewTask            0x0308 // world -> zone. World verified, continue AcceptNewTask

// used in
// ServerOP_SharedTaskRequest

// ServerOP_SharedTaskAcceptNewTask
struct ServerSharedTaskRequest_Struct {
	uint32 requested_character_id;
	uint32 requested_task_id;
};

// ServerOP_SharedTaskAttemptRemove
// gets re-used when sent back to clients
struct ServerSharedTaskAttemptRemove_Struct {
	uint32 requested_character_id;
	uint32 requested_task_id;
	bool   remove_from_db;
};

// used in the shared task request process (currently)
struct SharedTaskMember {
	uint32      character_id   = 0;
	std::string character_name; // potentially remove, use only character_id
	uint32      level          = 0;
	bool        is_grouped     = false; // this shouldn't be necessary either, potentially remove
	bool        is_raided      = false; // this shouldn't be necessary either, potentially remove
	bool        is_leader      = false;
};

// ServerOP_SharedTaskMemberlist
// builds the buffer and sends to clients directly
struct ServerSharedTaskMemberListPacket_Struct {
	uint32 destination_character_id;
	uint32 shared_task_id;
};

struct SharedTaskActivityStateEntry {
	uint32 activity_id;
	uint32 done_count;
	uint32 max_done_count; // goalcount
	uint32 completed_time;
};

struct ServerSharedTaskActivityUpdate_Struct {
	uint32 source_character_id;
	uint32 task_id;
	uint32 activity_id;
	uint32 done_count;
	bool   ignore_quest_update;
};

struct ServerSharedTaskRequestMemberlist_Struct {
	uint32 source_character_id;
	uint32 task_id;
};

struct ServerSharedTaskRemovePlayer_Struct {
	uint32 source_character_id;
	uint32 task_id;
	char   player_name[64];
};

struct ServerSharedTaskMakeLeader_Struct {
	uint32 source_character_id;
	uint32 task_id;
	char   player_name[64];
};

class SharedTask {
public:
	std::vector<SharedTaskActivityStateEntry> GetActivityState() const;
	std::vector<SharedTaskMember> GetMembers() const;

	// getters
	const std::vector<TaskActivitiesRepository::TaskActivities> &GetTaskActivityData() const;
	const TasksRepository::Tasks &GetTaskData() const;

	// setters
	void SetMembers(const std::vector<SharedTaskMember> &members);
	void SetSharedTaskActivityState(const std::vector<SharedTaskActivityStateEntry> &activity_state);
	void SetTaskActivityData(const std::vector<TaskActivitiesRepository::TaskActivities> &task_activity_data);
	void SetTaskData(const TasksRepository::Tasks &task_data);

	// active record of database shared task
	SharedTasksRepository::SharedTasks m_db_shared_task;

	std::vector<SharedTaskActivityStateEntry> m_shared_task_activity_state;
	std::vector<SharedTaskMember>             m_members;

protected:

	// reference to task data (only for this shared task)
	TasksRepository::Tasks                                m_task_data;
	std::vector<TaskActivitiesRepository::TaskActivities> m_task_activity_data;
};

#endif //EQEMU_SHARED_TASKS_H
