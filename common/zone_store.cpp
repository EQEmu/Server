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

#define DEFAULT_MINIMUM_CLIP 50.0f
#define DEFAULT_MAXIMUM_CLIP 175.0f
#define DEFAULT_ZONE_FAST_REGEN 180
#define DEFAULT_ZONE_GRAVITY 0.4f
#define DEFAULT_ZONE_MIN_MAX_EXPANSION -1
#define DEFAULT_ZONE_MAX_AGGRO_DISTANCE 600
#define DEFAULT_ZONE_MAX_MOVEMENT_UPDATE_RANGE 600
#define DEFAULT_ZONE_RUNSPEED 0.4f
#define DEFAULT_ZONE_SHUTDOWN_DELAY 3600000
#define DEFAULT_ZONE_TYPE 1
#define FOG_SLOT_ONE 1
#define FOG_SLOT_TWO 2
#define FOG_SLOT_THREE 3
#define FOG_SLOT_FOUR 4
#define RAIN_SLOT_ONE 1
#define RAIN_SLOT_TWO 2
#define RAIN_SLOT_THREE 3
#define RAIN_SLOT_FOUR 4
#define SNOW_SLOT_ONE 1
#define SNOW_SLOT_TWO 2
#define SNOW_SLOT_THREE 3
#define SNOW_SLOT_FOUR 4

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

glm::vec4 ZoneStore::GetZoneSafeCoordinates(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return glm::vec4(z.safe_x, z.safe_y, z.safe_z, z.safe_heading);
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return glm::vec4(z.safe_x, z.safe_y, z.safe_z, z.safe_heading);
		}
	}

	return glm::vec4(0.f);
}

float ZoneStore::GetZoneGraveyardID(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.graveyard_id;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.graveyard_id;
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneMinimumLevel(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.min_level;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.min_level;
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneMaximumLevel(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.max_level;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.max_level;
		}
	}

	return UINT8_MAX;
}

uint8 ZoneStore::GetZoneMinimumStatus(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.min_status;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.min_status;
		}
	}

	return 0;
}

int ZoneStore::GetZoneTimeZone(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.min_status;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.min_status;
		}
	}

	return 0;
}

int ZoneStore::GetZoneMaximumPlayers(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.maxclients;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.maxclients;
		}
	}

	return 0;
}

uint32 ZoneStore::GetZoneRuleSet(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.ruleset;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.ruleset;
		}
	}

	return 0;
}

const std::string& ZoneStore::GetZoneNote(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.note;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.note;
		}
	}

	const auto& note = std::string();
	return note;
}

float ZoneStore::GetZoneUnderworld(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.underworld;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.underworld;
		}
	}

	return 0.0f;
}

float ZoneStore::GetZoneMinimumClip(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.minclip;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.minclip;
		}
	}

	return DEFAULT_MINIMUM_CLIP;
}

float ZoneStore::GetZoneMaximumClip(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.maxclip;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.maxclip;
		}
	}

	return DEFAULT_MAXIMUM_CLIP;
}

float ZoneStore::GetZoneFogMinimumClip(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_minclip1;
				case FOG_SLOT_TWO:
					return z.fog_minclip2;
				case FOG_SLOT_THREE:
					return z.fog_minclip3;
				case FOG_SLOT_FOUR:
					return z.fog_minclip4;
				default:
					return z.fog_minclip;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_minclip1;
				case FOG_SLOT_TWO:
					return z.fog_minclip2;
				case FOG_SLOT_THREE:
					return z.fog_minclip3;
				case FOG_SLOT_FOUR:
					return z.fog_minclip4;
				default:
					return z.fog_minclip;
			}
		}
	}

	return DEFAULT_MINIMUM_CLIP;
}

float ZoneStore::GetZoneFogMaximumClip(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_maxclip1;
				case FOG_SLOT_TWO:
					return z.fog_maxclip2;
				case FOG_SLOT_THREE:
					return z.fog_maxclip3;
				case FOG_SLOT_FOUR:
					return z.fog_maxclip4;
				default:
					return z.fog_maxclip;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_maxclip1;
				case FOG_SLOT_TWO:
					return z.fog_maxclip2;
				case FOG_SLOT_THREE:
					return z.fog_maxclip3;
				case FOG_SLOT_FOUR:
					return z.fog_maxclip4;
				default:
					return z.fog_maxclip;
			}
		}
	}

	return DEFAULT_MAXIMUM_CLIP;
}

