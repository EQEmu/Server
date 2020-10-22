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

#include "dynamiczone.h"
#include "client.h"
#include "worldserver.h"
#include "zonedb.h"
#include "../common/eqemu_logsys.h"

extern WorldServer worldserver;

DynamicZone::DynamicZone(
	uint32_t zone_id, uint32_t version, uint32_t duration, DynamicZoneType type
) :
	m_zone_id(zone_id),
	m_version(version),
	m_duration(duration),
	m_type(type)
{
}

DynamicZone::DynamicZone(
	std::string zone_shortname, uint32_t version, uint32_t duration, DynamicZoneType type
) :
	m_version(version),
	m_duration(duration),
	m_type(type)
{
	m_zone_id = ZoneID(zone_shortname.c_str());

	if (!m_zone_id)
	{
		LogDynamicZones("Failed to get zone id for zone [{}]", zone_shortname);
	}
}

std::unordered_map<uint32_t, DynamicZone> DynamicZone::LoadMultipleDzFromDatabase(
	const std::vector<uint32_t>& dynamic_zone_ids)
{
	LogDynamicZonesDetail("Loading dynamic zone data for [{}] instances", dynamic_zone_ids.size());

	std::string in_dynamic_zone_ids_query = fmt::format("{}", fmt::join(dynamic_zone_ids, ","));

	std::unordered_map<uint32_t, DynamicZone> dynamic_zones;

	if (!in_dynamic_zone_ids_query.empty())
	{
		std::string query = fmt::format(SQL(
			{} WHERE dynamic_zones.id IN ({});
		), DynamicZoneSelectQuery(), in_dynamic_zone_ids_query);

		auto results = database.QueryDatabase(query);
		if (results.Success())
		{
			for (auto row = results.begin(); row != results.end(); ++row)
			{
				DynamicZone dz;
				dz.LoadDatabaseResult(row);
				dynamic_zones.emplace(dz.GetID(), dz);
			}
		}
	}

	return dynamic_zones;
}

uint32_t DynamicZone::Create()
{
	if (m_id != 0)
	{
		return m_id;
	}

	if (GetInstanceID() == 0)
	{
		CreateInstance();
	}

	m_id = SaveToDatabase();

	return m_id;
}

uint32_t DynamicZone::CreateInstance()
{
	if (m_instance_id)
	{
		LogDynamicZones("CreateInstance failed, instance id [{}] already created", m_instance_id);
		return 0;
	}

	if (!m_zone_id)
	{
		LogDynamicZones("CreateInstance failed, invalid zone id [{}]", m_zone_id);
		return 0;
	}

	uint16_t instance_id = 0;
	if (!database.GetUnusedInstanceID(instance_id)) // todo: doesn't this race with insert?
	{
		LogDynamicZones("Failed to find unused instance id");
		return 0;
	}

	m_start_time    = std::chrono::system_clock::now();
	auto start_time = std::chrono::system_clock::to_time_t(m_start_time);

	std::string query = fmt::format(SQL(
		INSERT INTO instance_list
			(id, zone, version, start_time, duration)
		VALUES
			({}, {}, {}, {}, {})
	), instance_id, m_zone_id, m_version, start_time, m_duration.count());

	auto results = database.QueryDatabase(query);
	if (!results.Success())
	{
		LogDynamicZones("Failed to create instance [{}] for Dynamic Zone [{}]", instance_id, m_zone_id);
		return 0;
	}

	m_instance_id   = instance_id;
	m_never_expires = false;
	m_expire_time   = m_start_time + m_duration;

	return m_instance_id;
}

