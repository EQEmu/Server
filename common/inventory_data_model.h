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

#ifndef COMMON_INVENTORY_DATA_MODEL_H
#define COMMON_INVENTORY_DATA_MODEL_H

#include "inventory.h"

namespace EQEmu
{
	class InventoryDataModel
	{
	public:
		InventoryDataModel() { }
		virtual ~InventoryDataModel() { }
		
		virtual void Begin() = 0;
		virtual bool Commit() = 0;
		virtual void Rollback() = 0;
		virtual void Insert(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst) = 0;
		virtual void Delete(const InventorySlot &slot) = 0;
	};
} // EQEmu

#endif
