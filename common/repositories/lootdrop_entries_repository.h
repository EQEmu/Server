#ifndef EQEMU_LOOTDROP_ENTRIES_REPOSITORY_H
#define EQEMU_LOOTDROP_ENTRIES_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_lootdrop_entries_repository.h"

class LootdropEntriesRepository: public BaseLootdropEntriesRepository {
public:

	static LootdropEntries NewNpcEntity()
	{
		LootdropEntries e{};

		e.lootdrop_id       = 0;
		e.item_id           = 0;
		e.item_charges      = 1;
		e.equip_item        = 1;
		e.chance            = 0;
		e.disabled_chance   = 0;
		e.trivial_min_level = 0;
		e.trivial_max_level = 0;
		e.multiplier        = 0;
		e.npc_min_level     = 0;
		e.npc_max_level     = 0;

		return e;
	}

};

#endif //EQEMU_LOOTDROP_ENTRIES_REPOSITORY_H