uint8 ZoneStore::GetZoneFogRed(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_red1;
				case FOG_SLOT_TWO:
					return z.fog_red2;
				case FOG_SLOT_THREE:
					return z.fog_red3;
				case FOG_SLOT_FOUR:
					return z.fog_red4;
				default:
					return z.fog_red;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_red1;
				case FOG_SLOT_TWO:
					return z.fog_red2;
				case FOG_SLOT_THREE:
					return z.fog_red3;
				case FOG_SLOT_FOUR:
					return z.fog_red4;
				default:
					return z.fog_red;
			}
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneFogGreen(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_green1;
				case FOG_SLOT_TWO:
					return z.fog_green2;
				case FOG_SLOT_THREE:
					return z.fog_green3;
				case FOG_SLOT_FOUR:
					return z.fog_green4;
				default:
					return z.fog_green;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_green1;
				case FOG_SLOT_TWO:
					return z.fog_green2;
				case FOG_SLOT_THREE:
					return z.fog_green3;
				case FOG_SLOT_FOUR:
					return z.fog_green4;
				default:
					return z.fog_green;
			}
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneFogBlue(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_blue1;
				case FOG_SLOT_TWO:
					return z.fog_blue2;
				case FOG_SLOT_THREE:
					return z.fog_blue3;
				case FOG_SLOT_FOUR:
					return z.fog_blue4;
				default:
					return z.fog_blue;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case FOG_SLOT_ONE:
					return z.fog_blue1;
				case FOG_SLOT_TWO:
					return z.fog_blue2;
				case FOG_SLOT_THREE:
					return z.fog_blue3;
				case FOG_SLOT_FOUR:
					return z.fog_blue4;
				default:
					return z.fog_blue;
			}
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneSky(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.sky;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.sky;
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneZType(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.ztype;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.ztype;
		}
	}

	return 0;
}

float ZoneStore::GetZoneExperienceMultiplier(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.zone_exp_multiplier;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.zone_exp_multiplier;
		}
	}

	return 1.0f;
}

float ZoneStore::GetZoneWalkSpeed(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.walkspeed;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.walkspeed;
		}
	}

	return DEFAULT_ZONE_RUNSPEED;
}

uint8 ZoneStore::GetZoneTimeType(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.time_type;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.time_type;
		}
	}

	return 0;
}

float ZoneStore::GetZoneFogDensity(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.fog_density;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.fog_density;
		}
	}

	return 0.0f;
}

const std::string& ZoneStore::GetZoneFlagNeeded(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.flag_needed;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.flag_needed;
		}
	}

	const auto& flag_needed = std::string();
	return flag_needed;
}

int8 ZoneStore::GetZoneCanBind(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.canbind;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.canbind;
		}
	}

	return 0;
}

int8 ZoneStore::GetZoneCanCombat(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.cancombat;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.cancombat;
		}
	}

	return 0;
}

int8 ZoneStore::GetZoneCanLevitate(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.canlevitate;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.canlevitate;
		}
	}

	return 0;
}

int8 ZoneStore::GetZoneCastOutdoor(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.castoutdoor;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.castoutdoor;
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneHotzone(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.hotzone;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.hotzone;
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneInstanceType(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.insttype;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.insttype;
		}
	}

	return 0;
}

uint64 ZoneStore::GetZoneShutdownDelay(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.shutdowndelay;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.shutdowndelay;
		}
	}

	return DEFAULT_ZONE_SHUTDOWN_DELAY;
}

int8 ZoneStore::GetZonePEQZone(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.peqzone;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.peqzone;
		}
	}

	return 0;
}

int8 ZoneStore::GetZoneExpansion(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.expansion;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.expansion;
		}
	}

	return 0;
}

int8 ZoneStore::GetZoneBypassExpansionCheck(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.bypass_expansion_check;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.bypass_expansion_check;
		}
	}

	return 0;
}

uint8 ZoneStore::GetZoneSuspendBuffs(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.suspendbuffs;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.suspendbuffs;
		}
	}

	return 0;
}

int ZoneStore::GetZoneRainChance(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case RAIN_SLOT_TWO:
					return z.rain_chance2;
				case RAIN_SLOT_THREE:
					return z.rain_chance3;
				case RAIN_SLOT_FOUR:
					return z.rain_chance4;
				case RAIN_SLOT_ONE:
				default:
					return z.rain_chance1;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case RAIN_SLOT_TWO:
					return z.rain_chance2;
				case RAIN_SLOT_THREE:
					return z.rain_chance3;
				case RAIN_SLOT_FOUR:
					return z.rain_chance4;
				case RAIN_SLOT_ONE:
				default:
					return z.rain_chance1;
			}
		}
	}

	return 0;
}

int ZoneStore::GetZoneRainDuration(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case RAIN_SLOT_TWO:
					return z.rain_duration2;
				case RAIN_SLOT_THREE:
					return z.rain_duration3;
				case RAIN_SLOT_FOUR:
					return z.rain_duration4;
				case RAIN_SLOT_ONE:
				default:
					return z.rain_duration1;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case RAIN_SLOT_TWO:
					return z.rain_duration2;
				case RAIN_SLOT_THREE:
					return z.rain_duration3;
				case RAIN_SLOT_FOUR:
					return z.rain_duration4;
				case RAIN_SLOT_ONE:
				default:
					return z.rain_duration1;
			}
		}
	}

	return 0;
}

