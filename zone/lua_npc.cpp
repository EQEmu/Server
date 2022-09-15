#ifdef LUA_EQEMU
#include <sol/sol.hpp>

#include "npc.h"
#include "lua_npc.h"
#include "lua_client.h"

struct Lua_NPC_Loot_List {
	std::vector<uint32> entries;
	sol::as_table_t<std::vector<uint32>> get_entries() { return sol::as_table(entries); }
};

Lua_NPC_Loot_List Lua_NPC::GetLootList()
{
	Lua_Safe_Call_Class(Lua_NPC_Loot_List);
	Lua_NPC_Loot_List ret;
	auto loot_list = self->GetLootList();

	for (auto item_id : loot_list) {
		ret.entries.push_back(item_id);
	}

	return ret;
}

void lua_register_npc(sol::state_view &sv)
{
	auto npc = sv.new_usertype<Lua_NPC>("NPC", sol::constructors<Lua_NPC()>(), sol::base_classes,
					    sol::bases<Lua_Mob, Lua_Entity>());
	npc["AI_SetRoambox"] = sol::overload(
	    (void(Lua_NPC::*)(float, float, float, float, float)) & Lua_NPC::AI_SetRoambox,
	    (void(Lua_NPC::*)(float, float, float, float, float, uint32, uint32)) & Lua_NPC::AI_SetRoambox);
	npc["AddAISpell"] =
	    sol::overload((void(Lua_NPC::*)(int, int, int, int, int, int)) & Lua_NPC::AddAISpell,
			  (void(Lua_NPC::*)(int, int, int, int, int, int, int, int)) & Lua_NPC::AddAISpell);
	npc["AddAISpellEffect"] = (void(Lua_NPC::*)(int,int,int,int))&Lua_NPC::AddAISpellEffect;
	npc["AddCash"] = (void(Lua_NPC::*)(int,int,int,int))&Lua_NPC::AddCash;
	npc["AddItem"] = sol::overload(
	    (void(Lua_NPC::*)(int, int)) & Lua_NPC::AddItem, (void(Lua_NPC::*)(int, int, bool)) & Lua_NPC::AddItem,
	    (void(Lua_NPC::*)(int, int, bool, int)) & Lua_NPC::AddItem,
	    (void(Lua_NPC::*)(int, int, bool, int, int)) & Lua_NPC::AddItem,
	    (void(Lua_NPC::*)(int, int, bool, int, int, int)) & Lua_NPC::AddItem,
	    (void(Lua_NPC::*)(int, int, bool, int, int, int, int)) & Lua_NPC::AddItem,
	    (void(Lua_NPC::*)(int, int, bool, int, int, int, int, int)) & Lua_NPC::AddItem,
	    (void(Lua_NPC::*)(int, int, bool, int, int, int, int, int, int)) & Lua_NPC::AddItem);
	npc["AddLootTable"] = sol::overload((void(Lua_NPC::*)(int)) & Lua_NPC::AddLootTable,
					    (void(Lua_NPC::*)(void)) & Lua_NPC::AddLootTable);
	npc["AssignWaypoints"] = (void(Lua_NPC::*)(int))&Lua_NPC::AssignWaypoints;
	npc["CalculateNewWaypoint"] = (void(Lua_NPC::*)(void))&Lua_NPC::CalculateNewWaypoint;
	npc["ChangeLastName"] = (void(Lua_NPC::*)(std::string))&Lua_NPC::ChangeLastName;
	npc["CheckNPCFactionAlly"] = (int(Lua_NPC::*)(int))&Lua_NPC::CheckNPCFactionAlly;
	npc["ClearItemList"] = (void(Lua_NPC::*)(void))&Lua_NPC::ClearItemList;
	npc["ClearLastName"] = (void(Lua_NPC::*)(void))&Lua_NPC::ClearLastName;
	npc["CountItem"] = (uint16(Lua_NPC::*)(uint32))&Lua_NPC::CountItem;
	npc["CountLoot"] = (int(Lua_NPC::*)(void))&Lua_NPC::CountLoot;
	npc["DisplayWaypointInfo"] = (void(Lua_NPC::*)(Lua_Client))&Lua_NPC::DisplayWaypointInfo;
	npc["DoClassAttacks"] = (void(Lua_NPC::*)(Lua_Mob))&Lua_NPC::DoClassAttacks;
	npc["GetAccuracyRating"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetAccuracyRating;
	npc["GetAttackDelay"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetAttackDelay;
	npc["GetAttackSpeed"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetAttackSpeed;
	npc["GetAvoidanceRating"] = &Lua_NPC::GetAvoidanceRating;
	npc["GetCopper"] = (uint32(Lua_NPC::*)(void))&Lua_NPC::GetCopper;
	npc["GetFirstSlotByItemID"] = (uint16(Lua_NPC::*)(uint32))&Lua_NPC::GetFirstSlotByItemID;
	npc["GetFollowCanRun"] = (bool(Lua_NPC::*)(void))&Lua_NPC::GetFollowCanRun;
	npc["GetFollowDistance"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetFollowDistance;
	npc["GetFollowID"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetFollowID;
	npc["GetGold"] = (uint32(Lua_NPC::*)(void))&Lua_NPC::GetGold;
	npc["GetGrid"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetGrid;
	npc["GetGuardPointX"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetGuardPointX;
	npc["GetGuardPointY"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetGuardPointY;
	npc["GetGuardPointZ"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetGuardPointZ;
	npc["GetHealScale"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetHealScale;
	npc["GetItemIDBySlot"] = (uint32(Lua_NPC::*)(uint16))&Lua_NPC::GetItemIDBySlot;
	npc["GetLootList"] = (Lua_NPC_Loot_List(Lua_NPC::*)(void))&Lua_NPC::GetLootList;
	npc["GetLoottableID"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetLoottableID;
	npc["GetMaxDMG"] = (uint32(Lua_NPC::*)(void))&Lua_NPC::GetMaxDMG;
	npc["GetMaxDamage"] = (uint32(Lua_NPC::*)(int))&Lua_NPC::GetMaxDamage;
	npc["GetMaxWp"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetMaxWp;
	npc["GetMinDMG"] = (uint32(Lua_NPC::*)(void))&Lua_NPC::GetMinDMG;
	npc["GetNPCFactionID"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetNPCFactionID;
	npc["GetNPCHate"] = (int64(Lua_NPC::*)(Lua_Mob))&Lua_NPC::GetNPCHate;
	npc["GetNPCSpellsID"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetNPCSpellsID;
	npc["GetNPCStat"] = (float(Lua_NPC::*)(const char*))&Lua_NPC::GetNPCStat;
	npc["GetPetSpellID"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetPetSpellID;
	npc["GetPlatinum"] = (uint32(Lua_NPC::*)(void))&Lua_NPC::GetPlatinum;
	npc["GetPrimSkill"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetPrimSkill;
	npc["GetPrimaryFaction"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetPrimaryFaction;
	npc["GetRawAC"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetRawAC;
	npc["GetScore"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetScore;
	npc["GetSecSkill"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetSecSkill;
	npc["GetSilver"] = (uint32(Lua_NPC::*)(void))&Lua_NPC::GetSilver;
	npc["GetSlowMitigation"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetSlowMitigation;
	npc["GetSp2"] = (uint32(Lua_NPC::*)(void))&Lua_NPC::GetSp2;
	npc["GetSpawnKillCount"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetSpawnKillCount;
	npc["GetSpawnPointH"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointH;
	npc["GetSpawnPointID"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointID;
	npc["GetSpawnPointX"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointX;
	npc["GetSpawnPointY"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointY;
	npc["GetSpawnPointZ"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetSpawnPointZ;
	npc["GetSpellFocusDMG"] = (void(Lua_NPC::*)(int))&Lua_NPC::GetSpellFocusDMG;
	npc["GetSpellFocusHeal"] = (void(Lua_NPC::*)(int))&Lua_NPC::GetSpellFocusHeal;
	npc["GetSpellScale"] = (float(Lua_NPC::*)(void))&Lua_NPC::GetSpellScale;
	npc["GetSwarmOwner"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetSwarmOwner;
	npc["GetSwarmTarget"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetSwarmTarget;
	npc["GetWaypointMax"] = (int(Lua_NPC::*)(void))&Lua_NPC::GetWaypointMax;
	npc["HasAISpellEffect"] = (bool(Lua_NPC::*)(int))&Lua_NPC::HasAISpellEffect;
	npc["HasItem"] = (bool(Lua_NPC::*)(uint32))&Lua_NPC::HasItem;
	npc["IsAnimal"] = (bool(Lua_NPC::*)(void))&Lua_NPC::IsAnimal;
	npc["IsGuarding"] = (bool(Lua_NPC::*)(void))&Lua_NPC::IsGuarding;
	npc["IsOnHatelist"] = (bool(Lua_NPC::*)(Lua_Mob))&Lua_NPC::IsOnHatelist;
	npc["IsRaidTarget"] = (bool(Lua_NPC::*)(void))&Lua_NPC::IsRaidTarget;
	npc["IsRareSpawn"] = (bool(Lua_NPC::*)(void))&Lua_NPC::IsRareSpawn;
	npc["IsTaunting"] = (bool(Lua_NPC::*)(void))&Lua_NPC::IsTaunting;
	npc["MerchantCloseShop"] = (void(Lua_NPC::*)(void))&Lua_NPC::MerchantCloseShop;
	npc["MerchantOpenShop"] = (void(Lua_NPC::*)(void))&Lua_NPC::MerchantOpenShop;
	npc["ModifyNPCStat"] = (void(Lua_NPC::*)(const char*,const char*))&Lua_NPC::ModifyNPCStat;
	npc["MoveTo"] = (void(Lua_NPC::*)(float,float,float,float,bool))&Lua_NPC::MoveTo;
	npc["NextGuardPosition"] = (void(Lua_NPC::*)(void))&Lua_NPC::NextGuardPosition;
	npc["PauseWandering"] = (void(Lua_NPC::*)(int))&Lua_NPC::PauseWandering;
	npc["PickPocket"] = (void(Lua_NPC::*)(Lua_Client))&Lua_NPC::PickPocket;
	npc["RecalculateSkills"] = (void(Lua_NPC::*)(void))&Lua_NPC::RecalculateSkills;
	npc["ReloadSpells"] = (void(Lua_NPC::*)(void))&Lua_NPC::ReloadSpells;
	npc["RemoveAISpell"] = (void(Lua_NPC::*)(int))&Lua_NPC::RemoveAISpell;
	npc["RemoveAISpellEffect"] = (void(Lua_NPC::*)(int))&Lua_NPC::RemoveAISpellEffect;
	npc["RemoveCash"] = (void(Lua_NPC::*)(void))&Lua_NPC::RemoveCash;
	npc["RemoveItem"] = sol::overload((void(Lua_NPC::*)(int)) & Lua_NPC::RemoveItem,
					  (void(Lua_NPC::*)(int, int)) & Lua_NPC::RemoveItem,
					  (void(Lua_NPC::*)(int, int, int)) & Lua_NPC::RemoveItem);
	npc["ResumeWandering"] = (void(Lua_NPC::*)(void))&Lua_NPC::ResumeWandering;
	npc["SaveGuardSpot"] = sol::overload((void(Lua_NPC::*)(void)) & Lua_NPC::SaveGuardSpot,
					     (void(Lua_NPC::*)(bool)) & Lua_NPC::SaveGuardSpot,
					     (void(Lua_NPC::*)(float, float, float, float)) & Lua_NPC::SaveGuardSpot);
	npc["ScaleNPC"] = (void(Lua_NPC::*)(uint8))&Lua_NPC::ScaleNPC;
	npc["SetCopper"] = (void(Lua_NPC::*)(uint32))&Lua_NPC::SetCopper;
	npc["SetFollowCanRun"] = (void(Lua_NPC::*)(bool))&Lua_NPC::SetFollowCanRun;
	npc["SetFollowDistance"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetFollowDistance;
	npc["SetFollowID"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetFollowID;
	npc["SetGold"] = (void(Lua_NPC::*)(uint32))&Lua_NPC::SetGold;
	npc["SetGrid"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetGrid;
	npc["SetNPCFactionID"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetNPCFactionID;
	npc["SetPetSpellID"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetPetSpellID;
	npc["SetPlatinum"] = (void(Lua_NPC::*)(uint32))&Lua_NPC::SetPlatinum;
	npc["SetPrimSkill"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetPrimSkill;
	npc["SetSaveWaypoint"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetSaveWaypoint;
	npc["SetSecSkill"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetSecSkill;
	npc["SetSilver"] = (void(Lua_NPC::*)(uint32))&Lua_NPC::SetSilver;
	npc["SetSimpleRoamBox"] = sol::overload((void(Lua_NPC::*)(float)) & Lua_NPC::SetSimpleRoamBox,
						(void(Lua_NPC::*)(float, float)) & Lua_NPC::SetSimpleRoamBox,
						(void(Lua_NPC::*)(float, float, int)) & Lua_NPC::SetSimpleRoamBox);
	npc["SetSp2"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetSp2;
	npc["SetSpellFocusDMG"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetSpellFocusDMG;
	npc["SetSpellFocusHeal"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetSpellFocusHeal;
	npc["SetSwarmTarget"] = (void(Lua_NPC::*)(int))&Lua_NPC::SetSwarmTarget;
	npc["SetTaunting"] = (void(Lua_NPC::*)(bool))&Lua_NPC::SetTaunting;
	npc["SetWaypointPause"] = (void(Lua_NPC::*)(void))&Lua_NPC::SetWaypointPause;
	npc["Signal"] = (void(Lua_NPC::*)(int))&Lua_NPC::Signal;
	npc["StartSwarmTimer"] = (void(Lua_NPC::*)(uint32))&Lua_NPC::StartSwarmTimer;
	npc["StopWandering"] = (void(Lua_NPC::*)(void))&Lua_NPC::StopWandering;
	npc["UpdateWaypoint"] = (void(Lua_NPC::*)(int))&Lua_NPC::UpdateWaypoint;
}

void lua_register_npc_loot_list(sol::state_view &sv)
{
	auto npc_loot_list = sv.new_usertype<Lua_NPC_Loot_List>("NPCLootList");
	npc_loot_list["entries"] = sol::readonly_property(&Lua_NPC_Loot_List::get_entries);
}

#endif
