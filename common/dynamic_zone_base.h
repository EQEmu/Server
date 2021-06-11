#ifndef COMMON_DYNAMIC_ZONE_BASE_H
#define COMMON_DYNAMIC_ZONE_BASE_H

#include "eq_constants.h"
#include "repositories/dynamic_zones_repository.h"
#include "repositories/dynamic_zone_members_repository.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Database;
class ServerPacket;

struct DynamicZoneMember
{
	uint32_t id = 0;
	std::string name;
	DynamicZoneMemberStatus status = DynamicZoneMemberStatus::Online;

	DynamicZoneMember() = default;
	DynamicZoneMember(uint32_t id, std::string name_)
		: id(id), name{std::move(name_)} {}
	DynamicZoneMember(uint32_t id, std::string name_, DynamicZoneMemberStatus status_)
		: id(id), name{std::move(name_)}, status(status_) {}

	bool IsOnline() const { return status == DynamicZoneMemberStatus::Online ||
	                               status == DynamicZoneMemberStatus::InDynamicZone; }
	bool IsValid() const { return id != 0 && !name.empty(); }
};

struct DynamicZoneLocation
{
	uint32_t zone_id = 0;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float heading = 0.0f;

	DynamicZoneLocation() = default;
	DynamicZoneLocation(uint32_t zone_id_, float x_, float y_, float z_, float heading_)
		: zone_id(zone_id_), x(x_), y(y_), z(z_), heading(heading_) {}
};

class DynamicZoneBase
{
public:
	virtual ~DynamicZoneBase() = default;
	DynamicZoneBase(const DynamicZoneBase&) = default;
	DynamicZoneBase(DynamicZoneBase&&) = default;
	DynamicZoneBase& operator=(const DynamicZoneBase&) = default;
	DynamicZoneBase& operator=(DynamicZoneBase&&) = default;
	DynamicZoneBase() = default;
	DynamicZoneBase(uint32_t dz_id) : m_id(dz_id) {}
	DynamicZoneBase(DynamicZoneType type) : m_type(type) {}
	DynamicZoneBase(DynamicZonesRepository::DynamicZoneInstance&& entry);

	static std::string GetDynamicZoneTypeName(DynamicZoneType dz_type);

	virtual void SetSecondsRemaining(uint32_t seconds_remaining) = 0;

	uint64_t GetExpireTime() const { return std::chrono::system_clock::to_time_t(m_expire_time); }
	uint32_t GetID() const { return m_id; }
	uint16_t GetInstanceID() const { return static_cast<uint16_t>(m_instance_id); }
	uint32_t GetMaxPlayers() const { return m_max_players; }
	uint32_t GetMemberCount() const { return static_cast<uint32_t>(m_members.size()); }
	uint32_t GetMinPlayers() const { return m_min_players; }
	uint32_t GetSecondsRemaining() const;
	uint16_t GetZoneID() const { return static_cast<uint16_t>(m_zone_id); }
	uint32_t GetZoneIndex() const { return (m_instance_id << 16) | (m_zone_id & 0xffff); }
	uint32_t GetZoneVersion() const { return m_zone_version; }
	DynamicZoneType GetType() const { return m_type; }
	const std::string& GetLeaderName() const { return m_leader.name; }
	const std::string& GetName() const { return m_name; }
	const DynamicZoneMember& GetLeader() const { return m_leader; }
	const std::vector<DynamicZoneMember>& GetMembers() const { return m_members; }
	const DynamicZoneLocation& GetCompassLocation() const { return m_compass; }
	const DynamicZoneLocation& GetSafeReturnLocation() const { return m_safereturn; }
	const DynamicZoneLocation& GetZoneInLocation() const { return m_zonein; }
	std::chrono::system_clock::duration GetDurationRemaining() const { return m_expire_time - std::chrono::system_clock::now(); }

