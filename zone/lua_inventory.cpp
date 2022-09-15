#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "masterentity.h"
#include "lua_inventory.h"
#include "lua_iteminst.h"
#include "lua_item.h"

void lua_register_inventory(sol::state_view &sv)
{
	auto inventory = sv.new_usertype<Lua_Inventory>("Inventory", sol::constructors<Lua_Inventory()>());
	inventory["CalcBagIdx"] = (int(Lua_Inventory::*)(int))&Lua_Inventory::CalcBagIdx;
	inventory["CalcMaterialFromSlot"] = (int(Lua_Inventory::*)(int))&Lua_Inventory::CalcMaterialFromSlot;
	inventory["CalcSlotFromMaterial"] = (int(Lua_Inventory::*)(int))&Lua_Inventory::CalcSlotFromMaterial;
	inventory["CalcSlotId"] = sol::overload((int(Lua_Inventory::*)(int)) & Lua_Inventory::CalcSlotId,
						(int(Lua_Inventory::*)(int, int)) & Lua_Inventory::CalcSlotId);
	inventory["CanItemFitInContainer"] = (bool(Lua_Inventory::*)(Lua_Item,Lua_Item))&Lua_Inventory::CanItemFitInContainer;
	inventory["CheckNoDrop"] = (bool(Lua_Inventory::*)(int))&Lua_Inventory::CheckNoDrop;
	inventory["CountAugmentEquippedByID"] = (int(Lua_Inventory::*)(uint32))&Lua_Inventory::CountAugmentEquippedByID;
	inventory["CountItemEquippedByID"] = (int(Lua_Inventory::*)(uint32))&Lua_Inventory::CountItemEquippedByID;
	inventory["DeleteItem"] = sol::overload((bool(Lua_Inventory::*)(int)) & Lua_Inventory::DeleteItem,
						(bool(Lua_Inventory::*)(int, int)) & Lua_Inventory::DeleteItem);
	inventory["FindFreeSlot"] =
	    sol::overload((int(Lua_Inventory::*)(bool, bool)) & Lua_Inventory::FindFreeSlot,
			  (int(Lua_Inventory::*)(bool, bool, int)) & Lua_Inventory::FindFreeSlot,
			  (int(Lua_Inventory::*)(bool, bool, int, bool)) & Lua_Inventory::FindFreeSlot);
	inventory["GetItem"] = sol::overload((Lua_ItemInst(Lua_Inventory::*)(int)) & Lua_Inventory::GetItem,
					     (Lua_ItemInst(Lua_Inventory::*)(int, int)) & Lua_Inventory::GetItem);
	inventory["GetSlotByItemInst"] = (int(Lua_Inventory::*)(Lua_ItemInst))&Lua_Inventory::GetSlotByItemInst;
	inventory["HasAugmentEquippedByID"] = (bool(Lua_Inventory::*)(uint32))&Lua_Inventory::HasAugmentEquippedByID;
	inventory["HasItem"] = sol::overload((int(Lua_Inventory::*)(int)) & Lua_Inventory::HasItem,
					     (int(Lua_Inventory::*)(int, int)) & Lua_Inventory::HasItem,
					     (int(Lua_Inventory::*)(int, int, int)) & Lua_Inventory::HasItem);
	inventory["HasItemByLoreGroup"] =
	    sol::overload((int(Lua_Inventory::*)(uint32)) & Lua_Inventory::HasItemByLoreGroup,
			  (int(Lua_Inventory::*)(uint32, int)) & Lua_Inventory::HasItemByLoreGroup);
	inventory["HasItemByUse"] =
	    sol::overload((int(Lua_Inventory::*)(int)) & Lua_Inventory::HasItemByUse,
			  (int(Lua_Inventory::*)(int, uint8)) & Lua_Inventory::HasItemByUse,
			  (int(Lua_Inventory::*)(int, uint8, uint8)) & Lua_Inventory::HasItemByUse);
	inventory["HasItemEquippedByID"] = (bool(Lua_Inventory::*)(uint32))&Lua_Inventory::HasItemEquippedByID;
	inventory["HasSpaceForItem"] = (bool(Lua_Inventory::*)(Lua_Item,int))&Lua_Inventory::HasSpaceForItem;
	inventory["PopItem"] = (Lua_ItemInst(Lua_Inventory::*)(int))&Lua_Inventory::PopItem;
	inventory["PushCursor"] = (int(Lua_Inventory::*)(Lua_ItemInst))&Lua_Inventory::PushCursor;
	inventory["PutItem"] = (int(Lua_Inventory::*)(int,Lua_ItemInst))&Lua_Inventory::PutItem;
	inventory["SupportsContainers"] = (bool(Lua_Inventory::*)(int))&Lua_Inventory::SupportsContainers;
	inventory["SwapItem"] = (bool(Lua_Inventory::*)(int,int))&Lua_Inventory::SwapItem;
}

#endif
