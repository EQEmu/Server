#ifndef SHARED_TASKS_H
#define SHARED_TASKS_H
#include <unordered_map>
#include <vector>
#include <string>

#include "../common/servertalk.h"
#include "../common/global_tasks.h"

class ClientListEntry;

struct SharedTaskMember {
	std::string name;
	ClientListEntry *cle;
	bool leader;
	// TODO: monster mission stuff
	SharedTaskMember() : cle(nullptr), leader(false) {}
	SharedTaskMember(std::string name, ClientListEntry *cle, bool leader) : name(name), cle(cle), leader(leader) {}
};

class SharedTask {
public:
	SharedTask() : id(0), task_id(0), accepted_time(0), locked(false) {}
	SharedTask(int id, int task_id) : id(id), task_id(task_id), accepted_time(0), locked(false) {}
	~SharedTask() {}

	void AddMember(std::string name, ClientListEntry *cle = nullptr, bool leader = false)
	{
		members.push_back({name, cle, leader});
		if (leader)
			leader_name = name;
	}
	void MemberLeftGame(ClientListEntry *cle);
	const std::string &GetLeaderName() const { return leader_name; }

	void SerializeMembers(SerializeBuffer &buf, bool include_leader = true) const;
	void SetCLESharedTasks();

private:
	inline void SetID(int in) { id = in; }
	inline void SetTaskID(int in) { task_id = in; }
	inline void SetAcceptedTime(int in) { accepted_time = in; }
	inline void SetLocked(bool in) { locked = in; }
	int id; // id we have in our map
	int task_id; // ID of the task we're on
	int accepted_time; // timestamp of when accepted, initially set by zone
	bool locked;
	std::string leader_name;
	std::vector<SharedTaskMember> members;
	ClientTaskInformation task_state; // book keeping

	friend class SharedTaskManager;
};

class SharedTaskManager {
public:
	SharedTaskManager() : next_id(0) {}
	~SharedTaskManager() {}

	bool LoadSharedTaskState();
	bool LoadSharedTasks(int single_task = 0);

	bool AppropriateLevel(int id, int level) const;

	// IPC packet processing
	void HandleTaskRequest(ServerPacket *pack);

	void Process();

private:
	int GetNextID();
	int next_id;
	std::unordered_map<int, SharedTask> tasks; // current active shared task states
	std::unordered_map<int, TaskInformation> task_information; // task info shit
};

#endif /* !SHARED_TASKS_H */
