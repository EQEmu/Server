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

#ifndef COMMON_INVENTORY_H
#define COMMON_INVENTORY_H

#include "item_container.h"
#include <string>

namespace EQEmu
{
	enum InventoryType : int
	{
		InvTypePersonal = 0,
		InvTypeBank,
		InvTypeSharedBank,
		InvTypeTrade,
		InvTypeWorld,
		InvTypeCursorBuffer,
		InvTypeTribute,
		InvTypeTrophyTribute,
		InvTypeGuildTribute
	};

	enum PersonaInventorylSlot : int
	{
		PersonalSlotCharm = 0,
		PersonalSlotEar1,
		PersonalSlotHead,
		PersonalSlotFace,
		PersonalSlotEar2,
		PersonalSlotNeck,
		PersonalSlotShoulders,
		PersonalSlotArms,
		PersonalSlotBack,
		PersonalSlotWrist1,
		PersonalSlotWrist2,
		PersonalSlotRange,
		PersonalSlotHands,
		PersonalSlotPrimary,
		PersonalSlotSecondary,
		PersonalSlotFinger1,
		PersonalSlotFinger2,
		PersonalSlotChest,
		PersonalSlotLegs,
		PersonalSlotFeet,
		PersonalSlotWaist,
		PersonalSlotPowerSource,
		PersonalSlotAmmo,
		PersonalSlotGeneral1,
		PersonalSlotGeneral2,
		PersonalSlotGeneral3,
		PersonalSlotGeneral4,
		PersonalSlotGeneral5,
		PersonalSlotGeneral6,
		PersonalSlotGeneral7,
		PersonalSlotGeneral8,
		PersonalSlotGeneral9,
		PersonalSlotGeneral10,
		PersonalSlotCursor
	};

	class InventorySlot
	{
	public:
		InventorySlot() : type_(-1), slot_(-1), bag_index_(-1), aug_index_(-1) { }
		InventorySlot(int type, int slot) 
			: type_(type), slot_(slot), bag_index_(-1), aug_index_(-1) { }
		InventorySlot(int type, int slot, int bag_index) 
			: type_(type), slot_(slot), bag_index_(bag_index), aug_index_(-1) { }
		InventorySlot(int type, int slot, int bag_index, int aug_index) 
			: type_(type), slot_(slot), bag_index_(bag_index), aug_index_(aug_index) { }

		bool IsValid() const;
		bool IsDelete() const;
		bool IsBank() const;
		bool IsCursor() const;
		bool IsEquipment() const;
		bool IsGeneral() const;
		bool IsWeapon() const;
		bool IsTrade() const;

		const std::string ToString() const;

		inline int Type() { return type_; }
		inline int Type() const { return type_; }
		inline int Slot() { return slot_; }
		inline int Slot() const { return slot_; }
		inline int BagIndex() { return bag_index_; }
		inline int BagIndex() const { return bag_index_; }
		inline int AugIndex() { return aug_index_; }
		inline int AugIndex() const { return aug_index_; }

	private:
		int type_;
		int slot_;
		int bag_index_;
		int aug_index_;
	};

	inline bool operator==(const InventorySlot &lhs, const InventorySlot &rhs) { 
		return lhs.Type() == rhs.Type() && 
			lhs.Slot() == rhs.Slot() && 
			lhs.BagIndex() == rhs.BagIndex() &&
			lhs.AugIndex() == rhs.AugIndex(); }
	inline bool operator!=(const InventorySlot &lhs, const InventorySlot &rhs) { return !(lhs == rhs); }

	class InventoryDataModel;
	class Inventory
	{
	public:
		Inventory(int race, int class_, int deity);
		~Inventory();

		void SetRace(int race);
		void SetClass(int class_);
		void SetDeity(int deity);
		void SetDataModel(InventoryDataModel *dm);

		std::shared_ptr<ItemInstance> Get(const InventorySlot &slot);
		bool Put(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst);
		bool Swap(const InventorySlot &src, const InventorySlot &dest, int charges);
		bool Summon(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst);
		bool PushToCursorBuffer(std::shared_ptr<ItemInstance> inst);
		bool PopFromCursorBuffer();

		//utility
		static int CalcMaterialFromSlot(const InventorySlot &slot);
		static InventorySlot CalcSlotFromMaterial(int material);
		bool CanEquip(std::shared_ptr<EQEmu::ItemInstance> inst, const EQEmu::InventorySlot &slot);
		bool CheckLoreConflict(const ItemData *item);
		bool Serialize(MemoryBuffer &buf);

		//testing
		void Interrogate();
	private:
		bool _swap(const InventorySlot &src, const InventorySlot &dest);
		bool _destroy(const InventorySlot &slot);

		struct impl;
		impl *impl_;
	};

} // EQEmu

#endif
