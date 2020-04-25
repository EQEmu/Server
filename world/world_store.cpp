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

#include "world_store.h"

WorldStore::WorldStore() = default;
WorldStore::~WorldStore() = default;

void WorldStore::LoadZones()
{
	zones = ZoneRepository::All();
}

uint32 WorldStore::GetZoneID(const char *in_zone_name)
{
	if (in_zone_name == nullptr) {
		return 0;
	}

	std::string zone_name = str_tolower(in_zone_name);

	return GetZoneID(zone_name);
}

uint32 WorldStore::GetZoneID(std::string zone_name)
{
	for (auto &z: zones) {
		if (z.short_name == zone_name) {
			return z.zoneidnumber;
		}
	}

	return 0;
}

/**
 * @param zone_id
 * @param error_unknown
 * @return
 */
const char *WorldStore::GetZoneName(uint32 zone_id, bool error_unknown)
{
	for (auto &z: zones) {
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
 * @return
 */
std::string WorldStore::GetZoneName(uint32 zone_id)
{
	for (auto &z: zones) {
		if (z.zoneidnumber == zone_id) {
			return z.short_name;
		}
	}

	return std::string();
}

/**
 * @param zone_id
 * @return
 */
std::string WorldStore::GetZoneLongName(uint32 zone_id)
{
	for (auto &z: zones) {
		if (z.zoneidnumber == zone_id) {
			return z.long_name;
		}
	}

	return std::string();
}

/**
 * @param zone_id
 * @param version
 * @return
 */
ZoneRepository::Zone WorldStore::GetZone(uint32 zone_id, int version)
{
	for (auto &z: zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z;
		}
	}

	return ZoneRepository::Zone();
}

/**
 * @param in_zone_name
 * @return
 */
ZoneRepository::Zone WorldStore::GetZone(const char *in_zone_name)
{
	for (auto &z: zones) {
		if (z.short_name == in_zone_name) {
			return z;
		}
	}

	return ZoneRepository::Zone();
}
