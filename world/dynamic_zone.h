#ifndef WORLD_DYNAMIC_ZONE_H
#define WORLD_DYNAMIC_ZONE_H

#include "../common/eq_constants.h"
#include <chrono>

class ServerPacket;

enum class DynamicZoneStatus
{
	Unknown = 0,
	Normal,
	Expired,
	ExpiredEmpty,
};

class DynamicZone
{
public:
	DynamicZone() = default;
	DynamicZone(uint32_t id, uint32_t zone_id, uint32_t instance_id, uint32_t zone_version,
		uint32_t start_time, uint32_t duration, DynamicZoneType type);

	static DynamicZone* FindDynamicZoneByID(uint32_t dz_id);
	static void HandleZoneMessage(ServerPacket* pack);

	uint32_t GetID() const { return m_id; }
	uint16_t GetInstanceID() const { return static_cast<uint16_t>(m_instance_id); }
	uint16_t GetZoneID() const { return static_cast<uint16_t>(m_zone_id); }
	uint32_t GetZoneVersion() const { return m_zone_version; }
	std::chrono::system_clock::duration GetRemainingDuration() const {
		return m_expire_time - std::chrono::system_clock::now(); }

	DynamicZoneStatus Process(bool force_expire);
	bool IsExpired() const { return m_expire_time < std::chrono::system_clock::now(); }
	void SetSecondsRemaining(uint32_t seconds_remaining);

private:
	void SendZonesDurationUpdate();

	uint32_t m_id = 0;
	uint32_t m_instance_id = 0;
	uint32_t m_zone_id = 0;
	uint32_t m_zone_version = 0;
	bool m_is_pending_early_shutdown = false;
	DynamicZoneType m_type{ DynamicZoneType::None };
	std::chrono::seconds m_duration;
	std::chrono::time_point<std::chrono::system_clock> m_start_time;
	std::chrono::time_point<std::chrono::system_clock> m_expire_time;
};

#endif
