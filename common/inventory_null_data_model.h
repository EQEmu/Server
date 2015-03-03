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

#ifndef COMMON_INVENTORY_NULL_DATA_MODEL_H
#define COMMON_INVENTORY_NULL_DATA_MODEL_H

#include "inventory_data_model.h"

namespace EQEmu
{
	class InventoryNullDataModel : public InventoryDataModel
	{
	public:
		InventoryNullDataModel() { }
		virtual ~InventoryNullDataModel() { }
		
		virtual void Begin() { printf("NDM: Begin\n"); }
		virtual void Commit() { printf("NDM: Commit\n"); }
		virtual void Rollback() { printf("NDM: Rollback\n"); }
		virtual bool Insert(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst) { printf("NDM: Insert %s %s\n", slot.ToString().c_str(), inst ? inst->GetBaseItem()->Name : "Null" ); return true; }
		virtual bool Delete(const InventorySlot &slot) { printf("NDM: Delete %s\n", slot.ToString().c_str()); return true; }
	};
} // EQEmu

#endif
