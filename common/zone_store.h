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

#include "../common/repositories/zone_repository.h"
#include "../common/repositories/base/base_content_flags_repository.h"

#include <glm/vec4.hpp>

class ZoneStore {
public:
	ZoneStore();
	virtual ~ZoneStore();

	const std::vector<ZoneRepository::Zone> &GetZones() const;

	void LoadZones(Database &db);

	ZoneRepository::Zone *GetZone(uint32 zone_id, int version = 0);
	ZoneRepository::Zone *GetZone(const char *in_zone_name);
	#if !defined(SWIG) && !defined(SWIG_VERSION)
	ZoneRepository::Zone *GetZone(const std::string& in_zone_name);
	#endif
	uint32 GetZoneID(const char *in_zone_name);
	uint32 GetZoneID(std::string zone_name);
	std::string GetZoneName(uint32 zone_id);
	std::string GetZoneLongName(uint32 zone_id);
	#if !defined(SWIG) && !defined(SWIG_VERSION)
	const char *GetZoneName(uint32 zone_id, bool error_unknown = false);
	const char *GetZoneLongName(uint32 zone_id, bool error_unknown = false);
	#endif
	ZoneRepository::Zone *GetZoneWithFallback(uint32 zone_id, int version = 0);

	glm::vec4 GetZoneSafeCoordinates(uint32 zone_id, int version = 0);
	float GetZoneGraveyardID(uint32 zone_id, int version = 0);
	uint8 GetZoneMinimumLevel(uint32 zone_id, int version = 0);
	uint8 GetZoneMaximumLevel(uint32 zone_id, int version = 0);
	uint8 GetZoneMinimumStatus(uint32 zone_id, int version = 0);
	int GetZoneTimeZone(uint32 zone_id, int version = 0);
	int GetZoneMaximumPlayers(uint32 zone_id, int version = 0);
	uint32 GetZoneRuleSet(uint32 zone_id, int version = 0);
	const std::string GetZoneNote(uint32 zone_id, int version = 0);
	float GetZoneUnderworld(uint32 zone_id, int version = 0);
	float GetZoneMinimumClip(uint32 zone_id, int version = 0);
	float GetZoneMaximumClip(uint32 zone_id, int version = 0);
	float GetZoneFogMinimumClip(uint32 zone_id, uint8 slot = 0, int version = 0);
	float GetZoneFogMaximumClip(uint32 zone_id, uint8 slot = 0, int version = 0);
	uint8 GetZoneFogRed(uint32 zone_id, uint8 slot = 0, int version = 0);
	uint8 GetZoneFogGreen(uint32 zone_id, uint8 slot = 0, int version = 0);
	uint8 GetZoneFogBlue(uint32 zone_id, uint8 slot = 0, int version = 0);
	uint8 GetZoneSky(uint32 zone_id, int version = 0);
	uint8 GetZoneZType(uint32 zone_id, int version = 0);
	float GetZoneExperienceMultiplier(uint32 zone_id, int version = 0);
	float GetZoneWalkSpeed(uint32 zone_id, int version = 0);
	uint8 GetZoneTimeType(uint32 zone_id, int version = 0);
	float GetZoneFogDensity(uint32 zone_id, int version = 0);
	const std::string GetZoneFlagNeeded(uint32 zone_id, int version = 0);
	int8 GetZoneCanBind(uint32 zone_id, int version = 0);
	int8 GetZoneCanCombat(uint32 zone_id, int version = 0);
	int8 GetZoneCanLevitate(uint32 zone_id, int version = 0);
	int8 GetZoneCastOutdoor(uint32 zone_id, int version = 0);
	uint8 GetZoneHotzone(uint32 zone_id, int version = 0);
	uint8 GetZoneInstanceType(uint32 zone_id, int version = 0);
	uint64 GetZoneShutdownDelay(uint32 zone_id, int version = 0);
	int8 GetZonePEQZone(uint32 zone_id, int version = 0);
	int8 GetZoneExpansion(uint32 zone_id, int version = 0);
	int8 GetZoneBypassExpansionCheck(uint32 zone_id, int version = 0);
	uint8 GetZoneSuspendBuffs(uint32 zone_id, int version = 0);
	int GetZoneRainChance(uint32 zone_id, uint8 slot = 0, int version = 0);
	int GetZoneRainDuration(uint32 zone_id, uint8 slot = 0, int version = 0);
	int GetZoneSnowChance(uint32 zone_id, uint8 slot = 0, int version = 0);
	int GetZoneSnowDuration(uint32 zone_id, uint8 slot = 0, int version = 0);
	float GetZoneGravity(uint32 zone_id, int version = 0);
	int GetZoneType(uint32 zone_id, int version = 0);
	int8 GetZoneSkyLock(uint32 zone_id, int version = 0);
	int GetZoneFastRegenHP(uint32 zone_id, int version = 0);
	int GetZoneFastRegenMana(uint32 zone_id, int version = 0);
	int GetZoneFastRegenEndurance(uint32 zone_id, int version = 0);
	int GetZoneNPCMaximumAggroDistance(uint32 zone_id, int version = 0);
	uint32 GetZoneMaximumMovementUpdateRange(uint32 zone_id, int version = 0);
	int8 GetZoneMinimumExpansion(uint32 zone_id, int version = 0);
	int8 GetZoneMaximumExpansion(uint32 zone_id, int version = 0);
	const std::string GetZoneContentFlags(uint32 zone_id, int version = 0);
	const std::string GetZoneContentFlagsDisabled(uint32 zone_id, int version = 0);
	int GetZoneUnderworldTeleportIndex(uint32 zone_id, int version = 0);
	int GetZoneLavaDamage(uint32 zone_id, int version = 0);
	int GetZoneMinimumLavaDamage(uint32 zone_id, int version = 0);
	uint8 GetZoneIdleWhenEmpty(uint32 zone_id, int version = 0);
	uint32 GetZoneSecondsBeforeIdle(uint32 zone_id, int version = 0);

private:
	std::vector<ZoneRepository::Zone> m_zones;
};

extern ZoneStore zone_store;

/**
 * Global helpers
 */
inline uint32 ZoneID(const char *in_zone_name) { return zone_store.GetZoneID(in_zone_name); }
inline uint32 ZoneID(const std::string& zone_name) { return zone_store.GetZoneID(zone_name); }
#if !defined(SWIG) && !defined(SWIG_VERSION)
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

inline ZoneRepository::Zone *GetZone(uint32 zone_id, int version = 0) { return zone_store.GetZone(zone_id, version); };
inline ZoneRepository::Zone *GetZone(const char *in_zone_name) { return zone_store.GetZone(in_zone_name); };
inline ZoneRepository::Zone *GetZone(const char *in_zone_name, int version = 0)
{
	return zone_store.GetZone(
		ZoneID(
			in_zone_name
		), version
	);
};
#endif
inline ZoneRepository::Zone *GetZoneVersionWithFallback(uint32 zone_id, int version = 0)
{
	return zone_store.GetZoneWithFallback(
		zone_id,
		version
	);
};

#endif //EQEMU_ZONE_STORE_H
