#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>

#include "masterentity.h"
#include "lua_inventory.h"
#include "lua_iteminst.h"
#include "lua_item.h"

Lua_ItemInst Lua_Inventory::GetItem(int slot_id) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return self->GetItem(slot_id);
}

Lua_ItemInst Lua_Inventory::GetItem(int slot_id, int bag_slot) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return self->GetItem(slot_id, bag_slot);
}

int Lua_Inventory::PutItem(int slot_id, Lua_ItemInst item) {
	Lua_Safe_Call_Int();
	EQ::ItemInstance *inst = item;
	if(!inst) {
		return 0;
	}

	return self->PutItem(slot_id, *inst);
}

int Lua_Inventory::PushCursor(Lua_ItemInst item) {
	Lua_Safe_Call_Int();
	EQ::ItemInstance *inst = item;
	if(!inst) {
		return 0;
	}

	return self->PushCursor(*inst);
}

bool Lua_Inventory::SwapItem(int source_slot, int destination_slot) {
	Lua_Safe_Call_Bool();
	EQ::InventoryProfile::SwapItemFailState fail_state = EQ::InventoryProfile::swapInvalid;
	return self->SwapItem(source_slot, destination_slot, fail_state);
}

bool Lua_Inventory::DeleteItem(int slot_id) {
	Lua_Safe_Call_Bool();
	return self->DeleteItem(slot_id);
}

bool Lua_Inventory::DeleteItem(int slot_id, int quantity) {
	Lua_Safe_Call_Bool();
	return self->DeleteItem(slot_id, quantity);
}

bool Lua_Inventory::CheckNoDrop(int slot_id) {
	Lua_Safe_Call_Bool();
	return self->CheckNoDrop(slot_id);
}

Lua_ItemInst Lua_Inventory::PopItem(int slot_id) {
	Lua_Safe_Call_Class(Lua_ItemInst);
	return Lua_ItemInst(self->PopItem(slot_id), true);
}

int Lua_Inventory::HasItem(int item_id) {
	Lua_Safe_Call_Int();
	return self->HasItem(item_id);
}

int Lua_Inventory::HasItem(int item_id, int quantity) {
	Lua_Safe_Call_Int();
	return self->HasItem(item_id, quantity);
}

int Lua_Inventory::HasItem(int item_id, int quantity, int where) {
	Lua_Safe_Call_Int();
	return self->HasItem(item_id, quantity, where);
}

bool Lua_Inventory::HasSpaceForItem(Lua_Item item, int quantity) {
	Lua_Safe_Call_Bool();
	return self->HasSpaceForItem(item, quantity);
}

int Lua_Inventory::HasItemByUse(int use) {
	Lua_Safe_Call_Int();
	return self->HasItemByUse(use);
}

int Lua_Inventory::HasItemByUse(int use, uint8 quantity) {
	Lua_Safe_Call_Int();
	return self->HasItemByUse(use, quantity);
}

int Lua_Inventory::HasItemByUse(int use, uint8 quantity, uint8 where) {
	Lua_Safe_Call_Int();
	return self->HasItemByUse(use, quantity, where);
}

int Lua_Inventory::HasItemByLoreGroup(uint32 loregroup) {
	Lua_Safe_Call_Int();
	return self->HasItemByLoreGroup(loregroup);
}

int Lua_Inventory::HasItemByLoreGroup(uint32 loregroup, int where) {
	Lua_Safe_Call_Int();
	return self->HasItemByLoreGroup(loregroup, where);
}

int Lua_Inventory::FindFreeSlot(bool for_bag, bool try_cursor) {
	Lua_Safe_Call_Int();
	return self->FindFreeSlot(for_bag, try_cursor);
}

int Lua_Inventory::FindFreeSlot(bool for_bag, bool try_cursor, int min_size) {
	Lua_Safe_Call_Int();
	return self->FindFreeSlot(for_bag, try_cursor, min_size);
}

int Lua_Inventory::FindFreeSlot(bool for_bag, bool try_cursor, int min_size, bool is_arrow) {
	Lua_Safe_Call_Int();
	return self->FindFreeSlot(for_bag, try_cursor, min_size, is_arrow);
}

int Lua_Inventory::CalcSlotId(int slot_id) {
	Lua_Safe_Call_Int();
	return self->CalcSlotId(slot_id);
}

int Lua_Inventory::CalcSlotId(int slot_id, int bag_slot) {
	Lua_Safe_Call_Int();
	return self->CalcSlotId(slot_id, bag_slot);
}

int Lua_Inventory::CalcBagIdx(int slot_id) {
	Lua_Safe_Call_Int();
	return self->CalcBagIdx(slot_id);
}

int Lua_Inventory::CalcSlotFromMaterial(int material) {
	Lua_Safe_Call_Int();
	return self->CalcSlotFromMaterial(material);
}

