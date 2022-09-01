#include "../common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "embperl.h"
#include "expedition.h"
#include "../common/zone_store.h"
#include "../common/global_define.h"

void Perl_Expedition_AddLockout(Expedition* self, std::string event_name, uint32_t seconds)
{
	self->AddLockout(event_name, seconds);
}

void Perl_Expedition_AddLockoutDuration(Expedition* self, std::string event_name, int seconds)
{
	self->AddLockoutDuration(event_name, seconds);
}

void Perl_Expedition_AddLockoutDuration(Expedition* self, std::string event_name, int seconds, bool members_only)
{
	self->AddLockoutDuration(event_name, seconds, members_only);
}

void Perl_Expedition_AddReplayLockout(Expedition* self, uint32_t seconds)
{
	self->AddReplayLockout(seconds);
}

void Perl_Expedition_AddReplayLockoutDuration(Expedition* self, int seconds)
{
	self->AddReplayLockoutDuration(seconds);
}

void Perl_Expedition_AddReplayLockoutDuration(Expedition* self, int seconds, bool members_only)
{
	self->AddReplayLockoutDuration(seconds, members_only);
}

uint32_t Perl_Expedition_GetDynamicZoneID(Expedition* self)
{
	return self->GetDynamicZone()->GetID();
}

uint32_t Perl_Expedition_GetID(Expedition* self)
{
	return self->GetID();
}

uint16_t Perl_Expedition_GetInstanceID(Expedition* self)
{
	return self->GetDynamicZone()->GetInstanceID();
}

std::string Perl_Expedition_GetLeaderName(Expedition* self)
{
	return self->GetLeaderName();
}

perl::reference Perl_Expedition_GetLockouts(Expedition* self)
{
	perl::hash table;
	auto lockouts = self->GetLockouts();
	for (const auto& lockout : lockouts)
	{
		table[lockout.first] = lockout.second.GetSecondsRemaining();
	}
	return perl::reference(table);
}

std::string Perl_Expedition_GetLootEventByNPCTypeID(Expedition* self, uint32_t npc_type_id)
{
	return self->GetLootEventByNPCTypeID(npc_type_id);
}

std::string Perl_Expedition_GetLootEventBySpawnID(Expedition* self, uint32_t spawn_id)
{
	return self->GetLootEventBySpawnID(spawn_id);
}

uint32_t Perl_Expedition_GetMemberCount(Expedition* self)
{
	return self->GetDynamicZone()->GetMemberCount();
}

perl::reference Perl_Expedition_GetMembers(Expedition* self)
{
	perl::hash table;
	for (const auto& member : self->GetDynamicZone()->GetMembers())
	{
		table[member.name] = member.id;
	}
	return perl::reference(table);
}

std::string Perl_Expedition_GetName(Expedition* self)
{
	return self->GetName();
}

uint32_t Perl_Expedition_GetSecondsRemaining(Expedition* self)
{
	return self->GetDynamicZone()->GetSecondsRemaining();
}

std::string Perl_Expedition_GetUUID(Expedition* self)
{
	return self->GetDynamicZone()->GetUUID();
}

uint16_t Perl_Expedition_GetZoneID(Expedition* self)
{
	return self->GetDynamicZone()->GetZoneID();
}

std::string Perl_Expedition_GetZoneName(Expedition* self)
{
	return ZoneName(self->GetDynamicZone()->GetZoneID());
}

uint32_t Perl_Expedition_GetZoneVersion(Expedition* self)
{
	return self->GetDynamicZone()->GetZoneVersion();
}

bool Perl_Expedition_HasLockout(Expedition* self, std::string event_name)
{
	return self->HasLockout(event_name);
}

bool Perl_Expedition_HasReplayLockout(Expedition* self)
{
	return self->HasReplayLockout();
}

bool Perl_Expedition_IsLocked(Expedition* self)
{
	return self->IsLocked();
}

void Perl_Expedition_RemoveCompass(Expedition* self)
{
	self->GetDynamicZone()->SetCompass(0, 0, 0, 0, true);
}

void Perl_Expedition_RemoveLockout(Expedition* self, std::string event_name)
{
	self->RemoveLockout(event_name);
}

void Perl_Expedition_SetCompass(Expedition* self, perl::scalar zone, float x, float y, float z)
{
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();
	self->GetDynamicZone()->SetCompass(zone_id, x, y, z, true);
}

void Perl_Expedition_SetLocked(Expedition* self, bool locked)
{
	self->SetLocked(locked, ExpeditionLockMessage::None);
}

void Perl_Expedition_SetLocked(Expedition* self, bool locked, int lock_msg)
{
	self->SetLocked(locked, static_cast<ExpeditionLockMessage>(lock_msg), true);
}

void Perl_Expedition_SetLocked(Expedition* self, bool locked, int lock_msg, uint32_t color)
{
	self->SetLocked(locked, static_cast<ExpeditionLockMessage>(lock_msg), true, color);
}

void Perl_Expedition_SetLootEventByNPCTypeID(Expedition* self, uint32_t npc_type_id, std::string event_name)
{
	self->SetLootEventByNPCTypeID(npc_type_id, event_name);
}

void Perl_Expedition_SetLootEventBySpawnID(Expedition* self, uint32_t entity_id, std::string event_name)
{
	self->SetLootEventBySpawnID(entity_id, event_name);
}

void Perl_Expedition_SetReplayLockoutOnMemberJoin(Expedition* self, bool enable)
{
	self->SetReplayLockoutOnMemberJoin(enable, true);
}

