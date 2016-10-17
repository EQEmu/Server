/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "inventory_profile.h"
#include "textures.h"
#include "eqemu_logsys.h"
//#include "classes.h"
//#include "global_define.h"
//#include "item_instance.h"
//#include "races.h"
//#include "rulesys.h"
//#include "shareddb.h"
#include "string_util.h"

#include "../common/light_source.h"

//#include <limits.h>

#include <iostream>

std::list<EQEmu::ItemInstance*> dirty_inst;


//
// class ItemInstQueue
//
ItemInstQueue::~ItemInstQueue()
{
	for (auto iter = m_list.begin(); iter != m_list.end(); ++iter) {
		safe_delete(*iter);
	}
	m_list.clear();
}

// Put item onto back of queue
void ItemInstQueue::push(EQEmu::ItemInstance* inst)
{
	m_list.push_back(inst);
}

// Put item onto front of queue
void ItemInstQueue::push_front(EQEmu::ItemInstance* inst)
{
	m_list.push_front(inst);
}

// Remove item from front of queue
EQEmu::ItemInstance* ItemInstQueue::pop()
{
	if (m_list.empty())
		return nullptr;

	EQEmu::ItemInstance* inst = m_list.front();
	m_list.pop_front();
	return inst;
}

// Remove item from back of queue
EQEmu::ItemInstance* ItemInstQueue::pop_back()
{
	if (m_list.empty())
		return nullptr;

	EQEmu::ItemInstance* inst = m_list.back();
	m_list.pop_back();
	return inst;
}

// Look at item at front of queue
EQEmu::ItemInstance* ItemInstQueue::peek_front() const
{
	return (m_list.empty()) ? nullptr : m_list.front();
}


