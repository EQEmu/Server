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

ZoneStore::ZoneStore() = default;
ZoneStore::~ZoneStore() = default;

void ZoneStore::LoadZonesStore()
{
	zones = ZoneRepository::All();
}

uint32 ZoneStore::GetZoneID(const char *in_zone_name)
{
	if (in_zone_name == nullptr) {
		return 0;
	}

	std::string zone_name = in_zone_name;

	return GetZoneID(zone_name);
}

uint32 ZoneStore::GetZoneID(std::string zone_name)
{
	for (auto &z: zones) {
		if (z.short_name == zone_name) {
			return z.zoneidnumber;
		}
	}

	return 0;
}

const char *ZoneStore::GetZoneName(uint32 zone_id, bool error_unknown)
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

std::string ZoneStore::GetZoneName(uint32 zone_id)
{
	for (auto &z: zones) {
		if (z.zoneidnumber == zone_id) {
			return z.short_name;
		}
	}

	return std::string();
}
