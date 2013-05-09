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
#include "../common/debug.h"
#include "../common/features.h"

#ifdef EQPROFILE
#include "zone_profile.h"
#include "../common/common_profile.h"
#include "../common/timer.h"
#include "../common/rdtsc.h"

#include <time.h>
#include <string.h>
#ifdef WIN32
	#include <process.h>

	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else
	#include <sys/types.h>
	#include <unistd.h>
	#include <stdarg.h>
#endif

#include <vector>
#include <algorithm>

using namespace std;

#ifdef COMMON_PROFILE
CommonProfiler _cp;
#endif
#ifdef ZONE_PROFILE
ZoneProfiler _zp;
#endif

class _DZP_Data {
public:
	_DZP_Data(const char *_str, unsigned long long _count, double _dur) {
		str = _str;
		count = _count;
		dur = _dur;
	}

	const char *str;
	unsigned long long count;
	double dur;
};

bool operator<(const _DZP_Data &l, const _DZP_Data &r) {
	return(l.dur < r.dur);
}

RDTSC_Collector __DZP_timer(true);

#ifdef COMMON_PROFILE
/*

*/
const char *__cp_names[CommonProfiler::MaxCommonProfilerId] = {
		"Database_SaveInventory = 0",
		"Database_StoreCharacter",
		"Database_GetCharacterInfoForLogin",
		"Database_GetCharacterInfoForLogin_result",
		"Database_GetPlayerProfile",
		"Database_GetInventory",
		"Database_GetInventory_name",
		"Database_SetPlayerProfile",
		"Database_DBLoadItems",
		"Database_GetWaypoints",
		"Database_DBLoadNPCFactionLists",

		"DBcore_RunQuery",

		"DBAsync_ProcessWork",
		"DBAsync_DispatchWork",
		"DBAsyncLoop_loop",

		"EQStreamServer_Process",

		"EQStream_Process",

		"EQStreamServerLoop",
		"EQStreamInLoop",
		"EQStreamOutLoop",
		"TCPServerLoop",
		"TCPConnectionLoop",

		"Inventory_GetItem",
		"Inventory_HasItem",

		"MakeRandomFloat",

		"Mutex_lock",
		"Timer_Check",

		"WorldConnection_Process"
};
#endif

