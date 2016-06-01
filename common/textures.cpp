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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "textures.h"
//#include "inventory_slot.h"

#include <string.h> // temp


//int EQEmu::textures::ConvertEquipmentSlotToTextureSlot(int equipment_slot)
//{
//	switch (equipment_slot) {
//	case inventory::PossessionsHead:
//		return textures::TextureHead;
//	case inventory::PossessionsChest:
//		return textures::TextureChest;
//	case inventory::PossessionsArms:
//		return textures::TextureArms;
//	case inventory::PossessionsWrist1:
//		return textures::TextureWrist;
//	case inventory::PossessionsHands:
//		return textures::TextureHands;
//	case inventory::PossessionsLegs:
//		return textures::TextureLegs;
//	case inventory::PossessionsFeet:
//		return textures::TextureFeet;
//	case inventory::PossessionsPrimary:
//		return textures::TexturePrimary;
//	case inventory::PossessionsSecondary:
//		return textures::TextureSecondary;
//	default:
//		return textures::TextureInvalid;
//	}
//}

//int EQEmu::textures::ConvertEquipmentSlotToTextureSlot(const InventorySlot &equipment_slot)
//{
//	if (equipment_slot.Type() != inventory::InvTypePossessions || equipment_slot.Bag() != inventory::InvBagInvalid || equipment_slot.Aug() != inventory::InvAugInvalid)
//		return textures::TextureInvalid;
//
//	return ConvertEquipmentSlotToTextureSlot(equipment_slot.Slot());
//}

//EQEmu::InventorySlot EQEmu::textures::ConvertTextureSlotToEquipmentSlot(int texture_slot)
//{
//	switch (texture_slot) {
//	case textures::TextureHead:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsHead);
//	case textures::TextureChest:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsChest);
//	case textures::TextureArms:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsArms);
//	case textures::TextureWrist:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsWrist1);
//	case textures::TextureHands:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsHands);
//	case textures::TextureLegs:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsLegs);
//	case textures::TextureFeet:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsFeet);
//	case textures::TexturePrimary:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsPrimary);
//	case textures::TextureSecondary:
//		return EQEmu::InventorySlot(inventory::InvTypePossessions, inventory::PossessionsSecondary);
//	default:
//		return EQEmu::InventorySlot();
//	}
//}

EQEmu::TextureProfile::TextureProfile()
{
	memset(&Slot, 0, (sizeof(Texture_Struct) * textures::TextureCount));
}

EQEmu::TextureShortProfile::TextureShortProfile()
{
	memset(&Slot, 0, (sizeof(uint32) * textures::TextureCount));
}

EQEmu::TintProfile::TintProfile()
{
	memset(&Slot, 0, (sizeof(uint32) * textures::TextureCount));
}
