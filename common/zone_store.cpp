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

#include "zone_store.h"
#include "../common/content/world_content_service.h"
#include "stacktrace/backward.hpp"

ZoneStore::ZoneStore() = default;
ZoneStore::~ZoneStore() = default;

// cache record of zones for fast successive retrieval
void ZoneStore::LoadZones(Database &db)
{
	m_zones = ZoneRepository::All(db);

	LogInfo("Loaded [{}] zones", m_zones.size());
}

/**
 * @param in_zone_name
 * @return
 */
uint32 ZoneStore::GetZoneID(const char *in_zone_name)
{
	if (in_zone_name == nullptr) {
		return 0;
	}

	std::string zone_name = Strings::ToLower(in_zone_name);

	return GetZoneID(zone_name);
}

/**
 * @param zone_name
 * @return
 */
uint32 ZoneStore::GetZoneID(std::string zone_name)
{
	for (auto &z: m_zones) {
		if (z.short_name == zone_name) {
			return z.zoneidnumber;
		}
	}

	LogInfo("Failed to get zone_name [{}]", zone_name);

	return 0;
}

/**
 * @param zone_id
 * @param error_unknown
 * @return
 */
const char *ZoneStore::GetZoneName(uint32 zone_id, bool error_unknown)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.short_name.c_str();
		}
	}

	if (error_unknown) {
		return "UNKNOWN";
	}

	return nullptr;
}

/**
 * @param zone_id
 * @param error_unknown
 * @return
 */
const char *ZoneStore::GetZoneLongName(uint32 zone_id, bool error_unknown)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.long_name.c_str();
		}
	}

	if (error_unknown) {
		return "UNKNOWN";
	}

	return nullptr;
}

/**
 * @param zone_id
 * @return
 */
std::string ZoneStore::GetZoneName(uint32 zone_id)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.short_name;
		}
	}

	LogInfo("Failed to get zone long name by zone_id [{}]", zone_id);

	return {};
}

/**
 * @param zone_id
 * @return
 */
std::string ZoneStore::GetZoneLongName(uint32 zone_id)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id) {
			return z.long_name;
		}
	}

	LogInfo("Failed to get zone long name by zone_id [{}]", zone_id);

	return {};
}

/**
 * @param zone_id
 * @param version
 * @return
 */
ZoneRepository::Zone *ZoneStore::GetZone(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return &z;
		}
	}

	LogInfo("Failed to get zone by zone_id [{}] version [{}]", zone_id, version);

	return nullptr;
}

/**
 * @param in_zone_name
 * @return
 */
ZoneRepository::Zone *ZoneStore::GetZone(const char *in_zone_name)
{
	for (auto &z: m_zones) {
		if (z.short_name == in_zone_name) {
			return &z;
		}
	}

	LogInfo("Failed to get zone by zone_name [{}]", in_zone_name);

	return nullptr;
}

ZoneRepository::Zone *ZoneStore::GetZone(const std::string& in_zone_name)
{
	for (auto &z: m_zones) {
		if (z.short_name == in_zone_name) {
			return &z;
		}
	}

	return nullptr;
}

const std::vector<ZoneRepository::Zone> &ZoneStore::GetZones() const
{
	return m_zones;
}

// gets zone data by using explicit version and falling back to version 0 if not found
ZoneRepository::Zone *ZoneStore::GetZoneWithFallback(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return &z;
		}
	}

	// second pass, default to version 0 if specific doesn't exist
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return &z;
		}
	}

	LogInfo("Failed to get zone by zone_id [{}] version [{}]", zone_id, version);

	return nullptr;
}
