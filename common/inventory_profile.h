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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  04111-1307  USA
*/

// @merth notes:
// These classes could be optimized with database reads/writes by storing
// a status flag indicating how object needs to interact with database

#ifndef COMMON_INVENTORY_PROFILE_H
#define COMMON_INVENTORY_PROFILE_H


#include "item_instance.h"

#include <list>


//FatherNitwit: location bits for searching specific
//places with HasItem() and HasItemByUse()
enum {
	invWhereWorn		= 0x01,
	invWherePersonal	= 0x02,	//in the character's inventory
	invWhereBank		= 0x04,
	invWhereSharedBank	= 0x08,
	invWhereTrading		= 0x10,
	invWhereCursor		= 0x20
};

// ########################################
// Class: Queue
//	Queue that allows a read-only iterator
class ItemInstQueue
{
public:
	~ItemInstQueue();
	/////////////////////////
	// Public Methods
	/////////////////////////

	inline std::list<EQEmu::ItemInstance*>::const_iterator cbegin() { return m_list.cbegin(); }
	inline std::list<EQEmu::ItemInstance*>::const_iterator cend() { return m_list.cend(); }

	inline int size() { return static_cast<int>(m_list.size()); } // TODO: change to size_t
	inline bool empty() { return m_list.empty(); }

	void push(EQEmu::ItemInstance* inst);
	void push_front(EQEmu::ItemInstance* inst);
	EQEmu::ItemInstance* pop();
	EQEmu::ItemInstance* pop_back();
	EQEmu::ItemInstance* peek_front() const;

protected:
	/////////////////////////
	// Protected Members
	/////////////////////////

	std::list<EQEmu::ItemInstance*> m_list;
};

// ########################################
// Class: EQEmu::InventoryProfile
//	Character inventory
namespace EQEmu
{
	class InventoryProfile
	{
		friend class ItemInstance;
	public:
		///////////////////////////////
		// Public Methods
		///////////////////////////////

		InventoryProfile() {
			m_mob_version = versions::MobVersion::Unknown;
			m_mob_version_set = false;
			m_lookup = inventory::Lookup(versions::MobVersion::Unknown);
		}
		~InventoryProfile();

		bool SetInventoryVersion(versions::MobVersion inventory_version) {
			if (!m_mob_version_set) {
				m_mob_version = versions::ValidateMobVersion(inventory_version);
				m_lookup = inventory::Lookup(m_mob_version);
				m_mob_version_set = true;
				return true;
			}
			else {
				m_lookup = inventory::Lookup(versions::MobVersion::Unknown);
				return false;
			}
		}
		bool SetInventoryVersion(versions::ClientVersion client_version) { return SetInventoryVersion(versions::ConvertClientVersionToMobVersion(client_version)); }

		versions::MobVersion InventoryVersion() { return m_mob_version; }

		static void CleanDirty();
		static void MarkDirty(ItemInstance *inst);

		// Retrieve a writeable item at specified slot
		ItemInstance* GetItem(int16 slot_id) const;
		ItemInstance* GetItem(int16 slot_id, uint8 bagidx) const;

		inline std::list<ItemInstance*>::const_iterator cursor_cbegin() { return m_cursor.cbegin(); }
		inline std::list<ItemInstance*>::const_iterator cursor_cend() { return m_cursor.cend(); }

		inline int CursorSize() { return m_cursor.size(); }
		inline bool CursorEmpty() { return m_cursor.empty(); }

		// Retrieve a read-only item from inventory
		inline const ItemInstance* operator[](int16 slot_id) const { return GetItem(slot_id); }

		// Add item to inventory
		int16 PutItem(int16 slot_id, const ItemInstance& inst);

		// Add item to cursor queue
		int16 PushCursor(const ItemInstance& inst);

		// Get cursor item in front of queue
		ItemInstance* GetCursorItem();

		// Swap items in inventory
		enum SwapItemFailState : int8 { swapInvalid = -1, swapPass = 0, swapNotAllowed, swapNullData, swapRaceClass, swapDeity, swapLevel };
		bool SwapItem(int16 slot_a, int16 slot_b, SwapItemFailState& fail_state, uint16 race_id = 0, uint8 class_id = 0, uint16 deity_id = 0, uint8 level = 0);

		// Remove item from inventory
		bool DeleteItem(int16 slot_id, uint8 quantity = 0);

		// Checks All items in a bag for No Drop
		bool CheckNoDrop(int16 slot_id, bool recurse = true);

		// Remove item from inventory (and take control of memory)
		ItemInstance* PopItem(int16 slot_id);

