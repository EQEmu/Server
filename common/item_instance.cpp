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
	char tracking_id_[17];
	uint32 serial_id_;
	uint32 recast_timestamp_;
	uint32 merchant_slot_;
	uint32 merchant_count_;
	uint32 price_;
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
	impl_->serial_id_ = 0;
	impl_->recast_timestamp_ = 0;
	impl_->merchant_slot_ = 0;
	impl_->merchant_count_ = 0;
	impl_->price_ = 0;
	memset(impl_->tracking_id_, 0, 17);
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
	impl_->serial_id_ = 0;
	impl_->recast_timestamp_ = 0;
	impl_->merchant_slot_ = 0;
	impl_->merchant_count_ = 0;
	impl_->price_ = 0;
	memset(impl_->tracking_id_, 0, 17);
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
	impl_->serial_id_ = 0;
	impl_->merchant_slot_ = 0;
	impl_->merchant_count_ = 0;
	impl_->price_ = 0;
	memset(impl_->tracking_id_, 0, 17);
}

EQEmu::ItemInstance::~ItemInstance() {
	delete impl_;
}

const ItemData *EQEmu::ItemInstance::GetItem() {
	return impl_->modified_item_ ? impl_->modified_item_ : impl_->base_item_;
}

const ItemData *EQEmu::ItemInstance::GetBaseItem() {
	return impl_->base_item_;
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

uint32 EQEmu::ItemInstance::GetSubItemCount() {
	return impl_->contents_.Size();
}

uint32 EQEmu::ItemInstance::GetSubItemCount() const {
	return impl_->contents_.Size();
}

int16 EQEmu::ItemInstance::GetCharges() {
	return impl_->charges_;
}

int16 EQEmu::ItemInstance::GetCharges() const {
	return impl_->charges_;
}

void EQEmu::ItemInstance::SetCharges(const int16 charges) {
	impl_->charges_ = charges;
}

void EQEmu::ItemInstance::SetColor(const uint32 color) {
	impl_->color_ = color;
}

bool EQEmu::ItemInstance::GetAttuned() {
	return impl_->attuned_;
}

bool EQEmu::ItemInstance::GetAttuned() const {
	return impl_->attuned_;
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

const char* EQEmu::ItemInstance::GetTrackingID() {
	return impl_->tracking_id_;
}

const char* EQEmu::ItemInstance::GetTrackingID() const {
	return impl_->tracking_id_;
}

void EQEmu::ItemInstance::SetTrackingID(const char *tracking_id) {
	size_t len = strlen(tracking_id);
	if(len > 16) {
		return;
	}

	strncpy(impl_->tracking_id_, tracking_id, 16);
}

uint32 EQEmu::ItemInstance::GetRecastTimestamp() {
	return impl_->recast_timestamp_;
}

uint32 EQEmu::ItemInstance::GetRecastTimestamp() const {
	return impl_->recast_timestamp_;
}

void EQEmu::ItemInstance::SetRecastTimestamp(const uint32 recast_timestamp) {
	impl_->recast_timestamp_ = recast_timestamp;
}

uint32 EQEmu::ItemInstance::GetMerchantSlot() {
	return impl_->merchant_slot_;
}

uint32 EQEmu::ItemInstance::GetMerchantSlot() const {
	return impl_->merchant_slot_;
}

void EQEmu::ItemInstance::SetMerchantSlot(uint32 slot) {
	impl_->merchant_slot_ = slot;
}

uint32 EQEmu::ItemInstance::GetMerchantCount() {
	return impl_->merchant_count_;
}

uint32 EQEmu::ItemInstance::GetMerchantCount() const {
	return impl_->merchant_count_;
}

void EQEmu::ItemInstance::SetMerchantCount(const uint32 cnt) {
	impl_->merchant_count_ = cnt;
}

uint32 EQEmu::ItemInstance::GetPrice() {
	return impl_->price_;
}

uint32 EQEmu::ItemInstance::GetPrice() const {
	return impl_->price_;
}

void EQEmu::ItemInstance::SetPrice(const uint32 p) {
	impl_->price_ = p;
}

uint32 EQEmu::ItemInstance::GetSerialNumber() {
	return impl_->serial_id_;
}

uint32 EQEmu::ItemInstance::GetSerialNumber() const {
	return impl_->serial_id_;
}

void EQEmu::ItemInstance::SetSerialNumber(const uint32 sn) {
	impl_->serial_id_ = sn;
}

bool EQEmu::ItemInstance::IsStackable() {
	return impl_->base_item_->Stackable;
}

bool EQEmu::ItemInstance::IsStackable() const {
	return impl_->base_item_->Stackable;
}
