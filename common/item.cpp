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

#include "classes.h"
#include "global_define.h"
#include "item.h"
#include "races.h"
#include "rulesys.h"
#include "shareddb.h"
#include "string_util.h"

#include <limits.h>

#include <iostream>

std::list<ItemInst*> dirty_inst;
int32 NextItemInstSerialNumber = 1;

static inline int32 GetNextItemInstSerialNumber() {

	// The Bazaar relies on each item a client has up for Trade having a unique
	// identifier. This 'SerialNumber' is sent in Serialized item packets and
	// is used in Bazaar packets to identify the item a player is buying or inspecting.
	//
	// E.g. A trader may have 3 Five dose cloudy potions, each with a different number of remaining charges
	// up for sale with different prices.
	//
	// NextItemInstSerialNumber is the next one to hand out.
	//
	// It is very unlikely to reach 2,147,483,647. Maybe we should call abort(), rather than wrapping back to 1.
	if(NextItemInstSerialNumber >= INT_MAX)
		NextItemInstSerialNumber = 1;
	else
		NextItemInstSerialNumber++;

	return NextItemInstSerialNumber;
}


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
void ItemInstQueue::push(ItemInst* inst)
{
	m_list.push_back(inst);
}

// Put item onto front of queue
void ItemInstQueue::push_front(ItemInst* inst)
{
	m_list.push_front(inst);
}

// Remove item from front of queue
ItemInst* ItemInstQueue::pop()
{
	if (m_list.empty())
		return nullptr;

	ItemInst* inst = m_list.front();
	m_list.pop_front();
	return inst;
}

// Remove item from back of queue
ItemInst* ItemInstQueue::pop_back()
{
	if (m_list.empty())
		return nullptr;

	ItemInst* inst = m_list.back();
	m_list.pop_back();
	return inst;
}

// Look at item at front of queue
ItemInst* ItemInstQueue::peek_front() const
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

void Inventory::MarkDirty(ItemInst *inst) {
	if (inst) {
		dirty_inst.push_back(inst);
	}
}

