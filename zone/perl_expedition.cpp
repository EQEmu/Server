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

#include "../common/features.h"

#ifdef EMBPERL_XS_CLASSES

#include "expedition.h"
#include "zone_store.h"
#include "embperl.h"
#include "../common/global_define.h"

#ifdef seed
#undef seed
#endif

#ifdef THIS        /* this macro seems to leak out on some systems */
#undef THIS
#endif

#define VALIDATE_THIS_IS_EXPEDITION \
	do { \
		if (sv_derived_from(ST(0), "Expedition")) { \
			IV tmp = SvIV((SV*)SvRV(ST(0))); \
			THIS = INT2PTR(Expedition*, tmp); \
		} else { \
			Perl_croak(aTHX_ "THIS is not of type Expedition"); \
		} \
		if (THIS == nullptr) { \
			Perl_croak(aTHX_ "THIS is nullptr, avoiding crash."); \
		} \
	} while (0);

XS(XS_Expedition_AddLockout);
XS(XS_Expedition_AddLockout) {
	dXSARGS;
	if (items != 3) {
		Perl_croak(aTHX_ "Usage: Expedition::AddLockout(THIS, string event_name, uint32 seconds)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	std::string event_name(SvPV_nolen(ST(1)));
	uint32_t seconds = static_cast<uint32_t>(SvUV(ST(2)));

	THIS->AddLockout(event_name, seconds);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_AddLockoutDuration);
XS(XS_Expedition_AddLockoutDuration) {
	dXSARGS;
	if (items != 3 && items != 4) {
		Perl_croak(aTHX_ "Usage: Expedition::AddLockout(THIS, string event_name, int seconds, [bool members_only = true])");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	std::string event_name(SvPV_nolen(ST(1)));
	int seconds = static_cast<int>(SvUV(ST(2)));
	if (items == 4)
	{
		bool members_only = (bool)SvTRUE(ST(3));
		THIS->AddLockoutDuration(event_name, seconds, members_only);
	}
	else
	{
		THIS->AddLockoutDuration(event_name, seconds);
	}

	XSRETURN_EMPTY;
}

XS(XS_Expedition_AddReplayLockout);
XS(XS_Expedition_AddReplayLockout) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: Expedition::AddReplayLockout(THIS, uint32 seconds)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	uint32_t seconds = static_cast<uint32_t>(SvUV(ST(1)));

	THIS->AddReplayLockout(seconds);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_AddReplayLockoutDuration);
XS(XS_Expedition_AddReplayLockoutDuration) {
	dXSARGS;
	if (items != 2 && items != 3) {
		Perl_croak(aTHX_ "Usage: Expedition::AddReplayLockoutDuration(THIS, int seconds, [bool members_only = true])");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	int seconds = static_cast<int>(SvUV(ST(1)));
	if (items == 3)
	{
		bool members_only = (bool)SvTRUE(ST(2));
		THIS->AddReplayLockoutDuration(seconds, members_only);
	}
	else
	{
		THIS->AddReplayLockoutDuration(seconds);
	}

	XSRETURN_EMPTY;
}

XS(XS_Expedition_GetDynamicZoneID);
XS(XS_Expedition_GetDynamicZoneID) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetDynamicZoneID(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_UV(THIS->GetDynamicZone().GetID());
}

XS(XS_Expedition_GetID);
XS(XS_Expedition_GetID) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetID(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_UV(THIS->GetID());
}

XS(XS_Expedition_GetInstanceID);
XS(XS_Expedition_GetInstanceID) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetInstanceID(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_UV(THIS->GetDynamicZone().GetInstanceID());
}

XS(XS_Expedition_GetLeaderName);
XS(XS_Expedition_GetLeaderName) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetLeaderName(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_PV(THIS->GetLeaderName().c_str());
}

XS(XS_Expedition_GetLockouts);
XS(XS_Expedition_GetLockouts) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetLockouts(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	HV* hash = newHV();

	auto lockouts = THIS->GetLockouts();
	for (const auto& lockout : lockouts)
	{
		hv_store(hash, lockout.first.c_str(), static_cast<uint32_t>(lockout.first.size()),
			newSVuv(lockout.second.GetSecondsRemaining()), 0);
	}

	ST(0) = sv_2mortal(newRV_noinc((SV*)hash)); // take ownership of hash (refcnt remains 1)
	XSRETURN(1);
}

XS(XS_Expedition_GetLootEventByNPCTypeID);
XS(XS_Expedition_GetLootEventByNPCTypeID) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: Expedition::GetLootEventByNPCTypeID(THIS, uint32 npc_type_id)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	uint32_t npc_type_id = static_cast<uint32_t>(SvUV(ST(1)));

	XSRETURN_PV(THIS->GetLootEventByNPCTypeID(npc_type_id).c_str());
}

XS(XS_Expedition_GetLootEventBySpawnID);
XS(XS_Expedition_GetLootEventBySpawnID) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: Expedition::GetLootEventBySpawnID(THIS, uint32 spawn_id)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	uint32_t spawn_id = static_cast<uint32_t>(SvUV(ST(1)));

	XSRETURN_PV(THIS->GetLootEventBySpawnID(spawn_id).c_str());
}

XS(XS_Expedition_GetMemberCount);
XS(XS_Expedition_GetMemberCount) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetMemberCount(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_UV(THIS->GetMemberCount());
}

XS(XS_Expedition_GetMembers);
XS(XS_Expedition_GetMembers) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetMembers(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	HV* hash = newHV();

	auto members = THIS->GetMembers();
	for (const auto& member : members)
	{
		hv_store(hash, member.name.c_str(), static_cast<uint32_t>(member.name.size()),
			newSVuv(member.char_id), 0);
	}

	ST(0) = sv_2mortal(newRV_noinc((SV*)hash));
	XSRETURN(1);
}

XS(XS_Expedition_GetName);
XS(XS_Expedition_GetName) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetName(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_PV(THIS->GetName().c_str());
}

XS(XS_Expedition_GetSecondsRemaining);
XS(XS_Expedition_GetSecondsRemaining) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetSecondsRemaining(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_UV(THIS->GetDynamicZone().GetSecondsRemaining());
}

XS(XS_Expedition_GetUUID);
XS(XS_Expedition_GetUUID) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetUUID(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_PV(THIS->GetUUID().c_str());
}

XS(XS_Expedition_GetZoneID);
XS(XS_Expedition_GetZoneID) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetZoneID(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_UV(THIS->GetDynamicZone().GetZoneID());
}

XS(XS_Expedition_GetZoneName);
XS(XS_Expedition_GetZoneName) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetZoneName(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_PV(ZoneName(THIS->GetDynamicZone().GetZoneID()));
}

XS(XS_Expedition_GetZoneVersion);
XS(XS_Expedition_GetZoneVersion) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::GetZoneVersion(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	XSRETURN_UV(THIS->GetDynamicZone().GetZoneVersion());
}

XS(XS_Expedition_HasLockout);
XS(XS_Expedition_HasLockout) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: Expedition::HasLockout(THIS, string event_name)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	std::string event_name(SvPV_nolen(ST(1)));

	bool result = THIS->HasLockout(event_name);
	ST(0) = boolSV(result);
	XSRETURN(1);
}

