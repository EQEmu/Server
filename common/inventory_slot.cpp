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
	case invslot::slotHead:
		return textures::armorHead;
	case invslot::slotChest:
		return textures::armorChest;
	case invslot::slotArms:
		return textures::armorArms;
	case invslot::slotWrist1:
		return textures::armorWrist;
	case invslot::slotHands:
		return textures::armorHands;
	case invslot::slotLegs:
		return textures::armorLegs;
	case invslot::slotFeet:
		return textures::armorFeet;
	case invslot::slotPrimary:
		return textures::weaponPrimary;
	case invslot::slotSecondary:
		return textures::weaponSecondary;
	default:
		return textures::textureInvalid;
	}
}

int8 EQEmu::inventory::ConvertEquipmentSlotToTextureIndex(const InventorySlot& inventory_slot)
{
	if ((!inventory_slot.Typeless() && !inventory_slot.IsTypeIndex(invtype::typePossessions)) || !inventory_slot.IsContainerIndex(invbag::SLOT_INVALID) || !inventory_slot.IsSocketIndex(invaug::SOCKET_INVALID))
		return textures::textureInvalid;

	return ConvertEquipmentIndexToTextureIndex(inventory_slot.SlotIndex());
}

int16 EQEmu::inventory::ConvertTextureIndexToEquipmentIndex(int8 texture_index)
{
	switch (texture_index) {
	case textures::armorHead:
		return invslot::slotHead;
	case textures::armorChest:
		return invslot::slotChest;
	case textures::armorArms:
		return invslot::slotArms;
	case textures::armorWrist:
		return invslot::slotWrist1;
	case textures::armorHands:
		return invslot::slotHands;
	case textures::armorLegs:
		return invslot::slotLegs;
	case textures::armorFeet:
		return invslot::slotFeet;
	case textures::weaponPrimary:
		return invslot::slotPrimary;
	case textures::weaponSecondary:
		return invslot::slotSecondary;
	default:
		return invslot::SLOT_INVALID;
	}
}