// Retrieve item at specified slot; returns false if item not found
ItemInst* Inventory::GetItem(int16 slot_id) const
{
	ItemInst* result = nullptr;

	// Cursor
	if (slot_id == SlotCursor) {
		// Cursor slot
		result = m_cursor.peek_front();
	}

	// Non bag slots
	else if (slot_id >= EQEmu::Constants::TRADE_BEGIN && slot_id <= EQEmu::Constants::TRADE_END) {
		result = _GetItem(m_trade, slot_id);
	}
	else if (slot_id >= EQEmu::Constants::SHARED_BANK_BEGIN && slot_id <= EQEmu::Constants::SHARED_BANK_END) {
		// Shared Bank slots
		result = _GetItem(m_shbank, slot_id);
	}
	else if (slot_id >= EQEmu::Constants::BANK_BEGIN && slot_id <= EQEmu::Constants::BANK_END) {
		// Bank slots
		result = _GetItem(m_bank, slot_id);
	}
	else if ((slot_id >= EQEmu::Constants::GENERAL_BEGIN && slot_id <= EQEmu::Constants::GENERAL_END)) {
		// Personal inventory slots
		result = _GetItem(m_inv, slot_id);
	}
	else if ((slot_id >= EQEmu::Constants::EQUIPMENT_BEGIN && slot_id <= EQEmu::Constants::EQUIPMENT_END) ||
		(slot_id >= EQEmu::Constants::TRIBUTE_BEGIN && slot_id <= EQEmu::Constants::TRIBUTE_END) || (slot_id == SlotPowerSource)) {
		// Equippable slots (on body)
		result = _GetItem(m_worn, slot_id);
	}

	// Inner bag slots
	else if (slot_id >= EQEmu::Constants::TRADE_BAGS_BEGIN && slot_id <= EQEmu::Constants::TRADE_BAGS_END) {
		// Trade bag slots
		ItemInst* inst = _GetItem(m_trade, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::Constants::SHARED_BANK_BAGS_BEGIN && slot_id <= EQEmu::Constants::SHARED_BANK_BAGS_END) {
		// Shared Bank bag slots
		ItemInst* inst = _GetItem(m_shbank, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::Constants::BANK_BAGS_BEGIN && slot_id <= EQEmu::Constants::BANK_BAGS_END) {
		// Bank bag slots
		ItemInst* inst = _GetItem(m_bank, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::Constants::CURSOR_BAG_BEGIN && slot_id <= EQEmu::Constants::CURSOR_BAG_END) {
		// Cursor bag slots
		ItemInst* inst = m_cursor.peek_front();
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}
	else if (slot_id >= EQEmu::Constants::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::Constants::GENERAL_BAGS_END) {
		// Personal inventory bag slots
		ItemInst* inst = _GetItem(m_inv, Inventory::CalcSlotId(slot_id));
		if (inst && inst->IsType(ItemClassContainer)) {
			result = inst->GetItem(Inventory::CalcBagIdx(slot_id));
		}
	}

	return result;
}

// Retrieve item at specified position within bag
ItemInst* Inventory::GetItem(int16 slot_id, uint8 bagidx) const
{
	return GetItem(Inventory::CalcSlotId(slot_id, bagidx));
}

// Put an item snto specified slot
int16 Inventory::PutItem(int16 slot_id, const ItemInst& inst)
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

int16 Inventory::PushCursor(const ItemInst& inst)
{
	m_cursor.push(inst.Clone());
	return SlotCursor;
}

ItemInst* Inventory::GetCursorItem()
{
	return m_cursor.peek_front();
}

// Swap items in inventory
bool Inventory::SwapItem(int16 slot_a, int16 slot_b)
{
	// Temp holding areas for a and b
	ItemInst* inst_a = GetItem(slot_a);
	ItemInst* inst_b = GetItem(slot_b);

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
	ItemInst* item_to_delete = PopItem(slot_id);

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
	ItemInst* inst = GetItem(slot_id);
	if (!inst) return false;
	if (!inst->GetItem()->NoDrop) return true;
	if (inst->GetItem()->ItemClass == 1) {
		for (uint8 i = SUB_BEGIN; i < EQEmu::Constants::ITEM_CONTAINER_SIZE; i++) {
			ItemInst* bagitem = GetItem(Inventory::CalcSlotId(slot_id, i));
			if (bagitem && !bagitem->GetItem()->NoDrop)
				return true;
		}
	}
	return false;
}

// Remove item from bucket without memory delete
// Returns item pointer if full delete was successful
ItemInst* Inventory::PopItem(int16 slot_id)
{
	ItemInst* p = nullptr;

	if (slot_id == SlotCursor) {
		p = m_cursor.pop();
	}
	else if ((slot_id >= EQEmu::Constants::EQUIPMENT_BEGIN && slot_id <= EQEmu::Constants::EQUIPMENT_END) || (slot_id == SlotPowerSource)) {
		p = m_worn[slot_id];
		m_worn.erase(slot_id);
	}
	else if ((slot_id >= EQEmu::Constants::GENERAL_BEGIN && slot_id <= EQEmu::Constants::GENERAL_END)) {
		p = m_inv[slot_id];
		m_inv.erase(slot_id);
	}
	else if (slot_id >= EQEmu::Constants::TRIBUTE_BEGIN && slot_id <= EQEmu::Constants::TRIBUTE_END) {
		p = m_worn[slot_id];
		m_worn.erase(slot_id);
	}
	else if (slot_id >= EQEmu::Constants::BANK_BEGIN && slot_id <= EQEmu::Constants::BANK_END) {
		p = m_bank[slot_id];
		m_bank.erase(slot_id);
	}
	else if (slot_id >= EQEmu::Constants::SHARED_BANK_BEGIN && slot_id <= EQEmu::Constants::SHARED_BANK_END) {
		p = m_shbank[slot_id];
		m_shbank.erase(slot_id);
	}
	else if (slot_id >= EQEmu::Constants::TRADE_BEGIN && slot_id <= EQEmu::Constants::TRADE_END) {
		p = m_trade[slot_id];
		m_trade.erase(slot_id);
	}
	else {
		// Is slot inside bag?
		ItemInst* baginst = GetItem(Inventory::CalcSlotId(slot_id));
		if (baginst != nullptr && baginst->IsType(ItemClassContainer)) {
			p = baginst->PopItem(Inventory::CalcBagIdx(slot_id));
		}
	}

	// Return pointer that needs to be deleted (or otherwise managed)
	return p;
}

bool Inventory::HasSpaceForItem(const Item_Struct *ItemToTry, int16 Quantity) {

	if (ItemToTry->Stackable) {

		for (int16 i = EQEmu::Constants::GENERAL_BEGIN; i <= EQEmu::Constants::GENERAL_END; i++) {

			ItemInst* InvItem = GetItem(i);

			if (InvItem && (InvItem->GetItem()->ID == ItemToTry->ID) && (InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

				int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

				if (Quantity <= ChargeSlotsLeft)
					return true;

				Quantity -= ChargeSlotsLeft;

			}
			if (InvItem && InvItem->IsType(ItemClassContainer)) {

				int16 BaseSlotID = Inventory::CalcSlotId(i, SUB_BEGIN);
				uint8 BagSize = InvItem->GetItem()->BagSlots;
				for (uint8 BagSlot = SUB_BEGIN; BagSlot < BagSize; BagSlot++) {

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

	for (int16 i = EQEmu::Constants::GENERAL_BEGIN; i <= EQEmu::Constants::GENERAL_END; i++) {

		ItemInst* InvItem = GetItem(i);

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
		else if (InvItem->IsType(ItemClassContainer) && CanItemFitInContainer(ItemToTry, InvItem->GetItem())) {

			int16 BaseSlotID = Inventory::CalcSlotId(i, SUB_BEGIN);

			uint8 BagSize = InvItem->GetItem()->BagSlots;

			for (uint8 BagSlot = SUB_BEGIN; BagSlot<BagSize; BagSlot++) {

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
	for (int16 i = EQEmu::Constants::GENERAL_BEGIN; i <= EQEmu::Constants::GENERAL_END; i++) {
		if (!GetItem(i))
			// Found available slot in personal inventory
			return i;
	}

	if (!for_bag) {
		for (int16 i = EQEmu::Constants::GENERAL_BEGIN; i <= EQEmu::Constants::GENERAL_END; i++) {
			const ItemInst* inst = GetItem(i);
			if (inst && inst->IsType(ItemClassContainer) && inst->GetItem()->BagSize >= min_size)
			{
				if (inst->GetItem()->BagType == BagTypeQuiver && inst->GetItem()->ItemType != ItemTypeArrow)
				{
					continue;
				}

				int16 base_slot_id = Inventory::CalcSlotId(i, SUB_BEGIN);

				uint8 slots = inst->GetItem()->BagSlots;
				uint8 j;
				for (j = SUB_BEGIN; j<slots; j++) {
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
		return SlotCursor;
	}

	// No available slots
	return INVALID_INDEX;
}

// This is a mix of HasSpaceForItem and FindFreeSlot..due to existing coding behavior, it was better to add a new helper function...
int16 Inventory::FindFreeSlotForTradeItem(const ItemInst* inst) {
	// Do not arbitrarily use this function..it is designed for use with Client::ResetTrade() and Client::FinishTrade().
	// If you have a need, use it..but, understand it is not a compatible replacement for Inventory::FindFreeSlot().
	//
	// I'll probably implement a bitmask in the new inventory system to avoid having to adjust stack bias

	if (!inst || !inst->GetID())
		return INVALID_INDEX;

	// step 1: find room for bags (caller should really ask for slots for bags first to avoid sending them to cursor..and bag item loss)
	if (inst->IsType(ItemClassContainer)) {
		for (int16 free_slot = EQEmu::Constants::GENERAL_BEGIN; free_slot <= EQEmu::Constants::GENERAL_END; ++free_slot) {
			if (!m_inv[free_slot])
				return free_slot;
		}

		return SlotCursor; // return cursor since bags do not stack and will not fit inside other bags..yet...)
	}

	// step 2: find partial room for stackables
	if (inst->IsStackable()) {
		for (int16 free_slot = EQEmu::Constants::GENERAL_BEGIN; free_slot <= EQEmu::Constants::GENERAL_END; ++free_slot) {
			const ItemInst* main_inst = m_inv[free_slot];

			if (!main_inst)
				continue;

			if ((main_inst->GetID() == inst->GetID()) && (main_inst->GetCharges() < main_inst->GetItem()->StackSize))
				return free_slot;
		}

		for (int16 free_slot = EQEmu::Constants::GENERAL_BEGIN; free_slot <= EQEmu::Constants::GENERAL_END; ++free_slot) {
			const ItemInst* main_inst = m_inv[free_slot];

			if (!main_inst)
				continue;

			if (main_inst->IsType(ItemClassContainer)) { // if item-specific containers already have bad items, we won't fix it here...
				for (uint8 free_bag_slot = SUB_BEGIN; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::Constants::ITEM_CONTAINER_SIZE); ++free_bag_slot) {
					const ItemInst* sub_inst = main_inst->GetItem(free_bag_slot);

					if (!sub_inst)
						continue;

					if ((sub_inst->GetID() == inst->GetID()) && (sub_inst->GetCharges() < sub_inst->GetItem()->StackSize))
						return Inventory::CalcSlotId(free_slot, free_bag_slot);
				}
			}
		}
	}

	// step 3a: find room for container-specific items (ItemClassArrow)
	if (inst->GetItem()->ItemType == ItemTypeArrow) {
		for (int16 free_slot = EQEmu::Constants::GENERAL_BEGIN; free_slot <= EQEmu::Constants::GENERAL_END; ++free_slot) {
			const ItemInst* main_inst = m_inv[free_slot];

			if (!main_inst || (main_inst->GetItem()->BagType != BagTypeQuiver) || !main_inst->IsType(ItemClassContainer))
				continue;

			for (uint8 free_bag_slot = SUB_BEGIN; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::Constants::ITEM_CONTAINER_SIZE); ++free_bag_slot) {
				if (!main_inst->GetItem(free_bag_slot))
					return Inventory::CalcSlotId(free_slot, free_bag_slot);
			}
		}
	}

	// step 3b: find room for container-specific items (ItemClassSmallThrowing)
	if (inst->GetItem()->ItemType == ItemTypeSmallThrowing) {
		for (int16 free_slot = EQEmu::Constants::GENERAL_BEGIN; free_slot <= EQEmu::Constants::GENERAL_END; ++free_slot) {
			const ItemInst* main_inst = m_inv[free_slot];

			if (!main_inst || (main_inst->GetItem()->BagType != BagTypeBandolier) || !main_inst->IsType(ItemClassContainer))
				continue;

			for (uint8 free_bag_slot = SUB_BEGIN; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::Constants::ITEM_CONTAINER_SIZE); ++free_bag_slot) {
				if (!main_inst->GetItem(free_bag_slot))
					return Inventory::CalcSlotId(free_slot, free_bag_slot);
			}
		}
	}

	// step 4: just find an empty slot
	for (int16 free_slot = EQEmu::Constants::GENERAL_BEGIN; free_slot <= EQEmu::Constants::GENERAL_END; ++free_slot) {
		const ItemInst* main_inst = m_inv[free_slot];

		if (!main_inst)
			return free_slot;
	}

	for (int16 free_slot = EQEmu::Constants::GENERAL_BEGIN; free_slot <= EQEmu::Constants::GENERAL_END; ++free_slot) {
		const ItemInst* main_inst = m_inv[free_slot];

		if (main_inst && main_inst->IsType(ItemClassContainer)) {
			if ((main_inst->GetItem()->BagSize < inst->GetItem()->Size) || (main_inst->GetItem()->BagType == BagTypeBandolier) || (main_inst->GetItem()->BagType == BagTypeQuiver))
				continue;

			for (uint8 free_bag_slot = SUB_BEGIN; (free_bag_slot < main_inst->GetItem()->BagSlots) && (free_bag_slot < EQEmu::Constants::ITEM_CONTAINER_SIZE); ++free_bag_slot) {
				if (!main_inst->GetItem(free_bag_slot))
					return Inventory::CalcSlotId(free_slot, free_bag_slot);
			}
		}
	}

	//return INVALID_INDEX; // everything else pushes to the cursor
	return SlotCursor;
}

// Opposite of below: Get parent bag slot_id from a slot inside of bag
int16 Inventory::CalcSlotId(int16 slot_id) {
	int16 parent_slot_id = INVALID_INDEX;

	// this is not a bag range... using this risks over-writing existing items
	//else if (slot_id >= EmuConstants::BANK_BEGIN && slot_id <= EmuConstants::BANK_END)
	//	parent_slot_id = EmuConstants::BANK_BEGIN + (slot_id - EmuConstants::BANK_BEGIN) / EmuConstants::ITEM_CONTAINER_SIZE;
	//else if (slot_id >= 3100 && slot_id <= 3179) should be {3031..3110}..where did this range come from!!? (verified db save range)
	
	if (slot_id >= EQEmu::Constants::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::Constants::GENERAL_BAGS_END) {
		parent_slot_id = EQEmu::Constants::GENERAL_BEGIN + (slot_id - EQEmu::Constants::GENERAL_BAGS_BEGIN) / EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}
	else if (slot_id >= EQEmu::Constants::CURSOR_BAG_BEGIN && slot_id <= EQEmu::Constants::CURSOR_BAG_END) {
		parent_slot_id = SlotCursor;
	}
	else if (slot_id >= EQEmu::Constants::BANK_BAGS_BEGIN && slot_id <= EQEmu::Constants::BANK_BAGS_END) {
		parent_slot_id = EQEmu::Constants::BANK_BEGIN + (slot_id - EQEmu::Constants::BANK_BAGS_BEGIN) / EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}
	else if (slot_id >= EQEmu::Constants::SHARED_BANK_BAGS_BEGIN && slot_id <= EQEmu::Constants::SHARED_BANK_BAGS_END) {
		parent_slot_id = EQEmu::Constants::SHARED_BANK_BEGIN + (slot_id - EQEmu::Constants::SHARED_BANK_BAGS_BEGIN) / EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}
	else if (slot_id >= EQEmu::Constants::TRADE_BAGS_BEGIN && slot_id <= EQEmu::Constants::TRADE_BAGS_END) {
		parent_slot_id = EQEmu::Constants::TRADE_BEGIN + (slot_id - EQEmu::Constants::TRADE_BAGS_BEGIN) / EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}

	return parent_slot_id;
}

// Calculate slot_id for an item within a bag
int16 Inventory::CalcSlotId(int16 bagslot_id, uint8 bagidx) {
	if (!Inventory::SupportsContainers(bagslot_id))
		return INVALID_INDEX;

	int16 slot_id = INVALID_INDEX;

	if (bagslot_id == SlotCursor || bagslot_id == 8000) {
		slot_id = EQEmu::Constants::CURSOR_BAG_BEGIN + bagidx;
	}
	else if (bagslot_id >= EQEmu::Constants::GENERAL_BEGIN && bagslot_id <= EQEmu::Constants::GENERAL_END) {
		slot_id = EQEmu::Constants::GENERAL_BAGS_BEGIN + (bagslot_id - EQEmu::Constants::GENERAL_BEGIN) * EQEmu::Constants::ITEM_CONTAINER_SIZE + bagidx;
	}
	else if (bagslot_id >= EQEmu::Constants::BANK_BEGIN && bagslot_id <= EQEmu::Constants::BANK_END) {
		slot_id = EQEmu::Constants::BANK_BAGS_BEGIN + (bagslot_id - EQEmu::Constants::BANK_BEGIN) * EQEmu::Constants::ITEM_CONTAINER_SIZE + bagidx;
	}
	else if (bagslot_id >= EQEmu::Constants::SHARED_BANK_BEGIN && bagslot_id <= EQEmu::Constants::SHARED_BANK_END) {
		slot_id = EQEmu::Constants::SHARED_BANK_BAGS_BEGIN + (bagslot_id - EQEmu::Constants::SHARED_BANK_BEGIN) * EQEmu::Constants::ITEM_CONTAINER_SIZE + bagidx;
	}
	else if (bagslot_id >= EQEmu::Constants::TRADE_BEGIN && bagslot_id <= EQEmu::Constants::TRADE_END) {
		slot_id = EQEmu::Constants::TRADE_BAGS_BEGIN + (bagslot_id - EQEmu::Constants::TRADE_BEGIN) * EQEmu::Constants::ITEM_CONTAINER_SIZE + bagidx;
	}

	return slot_id;
}

uint8 Inventory::CalcBagIdx(int16 slot_id) {
	uint8 index = 0;

	// this is not a bag range... using this risks over-writing existing items
	//else if (slot_id >= EmuConstants::BANK_BEGIN && slot_id <= EmuConstants::BANK_END)
	//	index = (slot_id - EmuConstants::BANK_BEGIN) % EmuConstants::ITEM_CONTAINER_SIZE;

	if (slot_id >= EQEmu::Constants::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::Constants::GENERAL_BAGS_END) {
		index = (slot_id - EQEmu::Constants::GENERAL_BAGS_BEGIN) % EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}
	else if (slot_id >= EQEmu::Constants::CURSOR_BAG_BEGIN && slot_id <= EQEmu::Constants::CURSOR_BAG_END) {
		index = (slot_id - EQEmu::Constants::CURSOR_BAG_BEGIN); // % EQEmu::Constants::ITEM_CONTAINER_SIZE; - not needed since range is 10 slots
	}
	else if (slot_id >= EQEmu::Constants::BANK_BAGS_BEGIN && slot_id <= EQEmu::Constants::BANK_BAGS_END) {
		index = (slot_id - EQEmu::Constants::BANK_BAGS_BEGIN) % EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}
	else if (slot_id >= EQEmu::Constants::SHARED_BANK_BAGS_BEGIN && slot_id <= EQEmu::Constants::SHARED_BANK_BAGS_END) {
		index = (slot_id - EQEmu::Constants::SHARED_BANK_BAGS_BEGIN) % EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}
	else if (slot_id >= EQEmu::Constants::TRADE_BAGS_BEGIN && slot_id <= EQEmu::Constants::TRADE_BAGS_END) {
		index = (slot_id - EQEmu::Constants::TRADE_BAGS_BEGIN) % EQEmu::Constants::ITEM_CONTAINER_SIZE;
	}
	else if (slot_id >= EQEmu::Constants::WORLD_BEGIN && slot_id <= EQEmu::Constants::WORLD_END) {
		index = (slot_id - EQEmu::Constants::WORLD_BEGIN); // % EQEmu::Constants::ITEM_CONTAINER_SIZE; - not needed since range is 10 slots
	}

	return index;
}

int16 Inventory::CalcSlotFromMaterial(uint8 material)
{
	switch (material)
	{
	case MaterialHead:
		return SlotHead;
	case MaterialChest:
		return SlotChest;
	case MaterialArms:
		return SlotArms;
	case MaterialWrist:
		return SlotWrist1;	// there's 2 bracers, only one bracer material
	case MaterialHands:
		return SlotHands;
	case MaterialLegs:
		return SlotLegs;
	case MaterialFeet:
		return SlotFeet;
	case MaterialPrimary:
		return SlotPrimary;
	case MaterialSecondary:
		return SlotSecondary;
	default:
		return INVALID_INDEX;
	}
}

uint8 Inventory::CalcMaterialFromSlot(int16 equipslot)
{
	switch (equipslot)
	{
	case SlotHead:
		return MaterialHead;
	case SlotChest:
		return MaterialChest;
	case SlotArms:
		return MaterialArms;
	case SlotWrist1:
	//case SLOT_BRACER02: // non-live behavior
		return MaterialWrist;
	case SlotHands:
		return MaterialHands;
	case SlotLegs:
		return MaterialLegs;
	case SlotFeet:
		return MaterialFeet;
	case SlotPrimary:
		return MaterialPrimary;
	case SlotSecondary:
		return MaterialSecondary;
	default:
		return MaterialInvalid;
	}
}

bool Inventory::CanItemFitInContainer(const Item_Struct *ItemToTry, const Item_Struct *Container) {

	if (!ItemToTry || !Container)
		return false;

	if (ItemToTry->Size > Container->BagSize)
		return false;

	if ((Container->BagType == BagTypeQuiver) && (ItemToTry->ItemType != ItemTypeArrow))
		return false;

	if ((Container->BagType == BagTypeBandolier) && (ItemToTry->ItemType != ItemTypeSmallThrowing))
		return false;

	return true;
}

bool Inventory::SupportsClickCasting(int16 slot_id)
{
	// there are a few non-potion items that identify as ItemTypePotion..so, we still need to ubiquitously include the equipment range
	if ((uint16)slot_id <= EQEmu::Constants::GENERAL_END || slot_id == SlotPowerSource)
	{
		return true;
	}
	else if (slot_id >= EQEmu::Constants::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::Constants::GENERAL_BAGS_END)
	{
		if (EQEmu::Limits::AllowsClickCastFromBag(m_version))
			return true;
	}

	return false;
}

bool Inventory::SupportsPotionBeltCasting(int16 slot_id)
{
	if ((uint16)slot_id <= EQEmu::Constants::GENERAL_END || slot_id == SlotPowerSource || (slot_id >= EQEmu::Constants::GENERAL_BAGS_BEGIN && slot_id <= EQEmu::Constants::GENERAL_BAGS_END))
		return true;

	return false;
}

// Test whether a given slot can support a container item
bool Inventory::SupportsContainers(int16 slot_id)
{
	if ((slot_id == SlotCursor) ||
		(slot_id >= EQEmu::Constants::GENERAL_BEGIN && slot_id <= EQEmu::Constants::GENERAL_END) ||
		(slot_id >= EQEmu::Constants::BANK_BEGIN && slot_id <= EQEmu::Constants::BANK_END) ||
		(slot_id >= EQEmu::Constants::SHARED_BANK_BEGIN && slot_id <= EQEmu::Constants::SHARED_BANK_END) ||
		(slot_id >= EQEmu::Constants::TRADE_BEGIN && slot_id <= EQEmu::Constants::TRADE_END)
		) {
		return true;
	}

	return false;
}

int Inventory::GetSlotByItemInst(ItemInst *inst) {
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
		return SlotCursor;
	}

	return INVALID_INDEX;
}

uint8 Inventory::FindBrightestLightType()
{
	uint8 brightest_light_type = 0;

	for (auto iter = m_worn.begin(); iter != m_worn.end(); ++iter) {
		if ((iter->first < EQEmu::Constants::EQUIPMENT_BEGIN || iter->first > EQEmu::Constants::EQUIPMENT_END) && iter->first != SlotPowerSource) { continue; }
		if (iter->first == SlotAmmo) { continue; }

		auto inst = iter->second;
		if (inst == nullptr) { continue; }
		auto item = inst->GetItem();
		if (item == nullptr) { continue; }

		if (EQEmu::LightSource::IsLevelGreater(item->Light, brightest_light_type))
			brightest_light_type = item->Light;
	}

	uint8 general_light_type = 0;
	for (auto iter = m_inv.begin(); iter != m_inv.end(); ++iter) {
		if (iter->first < EQEmu::Constants::GENERAL_BEGIN || iter->first > EQEmu::Constants::GENERAL_END) { continue; }

		auto inst = iter->second;
		if (inst == nullptr) { continue; }
		auto item = inst->GetItem();
		if (item == nullptr) { continue; }

		if (item->ItemClass != ItemClassCommon) { continue; }
		if (item->Light < 9 || item->Light > 13) { continue; }

		if (EQEmu::LightSource::TypeToLevel(item->Light))
			general_light_type = item->Light;
	}

	if (EQEmu::LightSource::IsLevelGreater(general_light_type, brightest_light_type))
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

int Inventory::GetSlotByItemInstCollection(const std::map<int16, ItemInst*> &collection, ItemInst *inst) {
	for (auto iter = collection.begin(); iter != collection.end(); ++iter) {
		ItemInst *t_inst = iter->second;
		if (t_inst == inst) {
			return iter->first;
		}

		if (t_inst && !t_inst->IsType(ItemClassContainer)) {
			for (auto b_iter = t_inst->_cbegin(); b_iter != t_inst->_cend(); ++b_iter) {
				if (b_iter->second == inst) {
					return Inventory::CalcSlotId(iter->first, b_iter->first);
				}
			}
		}
	}

	return -1;
}

void Inventory::dumpItemCollection(const std::map<int16, ItemInst*> &collection)
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

void Inventory::dumpBagContents(ItemInst *inst, std::map<int16, ItemInst*>::const_iterator *it)
{
	if (!inst || !inst->IsType(ItemClassContainer))
		return;

	// Go through bag, if bag
	for (auto itb = inst->_cbegin(); itb != inst->_cend(); ++itb) {
		ItemInst* baginst = itb->second;
		if (!baginst || !baginst->GetItem())
			continue;

		std::string subSlot = StringFormat("	Slot %d: %s (%d)", Inventory::CalcSlotId((*it)->first, itb->first),
			baginst->GetItem()->Name, (baginst->GetCharges() <= 0) ? 1 : baginst->GetCharges());
		std::cout << subSlot << std::endl;
	}

}

// Internal Method: Retrieves item within an inventory bucket
ItemInst* Inventory::_GetItem(const std::map<int16, ItemInst*>& bucket, int16 slot_id) const
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
int16 Inventory::_PutItem(int16 slot_id, ItemInst* inst)
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

	if (slot_id == SlotCursor) {
		// Replace current item on cursor, if exists
		m_cursor.pop(); // no memory delete, clients of this function know what they are doing
		m_cursor.push_front(inst);
		result = slot_id;
	}
	else if ((slot_id >= EQEmu::Constants::EQUIPMENT_BEGIN && slot_id <= EQEmu::Constants::EQUIPMENT_END) || (slot_id == SlotPowerSource)) {
		m_worn[slot_id] = inst;
		result = slot_id;
	}
	else if ((slot_id >= EQEmu::Constants::GENERAL_BEGIN && slot_id <= EQEmu::Constants::GENERAL_END)) {
		m_inv[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::Constants::TRIBUTE_BEGIN && slot_id <= EQEmu::Constants::TRIBUTE_END) {
		m_worn[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::Constants::BANK_BEGIN && slot_id <= EQEmu::Constants::BANK_END) {
		m_bank[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::Constants::SHARED_BANK_BEGIN && slot_id <= EQEmu::Constants::SHARED_BANK_END) {
		m_shbank[slot_id] = inst;
		result = slot_id;
	}
	else if (slot_id >= EQEmu::Constants::TRADE_BEGIN && slot_id <= EQEmu::Constants::TRADE_END) {
		m_trade[slot_id] = inst;
		result = slot_id;
	}
	else {
		// Slot must be within a bag
		parentSlot = Inventory::CalcSlotId(slot_id);
		ItemInst* baginst = GetItem(parentSlot); // Get parent bag
		if (baginst && baginst->IsType(ItemClassContainer))
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
int16 Inventory::_HasItem(std::map<int16, ItemInst*>& bucket, uint32 item_id, uint8 quantity)
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

		for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
			if (inst->GetAugmentItemID(index) == item_id && quantity <= 1)
				return EQEmu::Legacy::SLOT_AUGMENT;
		}
		
		if (!inst->IsType(ItemClassContainer)) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->GetID() == item_id) {
				quantity_found += (bag_inst->GetCharges() <= 0) ? 1 : bag_inst->GetCharges();
				if (quantity_found >= quantity)
					return Inventory::CalcSlotId(iter->first, bag_iter->first);
			}

			for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
				if (bag_inst->GetAugmentItemID(index) == item_id && quantity <= 1)
					return EQEmu::Legacy::SLOT_AUGMENT;
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
				return SlotCursor;
		}

		for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
			if (inst->GetAugmentItemID(index) == item_id && quantity <= 1)
				return EQEmu::Legacy::SLOT_AUGMENT;
		}

		if (!inst->IsType(ItemClassContainer)) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->GetID() == item_id) {
				quantity_found += (bag_inst->GetCharges() <= 0) ? 1 : bag_inst->GetCharges();
				if (quantity_found >= quantity)
					return Inventory::CalcSlotId(SlotCursor, bag_iter->first);
			}

			for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
				if (bag_inst->GetAugmentItemID(index) == item_id && quantity <= 1)
					return EQEmu::Legacy::SLOT_AUGMENT;
			}
		}

		// We only check the visible cursor due to lack of queue processing ability (client allows duplicate in limbo)
		break;
	}

	return INVALID_INDEX;
}

// Internal Method: Checks an inventory bucket for a particular item
int16 Inventory::_HasItemByUse(std::map<int16, ItemInst*>& bucket, uint8 use, uint8 quantity)
{
	uint32 quantity_found = 0;

	for (auto iter = bucket.begin(); iter != bucket.end(); ++iter) {
		auto inst = iter->second;
		if (inst == nullptr) { continue; }

		if (inst->IsType(ItemClassCommon) && inst->GetItem()->ItemType == use) {
			quantity_found += (inst->GetCharges() <= 0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return iter->first;
		}

		if (!inst->IsType(ItemClassContainer)) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsType(ItemClassCommon) && bag_inst->GetItem()->ItemType == use) {
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

		if (inst->IsType(ItemClassCommon) && inst->GetItem()->ItemType == use) {
			quantity_found += (inst->GetCharges() <= 0) ? 1 : inst->GetCharges();
			if (quantity_found >= quantity)
				return SlotCursor;
		}

		if (!inst->IsType(ItemClassContainer)) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsType(ItemClassCommon) && bag_inst->GetItem()->ItemType == use) {
				quantity_found += (bag_inst->GetCharges() <= 0) ? 1 : bag_inst->GetCharges();
				if (quantity_found >= quantity)
					return Inventory::CalcSlotId(SlotCursor, bag_iter->first);
			}
		}

		// We only check the visible cursor due to lack of queue processing ability (client allows duplicate in limbo)
		break;
	}

	return INVALID_INDEX;
}

int16 Inventory::_HasItemByLoreGroup(std::map<int16, ItemInst*>& bucket, uint32 loregroup)
{
	for (auto iter = bucket.begin(); iter != bucket.end(); ++iter) {
		auto inst = iter->second;
		if (inst == nullptr) { continue; }

		if (inst->GetItem()->LoreGroup == loregroup)
			return iter->first;

		for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
			auto aug_inst = inst->GetAugment(index);
			if (aug_inst == nullptr) { continue; }

			if (aug_inst->GetItem()->LoreGroup == loregroup)
				return EQEmu::Legacy::SLOT_AUGMENT;
		}

		if (!inst->IsType(ItemClassContainer)) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsType(ItemClassCommon) && bag_inst->GetItem()->LoreGroup == loregroup)
				return Inventory::CalcSlotId(iter->first, bag_iter->first);

			for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
				auto aug_inst = bag_inst->GetAugment(index);
				if (aug_inst == nullptr) { continue; }

				if (aug_inst->GetItem()->LoreGroup == loregroup)
					return EQEmu::Legacy::SLOT_AUGMENT;
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
			return SlotCursor;

		for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
			auto aug_inst = inst->GetAugment(index);
			if (aug_inst == nullptr) { continue; }

			if (aug_inst->GetItem()->LoreGroup == loregroup)
				return EQEmu::Legacy::SLOT_AUGMENT;
		}

		if (!inst->IsType(ItemClassContainer)) { continue; }

		for (auto bag_iter = inst->_cbegin(); bag_iter != inst->_cend(); ++bag_iter) {
			auto bag_inst = bag_iter->second;
			if (bag_inst == nullptr) { continue; }

			if (bag_inst->IsType(ItemClassCommon) && bag_inst->GetItem()->LoreGroup == loregroup)
				return Inventory::CalcSlotId(SlotCursor, bag_iter->first);

			for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
				auto aug_inst = bag_inst->GetAugment(index);
				if (aug_inst == nullptr) { continue; }

				if (aug_inst->GetItem()->LoreGroup == loregroup)
					return EQEmu::Legacy::SLOT_AUGMENT;
			}
		}

		// We only check the visible cursor due to lack of queue processing ability (client allows duplicate in limbo)
		break;
	}
	
	return INVALID_INDEX;
}


//
// class ItemInst
//
ItemInst::ItemInst(const Item_Struct* item, int16 charges) {
	m_use_type = ItemInstNormal;
	if(item) {
		m_item = new Item_Struct(*item);
	} else {
		m_item = nullptr;
	}
	m_charges = charges;
	m_price = 0;
	m_attuned = false;
	m_merchantslot = 0;
	if(m_item &&m_item->ItemClass == ItemClassCommon)
		m_color = m_item->Color;
	else
		m_color = 0;
	m_merchantcount = 1;
	m_SerialNumber = GetNextItemInstSerialNumber();

	m_exp = 0;
	m_evolveLvl = 0;
	m_activated = false;
	m_scaledItem = nullptr;
	m_evolveInfo = nullptr;
	m_scaling = false;
	m_ornamenticon = 0;
	m_ornamentidfile = 0;
	m_ornament_hero_model = 0;
	m_recast_timestamp = 0;
}

ItemInst::ItemInst(SharedDatabase *db, uint32 item_id, int16 charges) {
	m_use_type = ItemInstNormal;
	m_item = db->GetItem(item_id);
	if(m_item) {
		m_item = new Item_Struct(*m_item);
	}
	else {
		m_item = nullptr;
	}

	m_charges = charges;
	m_price = 0;
	m_merchantslot = 0;
	m_attuned=false;
	if(m_item && m_item->ItemClass == ItemClassCommon)
		m_color = m_item->Color;
	else
		m_color = 0;
	m_merchantcount = 1;
	m_SerialNumber = GetNextItemInstSerialNumber();

	m_exp = 0;
	m_evolveLvl = 0;
	m_activated = false;
	m_scaledItem = nullptr;
	m_evolveInfo = nullptr;
	m_scaling = false;
	m_ornamenticon = 0;
	m_ornamentidfile = 0;
	m_ornament_hero_model = 0;
	m_recast_timestamp = 0;
}

ItemInst::ItemInst(ItemInstTypes use_type) {
	m_use_type = use_type;
	m_item = nullptr;
	m_charges = 0;
	m_price = 0;
	m_attuned = false;
	m_merchantslot = 0;
	m_color = 0;

	m_exp = 0;
	m_evolveLvl = 0;
	m_activated = false;
	m_scaledItem = nullptr;
	m_evolveInfo = nullptr;
	m_scaling = false;
	m_ornamenticon = 0;
	m_ornamentidfile = 0;
	m_ornament_hero_model = 0;
	m_recast_timestamp = 0;
}

// Make a copy of an ItemInst object
ItemInst::ItemInst(const ItemInst& copy)
{
	m_use_type=copy.m_use_type;
	if(copy.m_item)
		m_item = new Item_Struct(*copy.m_item);
	else
		m_item = nullptr;

	m_charges=copy.m_charges;
	m_price=copy.m_price;
	m_color=copy.m_color;
	m_merchantslot=copy.m_merchantslot;
	m_currentslot=copy.m_currentslot;
	m_attuned=copy.m_attuned;
	m_merchantcount=copy.m_merchantcount;
	// Copy container contents
	for (auto it = copy.m_contents.begin(); it != copy.m_contents.end(); ++it) {
		ItemInst* inst_old = it->second;
		ItemInst* inst_new = nullptr;

		if (inst_old) {
			inst_new = inst_old->Clone();
		}

		if (inst_new != nullptr) {
			m_contents[it->first] = inst_new;
		}
	}
	std::map<std::string, std::string>::const_iterator iter;
	for (iter = copy.m_custom_data.begin(); iter != copy.m_custom_data.end(); ++iter) {
		m_custom_data[iter->first] = iter->second;
	}
	m_SerialNumber = copy.m_SerialNumber;
	m_custom_data = copy.m_custom_data;
	m_timers = copy.m_timers;

	m_exp = copy.m_exp;
	m_evolveLvl = copy.m_evolveLvl;
	m_activated = copy.m_activated;
	if (copy.m_scaledItem)
		m_scaledItem = new Item_Struct(*copy.m_scaledItem);
	else
		m_scaledItem = nullptr;

	if(copy.m_evolveInfo)
		m_evolveInfo = new EvolveInfo(*copy.m_evolveInfo);
	else
		m_evolveInfo = nullptr;

	m_scaling = copy.m_scaling;
	m_ornamenticon = copy.m_ornamenticon;
	m_ornamentidfile = copy.m_ornamentidfile;
	m_ornament_hero_model = copy.m_ornament_hero_model;
	m_recast_timestamp = copy.m_recast_timestamp;
}

// Clean up container contents
ItemInst::~ItemInst()
{
	Clear();
	safe_delete(m_item);
	safe_delete(m_scaledItem);
	safe_delete(m_evolveInfo);
}

// Query item type
bool ItemInst::IsType(ItemClassTypes item_class) const
{
	// IsType(<ItemClassTypes>) does not protect against 'm_item = nullptr'
	
	// Check usage type
	if ((m_use_type == ItemInstWorldContainer) && (item_class == ItemClassContainer))
		return true;

	if (!m_item)
		return false;

	return (m_item->ItemClass == item_class);
}

// Is item stackable?
bool ItemInst::IsStackable() const
{
	if (!m_item)
		return false;
	
	return m_item->Stackable;
}

bool ItemInst::IsCharged() const
{
	if (!m_item)
		return false;
	
	if (m_item->MaxCharges > 1)
		return true;
	else
		return false;
}

// Can item be equipped?
bool ItemInst::IsEquipable(uint16 race, uint16 class_) const
{
	if (!m_item || (m_item->Slots == 0))
		return false;

	return m_item->IsEquipable(race, class_);
}

// Can equip at this slot?
bool ItemInst::IsEquipable(int16 slot_id) const
{
	if (!m_item)
		return false;

	// another "shouldn't do" fix..will be fixed in future updates (requires code and database work)
	int16 use_slot = INVALID_INDEX;
	if (slot_id == SlotPowerSource) { use_slot = SlotGeneral1; }
	if ((uint16)slot_id <= EQEmu::Constants::EQUIPMENT_END) { use_slot = slot_id; }

	if (use_slot != INVALID_INDEX) {
		if (m_item->Slots & (1 << use_slot))
			return true;
	}

	return false;
}

bool ItemInst::IsAugmentable() const
{
	if (!m_item)
		return false;

	for (int index = 0; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
		if (m_item->AugSlotType[index] != NO_ITEM)
			return true;
	}

	return false;
}

bool ItemInst::AvailableWearSlot(uint32 aug_wear_slots) const {
	// TODO: check to see if incoming 'aug_wear_slots' "switches" bit assignments like above...
	// (if wrong, would only affect MainAmmo and MainPowerSource augments)
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return false;

	int index = EQEmu::Constants::EQUIPMENT_BEGIN;
	for (; index <= SlotGeneral1; ++index) { // MainGeneral1 should be EQEmu::Constants::EQUIPMENT_END
		if (m_item->Slots & (1 << index)) {
			if (aug_wear_slots & (1 << index))
				break;
		}
	}

	return (index < 23) ? true : false;
}

int8 ItemInst::AvailableAugmentSlot(int32 augtype) const
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return INVALID_INDEX;

	int index = AUG_BEGIN;
	for (; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
		if (GetItem(index)) { continue; }
		if (augtype == -1 || (m_item->AugSlotType[index] && ((1 << (m_item->AugSlotType[index] - 1)) & augtype)))
			break;
	}

	return (index < EQEmu::Constants::ITEM_COMMON_SIZE) ? index : INVALID_INDEX;
}

