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
#include "item_container_personal_serialization.h"
#include "string_util.h"
#include <map>

bool EQEmu::InventorySlot::IsValid() const {
	if(type_ == InvTypePersonal && EQEmu::ValueWithin(slot_, PersonalSlotCharm, PersonalSlotCursor)) {
		return true;
	}

	if(type_ == InvTypeBank && EQEmu::ValueWithin(slot_, 0, 23)) {
		return true;
	}

	if(type_ == InvTypeSharedBank && EQEmu::ValueWithin(slot_, 0, 1)) {
		return true;
	}

	if(type_ == InvTypeTribute && EQEmu::ValueWithin(slot_, 0, 4)) {
		return true;
	}

	if(type_ == InvTypeTrade && EQEmu::ValueWithin(slot_, 0, 7)) {
		return true;
	}

	if(type_ == InvTypeWorld && EQEmu::ValueWithin(slot_, 0, 255)) {
		return true;
	}


	return false;
}

bool EQEmu::InventorySlot::IsBank() const {
	if(type_ == InvTypeBank && EQEmu::ValueWithin(slot_, 0, 23)) {
		return true;
	}

	if(type_ == InvTypeSharedBank && EQEmu::ValueWithin(slot_, 0, 1)) {
		return true;
	}

	return false;
}

bool EQEmu::InventorySlot::IsCursor() const {
	if(type_ == InvTypePersonal && slot_ == PersonalSlotCursor) {
		return true;
	}

	if(type_ == InvTypeCursorBuffer) {
		return true;
	}

	return false;
}

bool EQEmu::InventorySlot::IsEquipment() const {
	if(type_ == InvTypePersonal && EQEmu::ValueWithin(slot_, PersonalSlotCharm, PersonalSlotAmmo)) {
		return true;
	}

	return false;
}

bool EQEmu::InventorySlot::IsGeneral() const {
	if(type_ == InvTypePersonal && EQEmu::ValueWithin(slot_, PersonalSlotGeneral1, PersonalSlotGeneral10)) {
		return true;
	}

	return false;
}

const std::string EQEmu::InventorySlot::ToString() const {
	return StringFormat("(%i, %i, %i, %i)", type_, slot_, bag_index_, aug_index_);
}

struct EQEmu::Inventory::impl
{
	std::map<int, ItemContainer> containers_;
	int race_;
	int class_;
};

EQEmu::Inventory::Inventory(int race, int class_) {
	impl_ = new impl;
	impl_->race_ = race;
	impl_->class_ = class_;
}

EQEmu::Inventory::~Inventory() {
	delete impl_;
}

std::shared_ptr<EQEmu::ItemInstance> EQEmu::Inventory::Get(const InventorySlot &slot) {
	auto iter = impl_->containers_.find(slot.Type());
	if(iter != impl_->containers_.end()) {
		auto item = iter->second.Get(slot.Slot());
		if(item) {
			if(slot.BagIndex() > -1) {
				auto sub_item = item->Get(slot.BagIndex());
				if(sub_item) {
					if(slot.AugIndex() > -1) {
						return sub_item->Get(slot.AugIndex());
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
	if(impl_->containers_.count(slot.Type()) == 0) {
		if(slot.Type() == 0) {
			impl_->containers_.insert(std::pair<int, ItemContainer>(slot.Type(), ItemContainer(new ItemContainerPersonalSerialization())));
		} else {
			impl_->containers_.insert(std::pair<int, ItemContainer>(slot.Type(), ItemContainer()));
		}
	}

	//Verify item can be put into the slot requested

	auto &container = impl_->containers_[slot.Type()];
	if(slot.BagIndex() > -1) {
		auto item = container.Get(slot.Slot());
		if(!item)
			return false;

		if(slot.AugIndex() > -1) {
			auto bag_item = item->Get(slot.BagIndex());
			if(!bag_item) {
				return false;
			}
			
			return bag_item->Put(slot.AugIndex(), inst);
		} else {
			return item->Put(slot.BagIndex(), inst);
		}
	} else {
		if(slot.AugIndex() > -1) {
			auto item = container.Get(slot.Slot());
			if(!item)
				return false;

			return item->Put(slot.AugIndex(), inst);
		}

		return container.Put(slot.Slot(), inst);
	}

	return false;
}

bool EQEmu::Inventory::Swap(const InventorySlot &src, const InventorySlot &dest, int charges) {
	return false;
}

int EQEmu::Inventory::CalcMaterialFromSlot(const InventorySlot &slot) {
	if(slot.Type() != 0)
		return _MaterialInvalid;

	switch(slot.Slot()) {
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

bool EQEmu::Inventory::CanEquip(std::shared_ptr<EQEmu::ItemInstance> inst, const EQEmu::InventorySlot &slot) {
	if(!inst) {
		return false;
	}
	
	if(slot.Type() != 0) {
		return false;
	}
	
	if(!EQEmu::ValueWithin(slot.Slot(), EQEmu::PersonalSlotCharm, EQEmu::PersonalSlotAmmo)) {
		return false;
	}
	
	auto item = inst->GetItem();
	//check slot
	
	int use_slot = -1;
	if(slot.Slot() == EQEmu::PersonalSlotPowerSource) {
		use_slot = EQEmu::PersonalSlotAmmo;
	}
	else if(slot.Slot() == EQEmu::PersonalSlotAmmo) {
		use_slot = EQEmu::PersonalSlotPowerSource;
	}
	else {
		use_slot = slot.Slot();
	}
	
	if(!(item->Slots & (1 << use_slot))) {
		return false;
	}
	
	if(!item->IsEquipable(impl_->race_, impl_->class_)) {
		return false;
	}

	return true;
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