	void AddCharacter(uint32_t character_id);
	void AddInternalMember(const DynamicZoneMember& member);
	void AddMemberFromRepositoryResult(DynamicZoneMembersRepository::MemberWithName&& entry);
	void ClearInternalMembers() { m_members.clear(); }
	uint32_t Create();
	uint32_t GetDatabaseMemberCount();
	DynamicZoneMember GetMemberData(uint32_t character_id);
	DynamicZoneMember GetMemberData(const std::string& character_name);
	bool HasDatabaseMember(uint32_t character_id);
	bool HasMember(uint32_t character_id);
	bool HasMember(const std::string& character_name);
	bool HasMembers() const { return !m_members.empty(); }
	bool HasZoneInLocation() const { return m_has_zonein; }
	bool IsExpired() const { return m_expire_time < std::chrono::system_clock::now(); }
	bool IsInstanceID(uint32_t instance_id) const { return (m_instance_id != 0 && m_instance_id == instance_id); }
	bool IsValid() const { return m_instance_id != 0; }
	bool IsSameDz(uint32_t zone_id, uint32_t instance_id) const { return zone_id == m_zone_id && instance_id == m_instance_id; }
	void RemoveAllCharacters(bool enable_removal_timers = true);
	void RemoveCharacter(uint32_t character_id);
	void RemoveInternalMember(uint32_t character_id);
	void SaveMembers(const std::vector<DynamicZoneMember>& members);
	void SetCompass(const DynamicZoneLocation& location, bool update_db = false);
	void SetCompass(uint32_t zone_id, float x, float y, float z, bool update_db = false);
	bool SetInternalMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status);
	void SetLeader(const DynamicZoneMember& leader) { m_leader = leader; }
	void SetMaxPlayers(uint32_t max_players) { m_max_players = max_players; }
	void SetMinPlayers(uint32_t min_players) { m_min_players = min_players; }
	void SetName(const std::string& name) { m_name = name; }
	void SetSafeReturn(const DynamicZoneLocation& location, bool update_db = false);
	void SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading, bool update_db = false);
	void SetZoneInLocation(const DynamicZoneLocation& location, bool update_db = false);
	void SetZoneInLocation(float x, float y, float z, float heading, bool update_db = false);

protected:
	virtual uint16_t GetCurrentInstanceID() { return 0; }
	virtual uint16_t GetCurrentZoneID() { return 0; }
	virtual Database& GetDatabase() = 0;
	virtual void ProcessCompassChange(const DynamicZoneLocation& location) { m_compass = location; }
	virtual void SendInstanceAddRemoveCharacter(uint32_t character_id, bool remove) = 0;
	virtual void SendInstanceRemoveAllCharacters() = 0;
	virtual void SendGlobalLocationChange(uint16_t server_opcode, const DynamicZoneLocation& location) = 0;

	uint32_t CreateInstance();
	void LoadRepositoryResult(DynamicZonesRepository::DynamicZoneInstance&& dz_entry);
	uint32_t SaveToDatabase();

	std::unique_ptr<ServerPacket> CreateServerAddRemoveCharacterPacket(uint32_t character_id, bool removed);
	std::unique_ptr<ServerPacket> CreateServerRemoveAllCharactersPacket();
	std::unique_ptr<ServerPacket> CreateServerDzLocationPacket(uint16_t server_opcode, const DynamicZoneLocation& location);

	uint32_t m_id = 0;
	uint32_t m_zone_id = 0;
	uint32_t m_instance_id = 0;
	uint32_t m_zone_version = 0;
	uint32_t m_min_players = 0;
	uint32_t m_max_players = 0;
	bool m_never_expires = false;
	bool m_has_zonein = false;
	std::string m_name;
	DynamicZoneMember m_leader;
	DynamicZoneType m_type{ DynamicZoneType::None };
	DynamicZoneLocation m_compass;
	DynamicZoneLocation m_safereturn;
	DynamicZoneLocation m_zonein;
	std::chrono::seconds m_duration;
	std::chrono::time_point<std::chrono::system_clock> m_start_time;
	std::chrono::time_point<std::chrono::system_clock> m_expire_time;
	std::vector<DynamicZoneMember> m_members;
};

#endif