#ifdef ZONE_PROFILE
const char *__zp_names[ZoneProfiler::MaxZoneProfilerId] = {
		"Client_Process = 0",
		"Client_HandlePacket",
		"Client_QueuePacket",
		"Client_Save",
		"Client_Attack",
		"Client_CalcBonuses",
		"Client_GetFactionLevel",
		"Client_SetFactionLevel",
		"Client_SetFactionLevel2",

		"NPC_Attack",
		"NPC_GetReverseFactionCon",
		"NPC_Process",
		"NPC_Death",
#ifdef EQBOTS
		"NPC_BotAttackMelee",
#endif

		"EntityList_TrapProcess",
		"EntityList_GroupProcess",
		"EntityList_QueueToGroupsForNPCHealthAA",
		"EntityList_DoorProcess",
		"EntityList_ObjectProcess",
		"EntityList_CorpseProcess",
		"EntityList_MobProcess",
		"EntityList_BeaconProcess",
		"EntityList_Process",
		"EntityList_RaidProcess",
		"EntityList_AICheckCloseAggro",
		"EntityList_AICheckCloseBeneficialSpells",
		"EntityList_CheckClientAggro",
		"EntityList_CheckClientAggro_Loop",
		"EntityList_AIYellForHelp",
#ifdef EQBOTS
		"EntityList_Bot_AICheckCloseBeneficialSpells",
#endif

		"HateList_Find",
		"HateList_GetDamageTop",
		"HateList_GetClosest",
		"HateList_DoFactionHits",
		"HateList_GetTop",
		"HateList_IsEmpty",
		"HateList_GetMostHate",

		"Mob_CheckWillAggro",
		"Mob_CheckLosFN",
		"Mob_Dist",
		"Mob_DistNoZ",
		"Mob_DistNoRoot",
		"Mob_DistNoRootNoZ",
		"Mob_AICastSpell",
		"Mob_SpellEffect",
		"Mob_DoBuffTic",
		"Mob_CastSpell",
		"Mob_DoCastSpell",
		"Mob_CastedSpellFinished",
		"Mob_SpellFinished",
		"Mob_IsImmuneToSpell",
		"Mob_CalculateNewPosition",
		"Mob_CalculateNewPosition2",
		"Mob_GetWeaponDamageA",
		"Mob_GetWeaponDamage",
		"Mob_GetWeaponDamageBonus",
		"Mob_TryWeaponProcA",
		"Mob_TryWeaponProcB",
#ifdef EQBOTS
		"NPC_Bot_AICastSpell",
		"Bot_AI_Process_pursue_cast",
		"Bot_PET_Process_IsEngaged",
#endif

		"Map_LineIntersectsZone",
		"Map_LineIntersectsNode",
		"Map_FindBestZ",

		"Pathing_FindRoute_FromNodes",
		"Pathing_CheckTerrainPassable",
		"Pathing_FindRoute_FromVertices",
		"Pathing_UpdatePath",
		"Pathing_NoHazards",
		"Pathing_OpenDoors",
		"Pathing_VertexDistance",
		"Pathing_VertexDistanceNoRoot",

		"Mob_AI_Process",
		"Mob_AI_Process_engaged",
		"Mob_AI_Process_engaged_cast",
		"Mob_AI_Process_pursue_cast",
		"Mob_AI_Process_autocast",
		"Mob_AI_Process_scanarea",
		"Mob_AI_Process_move",
		"Mob_AI_Process_pet",
		"Mob_AI_Process_roambox",
		"Mob_AI_Process_roamer",
		"Mob_AI_Process_guard",
#ifdef EQBOTS
		"Mob_BOT_Process",
		"Mob_BOT_Process_IsEngaged",
		"Bot_AI_Process_engaged_cast",
		"NPC_Bot_AI_IdleCastCheck",
#endif

		"Database_AddLootTableToNPC",

		"Zone_Bootup",
		"Zone_Process",

		"WorldServer_Process",

		"Spawn2_Process",

		"PerlembParser_SendCommands",
		"PerlXSParser_SendCommands",

		"command_realdispatch",
		"net_main"
};
#endif

void DumpZoneProfile() {
	__DZP_timer.stop();
	time_t aclock;
	struct tm *newtime;

	time( &aclock ); /* Get time in seconds */
	newtime = localtime( &aclock ); /* Convert time to struct */
	LogFile->write(EQEMuLog::Debug, "Profiling dump at: [%02d/%02d - %02d:%02d:%02d] (%.2f ms of data)",
		newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec,
		__DZP_timer.getTotalDuration());

	vector<_DZP_Data> data;


	int r;
#ifdef COMMON_PROFILE
	//Dump common profile
	for(r = 0; r < CommonProfiler::MaxCommonProfilerId; r++) {
		data.push_back(_DZP_Data(__cp_names[r], _cp.getCount(r), _cp.getTotalDuration(r)));
	}
#endif

#ifdef ZONE_PROFILE
	//Dump zone profile
	for(r = 0; r < ZoneProfiler::MaxZoneProfilerId; r++) {
		data.push_back(_DZP_Data(__zp_names[r], _zp.getCount(r), _zp.getTotalDuration(r)));
	}
#endif

	sort(data.begin(), data.end());

	vector<_DZP_Data>::iterator cur,end;
	cur = data.begin();
	end = data.end();

	for(; cur != end; cur++) {
		if(cur->count == 0)
			continue;	//dont print empty timers.

		LogFile->write(EQEMuLog::Debug, "..%s: %llu calls, %.4fms", cur->str, (unsigned long long)cur->count, cur->dur);
	}

	LogFile->write(EQEMuLog::Debug, "End Profiling dump at: [%02d/%02d - %02d:%02d:%02d] (%.2f ms of data)",
		newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec,
		__DZP_timer.getTotalDuration());

	__DZP_timer.start();
}

void ResetZoneProfile() {
	LogFile->write(EQEMuLog::Debug, "Profiling information reset...");
#ifdef COMMON_PROFILE
	_cp.reset();
#endif
#ifdef ZONE_PROFILE
	_zp.reset();
#endif
	__DZP_timer.reset();
}

#endif //EQPROFILE