std::string DynamicZone::DynamicZoneSelectQuery()
{
	return std::string(SQL(
		SELECT
			instance_list.id,
			instance_list.zone,
			instance_list.version,
			instance_list.start_time,
			instance_list.duration,
			instance_list.never_expires,
			dynamic_zones.id,
			dynamic_zones.type,
			dynamic_zones.compass_zone_id,
			dynamic_zones.compass_x,
			dynamic_zones.compass_y,
			dynamic_zones.compass_z,
			dynamic_zones.safe_return_zone_id,
			dynamic_zones.safe_return_x,
			dynamic_zones.safe_return_y,
			dynamic_zones.safe_return_z,
			dynamic_zones.safe_return_heading,
			dynamic_zones.zone_in_x,
			dynamic_zones.zone_in_y,
			dynamic_zones.zone_in_z,
			dynamic_zones.zone_in_heading,
			dynamic_zones.has_zone_in
		FROM dynamic_zones
			INNER JOIN instance_list ON dynamic_zones.instance_id = instance_list.id
	));
}

void DynamicZone::LoadDatabaseResult(MySQLRequestRow& row)
{
	m_instance_id        = strtoul(row[0], nullptr, 10);
	m_zone_id            = strtoul(row[1], nullptr, 10);
	m_version            = strtoul(row[2], nullptr, 10);
	m_start_time         = std::chrono::system_clock::from_time_t(strtoul(row[3], nullptr, 10));
	m_duration           = std::chrono::seconds(strtoul(row[4], nullptr, 10));
	m_expire_time        = m_start_time + m_duration;
	m_never_expires      = (strtoul(row[5], nullptr, 10) != 0);
	m_id                 = strtoul(row[6], nullptr, 10);
	m_type               = static_cast<DynamicZoneType>(strtoul(row[7], nullptr, 10));
	m_compass.zone_id    = strtoul(row[8], nullptr, 10);
	m_compass.x          = strtof(row[9], nullptr);
	m_compass.y          = strtof(row[10], nullptr);
	m_compass.z          = strtof(row[11], nullptr);
	m_safereturn.zone_id = strtoul(row[12], nullptr, 10);
	m_safereturn.x       = strtof(row[13], nullptr);
	m_safereturn.y       = strtof(row[14], nullptr);
	m_safereturn.z       = strtof(row[15], nullptr);
	m_safereturn.heading = strtof(row[16], nullptr);
	m_zonein.x           = strtof(row[17], nullptr);
	m_zonein.y           = strtof(row[18], nullptr);
	m_zonein.z           = strtof(row[19], nullptr);
	m_zonein.heading     = strtof(row[20], nullptr);
	m_has_zonein         = (strtoul(row[21], nullptr, 10) != 0);
}