bool ItemInst::IsAugmentSlotAvailable(int32 augtype, uint8 slot) const
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		 return false;

	if ((!GetItem(slot) && m_item->AugSlotVisible[slot]) && augtype == -1 || (m_item->AugSlotType[slot] && ((1 << (m_item->AugSlotType[slot] - 1)) & augtype))) {
		return true;
	}
		return false;
}

// Retrieve item inside container
ItemInst* ItemInst::GetItem(uint8 index) const
{
	auto it = m_contents.find(index);
	if (it != m_contents.end()) {
		return it->second;
	}

	return nullptr;
}

uint32 ItemInst::GetItemID(uint8 slot) const
{
	ItemInst *item = GetItem(slot);
	if (item)
		return item->GetID();

	return NO_ITEM;
}

void ItemInst::PutItem(uint8 index, const ItemInst& inst)
{
	// Clean up item already in slot (if exists)
	DeleteItem(index);

	// Delegate to internal method
	_PutItem(index, inst.Clone());
}

// Remove item inside container
void ItemInst::DeleteItem(uint8 index)
{
	ItemInst* inst = PopItem(index);
	safe_delete(inst);
}

// Remove item from container without memory delete
// Hands over memory ownership to client of this function call
ItemInst* ItemInst::PopItem(uint8 index)
{
	auto iter = m_contents.find(index);
	if (iter != m_contents.end()) {
		ItemInst* inst = iter->second;
		m_contents.erase(index);
		return inst; // Return pointer that needs to be deleted (or otherwise managed)
	}
	
	return nullptr;
}

