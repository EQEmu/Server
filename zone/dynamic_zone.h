/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef DYNAMIC_ZONE_H
#define DYNAMIC_ZONE_H

#include "../common/eq_constants.h"
#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class MySQLRequestRow;
class ServerPacket;

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

class DynamicZone
{
public:
	DynamicZone() = default;
	DynamicZone(uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type);
	DynamicZone(std::string zone_shortname, uint32_t version, uint32_t duration, DynamicZoneType type);
	DynamicZone(uint32_t dz_id) : m_id(dz_id) {}
	DynamicZone(DynamicZoneType type) : m_type(type) {}

	static DynamicZone* FindDynamicZoneByID(uint32_t dz_id);
	static std::unordered_map<uint32_t, DynamicZone> LoadMultipleDzFromDatabase(
		const std::vector<uint32_t>& dynamic_zone_ids);
	static void HandleWorldMessage(ServerPacket* pack);

	uint64_t GetExpireTime() const { return std::chrono::system_clock::to_time_t(m_expire_time); }
	uint32_t GetID() const { return m_id; }
	uint16_t GetInstanceID() const { return static_cast<uint16_t>(m_instance_id); }
	uint32_t GetSecondsRemaining() const;
	uint16_t GetZoneID() const { return static_cast<uint16_t>(m_zone_id); }
	uint32_t GetZoneIndex() const { return (m_instance_id << 16) | (m_zone_id & 0xffff); }
	uint32_t GetZoneVersion() const { return m_version; }
	const std::string& GetLeaderName() const { return m_leader_name; }
	const std::string& GetName() const { return m_name; }
	DynamicZoneType GetType() const { return m_type; }
	DynamicZoneLocation GetCompassLocation() const { return m_compass; }
	DynamicZoneLocation GetSafeReturnLocation() const { return m_safereturn; }
	DynamicZoneLocation GetZoneInLocation() const { return m_zonein; }

	void     AddCharacter(uint32_t character_id);
	uint32_t Create();
	uint32_t CreateInstance();
	bool     HasZoneInLocation() const { return m_has_zonein; }
	bool     IsCurrentZoneDzInstance() const;
	bool     IsInstanceID(uint32_t instance_id) const;
	bool     IsValid() const { return m_instance_id != 0; }
	bool     IsSameDz(uint32_t zone_id, uint32_t instance_id) const;
	void     RegisterOnCompassChange(const std::function<void()>& on_change) { m_on_compass_change = on_change; }
	void     RemoveAllCharacters(bool enable_removal_timers = true);
	void     RemoveCharacter(uint32_t character_id);
	void     SaveInstanceMembersToDatabase(const std::vector<uint32_t>& character_ids);
	void     SendInstanceCharacterChange(uint32_t character_id, bool removed);
	void     SetCompass(const DynamicZoneLocation& location, bool update_db = false);
	void     SetCompass(uint32_t zone_id, float x, float y, float z, bool update_db = false);
	void     SetLeaderName(const std::string& leader_name) { m_leader_name = leader_name; }
	void     SetName(const std::string& name) { m_name = name; }
	void     SetSafeReturn(const DynamicZoneLocation& location, bool update_db = false);
	void     SetSafeReturn(uint32_t zone_id, float x, float y, float z, float heading, bool update_db = false);
	void     SetSecondsRemaining(uint32_t seconds_remaining);
	void     SetZoneInLocation(const DynamicZoneLocation& location, bool update_db = false);
	void     SetZoneInLocation(float x, float y, float z, float heading, bool update_db = false);
	void     SetUpdatedDuration(uint32_t seconds);

private:
	static std::string DynamicZoneSelectQuery();
	void LoadDatabaseResult(MySQLRequestRow& row);
	void SaveCompassToDatabase();
	void SaveSafeReturnToDatabase();
	void SaveZoneInLocationToDatabase();
	void SendWorldSetLocation(uint16_t server_opcode, const DynamicZoneLocation& location);
	uint32_t SaveToDatabase();

	uint32_t m_id            = 0;
	uint32_t m_zone_id       = 0;
	uint32_t m_instance_id   = 0;
	uint32_t m_version       = 0;
	bool     m_never_expires = false;
	bool     m_has_zonein    = false;
	std::string m_name;
	std::string m_leader_name;
	DynamicZoneType m_type{ DynamicZoneType::None };
	DynamicZoneLocation m_compass;
	DynamicZoneLocation m_safereturn;
	DynamicZoneLocation m_zonein;
	std::chrono::seconds m_duration;
	std::chrono::time_point<std::chrono::system_clock> m_start_time;
	std::chrono::time_point<std::chrono::system_clock> m_expire_time;
	std::function<void()> m_on_compass_change;
};

#endif