XS(XS_Expedition_HasReplayLockout);
XS(XS_Expedition_HasReplayLockout) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::HasReplayLockout(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	bool result = THIS->HasReplayLockout();
	ST(0) = boolSV(result);
	XSRETURN(1);
}

XS(XS_Expedition_IsLocked);
XS(XS_Expedition_IsLocked) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::IsLocked(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	ST(0) = boolSV(THIS->IsLocked());
	XSRETURN(1);
}

XS(XS_Expedition_RemoveCompass);
XS(XS_Expedition_RemoveCompass) {
	dXSARGS;
	if (items != 1) {
		Perl_croak(aTHX_ "Usage: Expedition::RemoveCompass(THIS)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	THIS->GetDynamicZone().SetCompass(0, 0, 0, 0, true);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_RemoveLockout);
XS(XS_Expedition_RemoveLockout) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: Expedition::RemoveLockout(THIS, string event_name)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	std::string event_name(SvPV_nolen(ST(1)));

	THIS->RemoveLockout(event_name);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetCompass);
XS(XS_Expedition_SetCompass) {
	dXSARGS;
	if (items != 5) {
		Perl_croak(aTHX_ "Usage: Expedition::SetCompass(THIS, uint32 zone_id | string zone_name, float x, float y, float z)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	float x = static_cast<float>(SvNV(ST(2)));
	float y = static_cast<float>(SvNV(ST(3)));
	float z = static_cast<float>(SvNV(ST(4)));

	if (SvTYPE(ST(1)) == SVt_PV)
	{
		std::string zone_name(SvPV_nolen(ST(1)));
		THIS->GetDynamicZone().SetCompass(ZoneID(zone_name), x, y, z, true);
	}
	else if (SvTYPE(ST(1)) == SVt_IV)
	{
		uint32_t zone_id = static_cast<uint32_t>(SvUV(ST(1)));
		THIS->GetDynamicZone().SetCompass(zone_id, x, y, z, true);
	}
	else
	{
		Perl_croak(aTHX_ "Expedition::SetCompass expected an integer or string");
	}

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetLocked);
XS(XS_Expedition_SetLocked) {
	dXSARGS;
	if (items != 2 && items != 3 && items != 4) {
		Perl_croak(aTHX_ "Usage: Expedition::SetLocked(THIS, bool locked, [int lock_msg = 0], [uint32 color = 15])");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	bool locked = (bool)SvTRUE(ST(1));
	int lock_msg = (items == 3) ? static_cast<int>(SvIV(ST(2))) : 0;
	if (items == 4)
	{
		THIS->SetLocked(locked, static_cast<ExpeditionLockMessage>(lock_msg), true, (uint32)SvUV(ST(3)));
	}
	else
	{
		THIS->SetLocked(locked, static_cast<ExpeditionLockMessage>(lock_msg), true);
	}

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetLootEventByNPCTypeID);
XS(XS_Expedition_SetLootEventByNPCTypeID) {
	dXSARGS;
	if (items != 3) {
		Perl_croak(aTHX_ "Usage: Expedition::SetLootEventByNPCTypeID(THIS, uint32 npc_type_id, string event_name)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	uint32_t npc_type_id = static_cast<uint32_t>(SvUV(ST(1)));
	std::string event_name(SvPV_nolen(ST(2)));

	THIS->SetLootEventByNPCTypeID(npc_type_id, event_name);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetLootEventBySpawnID);
XS(XS_Expedition_SetLootEventBySpawnID) {
	dXSARGS;
	if (items != 3) {
		Perl_croak(aTHX_ "Usage: Expedition::SetLootEventBySpawnID(THIS, uint32 spawn_id, string event_name)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	uint32_t spawn_id = static_cast<uint32_t>(SvUV(ST(1)));
	std::string event_name(SvPV_nolen(ST(2)));

	THIS->SetLootEventBySpawnID(spawn_id, event_name);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetReplayLockoutOnMemberJoin);
XS(XS_Expedition_SetReplayLockoutOnMemberJoin) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: Expedition::SetReplayLockoutOnMemberJoin(THIS, bool enable)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	bool enable = (bool)SvTRUE(ST(1));

	THIS->SetReplayLockoutOnMemberJoin(enable, true);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetSafeReturn);
XS(XS_Expedition_SetSafeReturn) {
	dXSARGS;
	if (items != 6) {
		Perl_croak(aTHX_ "Usage: Expedition::SetSafeReturn(THIS, uint32 zone_id | string zone_name, float x, float y, float z, float heading)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	float x = static_cast<float>(SvNV(ST(2)));
	float y = static_cast<float>(SvNV(ST(3)));
	float z = static_cast<float>(SvNV(ST(4)));
	float heading = static_cast<float>(SvNV(ST(5)));

	if (SvTYPE(ST(1)) == SVt_PV)
	{
		std::string zone_name(SvPV_nolen(ST(1)));
		THIS->GetDynamicZone().SetSafeReturn(ZoneID(zone_name), x, y, z, heading, true);
	}
	else if (SvTYPE(ST(1)) == SVt_IV)
	{
		uint32_t zone_id = static_cast<uint32_t>(SvUV(ST(1)));
		THIS->GetDynamicZone().SetSafeReturn(zone_id, x, y, z, heading, true);
	}
	else
	{
		Perl_croak(aTHX_ "Expedition::SetSafeReturn expected an integer or string");
	}

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetSecondsRemaining);
XS(XS_Expedition_SetSecondsRemaining) {
	dXSARGS;
	if (items != 2) {
		Perl_croak(aTHX_ "Usage: Expedition::SetSecondsRemaining(THIS, uint32 seconds_remaining)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	uint32_t seconds_remaining = static_cast<uint32_t>(SvUV(ST(1)));
	THIS->GetDynamicZone().SetSecondsRemaining(seconds_remaining);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_SetZoneInLocation);
XS(XS_Expedition_SetZoneInLocation) {
	dXSARGS;
	if (items != 5) {
		Perl_croak(aTHX_ "Usage: Expedition::SetZoneInLocation(THIS, float x, float y, float z, float heading)");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	float x = static_cast<float>(SvNV(ST(1)));
	float y = static_cast<float>(SvNV(ST(2)));
	float z = static_cast<float>(SvNV(ST(3)));
	float heading = static_cast<float>(SvNV(ST(4)));

	THIS->GetDynamicZone().SetZoneInLocation(x, y, z, heading, true);

	XSRETURN_EMPTY;
}

XS(XS_Expedition_UpdateLockoutDuration);
XS(XS_Expedition_UpdateLockoutDuration) {
	dXSARGS;
	if (items != 3 && items != 4) {
		Perl_croak(aTHX_ "Usage: Expedition::UpdateLockoutDuration(THIS, string event_name, uint32 seconds, [bool members_only = true])");
	}

	Expedition* THIS = nullptr;
	VALIDATE_THIS_IS_EXPEDITION;

	std::string event_name(SvPV_nolen(ST(1)));
	uint32_t seconds = static_cast<uint32_t>(SvUV(ST(2)));

	if (items == 4)
	{
		bool members_only = (bool)SvTRUE(ST(3));
		THIS->UpdateLockoutDuration(event_name, seconds, members_only);
	}
	else
	{
		THIS->UpdateLockoutDuration(event_name, seconds);
	}

	XSRETURN_EMPTY;
}

XS(boot_Expedition);
XS(boot_Expedition) {
	dXSARGS;
	char file[256];
	strncpy(file, __FILE__, 256);
	file[255] = 0;

	if (items != 1) {
		fprintf(stderr, "boot_Expedition does not take any arguments.");
	}
	char buf[128];

	XS_VERSION_BOOTCHECK;
	newXSproto(strcpy(buf, "AddLockout"), XS_Expedition_AddLockout, file, "$$$");
	newXSproto(strcpy(buf, "AddLockoutDuration"), XS_Expedition_AddLockoutDuration, file, "$$$;$");
	newXSproto(strcpy(buf, "AddReplayLockout"), XS_Expedition_AddReplayLockout, file, "$$");
	newXSproto(strcpy(buf, "AddReplayLockoutDuration"), XS_Expedition_AddReplayLockoutDuration, file, "$$;$");
	newXSproto(strcpy(buf, "GetDynamicZoneID"), XS_Expedition_GetDynamicZoneID, file, "$");
	newXSproto(strcpy(buf, "GetID"), XS_Expedition_GetID, file, "$");
	newXSproto(strcpy(buf, "GetInstanceID"), XS_Expedition_GetInstanceID, file, "$");
	newXSproto(strcpy(buf, "GetLeaderName"), XS_Expedition_GetLeaderName, file, "$");
	newXSproto(strcpy(buf, "GetLockouts"), XS_Expedition_GetLockouts, file, "$");
	newXSproto(strcpy(buf, "GetLootEventByNPCTypeID"), XS_Expedition_GetLootEventByNPCTypeID, file, "$$");
	newXSproto(strcpy(buf, "GetLootEventBySpawnID"), XS_Expedition_GetLootEventBySpawnID, file, "$$");
	newXSproto(strcpy(buf, "GetMemberCount"), XS_Expedition_GetMemberCount, file, "$");
	newXSproto(strcpy(buf, "GetMembers"), XS_Expedition_GetMembers, file, "$");
	newXSproto(strcpy(buf, "GetName"), XS_Expedition_GetName, file, "$");
	newXSproto(strcpy(buf, "GetSecondsRemaining"), XS_Expedition_GetSecondsRemaining, file, "$");
	newXSproto(strcpy(buf, "GetUUID"), XS_Expedition_GetUUID, file, "$");
	newXSproto(strcpy(buf, "GetZoneID"), XS_Expedition_GetZoneID, file, "$");
	newXSproto(strcpy(buf, "GetZoneName"), XS_Expedition_GetZoneName, file, "$");
	newXSproto(strcpy(buf, "GetZoneVersion"), XS_Expedition_GetZoneVersion, file, "$");
	newXSproto(strcpy(buf, "HasLockout"), XS_Expedition_HasLockout, file, "$$");
	newXSproto(strcpy(buf, "HasReplayLockout"), XS_Expedition_HasReplayLockout, file, "$");
	newXSproto(strcpy(buf, "IsLocked"), XS_Expedition_IsLocked, file, "$");
	newXSproto(strcpy(buf, "RemoveCompass"), XS_Expedition_RemoveCompass, file, "$");
	newXSproto(strcpy(buf, "RemoveLockout"), XS_Expedition_RemoveLockout, file, "$$");
	newXSproto(strcpy(buf, "SetCompass"), XS_Expedition_SetCompass, file, "$$$$$");
	newXSproto(strcpy(buf, "SetLocked"), XS_Expedition_SetLocked, file, "$$;$$");
	newXSproto(strcpy(buf, "SetLootEventByNPCTypeID"), XS_Expedition_SetLootEventByNPCTypeID, file, "$$$");
	newXSproto(strcpy(buf, "SetLootEventBySpawnID"), XS_Expedition_SetLootEventBySpawnID, file, "$$$");
	newXSproto(strcpy(buf, "SetReplayLockoutOnMemberJoin"), XS_Expedition_SetReplayLockoutOnMemberJoin, file, "$$");
	newXSproto(strcpy(buf, "SetSafeReturn"), XS_Expedition_SetSafeReturn, file, "$$$$$$");
	newXSproto(strcpy(buf, "SetSecondsRemaining"), XS_Expedition_SetSecondsRemaining, file, "$$");
	newXSproto(strcpy(buf, "SetZoneInLocation"), XS_Expedition_SetZoneInLocation, file, "$$$$$");
	newXSproto(strcpy(buf, "UpdateLockoutDuration"), XS_Expedition_UpdateLockoutDuration, file, "$$$;$");

	HV* stash = gv_stashpvs("ExpeditionLockMessage", GV_ADD);
	newCONSTSUB(stash, "None", newSViv(static_cast<int>(ExpeditionLockMessage::None)));
	newCONSTSUB(stash, "Close", newSViv(static_cast<int>(ExpeditionLockMessage::Close)));
	newCONSTSUB(stash, "Begin", newSViv(static_cast<int>(ExpeditionLockMessage::Begin)));

	XSRETURN_YES;
}

#endif //EMBPERL_XS_CLASSES