		// Check whether there is space for the specified number of the specified item.
		bool HasSpaceForItem(const ItemData *ItemToTry, int16 Quantity);

		// Check whether item exists in inventory
		// where argument specifies OR'd list of invWhere constants to look
		int16 HasItem(uint32 item_id, uint8 quantity = 0, uint8 where = 0xFF);

		// Check whether item exists in inventory
		// where argument specifies OR'd list of invWhere constants to look
		int16 HasItemByUse(uint8 use, uint8 quantity = 0, uint8 where = 0xFF);

		// Check whether item exists in inventory
		// where argument specifies OR'd list of invWhere constants to look
		int16 HasItemByLoreGroup(uint32 loregroup, uint8 where = 0xFF);

		// Locate an available inventory slot
		int16 FindFreeSlot(bool for_bag, bool try_cursor, uint8 min_size = 0, bool is_arrow = false);
		int16 FindFreeSlotForTradeItem(const ItemInstance* inst, int16 general_start = legacy::GENERAL_BEGIN, uint8 bag_start = inventory::containerBegin);

		// Calculate slot_id for an item within a bag
		static int16 CalcSlotId(int16 slot_id); // Calc parent bag's slot_id
		static int16 CalcSlotId(int16 bagslot_id, uint8 bagidx); // Calc slot_id for item inside bag
		static uint8 CalcBagIdx(int16 slot_id); // Calc bagidx for slot_id
		static int16 CalcSlotFromMaterial(uint8 material);
		static uint8 CalcMaterialFromSlot(int16 equipslot);

		static bool CanItemFitInContainer(const ItemData *ItemToTry, const ItemData *Container);

		//  Test for valid inventory casting slot
		bool SupportsClickCasting(int16 slot_id);
		bool SupportsPotionBeltCasting(int16 slot_id);

		// Test whether a given slot can support a container item
		static bool SupportsContainers(int16 slot_id);

		int GetSlotByItemInst(ItemInstance *inst);

		uint8 FindBrightestLightType();

		void dumpEntireInventory();
		void dumpWornItems();
		void dumpInventory();
		void dumpBankItems();
		void dumpSharedBankItems();

		void SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, std::string value);
		void SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, int value);
		void SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, float value);
		void SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, bool value);
		std::string GetCustomItemData(int16 slot_id, std::string identifier);
	protected:
		///////////////////////////////
		// Protected Methods
		///////////////////////////////

		int GetSlotByItemInstCollection(const std::map<int16, ItemInstance*> &collection, ItemInstance *inst);
		void dumpItemCollection(const std::map<int16, ItemInstance*> &collection);
		void dumpBagContents(ItemInstance *inst, std::map<int16, ItemInstance*>::const_iterator *it);

		// Retrieves item within an inventory bucket
		ItemInstance* _GetItem(const std::map<int16, ItemInstance*>& bucket, int16 slot_id) const;

		// Private "put" item into bucket, without regard for what is currently in bucket
		int16 _PutItem(int16 slot_id, ItemInstance* inst);

		// Checks an inventory bucket for a particular item
		int16 _HasItem(std::map<int16, ItemInstance*>& bucket, uint32 item_id, uint8 quantity);
		int16 _HasItem(ItemInstQueue& iqueue, uint32 item_id, uint8 quantity);
		int16 _HasItemByUse(std::map<int16, ItemInstance*>& bucket, uint8 use, uint8 quantity);
		int16 _HasItemByUse(ItemInstQueue& iqueue, uint8 use, uint8 quantity);
		int16 _HasItemByLoreGroup(std::map<int16, ItemInstance*>& bucket, uint32 loregroup);
		int16 _HasItemByLoreGroup(ItemInstQueue& iqueue, uint32 loregroup);


		// Player inventory
		std::map<int16, ItemInstance*>	m_worn;		// Items worn by character
		std::map<int16, ItemInstance*>	m_inv;		// Items in character personal inventory
		std::map<int16, ItemInstance*>	m_bank;		// Items in character bank
		std::map<int16, ItemInstance*>	m_shbank;	// Items in character shared bank
		std::map<int16, ItemInstance*>	m_trade;	// Items in a trade session
		::ItemInstQueue					m_cursor;	// Items on cursor: FIFO

	private:
		// Active mob version
		versions::MobVersion m_mob_version;
		bool m_mob_version_set;
		const inventory::LookupEntry* m_lookup;
	};
}

#endif /*COMMON_INVENTORY_PROFILE_H*/
