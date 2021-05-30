#include "expedition_base.h"
#include "repositories/expeditions_repository.h"

ExpeditionBase::ExpeditionBase(uint32_t id, uint32_t dz_id) :
	m_id(id),
	m_dynamic_zone_id(dz_id)
{
}

void ExpeditionBase::LoadRepositoryResult(const ExpeditionsRepository::Expeditions& entry)
{
	m_id                 = entry.id;
	m_dynamic_zone_id    = entry.dynamic_zone_id;
	m_add_replay_on_join = entry.add_replay_on_join;
	m_is_locked          = entry.is_locked;
}
