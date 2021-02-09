#include "expedition_base.h"
#include "repositories/expeditions_repository.h"
#include "rulesys.h"

ExpeditionBase::ExpeditionBase(uint32_t id, const std::string& uuid,
	const std::string& expedition_name, const DynamicZoneMember& leader,
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
	m_leader.id          = entry.leader_id;
	m_leader.name        = std::move(entry.leader_name);
}

void ExpeditionBase::AddMemberFromRepositoryResult(
	ExpeditionMembersRepository::MemberWithName&& entry)
{
	auto status = DynamicZoneMemberStatus::Unknown;
	AddInternalMember({ entry.character_id, std::move(entry.character_name), status });
}

void ExpeditionBase::AddInternalMember(const DynamicZoneMember& member)
{
	if (!HasMember(member.id))
	{
		m_members.emplace_back(member);
	}
}

void ExpeditionBase::RemoveInternalMember(uint32_t character_id)
{
	m_members.erase(std::remove_if(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) { return member.id == character_id; }
	), m_members.end());
}


bool ExpeditionBase::HasMember(uint32_t character_id)
{
	return std::any_of(m_members.begin(), m_members.end(), [&](const DynamicZoneMember& member) {
		return member.id == character_id;
	});
}

bool ExpeditionBase::HasMember(const std::string& character_name)
{
	return std::any_of(m_members.begin(), m_members.end(), [&](const DynamicZoneMember& member) {
		return (strcasecmp(member.name.c_str(), character_name.c_str()) == 0);
	});
}

DynamicZoneMember ExpeditionBase::GetMemberData(uint32_t character_id)
{
	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const DynamicZoneMember& member) {
		return member.id == character_id;
	});

	DynamicZoneMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}

DynamicZoneMember ExpeditionBase::GetMemberData(const std::string& character_name)
{
	auto it = std::find_if(m_members.begin(), m_members.end(), [&](const DynamicZoneMember& member) {
		return (strcasecmp(member.name.c_str(), character_name.c_str()) == 0);
	});

	DynamicZoneMember member_data;
	if (it != m_members.end())
	{
		member_data = *it;
	}
	return member_data;
}

bool ExpeditionBase::SetInternalMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status)
{
	if (status == DynamicZoneMemberStatus::InDynamicZone && !RuleB(Expedition, EnableInDynamicZoneStatus))
	{
		status = DynamicZoneMemberStatus::Online;
	}

	if (character_id == m_leader.id)
	{
		m_leader.status = status;
	}

	auto it = std::find_if(m_members.begin(), m_members.end(),
		[&](const DynamicZoneMember& member) { return member.id == character_id; });

	if (it != m_members.end() && it->status != status)
	{
		it->status = status;
		return true;
	}

	return false;
}
