#ifndef CODE_LOOT_H
#define CODE_LOOT_H

#include <list>
#include <string>
#include "../common/types.h"

struct LootItem {
	uint32      item_id;
	int16       equip_slot;
	uint16      charges;
	uint16      lootslot;
	uint32      aug_1;
	uint32      aug_2;
	uint32      aug_3;
	uint32      aug_4;
	uint32      aug_5;
	uint32      aug_6;
	bool        attuned;
	std::string custom_data;
	uint32      ornamenticon{};
	uint32      ornamentidfile{};
	uint32      ornament_hero_model{};
	uint16      trivial_min_level;
	uint16      trivial_max_level;
	uint16      npc_min_level;
	uint16      npc_max_level;
};

typedef std::list<LootItem*> LootItems;


#endif //CODE_LOOT_H
