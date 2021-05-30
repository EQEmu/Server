#ifndef COMMON_EXPEDITION_BASE_H
#define COMMON_EXPEDITION_BASE_H

#include "dynamic_zone_base.h"
#include "repositories/expeditions_repository.h"
#include <cstdint>
#include <string>

class ExpeditionBase
{
public:
	virtual ~ExpeditionBase() = default;
	ExpeditionBase(const ExpeditionBase&) = default;
	ExpeditionBase(ExpeditionBase&&) = default;
	ExpeditionBase& operator=(const ExpeditionBase&) = default;
	ExpeditionBase& operator=(ExpeditionBase&&) = default;

	uint32_t GetID() const { return m_id; }
	uint32_t GetDynamicZoneID() const { return m_dynamic_zone_id; }

	void LoadRepositoryResult(const ExpeditionsRepository::Expeditions& entry);

protected:
	ExpeditionBase() = default;
	ExpeditionBase(uint32_t id, uint32_t dz_id);

	uint32_t m_id = 0;
	uint32_t m_dynamic_zone_id = 0;
	bool m_is_locked = false;
	bool m_add_replay_on_join = true;
};

#endif
