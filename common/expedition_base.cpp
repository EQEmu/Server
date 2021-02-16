#include "expedition_base.h"
#include "repositories/expeditions_repository.h"

ExpeditionBase::ExpeditionBase(uint32_t id) :
	m_id(id)
{
}

void ExpeditionBase::LoadRepositoryResult(const ExpeditionsRepository::ExpeditionWithLeader& entry)
{
	m_id                 = entry.id;
	m_add_replay_on_join = entry.add_replay_on_join;
	m_is_locked          = entry.is_locked;
}
