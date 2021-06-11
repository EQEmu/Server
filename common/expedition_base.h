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
	uint32_t GetLeaderID() const { return m_leader.id; }
	const std::string& GetName() const { return m_expedition_name; }
	const std::string& GetLeaderName() const { return m_leader.name; }
	const std::string& GetUUID() const { return m_uuid; }
	const DynamicZoneMember& GetLeader() const { return m_leader; }

	void LoadRepositoryResult(ExpeditionsRepository::ExpeditionWithLeader&& entry);

protected:
	ExpeditionBase() = default;
	ExpeditionBase(uint32_t id, const std::string& uuid, const std::string& expedition_name,
		const DynamicZoneMember& leader);

	uint32_t m_id = 0;
	bool m_is_locked = false;
	bool m_add_replay_on_join = true;
	std::string m_uuid;
	std::string m_expedition_name;
	DynamicZoneMember m_leader;
};

#endif
