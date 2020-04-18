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

#ifndef DYNAMICZONE_H
#define DYNAMICZONE_H

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

class ServerPacket;

enum class DynamicZoneType : uint8_t // DynamicZoneActiveType
{
	None = 0,
	Expedition,
	Tutorial,
	Task,
	Mission, // Shared Task
	Quest
};

struct DynamicZoneLocation
{
	uint32_t zone_id = 0;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float heading = 0.0f;

	DynamicZoneLocation() {}
	DynamicZoneLocation(uint32_t zone_id_, float x_, float y_, float z_, float heading_)
		: zone_id(zone_id_), x(x_), y(y_), z(z_), heading(heading_) {}
};

class DynamicZone
{
public:
	DynamicZone() = default;
	DynamicZone(uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type);
	DynamicZone(std::string zone_shortname, uint32_t version, uint32_t duration, DynamicZoneType type);
	DynamicZone(DynamicZoneType type) : m_type(type) { }

	static DynamicZone LoadDzFromDatabase(uint32_t instance_id);
	static void HandleWorldMessage(ServerPacket* pack);

	DynamicZoneType     GetType() const { return m_type; }
	DynamicZoneLocation GetCompassLocation() const { return m_compass; }
	DynamicZoneLocation GetSafeReturnLocation() const { return m_safereturn; }
	DynamicZoneLocation GetZoneInLocation() const { return m_zonein; }

	uint32_t CreateInstance();
	void     AddCharacter(uint32_t character_id);
	void     SaveInstanceMembersToDatabase(const std::unordered_set<uint32_t> character_ids);

	uint64_t GetExpireTime() const { return std::chrono::system_clock::to_time_t(m_expire_time); }
	uint16_t GetInstanceID() const { return static_cast<uint16_t>(m_instance_id); };
	//uint32_t GetRealID() const { return (m_instance_id << 16) | (m_zone_id & 0xffff); }
	uint32_t GetSecondsRemaining() const;
	uint16_t GetZoneID() const { return static_cast<uint16_t>(m_zone_id); };
	uint32_t GetZoneVersion() const { return m_version; };

	bool     HasZoneInLocation() const { return m_has_zonein; }
	bool     IsCurrentZoneDzInstance() const;
	bool     IsInstanceID(uint32_t instance_id) const;
	bool     IsValid() const { return m_instance_id != 0; }
	void     RemoveAllCharacters();
	void     RemoveCharacter(uint32_t character_id);
	void     SendInstanceCharacterChange(uint32_t character_id, bool removed);
	void     SetCompass(const DynamicZoneLocation& location, bool update_db = false);
	void     SetSafeReturn(const DynamicZoneLocation& location, bool update_db = false);
	void     SetZoneInLocation(const DynamicZoneLocation& location, bool update_db = false);
	void     UpdateExpireTime(uint32_t seconds);

	void     LoadFromDatabase(uint32_t instance_id);
	uint32_t SaveToDatabase();

private:
	void DeleteFromDatabase();
	void SaveCompassToDatabase();
	void SaveSafeReturnToDatabase();
	void SaveZoneInLocationToDatabase();

	uint32_t m_zone_id       = 0;
	uint32_t m_instance_id   = 0;
	uint32_t m_version       = 0;
	uint32_t m_start_time    = 0;
	uint32_t m_duration      = 0;
	bool     m_never_expires = false;
	bool     m_has_zonein    = false;
	DynamicZoneType m_type   = DynamicZoneType::None;
	DynamicZoneLocation m_compass;
	DynamicZoneLocation m_safereturn;
	DynamicZoneLocation m_zonein;
	std::chrono::time_point<std::chrono::system_clock> m_expire_time; //uint64_t m_expire_time = 0;
};

#endif
