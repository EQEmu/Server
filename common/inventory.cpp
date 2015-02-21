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

std::shared_ptr<EQEmu::ItemInstance> EQEmu::Inventory::Get(int container_id, int slot_id) {
	auto iter = impl_->containers_.find(container_id);
	if(iter != impl_->containers_.end()) {
		return iter->second.Get(slot_id);
	}

	return std::shared_ptr<ItemInstance>(nullptr);
}

std::shared_ptr<EQEmu::ItemInstance> EQEmu::Inventory::Get(int container_id, int slot_id, int bag_idx) {
	auto iter = impl_->containers_.find(container_id);
	if(iter != impl_->containers_.end()) {
		auto item = iter->second.Get(slot_id);
		if(item) {
			return item->GetItem(bag_idx);
		}
	}

	return std::shared_ptr<ItemInstance>(nullptr);
}

bool EQEmu::Inventory::Put(int container_id, int slot_id, std::shared_ptr<ItemInstance> inst) {
	if(impl_->containers_.count(container_id) == 0) {
		auto &container = impl_->containers_[container_id];
		return container.Put(slot_id, inst);
	} else {
		ItemContainer container;
		bool v = container.Put(slot_id, inst);
		impl_->containers_[container_id] = container;

		return v;
	}
}