// Remove all items from container
void ItemInst::Clear()
{
	// Destroy container contents
	for (auto iter = m_contents.begin(); iter != m_contents.end(); ++iter) {
		safe_delete(iter->second);
	}
	m_contents.clear();
}

// Remove all items from container
void ItemInst::ClearByFlags(byFlagSetting is_nodrop, byFlagSetting is_norent)
{
	// TODO: This needs work...

	// Destroy container contents
	std::map<uint8, ItemInst*>::const_iterator cur, end, del;
	cur = m_contents.begin();
	end = m_contents.end();
	for (; cur != end;) {
		ItemInst* inst = cur->second;
		if (inst == nullptr) {
			cur = m_contents.erase(cur);
			continue;
		}

		const Item_Struct* item = inst->GetItem();
		if (item == nullptr) {
			cur = m_contents.erase(cur);
			continue;
		}

		del = cur;
		++cur;

		switch (is_nodrop) {
		case byFlagSet:
			if (item->NoDrop == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
			// no 'break;' deletes 'byFlagNotSet' type - can't add at the moment because it really *breaks* the process somewhere
		case byFlagNotSet:
			if (item->NoDrop != 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		default:
			break;
		}

		switch (is_norent) {
		case byFlagSet:
			if (item->NoRent == 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
			// no 'break;' deletes 'byFlagNotSet' type - can't add at the moment because it really *breaks* the process somewhere
		case byFlagNotSet:
			if (item->NoRent != 0) {
				safe_delete(inst);
				m_contents.erase(del->first);
				continue;
			}
		default:
			break;
		}
	}
}

uint8 ItemInst::FirstOpenSlot() const
{
	if (!m_item)
		return INVALID_INDEX;

	uint8 slots = m_item->BagSlots, i;
	for (i = SUB_BEGIN; i < slots; i++) {
		if (!GetItem(i))
			break;
	}

	return (i < slots) ? i : INVALID_INDEX;
}

uint8 ItemInst::GetTotalItemCount() const
{
	uint8 item_count = 1;

	if (m_item && m_item->ItemClass != ItemClassContainer) { return item_count; }

	for (int index = SUB_BEGIN; index < m_item->BagSlots; ++index) { if (GetItem(index)) { ++item_count; } }

	return item_count;
}

bool ItemInst::IsNoneEmptyContainer()
{
	if (!m_item || m_item->ItemClass != ItemClassContainer)
		return false;

	for (int index = SUB_BEGIN; index < m_item->BagSlots; ++index) {
		if (GetItem(index))
			return true;
	}

	return false;
}

// Retrieve augment inside item
ItemInst* ItemInst::GetAugment(uint8 slot) const
{
	if (m_item && m_item->ItemClass == ItemClassCommon)
		return GetItem(slot);

	return nullptr;
}

ItemInst* ItemInst::GetOrnamentationAug(int32 ornamentationAugtype) const
{
	if (!m_item || m_item->ItemClass != ItemClassCommon) { return nullptr; }
	if (ornamentationAugtype == 0) { return nullptr; }

	for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; i++)
	{
		if (GetAugment(i) && m_item->AugSlotType[i] == ornamentationAugtype)
		{
			const char *item_IDFile = GetAugment(i)->GetItem()->IDFile;
			if (
				(strncmp(item_IDFile, "IT64", strlen(item_IDFile)) == 0
				|| strncmp(item_IDFile, "IT63", strlen(item_IDFile)) == 0)
				&& GetAugment(i)->GetItem()->HerosForgeModel == 0
				)
			{
				continue;
			}
			return this->GetAugment(i);
		}
	}

	return nullptr;
}

uint32 ItemInst::GetOrnamentHeroModel(int32 material_slot) const {
	uint32 HeroModel = 0;
	if (m_ornament_hero_model > 0)
	{
		HeroModel = m_ornament_hero_model;
		if (material_slot >= 0)
		{
			HeroModel = (m_ornament_hero_model * 100) + material_slot;
		}
	}
	return HeroModel;
}

bool ItemInst::UpdateOrnamentationInfo() {
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return false;
	
	bool ornamentSet = false;

	int32 ornamentationAugtype = RuleI(Character, OrnamentationAugmentType);
	if (GetOrnamentationAug(ornamentationAugtype))
	{
		const Item_Struct* ornamentItem;
		ornamentItem = GetOrnamentationAug(ornamentationAugtype)->GetItem();
		if (ornamentItem != nullptr)
		{
			SetOrnamentIcon(ornamentItem->Icon);
			SetOrnamentHeroModel(ornamentItem->HerosForgeModel);
			if (strlen(ornamentItem->IDFile) > 2)
			{
				SetOrnamentationIDFile(atoi(&ornamentItem->IDFile[2]));
			}
			else
			{
				SetOrnamentationIDFile(0);
			}
			ornamentSet = true;
		}
	}
	else
	{
		SetOrnamentIcon(0);
		SetOrnamentHeroModel(0);
		SetOrnamentationIDFile(0);
	}

	return ornamentSet;
}

bool ItemInst::CanTransform(const Item_Struct *ItemToTry, const Item_Struct *Container, bool AllowAll) {
	if (!ItemToTry || !Container) return false;

	if (ItemToTry->ItemType == ItemTypeArrow || strnlen(Container->CharmFile, 30) == 0)
		return false;

	if (AllowAll && strncasecmp(Container->CharmFile, "ITEMTRANSFIGSHIELD", 18) && strncasecmp(Container->CharmFile, "ITEMTransfigBow", 15)) {
		switch (ItemToTry->ItemType) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 35:
			case 45:
				return true;
		}
	}

	static std::map<std::string, int> types;
	types["itemtransfig1hp"] = 2;
	types["itemtransfig1hs"] = 0;
	types["itemtransfig2hb"] = 4;
	types["itemtransfig2hp"] = 35;
	types["itemtransfig2hs"] = 1;
	types["itemtransfigblunt"] = 3;
	types["itemtransfigbow"] = 5;
	types["itemtransfighth"] = 45;
	types["itemtransfigshield"] = 8;
	types["itemtransfigslashing"] = 0;

	auto i = types.find(MakeLowerString(Container->CharmFile));
	if (i != types.end() && i->second == ItemToTry->ItemType)
		return true;

	static std::map<std::string, int> typestwo;
	typestwo["itemtransfigblunt"] = 4;
	typestwo["itemtransfigslashing"] = 1;

	i = typestwo.find(MakeLowerString(Container->CharmFile));
	if (i != typestwo.end() && i->second == ItemToTry->ItemType)
		return true;

	return false;
}

uint32 ItemInst::GetAugmentItemID(uint8 slot) const
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return NO_ITEM;

	return GetItemID(slot);
}

