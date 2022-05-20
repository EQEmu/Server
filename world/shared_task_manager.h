#ifndef EQEMU_SHARED_TASK_MANAGER_H
#define EQEMU_SHARED_TASK_MANAGER_H

#include "../common/database.h"
#include "../common/shared_tasks.h"

class DynamicZone;

namespace EQ {
	namespace Net {
		class DynamicPacket;
	}
}

struct SharedTaskActiveInvitation {
	uint32 shared_task_id;
	uint32 character_id;
};

class SharedTaskManager {
public:
	SharedTaskManager *SetDatabase(Database *db);
	SharedTaskManager *SetContentDatabase(Database *db);

	// loads task data into memory
	SharedTaskManager *LoadTaskData();

	// loads shared task state into memory
	void LoadSharedTaskState();

	// helper, references task memory data
	TasksRepository::Tasks GetSharedTaskDataByTaskId(uint32 task_id);
	std::vector<TaskActivitiesRepository::TaskActivities> GetSharedTaskActivityDataByTaskId(uint32 task_id);

	// gets group / raid members belonging to requested character
	std::vector<SharedTaskMember> GetRequestMembers(
		uint32 requestor_character_id,
		const std::vector<CharacterDataRepository::CharacterData> &characters
	);

	// client attempting to create a shared task
	void AttemptSharedTaskCreation(uint32 requested_task_id, uint32 requested_character_id, uint32 npc_type_id);
	void AttemptSharedTaskRemoval(uint32 requested_task_id, uint32 requested_character_id, bool remove_from_db);

	// shared task activity update middleware
	void SharedTaskActivityUpdate(
		uint32 source_character_id,
		uint32 task_id,
		uint32 activity_id,
		uint32 done_count,
		bool ignore_quest_update
	);

	SharedTask *FindSharedTaskByTaskIdAndCharacterId(uint32 task_id, uint32 character_id);
	SharedTask *FindSharedTaskById(int64 shared_task_id);

	void DeleteSharedTask(int64 shared_task_id);
	void SaveSharedTaskActivityState(int64 shared_task_id, std::vector<SharedTaskActivityStateEntry> activity_state);

	bool IsSharedTaskLeader(SharedTask *s, uint32 character_id);
	void SendAcceptNewSharedTaskPacket(uint32 character_id, uint32 task_id, uint32_t npc_context_id, int accept_time);
	void SendRemovePlayerFromSharedTaskPacket(uint32 character_id, uint32 task_id, bool remove_from_db);
	void SendSharedTaskMemberList(uint32 character_id, const std::vector<SharedTaskMember> &members);
	void SendSharedTaskMemberList(uint32 character_id, const EQ::Net::DynamicPacket &serialized_members);
	void SendSharedTaskMemberChange(
		uint32 character_id,
		int64 shared_task_id,
		const std::string &player_name,
		bool removed
	);
	void RemovePlayerFromSharedTask(SharedTask *s, uint32 character_id);
	void PrintSharedTaskState();
	void RemovePlayerFromSharedTaskByPlayerName(SharedTask *s, const std::string &character_name);
	void RemoveEveryoneFromSharedTask(SharedTask *s, uint32 requested_character_id);

	void MakeLeaderByPlayerName(SharedTask *s, const std::string &character_name);
	void AddPlayerByCharacterIdAndName(SharedTask *s, int64 character_id, const std::string &character_name);
	void InvitePlayerByPlayerName(SharedTask *s, const std::string &player_name);

	// invitations
	void QueueActiveInvitation(int64 shared_task_id, int64 character_id);
	bool IsInvitationActive(uint32 shared_task_id, uint32 character_id);
	void RemoveActiveInvitation(int64 shared_task_id, int64 character_id);
	void RemoveActiveInvitationByCharacterID(uint32_t character_id);

	// dz
	void CreateDynamicZone(SharedTask *s, DynamicZone &dz_request);

	void PurgeAllSharedTasks();

	// messages
	void SendLeaderMessage(SharedTask *s, int chat_type, const std::string &message);
	void SendLeaderMessageID(SharedTask *s, int chat_type, int eqstr_id, std::initializer_list<std::string> args = {});
	void SendMembersMessage(SharedTask *s, int chat_type, const std::string &message);
	void SendMembersMessageID(SharedTask *s, int chat_type, int eqstr_id, std::initializer_list<std::string> args = {});

	const std::vector<SharedTask> &GetSharedTasks() const;
	void SetSharedTasks(const std::vector<SharedTask> &shared_tasks);

	SharedTaskManager * PurgeExpiredSharedTasks();
protected:
	// reference to database
	Database *m_database;
	Database *m_content_database;

	// reference to task data (all)
	std::vector<TasksRepository::Tasks> m_task_data{};
	std::vector<TaskActivitiesRepository::TaskActivities> m_task_activity_data{};

	// internal shared tasks list
	std::vector<SharedTask> m_shared_tasks{};

	// store a reference of active invitations that have been sent to players
	std::vector<SharedTaskActiveInvitation> m_active_invitations{};

	void AddReplayTimers(SharedTask *s);
	bool CanAddPlayer(SharedTask *s, uint32_t character_id, std::string player_name, bool accepted);
	bool CanRequestSharedTask(uint32_t task_id, uint32_t character_id, const SharedTaskRequestCharacters &request);
	void ChooseNewLeader(SharedTask *s);
	void SendSharedTaskMemberListToAllMembers(SharedTask *s);
	void SendSharedTaskMemberAddedToAllMembers(SharedTask *s, const std::string &player_name);
	void SendSharedTaskMemberRemovedToAllMembers(SharedTask *s, const std::string &player_name);
	void SaveMembers(SharedTask *s, std::vector<SharedTaskMember> members);
	void SendSharedTaskInvitePacket(SharedTask *s, int64 invited_character_id);
	void RecordSharedTaskCompletion(SharedTask *s);
	void RemoveAllMembersFromDynamicZones(SharedTask *s);

	// memory search
	std::vector<uint32_t> FindCharactersInSharedTasks(const std::vector<uint32_t> &find_characters);
};

#endif //EQEMU_SHARED_TASK_MANAGER_H
