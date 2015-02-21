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
	class ItemInstance
	{
	public:
		ItemInstance();
		ItemInstance(const ItemData* idata);
		ItemInstance(const ItemData* idata, const int16 charges);
		~ItemInstance();

		const ItemData *GetItem();
		std::shared_ptr<ItemInstance> Get(const int index);
		bool Put(const int index, std::shared_ptr<ItemInstance> inst);

		void SetCharges(const int16 charges);
		void SetColor(const uint32 color);
		void SetAttuned(const bool attuned);
		void SetCustomData(const std::string &custom_data);
		void SetOrnamentIDFile(const uint32 ornament_idfile);
		void SetOrnamentIcon(const uint32 ornament_icon);
		void SetOrnamentHeroModel(const uint32 ornament_hero_model);
		void SetTrackingID(const uint64 tracking_id);
		void SetRecastTimestamp(const uint32 recast_timestamp);
	private:
		struct impl;
		impl *impl_;
	};

} // EQEmu

#endif