// Add an augment to the item
void ItemInst::PutAugment(uint8 slot, const ItemInst& augment)
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return;
	
	PutItem(slot, augment);
}

void ItemInst::PutAugment(SharedDatabase *db, uint8 slot, uint32 item_id)
{
	if (item_id == NO_ITEM) { return; }
	if (db == nullptr) { return; /* TODO: add log message for nullptr */ }

	const ItemInst* aug = db->CreateItem(item_id);
	if (aug) {
		PutAugment(slot, *aug);
		safe_delete(aug);
	}	
}

// Remove augment from item and destroy it
void ItemInst::DeleteAugment(uint8 index)
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return;
	
	DeleteItem(index);
}

// Remove augment from item and return it
ItemInst* ItemInst::RemoveAugment(uint8 index)
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return nullptr;
	
	return PopItem(index);
}

bool ItemInst::IsAugmented()
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return false;
	
	for (int index = AUG_BEGIN; index < EQEmu::Constants::ITEM_COMMON_SIZE; ++index) {
		if (GetAugmentItemID(index))
			return true;
	}

	return false;
}

// Has attack/delay?
bool ItemInst::IsWeapon() const
{
	if (!m_item || m_item->ItemClass != ItemClassCommon)
		return false;

	if (m_item->ItemType == ItemTypeArrow && m_item->Damage != 0)
		return true;
	else
		return ((m_item->Damage != 0) && (m_item->Delay != 0));
}

