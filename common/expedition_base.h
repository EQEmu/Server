#ifndef COMMON_EXPEDITION_BASE_H
#define COMMON_EXPEDITION_BASE_H

#include "eq_constants.h"
#include "repositories/expeditions_repository.h"
#include "repositories/expedition_members_repository.h"
#include <cstdint>
#include <string>
#include <vector>

struct ExpeditionMember
{
	uint32_t char_id = 0;
	std::string name;
	ExpeditionMemberStatus status = ExpeditionMemberStatus::Online;

	ExpeditionMember() = default;
	ExpeditionMember(uint32_t id, const std::string& name_)
		: char_id(id), name(name_) {}
	ExpeditionMember(uint32_t id, const std::string& name_, ExpeditionMemberStatus status_)
		: char_id(id), name(name_), status(status_) {}

	bool IsValid() const { return char_id != 0 && !name.empty(); }
};

class ExpeditionBase
{
public:
	virtual ~ExpeditionBase() = default;
	ExpeditionBase(const ExpeditionBase&) = default;
	ExpeditionBase(ExpeditionBase&&) = default;
	ExpeditionBase& operator=(const ExpeditionBase&) = default;
	ExpeditionBase& operator=(ExpeditionBase&&) = default;

	uint32_t GetID() const { return m_id; }
	uint32_t GetLeaderID() const { return m_leader.char_id; }
	uint32_t GetMinPlayers() const { return m_min_players; }
	uint32_t GetMaxPlayers() const { return m_max_players; }
	uint32_t GetMemberCount() const { return static_cast<uint32_t>(m_members.size()); }
	const std::string& GetName() const { return m_expedition_name; }
	const std::string& GetLeaderName() const { return m_leader.name; }
	const std::string& GetUUID() const { return m_uuid; }
	const std::vector<ExpeditionMember>& GetMembers() const { return m_members; }

	void AddInternalMember(const ExpeditionMember& member);
	void ClearInternalMembers() { m_members.clear(); }
	bool HasMember(const std::string& character_name);
	bool HasMember(uint32_t character_id);
	bool IsEmpty() const { return m_members.empty(); }
	void RemoveInternalMember(uint32_t character_id);

	void LoadRepositoryResult(ExpeditionsRepository::ExpeditionWithLeader&& entry);
	void AddMemberFromRepositoryResult(ExpeditionMembersRepository::MemberWithName&& entry);

protected:
	ExpeditionBase() = default;
	ExpeditionBase(uint32_t id, const std::string& uuid, const std::string& expedition_name,
		const ExpeditionMember& leader, uint32_t min_players, uint32_t max_players);

	ExpeditionMember GetMemberData(uint32_t character_id);
	ExpeditionMember GetMemberData(const std::string& character_name);

	uint32_t m_id = 0;
	uint32_t m_min_players = 0;
	uint32_t m_max_players = 0;
	bool m_is_locked = false;
	bool m_add_replay_on_join = true;
	std::string m_uuid;
	std::string m_expedition_name;
	ExpeditionMember m_leader;
	std::vector<ExpeditionMember> m_members;
};

#endif
