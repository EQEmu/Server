/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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

#include "inventory.h"
#include "data_verification.h"
#include <map>

struct EQEmu::Inventory::impl
{
	std::map<int, ItemContainer> containers_;
};

EQEmu::Inventory::Inventory() {
	impl_ = new impl;
}

EQEmu::Inventory::~Inventory() {
	delete impl_;
}

std::shared_ptr<EQEmu::ItemInstance> EQEmu::Inventory::Get(const InventorySlot &slot) {
	auto iter = impl_->containers_.find(slot.type_);
	if(iter != impl_->containers_.end()) {
		auto item = iter->second.Get(slot.slot_);
		if(item) {
			if(slot.bag_index_ > -1) {
				auto sub_item = item->Get(slot.bag_index_);
				if(sub_item) {
					if(slot.aug_index_ > -1) {
						return sub_item->Get(slot.aug_index_);
					} else {
						return sub_item;
					}
				}
			} else {
				return item;
			}
		}
	}

	return std::shared_ptr<ItemInstance>(nullptr);
}

bool EQEmu::Inventory::Put(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst) {
	if(impl_->containers_.count(slot.type_) == 0) {
		impl_->containers_.insert(std::pair<int, ItemContainer>(slot.type_, ItemContainer()));
	}

	//Verify item can be put into the slot requested

	auto &container = impl_->containers_[slot.type_];
	if(slot.bag_index_ > -1) {
		auto item = container.Get(slot.slot_);
		if(!item)
			return false;

		if(slot.aug_index_ > -1) {
			auto bag_item = item->Get(slot.bag_index_);
			if(!bag_item) {
				return false;
			}
			
			return bag_item->Put(slot.aug_index_, inst);
		} else {
			return item->Put(slot.bag_index_, inst);
		}
	} else {
		if(slot.aug_index_ > -1) {
			auto item = container.Get(slot.slot_);
			if(!item)
				return false;

			return item->Put(slot.aug_index_, inst);
		}

		return container.Put(slot.slot_, inst);
	}

	return false;
}

bool EQEmu::Inventory::Swap(const InventorySlot &src, const InventorySlot &dest, int charges) {
	return false;
}

int EQEmu::Inventory::CalcMaterialFromSlot(const InventorySlot &slot) {
	if(slot.type_ != 0)
		return _MaterialInvalid;

	switch(slot.slot_) {
	case PersonalSlotHead:
		return MaterialHead;
	case PersonalSlotChest:
		return MaterialChest;
	case PersonalSlotArms:
		return MaterialArms;
	case PersonalSlotWrist1:
		return MaterialWrist;
	case PersonalSlotHands:
		return MaterialHands;
	case PersonalSlotLegs:
		return MaterialLegs;
	case PersonalSlotFeet:
		return MaterialFeet;
	case PersonalSlotPrimary:
		return MaterialPrimary;
	case PersonalSlotSecondary:
		return MaterialSecondary;
	default:
		return _MaterialInvalid;
	}
}

EQEmu::InventorySlot EQEmu::Inventory::CalcSlotFromMaterial(int material) {
	switch(material)
	{
	case MaterialHead:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotHead);
	case MaterialChest:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotChest);
	case MaterialArms:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotArms);
	case MaterialWrist:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotWrist1);
	case MaterialHands:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotHands);
	case MaterialLegs:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotLegs);
	case MaterialFeet:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotFeet);
	case MaterialPrimary:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotPrimary);
	case MaterialSecondary:
		return EQEmu::InventorySlot(InvTypePersonal, PersonalSlotSecondary);
	default:
		return EQEmu::InventorySlot(-1, -1);
	}
}

bool EQEmu::Inventory::Serialize(MemoryBuffer &buf) {
	buf.SetWritePosition(0);
	buf.SetReadPosition(0);
	buf.Resize(0);

	bool value = false;
	for(auto &iter : impl_->containers_) {
		bool v = iter.second.Serialize(buf, iter.first);
		if(v && !value) {
			value = true;
		}
	}

	return value;
}
