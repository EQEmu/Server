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

#include "spells.h"
#include "../common/global_define.h"
#include "../common/shareddb.h"
#include "../common/spdat.h"
#include "../common/shared/shared_memory.h"
#include "../common/shared/shared_memory_map.h"

namespace eqs = eq::shared;

void LoadSpells(SharedDatabase *database, const std::string &prefix) {
	int records = database->GetSpellCount();
	int max_spell_id = database->GetMaxSpellID() + 1;
	auto size = records * sizeof(SPDat_Spell_Struct) * 125 / 100 + (64 * 1024);
	eqs::shared_memory shared("shared/spells", size);

	auto spells_result = shared.map<eqs::unordered_map<int, SPDat_Spell_Struct>>("spells");
	if (!spells_result) {
		//todo: get rid of the exceptions entirely.
		EQ_EXCEPT("Shared Memory", "Unable to map shared memory");
	}

	auto max_spell_id_result = shared.map<int>("max_spell_id");
	if (!max_spell_id_result) {
		EQ_EXCEPT("Shared Memory", "Unable to map shared memory");
	}

	eqs::unordered_map<int, SPDat_Spell_Struct>& spells_shm = spells_result.value();
	int& max_spell_id_shm = max_spell_id_result.value();
	max_spell_id_shm = max_spell_id;

	database->LoadSpells([&spells_shm](const SPDat_Spell_Struct &spell) {
		spells_shm.insert_or_assign(spell.id, spell);
	});
}