bool ItemInst::IsAmmo() const
{
	if (!m_item)
		return false;

	if ((m_item->ItemType == ItemTypeArrow) ||
		(m_item->ItemType == ItemTypeLargeThrowing) ||
		(m_item->ItemType == ItemTypeSmallThrowing)
		) {
		return true;
	}

	return false;

}

const Item_Struct* ItemInst::GetItem() const
{
	if (!m_item)
		return nullptr;

	if (m_scaledItem)
		return m_scaledItem;

	return m_item;
}

const Item_Struct* ItemInst::GetUnscaledItem() const
{
	// No operator calls and defaults to nullptr
	return m_item;
}

std::string ItemInst::GetCustomDataString() const {
	std::string ret_val;
	std::map<std::string, std::string>::const_iterator iter = m_custom_data.begin();
	while (iter != m_custom_data.end()) {
		if (ret_val.length() > 0) {
			ret_val += "^";
		}
		ret_val += iter->first;
		ret_val += "^";
		ret_val += iter->second;
		++iter;

		if (ret_val.length() > 0) {
			ret_val += "^";
		}
	}
	return ret_val;
}

std::string ItemInst::GetCustomData(std::string identifier) {
	std::map<std::string, std::string>::const_iterator iter = m_custom_data.find(identifier);
	if (iter != m_custom_data.end()) {
		return iter->second;
	}

	return "";
}

void ItemInst::SetCustomData(std::string identifier, std::string value) {
	DeleteCustomData(identifier);
	m_custom_data[identifier] = value;
}