bool EQEmu::InventorySlot::IsValidSlot() const
{
	if (_typeless)
		return false;
	
	int16 slot_count = invtype::GetInvTypeSize(_type_index);
	if (!slot_count || _slot_index < invslot::SLOT_BEGIN || _slot_index >= slot_count)
		return false;

	if (_container_index < invbag::SLOT_INVALID || _container_index >= invbag::SLOT_COUNT)
		return false;

	if (_socket_index < invaug::SOCKET_INVALID || _socket_index >= invaug::SOCKET_COUNT)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsDeleteSlot() const
{
	if (_typeless)
		return (_slot_index == invslot::SLOT_INVALID && _container_index == invbag::SLOT_INVALID && _socket_index == invaug::SOCKET_INVALID);
	else
		return (_type_index == invtype::TYPE_INVALID && _slot_index == invslot::SLOT_INVALID && _container_index == invbag::SLOT_INVALID && _socket_index == invaug::SOCKET_INVALID);
}

bool EQEmu::InventorySlot::IsEquipmentIndex(int16 slot_index)
{
	/*if (slot_index < inventory::EquipmentBegin || slot_index > inventory::EquipmentEnd)
		return false;*/
	if ((slot_index < invslot::EQUIPMENT_BEGIN || slot_index > invslot::EQUIPMENT_END) && slot_index != invslot::SLOT_POWER_SOURCE)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsGeneralIndex(int16 slot_index)
{
	/*if (slot_index < inventory::GeneralBegin || slot_index > inventory::GeneralEnd)
		return false;*/
	if (slot_index < invslot::GENERAL_BEGIN || slot_index > invslot::GENERAL_END)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsCursorIndex(int16 slot_index)
{
	/*if (slot_index != inventory::slotCursor)
		return false;*/
	if (slot_index != invslot::slotCursor)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsWeaponIndex(int16 slot_index)
{
	/*if ((slot_index != inventory::slotRange) && (slot_index != inventory::slotPrimary) && (slot_index != inventory::slotSecondary))
		return false;*/
	if ((slot_index != invslot::slotRange) && (slot_index != invslot::slotPrimary) && (slot_index != invslot::slotSecondary))
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsTextureIndex(int16 slot_index)
{
	switch (slot_index) {
	case invslot::slotHead:
	case invslot::slotChest:
	case invslot::slotArms:
	case invslot::slotWrist1:
	case invslot::slotHands:
	case invslot::slotLegs:
	case invslot::slotFeet:
	case invslot::slotPrimary:
	case invslot::slotSecondary:
		return true;
	default:
		return false;
	}
}

bool EQEmu::InventorySlot::IsTintableIndex(int16 slot_index)
{
	switch (slot_index) {
	case invslot::slotHead:
	case invslot::slotChest:
	case invslot::slotArms:
	case invslot::slotWrist1:
	case invslot::slotHands:
	case invslot::slotLegs:
	case invslot::slotFeet:
		return true;
	default:
		return false;
	}
}

bool EQEmu::InventorySlot::IsEquipmentSlot() const
{
	if (!_typeless && (_type_index != invtype::typePossessions))
		return false;

	if ((_container_index != invbag::SLOT_INVALID) || (_socket_index != invaug::SOCKET_INVALID))
		return false;

	return IsEquipmentIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsGeneralSlot() const
{
	if (!_typeless && (_type_index != invtype::typePossessions))
		return false;

	if ((_container_index != invbag::SLOT_INVALID) || (_socket_index != invaug::SOCKET_INVALID))
		return false;

	return IsGeneralIndex(_socket_index);
}

bool EQEmu::InventorySlot::IsCursorSlot() const
{
	if (!_typeless && (_type_index != invtype::typePossessions))
		return false;

	if ((_container_index != invbag::SLOT_INVALID) || (_socket_index != invaug::SOCKET_INVALID))
		return false;

	return IsCursorIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsWeaponSlot() const
{
	if (!_typeless && (_type_index != invtype::typePossessions))
		return false;

	if ((_container_index != invbag::SLOT_INVALID) || (_socket_index != invaug::SOCKET_INVALID))
		return false;

	return IsWeaponIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsTextureSlot() const
{
	if (!_typeless && (_type_index != invtype::typePossessions))
		return false;

	if ((_container_index != invbag::SLOT_INVALID) || (_socket_index != invaug::SOCKET_INVALID))
		return false;

	return IsTextureIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsTintableSlot() const
{
	if (!_typeless && (_type_index != invtype::typePossessions))
		return false;

	if ((_container_index != invbag::SLOT_INVALID) || (_socket_index != invaug::SOCKET_INVALID))
		return false;

	return IsTintableIndex(_slot_index);
}

bool EQEmu::InventorySlot::IsSlot() const
{
	if (!_typeless && (_type_index == invtype::TYPE_INVALID))
		return false;
	if (_slot_index == invslot::SLOT_INVALID)
		return false;
	if (_container_index != invbag::SLOT_INVALID)
		return false;
	if (_socket_index != invaug::SOCKET_INVALID)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsSlotSocket() const
{
	if (!_typeless && (_type_index == invtype::TYPE_INVALID))
		return false;
	if (_slot_index == invslot::SLOT_INVALID)
		return false;
	if (_container_index != invbag::SLOT_INVALID)
		return false;
	if (_socket_index == invaug::SOCKET_INVALID)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsContainer() const
{
	if (!_typeless && (_type_index == invtype::TYPE_INVALID))
		return false;
	if (_slot_index == invslot::SLOT_INVALID)
		return false;
	if (_container_index == invbag::SLOT_INVALID)
		return false;
	if (_socket_index != invaug::SOCKET_INVALID)
		return false;

	return true;
}

bool EQEmu::InventorySlot::IsContainerSocket() const
{
	if (!_typeless && (_type_index == invtype::TYPE_INVALID))
		return false;
	if (_slot_index == invslot::SLOT_INVALID)
		return false;
	if (_container_index == invbag::SLOT_INVALID)
		return false;
	if (_socket_index == invaug::SOCKET_INVALID)
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
	_type_index = invtype::TYPE_INVALID;
	_slot_index = invslot::SLOT_INVALID;
	_container_index = invbag::SLOT_INVALID;
	_socket_index = invaug::SOCKET_INVALID;
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