int Lua_Inventory::CalcMaterialFromSlot(int equipslot) {
	Lua_Safe_Call_Int();
	return self->CalcMaterialFromSlot(equipslot);
}

bool Lua_Inventory::CanItemFitInContainer(Lua_Item item, Lua_Item container) {
	Lua_Safe_Call_Bool();
	return self->CanItemFitInContainer(item, container);
}

bool Lua_Inventory::SupportsContainers(int slot_id) {
	Lua_Safe_Call_Bool();
	return self->SupportsContainers(slot_id);
}

int Lua_Inventory::GetSlotByItemInst(Lua_ItemInst inst) {
	Lua_Safe_Call_Int();
	return self->GetSlotByItemInst(inst);
}

luabind::scope lua_register_inventory() {
	return luabind::class_<Lua_Inventory>("Inventory")
		.def(luabind::constructor<>())
		.def("GetItem", (Lua_ItemInst(Lua_Inventory::*)(int))&Lua_Inventory::GetItem)
		.def("GetItem", (Lua_ItemInst(Lua_Inventory::*)(int,int))&Lua_Inventory::GetItem)
		.def("PutItem", (int(Lua_Inventory::*)(int,Lua_ItemInst))&Lua_Inventory::PutItem)
		.def("PushCursor", (int(Lua_Inventory::*)(Lua_ItemInst))&Lua_Inventory::PushCursor)
		.def("SwapItem", (bool(Lua_Inventory::*)(int,int))&Lua_Inventory::SwapItem)
		.def("DeleteItem", (bool(Lua_Inventory::*)(int))&Lua_Inventory::DeleteItem)
		.def("DeleteItem", (bool(Lua_Inventory::*)(int,int))&Lua_Inventory::DeleteItem)
		.def("CheckNoDrop", (bool(Lua_Inventory::*)(int))&Lua_Inventory::CheckNoDrop)
		.def("PopItem", (Lua_ItemInst(Lua_Inventory::*)(int))&Lua_Inventory::PopItem)
		.def("HasItem", (int(Lua_Inventory::*)(int))&Lua_Inventory::HasItem)
		.def("HasItem", (int(Lua_Inventory::*)(int,int))&Lua_Inventory::HasItem)
		.def("HasItem", (int(Lua_Inventory::*)(int,int,int))&Lua_Inventory::HasItem)
		.def("HasSpaceForItem", (bool(Lua_Inventory::*)(Lua_Item,int))&Lua_Inventory::HasSpaceForItem)
		.def("HasItemByUse", (int(Lua_Inventory::*)(int))&Lua_Inventory::HasItemByUse)
		.def("HasItemByUse", (int(Lua_Inventory::*)(int,uint8))&Lua_Inventory::HasItemByUse)
		.def("HasItemByUse", (int(Lua_Inventory::*)(int,uint8,uint8))&Lua_Inventory::HasItemByUse)
		.def("HasItemByLoreGroup", (int(Lua_Inventory::*)(uint32))&Lua_Inventory::HasItemByLoreGroup)
		.def("HasItemByLoreGroup", (int(Lua_Inventory::*)(uint32,int))&Lua_Inventory::HasItemByLoreGroup)
		.def("FindFreeSlot", (int(Lua_Inventory::*)(bool,bool))&Lua_Inventory::FindFreeSlot)
		.def("FindFreeSlot", (int(Lua_Inventory::*)(bool,bool,int))&Lua_Inventory::FindFreeSlot)
		.def("FindFreeSlot", (int(Lua_Inventory::*)(bool,bool,int,bool))&Lua_Inventory::FindFreeSlot)
		.def("CalcSlotId", (int(Lua_Inventory::*)(int))&Lua_Inventory::CalcSlotId)
		.def("CalcSlotId", (int(Lua_Inventory::*)(int,int))&Lua_Inventory::CalcSlotId)
		.def("CalcBagIdx", (int(Lua_Inventory::*)(int))&Lua_Inventory::CalcBagIdx)
		.def("CalcSlotFromMaterial", (int(Lua_Inventory::*)(int))&Lua_Inventory::CalcSlotFromMaterial)
		.def("CalcMaterialFromSlot", (int(Lua_Inventory::*)(int))&Lua_Inventory::CalcMaterialFromSlot)
		.def("CanItemFitInContainer", (bool(Lua_Inventory::*)(Lua_Item,Lua_Item))&Lua_Inventory::CanItemFitInContainer)
		.def("SupportsContainers", (bool(Lua_Inventory::*)(int))&Lua_Inventory::SupportsContainers)
		.def("GetSlotByItemInst", (int(Lua_Inventory::*)(Lua_ItemInst))&Lua_Inventory::GetSlotByItemInst);
}

#endif
