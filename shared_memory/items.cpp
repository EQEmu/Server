/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemulator.net)

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

#include "items.h"
#include "../common/global_define.h"
#include "../common/shareddb.h"
#include "../common/ipc_mutex.h"
#include "../common/memory_mapped_file.h"
#include "../common/eqemu_exception.h"
#include "../common/item_data.h"

void LoadItems(SharedDatabase *database, const std::string &prefix) {
	EQEmu::IPCMutex mutex("items");
	mutex.Lock();

	int32 items = -1;
	uint32 max_item = 0;
	database->GetItemsCount(items, max_item);
	if(items == -1) {
		EQ_EXCEPT("Shared Memory", "Unable to get any items from the database.");
	}

	uint32 size = static_cast<uint32>(EQEmu::FixedMemoryHashSet<EQEmu::ItemData>::estimated_size(items, max_item));

	auto Config = EQEmuConfig::get();
	std::string file_name = Config->SharedMemDir + prefix + std::string("items");
	EQEmu::MemoryMappedFile mmf(file_name, size);
	mmf.ZeroFile();

	void *ptr = mmf.Get();
	database->LoadItems(ptr, size, items, max_item);
	mutex.Unlock();
}
