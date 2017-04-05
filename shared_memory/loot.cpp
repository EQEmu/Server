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

#include "loot.h"
#include "../common/global_define.h"
#include "../common/shareddb.h"
#include "../common/ipc_mutex.h"
#include "../common/memory_mapped_file.h"
#include "../common/eqemu_exception.h"
#include "../common/fixed_memory_variable_hash_set.h"
#include "../common/loottable.h"

void LoadLoot(SharedDatabase *database, const std::string &prefix) {
	EQEmu::IPCMutex mutex("loot");
	mutex.Lock();

	uint32 loot_table_count, loot_table_max, loot_table_entries_count;
	uint32 loot_drop_count, loot_drop_max, loot_drop_entries_count;
	database->GetLootTableInfo(loot_table_count, loot_table_max, loot_table_entries_count);
	database->GetLootDropInfo(loot_drop_count, loot_drop_max, loot_drop_entries_count);

	uint32 loot_table_size = (3 * sizeof(uint32)) +						//header
		((loot_table_max + 1) * sizeof(uint32)) +						//offset list
		(loot_table_count * sizeof(LootTable_Struct)) +					//loot table headers
		(loot_table_entries_count * sizeof(LootTableEntries_Struct));	//number of loot table entries

	uint32 loot_drop_size = (3 * sizeof(uint32)) +					//header
		((loot_drop_max + 1) * sizeof(uint32)) +					//offset list
		(loot_drop_count * sizeof(LootDrop_Struct)) +				//loot table headers
		(loot_drop_entries_count * sizeof(LootDropEntries_Struct));	//number of loot table entries

	auto Config = EQEmuConfig::get();
	std::string file_name_lt = Config->SharedMemDir + prefix + std::string("loot_table");
	std::string file_name_ld = Config->SharedMemDir + prefix + std::string("loot_drop");

	EQEmu::MemoryMappedFile mmf_loot_table(file_name_lt, loot_table_size);
	EQEmu::MemoryMappedFile mmf_loot_drop(file_name_ld, loot_drop_size);
	mmf_loot_table.ZeroFile();
	mmf_loot_drop.ZeroFile();

	EQEmu::FixedMemoryVariableHashSet<LootTable_Struct> loot_table_hash(reinterpret_cast<byte*>(mmf_loot_table.Get()),
		loot_table_size, loot_table_max);

	EQEmu::FixedMemoryVariableHashSet<LootDrop_Struct> loot_drop_hash(reinterpret_cast<byte*>(mmf_loot_drop.Get()),
		loot_drop_size, loot_drop_max);

	database->LoadLootTables(mmf_loot_table.Get(), loot_table_max);
	database->LoadLootDrops(mmf_loot_drop.Get(), loot_drop_max);
	mutex.Unlock();
}
