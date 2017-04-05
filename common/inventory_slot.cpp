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

#include "inventory_slot.h"
#include "textures.h"
#include "string_util.h"


int8 EQEmu::inventory::ConvertEquipmentIndexToTextureIndex(int16 slot_index)
{
	switch (slot_index) {
	case slotHead:
		return textures::armorHead;
	case slotChest:
		return textures::armorChest;
	case slotArms:
		return textures::armorArms;
	case slotWrist1:
		return textures::armorWrist;
	case slotHands:
		return textures::armorHands;
	case slotLegs:
		return textures::armorLegs;
	case slotFeet:
		return textures::armorFeet;
	case slotPrimary:
		return textures::weaponPrimary;
	case slotSecondary:
		return textures::weaponSecondary;
	default:
		return textures::textureInvalid;
	}
}

int8 EQEmu::inventory::ConvertEquipmentSlotToTextureIndex(const InventorySlot& inventory_slot)
{
	if ((!inventory_slot.Typeless() && !inventory_slot.IsTypeIndex(typePossessions)) || !inventory_slot.IsContainerIndex(containerInvalid) || !inventory_slot.IsSocketIndex(socketInvalid))
		return textures::textureInvalid;

	return ConvertEquipmentIndexToTextureIndex(inventory_slot.SlotIndex());
}

int16 EQEmu::inventory::ConvertTextureIndexToEquipmentIndex(int8 texture_index)
{
	switch (texture_index) {
	case textures::armorHead:
		return slotHead;
	case textures::armorChest:
		return slotChest;
	case textures::armorArms:
		return slotArms;
	case textures::armorWrist:
		return slotWrist1;
	case textures::armorHands:
		return slotHands;
	case textures::armorLegs:
		return slotLegs;
	case textures::armorFeet:
		return slotFeet;
	case textures::weaponPrimary:
		return slotPrimary;
	case textures::weaponSecondary:
		return slotSecondary;
	default:
		return slotInvalid;
	}
}

