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
#include "../common/repositories/base/base_content_flags_repository.h"

class ZoneStore {
public:
	ZoneStore();
	virtual ~ZoneStore();

	std::vector<ZoneRepository::Zone> zones;

	void LoadZones();

	ZoneRepository::Zone GetZone(uint32 zone_id, int version = 0);
	ZoneRepository::Zone GetZone(const char *in_zone_name);
	uint32 GetZoneID(const char *in_zone_name);
	uint32 GetZoneID(std::string zone_name);
	std::string GetZoneName(uint32 zone_id);
	std::string GetZoneLongName(uint32 zone_id);
	const char *GetZoneName(uint32 zone_id, bool error_unknown = false);
	const char *GetZoneLongName(uint32 zone_id, bool error_unknown = false);
};

extern ZoneStore zone_store;

/**
 * Global helpers
 */
inline uint32 ZoneID(const char *in_zone_name) { return zone_store.GetZoneID(in_zone_name); }
inline uint32 ZoneID(std::string zone_name) { return zone_store.GetZoneID(zone_name); }
inline const char *ZoneName(uint32 zone_id, bool error_unknown = false)
{
	return zone_store.GetZoneName(
		zone_id,
		error_unknown
	);
}
inline const char *ZoneLongName(uint32 zone_id, bool error_unknown = false)
{
	return zone_store.GetZoneLongName(
		zone_id,
		error_unknown
	);
}
inline ZoneRepository::Zone GetZone(uint32 zone_id, int version = 0) { return zone_store.GetZone(zone_id, version); };
inline ZoneRepository::Zone GetZone(const char *in_zone_name) { return zone_store.GetZone(in_zone_name); };

#endif //EQEMU_ZONE_STORE_H