uint32_t DynamicZone::SaveToDatabase()
{
	LogDynamicZonesDetail("Saving dz instance [{}] to database", m_instance_id);

	if (m_instance_id != 0)
	{
		std::string query = fmt::format(SQL(
			INSERT INTO dynamic_zones
				(
					instance_id,
					type,
					compass_zone_id,
					compass_x,
					compass_y,
					compass_z,
					safe_return_zone_id,
					safe_return_x,
					safe_return_y,
					safe_return_z,
					safe_return_heading,
					zone_in_x,
					zone_in_y,
					zone_in_z,
					zone_in_heading,
					has_zone_in
				)
			VALUES
				({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
		),
			m_instance_id,
			static_cast<uint8_t>(m_type),
			m_compass.zone_id,
			m_compass.x,
			m_compass.y,
			m_compass.z,
			m_safereturn.zone_id,
			m_safereturn.x,
			m_safereturn.y,
			m_safereturn.z,
			m_safereturn.heading,
			m_zonein.x,
			m_zonein.y,
			m_zonein.z,
			m_zonein.heading,
			m_has_zonein
		);

		auto results = database.QueryDatabase(query);
		if (results.Success())
		{
			return results.LastInsertedID();
		}
	}
	return 0;
}

void DynamicZone::SaveCompassToDatabase()
{
	LogDynamicZonesDetail(
		"Instance [{}] saving compass zone: [{}] xyz: ([{}], [{}], [{}])",
		m_instance_id, m_compass.zone_id, m_compass.x, m_compass.y, m_compass.z
	);

	if (m_instance_id != 0)
	{
		std::string query = fmt::format(SQL(
			UPDATE dynamic_zones SET
				compass_zone_id = {},
				compass_x = {},
				compass_y = {},
				compass_z = {}
			WHERE instance_id = {};
		),
			m_compass.zone_id,
			m_compass.x,
			m_compass.y,
			m_compass.z,
			m_instance_id
		);

		database.QueryDatabase(query);
	}
}

void DynamicZone::SaveSafeReturnToDatabase()
{
	LogDynamicZonesDetail(
		"Instance [{}] saving safereturn zone: [{}] xyzh: ([{}], [{}], [{}], [{}])",
		m_instance_id, m_safereturn.zone_id, m_safereturn.x, m_safereturn.y, m_safereturn.z, m_safereturn.heading
	);

	if (m_instance_id != 0)
	{
		std::string query = fmt::format(SQL(
			UPDATE dynamic_zones SET
				safe_return_zone_id = {},
				safe_return_x = {},
				safe_return_y = {},
				safe_return_z = {},
				safe_return_heading = {}
			WHERE instance_id = {};
		),
			m_safereturn.zone_id,
			m_safereturn.x,
			m_safereturn.y,
			m_safereturn.z,
			m_safereturn.heading,
			m_instance_id
		);

		database.QueryDatabase(query);
	}
}

void DynamicZone::SaveZoneInLocationToDatabase()
{
	LogDynamicZonesDetail(
		"Instance [{}] saving zonein zone: [{}] xyzh: ([{}], [{}], [{}], [{}]) has: [{}]",
		m_instance_id, m_zone_id, m_zonein.x, m_zonein.y, m_zonein.z, m_zonein.heading, m_has_zonein
	);

	if (m_instance_id != 0)
	{
		std::string query = fmt::format(SQL(
			UPDATE dynamic_zones SET
				zone_in_x = {},
				zone_in_y = {},
				zone_in_z = {},
				zone_in_heading = {},
				has_zone_in = {}
			WHERE instance_id = {};
		),
			m_zonein.x,
			m_zonein.y,
			m_zonein.z,
			m_zonein.heading,
			m_has_zonein,
			m_instance_id
		);

		database.QueryDatabase(query);
	}
}

void DynamicZone::AddCharacter(uint32_t character_id)
{
	database.AddClientToInstance(m_instance_id, character_id);
	SendInstanceCharacterChange(character_id, false); // stops client kick timer
}

void DynamicZone::RemoveCharacter(uint32_t character_id)
{
	database.RemoveClientFromInstance(m_instance_id, character_id);
	SendInstanceCharacterChange(character_id, true); // start client kick timer
}

void DynamicZone::RemoveAllCharacters(bool enable_removal_timers)
{
	if (GetInstanceID() == 0)
	{
		return;
	}

	if (enable_removal_timers)
	{
		// just remove all clients in bulk instead of only characters assigned to the instance
		if (IsCurrentZoneDzInstance())
		{
			for (const auto& client_iter : entity_list.GetClientList())
			{
				if (client_iter.second)
				{
					client_iter.second->SetDzRemovalTimer(true);
				}
			}
		}
		else if (GetInstanceID() != 0)
		{
			uint32_t packsize = sizeof(ServerDzCharacter_Struct);
			auto pack = std::unique_ptr<ServerPacket>(new ServerPacket(ServerOP_DzRemoveAllCharacters, packsize));
			auto packbuf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
			packbuf->zone_id = GetZoneID();
			packbuf->instance_id = GetInstanceID();
			packbuf->remove = true;
			packbuf->character_id = 0;
			worldserver.SendPacket(pack.get());
		}
	}

	database.RemoveClientsFromInstance(GetInstanceID());
}

void DynamicZone::SaveInstanceMembersToDatabase(const std::vector<uint32_t>& character_ids)
{
	LogDynamicZonesDetail("Saving [{}] instance members to database", character_ids.size());

	std::string insert_values;
	for (const auto& character_id : character_ids)
	{
		fmt::format_to(std::back_inserter(insert_values), "({}, {}),", m_instance_id, character_id);
	}

	if (!insert_values.empty())
	{
		insert_values.pop_back(); // trailing comma

		std::string query = fmt::format(SQL(
			REPLACE INTO instance_list_player (id, charid) VALUES {};
		), insert_values);

		database.QueryDatabase(query);
	}
}

void DynamicZone::SendInstanceCharacterChange(uint32_t character_id, bool removed)
{
	// if removing, sets removal timer on client inside the instance
	if (IsCurrentZoneDzInstance())
	{
		Client* client = entity_list.GetClientByCharID(character_id);
		if (client)
		{
			client->SetDzRemovalTimer(removed);
		}
	}
	else if (GetInstanceID() != 0)
	{
		uint32_t packsize = sizeof(ServerDzCharacter_Struct);
		auto pack = std::unique_ptr<ServerPacket>(new ServerPacket(ServerOP_DzCharacterChange, packsize));
		auto packbuf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
		packbuf->zone_id = GetZoneID();
		packbuf->instance_id = GetInstanceID();
		packbuf->remove = removed;
		packbuf->character_id = character_id;
		worldserver.SendPacket(pack.get());
	}
}

void DynamicZone::SetCompass(const DynamicZoneLocation& location, bool update_db)
{
	m_compass = location;

	if (update_db)
	{
		SaveCompassToDatabase();
	}
}

void DynamicZone::SetSafeReturn(const DynamicZoneLocation& location, bool update_db)
{
	m_safereturn = location;

	if (update_db)
	{
		SaveSafeReturnToDatabase();
	}
}

void DynamicZone::SetZoneInLocation(const DynamicZoneLocation& location, bool update_db)
{
	m_zonein = location;
	m_has_zonein = true;

	if (update_db)
	{
		SaveZoneInLocationToDatabase();
	}
}

bool DynamicZone::IsCurrentZoneDzInstance() const
{
	return (zone && zone->GetInstanceID() != 0 && zone->GetInstanceID() == GetInstanceID());
}

bool DynamicZone::IsInstanceID(uint32_t instance_id) const
{
	return (GetInstanceID() != 0 && GetInstanceID() == instance_id);
}

bool DynamicZone::IsSameDz(uint32_t zone_id, uint32_t instance_id) const
{
	return zone_id == m_zone_id && instance_id == m_instance_id;
}

uint32_t DynamicZone::GetSecondsRemaining() const
{
	auto now = std::chrono::system_clock::now();
	if (m_expire_time > now)
	{
		auto remaining = m_expire_time - now;
		return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(remaining).count());
	}
	return 0;
}

void DynamicZone::SetUpdatedDuration(uint32_t new_duration)
{
	// preserves original start time, just modifies duration and expire time
	m_duration = std::chrono::seconds(new_duration);
	m_expire_time = m_start_time + m_duration;

	LogDynamicZones("Updated zone [{}]:[{}] seconds remaining: [{}]",
		m_zone_id, m_instance_id, GetSecondsRemaining());

	if (zone && IsCurrentZoneDzInstance())
	{
		zone->SetInstanceTimer(GetSecondsRemaining());
	}
}

void DynamicZone::HandleWorldMessage(ServerPacket* pack)
{
	switch (pack->opcode)
	{
	case ServerOP_DzCharacterChange:
	{
		auto buf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
		Client* client = entity_list.GetClientByCharID(buf->character_id);
		if (client)
		{
			client->SetDzRemovalTimer(buf->remove); // instance kick timer
		}
		break;
	}
	case ServerOP_DzRemoveAllCharacters:
	{
		auto buf = reinterpret_cast<ServerDzCharacter_Struct*>(pack->pBuffer);
		if (buf->remove)
		{
			for (const auto& client_list_iter : entity_list.GetClientList())
			{
				if (client_list_iter.second)
				{
					client_list_iter.second->SetDzRemovalTimer(true);
				}
			}
		}
		break;
	}
	}
}
