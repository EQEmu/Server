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
#include "inventory_null_data_model.h"
#include "item_container_personal_serialization.h"
#include "data_verification.h"
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

bool EQEmu::InventorySlot::IsDelete() const {
	return type_ == -1 && slot_ == -1 && bag_index_ == -1 && aug_index_ == -1;
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

bool EQEmu::InventorySlot::IsWeapon() const {
	if(type_ == InvTypePersonal && 
	   (EQEmu::ValueWithin(slot_, PersonalSlotPrimary, PersonalSlotSecondary) || slot_ == PersonalSlotRange))
	{
		return true;
	}

	return false;
}

bool EQEmu::InventorySlot::IsTrade() const {
	if(type_ == InvTypeTrade) {
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
	int deity_;
	std::unique_ptr<InventoryDataModel> data_model_;
};

EQEmu::Inventory::Inventory(int race, int class_, int deity) {
	impl_ = new impl;
	impl_->race_ = race;
	impl_->class_ = class_;
	impl_->deity_ = deity;
	impl_->data_model_ = std::unique_ptr<InventoryDataModel>(new InventoryNullDataModel());
}

EQEmu::Inventory::~Inventory() {
	delete impl_;
}

void EQEmu::Inventory::SetRace(int race) {
	impl_->race_ = race;
}

void EQEmu::Inventory::SetClass(int class_) {
	impl_->class_ = class_;
}

void EQEmu::Inventory::SetDeity(int deity) {
	impl_->deity_ = deity;
}

void EQEmu::Inventory::SetDataModel(InventoryDataModel *dm) {
	impl_->data_model_ = std::unique_ptr<InventoryDataModel>(dm);
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
	if(src == dest) {
		return true;
	}

	if(!src.IsValid()) {
		return false;
	}

	if(src.Type() == InvTypeCursorBuffer || dest.Type() == InvTypeCursorBuffer) {
		return true;
	}

	if(dest.IsDelete()) {
		impl_->data_model_->Begin();
		bool v = _destroy(src);
		if(v) {
			impl_->data_model_->Commit();
		}
		else {
			impl_->data_model_->Rollback();
		}

		return v;
	}

	if(!dest.IsValid()) {
		return false;
	}

	auto i_src = Get(src);
	auto i_dest = Get(dest);

	if(!i_src) {
		return false;
	}

	if(i_src->GetBaseItem()->ItemClass == ItemClassContainer && dest.BagIndex() > -1) {
		if(i_src->GetContainer()->Size() > 0) {
			return false;
		}
	}

	if(dest.IsEquipment() && !CanEquip(i_src, dest)) {
		return false;
	}

	//Check this -> trade no drop
	if(dest.IsTrade() && i_src->IsNoDrop()) {
		return false;
	}

	impl_->data_model_->Begin();
	if(i_src->IsStackable()) {
		//move # charges from src to dest

		//0 means *all* the charges
		if(charges == 0) {
			charges = i_src->GetCharges();
		}

		//src needs to have that many charges
		if(i_src->GetCharges() < charges) {
			impl_->data_model_->Rollback();
			return false;
		}

		//if dest exists it needs to not only be the same item id but also be able to hold enough charges to combine
		//we can also swap if src id != dest id
		if(i_dest) {
			uint32 src_id = i_src->GetBaseItem()->ID;
			uint32 dest_id = i_dest->GetBaseItem()->ID;
			if(src_id != dest_id) {
				bool v = _swap(src, dest);
				if(v) {
					impl_->data_model_->Commit();
				}
				else {
					impl_->data_model_->Rollback();
				}

				return v;
			}

			int charges_avail = i_dest->GetBaseItem()->StackSize - i_dest->GetCharges();
			if(charges_avail < charges) {
				impl_->data_model_->Rollback();
				return false;
			}

			if(i_src->GetCharges() == charges) {
				if(!_destroy(src)) {
					impl_->data_model_->Rollback();
					return false;
				}
			} else {
				i_src->SetCharges(i_src->GetCharges() - charges);
				impl_->data_model_->Insert(src, i_src);
			}

			i_dest->SetCharges(i_dest->GetCharges() + charges);
			impl_->data_model_->Insert(dest, i_dest);
			impl_->data_model_->Commit();
			return true;
		} else {
			//if dest does not exist and src charges > # charges then we need to create a new item with # charges in dest
			//if dest does not exist and src charges == # charges then we need to swap src to dest
			if(i_src->GetCharges() > charges) {
				auto split = i_src->Split(charges);
				if(!split) {
					impl_->data_model_->Rollback();
					return false;
				}

				Put(dest, split);
				impl_->data_model_->Insert(src, i_src);
				impl_->data_model_->Insert(dest, split);
				impl_->data_model_->Commit();
				return true;
			} else {
				bool v = _swap(src, dest);
				if(v) {
					impl_->data_model_->Commit();
				} else {
					impl_->data_model_->Rollback();
				}

				return v;
			}
		}
	} else {
		bool v = _swap(src, dest);
		if(v) {
			impl_->data_model_->Commit();
		}
		else {
			impl_->data_model_->Rollback();
		}

		return v;
	}

	impl_->data_model_->Commit();
	return true;
}

bool EQEmu::Inventory::TryStacking(std::shared_ptr<EQEmu::ItemInstance> inst, const InventorySlot &slot) {
	auto target_inst = Get(slot);
	
	if(!inst || !target_inst ||
	   !inst->IsStackable() || !target_inst->IsStackable()) 
	{
		return false;
	}
	
	if(inst->GetBaseItem()->ID != target_inst->GetBaseItem()->ID) {
		return false;
	}
	
	int stack_avail = target_inst->GetBaseItem()->StackSize - target_inst->GetCharges();
	
	if(stack_avail <= 0) {
		return false;
	}
	
	impl_->data_model_->Begin();
	if(inst->GetCharges() <= stack_avail) {
		inst->SetCharges(0);
		target_inst->SetCharges(target_inst->GetCharges() + inst->GetCharges());
		impl_->data_model_->Delete(slot);
		impl_->data_model_->Insert(slot, target_inst);
	} else {
		inst->SetCharges(inst->GetCharges() - stack_avail);
		target_inst->SetCharges(target_inst->GetCharges() + stack_avail);
		impl_->data_model_->Delete(slot);
		impl_->data_model_->Insert(slot, target_inst);
	}

	impl_->data_model_->Commit();

	return true;
}

bool EQEmu::Inventory::Summon(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst) {
	if(!inst)
		return false;

	if(CheckLoreConflict(inst->GetBaseItem())) {
		return false;
	}

	auto cur = Get(slot);
	if(cur) {
		if(slot.IsCursor()) {
			PushToCursorBuffer(inst);
		}

		return false;
	}

	impl_->data_model_->Begin();
	bool v = Put(slot, inst);
	if(v) {
		impl_->data_model_->Insert(slot, inst);
		impl_->data_model_->Commit();
	} else {
		impl_->data_model_->Rollback();
	}

	return v;
}

bool EQEmu::Inventory::PushToCursorBuffer(std::shared_ptr<ItemInstance> inst) {
	if(impl_->containers_.count(InvTypeCursorBuffer) == 0) {
		impl_->containers_.insert(std::pair<int, ItemContainer>(InvTypeCursorBuffer, ItemContainer()));
	}

	int32 top = 0;
	auto &container = impl_->containers_[InvTypeCursorBuffer];
	auto iter = container.Begin();
	while(iter != container.End()) {
		top = iter->first;
		++iter;
	}

	InventorySlot slot(InvTypeCursorBuffer, top + 1);
	impl_->data_model_->Begin();
	bool v = Put(slot, inst);
	if(v) {
		impl_->data_model_->Insert(slot, inst);
		impl_->data_model_->Commit();
	}
	else {
		impl_->data_model_->Rollback();
	}

	return v;
}

bool EQEmu::Inventory::PopFromCursorBuffer() {
	InventorySlot cursor(InvTypePersonal, PersonalSlotCursor);
	auto inst = Get(cursor);
	if(inst) {
		return false;
	}

	if(impl_->containers_.count(InvTypeCursorBuffer) == 0) {
		return false;
	}

	int32 top = 0;
	auto &container = impl_->containers_[InvTypeCursorBuffer];
	auto iter = container.Begin();
	while(iter != container.End()) {
		top = iter->first;
		++iter;
	}

	InventorySlot slot(InvTypeCursorBuffer, top);
	inst = Get(slot);

	if(inst) {
		impl_->data_model_->Begin();

		bool v = _destroy(slot);
		impl_->data_model_->Delete(slot);

		if(!v) {
			impl_->data_model_->Rollback();
			return false;
		}

		v = Put(cursor, inst);
		impl_->data_model_->Insert(cursor, inst);
		if(!v) {
			impl_->data_model_->Rollback();
			return false;
		}

		impl_->data_model_->Commit();
		return true;
	}

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
	
	//todo: check deity
	if(!item->IsEquipable(impl_->race_, impl_->class_)) {
		return false;
	}

	//Checking augments
	auto iter = inst->GetContainer()->Begin();
	auto end = inst->GetContainer()->End();
	while(iter != end) {
		if(!CanEquip(iter->second, InventorySlot(slot.Type(), slot.Slot(), slot.BagIndex(), iter->first))) {
			return false;
		}
		++iter;
	}

	return true;
}

bool EQEmu::Inventory::CheckLoreConflict(const ItemData *item) {
	if(!item)
		return false;

	if(!item->LoreFlag)
		return false;

	if(item->LoreGroup == 0)
		return false;

	if(item->LoreGroup == 0xFFFFFFFF) {
		//look everywhere except shared bank
		for(auto &container : impl_->containers_) {
			if(container.first != InvTypeSharedBank && container.second.HasItem(item->ID)) {
				return true;
			}
		}
	}
	else {
		//look everywhere except shared bank
		for(auto &container : impl_->containers_) {
			if(container.first != InvTypeSharedBank && container.second.HasItemByLoreGroup(item->LoreGroup)) {
				return true;
			}
		}
	}

	return false;
}

bool EQEmu::Inventory::Serialize(MemoryBuffer &buf) {
	buf.SetWritePosition(0);
	buf.SetReadPosition(0);
	buf.Resize(0);
	buf.Write<int32>(105);

	bool value = false;
	for(auto &iter : impl_->containers_) {
		bool v = iter.second.Serialize(buf, iter.first);
		if(v && !value) {
			value = true;
		}
	}

	return value;
}

void EQEmu::Inventory::Interrogate() {
	printf("Inventory:\n");
	printf("Class: %u, Race: %u, Deity: %u\n", impl_->class_, impl_->race_, impl_->deity_);
	for(auto &iter : impl_->containers_) {
		printf("Container: %u\n", iter.first);
		iter.second.Interrogate(1);
	}
	printf("\n");
}

bool EQEmu::Inventory::_swap(const InventorySlot &src, const InventorySlot &dest) {
	auto src_i = Get(src);
	auto dest_i = Get(dest);

	if(src_i) {
		if(!_destroy(src)) {
			return false;
		}
	}

	if(dest_i) {
		if(!_destroy(dest)) {
			return false;
		}

		impl_->data_model_->Insert(src, dest_i);
		if(!Put(src, dest_i)) {
			return false;
		}
	}

	if(src_i) {
		impl_->data_model_->Insert(dest, src_i);
		if(!Put(dest, src_i)) {
			return false;
		}
	}

	return true;
}

bool EQEmu::Inventory::_destroy(const InventorySlot &slot) {
	bool v = Put(slot, std::shared_ptr<EQEmu::ItemInstance>(nullptr));
	impl_->data_model_->Delete(slot);
	return v;
}
