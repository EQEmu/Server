#ifndef EQEMU_SHARED_TASKS_H
#define EQEMU_SHARED_TASKS_H

#include "database.h"
#include "types.h"
#include <vector>
#include <string>
#include <iostream>

// shared tasks
#define ServerOP_SharedTaskRequest        0x0300 // zone -> world. Player trying to get task
#define ServerOP_SharedTaskGrant          0x0301 // world -> zone. World verified everything is good
#define ServerOP_SharedTaskReject         0x0302 // world -> zone. Something failed ABORT
#define ServerOP_SharedTaskAddPlayer      0x0303 // bidirectional. /taskaddplayer request zone -> world. success world -> zone
#define ServerOP_SharedTaskRemovePlayer   0x0304 // .. /taskremoveplayer ..
#define ServerOP_SharedTaskZoneCreated    0x0305 // zone -> world. Something didn't go wrong creating the new task! Now World needs to tell other players to join world -> zone response to tell someone to join
#define ServerOP_SharedTaskZoneFailed     0x0306 // zone -> world. Something went wrong above ABORT
#define ServerOP_SharedTaskActivityUpdate 0x0307 // zone -> world. Is this valid update? world -> zone update activity
#define ServerOP_SharedTaskCompleted      0x0308 // world -> zone. We completed! Do stuff zone!
#define ServerOP_SharedTaskAcceptNewTask  0x0308 // world -> zone. World verified, continue AcceptNewTask

// used in
// ServerOP_SharedTaskRequest
// ServerOP_SharedTaskAcceptNewTask
struct ServerSharedTaskRequest_Struct {
	uint32 requested_character_id;
	uint32 requested_task_id;
};

// used in the shared task request process (currently)
struct SharedTaskMember {
	uint32      character_id;
	std::string character_name;
	uint32      level;
	bool        is_grouped;
	bool        is_raided;
};

struct SharedTaskActivityStateEntry {
	uint32 activity_id;
	uint32 done_count;
	uint32 max_done_count; // goalcount
};

class SharedTask {
public:
	// shared task stuff

	std::vector<SharedTaskActivityStateEntry> GetActivityState() const;
	std::vector<SharedTaskMember> GetMembers() const;

	void SetSharedTaskActivityState(const std::vector<SharedTaskActivityStateEntry> &activity_state);

protected:
	std::vector<SharedTaskActivityStateEntry> shared_task_activity_state;
	std::vector<SharedTaskMember>             members;
};

#endif //EQEMU_SHARED_TASKS_H
