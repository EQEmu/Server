#ifndef WORLD_DYNAMIC_ZONE_H
#define WORLD_DYNAMIC_ZONE_H

#include "../common/dynamic_zone_base.h"

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

	DynamicZoneStatus Process(bool force_expire);

protected:
	Database& GetDatabase() override;
	void SendInstanceAddRemoveCharacter(uint32_t character_id, bool remove) override;
	void SendInstanceRemoveAllCharacters() override;
	void SendGlobalLocationChange(uint16_t server_opcode, const DynamicZoneLocation& location) override;

private:
	void SendZonesDurationUpdate();

	bool m_is_pending_early_shutdown = false;
};

#endif