int ZoneStore::GetZoneSnowChance(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case SNOW_SLOT_TWO:
					return z.snow_chance2;
				case SNOW_SLOT_THREE:
					return z.snow_chance3;
				case SNOW_SLOT_FOUR:
					return z.snow_chance4;
				case SNOW_SLOT_ONE:
				default:
					return z.snow_chance1;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case SNOW_SLOT_TWO:
					return z.snow_chance2;
				case SNOW_SLOT_THREE:
					return z.snow_chance3;
				case SNOW_SLOT_FOUR:
					return z.snow_chance4;
				case SNOW_SLOT_ONE:
				default:
					return z.snow_chance1;
			}
		}
	}

	return 0;
}

int ZoneStore::GetZoneSnowDuration(uint32 zone_id, uint8 slot, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			switch (slot) {
				case SNOW_SLOT_TWO:
					return z.snow_duration2;
				case SNOW_SLOT_THREE:
					return z.snow_duration3;
				case SNOW_SLOT_FOUR:
					return z.snow_duration4;
				case SNOW_SLOT_ONE:
				default:
					return z.snow_duration1;
			}
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			switch (slot) {
				case SNOW_SLOT_TWO:
					return z.snow_duration2;
				case SNOW_SLOT_THREE:
					return z.snow_duration3;
				case SNOW_SLOT_FOUR:
					return z.snow_duration4;
				case SNOW_SLOT_ONE:
				default:
					return z.snow_duration1;
			}
		}
	}

	return 0;
}

float ZoneStore::GetZoneGravity(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.gravity;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.gravity;
		}
	}

	return DEFAULT_ZONE_GRAVITY;
}

int ZoneStore::GetZoneType(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.type;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.type;
		}
	}

	return DEFAULT_ZONE_TYPE;
}

int8 ZoneStore::GetZoneSkyLock(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.skylock;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.skylock;
		}
	}

	return 0;
}

int ZoneStore::GetZoneFastRegenHP(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.fast_regen_hp;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.fast_regen_hp;
		}
	}

	return DEFAULT_ZONE_FAST_REGEN;
}

int ZoneStore::GetZoneFastRegenMana(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.fast_regen_mana;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.fast_regen_mana;
		}
	}

	return DEFAULT_ZONE_FAST_REGEN;
}

int ZoneStore::GetZoneFastRegenEndurance(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.fast_regen_endurance;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.fast_regen_endurance;
		}
	}

	return DEFAULT_ZONE_FAST_REGEN;
}

int ZoneStore::GetZoneNPCMaximumAggroDistance(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.npc_max_aggro_dist;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.npc_max_aggro_dist;
		}
	}

	return DEFAULT_ZONE_MAX_AGGRO_DISTANCE;
}

uint32 ZoneStore::GetZoneMaximumMovementUpdateRange(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.max_movement_update_range;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.max_movement_update_range;
		}
	}

	return DEFAULT_ZONE_MAX_MOVEMENT_UPDATE_RANGE;
}

int8 ZoneStore::GetZoneMinimumExpansion(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.min_expansion;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.min_expansion;
		}
	}

	return DEFAULT_ZONE_MIN_MAX_EXPANSION;
}

int8 ZoneStore::GetZoneMaximumExpansion(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.max_expansion;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.max_expansion;
		}
	}

	return DEFAULT_ZONE_MIN_MAX_EXPANSION;
}

const std::string& ZoneStore::GetZoneContentFlags(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.content_flags;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.content_flags;
		}
	}

	const auto& content_flags = std::string();
	return content_flags;
}

const std::string& ZoneStore::GetZoneContentFlagsDisabled(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.content_flags_disabled;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.content_flags_disabled;
		}
	}

	const auto& content_flags_disabled = std::string();
	return content_flags_disabled;
}

int ZoneStore::GetZoneUnderworldTeleportIndex(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.underworld_teleport_index;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.underworld_teleport_index;
		}
	}

	return 0;
}

int ZoneStore::GetZoneLavaDamage(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.lava_damage;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.lava_damage;
		}
	}

	return 0;
}

int ZoneStore::GetZoneMinimumLavaDamage(uint32 zone_id, int version)
{
	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == version) {
			return z.min_lava_damage;
		}
	}

	for (auto &z: m_zones) {
		if (z.zoneidnumber == zone_id && z.version == 0) {
			return z.min_lava_damage;
		}
	}

	return 0;
}
