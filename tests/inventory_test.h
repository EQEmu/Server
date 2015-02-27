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

#ifndef __EQEMU_TESTS_INVENTORY_H
#define __EQEMU_TESTS_INVENTORY_H

#include "cppunit/cpptest.h"
#include "../common/inventory.h"
#include <string.h>

class InventoryTest : public Test::Suite {
	typedef void(InventoryTest::*TestFunction)(void);
public:
	InventoryTest() : inv(1, 1) {
		InitContainer();
		InitArmor();
		InitAugment();
		InitStackable();
		InitInventory();
		TEST_ADD(InventoryTest::InventoryVerifyInitialItemsTest);
		TEST_ADD(InventoryTest::InventoryCanEquipTest);
	}

	~InventoryTest() {
	}

private:	
	void InitContainer() {
		memset(&container, 0, sizeof(container));
		strcpy(container.Name, "Backpack");
		strcpy(container.IDFile, "IT64");
		container.ID = 1000;
		container.BagSize = 3;
		container.BagSlots = 8;
		container.BagType = 5;
		container.BagWR = 50;
		container.ItemClass = 1;
		container.Classes = 65535U;
		container.Focus.Effect = -1;
		container.ItemType = 11;
		container.NoDrop = 1;
		container.NoRent = 1;
		container.Races = 131071U;
		container.Size = 3;
		container.SkillModType = -1;
		container.Click.Effect = -1;
		container.Weight = 30;
		container.StackSize = 1;
		container.Proc.Effect = -1;
		container.Worn.Effect = -1;
		container.Scroll.Effect = -1;
	}

	void InitArmor() {
		memset(&armor, 0, sizeof(armor));
		strcpy(armor.Name, "Cloth Shirt");
		strcpy(armor.IDFile, "IT64");
		armor.ID = 1001;
		armor.AC = 4;
		armor.AugSlotType[0] = 7;
		for(int i = 0; i < 6; ++i)
			armor.AugSlotVisible[i] = 1;
		armor.Size = 2;
		armor.Slots = 131072;
		armor.Classes = 65535U;
		armor.Focus.Effect = -1;
		armor.ItemType = 10;
		armor.NoDrop = 1;
		armor.NoRent = 1;
		armor.Races = 131071U;
		armor.SkillModType = -1;
		armor.Click.Effect = -1;
		armor.Weight = 8;
		armor.StackSize = 1;
		armor.Proc.Effect = -1;
		armor.Worn.Effect = -1;
		armor.Scroll.Effect = -1;
	}

	void InitAugment() {
		memset(&augment, 0, sizeof(augment));
		strcpy(augment.Name, "Cloth Augment");
		strcpy(augment.IDFile, "IT64");
		augment.ID = 1002;
		augment.AWis = 10;
		augment.AInt = 10;
		augment.AugType = 64;
		augment.Slots = 2072574;
		augment.Classes = 65535U;
		augment.Focus.Effect = -1;
		augment.ItemType = 54;
		augment.NoDrop = 1;
		augment.NoRent = 1;
		augment.Races = 131071U;
		augment.SkillModType = -1;
		augment.Click.Effect = -1;
		augment.Weight = 5;
		augment.StackSize = 1;
		augment.Size = 1;
		augment.Proc.Effect = -1;
		augment.Worn.Effect = -1;
		augment.Scroll.Effect = -1;
	}

	void InitStackable() {
		memset(&stackable, 0, sizeof(stackable));
		strcpy(stackable.Name, "Stackable Item");
		strcpy(stackable.IDFile, "IT64");
		stackable.ID = 1003;
		stackable.Classes = 65535U;
		stackable.Focus.Effect = -1;
		stackable.ItemType = 54;
		stackable.NoDrop = 1;
		stackable.NoRent = 1;
		stackable.Races = 131071U;
		stackable.SkillModType = -1;
		stackable.Click.Effect = -1;
		stackable.Weight = 5;
		stackable.StackSize = 100;
		stackable.Stackable = 1;
		stackable.Size = 1;
		stackable.Proc.Effect = -1;
		stackable.Worn.Effect = -1;
		stackable.Scroll.Effect = -1;
	}

