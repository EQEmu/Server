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

#ifndef COMMON_INVENTORY_DB_DATA_MODEL_H
#define COMMON_INVENTORY_DB_DATA_MODEL_H

#include "inventory_data_model.h"

class SharedDatabase;

namespace EQEmu
{
	class InventoryDatabaseDataModel : public InventoryDataModel
	{
	public:
		InventoryDatabaseDataModel(SharedDatabase *db, uint32 char_id);
		virtual ~InventoryDatabaseDataModel();
		
		virtual void Begin();
		virtual bool Commit();
		virtual void Rollback();
		virtual void Insert(const InventorySlot &slot, std::shared_ptr<ItemInstance> inst);
		virtual void Delete(const InventorySlot &slot);
	private:
		struct impl;
		impl *impl_;
	};
} // EQEmu

#endif
