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

EQEmu::ItemInstance::ItemInstance() {
	base_item_ = nullptr;
	modified_item_ = nullptr;
	charges_ = -1;
	color_ = 0;
	attuned_ = false;
	ornament_idfile_ = 0;
	ornament_icon_ = 0;
	ornament_hero_model_ = 0;
	tracking_id_ = 0;
}

EQEmu::ItemInstance::ItemInstance(const ItemData* idata) {
	base_item_ = idata;
	modified_item_ = nullptr;
	charges_ = -1;
	color_ = 0;
	attuned_ = false;
	ornament_idfile_ = 0;
	ornament_icon_ = 0;
	ornament_hero_model_ = 0;
	tracking_id_ = 0;
}

EQEmu::ItemInstance::ItemInstance(const ItemData* idata, int16 charges) {
	base_item_ = idata;
	modified_item_ = nullptr;
	charges_ = charges;
	color_ = 0;
	attuned_ = false;
	ornament_idfile_ = 0;
	ornament_icon_ = 0;
	ornament_hero_model_ = 0;
	tracking_id_ = 0;
}

EQEmu::ItemInstance::~ItemInstance() {
}

std::shared_ptr<EQEmu::ItemInstance> EQEmu::ItemInstance::GetItem(int index) {
	if(EQEmu::ValueWithin(index, 0, 200)) {
		auto iter = contents_.find(index);
		if(iter != contents_.end()) {
			return iter->second;
		}
	}

	return std::shared_ptr<EQEmu::ItemInstance>(nullptr);
}
