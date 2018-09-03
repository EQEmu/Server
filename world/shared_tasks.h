#ifndef SHARED_TASKS_H
#define SHARED_TASKS_H
#include <unordered_map>
#include <vector>
#include <string>

#include "../common/servertalk.h"

struct SharedTaskMember {
	std::string name;
	bool leader;
	SharedTaskMember() : leader(false) {}
	SharedTaskMember(std::string name, bool leader) : name(name), leader(leader) {}
};

class SharedTask {
public:
	SharedTask() : id(0), task_id(0), missing_count(0) {}
	SharedTask(int id, int task_id) : id(id), task_id(task_id), missing_count(0) {}
	~SharedTask() {}

	void AddMember(std::string name, bool leader = false) { members.push_back({name, leader}); }
	inline void SetMissingCount(int in) { missing_count = in; }

private:
	int id; // id we have in our map
	int task_id; // ID of the task we're on
	int missing_count; // other toons waiting to verify (out of zone, etc)
	std::vector<SharedTaskMember> members;
};

class SharedTaskManager {
public:
	SharedTaskManager() {}
	~SharedTaskManager() {}

	// IPC packet processing
	void HandleTaskRequest(ServerPacket *pack);

private:
	inline int GetNextID() { return ++next_id; }
	int next_id;
	std::unordered_map<int, SharedTask> tasks;
};

#endif /* !SHARED_TASKS_H */
