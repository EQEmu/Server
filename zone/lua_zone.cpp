#include "../common/features.h"
#include "zone.h"


#include <luabind/luabind.hpp>
#include "../common/global_define.h"
#include "embperl.h"
#include "lua_zone.h"

bool Lua_Zone::BuffTimersSuspended()
{
	Lua_Safe_Call_Bool();
	return self->BuffTimersSuspended();
}

bool Lua_Zone::BypassesExpansionCheck()
{
	Lua_Safe_Call_Bool();
	return zone_store.GetZoneBypassExpansionCheck(self->GetZoneID(), self->GetInstanceVersion());
}

bool Lua_Zone::CanBind()
{
	Lua_Safe_Call_Bool();
	return self->CanBind();
}

bool Lua_Zone::CanCastOutdoor()
{
	Lua_Safe_Call_Bool();
	return self->CanCastOutdoor();
}

bool Lua_Zone::CanDoCombat()
{
	Lua_Safe_Call_Bool();
	return self->CanDoCombat();
}

bool Lua_Zone::CanLevitate()
{
	Lua_Safe_Call_Bool();
	return self->CanLevitate();
}

void Lua_Zone::ClearSpawnTimers()
{
	Lua_Safe_Call_Void();
	self->ClearSpawnTimers();
}

void Lua_Zone::Depop()
{
	Lua_Safe_Call_Void();
	self->Depop();
}

void Lua_Zone::Depop(bool start_spawn_timers)
{
	Lua_Safe_Call_Void();
	self->Depop(start_spawn_timers);
}

void Lua_Zone::Despawn(uint32 spawngroup_id)
{
	Lua_Safe_Call_Void();
	self->Despawn(spawngroup_id);
}

float Lua_Zone::GetAAEXPModifier(Lua_Client c)
{
	Lua_Safe_Call_Real();
	return self->GetAAEXPModifier(c);
}

float Lua_Zone::GetAAEXPModifierByCharacterID(uint32 character_id)
{
	Lua_Safe_Call_Real();
	return self->GetAAEXPModifierByCharacterID(character_id);
}