bool EQEmu::InventorySlot::IsValidSlot() const
{
	if (_typeless)
		return false;
	
	int16 slot_count = inventory::SlotCount(_type_index);
	if (!slot_count || _slot_index < inventory::slotBegin || _slot_index >= slot_count)
		return false;

	if (_container_index < inventory::containerInvalid || _container_index >= inventory::ContainerCount)
		return false;

	if (_socket_index < inventory::socketInvalid || _socket_index >= inventory::SocketCount)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsDeleteSlot() const
{
	if (_typeless)
		return (_slot_index == inventory::slotInvalid && _container_index == inventory::containerInvalid && _socket_index == inventory::socketInvalid);
	else
		return (_type_index == inventory::typeInvalid && _slot_index == inventory::slotInvalid && _container_index == inventory::containerInvalid && _socket_index == inventory::socketInvalid);
}

bool EQEmu::InventorySlot::IsEquipmentIndex(int16 slot_index)
{
	/*if (slot_index < inventory::EquipmentBegin || slot_index > inventory::EquipmentEnd)
		return false;*/
	if ((slot_index < legacy::EQUIPMENT_BEGIN || slot_index > legacy::EQUIPMENT_END) && slot_index != legacy::SLOT_POWER_SOURCE)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsGeneralIndex(int16 slot_index)
{
	/*if (slot_index < inventory::GeneralBegin || slot_index > inventory::GeneralEnd)
		return false;*/
	if (slot_index < legacy::GENERAL_BEGIN || slot_index > legacy::GENERAL_END)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsCursorIndex(int16 slot_index)
{
	/*if (slot_index != inventory::slotCursor)
		return false;*/
	if (slot_index != legacy::SLOT_CURSOR)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsWeaponIndex(int16 slot_index)
{
	/*if ((slot_index != inventory::slotRange) && (slot_index != inventory::slotPrimary) && (slot_index != inventory::slotSecondary))
		return false;*/
	if ((slot_index != legacy::SLOT_RANGE) && (slot_index != legacy::SLOT_PRIMARY) && (slot_index != legacy::SLOT_SECONDARY))
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsTextureIndex(int16 slot_index)
{
	switch (slot_index) {
	case inventory::slotHead:
	case inventory::slotChest:
	case inventory::slotArms:
	case inventory::slotWrist1:
	case inventory::slotHands:
	case inventory::slotLegs:
	case inventory::slotFeet:
	case inventory::slotPrimary:
	case inventory::slotSecondary:
		return true;
	default:
		return false;
	}
}

bool EQEmu::InventorySlot::IsTintableIndex(int16 slot_index)
{
	switch (slot_index) {
	case inventory::slotHead:
	case inventory::slotChest:
	case inventory::slotArms:
	case inventory::slotWrist1:
	case inventory::slotHands:
	case inventory::slotLegs:
	case inventory::slotFeet:
		return true;
	default:
		return false;
	}
}

bool EQEmu::InventorySlot::IsEquipmentSlot() const
{
	if (!_typeless && (_type_index != inventory::typePossessions))
		return false;

	if ((_container_index != inventory::containerInvalid) || (_socket_index != inventory::socketInvalid))
		return false;

	return IsEquipmentIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsGeneralSlot() const
{
	if (!_typeless && (_type_index != inventory::typePossessions))
		return false;

	if ((_container_index != inventory::containerInvalid) || (_socket_index != inventory::socketInvalid))
		return false;

	return IsGeneralIndex(_socket_index);
}

bool EQEmu::InventorySlot::IsCursorSlot() const
{
	if (!_typeless && (_type_index != inventory::typePossessions))
		return false;

	if ((_container_index != inventory::containerInvalid) || (_socket_index != inventory::socketInvalid))
		return false;

	return IsCursorIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsWeaponSlot() const
{
	if (!_typeless && (_type_index != inventory::typePossessions))
		return false;

	if ((_container_index != inventory::containerInvalid) || (_socket_index != inventory::socketInvalid))
		return false;

	return IsWeaponIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsTextureSlot() const
{
	if (!_typeless && (_type_index != inventory::typePossessions))
		return false;

	if ((_container_index != inventory::containerInvalid) || (_socket_index != inventory::socketInvalid))
		return false;

	return IsTextureIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsTintableSlot() const
{
	if (!_typeless && (_type_index != inventory::typePossessions))
		return false;

	if ((_container_index != inventory::containerInvalid) || (_socket_index != inventory::socketInvalid))
		return false;

	return IsTintableIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsSlot() const
{
	if (!_typeless && (_type_index == inventory::typeInvalid))
		return false;
	if (_slot_index == inventory::slotInvalid)
		return false;
	if (_container_index != inventory::containerInvalid)
		return false;
	if (_socket_index != inventory::socketInvalid)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsSlotSocket() const
{
	if (!_typeless && (_type_index == inventory::typeInvalid))
		return false;
	if (_slot_index == inventory::slotInvalid)
		return false;
	if (_container_index != inventory::containerInvalid)
		return false;
	if (_socket_index == inventory::socketInvalid)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsContainer() const
{
	if (!_typeless && (_type_index == inventory::typeInvalid))
		return false;
	if (_slot_index == inventory::slotInvalid)
		return false;
	if (_container_index == inventory::containerInvalid)
		return false;
	if (_socket_index != inventory::socketInvalid)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsContainerSocket() const
{
	if (!_typeless && (_type_index == inventory::typeInvalid))
		return false;
	if (_slot_index == inventory::slotInvalid)
		return false;
	if (_container_index == inventory::containerInvalid)
		return false;
	if (_socket_index == inventory::socketInvalid)
		return false;

	return true;
}

EQEmu::InventorySlot EQEmu::InventorySlot::ToTopOwner() const
{
	return InventorySlot(_type_index, _slot_index);
}

EQEmu::InventorySlot EQEmu::InventorySlot::ToOwner() const
{
	if (IsSlot() || IsSlotSocket() || IsContainer())
		return InventorySlot(_type_index, _slot_index);

	if (IsContainerSocket())
		return InventorySlot(_type_index, _slot_index, _container_index);

	return InventorySlot();
}

const std::string EQEmu::InventorySlot::ToString() const
{
	return StringFormat("(%i%s, %i, %i, %i)", _type_index, (_typeless ? " [typeless]" : ""), _slot_index, _container_index, _socket_index);
}

const std::string EQEmu::InventorySlot::ToName() const
{
	return StringFormat("InventorySlot - _type_index: %i%s, _slot_index: %i, _container_index: %i, _socket_index: %i", _type_index, (_typeless ? " [typeless]" : ""), _slot_index, _container_index, _socket_index);
}

void EQEmu::InventorySlot::SetInvalidSlot()
{
	_type_index = inventory::typeInvalid;
	_slot_index = inventory::slotInvalid;
	_container_index = inventory::containerInvalid;
	_socket_index = inventory::socketInvalid;
}

//bool EQEmu::InventorySlot::IsBonusIndex(int16 slot_index)
//{
//	if ((slot_index >= inventory::EquipmentBegin) && (slot_index <= inventory::EquipmentEnd) && (slot_index != inventory::slotAmmo))
//		return true;
//
//	return false;
//}

//bool EQEmu::InventorySlot::IsBonusSlot() const
//{
//	if ((_type_index != inventory::typePossessions) || (_container_index != inventory::containerInvalid) || (_socket_index != inventory::socketInvalid))
//		return false;
//
//	return IsBonusIndex(_slot_index);
//}

bool inventory_slot_typeless_lessthan(const EQEmu::InventorySlot& lhs, const EQEmu::InventorySlot& rhs)
{
	if (lhs.SlotIndex() < rhs.SlotIndex())
		return true;

	if ((lhs.SlotIndex() == rhs.SlotIndex()) && (lhs.ContainerIndex() < rhs.ContainerIndex()))
		return true;

	if ((lhs.SlotIndex() == rhs.SlotIndex()) && (lhs.ContainerIndex() == rhs.ContainerIndex()) && (lhs.SocketIndex() < rhs.SocketIndex()))
		return true;

	return false;
}

bool EQEmu::InventorySlot::operator<(const InventorySlot& rhs) const
{
	if (Typeless() || rhs.Typeless())
		return inventory_slot_typeless_lessthan(*this, rhs);
	
	if (TypeIndex() < rhs.TypeIndex())
		return true;

	if ((TypeIndex() == rhs.TypeIndex()) && (SlotIndex() < rhs.SlotIndex()))
		return true;

	if ((TypeIndex() == rhs.TypeIndex()) && (SlotIndex() == rhs.SlotIndex()) && (ContainerIndex() < rhs.ContainerIndex()))
		return true;

	if ((TypeIndex() == rhs.TypeIndex()) && (SlotIndex() == rhs.SlotIndex()) && (ContainerIndex() == rhs.ContainerIndex()) && (SocketIndex() < rhs.SocketIndex()))
		return true;

	return false;
}

bool EQEmu::operator==(const InventorySlot& lhs, const InventorySlot& rhs)
{
	if (lhs.Typeless() || rhs.Typeless())
		return ((lhs.SlotIndex() == rhs.SlotIndex()) && (lhs.ContainerIndex() == rhs.ContainerIndex()) && (lhs.SocketIndex() == rhs.SocketIndex()));
	
	return ((lhs.TypeIndex() == rhs.TypeIndex()) && (lhs.SlotIndex() == rhs.SlotIndex()) && (lhs.ContainerIndex() == rhs.ContainerIndex()) && (lhs.SocketIndex() == rhs.SocketIndex()));
}
