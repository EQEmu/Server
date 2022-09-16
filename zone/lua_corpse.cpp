#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "corpse.h"
#include "lua_corpse.h"
#include "lua_client.h"
#include "lua_entity.h"

struct Lua_Corpse_Loot_List {
	std::vector<uint32> entries;
	sol::as_table_t<std::vector<uint32>> get_entries() { return sol::as_table(entries); }
};

Lua_Corpse_Loot_List Lua_Corpse::GetLootList()
{
	Lua_Safe_Call_Class(Lua_Corpse_Loot_List);
	Lua_Corpse_Loot_List ret;
	auto loot_list = self->GetLootList();

	for (auto item_id : loot_list) {
		ret.entries.push_back(item_id);
	}

	return ret;
}

void lua_register_corpse(sol::state_view &sv)
{
	auto corpse = sv.new_usertype<Lua_Corpse>("Corpse", sol::constructors<Lua_Corpse()>(), sol::base_classes,
						  sol::bases<Lua_Mob, Lua_Entity>());
	corpse["null"] = sol::readonly_property(&Lua_Corpse::Null);
	corpse["valid"] = sol::readonly_property(&Lua_Corpse::Valid);
	corpse["AddItem"] = (void(Lua_Corpse::*)(uint32, uint16, int16, uint32, uint32, uint32, uint32, uint32))&Lua_Corpse::AddItem;
	corpse["AddLooter"] = (void(Lua_Corpse::*)(Lua_Mob))&Lua_Corpse::AddLooter;
	corpse["AllowMobLoot"] = (void(Lua_Corpse::*)(Lua_Mob, uint8))&Lua_Corpse::AllowMobLoot;
	corpse["Bury"] = (void(Lua_Corpse::*)(void))&Lua_Corpse::Bury;
	corpse["CanMobLoot"] = (bool(Lua_Corpse::*)(int))&Lua_Corpse::CanMobLoot;
	corpse["CountItem"] = (uint16(Lua_Corpse::*)(uint32))&Lua_Corpse::CountItem;
	corpse["CountItems"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::CountItems;
	corpse["Delete"] = (void(Lua_Corpse::*)(void))&Lua_Corpse::Delete;
	corpse["Depop"] = (void(Lua_Corpse::*)(void))&Lua_Corpse::Depop;
	corpse["GetCharID"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetCharID;
	corpse["GetCopper"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetCopper;
	corpse["GetDBID"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetDBID;
	corpse["GetDecayTime"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetDecayTime;
	corpse["GetFirstSlotByItemID"] = (uint16(Lua_Corpse::*)(uint32))&Lua_Corpse::GetFirstSlotByItemID;
	corpse["GetGold"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetGold;
	corpse["GetItemIDBySlot"] = (uint32(Lua_Corpse::*)(uint16))&Lua_Corpse::GetItemIDBySlot;
	corpse["GetLootList"] = (Lua_Corpse_Loot_List(Lua_Corpse::*)())&Lua_Corpse::GetLootList;
	corpse["GetOwnerName"] = (const char *(Lua_Corpse::*)(void))&Lua_Corpse::GetOwnerName;
	corpse["GetPlatinum"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetPlatinum;
	corpse["GetSilver"] = (uint32(Lua_Corpse::*)(void))&Lua_Corpse::GetSilver;
	corpse["GetWornItem"] = (uint32(Lua_Corpse::*)(int16))&Lua_Corpse::GetWornItem;
	corpse["HasItem"] = (bool(Lua_Corpse::*)(uint32))&Lua_Corpse::HasItem;
	corpse["IsEmpty"] = (bool(Lua_Corpse::*)(void))&Lua_Corpse::IsEmpty;
	corpse["IsLocked"] = (bool(Lua_Corpse::*)(void))&Lua_Corpse::IsLocked;
	corpse["IsRezzed"] = (bool(Lua_Corpse::*)(void))&Lua_Corpse::IsRezzed;
	corpse["Lock"] = (void(Lua_Corpse::*)(void))&Lua_Corpse::Lock;
	corpse["RemoveCash"] = (void(Lua_Corpse::*)(void))&Lua_Corpse::RemoveCash;
	corpse["RemoveItem"] = (void(Lua_Corpse::*)(uint16))&Lua_Corpse::RemoveItem;
	corpse["RemoveItemByID"] = sol::overload((void(Lua_Corpse::*)(uint32)) & Lua_Corpse::RemoveItemByID,
						 (void(Lua_Corpse::*)(uint32, int)) & Lua_Corpse::RemoveItemByID);
	corpse["ResetLooter"] = (void(Lua_Corpse::*)(void))&Lua_Corpse::ResetLooter;
	corpse["Save"] = (bool(Lua_Corpse::*)(void))&Lua_Corpse::Save;
	corpse["SetCash"] = (void(Lua_Corpse::*)(uint32, uint32, uint32, uint32))&Lua_Corpse::SetCash;
	corpse["SetDecayTimer"] = (void(Lua_Corpse::*)(uint32))&Lua_Corpse::SetDecayTimer;
	corpse["Summon"] = (bool(Lua_Corpse::*)(Lua_Client, bool, bool))&Lua_Corpse::Summon;
	corpse["UnLock"] = (void(Lua_Corpse::*)(void))&Lua_Corpse::UnLock;
}

void lua_register_corpse_loot_list(sol::state_view &sv)
{
	auto corpse_loot_list = sv.new_usertype<Lua_Corpse_Loot_List>("CorpseLootList");
	corpse_loot_list["entries"] = sol::readonly_property(&Lua_Corpse_Loot_List::get_entries);
}

#endif