std::string Lua_Zone::GetContentFlags()
{
	Lua_Safe_Call_String();
	return zone_store.GetZoneContentFlags(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Lua_Zone::GetContentFlagsDisabled()
{
	Lua_Safe_Call_String();
	return zone_store.GetZoneContentFlagsDisabled(self->GetZoneID(), self->GetInstanceVersion());
}

float Lua_Zone::GetExperienceMultiplier()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneExperienceMultiplier(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Lua_Zone::GetExpansion()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneExpansion(self->GetZoneID(), self->GetInstanceVersion());
}

float Lua_Zone::GetEXPModifier(Lua_Client c)
{
	Lua_Safe_Call_Real();
	return self->GetEXPModifier(c);
}

float Lua_Zone::GetEXPModifierByCharacterID(uint32 character_id)
{
	Lua_Safe_Call_Real();
	return self->GetEXPModifierByCharacterID(character_id);
}

int Lua_Zone::GetFastRegenEndurance()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFastRegenEndurance(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetFastRegenHP()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFastRegenHP(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetFastRegenMana()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFastRegenMana(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Lua_Zone::GetFileName()
{
	Lua_Safe_Call_String();
	return self->GetFileName();
}

std::string Lua_Zone::GetFlagNeeded()
{
	Lua_Safe_Call_String();
	return zone_store.GetZoneFlagNeeded(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Lua_Zone::GetFogBlue()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFogBlue(self->GetZoneID(), 0, self->GetInstanceVersion());
}

uint8 Lua_Zone::GetFogBlue(uint8 slot)
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFogBlue(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Lua_Zone::GetFogDensity()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneFogDensity(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Lua_Zone::GetFogGreen()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFogGreen(self->GetZoneID(), 0, self->GetInstanceVersion());
}

uint8 Lua_Zone::GetFogGreen(uint8 slot)
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFogGreen(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Lua_Zone::GetFogMaximumClip()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneFogMaximumClip(self->GetZoneID(), 0, self->GetInstanceVersion());
}

float Lua_Zone::GetFogMaximumClip(uint8 slot)
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneFogMaximumClip(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Lua_Zone::GetFogMinimumClip()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneFogMinimumClip(self->GetZoneID(), 0, self->GetInstanceVersion());
}

float Lua_Zone::GetFogMinimumClip(uint8 slot)
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneFogMinimumClip(self->GetZoneID(), slot, self->GetInstanceVersion());
}

uint8 Lua_Zone::GetFogRed()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFogRed(self->GetZoneID(), 0, self->GetInstanceVersion());
}

uint8 Lua_Zone::GetFogRed(uint8 slot)
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneFogRed(self->GetZoneID(), slot, self->GetInstanceVersion());
}

float Lua_Zone::GetGraveyardHeading()
{
	Lua_Safe_Call_Real();
	return self->GetGraveyardPoint().w;
}

uint32 Lua_Zone::GetGraveyardID()
{
	Lua_Safe_Call_Int();
	return self->graveyard_id();
}

float Lua_Zone::GetGraveyardX()
{
	Lua_Safe_Call_Real();
	return self->GetGraveyardPoint().x;
}

float Lua_Zone::GetGraveyardY()
{
	Lua_Safe_Call_Real();
	return self->GetGraveyardPoint().y;
}

float Lua_Zone::GetGraveyardZ()
{
	Lua_Safe_Call_Real();
	return self->GetGraveyardPoint().z;
}

uint32 Lua_Zone::GetGraveyardZoneID()
{
	Lua_Safe_Call_Int();
	return self->graveyard_zoneid();
}

float Lua_Zone::GetGravity()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneGravity(self->GetZoneID(), self->GetInstanceVersion());
}

uint32 Lua_Zone::GetInstanceID()
{
	Lua_Safe_Call_Int();
	return self->GetInstanceID();
}

uint8 Lua_Zone::GetInstanceType()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneInstanceType(self->GetZoneID(), self->GetInstanceVersion());
}

uint16 Lua_Zone::GetInstanceVersion()
{
	Lua_Safe_Call_Int();
	return self->GetInstanceVersion();
}

uint32 Lua_Zone::GetInstanceTimeRemaining()
{
	Lua_Safe_Call_Int();
	return self->GetInstanceTimeRemaining();
}

int Lua_Zone::GetLavaDamage()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneLavaDamage(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Lua_Zone::GetLongName()
{
	Lua_Safe_Call_String();
	return self->GetLongName();
}

float Lua_Zone::GetMaximumClip()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneMaximumClip(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Lua_Zone::GetMaximumExpansion()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneMaximumExpansion(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Lua_Zone::GetMaximumLevel()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneMaximumLevel(self->GetZoneID(), self->GetInstanceVersion());
}

uint32 Lua_Zone::GetMaxClients()
{
	Lua_Safe_Call_Int();
	return self->GetMaxClients();
}

float Lua_Zone::GetMinimumClip()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneMinimumClip(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Lua_Zone::GetMinimumExpansion()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneMinimumExpansion(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Lua_Zone::GetMinimumLevel()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneMinimumLevel(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetMinimumLavaDamage()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneMinimumLavaDamage(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Lua_Zone::GetMinimumStatus()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneMinimumStatus(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Lua_Zone::GetNote()
{
	Lua_Safe_Call_String();
	return zone_store.GetZoneNote(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetNPCMaximumAggroDistance()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneNPCMaximumAggroDistance(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Lua_Zone::GetPEQZone()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZonePEQZone(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetRainChance()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneRainChance(self->GetZoneID(), 0, self->GetInstanceVersion());
}

int Lua_Zone::GetRainChance(uint8 slot)
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneRainChance(self->GetZoneID(), slot, self->GetInstanceVersion());
}

int Lua_Zone::GetRainDuration()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneRainDuration(self->GetZoneID(), 0, self->GetInstanceVersion());
}

int Lua_Zone::GetRainDuration(uint8 slot)
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneRainDuration(self->GetZoneID(), slot, self->GetInstanceVersion());
}

uint32 Lua_Zone::GetRuleSet()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneRuleSet(self->GetZoneID(), self->GetInstanceVersion());
}

float Lua_Zone::GetSafeHeading()
{
	Lua_Safe_Call_Real();
	return self->GetSafePoint().w;
}

float Lua_Zone::GetSafeX()
{
	Lua_Safe_Call_Real();
	return self->GetSafePoint().x;
}

float Lua_Zone::GetSafeY()
{
	Lua_Safe_Call_Real();
	return self->GetSafePoint().y;
}

float Lua_Zone::GetSafeZ()
{
	Lua_Safe_Call_Real();
	return self->GetSafePoint().z;
}

std::string Lua_Zone::GetShortName()
{
	Lua_Safe_Call_String();
	return self->GetShortName();
}

uint32 Lua_Zone::GetSecondsBeforeIdle()
{
	Lua_Safe_Call_Int();
	return self->GetSecondsBeforeIdle();
}

uint64 Lua_Zone::GetShutdownDelay()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneShutdownDelay(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Lua_Zone::GetSky()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneSky(self->GetZoneID(), self->GetInstanceVersion());
}

int8 Lua_Zone::GetSkyLock()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneSkyLock(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetSnowChance()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneSnowChance(self->GetZoneID(), 0, self->GetInstanceVersion());
}

int Lua_Zone::GetSnowChance(uint8 slot)
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneSnowChance(self->GetZoneID(), slot, self->GetInstanceVersion());
}

int Lua_Zone::GetSnowDuration()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneSnowDuration(self->GetZoneID(), 0, self->GetInstanceVersion());
}

int Lua_Zone::GetSnowDuration(uint8 slot)
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneSnowDuration(self->GetZoneID(), slot, self->GetInstanceVersion());
}

uint8 Lua_Zone::GetTimeType()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneTimeType(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetTimeZone()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneTimeZone(self->GetZoneID(), self->GetInstanceVersion());
}

std::string Lua_Zone::GetZoneDescription()
{
	Lua_Safe_Call_String();
	return self->GetZoneDescription();
}

uint32 Lua_Zone::GetZoneID()
{
	Lua_Safe_Call_Int();
	return self->GetZoneID();
}

uint8 Lua_Zone::GetZoneType()
{
	Lua_Safe_Call_Int();
	return self->GetZoneType();
}

float Lua_Zone::GetUnderworld()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneUnderworld(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetUnderworldTeleportIndex()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneUnderworldTeleportIndex(self->GetZoneID(), self->GetInstanceVersion());
}

float Lua_Zone::GetWalkSpeed()
{
	Lua_Safe_Call_Real();
	return zone_store.GetZoneWalkSpeed(self->GetZoneID(), self->GetInstanceVersion());
}

uint8 Lua_Zone::GetZoneZType()
{
	Lua_Safe_Call_Int();
	return zone_store.GetZoneZType(self->GetZoneID(), self->GetInstanceVersion());
}

int Lua_Zone::GetZoneTotalBlockedSpells()
{
	Lua_Safe_Call_Int();
	return self->GetZoneTotalBlockedSpells();
}

bool Lua_Zone::HasGraveyard()
{
	Lua_Safe_Call_Bool();
	return self->HasGraveyard();
}

bool Lua_Zone::HasMap()
{
	Lua_Safe_Call_Bool();
	return self->HasMap();
}

bool Lua_Zone::HasWaterMap()
{
	Lua_Safe_Call_Bool();
	return self->HasWaterMap();
}

bool Lua_Zone::HasWeather()
{
	Lua_Safe_Call_Bool();
	return self->HasWeather();
}

bool Lua_Zone::IsCity()
{
	Lua_Safe_Call_Bool();
	return self->IsCity();
}

bool Lua_Zone::IsHotzone()
{
	Lua_Safe_Call_Bool();
	return self->IsHotzone();
}

bool Lua_Zone::IsInstancePersistent()
{
	Lua_Safe_Call_Bool();
	return self->IsInstancePersistent();
}

bool Lua_Zone::IsIdleWhenEmpty()
{
	Lua_Safe_Call_Bool();
	return self->IsIdleWhenEmpty();
}

bool Lua_Zone::IsPVPZone()
{
	Lua_Safe_Call_Bool();
	return self->IsPVPZone();
}

bool Lua_Zone::IsRaining()
{
	Lua_Safe_Call_Bool();
	return self->IsRaining();
}

bool Lua_Zone::IsSnowing()
{
	Lua_Safe_Call_Bool();
	return self->IsSnowing();
}

bool Lua_Zone::IsSpecialBindLocation(float x, float y, float z, float heading)
{
	Lua_Safe_Call_Bool();
	return self->IsSpecialBindLocation(glm::vec4(x, y, z, heading));
}

bool Lua_Zone::IsSpellBlocked(uint32 spell_id, float x, float y, float z)
{
	Lua_Safe_Call_Bool();
	return self->IsSpellBlocked(spell_id, glm::vec3(x, y, z));
}

bool Lua_Zone::IsStaticZone()
{
	Lua_Safe_Call_Bool();
	return self->IsStaticZone();
}

bool Lua_Zone::IsUCSServerAvailable()
{
	Lua_Safe_Call_Bool();
	return self->IsUCSServerAvailable();
}

bool Lua_Zone::IsWaterZone(float z)
{
	Lua_Safe_Call_Bool();
	return self->IsWaterZone(z);
}

void Lua_Zone::Repop()
{
	Lua_Safe_Call_Void();
	self->Repop();
}

void Lua_Zone::Repop(bool is_forced)
{
	Lua_Safe_Call_Void();
	self->Repop(is_forced);
}

void Lua_Zone::SetAAEXPModifier(Lua_Client c, float aa_modifier)
{
	Lua_Safe_Call_Void();
	self->SetAAEXPModifier(c, aa_modifier);
}

void Lua_Zone::SetAAEXPModifierByCharacterID(uint32 character_id, float aa_modifier)
{
	Lua_Safe_Call_Void();
	self->SetAAEXPModifierByCharacterID(character_id, aa_modifier);
}

void Lua_Zone::SetEXPModifier(Lua_Client c, float exp_modifier)
{
	Lua_Safe_Call_Void();
	self->SetAAEXPModifier(c, exp_modifier);
}

void Lua_Zone::SetEXPModifierByCharacterID(uint32 character_id, float exp_modifier)
{
	Lua_Safe_Call_Void();
	self->SetAAEXPModifierByCharacterID(character_id, exp_modifier);
}

void Lua_Zone::SetInstanceTimer(uint32 new_duration)
{
	Lua_Safe_Call_Void();
	self->SetInstanceTimer(new_duration);
}

void Lua_Zone::SetInstanceTimeRemaining(uint32 time_remaining)
{
	Lua_Safe_Call_Void();
	self->SetInstanceTimeRemaining(time_remaining);
}

void Lua_Zone::SetIsHotzone(bool is_hotzone)
{
	Lua_Safe_Call_Void();
	self->SetIsHotzone(is_hotzone);
}

void Lua_Zone::ShowZoneGlobalLoot(Lua_Client c)
{
	Lua_Safe_Call_Void();
	self->ShowZoneGlobalLoot(c);
}

luabind::scope lua_register_zone() {
	return luabind::class_<Lua_Zone>("Zone")
	.def(luabind::constructor<>())
	.def("BuffTimersSuspended", &Lua_Zone::BuffTimersSuspended)
	.def("BypassesExpansionCheck", &Lua_Zone::BypassesExpansionCheck)
	.def("CanBind", &Lua_Zone::CanBind)
	.def("CanCastOutdoor", &Lua_Zone::CanCastOutdoor)
	.def("CanDoCombat", &Lua_Zone::CanDoCombat)
	.def("CanLevitate", &Lua_Zone::CanLevitate)
	.def("ClearSpawnTimers", &Lua_Zone::ClearSpawnTimers)
	.def("Depop", (void(Lua_Zone::*)(void))&Lua_Zone::Depop)
	.def("Depop", (void(Lua_Zone::*)(bool))&Lua_Zone::Depop)
	.def("Despawn", &Lua_Zone::Despawn)
	.def("GetAAEXPModifier", &Lua_Zone::GetAAEXPModifier)
	.def("GetAAEXPModifierByCharacterID", &Lua_Zone::GetAAEXPModifierByCharacterID)
	.def("GetContentFlags", &Lua_Zone::GetContentFlags)
	.def("GetContentFlagsDisabled", &Lua_Zone::GetContentFlagsDisabled)
	.def("GetExperienceMultiplier", &Lua_Zone::GetExperienceMultiplier)
	.def("GetExpansion", &Lua_Zone::GetExpansion)
	.def("GetEXPModifier", &Lua_Zone::GetEXPModifier)
	.def("GetEXPModifierByCharacterID", &Lua_Zone::GetEXPModifierByCharacterID)
	.def("GetFastRegenEndurance", &Lua_Zone::GetFastRegenEndurance)
	.def("GetFastRegenHP", &Lua_Zone::GetFastRegenHP)
	.def("GetFastRegenMana", &Lua_Zone::GetFastRegenMana)
	.def("GetFileName", &Lua_Zone::GetFileName)
	.def("GetFlagNeeded", &Lua_Zone::GetFlagNeeded)
	.def("GetFogBlue", (uint8(Lua_Zone::*)(void))&Lua_Zone::GetFogBlue)
	.def("GetFogBlue", (uint8(Lua_Zone::*)(uint8))&Lua_Zone::GetFogBlue)
	.def("GetFogDensity", &Lua_Zone::GetFogDensity)
	.def("GetFogGreen", (uint8(Lua_Zone::*)(void))&Lua_Zone::GetFogGreen)
	.def("GetFogGreen", (uint8(Lua_Zone::*)(uint8))&Lua_Zone::GetFogGreen)
	.def("GetFogMaximumClip", (float(Lua_Zone::*)(void))&Lua_Zone::GetFogMaximumClip)
	.def("GetFogMaximumClip", (float(Lua_Zone::*)(uint8))&Lua_Zone::GetFogMaximumClip)
	.def("GetFogMinimumClip", (float(Lua_Zone::*)(void))&Lua_Zone::GetFogMinimumClip)
	.def("GetFogMinimumClip", (float(Lua_Zone::*)(uint8))&Lua_Zone::GetFogMinimumClip)
	.def("GetFogRed", (uint8(Lua_Zone::*)(void))&Lua_Zone::GetFogRed)
	.def("GetFogRed", (uint8(Lua_Zone::*)(uint8))&Lua_Zone::GetFogRed)
	.def("GetGraveyardHeading", &Lua_Zone::GetGraveyardHeading)
	.def("GetGraveyardID", &Lua_Zone::GetGraveyardID)
	.def("GetGraveyardX", &Lua_Zone::GetGraveyardX)
	.def("GetGraveyardY", &Lua_Zone::GetGraveyardY)
	.def("GetGraveyardZ", &Lua_Zone::GetGraveyardZ)
	.def("GetGraveyardZoneID", &Lua_Zone::GetGraveyardZoneID)
	.def("GetGravity", &Lua_Zone::GetGravity)
	.def("GetInstanceID", &Lua_Zone::GetInstanceID)
	.def("GetInstanceType", &Lua_Zone::GetInstanceType)
	.def("GetInstanceVersion", &Lua_Zone::GetInstanceVersion)
	.def("GetInstanceTimeRemaining", &Lua_Zone::GetInstanceTimeRemaining)
	.def("GetLavaDamage", &Lua_Zone::GetLavaDamage)
	.def("GetLongName", &Lua_Zone::GetLongName)
	.def("GetMaximumClip", &Lua_Zone::GetMaximumClip)
	.def("GetMaximumExpansion", &Lua_Zone::GetMaximumExpansion)
	.def("GetMaximumLevel", &Lua_Zone::GetMaximumLevel)
	.def("GetMaxClients", &Lua_Zone::GetMaxClients)
	.def("GetMinimumClip", &Lua_Zone::GetMinimumClip)
	.def("GetMinimumExpansion", &Lua_Zone::GetMinimumExpansion)
	.def("GetMinimumLevel", &Lua_Zone::GetMinimumLevel)
	.def("GetMinimumLavaDamage", &Lua_Zone::GetMinimumLavaDamage)
	.def("GetMinimumStatus", &Lua_Zone::GetMinimumStatus)
	.def("GetNote", &Lua_Zone::GetNote)
	.def("GetNPCMaximumAggroDistance", &Lua_Zone::GetNPCMaximumAggroDistance)
	.def("GetPEQZone", &Lua_Zone::GetPEQZone)
	.def("GetRainChance", (int(Lua_Zone::*)(void))&Lua_Zone::GetRainChance)
	.def("GetRainChance", (int(Lua_Zone::*)(uint8))&Lua_Zone::GetRainChance)
	.def("GetRainDuration", (int(Lua_Zone::*)(void))&Lua_Zone::GetRainDuration)
	.def("GetRainDuration", (int(Lua_Zone::*)(uint8))&Lua_Zone::GetRainDuration)
	.def("GetRuleSet", &Lua_Zone::GetRuleSet)
	.def("GetSafeHeading", &Lua_Zone::GetSafeHeading)
	.def("GetSafeX", &Lua_Zone::GetSafeX)
	.def("GetSafeY", &Lua_Zone::GetSafeY)
	.def("GetSafeZ", &Lua_Zone::GetSafeZ)
	.def("GetShortName", &Lua_Zone::GetShortName)
	.def("GetSecondsBeforeIdle", &Lua_Zone::GetSecondsBeforeIdle)
	.def("GetShutdownDelay", &Lua_Zone::GetShutdownDelay)
	.def("GetSky", &Lua_Zone::GetSky)
	.def("GetSkyLock", &Lua_Zone::GetSkyLock)
	.def("GetSnowChance", (int(Lua_Zone::*)(void))&Lua_Zone::GetSnowChance)
	.def("GetSnowChance", (int(Lua_Zone::*)(uint8))&Lua_Zone::GetSnowChance)
	.def("GetSnowDuration", (int(Lua_Zone::*)(void))&Lua_Zone::GetSnowDuration)
	.def("GetSnowDuration", (int(Lua_Zone::*)(uint8))&Lua_Zone::GetSnowDuration)
	.def("GetTimeType", &Lua_Zone::GetTimeType)
	.def("GetTimeZone", &Lua_Zone::GetTimeZone)
	.def("GetZoneDescription", &Lua_Zone::GetZoneDescription)
	.def("GetZoneID", &Lua_Zone::GetZoneID)
	.def("GetZoneType", &Lua_Zone::GetZoneType)
	.def("GetUnderworld", &Lua_Zone::GetUnderworld)
	.def("GetUnderworldTeleportIndex", &Lua_Zone::GetUnderworldTeleportIndex)
	.def("GetWalkSpeed", &Lua_Zone::GetWalkSpeed)
	.def("GetZoneZType", &Lua_Zone::GetZoneZType)
	.def("GetZoneTotalBlockedSpells", &Lua_Zone::GetZoneTotalBlockedSpells)
	.def("HasGraveyard", &Lua_Zone::HasGraveyard)
	.def("HasMap", &Lua_Zone::HasMap)
	.def("HasWaterMap", &Lua_Zone::HasWaterMap)
	.def("HasWeather", &Lua_Zone::HasWeather)
	.def("IsCity", &Lua_Zone::IsCity)
	.def("IsHotzone", &Lua_Zone::IsHotzone)
	.def("IsInstancePersistent", &Lua_Zone::IsInstancePersistent)
	.def("IsIdleWhenEmpty", &Lua_Zone::IsIdleWhenEmpty)
	.def("IsPVPZone", &Lua_Zone::IsPVPZone)
	.def("IsRaining", &Lua_Zone::IsRaining)
	.def("IsSnowing", &Lua_Zone::IsSnowing)
	.def("IsSpecialBindLocation", &Lua_Zone::IsSpecialBindLocation)
	.def("IsSpellBlocked", &Lua_Zone::IsSpellBlocked)
	.def("IsStaticZone", &Lua_Zone::IsStaticZone)
	.def("IsUCSServerAvailable", &Lua_Zone::IsUCSServerAvailable)
	.def("IsWaterZone", &Lua_Zone::IsWaterZone)
	.def("Repop", (void(Lua_Zone::*)(void))&Lua_Zone::Repop)
	.def("Repop", (void(Lua_Zone::*)(bool))&Lua_Zone::Repop)
	.def("SetAAEXPModifier", &Lua_Zone::SetAAEXPModifier)
	.def("SetAAEXPModifierByCharacterID", &Lua_Zone::SetAAEXPModifierByCharacterID)
	.def("SetEXPModifier", &Lua_Zone::SetEXPModifier)
	.def("SetEXPModifierByCharacterID", &Lua_Zone::SetEXPModifierByCharacterID)
	.def("SetInstanceTimer", &Lua_Zone::SetInstanceTimer)
	.def("SetInstanceTimeRemaining", &Lua_Zone::SetInstanceTimeRemaining)
	.def("SetIsHotzone", &Lua_Zone::SetIsHotzone)
	.def("ShowZoneGlobalLoot", &Lua_Zone::ShowZoneGlobalLoot);
}