void ItemInst::SetCustomData(std::string identifier, int value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void ItemInst::SetCustomData(std::string identifier, float value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void ItemInst::SetCustomData(std::string identifier, bool value) {
	DeleteCustomData(identifier);
	std::stringstream ss;
	ss << value;
	m_custom_data[identifier] = ss.str();
}

void ItemInst::DeleteCustomData(std::string identifier) {
	std::map<std::string, std::string>::iterator iter = m_custom_data.find(identifier);
	if (iter != m_custom_data.end()) {
		m_custom_data.erase(iter);
	}
}

// Clone a type of ItemInst object
// c++ doesn't allow a polymorphic copy constructor,
// so we have to resort to a polymorphic Clone()
ItemInst* ItemInst::Clone() const
{
	// Pseudo-polymorphic copy constructor
	return new ItemInst(*this);
}

bool ItemInst::IsSlotAllowed(int16 slot_id) const {
	// 'SupportsContainers' and 'slot_id > 21' previously saw the reassigned PowerSource slot (9999 to 22) as valid
	if (!m_item) { return false; }
	else if (Inventory::SupportsContainers(slot_id)) { return true; }
	else if (m_item->Slots & (1 << slot_id)) { return true; }
	else if (slot_id == SlotPowerSource && (m_item->Slots & (1 << 22))) { return true; } // got lazy... <watch>
	else if (slot_id != SlotPowerSource && slot_id > EQEmu::Constants::EQUIPMENT_END) { return true; }
	else { return false; }
}

void ItemInst::Initialize(SharedDatabase *db) {
	// if there's no actual item, don't do anything
	if (!m_item)
		return;

	// initialize scaling items
	if (m_item->CharmFileID != 0) {
		m_scaling = true;
		ScaleItem();
	}

	// initialize evolving items
	else if ((db) && m_item->LoreGroup >= 1000 && m_item->LoreGroup != -1) {
		// not complete yet
	}
}

void ItemInst::ScaleItem() {
	if (!m_item)
		return;

	if (m_scaledItem) {
		memcpy(m_scaledItem, m_item, sizeof(Item_Struct));
	}
	else {
		m_scaledItem = new Item_Struct(*m_item);
	}

	float Mult = (float)(GetExp()) / 10000;	// scaling is determined by exp, with 10,000 being full stats

	m_scaledItem->AStr = (int8)((float)m_item->AStr*Mult);
	m_scaledItem->ASta = (int8)((float)m_item->ASta*Mult);
	m_scaledItem->AAgi = (int8)((float)m_item->AAgi*Mult);
	m_scaledItem->ADex = (int8)((float)m_item->ADex*Mult);
	m_scaledItem->AInt = (int8)((float)m_item->AInt*Mult);
	m_scaledItem->AWis = (int8)((float)m_item->AWis*Mult);
	m_scaledItem->ACha = (int8)((float)m_item->ACha*Mult);

	m_scaledItem->MR = (int8)((float)m_item->MR*Mult);
	m_scaledItem->PR = (int8)((float)m_item->PR*Mult);
	m_scaledItem->DR = (int8)((float)m_item->DR*Mult);
	m_scaledItem->CR = (int8)((float)m_item->CR*Mult);
	m_scaledItem->FR = (int8)((float)m_item->FR*Mult);

	m_scaledItem->HP = (int32)((float)m_item->HP*Mult);
	m_scaledItem->Mana = (int32)((float)m_item->Mana*Mult);
	m_scaledItem->AC = (int32)((float)m_item->AC*Mult);

	// check these..some may not need to be modified (really need to check all stats/bonuses)
	//m_scaledItem->SkillModValue = (int32)((float)m_item->SkillModValue*Mult);
	//m_scaledItem->BaneDmgAmt = (int8)((float)m_item->BaneDmgAmt*Mult);	// watch (10 entries with charmfileid)
	m_scaledItem->BardValue = (int32)((float)m_item->BardValue*Mult);		// watch (no entries with charmfileid)
	m_scaledItem->ElemDmgAmt = (uint8)((float)m_item->ElemDmgAmt*Mult);		// watch (no entries with charmfileid)
	m_scaledItem->Damage = (uint32)((float)m_item->Damage*Mult);			// watch

	m_scaledItem->CombatEffects = (int8)((float)m_item->CombatEffects*Mult);
	m_scaledItem->Shielding = (int8)((float)m_item->Shielding*Mult);
	m_scaledItem->StunResist = (int8)((float)m_item->StunResist*Mult);
	m_scaledItem->StrikeThrough = (int8)((float)m_item->StrikeThrough*Mult);
	m_scaledItem->ExtraDmgAmt = (uint32)((float)m_item->ExtraDmgAmt*Mult);
	m_scaledItem->SpellShield = (int8)((float)m_item->SpellShield*Mult);
	m_scaledItem->Avoidance = (int8)((float)m_item->Avoidance*Mult);
	m_scaledItem->Accuracy = (int8)((float)m_item->Accuracy*Mult);

	m_scaledItem->FactionAmt1 = (int32)((float)m_item->FactionAmt1*Mult);
	m_scaledItem->FactionAmt2 = (int32)((float)m_item->FactionAmt2*Mult);
	m_scaledItem->FactionAmt3 = (int32)((float)m_item->FactionAmt3*Mult);
	m_scaledItem->FactionAmt4 = (int32)((float)m_item->FactionAmt4*Mult);

	m_scaledItem->Endur = (uint32)((float)m_item->Endur*Mult);
	m_scaledItem->DotShielding = (uint32)((float)m_item->DotShielding*Mult);
	m_scaledItem->Attack = (uint32)((float)m_item->Attack*Mult);
	m_scaledItem->Regen = (uint32)((float)m_item->Regen*Mult);
	m_scaledItem->ManaRegen = (uint32)((float)m_item->ManaRegen*Mult);
	m_scaledItem->EnduranceRegen = (uint32)((float)m_item->EnduranceRegen*Mult);
	m_scaledItem->Haste = (uint32)((float)m_item->Haste*Mult);
	m_scaledItem->DamageShield = (uint32)((float)m_item->DamageShield*Mult);

	m_scaledItem->Purity = (uint32)((float)m_item->Purity*Mult);
	m_scaledItem->BackstabDmg = (uint32)((float)m_item->BackstabDmg*Mult);
	m_scaledItem->DSMitigation = (uint32)((float)m_item->DSMitigation*Mult);
	m_scaledItem->HeroicStr = (int32)((float)m_item->HeroicStr*Mult);
	m_scaledItem->HeroicInt = (int32)((float)m_item->HeroicInt*Mult);
	m_scaledItem->HeroicWis = (int32)((float)m_item->HeroicWis*Mult);
	m_scaledItem->HeroicAgi = (int32)((float)m_item->HeroicAgi*Mult);
	m_scaledItem->HeroicDex = (int32)((float)m_item->HeroicDex*Mult);
	m_scaledItem->HeroicSta = (int32)((float)m_item->HeroicSta*Mult);
	m_scaledItem->HeroicCha = (int32)((float)m_item->HeroicCha*Mult);
	m_scaledItem->HeroicMR = (int32)((float)m_item->HeroicMR*Mult);
	m_scaledItem->HeroicFR = (int32)((float)m_item->HeroicFR*Mult);
	m_scaledItem->HeroicCR = (int32)((float)m_item->HeroicCR*Mult);
	m_scaledItem->HeroicDR = (int32)((float)m_item->HeroicDR*Mult);
	m_scaledItem->HeroicPR = (int32)((float)m_item->HeroicPR*Mult);
	m_scaledItem->HeroicSVCorrup = (int32)((float)m_item->HeroicSVCorrup*Mult);
	m_scaledItem->HealAmt = (int32)((float)m_item->HealAmt*Mult);
	m_scaledItem->SpellDmg = (int32)((float)m_item->SpellDmg*Mult);
	m_scaledItem->Clairvoyance = (uint32)((float)m_item->Clairvoyance*Mult);

	m_scaledItem->CharmFileID = 0;	// this stops the client from trying to scale the item itself.
}

bool ItemInst::EvolveOnAllKills() const {
	return (m_evolveInfo && m_evolveInfo->AllKills);
}

int8 ItemInst::GetMaxEvolveLvl() const {
	if (m_evolveInfo)
		return m_evolveInfo->MaxLvl;
	else
		return 0;
}

uint32 ItemInst::GetKillsNeeded(uint8 currentlevel) {
	uint32 kills = -1;	// default to -1 (max uint32 value) because this value is usually divided by, so we don't want to ever return zero.
	if (m_evolveInfo)
		if (currentlevel != m_evolveInfo->MaxLvl)
			kills = m_evolveInfo->LvlKills[currentlevel - 1];

	if (kills == 0)
		kills = -1;

	return kills;
}

void ItemInst::SetTimer(std::string name, uint32 time) {
	Timer t(time);
	t.Start(time, false);
	m_timers[name] = t;
}

void ItemInst::StopTimer(std::string name) {
	auto iter = m_timers.find(name);
	if(iter != m_timers.end()) {
		m_timers.erase(iter);
	}
}

void ItemInst::ClearTimers() {
	m_timers.clear();
}

int ItemInst::GetItemArmorClass(bool augments) const
{
	int ac = 0;
	const auto item = GetItem();
	if (item) {
		ac = item->AC;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					ac += GetAugment(i)->GetItemArmorClass();
	}
	return ac;
}

int ItemInst::GetItemElementalDamage(int &magic, int &fire, int &cold, int &poison, int &disease, int &chromatic, int &prismatic, int &physical, int &corruption, bool augments) const
{
	const auto item = GetItem();
	if (item) {
		switch (item->ElemDmgType) {
		case RESIST_MAGIC:
			magic += item->ElemDmgAmt;
			break;
		case RESIST_FIRE:
			fire += item->ElemDmgAmt;
			break;
		case RESIST_COLD:
			cold += item->ElemDmgAmt;
			break;
		case RESIST_POISON:
			poison += item->ElemDmgAmt;
			break;
		case RESIST_DISEASE:
			disease += item->ElemDmgAmt;
			break;
		case RESIST_CHROMATIC:
			chromatic += item->ElemDmgAmt;
			break;
		case RESIST_PRISMATIC:
			prismatic += item->ElemDmgAmt;
			break;
		case RESIST_PHYSICAL:
			physical += item->ElemDmgAmt;
			break;
		case RESIST_CORRUPTION:
			corruption += item->ElemDmgAmt;
			break;
		}

		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					GetAugment(i)->GetItemElementalDamage(magic, fire, cold, poison, disease, chromatic, prismatic, physical, corruption);
	}
	return magic + fire + cold + poison + disease + chromatic + prismatic + physical + corruption;
}

int ItemInst::GetItemElementalFlag(bool augments) const
{
	int flag = 0;
	const auto item = GetItem();
	if (item) {
		flag = item->ElemDmgType;
		if (flag)
			return flag;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i) {
				if (GetAugment(i))
					flag = GetAugment(i)->GetItemElementalFlag();
				if (flag)
					return flag;
			}
		}
	}
	return flag;
}