void Perl_Expedition_SetSafeReturn(Expedition* self, perl::scalar zone, float x, float y, float z, float heading)
{
	uint32_t zone_id = zone.is_string() ? ZoneID(zone.c_str()) : zone.as<uint32_t>();
	self->GetDynamicZone()->SetSafeReturn(zone_id, x, y, z, heading, true);
}

void Perl_Expedition_SetSecondsRemaining(Expedition* self, uint32_t seconds_remaining)
{
	self->GetDynamicZone()->SetSecondsRemaining(seconds_remaining);
}

void Perl_Expedition_SetSwitchID(Expedition* self, int dz_switch_id)
{
	self->GetDynamicZone()->SetSwitchID(dz_switch_id, true);
}

void Perl_Expedition_SetZoneInLocation(Expedition* self, float x, float y, float z, float heading)
{
	self->GetDynamicZone()->SetZoneInLocation(x, y, z, heading, true);
}

void Perl_Expedition_UpdateLockoutDuration(Expedition* self, std::string event_name, uint32_t seconds)
{
	self->UpdateLockoutDuration(event_name, seconds);
}

void Perl_Expedition_UpdateLockoutDuration(Expedition* self, std::string event_name, uint32_t seconds, bool members_only)
{
	self->UpdateLockoutDuration(event_name, seconds, members_only);
}

void perl_register_expedition()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_class<Expedition>("Expedition");
	package.add("AddLockout", &Perl_Expedition_AddLockout);
	package.add("AddLockoutDuration", (void(*)(Expedition*, std::string, int))&Perl_Expedition_AddLockoutDuration);
	package.add("AddLockoutDuration", (void(*)(Expedition*, std::string, int, bool))&Perl_Expedition_AddLockoutDuration);
	package.add("AddReplayLockout", &Perl_Expedition_AddReplayLockout);
	package.add("AddReplayLockoutDuration", (void(*)(Expedition*, int))&Perl_Expedition_AddReplayLockoutDuration);
	package.add("AddReplayLockoutDuration", (void(*)(Expedition*, int, bool))&Perl_Expedition_AddReplayLockoutDuration);
	package.add("GetDynamicZoneID", &Perl_Expedition_GetDynamicZoneID);
	package.add("GetID", &Perl_Expedition_GetID);
	package.add("GetInstanceID", &Perl_Expedition_GetInstanceID);
	package.add("GetLeaderName", &Perl_Expedition_GetLeaderName);
	package.add("GetLockouts", &Perl_Expedition_GetLockouts);
	package.add("GetLootEventByNPCTypeID", &Perl_Expedition_GetLootEventByNPCTypeID);
	package.add("GetLootEventBySpawnID", &Perl_Expedition_GetLootEventBySpawnID);
	package.add("GetMemberCount", &Perl_Expedition_GetMemberCount);
	package.add("GetMembers", &Perl_Expedition_GetMembers);
	package.add("GetName", &Perl_Expedition_GetName);
	package.add("GetSecondsRemaining", &Perl_Expedition_GetSecondsRemaining);
	package.add("GetUUID", &Perl_Expedition_GetUUID);
	package.add("GetZoneID", &Perl_Expedition_GetZoneID);
	package.add("GetZoneName", &Perl_Expedition_GetZoneName);
	package.add("GetZoneVersion", &Perl_Expedition_GetZoneVersion);
	package.add("HasLockout", &Perl_Expedition_HasLockout);
	package.add("HasReplayLockout", &Perl_Expedition_HasReplayLockout);
	package.add("IsLocked", &Perl_Expedition_IsLocked);
	package.add("RemoveCompass", &Perl_Expedition_RemoveCompass);
	package.add("RemoveLockout", &Perl_Expedition_RemoveLockout);
	package.add("SetCompass", &Perl_Expedition_SetCompass);
	package.add("SetLocked", (void(*)(Expedition*, bool))&Perl_Expedition_SetLocked);
	package.add("SetLocked", (void(*)(Expedition*, bool, int))&Perl_Expedition_SetLocked);
	package.add("SetLocked", (void(*)(Expedition*, bool, int, uint32_t))&Perl_Expedition_SetLocked);
	package.add("SetLootEventByNPCTypeID", &Perl_Expedition_SetLootEventByNPCTypeID);
	package.add("SetLootEventBySpawnID", &Perl_Expedition_SetLootEventBySpawnID);
	package.add("SetReplayLockoutOnMemberJoin", &Perl_Expedition_SetReplayLockoutOnMemberJoin);
	package.add("SetSafeReturn", &Perl_Expedition_SetSafeReturn);
	package.add("SetSecondsRemaining", &Perl_Expedition_SetSecondsRemaining);
	package.add("SetSwitchID", &Perl_Expedition_SetSwitchID);
	package.add("SetZoneInLocation", &Perl_Expedition_SetZoneInLocation);
	package.add("UpdateLockoutDuration", (void(*)(Expedition*, std::string, uint32_t))&Perl_Expedition_UpdateLockoutDuration);
	package.add("UpdateLockoutDuration", (void(*)(Expedition*, std::string, uint32_t, bool))&Perl_Expedition_UpdateLockoutDuration);
}

void perl_register_expedition_lock_messages()
{
	perl::interpreter perl(PERL_GET_THX);

	auto package = perl.new_package("ExpeditionLockMessage");
	package.add_const("None", static_cast<int>(ExpeditionLockMessage::None));
	package.add_const("Close", static_cast<int>(ExpeditionLockMessage::Close));
	package.add_const("Begin", static_cast<int>(ExpeditionLockMessage::Begin));
}

#endif //EMBPERL_XS_CLASSES
