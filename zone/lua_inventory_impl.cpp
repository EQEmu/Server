#ifdef LUA_EQEMU
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

int Lua_Inventory::CountAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountAugmentEquippedByID(item_id);
}

bool Lua_Inventory::HasAugmentEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->HasAugmentEquippedByID(item_id);
}

int Lua_Inventory::CountItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Int();
	return self->CountItemEquippedByID(item_id);
}

bool Lua_Inventory::HasItemEquippedByID(uint32 item_id) {
	Lua_Safe_Call_Bool();
	return self->HasItemEquippedByID(item_id);
}
#endif
