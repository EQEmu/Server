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

#ifndef EQEMU_ZONE_STORE_H
#define EQEMU_ZONE_STORE_H

#include "zonedb.h"
#include "../common/repositories/zone_repository.h"

class ZoneStore {
public:
	ZoneStore();
	virtual ~ZoneStore();

	std::vector<ZoneRepository::Zone> zones;

	void LoadZonesStore();

	uint32 GetZoneID(const char *in_zone_name);
	uint32 GetZoneID(std::string zone_name);
	std::string GetZoneName(uint32 zone_id);
	const char *GetZoneName(uint32 zone_id, bool error_unknown);
};

extern ZoneStore zone_store;

/**
 * Global helpers
 */
inline uint32 ZoneID(const char *in_zone_name) { return zone_store.GetZoneID(in_zone_name); }
inline uint32 ZoneID(std::string zone_name) { return zone_store.GetZoneID(zone_name); }
inline std::string ZoneName(uint32 zone_id) { return zone_store.GetZoneName(zone_id); }
inline const char *ZoneName(uint32 zone_id, bool error_unknown)
{
	return zone_store.GetZoneName(
		zone_id,
		error_unknown
	);
}


#endif //EQEMU_ZONE_STORE_H