	void InitInventory()
	{
		std::shared_ptr<EQEmu::ItemInstance> m_bag(new EQEmu::ItemInstance(&container));
		std::shared_ptr<EQEmu::ItemInstance> m_armor(new EQEmu::ItemInstance(&armor));
		std::shared_ptr<EQEmu::ItemInstance> m_augment(new EQEmu::ItemInstance(&augment));
		std::shared_ptr<EQEmu::ItemInstance> m_stackable(new EQEmu::ItemInstance(&stackable, 45));
		inv.Put(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral1), m_bag);
		inv.Put(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral1, 0), m_armor);
		inv.Put(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral1, 1), m_augment);
		inv.Put(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral1, 7), m_stackable);
	}

	void InventoryVerifyInitialItemsTest()
	{
		auto m_bag = inv.Get(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral1));
		TEST_ASSERT(m_bag);
		TEST_ASSERT(m_bag->GetItem());
		TEST_ASSERT(m_bag->GetItem()->ID == 1000);
		
		auto m_armor = m_bag->Get(0);
		TEST_ASSERT(m_armor);
		TEST_ASSERT(m_armor->GetItem());
		TEST_ASSERT(m_armor->GetItem()->ID == 1001);
		
		auto m_augment = m_bag->Get(1);
		TEST_ASSERT(m_augment);
		TEST_ASSERT(m_augment->GetItem());
		TEST_ASSERT(m_augment->GetItem()->ID == 1002);
		
		auto m_stackable = m_bag->Get(7);
		TEST_ASSERT(m_stackable);
		TEST_ASSERT(m_stackable->GetItem());
		TEST_ASSERT(m_stackable->GetItem()->ID == 1003);
	}

	void InventoryCanEquipTest() {
		auto m_bag = inv.Get(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral1));
		TEST_ASSERT(m_bag);
		TEST_ASSERT(m_bag->GetItem());
		TEST_ASSERT(m_bag->GetItem()->ID == 1000);

		auto m_armor = m_bag->Get(0);
		TEST_ASSERT(m_armor);
		TEST_ASSERT(m_armor->GetItem());
		TEST_ASSERT(m_armor->GetItem()->ID == 1001);

		auto can_equip = inv.CanEquip(m_armor, EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotChest));
		TEST_ASSERT(can_equip);

		can_equip = inv.CanEquip(m_armor, EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotWaist));
		TEST_ASSERT(!can_equip);

		armor.Classes -= 1;
		can_equip = inv.CanEquip(m_armor, EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotChest));
		TEST_ASSERT(!can_equip);
		armor.Classes += 1;

		armor.Races -= 1;
		can_equip = inv.CanEquip(m_armor, EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotChest));
		TEST_ASSERT(!can_equip);
		armor.Races += 1;
	}

	void InventorySwapItemsTest()
	{
		auto swap_result = inv.Swap(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral1),
									EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral2), 0);
		TEST_ASSERT(swap_result == true);

		auto m_bag = inv.Get(EQEmu::InventorySlot(EQEmu::InvTypePersonal, EQEmu::PersonalSlotGeneral2));
		TEST_ASSERT(m_bag);
		TEST_ASSERT(m_bag->GetItem());
		TEST_ASSERT(m_bag->GetItem()->ID == 1000);

		auto m_armor = m_bag->Get(0);
		TEST_ASSERT(m_armor);
		TEST_ASSERT(m_armor->GetItem());
		TEST_ASSERT(m_armor->GetItem()->ID == 1001);

		auto m_augment = m_bag->Get(1);
		TEST_ASSERT(m_augment);
		TEST_ASSERT(m_augment->GetItem());
		TEST_ASSERT(m_augment->GetItem()->ID == 1002);

		auto m_stackable = m_bag->Get(7);
		TEST_ASSERT(m_stackable);
		TEST_ASSERT(m_stackable->GetItem());
		TEST_ASSERT(m_stackable->GetItem()->ID == 1003);
	}

	EQEmu::Inventory inv;
	ItemData container;
	ItemData armor;
	ItemData augment;
	ItemData stackable;
};

#endif
