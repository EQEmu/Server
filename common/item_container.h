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

#ifndef COMMON_ITEM_CONTAINER_H
#define COMMON_ITEM_CONTAINER_H

#include "item_instance.h"
#include "item_container_serialization_strategy.h"
#include "memory_buffer.h"
#include <memory>
#include <map>

namespace EQEmu
{
	class ItemContainerSerializationStrategy;
	class ItemContainer
	{
	public:
		typedef std::map<int, std::shared_ptr<ItemInstance>>::const_iterator ItemContainerIter;

		ItemContainer();
		ItemContainer(ItemContainerSerializationStrategy *strategy);
		~ItemContainer();
		ItemContainer(ItemContainer &&other);
		ItemContainer& operator=(ItemContainer &&other);

		std::shared_ptr<ItemInstance> Get(const int slot_id);
		bool Put(const int slot_id, std::shared_ptr<ItemInstance> inst);
		bool Delete(const int slot_id);
		uint32 Size();
		uint32 Size() const;

		//Low level interface for encode/decode
		bool Serialize(MemoryBuffer &buf, int container_number);
		ItemContainerIter Begin();
		ItemContainerIter End();

		//testing
		void Interrogate(int level);
	protected:
		struct impl;
		impl *impl_;

	private:
		ItemContainer(const ItemContainer &other);
		ItemContainer& operator=(const ItemContainer &other);
	};
} // EQEmu

#endif