//
// class Inventory
//
Inventory::~Inventory()
{
	for (auto iter = m_worn.begin(); iter != m_worn.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_worn.clear();

	for (auto iter = m_inv.begin(); iter != m_inv.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_inv.clear();

	for (auto iter = m_bank.begin(); iter != m_bank.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_bank.clear();

	for (auto iter = m_shbank.begin(); iter != m_shbank.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_shbank.clear();

	for (auto iter = m_trade.begin(); iter != m_trade.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_trade.clear();
}

void Inventory::CleanDirty() {
	auto iter = dirty_inst.begin();
	while (iter != dirty_inst.end()) {
		delete (*iter);
		++iter;
	}
	dirty_inst.clear();
}

void Inventory::MarkDirty(EQEmu::ItemInstance *inst) {
	if (inst) {
		dirty_inst.push_back(inst);
	}
}

// Retrieve item at specified slot; returns false if item not found
EQEmu::ItemInstance* Inventory::GetItem(int16 slot_id) const
{
	EQEmu::ItemInstance* result = nullptr;

	// Cursor
	if (slot_id == EQEmu::inventory::slotCursor) {
		// Cursor slot
		result = m_cursor.peek_front();
	}

	// Non bag slots
	else if (slot_id >= EQEmu::legacy::TRADE_BEGIN && slot_id <= EQEmu::legacy::TRADE_END) {
		result = _GetItem(m_trade, slot_id);
	}
	else if (slot_id >= EQEmu::legacy::SHARED_BANK_BEGIN && slot_id <= EQEmu::legacy::SHARED_BANK_END) {
		// Shared Bank slots
		result = _GetItem(m_shbank, slot_id);
	}
	else if (slot_id >= EQEmu::legacy::BANK_BEGIN && slot_id <= EQEmu::legacy::BANK_END) {
		// Bank slots
		result = _GetItem(m_bank, slot_id);
	}
	else if ((slot_id >= EQEmu::legacy::GENERAL_BEGIN && slot_id <= EQEmu::legacy::GENERAL_END)) {
		// Personal inventory slots
		result = _GetItem(m_inv, slot_id);
	}
	else if ((slot_id >= EQEmu::legacy::EQUIPMENT_BEGIN && slot_id <= EQEmu::legacy::EQUIPMENT_END) ||
		(slot_id >= EQEmu::legacy::TRIBUTE_BEGIN && slot_id <= EQEmu::legacy::TRIBUTE_END) || (slot_id == EQEmu::inventory::slotPowerSource)) {
		// Equippable slots (on body)
		result = _GetItem(m_worn, slot_id);
	}

	// Inner bag slots
	else if (slot_id >= EQEmu::legacy::TRADE_BAGS_BEGIN && slot_id <= EQEmu::legacy::TRADE_BAGS_END) {
		// Trade bag slots
		EQEmu::ItemInstance* inst = _GetItem(m_trade, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsClassBag()) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::legacy::SHARED_BANK_BAGS_BEGIN && slot_id <= EQEmu::legacy::SHARED_BANK_BAGS_END) {
		// Shared Bank bag slots
		EQEmu::ItemInstance* inst = _GetItem(m_shbank, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsClassBag()) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::legacy::BANK_BAGS_BEGIN && slot_id <= EQEmu::legacy::BANK_BAGS_END) {
		// Bank bag slots
		EQEmu::ItemInstance* inst = _GetItem(m_bank, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsClassBag()) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::legacy::CURSOR_BAG_BEGIN && slot_id <= EQEmu::legacy::CURSOR_BAG_END) {
		// Cursor bag slots
		EQEmu::ItemInstance* inst = m_cursor.peek_front();
		if (inst && inst->IsClassBag()) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::legacy::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::legacy::GENERAL_BAGS_END) {
		// Personal inventory bag slots
		EQEmu::ItemInstance* inst = _GetItem(m_inv, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsClassBag()) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}

	return result;
}

// Retrieve item at specified position within bag
EQEmu::ItemInstance* Inventory::GetItem(int16 slot_id, uint8 bagidx) const
{
	return GetItem(Inventory::CalcSlotId(slot_id, bagidx));
}

// Put an item snto specified slot
int16 Inventory::PutItem(int16 slot_id, const EQEmu::ItemInstance& inst)
{
	// Clean up item already in slot (if exists)
	DeleteItem(slot_id);

	if (!inst) {
		// User is effectively deleting the item
		// in the slot, why hold a null ptr in map<>?
		return slot_id;
	}

	// Delegate to internal method
	return _PutItem(slot_id, inst.Clone());
}

int16 Inventory::PushCursor(const EQEmu::ItemInstance& inst)
{
	m_cursor.push(inst.Clone());
	return EQEmu::inventory::slotCursor;
}

EQEmu::ItemInstance* Inventory::GetCursorItem()
{
	return m_cursor.peek_front();
}

// Swap items in inventory
bool Inventory::SwapItem(int16 slot_a, int16 slot_b)
{
	// Temp holding areas for a and b
	EQEmu::ItemInstance* inst_a = GetItem(slot_a);
	EQEmu::ItemInstance* inst_b = GetItem(slot_b);

	if (inst_a) { if (!inst_a->IsSlotAllowed(slot_b)) { return false; } }
	if (inst_b) { if (!inst_b->IsSlotAllowed(slot_a)) { return false; } }

	_PutItem(slot_a, inst_b); // Copy b->a
	_PutItem(slot_b, inst_a); // Copy a->b

	return true;
}

// Remove item from inventory (with memory delete)
bool Inventory::DeleteItem(int16 slot_id, uint8 quantity)
{
	// Pop item out of inventory map (or queue)
	EQEmu::ItemInstance* item_to_delete = PopItem(slot_id);

	// Determine if object should be fully deleted, or
	// just a quantity of charges of the item can be deleted
	if (item_to_delete && (quantity > 0)) {

		item_to_delete->SetCharges(item_to_delete->GetCharges() - quantity);

		// If there are no charges left on the item,
		if (item_to_delete->GetCharges() <= 0) {
			// If the item is stackable (e.g arrows), or
			// the item is not stackable, and is not a charged item, or is expendable, delete it
			if (item_to_delete->IsStackable() ||
				(!item_to_delete->IsStackable() &&
				((item_to_delete->GetItem()->MaxCharges == 0) || item_to_delete->IsExpendable()))
				) {
				// Item can now be destroyed
				Inventory::MarkDirty(item_to_delete);
				return true;
			}
		}

		// Charges still exist, or it is a charged item that is not expendable. Put back into inventory
		_PutItem(slot_id, item_to_delete);
		return false;
	}

	Inventory::MarkDirty(item_to_delete);
	return true;

}

// Checks All items in a bag for No Drop
bool Inventory::CheckNoDrop(int16 slot_id) {
	EQEmu::ItemInstance* inst = GetItem(slot_id);
	if (!inst) return false;
	if (!inst->GetItem()->NoDrop) return true;
	if (inst->GetItem()->ItemClass == 1) {
		for (uint8 i = EQEmu::inventory::containerBegin; i < EQEmu::inventory::ContainerCount; i++) {
			EQEmu::ItemInstance* bagitem = GetItem(Inventory::CalcSlotId(slot_id, i));
			if (bagitem && !bagitem->GetItem()->NoDrop)
				return true;
		}
	}
	return false;
}

// Remove item from bucket without memory delete
// Returns item pointer if full delete was successful
EQEmu::ItemInstance* Inventory::PopItem(int16 slot_id)
{
	EQEmu::ItemInstance* p = nullptr;

	if (slot_id == EQEmu::inventory::slotCursor) {
		p = m_cursor.pop();
	}
	else if ((slot_id >= EQEmu::legacy::EQUIPMENT_BEGIN && slot_id <= EQEmu::legacy::EQUIPMENT_END) || (slot_id == EQEmu::inventory::slotPowerSource)) {
		p = m_worn[slot_id];
		m_worn.erase(slot_id);
	}
	else if ((slot_id >= EQEmu::legacy::GENERAL_BEGIN && slot_id <= EQEmu::legacy::GENERAL_END)) {
		p = m_inv[slot_id];
		m_inv.erase(slot_id);
	}
	else if (slot_id >= EQEmu::legacy::TRIBUTE_BEGIN && slot_id <= EQEmu::legacy::TRIBUTE_END) {
		p = m_worn[slot_id];
		m_worn.erase(slot_id);
	}
	else if (slot_id >= EQEmu::legacy::BANK_BEGIN && slot_id <= EQEmu::legacy::BANK_END) {
		p = m_bank[slot_id];
		m_bank.erase(slot_id);
	}
	else if (slot_id >= EQEmu::legacy::SHARED_BANK_BEGIN && slot_id <= EQEmu::legacy::SHARED_BANK_END) {
		p = m_shbank[slot_id];
		m_shbank.erase(slot_id);
	}
	else if (slot_id >= EQEmu::legacy::TRADE_BEGIN && slot_id <= EQEmu::legacy::TRADE_END) {
		p = m_trade[slot_id];
		m_trade.erase(slot_id);
	}
	else {
		// Is slot inside bag?
		EQEmu::ItemInstance* baginst = GetItem(Inventory::CalcSlotId(slot_id));
		if (baginst != nullptr && baginst->IsClassBag()) {
			p = baginst->PopItem(Inventory::CalcBagIdx(slot_id));
		}
	}

	// Return pointer that needs to be deleted (or otherwise managed)
	return p;
}

bool Inventory::HasSpaceForItem(const EQEmu::ItemData *ItemToTry, int16 Quantity) {

	if (ItemToTry->Stackable) {

		for (int16 i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++) {

			EQEmu::ItemInstance* InvItem = GetItem(i);

			if (InvItem && (InvItem->GetItem()->ID == ItemToTry->ID) && (InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

				int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

				if (Quantity <= ChargeSlotsLeft)
					return true;

				Quantity -= ChargeSlotsLeft;

			}
			if (InvItem && InvItem->IsClassBag()) {

				int16 BaseSlotID = Inventory::CalcSlotId(i, EQEmu::inventory::containerBegin);
				uint8 BagSize = InvItem->GetItem()->BagSlots;
				for (uint8 BagSlot = EQEmu::inventory::containerBegin; BagSlot < BagSize; BagSlot++) {

					InvItem = GetItem(BaseSlotID + BagSlot);

					if (InvItem && (InvItem->GetItem()->ID == ItemToTry->ID) &&
						(InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

						int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

						if (Quantity <= ChargeSlotsLeft)
							return true;

						Quantity -= ChargeSlotsLeft;
					}
				}
			}
		}
	}

	for (int16 i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++) {

		EQEmu::ItemInstance* InvItem = GetItem(i);

		if (!InvItem) {

			if (!ItemToTry->Stackable) {

				if (Quantity == 1)
					return true;
				else
					Quantity--;
			}
			else {
				if (Quantity <= ItemToTry->StackSize)
					return true;
				else
					Quantity -= ItemToTry->StackSize;
			}

		}
		else if (InvItem->IsClassBag() && CanItemFitInContainer(ItemToTry, InvItem->GetItem())) {

			int16 BaseSlotID = Inventory::CalcSlotId(i, EQEmu::inventory::containerBegin);

			uint8 BagSize = InvItem->GetItem()->BagSlots;

			for (uint8 BagSlot = EQEmu::inventory::containerBegin; BagSlot<BagSize; BagSlot++) {

				InvItem = GetItem(BaseSlotID + BagSlot);

				if (!InvItem) {
					if (!ItemToTry->Stackable) {

						if (Quantity == 1)
							return true;
						else
							Quantity--;
					}
					else {
						if (Quantity <= ItemToTry->StackSize)
							return true;
						else
							Quantity -= ItemToTry->StackSize;
					}
				}
			}
		}
	}

	return false;

}

// Checks that user has at least 'quantity' number of items in a given inventory slot
// Returns first slot it was found in, or SLOT_INVALID if not found

//This function has a flaw in that it only returns the last stack that it looked at
//when quantity is greater than 1 and not all of quantity can be found in 1 stack.
int16 Inventory::HasItem(uint32 item_id, uint8 quantity, uint8 where)
{
	int16 slot_id = INVALID_INDEX;

	//Altered by Father Nitwit to support a specification of
	//where to search, with a default value to maintain compatibility

	// Check each inventory bucket
	if (where & invWhereWorn) {
		slot_id = _HasItem(m_worn, item_id, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWherePersonal) {
		slot_id = _HasItem(m_inv, item_id, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereBank) {
		slot_id = _HasItem(m_bank, item_id, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereSharedBank) {
		slot_id = _HasItem(m_shbank, item_id, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereTrading) {
		slot_id = _HasItem(m_trade, item_id, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	// Behavioral change - Limbo is no longer checked due to improper handling of return value
	if (where & invWhereCursor) {
		// Check cursor queue
		slot_id = _HasItem(m_cursor, item_id, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	return slot_id;
}

//this function has the same quantity flaw mentioned above in HasItem()
int16 Inventory::HasItemByUse(uint8 use, uint8 quantity, uint8 where)
{
	int16 slot_id = INVALID_INDEX;

	// Check each inventory bucket
	if (where & invWhereWorn) {
		slot_id = _HasItemByUse(m_worn, use, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWherePersonal) {
		slot_id = _HasItemByUse(m_inv, use, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereBank) {
		slot_id = _HasItemByUse(m_bank, use, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereSharedBank) {
		slot_id = _HasItemByUse(m_shbank, use, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereTrading) {
		slot_id = _HasItemByUse(m_trade, use, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	// Behavioral change - Limbo is no longer checked due to improper handling of return value
	if (where & invWhereCursor) {
		// Check cursor queue
		slot_id = _HasItemByUse(m_cursor, use, quantity);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	return slot_id;
}

int16 Inventory::HasItemByLoreGroup(uint32 loregroup, uint8 where)
{
	int16 slot_id = INVALID_INDEX;

	// Check each inventory bucket
	if (where & invWhereWorn) {
		slot_id = _HasItemByLoreGroup(m_worn, loregroup);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWherePersonal) {
		slot_id = _HasItemByLoreGroup(m_inv, loregroup);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereBank) {
		slot_id = _HasItemByLoreGroup(m_bank, loregroup);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereSharedBank) {
		slot_id = _HasItemByLoreGroup(m_shbank, loregroup);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	if (where & invWhereTrading) {
		slot_id = _HasItemByLoreGroup(m_trade, loregroup);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	// Behavioral change - Limbo is no longer checked due to improper handling of return value
	if (where & invWhereCursor) {
		// Check cursor queue
		slot_id = _HasItemByLoreGroup(m_cursor, loregroup);
		if (slot_id != INVALID_INDEX)
			return slot_id;
	}

	return slot_id;
}

// Locate an available inventory slot
// Returns slot_id when there's one available, else SLOT_INVALID
int16 Inventory::FindFreeSlot(bool for_bag, bool try_cursor, uint8 min_size, bool is_arrow)
{
	// Check basic inventory
	for (int16 i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++) {
		if (!GetItem(i))
			// Found available slot in personal inventory
			return i;
	}

	if (!for_bag) {
		for (int16 i = EQEmu::legacy::GENERAL_BEGIN; i <= EQEmu::legacy::GENERAL_END; i++) {
			const EQEmu::ItemInstance* inst = GetItem(i);
			if (inst && inst->IsClassBag() && inst->GetItem()->BagSize >= min_size)
			{
				if (inst->GetItem()->BagType == EQEmu::item::BagTypeQuiver && inst->GetItem()->ItemType != EQEmu::item::ItemTypeArrow)
				{
					continue;
				}

				int16 base_slot_id = Inventory::CalcSlotId(i, EQEmu::inventory::containerBegin);

				uint8 slots = inst->GetItem()->BagSlots;
				uint8 j;
				for (j = EQEmu::inventory::containerBegin; j<slots; j++) {
					if (!GetItem(base_slot_id + j)) {
						// Found available slot within bag
						return (base_slot_id + j);
					}
				}
			}
		}
	}

	if (try_cursor) {
		// Always room on cursor (it's a queue)
		// (we may wish to cap this in the future)
		return EQEmu::inventory::slotCursor;
	}

	// No available slots
	return INVALID_INDEX;
}

// This is a mix of HasSpaceForItem and FindFreeSlot..due to existing coding behavior, it was better to add a new helper function...
int16 Inventory::FindFreeSlotForTradeItem(const EQEmu::ItemInstance* inst) {
	// Do not arbitrarily use this function..it is designed for use with Client::ResetTrade() and Client::FinishTrade().
	// If you have a need, use it..but, understand it is not a compatible replacement for Inventory::FindFreeSlot().
	//
	// I'll probably implement a bitmask in the new inventory system to avoid having to adjust stack bias

	if (!inst || !inst->GetID())
		return INVALID_INDEX;

	// step 1: find room for bags (caller should really ask for slots for bags first to avoid sending them to cursor..and bag item loss)
	if (inst->IsClassBag()) {
		for (int16 free_slot = EQEmu::legacy::GENERAL_BEGIN; free_slot <= EQEmu::legacy::GENERAL_END; ++free_slot) {
			if (!m_inv[free_slot])
				return free_slot;
		}

		return EQEmu::inventory::slotCursor; // return cursor since bags do not stack and will not fit inside other bags..yet...)
	}

	// step 2: find partial room for stackables
	if (inst->IsStackable()) {
		for (int16 free_slot = EQEmu::legacy::GENERAL_BEGIN; free_slot <= EQEmu::legacy::GENERAL_END; ++free_slot) {
			const EQEmu::ItemInstance* main_inst = m_inv[free_slot];

			if (!main_inst)
				continue;

			if ((main_inst->GetID() == inst->GetID()) && (main_inst->GetCharges() < main_inst->GetItem()->StackSize))
				return free_slot;
		}

		for (int16 free_slot = EQEmu::legacy::GENERAL_BEGIN; free_slot <= EQEmu::legacy::GENERAL_END; ++free_slot) {
			const EQEmu::ItemInstance* main_inst = m_inv[free_slot];

			if (!main_inst)
				continue;

			if (main_inst->IsClassBag()) { // if item-specific containers already have bad items, we won't fix it here...
				for (uint8 free_bag_slot = EQEmu::inventory::containerBegin; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::inventory::ContainerCount); ++free_bag_slot) {
					const EQEmu::ItemInstance* sub_inst = main_inst->GetItem(free_bag_slot);

					if (!sub_inst)
						continue;

					if ((sub_inst->GetID() == inst->GetID()) && (sub_inst->GetCharges() < sub_inst->GetItem()->StackSize))
						return Inventory::CalcSlotId(free_slot, free_bag_slot);
				}
			}
		}
	}

	// step 3a: find room for container-specific items (ItemClassArrow)
	if (inst->GetItem()->ItemType == EQEmu::item::ItemTypeArrow) {
		for (int16 free_slot = EQEmu::legacy::GENERAL_BEGIN; free_slot <= EQEmu::legacy::GENERAL_END; ++free_slot) {
			const EQEmu::ItemInstance* main_inst = m_inv[free_slot];

			if (!main_inst || (main_inst->GetItem()->BagType != EQEmu::item::BagTypeQuiver) || !main_inst->IsClassBag())
				continue;

			for (uint8 free_bag_slot = EQEmu::inventory::containerBegin; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::inventory::ContainerCount); ++free_bag_slot) {
				if (!main_inst->GetItem(free_bag_slot))
					return Inventory::CalcSlotId(free_slot, free_bag_slot);
			}
		}
	}

	// step 3b: find room for container-specific items (ItemClassSmallThrowing)
	if (inst->GetItem()->ItemType == EQEmu::item::ItemTypeSmallThrowing) {
		for (int16 free_slot = EQEmu::legacy::GENERAL_BEGIN; free_slot <= EQEmu::legacy::GENERAL_END; ++free_slot) {
			const EQEmu::ItemInstance* main_inst = m_inv[free_slot];

			if (!main_inst || (main_inst->GetItem()->BagType != EQEmu::item::BagTypeBandolier) || !main_inst->IsClassBag())
				continue;

			for (uint8 free_bag_slot = EQEmu::inventory::containerBegin; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::inventory::ContainerCount); ++free_bag_slot) {
				if (!main_inst->GetItem(free_bag_slot))
					return Inventory::CalcSlotId(free_slot, free_bag_slot);
			}
		}
	}

	// step 4: just find an empty slot
	for (int16 free_slot = EQEmu::legacy::GENERAL_BEGIN; free_slot <= EQEmu::legacy::GENERAL_END; ++free_slot) {
		const EQEmu::ItemInstance* main_inst = m_inv[free_slot];

		if (!main_inst)
			return free_slot;
	}

	for (int16 free_slot = EQEmu::legacy::GENERAL_BEGIN; free_slot <= EQEmu::legacy::GENERAL_END; ++free_slot) {
		const EQEmu::ItemInstance* main_inst = m_inv[free_slot];

		if (main_inst && main_inst->IsClassBag()) {
			if ((main_inst->GetItem()->BagSize < inst->GetItem()->Size) || (main_inst->GetItem()->BagType == EQEmu::item::BagTypeBandolier) || (main_inst->GetItem()->BagType == EQEmu::item::BagTypeQuiver))
				continue;

			for (uint8 free_bag_slot = EQEmu::inventory::containerBegin; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::inventory::ContainerCount); ++free_bag_slot) {
				if (!main_inst->GetItem(free_bag_slot))
					return Inventory::CalcSlotId(free_slot, free_bag_slot);
			}
		}
	}

	//return INVALID_INDEX; // everything else pushes to the cursor
	return EQEmu::inventory::slotCursor;
}

// Opposite of below: Get parent bag slot_id from a slot inside of bag
int16 Inventory::CalcSlotId(int16 slot_id) {
	int16 parent_slot_id = INVALID_INDEX;

	// this is not a bag range... using this risks over-writing existing items
	//else if (slot_id >= EmuConstants::BANK_BEGIN && slot_id <= EmuConstants::BANK_END)
	//	parent_slot_id = EmuConstants::BANK_BEGIN + (slot_id - EmuConstants::BANK_BEGIN) / EmuConstants::ITEM_CONTAINER_SIZE;
	//else if (slot_id >= 3100 && slot_id <= 3179) should be {3031..3110}..where did this range come from!!? (verified db save range)
	
	if (slot_id >= EQEmu::legacy::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::legacy::GENERAL_BAGS_END) {
		parent_slot_id = EQEmu::legacy::GENERAL_BEGIN + (slot_id - EQEmu::legacy::GENERAL_BAGS_BEGIN) / EQEmu::inventory::ContainerCount;
	}
	else if (slot_id >= EQEmu::legacy::CURSOR_BAG_BEGIN && slot_id <= EQEmu::legacy::CURSOR_BAG_END) {
		parent_slot_id = EQEmu::inventory::slotCursor;
	}
	else if (slot_id >= EQEmu::legacy::BANK_BAGS_BEGIN && slot_id <= EQEmu::legacy::BANK_BAGS_END) {
		parent_slot_id = EQEmu::legacy::BANK_BEGIN + (slot_id - EQEmu::legacy::BANK_BAGS_BEGIN) / EQEmu::inventory::ContainerCount;
	}
	else if (slot_id >= EQEmu::legacy::SHARED_BANK_BAGS_BEGIN && slot_id <= EQEmu::legacy::SHARED_BANK_BAGS_END) {
		parent_slot_id = EQEmu::legacy::SHARED_BANK_BEGIN + (slot_id - EQEmu::legacy::SHARED_BANK_BAGS_BEGIN) / EQEmu::inventory::ContainerCount;
	}
	else if (slot_id >= EQEmu::legacy::TRADE_BAGS_BEGIN && slot_id <= EQEmu::legacy::TRADE_BAGS_END) {
		parent_slot_id = EQEmu::legacy::TRADE_BEGIN + (slot_id - EQEmu::legacy::TRADE_BAGS_BEGIN) / EQEmu::inventory::ContainerCount;
	}

	return parent_slot_id;
}

// Calculate slot_id for an item within a bag
int16 Inventory::CalcSlotId(int16 bagslot_id, uint8 bagidx) {
	if (!Inventory::SupportsContainers(bagslot_id))
		return INVALID_INDEX;

	int16 slot_id = INVALID_INDEX;

	if (bagslot_id == EQEmu::inventory::slotCursor || bagslot_id == 8000) {
		slot_id = EQEmu::legacy::CURSOR_BAG_BEGIN + bagidx;
	}
	else if (bagslot_id >= EQEmu::legacy::GENERAL_BEGIN && bagslot_id <= EQEmu::legacy::GENERAL_END) {
		slot_id = EQEmu::legacy::GENERAL_BAGS_BEGIN + (bagslot_id - EQEmu::legacy::GENERAL_BEGIN) * EQEmu::inventory::ContainerCount + bagidx;
	}
	else if (bagslot_id >= EQEmu::legacy::BANK_BEGIN && bagslot_id <= EQEmu::legacy::BANK_END) {
		slot_id = EQEmu::legacy::BANK_BAGS_BEGIN + (bagslot_id - EQEmu::legacy::BANK_BEGIN) * EQEmu::inventory::ContainerCount + bagidx;
	}
	else if (bagslot_id >= EQEmu::legacy::SHARED_BANK_BEGIN && bagslot_id <= EQEmu::legacy::SHARED_BANK_END) {
		slot_id = EQEmu::legacy::SHARED_BANK_BAGS_BEGIN + (bagslot_id - EQEmu::legacy::SHARED_BANK_BEGIN) * EQEmu::inventory::ContainerCount + bagidx;
	}
	else if (bagslot_id >= EQEmu::legacy::TRADE_BEGIN && bagslot_id <= EQEmu::legacy::TRADE_END) {
		slot_id = EQEmu::legacy::TRADE_BAGS_BEGIN + (bagslot_id - EQEmu::legacy::TRADE_BEGIN) * EQEmu::inventory::ContainerCount + bagidx;
	}

	return slot_id;
}

uint8 Inventory::CalcBagIdx(int16 slot_id) {
	uint8 index = 0;

	// this is not a bag range... using this risks over-writing existing items
	//else if (slot_id >= EmuConstants::BANK_BEGIN && slot_id <= EmuConstants::BANK_END)
	//	index = (slot_id - EmuConstants::BANK_BEGIN) % EmuConstants::ITEM_CONTAINER_SIZE;

	if (slot_id >= EQEmu::legacy::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::legacy::GENERAL_BAGS_END) {
		index = (slot_id - EQEmu::legacy::GENERAL_BAGS_BEGIN) % EQEmu::inventory::ContainerCount;
	}
	else if (slot_id >= EQEmu::legacy::CURSOR_BAG_BEGIN && slot_id <= EQEmu::legacy::CURSOR_BAG_END) {
		index = (slot_id - EQEmu::legacy::CURSOR_BAG_BEGIN); // % EQEmu::legacy::ITEM_CONTAINER_SIZE; - not needed since range is 10 slots
	}
	else if (slot_id >= EQEmu::legacy::BANK_BAGS_BEGIN && slot_id <= EQEmu::legacy::BANK_BAGS_END) {
		index = (slot_id - EQEmu::legacy::BANK_BAGS_BEGIN) % EQEmu::inventory::ContainerCount;
	}
	else if (slot_id >= EQEmu::legacy::SHARED_BANK_BAGS_BEGIN && slot_id <= EQEmu::legacy::SHARED_BANK_BAGS_END) {
		index = (slot_id - EQEmu::legacy::SHARED_BANK_BAGS_BEGIN) % EQEmu::inventory::ContainerCount;
	}
	else if (slot_id >= EQEmu::legacy::TRADE_BAGS_BEGIN && slot_id <= EQEmu::legacy::TRADE_BAGS_END) {
		index = (slot_id - EQEmu::legacy::TRADE_BAGS_BEGIN) % EQEmu::inventory::ContainerCount;
	}
	else if (slot_id >= EQEmu::legacy::WORLD_BEGIN && slot_id <= EQEmu::legacy::WORLD_END) {
		index = (slot_id - EQEmu::legacy::WORLD_BEGIN); // % EQEmu::legacy::ITEM_CONTAINER_SIZE; - not needed since range is 10 slots
	}

	return index;
}

int16 Inventory::CalcSlotFromMaterial(uint8 material)
{
	switch (material)
	{
	case EQEmu::textures::armorHead:
		return EQEmu::inventory::slotHead;
	case EQEmu::textures::armorChest:
		return EQEmu::inventory::slotChest;
	case EQEmu::textures::armorArms:
		return EQEmu::inventory::slotArms;
	case EQEmu::textures::armorWrist:
		return EQEmu::inventory::slotWrist1;	// there's 2 bracers, only one bracer material
	case EQEmu::textures::armorHands:
		return EQEmu::inventory::slotHands;
	case EQEmu::textures::armorLegs:
		return EQEmu::inventory::slotLegs;
	case EQEmu::textures::armorFeet:
		return EQEmu::inventory::slotFeet;
	case EQEmu::textures::weaponPrimary:
		return EQEmu::inventory::slotPrimary;
	case EQEmu::textures::weaponSecondary:
		return EQEmu::inventory::slotSecondary;
	default:
		return INVALID_INDEX;
	}
}

uint8 Inventory::CalcMaterialFromSlot(int16 equipslot)
{
	switch (equipslot)
	{
	case EQEmu::inventory::slotHead:
		return EQEmu::textures::armorHead;
	case EQEmu::inventory::slotChest:
		return EQEmu::textures::armorChest;
	case EQEmu::inventory::slotArms:
		return EQEmu::textures::armorArms;
	case EQEmu::inventory::slotWrist1:
	//case SLOT_BRACER02: // non-live behavior
		return EQEmu::textures::armorWrist;
	case EQEmu::inventory::slotHands:
		return EQEmu::textures::armorHands;
	case EQEmu::inventory::slotLegs:
		return EQEmu::textures::armorLegs;
	case EQEmu::inventory::slotFeet:
		return EQEmu::textures::armorFeet;
	case EQEmu::inventory::slotPrimary:
		return EQEmu::textures::weaponPrimary;
	case EQEmu::inventory::slotSecondary:
		return EQEmu::textures::weaponSecondary;
	default:
		return EQEmu::textures::materialInvalid;
	}
}

bool Inventory::CanItemFitInContainer(const EQEmu::ItemData *ItemToTry, const EQEmu::ItemData *Container) {

	if (!ItemToTry || !Container)
		return false;

	if (ItemToTry->Size > Container->BagSize)
		return false;

	if ((Container->BagType == EQEmu::item::BagTypeQuiver) && (ItemToTry->ItemType != EQEmu::item::ItemTypeArrow))
		return false;

	if ((Container->BagType == EQEmu::item::BagTypeBandolier) && (ItemToTry->ItemType != EQEmu::item::ItemTypeSmallThrowing))
		return false;

	return true;
}

bool Inventory::SupportsClickCasting(int16 slot_id)
{
	// there are a few non-potion items that identify as ItemTypePotion..so, we still need to ubiquitously include the equipment range
	if ((uint16)slot_id <= EQEmu::legacy::GENERAL_END || slot_id == EQEmu::inventory::slotPowerSource)
	{
		return true;
	}
	else if (slot_id >= EQEmu::legacy::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::legacy::GENERAL_BAGS_END)
	{
		if (EQEmu::inventory::Lookup(m_inventory_version)->AllowClickCastFromBag)
			return true;
	}

	return false;
}

bool Inventory::SupportsPotionBeltCasting(int16 slot_id)
{
	if ((uint16)slot_id <= EQEmu::legacy::GENERAL_END || slot_id == EQEmu::inventory::slotPowerSource || (slot_id >= EQEmu::legacy::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::legacy::GENERAL_BAGS_END))
		return true;

	return false;
}

// Test whether a given slot can support a container item
bool Inventory::SupportsContainers(int16 slot_id)
{
	if ((slot_id == EQEmu::inventory::slotCursor) ||
		(slot_id >= EQEmu::legacy::GENERAL_BEGIN && slot_id <= EQEmu::legacy::GENERAL_END) ||
		(slot_id >= EQEmu::legacy::BANK_BEGIN && slot_id <= EQEmu::legacy::BANK_END) ||
		(slot_id >= EQEmu::legacy::SHARED_BANK_BEGIN && slot_id <= EQEmu::legacy::SHARED_BANK_END) ||
		(slot_id >= EQEmu::legacy::TRADE_BEGIN && slot_id <= EQEmu::legacy::TRADE_END)
		) {
		return true;
	}

	return false;
}

int Inventory::GetSlotByItemInst(EQEmu::ItemInstance *inst) {
	if (!inst)
		return INVALID_INDEX;

	int i = GetSlotByItemInstCollection(m_worn, inst);
	if (i != INVALID_INDEX) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_inv, inst);
	if (i != INVALID_INDEX) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_bank, inst);
	if (i != INVALID_INDEX) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_shbank, inst);
	if (i != INVALID_INDEX) {
		return i;
	}

	i = GetSlotByItemInstCollection(m_trade, inst);
	if (i != INVALID_INDEX) {
		return i;
	}

	if (m_cursor.peek_front() == inst) {
		return EQEmu::inventory::slotCursor;
	}

	return INVALID_INDEX;
}

uint8 Inventory::FindBrightestLightType()
{
	uint8 brightest_light_type = 0;

	for (auto iter = m_worn.begin(); iter != m_worn.end(); ++iter) {
		if ((iter->first < EQEmu::legacy::EQUIPMENT_BEGIN || iter->first > EQEmu::legacy::EQUIPMENT_END) && iter->first != EQEmu::inventory::slotPowerSource) { continue; }
		if (iter->first == EQEmu::inventory::slotAmmo) { continue; }

		auto inst = iter->second;
		if (inst == nullptr) { continue; }
		auto item = inst->GetItem();
		if (item == nullptr) { continue; }

		if (EQEmu::lightsource::IsLevelGreater(item->Light, brightest_light_type))
			brightest_light_type = item->Light;
	}

	uint8 general_light_type = 0;
	for (auto iter = m_inv.begin(); iter != m_inv.end(); ++iter) {
		if (iter->first < EQEmu::legacy::GENERAL_BEGIN || iter->first > EQEmu::legacy::GENERAL_END) { continue; }

		auto inst = iter->second;
		if (inst == nullptr) { continue; }
		auto item = inst->GetItem();
		if (item == nullptr) { continue; }

		if (!item->IsClassCommon()) { continue; }
		if (item->Light < 9 || item->Light > 13) { continue; }

		if (EQEmu::lightsource::TypeToLevel(item->Light))
			general_light_type = item->Light;
	}

	if (EQEmu::lightsource::IsLevelGreater(general_light_type, brightest_light_type))
		brightest_light_type = general_light_type;

	return brightest_light_type;
}

void Inventory::dumpEntireInventory() {

	dumpWornItems();
	dumpInventory();
	dumpBankItems();
	dumpSharedBankItems();

	std::cout << std::endl;
}

void Inventory::dumpWornItems() {
	std::cout << "Worn items:" << std::endl;
	dumpItemCollection(m_worn);
}

void Inventory::dumpInventory() {
	std::cout << "Inventory items:" << std::endl;
	dumpItemCollection(m_inv);
}

void Inventory::dumpBankItems() {

	std::cout << "Bank items:" << std::endl;
	dumpItemCollection(m_bank);
}

void Inventory::dumpSharedBankItems() {

	std::cout << "Shared Bank items:" << std::endl;
	dumpItemCollection(m_shbank);
}

int Inventory::GetSlotByItemInstCollection(const std::map<int16, EQEmu::ItemInstance*> &collection, EQEmu::ItemInstance *inst) {
	for (auto iter = collection.begin(); iter != collection.end(); ++iter) {
		EQEmu::ItemInstance *t_inst = iter->second;
		if (t_inst == inst) {
			return iter->first;
		}

		if (t_inst && !t_inst->IsClassBag()) {
			for (auto b_iter = t_inst->_cbegin(); b_iter != t_inst->_cend(); ++b_iter) {
				if (b_iter->second == inst) {
					return Inventory::CalcSlotId(iter->first, b_iter->first);
				}
			}
		}
	}

	return -1;
}

void Inventory::dumpItemCollection(const std::map<int16, EQEmu::ItemInstance*> &collection)
{
	for (auto it = collection.cbegin(); it != collection.cend(); ++it) {
		auto inst = it->second;
		if (!inst || !inst->GetItem())
			continue;

		std::string slot = StringFormat("Slot %d: %s (%d)", it->first, it->second->GetItem()->Name, (inst->GetCharges() <= 0) ? 1 : inst->GetCharges());
		std::cout << slot << std::endl;

		dumpBagContents(inst, &it);
	}
}

void Inventory::dumpBagContents(EQEmu::ItemInstance *inst, std::map<int16, EQEmu::ItemInstance*>::const_iterator *it)
{
	if (!inst || !inst->IsClassBag())
		return;

	// Go through bag, if bag
	for (auto itb = inst->_cbegin(); itb != inst->_cend(); ++itb) {
		EQEmu::ItemInstance* baginst = itb->second;
		if (!baginst || !baginst->GetItem())
			continue;

		std::string subSlot = StringFormat("	Slot %d: %s (%d)", Inventory::CalcSlotId((*it)->first, itb->first),
			baginst->GetItem()->Name, (baginst->GetCharges() <= 0) ? 1 : baginst->GetCharges());
		std::cout << subSlot << std::endl;
	}

}

// Internal Method: Retrieves item within an inventory bucket
EQEmu::ItemInstance* Inventory::_GetItem(const std::map<int16, EQEmu::ItemInstance*>& bucket, int16 slot_id) const
{
	auto it = bucket.find(slot_id);
	if (it != bucket.end()) {
		return it->second;
	}

	// Not found!
	return nullptr;
}

// Internal Method: "put" item into bucket, without regard for what is currently in bucket
// Assumes item has already been allocated
int16 Inventory::_PutItem(int16 slot_id, EQEmu::ItemInstance* inst)
{
	// What happens here when we _PutItem(MainCursor)? Bad things..really bad things...
	//
	// If putting a nullptr into slot, we need to remove slot without memory delete
	if (inst == nullptr) {
		//Why do we not delete the poped item here????
		PopItem(slot_id);
		return slot_id;
	}

	int16 result = INVALID_INDEX;
	int16 parentSlot = INVALID_INDEX;

	if (slot_id == EQEmu::inventory::slotCursor) {
		// Replace current item on cursor, if exists
		m_cursor.pop(); // no memory delete, clients of this function know what they are doing
		m_cursor.push_front(inst);
		result = slot_id;
	}
	else if ((slot_id >= EQEmu::legacy::EQUIPMENT_BEGIN && slot_id <= EQEmu::legacy::EQUIPMENT_END) || (slot_id == EQEmu::inventory::slotPowerSource)) {
		m_worn[slot_id] = inst;
		result = slot_id;
	}
	else if ((slot_id >= EQEmu::legacy::GENERAL_BEGIN && slot_id <= EQEmu::legacy::GENERAL_END)) {
		m_inv[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::legacy::TRIBUTE_BEGIN && slot_id <= EQEmu::legacy::TRIBUTE_END) {
		m_worn[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::legacy::BANK_BEGIN && slot_id <= EQEmu::legacy::BANK_END) {
		m_bank[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::legacy::SHARED_BANK_BEGIN && slot_id <= EQEmu::legacy::SHARED_BANK_END) {
		m_shbank[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::legacy::TRADE_BEGIN && slot_id <= EQEmu::legacy::TRADE_END) {
		m_trade[slot_id] = inst;
		result = slot_id;
	}
	else {
		// Slot must be within a bag
		parentSlot = Inventory::CalcSlotId(slot_id);
		EQEmu::ItemInstance* baginst = GetItem(parentSlot); // Get parent bag
		if (baginst && baginst->IsClassBag())
		{
			baginst->_PutItem(Inventory::CalcBagIdx(slot_id), inst);
			result = slot_id;
		}
	}
	
	if (result == INVALID_INDEX) {
		Log.Out(Logs::General, Logs::Error, "Inventory::_PutItem: Invalid slot_id specified (%i) with parent slot id (%i)", slot_id, parentSlot);
		Inventory::MarkDirty(inst); // Slot not found, clean up
	}

	return result;
}

// Internal Method: Checks an inventory bucket for a particular item
int16 Inventory::_HasItem(std::map<int16, EQEmu::ItemInstance*>& bucket, uint32 item_id, uint8 quantity)
{
	uint32 quantity_found = 0;

	for (auto iter = bucket.begin(); iter != bucket.end(); ++iter) {
		auto inst = iter->second;
		if (inst == nullptr) { continue; }

		if (inst->GetID() == item_id) {
			quantity_found += (inst->GetCharges() <= 0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return iter->first;
		}

		for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
			if (inst->GetAugmentItemID(index) == item_id && quantity <= 1)
				return EQEmu::legacy::SLOT_AUGMENT;
		}
		
		if (!inst->IsClassBag()) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->GetID() == item_id) {
				quantity_found += (bag_inst->GetCharges() <= 0) ? 1 : bag_inst->GetCharges();
				if (quantity_found >= quantity)
					return Inventory::CalcSlotId(iter->first, bag_iter->first);
			}

			for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
				if (bag_inst->GetAugmentItemID(index) == item_id && quantity <= 1)
					return EQEmu::legacy::SLOT_AUGMENT;
			}
		}
	}

	return INVALID_INDEX;
}

// Internal Method: Checks an inventory queue type bucket for a particular item
int16 Inventory::_HasItem(ItemInstQueue& iqueue, uint32 item_id, uint8 quantity)
{
	// The downfall of this (these) queue procedure is that callers presume that when an item is
	// found, it is presented as being available on the cursor. In cases of a parity check, this
	// is sufficient. However, in cases where referential criteria is considered, this can lead
	// to unintended results. Funtionality should be observed when referencing the return value
	// of this query
	
	uint32 quantity_found = 0;

	for (auto iter = iqueue.cbegin(); iter != iqueue.cend(); ++iter) {
		auto inst = *iter;
		if (inst == nullptr) { continue; }

		if (inst->GetID() == item_id) {
			quantity_found += (inst->GetCharges() <= 0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return EQEmu::inventory::slotCursor;
		}

		for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
			if (inst->GetAugmentItemID(index) == item_id && quantity <= 1)
				return EQEmu::legacy::SLOT_AUGMENT;
		}

		if (!inst->IsClassBag()) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->GetID() == item_id) {
				quantity_found += (bag_inst->GetCharges() <= 0) ? 1 : bag_inst->GetCharges();
				if (quantity_found >= quantity)
					return Inventory::CalcSlotId(EQEmu::inventory::slotCursor, bag_iter->first);
			}

			for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
				if (bag_inst->GetAugmentItemID(index) == item_id && quantity <= 1)
					return EQEmu::legacy::SLOT_AUGMENT;
			}
		}

		// We only check the visible cursor due to lack of queue processing ability (client allows duplicate in limbo)
		break;
	}

	return INVALID_INDEX;
}

// Internal Method: Checks an inventory bucket for a particular item
int16 Inventory::_HasItemByUse(std::map<int16, EQEmu::ItemInstance*>& bucket, uint8 use, uint8 quantity)
{
	uint32 quantity_found = 0;

	for (auto iter = bucket.begin(); iter != bucket.end(); ++iter) {
		auto inst = iter->second;
		if (inst == nullptr) { continue; }

		if (inst->IsClassCommon() && inst->GetItem()->ItemType == use) {
			quantity_found += (inst->GetCharges() <= 0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return iter->first;
		}

		if (!inst->IsClassBag()) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsClassCommon() && bag_inst->GetItem()->ItemType == use) {
				quantity_found += (bag_inst->GetCharges() <= 0) ? 1 : bag_inst->GetCharges();
				if (quantity_found >= quantity)
					return Inventory::CalcSlotId(iter->first, bag_iter->first);
			}
		}
	}

	return INVALID_INDEX;
}

// Internal Method: Checks an inventory queue type bucket for a particular item
int16 Inventory::_HasItemByUse(ItemInstQueue& iqueue, uint8 use, uint8 quantity)
{
	uint32 quantity_found = 0;

	for (auto iter = iqueue.cbegin(); iter != iqueue.cend(); ++iter) {
		auto inst = *iter;
		if (inst == nullptr) { continue; }

		if (inst->IsClassCommon() && inst->GetItem()->ItemType == use) {
			quantity_found += (inst->GetCharges() <= 0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return EQEmu::inventory::slotCursor;
		}

		if (!inst->IsClassBag()) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsClassCommon() && bag_inst->GetItem()->ItemType == use) {
				quantity_found += (bag_inst->GetCharges() <= 0) ? 1 : bag_inst->GetCharges();
				if (quantity_found >= quantity)
					return Inventory::CalcSlotId(EQEmu::inventory::slotCursor, bag_iter->first);
			}
		}

		// We only check the visible cursor due to lack of queue processing ability (client allows duplicate in limbo)
		break;
	}

	return INVALID_INDEX;
}

int16 Inventory::_HasItemByLoreGroup(std::map<int16, EQEmu::ItemInstance*>& bucket, uint32 loregroup)
{
	for (auto iter = bucket.begin(); iter != bucket.end(); ++iter) {
		auto inst = iter->second;
		if (inst == nullptr) { continue; }

		if (inst->GetItem()->LoreGroup == loregroup)
			return iter->first;

		for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
			auto aug_inst = inst->GetAugment(index);
			if (aug_inst == nullptr) { continue; }

			if (aug_inst->GetItem()->LoreGroup == loregroup)
				return EQEmu::legacy::SLOT_AUGMENT;
		}

		if (!inst->IsClassBag()) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsClassCommon() && bag_inst->GetItem()->LoreGroup == loregroup)
				return Inventory::CalcSlotId(iter->first, bag_iter->first);

			for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
				auto aug_inst = bag_inst->GetAugment(index);
				if (aug_inst == nullptr) { continue; }

				if (aug_inst->GetItem()->LoreGroup == loregroup)
					return EQEmu::legacy::SLOT_AUGMENT;
			}
		}
	}

	return INVALID_INDEX;
}

// Internal Method: Checks an inventory queue type bucket for a particular item
int16 Inventory::_HasItemByLoreGroup(ItemInstQueue& iqueue, uint32 loregroup)
{
	for (auto iter = iqueue.cbegin(); iter != iqueue.cend(); ++iter) {
		auto inst = *iter;
		if (inst == nullptr) { continue; }

		if (inst->GetItem()->LoreGroup == loregroup)
			return EQEmu::inventory::slotCursor;

		for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
			auto aug_inst = inst->GetAugment(index);
			if (aug_inst == nullptr) { continue; }

			if (aug_inst->GetItem()->LoreGroup == loregroup)
				return EQEmu::legacy::SLOT_AUGMENT;
		}

		if (!inst->IsClassBag()) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsClassCommon() && bag_inst->GetItem()->LoreGroup == loregroup)
				return Inventory::CalcSlotId(EQEmu::inventory::slotCursor, bag_iter->first);

			for (int index = EQEmu::inventory::socketBegin; index < EQEmu::inventory::SocketCount; ++index) {
				auto aug_inst = bag_inst->GetAugment(index);
				if (aug_inst == nullptr) { continue; }

				if (aug_inst->GetItem()->LoreGroup == loregroup)
					return EQEmu::legacy::SLOT_AUGMENT;
			}
		}

		// We only check the visible cursor due to lack of queue processing ability (client allows duplicate in limbo)
		break;
	}
	
	return INVALID_INDEX;
}
