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

#ifndef COMMON_ITEM_INSTANCE_H
#define COMMON_ITEM_INSTANCE_H

#include "item_data.h"
#include <memory>

namespace EQEmu
{
	uint32 GetNextItemInstanceSerial();

	class ItemContainer;
	class ItemInstance
	{
	public:
		ItemInstance(const ItemData* idata);
		ItemInstance(const ItemData* idata, const int16 charges);
		~ItemInstance();

		const ItemData *GetItem();
		const ItemData *GetBaseItem();
		const ItemData *GetBaseItem() const;

		std::shared_ptr<ItemInstance> Split(int charges);

		//Container
		std::shared_ptr<ItemInstance> Get(const int index);
		bool Put(const int index, std::shared_ptr<ItemInstance> inst);

		//Persistent State
		int16 GetCharges();
		int16 GetCharges() const;
		void SetCharges(const int16 charges);

		void SetColor(const uint32 color);

		bool GetAttuned();
		bool GetAttuned() const;
		void SetAttuned(const bool attuned);

		void SetCustomData(const std::string &custom_data);

		uint32 GetOrnamentIDFile();
		uint32 GetOrnamentIDFile() const;
		void SetOrnamentIDFile(const uint32 ornament_idfile);

		uint32 GetOrnamentIcon();
		uint32 GetOrnamentIcon() const;
		void SetOrnamentIcon(const uint32 ornament_icon);

		uint32 GetOrnamentHeroModel(int material_slot);
		uint32 GetOrnamentHeroModel(int material_slot) const;
		void SetOrnamentHeroModel(const uint32 ornament_hero_model);

		const char* GetTrackingID();
		const char* GetTrackingID() const;
		void SetTrackingID(const char *tracking_id);

		uint32 GetRecastTimestamp();
		uint32 GetRecastTimestamp() const;
		void SetRecastTimestamp(const uint32 recast_timestamp);

		//Merchant
		uint32 GetMerchantSlot();
		uint32 GetMerchantSlot() const;
		void SetMerchantSlot(const uint32 slot);

		uint32 GetMerchantCount();
		uint32 GetMerchantCount() const;
		void SetMerchantCount(const uint32 cnt);

		uint32 GetPrice();
		uint32 GetPrice() const;
		void SetPrice(const uint32 p);

		//Serial Number
		uint32 GetSerialNumber();
		uint32 GetSerialNumber() const;
		void SetSerialNumber(uint32 sn);

		//Basic Stats
		bool IsStackable();
		bool IsStackable() const;

		bool IsNoDrop();
		bool IsNoDrop() const;

		//Internal state
		//Used for low level operations such as encode/decode
		ItemContainer *GetContainer();
	private:
		struct impl;
		impl *impl_;
	};

} // EQEmu

#endif
