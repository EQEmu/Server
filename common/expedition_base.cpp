#include "expedition_base.h"
#include "repositories/expeditions_repository.h"

ExpeditionBase::ExpeditionBase(uint32_t id, const std::string& uuid,
	const std::string& expedition_name, const DynamicZoneMember& leader
) :
	m_id(id),
	m_uuid(uuid),
	m_expedition_name(expedition_name),
	m_leader(leader)
{
}

void ExpeditionBase::LoadRepositoryResult(ExpeditionsRepository::ExpeditionWithLeader&& entry)
{
	m_id                 = entry.id;
	m_uuid               = std::move(entry.uuid);
	m_expedition_name    = std::move(entry.expedition_name);
	m_add_replay_on_join = entry.add_replay_on_join;
	m_is_locked          = entry.is_locked;
	m_leader.id          = entry.leader_id;
	m_leader.name        = std::move(entry.leader_name);
}
