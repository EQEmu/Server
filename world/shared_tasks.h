#ifndef SHARED_TASKS_H
#define SHARED_TASKS_H
#include <unordered_map>
#include <vector>
#include <string>

#include "../common/servertalk.h"
#include "../common/global_tasks.h"
#include "cliententry.h"

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
	SharedTask() : id(0), task_id(0), locked(false) {}
	SharedTask(int id, int task_id) : id(id), task_id(task_id), locked(false) {}
	~SharedTask() {}

	void AddMember(std::string name, ClientListEntry *cle = nullptr, bool leader = false)
	{
		members.push_back({name, cle, leader});
		if (leader)
			leader_name = name;
		auto it = std::find(char_ids.begin(), char_ids.end(), cle->CharID());
		if (it == char_ids.end())
			char_ids.push_back(cle->CharID());
	}
	void MemberLeftGame(ClientListEntry *cle);
	inline const std::string &GetLeaderName() const { return leader_name; }
	inline SharedTaskMember *GetLeader() {
		auto it = std::find_if(members.begin(), members.end(), [](const SharedTaskMember &m) { return m.leader; });
		if (it != members.end())
			return &(*it);
		else
			return nullptr;
	}

	void SerializeMembers(SerializeBuffer &buf, bool include_leader = true) const;
	void SetCLESharedTasks();
	void InitActivities();
	bool UnlockActivities();

	void Save() const; // save to database

private:
	inline void SetID(int in) { id = in; }
	inline void SetTaskID(int in) { task_id = in; }
	inline void SetAcceptedTime(int in) { task_state.AcceptedTime = in; }
	inline void SetLocked(bool in) { locked = in; }

	inline int GetAcceptedTime() const { return task_state.AcceptedTime; }
	int id; // id we have in our map
	int task_id; // ID of the task we're on
	bool locked;
	std::string leader_name;
	std::vector<SharedTaskMember> members;
	std::vector<int> char_ids; // every char id of someone to be locked out, different in case they leave/removed
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

	inline SharedTask *GetSharedTask(int id) {
		auto it = tasks.find(id);
		if (it != tasks.end())
			return &it->second;
		else
			return nullptr;
	}

	inline int GetTaskActivityCount(int task_id) const {
		auto it = task_information.find(task_id);
		if (it != task_information.end())
			return it->second.ActivityCount;
		else
			return 0; // hmm
	}

	// IPC packet processing
	void HandleTaskRequest(ServerPacket *pack);
	void HandleTaskZoneCreated(ServerPacket *pack);

	void Process();

private:
	int GetNextID();
	int next_id;
	std::unordered_map<int, SharedTask> tasks; // current active shared task states
	std::unordered_map<int, TaskInformation> task_information; // task info shit
};

#endif /* !SHARED_TASKS_H */
