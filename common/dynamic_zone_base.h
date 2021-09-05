#ifndef COMMON_DYNAMIC_ZONE_BASE_H
#define COMMON_DYNAMIC_ZONE_BASE_H

#include "eq_constants.h"
#include "net/packet.h"
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
	DynamicZoneMemberStatus status = DynamicZoneMemberStatus::Unknown;

	DynamicZoneMember() = default;
	DynamicZoneMember(uint32_t id, std::string name_)
		: id(id), name{std::move(name_)} {}
	DynamicZoneMember(uint32_t id, std::string name_, DynamicZoneMemberStatus status_)
		: id(id), name{std::move(name_)}, status(status_) {}

	bool IsOnline() const { return status == DynamicZoneMemberStatus::Online ||
	                               status == DynamicZoneMemberStatus::InDynamicZone; }
	bool IsValid() const { return id != 0 && !name.empty(); }

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(id, name, status);
	}
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

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(zone_id, x, y, z, heading);
	}
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
	uint32_t GetLeaderID() const { return m_leader.id; }
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
	const std::string& GetUUID() const { return m_uuid; }
	const DynamicZoneMember& GetLeader() const { return m_leader; }
	const std::vector<DynamicZoneMember>& GetMembers() const { return m_members; }
	const DynamicZoneLocation& GetCompassLocation() const { return m_compass; }
	const DynamicZoneLocation& GetSafeReturnLocation() const { return m_safereturn; }
	const DynamicZoneLocation& GetZoneInLocation() const { return m_zonein; }
	std::chrono::system_clock::duration GetDurationRemaining() const { return m_expire_time - std::chrono::system_clock::now(); }

	bool AddMember(const DynamicZoneMember& add_member);
	void AddMemberFromRepositoryResult(DynamicZoneMembersRepository::MemberWithName&& entry);
	uint32_t GetDatabaseMemberCount();
	DynamicZoneMember GetMemberData(uint32_t character_id);
	DynamicZoneMember GetMemberData(const std::string& character_name);
	EQ::Net::DynamicPacket GetSerializedDzPacket();
	bool HasDatabaseMember(uint32_t character_id);
	bool HasMember(uint32_t character_id);
	bool HasMember(const std::string& character_name);
	bool HasMembers() const { return !m_members.empty(); }
	bool HasZoneInLocation() const { return m_has_zonein; }
	bool IsExpired() const { return m_expire_time < std::chrono::system_clock::now(); }
	bool IsInstanceID(uint32_t instance_id) const { return (m_instance_id != 0 && m_instance_id == instance_id); }
	bool IsValid() const { return m_instance_id != 0; }
	bool IsSameDz(uint32_t zone_id, uint32_t instance_id) const { return zone_id == m_zone_id && instance_id == m_instance_id; }
	void LoadSerializedDzPacket(char* cereal_data, uint32_t cereal_size);
	void RemoveAllMembers();
	bool RemoveMember(uint32_t character_id);
	bool RemoveMember(const std::string& character_name);
	bool RemoveMember(const DynamicZoneMember& remove_member);
	void SaveMembers(const std::vector<DynamicZoneMember>& members);
	void SetCompass(const DynamicZoneLocation& location, bool update_db = false);
	void SetCompass(uint32_t zone_id, float x, float y, float z, bool update_db = false);
	void SetDuration(uint32_t seconds) { m_duration = std::chrono::seconds(seconds); }
	void SetLeader(const DynamicZoneMember& leader, bool update_db = false);
	void SetMaxPlayers(uint32_t max_players) { m_max_players = max_players; }
	void SetMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status);
	void SetMinPlayers(uint32_t min_players) { m_min_players = min_players; }
	void SetName(const std::string& name) { m_name = name; }
	void SetSafeReturn(const DynamicZoneLocation& location, bool update_db = false);
	void SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading, bool update_db = false);
	void SetType(DynamicZoneType type) { m_type = type; }
	void SetUUID(std::string uuid) { m_uuid = std::move(uuid); }
	void SetZoneInLocation(const DynamicZoneLocation& location, bool update_db = false);
	void SetZoneInLocation(float x, float y, float z, float heading, bool update_db = false);
	bool SwapMember(const DynamicZoneMember& add_member, const std::string& remove_char_name);

protected:
	virtual uint16_t GetCurrentInstanceID() { return 0; }
	virtual uint16_t GetCurrentZoneID() { return 0; }
	virtual Database& GetDatabase() = 0;
	virtual void ProcessCompassChange(const DynamicZoneLocation& location) { m_compass = location; }
	virtual void ProcessMemberAddRemove(const DynamicZoneMember& member, bool removed);
	virtual bool ProcessMemberStatusChange(uint32_t member_id, DynamicZoneMemberStatus status);
	virtual void ProcessRemoveAllMembers(bool silent = false) { m_members.clear(); }
	virtual bool SendServerPacket(ServerPacket* packet) = 0;

	void AddInternalMember(const DynamicZoneMember& member);
	uint32_t Create();
	uint32_t CreateInstance();
	void LoadRepositoryResult(DynamicZonesRepository::DynamicZoneInstance&& dz_entry);
	void RemoveInternalMember(uint32_t character_id);
	uint32_t SaveToDatabase();
	bool SetInternalMemberStatus(uint32_t character_id, DynamicZoneMemberStatus status);

	std::unique_ptr<ServerPacket> CreateServerDzCreatePacket(uint16_t origin_zone_id, uint16_t origin_instance_id);
	std::unique_ptr<ServerPacket> CreateServerDzLocationPacket(uint16_t server_opcode, const DynamicZoneLocation& location);
	std::unique_ptr<ServerPacket> CreateServerMemberAddRemovePacket(const DynamicZoneMember& member, bool removed);
	std::unique_ptr<ServerPacket> CreateServerMemberStatusPacket(uint32_t character_id, DynamicZoneMemberStatus status);
	std::unique_ptr<ServerPacket> CreateServerMemberSwapPacket(const DynamicZoneMember& remove_member, const DynamicZoneMember& add_member);
	std::unique_ptr<ServerPacket> CreateServerRemoveAllMembersPacket();

	uint32_t m_id = 0;
	uint32_t m_zone_id = 0;
	uint32_t m_instance_id = 0;
	uint32_t m_zone_version = 0;
	uint32_t m_min_players = 0;
	uint32_t m_max_players = 0;
	bool m_never_expires = false;
	bool m_has_zonein = false;
	bool m_has_member_statuses = false;
	std::string m_name;
	std::string m_uuid;
	DynamicZoneMember m_leader;
	DynamicZoneType m_type{ DynamicZoneType::None };
	DynamicZoneLocation m_compass;
	DynamicZoneLocation m_safereturn;
	DynamicZoneLocation m_zonein;
	std::chrono::seconds m_duration;
	std::chrono::time_point<std::chrono::system_clock> m_start_time;
	std::chrono::time_point<std::chrono::system_clock> m_expire_time;
	std::vector<DynamicZoneMember> m_members;

public:
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			m_id,
			m_zone_id,
			m_instance_id,
			m_zone_version,
			m_min_players,
			m_max_players,
			m_never_expires,
			m_has_zonein,
			m_has_member_statuses,
			m_name,
			m_uuid,
			m_leader,
			m_type,
			m_compass,
			m_safereturn,
			m_zonein,
			m_duration,
			m_start_time,
			m_expire_time,
			m_members
		);
	}
};

#endif