int ItemInst::GetItemElementalDamage(bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->ElemDmgAmt;
		if (damage)
			return damage;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i) {
				if (GetAugment(i))
					damage = GetAugment(i)->GetItemElementalDamage();
				if (damage)
					return damage;
			}
		}
	}
	return damage;
}

int ItemInst::GetItemRecommendedLevel(bool augments) const
{
	int level = 0;
	const auto item = GetItem();
	if (item) {
		level = item->RecLevel;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i) {
				int temp = 0;
				if (GetAugment(i)) {
					temp = GetAugment(i)->GetItemRecommendedLevel();
					if (temp > level)
						level = temp;
				}
			}
		}
	}

	return level;
}

int ItemInst::GetItemRequiredLevel(bool augments) const
{
	int level = 0;
	const auto item = GetItem();
	if (item) {
		level = item->ReqLevel;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i) {
				int temp = 0;
				if (GetAugment(i)) {
					temp = GetAugment(i)->GetItemRequiredLevel();
					if (temp > level)
						level = temp;
				}
			}
		}
	}

	return level;
}

int ItemInst::GetItemWeaponDamage(bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->Damage;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemWeaponDamage();
		}
	}
	return damage;
}

int ItemInst::GetItemBackstabDamage(bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		damage = item->BackstabDmg;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBackstabDamage();
		}
	}
	return damage;
}

int ItemInst::GetItemBaneDamageBody(bool augments) const
{
	int body = 0;
	const auto item = GetItem();
	if (item) {
		body = item->BaneDmgBody;
		if (body)
			return body;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i)) {
					body = GetAugment(i)->GetItemBaneDamageBody();
					if (body)
						return body;
				}
		}
	}
	return body;
}

int ItemInst::GetItemBaneDamageRace(bool augments) const
{
	int race = 0;
	const auto item = GetItem();
	if (item) {
		race = item->BaneDmgRace;
		if (race)
			return race;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i)) {
					race = GetAugment(i)->GetItemBaneDamageRace();
					if (race)
						return race;
				}
		}
	}
	return race;
}

int ItemInst::GetItemBaneDamageBody(bodyType against, bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		if (item->BaneDmgBody == against)
			damage += item->BaneDmgAmt;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBaneDamageBody(against);
		}
	}
	return damage;
}

int ItemInst::GetItemBaneDamageRace(uint16 against, bool augments) const
{
	int damage = 0;
	const auto item = GetItem();
	if (item) {
		if (item->BaneDmgRace == against)
			damage += item->BaneDmgRaceAmt;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					damage += GetAugment(i)->GetItemBaneDamageRace(against);
		}
	}
	return damage;
}

int ItemInst::GetItemMagical(bool augments) const
{
	const auto item = GetItem();
	if (item) {
		if (item->Magic)
			return 1;

		if (augments) {
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i) && GetAugment(i)->GetItemMagical())
					return 1;
		}
	}
	return 0;
}

int ItemInst::GetItemHP(bool augments) const
{
	int hp = 0;
	const auto item = GetItem();
	if (item) {
		hp = item->HP;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					hp += GetAugment(i)->GetItemHP();
	}
	return hp;
}

int ItemInst::GetItemMana(bool augments) const
{
	int mana = 0;
	const auto item = GetItem();
	if (item) {
		mana = item->Mana;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					mana += GetAugment(i)->GetItemMana();
	}
	return mana;
}

int ItemInst::GetItemEndur(bool augments) const
{
	int endur = 0;
	const auto item = GetItem();
	if (item) {
		endur = item->Endur;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					endur += GetAugment(i)->GetItemEndur();
	}
	return endur;
}

int ItemInst::GetItemAttack(bool augments) const
{
	int atk = 0;
	const auto item = GetItem();
	if (item) {
		atk = item->Attack;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					atk += GetAugment(i)->GetItemAttack();
	}
	return atk;
}

int ItemInst::GetItemStr(bool augments) const
{
	int str = 0;
	const auto item = GetItem();
	if (item) {
		str = item->AStr;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					str += GetAugment(i)->GetItemStr();
	}
	return str;
}

int ItemInst::GetItemSta(bool augments) const
{
	int sta = 0;
	const auto item = GetItem();
	if (item) {
		sta = item->ASta;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					sta += GetAugment(i)->GetItemSta();
	}
	return sta;
}

int ItemInst::GetItemDex(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->ADex;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemDex();
	}
	return total;
}

int ItemInst::GetItemAgi(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AAgi;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemAgi();
	}
	return total;
}

int ItemInst::GetItemInt(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AInt;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemInt();
	}
	return total;
}

int ItemInst::GetItemWis(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->AWis;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemWis();
	}
	return total;
}

int ItemInst::GetItemCha(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->ACha;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCha();
	}
	return total;
}

int ItemInst::GetItemMR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->MR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemMR();
	}
	return total;
}

int ItemInst::GetItemFR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->FR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemFR();
	}
	return total;
}

int ItemInst::GetItemCR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->CR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCR();
	}
	return total;
}

int ItemInst::GetItemPR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->PR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemPR();
	}
	return total;
}

int ItemInst::GetItemDR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->DR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemDR();
	}
	return total;
}

int ItemInst::GetItemCorrup(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->SVCorruption;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemCorrup();
	}
	return total;
}

int ItemInst::GetItemHeroicStr(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicStr;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicStr();
	}
	return total;
}

int ItemInst::GetItemHeroicSta(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicSta;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicSta();
	}
	return total;
}

int ItemInst::GetItemHeroicDex(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicDex;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicDex();
	}
	return total;
}

int ItemInst::GetItemHeroicAgi(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicAgi;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicAgi();
	}
	return total;
}

int ItemInst::GetItemHeroicInt(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicInt;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicInt();
	}
	return total;
}

int ItemInst::GetItemHeroicWis(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicWis;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicWis();
	}
	return total;
}

int ItemInst::GetItemHeroicCha(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicCha;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCha();
	}
	return total;
}

int ItemInst::GetItemHeroicMR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicMR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicMR();
	}
	return total;
}

int ItemInst::GetItemHeroicFR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicFR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicFR();
	}
	return total;
}

int ItemInst::GetItemHeroicCR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicCR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCR();
	}
	return total;
}

int ItemInst::GetItemHeroicPR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicPR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicPR();
	}
	return total;
}

int ItemInst::GetItemHeroicDR(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicDR;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicDR();
	}
	return total;
}

int ItemInst::GetItemHeroicCorrup(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->HeroicSVCorrup;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i))
					total += GetAugment(i)->GetItemHeroicCorrup();
	}
	return total;
}

int ItemInst::GetItemHaste(bool augments) const
{
	int total = 0;
	const auto item = GetItem();
	if (item) {
		total = item->Haste;
		if (augments)
			for (int i = AUG_BEGIN; i < EQEmu::Constants::ITEM_COMMON_SIZE; ++i)
				if (GetAugment(i)) {
					int temp = GetAugment(i)->GetItemHaste();
					if (temp > total)
						total = temp;
				}
	}
	return total;
}

//
// class EvolveInfo
//
EvolveInfo::EvolveInfo() {
	// nothing here yet
}

EvolveInfo::EvolveInfo(uint32 first, uint8 max, bool allkills, uint32 L2, uint32 L3, uint32 L4, uint32 L5, uint32 L6, uint32 L7, uint32 L8, uint32 L9, uint32 L10) {
	FirstItem = first;
	MaxLvl = max;
	AllKills = allkills;
	LvlKills[0] = L2;
	LvlKills[1] = L3;
	LvlKills[2] = L4;
	LvlKills[3] = L5;
	LvlKills[4] = L6;
	LvlKills[5] = L7;
	LvlKills[6] = L8;
	LvlKills[7] = L9;
	LvlKills[8] = L10;
}

EvolveInfo::~EvolveInfo() {

}


//
// struct Item_Struct
//
bool Item_Struct::IsEquipable(uint16 Race, uint16 Class_) const
{
	bool IsRace = false;
	bool IsClass = false;

	uint32 Classes_ = Classes;
	uint32 Races_ = Races;
	uint32 Race_ = GetPlayerRaceValue(Race);

	for (int CurrentClass = 1; CurrentClass <= PLAYER_CLASS_COUNT; ++CurrentClass) {
		if (Classes_ & 1) {
			if (CurrentClass == Class_) {
				IsClass = true;
				break;
			}
		}
		Classes_ >>= 1;
	}

	Race_ = (Race_ == 18 ? 16 : Race_);

	for (unsigned int CurrentRace = 1; CurrentRace <= PLAYER_RACE_COUNT; ++CurrentRace) {
		if (Races_ & 1) {
			if (CurrentRace == Race_) {
				IsRace = true;
				break;
			}
		}
		Races_ >>= 1;
	}

	return (IsRace && IsClass);
}
