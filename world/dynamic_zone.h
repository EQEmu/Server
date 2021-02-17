#ifndef WORLD_DYNAMIC_ZONE_H
#define WORLD_DYNAMIC_ZONE_H

#include "../common/dynamic_zone_base.h"
#include <functional>

class Database;
class ServerPacket;

enum class DynamicZoneStatus
{
	Unknown = 0,
	Normal,
	Expired,
	ExpiredEmpty,
};

class DynamicZone : public DynamicZoneBase
{
public:
	using DynamicZoneBase::DynamicZoneBase; // inherit base constructors

	static DynamicZone* FindDynamicZoneByID(uint32_t dz_id);
	static void HandleZoneMessage(ServerPacket* pack);

	void SetSecondsRemaining(uint32_t seconds_remaining) override;

	void CacheMemberStatuses();
	DynamicZoneStatus Process();
	void RegisterOnMemberAddRemove(std::function<void(const DynamicZoneMember&, bool)> on_addremove);
	void RegisterOnStatusChanged(std::function<void(const DynamicZoneMember&)> on_status_changed);

protected:
	Database& GetDatabase() override;
	void ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed) override;
	bool ProcessMemberStatusChange(uint32_t member_id, DynamicZoneMemberStatus status) override;
	bool SendServerPacket(ServerPacket* packet) override;

private:
	void SendZoneMemberStatuses(uint16_t zone_id, uint16_t instance_id);
	void SendZonesDurationUpdate();

	bool m_is_pending_early_shutdown = false;
	std::function<void(const DynamicZoneMember&, bool)> m_on_addremove;
	std::function<void(const DynamicZoneMember&)> m_on_status_changed;
};

#endif
