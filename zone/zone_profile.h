/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef ZONE_PROFILE_H
#define ZONE_PROFILE_H

#ifdef ZONE	//only possibly profile if we are building zone
#include "../common/features.h"
#endif

#ifdef EQPROFILE
	extern void DumpZoneProfile();
	extern void ResetZoneProfile();
	#ifdef PROFILE_DUMP_TIME
		extern void ProfilerProcess();
	#endif
#else
	#ifdef ZONE_PROFILE	//no EQPROFILE == no zone profile
	#undef ZONE_PROFILE
	#endif
#endif

#ifdef ZONE_PROFILE

#include "../common/profiler.h"

class ZoneProfiler : public GeneralProfiler {
public:
	enum {
		Client_Process = 0,
		Client_HandlePacket,
		Client_QueuePacket,
		Client_Save,
		Client_Attack,
		Client_CalcBonuses,
		Client_GetFactionLevel,
		Client_SetFactionLevel,
		Client_SetFactionLevel2,

		NPC_Attack,
		NPC_GetReverseFactionCon,
		NPC_Process,
		NPC_Death,

		EntityList_TrapProcess,
		EntityList_GroupProcess,
		EntityList_QueueToGroupsForNPCHealthAA,
		EntityList_DoorProcess,
		EntityList_ObjectProcess,
		EntityList_CorpseProcess,
		EntityList_MobProcess,
		EntityList_BeaconProcess,
		EntityList_Process,
		EntityList_RaidProcess,
		EntityList_AICheckCloseAggro,
		EntityList_AICheckCloseBeneficialSpells,
		EntityList_CheckClientAggro,
		EntityList_CheckClientAggro_Loop,
		EntityList_AIYellForHelp,

		HateList_Find,
		HateList_GetDamageTop,
		HateList_GetClosest,
		HateList_DoFactionHits,
		HateList_GetTop,
		HateList_IsEmpty,
		HateList_GetMostHate,

		Mob_CheckWillAggro,
		Mob_CheckLosFN,
		Mob_Dist,
		Mob_DistNoZ,
		Mob_DistNoRoot,
		Mob_DistNoRootNoZ,
		Mob_AICastSpell,
		Mob_SpellEffect,
		Mob_DoBuffTic,
		Mob_CastSpell,
		Mob_DoCastSpell,
		Mob_CastedSpellFinished,
		Mob_SpellFinished,
		Mob_IsImmuneToSpell,
		Mob_CalculateNewPosition,
		Mob_CalculateNewPosition2,
		Mob_GetWeaponDamageA,
		Mob_GetWeaponDamageB,
		Mob_GetWeaponDamageBonus,
		Mob_TryWeaponProcA,
		Mob_TryWeaponProcB,

		Map_LineIntersectsZone,
		Map_LineIntersectsNode,
		Map_FindBestZ,

		Pathing_FindRoute_FromNodes,
		Pathing_CheckTerrainPassable,
		Pathing_FindRoute_FromVertices,
		Pathing_UpdatePath,
		Pathing_NoHazards,
		Pathing_OpenDoors,
		Pathing_VertexDistance,
		Pathing_VertexDistanceNoRoot,

		Mob_AI_Process,
		Mob_AI_Process_engaged,
		Mob_AI_Process_engaged_cast,
		Mob_AI_Process_pursue_cast,
		Mob_AI_Process_autocast,
		Mob_AI_Process_scanarea,
		Mob_AI_Process_move,
		Mob_AI_Process_pet,
		Mob_AI_Process_roambox,
		Mob_AI_Process_roamer,
		Mob_AI_Process_guard,



		Database_AddLootTableToNPC,

		Zone_Bootup,
		Zone_Process,

		WorldServer_Process,

		Spawn2_Process,

		PerlembParser_SendCommands,
		PerlXSParser_SendCommands,

		command_realdispatch,

		net_main,

		MaxZoneProfilerId
	};

	inline ZoneProfiler() : GeneralProfiler(MaxZoneProfilerId) { }

};

extern ZoneProfiler _zp;

#define _ZP(name) _GP(_zp, ZoneProfiler, name)
#else
	//no zone profiling, dummy functions
#define _ZP(name) ;

#endif //ZONE_PROFILE

#endif

