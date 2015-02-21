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

#include "item_instance.h"
#include "data_verification.h"
#include "item_container.h"

struct EQEmu::ItemInstance::impl {
	const ItemData *base_item_;
	ItemData *modified_item_;
	int16 charges_;
	uint32 color_;
	bool attuned_;
	std::string custom_data_;
	uint32 ornament_idfile_;
	uint32 ornament_icon_;
	uint32 ornament_hero_model_;
	uint64 tracking_id_;
	uint32 recast_timestamp_;
	ItemContainer contents_;
};

EQEmu::ItemInstance::ItemInstance() {
	impl_ = new impl;
	impl_->base_item_ = nullptr;
	impl_->modified_item_ = nullptr;
	impl_->charges_ = -1;
	impl_->color_ = 0;
	impl_->attuned_ = false;
	impl_->ornament_idfile_ = 0;
	impl_->ornament_icon_ = 0;
	impl_->ornament_hero_model_ = 0;
	impl_->recast_timestamp_ = 0;
	impl_->tracking_id_ = 0;
}

EQEmu::ItemInstance::ItemInstance(const ItemData* idata) {
	impl_ = new impl;
	impl_->base_item_ = idata;
	impl_->modified_item_ = nullptr;
	impl_->charges_ = -1;
	impl_->color_ = 0;
	impl_->attuned_ = false;
	impl_->ornament_idfile_ = 0;
	impl_->ornament_icon_ = 0;
	impl_->ornament_hero_model_ = 0;
	impl_->recast_timestamp_ = 0;
	impl_->tracking_id_ = 0;
}

EQEmu::ItemInstance::ItemInstance(const ItemData* idata, int16 charges) {
	impl_ = new impl;
	impl_->base_item_ = idata;
	impl_->modified_item_ = nullptr;
	impl_->charges_ = charges;
	impl_->color_ = 0;
	impl_->attuned_ = false;
	impl_->ornament_idfile_ = 0;
	impl_->ornament_icon_ = 0;
	impl_->ornament_hero_model_ = 0;
	impl_->recast_timestamp_ = 0;
	impl_->tracking_id_ = 0;
}

EQEmu::ItemInstance::~ItemInstance() {
	delete impl_;
}

const ItemData *EQEmu::ItemInstance::GetItem() {
	return impl_->modified_item_ ? impl_->modified_item_ : impl_->base_item_;
}

std::shared_ptr<EQEmu::ItemInstance> EQEmu::ItemInstance::Get(const int index) {
	if(EQEmu::ValueWithin(index, 0, 255)) {
		return impl_->contents_.Get(index);
	}
	
	return std::shared_ptr<EQEmu::ItemInstance>(nullptr);
}

bool EQEmu::ItemInstance::Put(const int index, std::shared_ptr<ItemInstance> inst) {
	if(!inst || !inst->GetItem()) {
		return false;
	}

	if(!impl_->base_item_) {
		return false;
	}

	auto *item = impl_->base_item_;
	if(item->ItemClass == ItemClassContainer) { // Bag
		if(!EQEmu::ValueWithin(index, 0, (int)item->BagSlots)) {
			return false;
		}

		return impl_->contents_.Put(index, inst);
	}
	else if(item->ItemClass == ItemClassCommon) { // Augment
		if(!EQEmu::ValueWithin(index, 0, (int)EmuConstants::ITEM_COMMON_SIZE)) {
			return false;
		}

		if(!item->AugSlotVisible[index]) {
			return false;
		}

		auto *aug_item = inst->GetItem();
		int aug_type = aug_item->AugType;
		if(aug_type == -1 || (1 << (item->AugSlotType[index] - 1)) & aug_type) {
			return impl_->contents_.Put(index, inst);
		}
		
		return false;
	}

	return false;
}

void EQEmu::ItemInstance::SetCharges(const int16 charges) {
	impl_->charges_ = charges;
}

void EQEmu::ItemInstance::SetColor(const uint32 color) {
	impl_->color_ = color;
}

void EQEmu::ItemInstance::SetAttuned(const bool attuned) {
	impl_->attuned_ = attuned;
}

void EQEmu::ItemInstance::SetCustomData(const std::string &custom_data) {
	//We need to actually set the custom data stuff based on this string
	impl_->custom_data_ = custom_data;
}

void EQEmu::ItemInstance::SetOrnamentIDFile(const uint32 ornament_idfile) {
	impl_->ornament_idfile_ = ornament_idfile;
}

void EQEmu::ItemInstance::SetOrnamentIcon(const uint32 ornament_icon) {
	impl_->ornament_icon_ = ornament_icon;
}

void EQEmu::ItemInstance::SetOrnamentHeroModel(const uint32 ornament_hero_model) {
	impl_->ornament_hero_model_ = ornament_hero_model;
}

void EQEmu::ItemInstance::SetTrackingID(const uint64 tracking_id) {
	impl_->tracking_id_ = tracking_id;
}

void EQEmu::ItemInstance::SetRecastTimestamp(const uint32 recast_timestamp) {
	impl_->recast_timestamp_ = recast_timestamp;
}
