#pragma once

#ifdef LUA_EQEMU

#include "lua_client.h"
#include "lua_ptr.h"
#include "common.h"
#include "zone.h"

class Lua_Zone;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_zone();

class Lua_Zone : public Lua_Ptr<Zone>
{
	typedef Zone NativeType;
public:
	Lua_Zone() : Lua_Ptr(nullptr) { }
	Lua_Zone(Zone* d) : Lua_Ptr(d) { }
	virtual ~Lua_Zone() { }

	operator Zone*() {
		return reinterpret_cast<Zone*>(GetLuaPtrData());
	}

	bool BuffTimersSuspended();
	bool BypassesExpansionCheck();
	bool CanBind();
	bool CanCastOutdoor();
	bool CanDoCombat();
	bool CanLevitate();
	void ClearSpawnTimers();
	void Depop();
	void Depop(bool start_spawn_timers);
	void Despawn(uint32 spawngroup_id);
	float GetAAEXPModifier(Lua_Client c);
	float GetAAEXPModifierByCharacterID(uint32 character_id);
	std::string GetContentFlags();
	std::string GetContentFlagsDisabled();
	float GetExperienceMultiplier();
	int8 GetExpansion();
	float GetEXPModifier(Lua_Client c);
	float GetEXPModifierByCharacterID(uint32 character_id);
	int GetFastRegenEndurance();
	int GetFastRegenHP();
	int GetFastRegenMana();
	std::string GetFileName();
	std::string GetFlagNeeded();
	uint8 GetFogBlue();
	uint8 GetFogBlue(uint8 slot);
	float GetFogDensity();
	uint8 GetFogGreen();
	uint8 GetFogGreen(uint8 slot);
	float GetFogMaximumClip();
	float GetFogMaximumClip(uint8 slot);
	float GetFogMinimumClip();
	float GetFogMinimumClip(uint8 slot);
	uint8 GetFogRed();
	uint8 GetFogRed(uint8 slot);
	float GetGraveyardHeading();
	uint32 GetGraveyardID();
	float GetGraveyardX();
	float GetGraveyardY();
	float GetGraveyardZ();
	uint32 GetGraveyardZoneID();
	float GetGravity();
	uint32 GetInstanceID();
	uint8 GetInstanceType();
	uint16 GetInstanceVersion();
	uint32 GetInstanceTimeRemaining();
	int GetLavaDamage();
	std::string GetLongName();
	float GetMaximumClip();
	int8 GetMaximumExpansion();
	uint8 GetMaximumLevel();
	uint32 GetMaxClients();
	float GetMinimumClip();
	int8 GetMinimumExpansion();
	uint8 GetMinimumLevel();
	int GetMinimumLavaDamage();
	uint8 GetMinimumStatus();
	std::string GetNote();
	int GetNPCMaximumAggroDistance();
	int8 GetPEQZone();
	int GetRainChance();
	int GetRainChance(uint8 slot);
	int GetRainDuration();
	int GetRainDuration(uint8 slot);
	uint32 GetRuleSet();
	float GetSafeHeading();
	float GetSafeX();
	float GetSafeY();
	float GetSafeZ();
	std::string GetShortName();
	uint32 GetSecondsBeforeIdle();
	uint64 GetShutdownDelay();
	uint8 GetSky();
	int8 GetSkyLock();
	int GetSnowChance();
	int GetSnowChance(uint8 slot);
	int GetSnowDuration();
	int GetSnowDuration(uint8 slot);
	uint8 GetTimeType();
	int GetTimeZone();
	std::string GetZoneDescription();
	uint32 GetZoneID();
	uint8 GetZoneType();
	float GetUnderworld();
	int GetUnderworldTeleportIndex();
	float GetWalkSpeed();
	uint8 GetZoneZType();
	int GetZoneTotalBlockedSpells();
	bool HasGraveyard();
	bool HasMap();
	bool HasWaterMap();
	bool HasWeather();
	bool IsCity();
	bool IsHotzone();
	bool IsInstancePersistent();
	bool IsIdleWhenEmpty();
	bool IsPVPZone();
	bool IsRaining();
	bool IsSnowing();
	bool IsSpecialBindLocation(float x, float y, float z, float heading);
	bool IsSpellBlocked(uint32 spell_id, float x, float y, float z);
	bool IsStaticZone();
	bool IsUCSServerAvailable();
	bool IsWaterZone(float z);
	void Repop();
	void Repop(bool is_forced);
	void SetAAEXPModifier(Lua_Client c, float aa_modifier);
	void SetAAEXPModifierByCharacterID(uint32 character_id, float aa_modifier);
	void SetEXPModifier(Lua_Client c, float exp_modifier);
	void SetEXPModifierByCharacterID(uint32 character_id, float exp_modifier);
	void SetInstanceTimer(uint32 new_duration);
	void SetInstanceTimeRemaining(uint32 time_remaining);
	void SetIsHotzone(bool is_hotzone);
	void ShowZoneGlobalLoot(Lua_Client c);

};

#endif
