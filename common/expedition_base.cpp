#include "expedition_base.h"
#include "repositories/expeditions_repository.h"

ExpeditionBase::ExpeditionBase(uint32_t id, const std::string& uuid,
	const std::string& expedition_name, const ExpeditionMember& leader,
	uint32_t min_players, uint32_t max_players
) :
	m_id(id),
	m_uuid(uuid),
	m_expedition_name(expedition_name),
	m_leader(leader),
	m_min_players(min_players),
	m_max_players(max_players)
{
}

void ExpeditionBase::LoadRepositoryResult(ExpeditionsRepository::ExpeditionWithLeader&& entry)
{
	m_id                 = entry.id;
	m_uuid               = std::move(entry.uuid);
	m_expedition_name    = std::move(entry.expedition_name);
	m_min_players        = entry.min_players;
	m_max_players        = entry.max_players;
	m_add_replay_on_join = entry.add_replay_on_join;
	m_is_locked          = entry.is_locked;
	m_leader.char_id     = entry.leader_id;
	m_leader.name        = std::move(entry.leader_name);
}

void ExpeditionBase::AddMemberFromRepositoryResult(
	ExpeditionMembersRepository::MemberWithName&& entry)
{
	auto status = ExpeditionMemberStatus::Unknown;
	AddInternalMember({ entry.character_id, std::move(entry.character_name), status });
}

void ExpeditionBase::AddInternalMember(const ExpeditionMember& member)
{
	if (!HasMember(member.char_id))
	{
		m_members.emplace_back(member);
	}
}

void ExpeditionBase::RemoveInternalMember(uint32_t character_id)
{
	m_members.erase(std::remove_if(m_members.begin(), m_members.end(),
		[&](const ExpeditionMember& member) { return member.char_id == character_id; }
	), m_members.end());
}


bool ExpeditionBase::HasMember(uint32_t character_id)
{
	return std::any_of(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return member.char_id == character_id;
	});
}

bool ExpeditionBase::HasMember(const std::string& character_name)
{
	return std::any_of(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return (strcasecmp(member.name.c_str(), character_name.c_str()) == 0);
	});
}

ExpeditionMember ExpeditionBase::GetMemberData(uint32_t character_id)
{
	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return member.char_id == character_id;
	});

	ExpeditionMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}

ExpeditionMember ExpeditionBase::GetMemberData(const std::string& character_name)
{
	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const ExpeditionMember& member) {
		return (strcasecmp(member.name.c_str(), character_name.c_str()) == 0);
	});

	ExpeditionMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}
