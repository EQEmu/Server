#include "bazaar.h"

#include "../../common/item_instance.h"
#include "repositories/trader_repository.h"
#include <memory>

std::vector<BazaarSearchResultsFromDB_Struct>
Bazaar::GetSearchResults(
	SharedDatabase &db,
	BazaarSearchCriteria_Struct search,
	uint32 char_zone_id,
	int32 char_zone_instance_id
)
{
	LogTrading(
		"Searching for items with search criteria - item_name [{}] min_cost [{}] max_cost [{}] min_level [{}] "
		"max_level [{}] max_results [{}] prestige [{}] augment [{}] trader_entity_id [{}] trader_id [{}] "
		"search_scope [{}] char_zone_id [{}], char_zone_instance_id [{}]",
		search.item_name,
		search.min_cost,
		search.max_cost,
		search.min_level,
		search.max_level,
		search.max_results,
		search.prestige,
		search.augment,
		search.trader_entity_id,
		search.trader_id,
		search.search_scope,
		char_zone_id,
		char_zone_instance_id
	);

	bool        convert = false;
	std::string search_criteria_trader("TRUE ");

	if (search.search_scope == NonRoFBazaarSearchScope) {
		search_criteria_trader.append(
			fmt::format(
				" AND trader.char_entity_id = {} AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
				search.trader_entity_id,
				Zones::BAZAAR,
				char_zone_instance_id
			)
		);
	}
	else if (search.search_scope == Local_Scope) {
		search_criteria_trader.append(fmt::format(
			" AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
			char_zone_id,
			char_zone_instance_id)
		);
	}
	else if (search.trader_id > 0) {
		if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
			if (search.trader_id >= TraderRepository::TRADER_CONVERT_ID) {
				convert = true;
				search_criteria_trader.append(fmt::format(
					" AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
					Zones::BAZAAR,
					search.trader_id - TraderRepository::TRADER_CONVERT_ID)
				);
			}
			else {
				search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
			}
		}
		else {
			search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
		}
	}

	if (search.min_cost != 0) {
		search_criteria_trader.append(fmt::format(" AND trader.item_cost >= {}", search.min_cost * 1000));
	}
	if (search.max_cost != 0) {
		search_criteria_trader.append(fmt::format(" AND trader.item_cost <= {}", (uint64) search.max_cost * 1000));
	}

	// not yet implemented
	//	if (search.prestige != 0) {
	// 	   0xffffffff prestige only, 0xfffffffe non-prestige, 0 all
	//		search_criteria.append(fmt::format(" AND items.type = {} ", search.prestige));
	//	}

	std::string query = fmt::format(
		"SELECT COUNT(item_id), trader.char_id, trader.item_id, trader.item_sn, trader.item_charges, trader.item_cost, "
		"trader.slot_id, SUM(trader.item_charges), trader.char_zone_id, trader.char_entity_id, character_data.name, "
		"aug_slot_1, aug_slot_2, aug_slot_3, aug_slot_4, aug_slot_5, aug_slot_6, trader.char_zone_instance_id "
		"FROM trader, character_data "
		"WHERE {} AND trader.char_id = character_data.id "
		"GROUP BY trader.item_sn, trader.item_charges, trader.char_id",
		search_criteria_trader.c_str()
	);

	std::vector<BazaarSearchResultsFromDB_Struct> all_entries;

	auto results = db.QueryDatabase(query);

	if (!results.Success()) {
		return all_entries;
	}

	struct ItemSearchType {
		EQ::item::ItemType type;
		bool               condition;
	};

	struct AddititiveSearchCriteria {
		bool should_check;
		bool condition;
	};

	for (auto row: results) {
		BazaarSearchResultsFromDB_Struct r{};

		r.item_id = Strings::ToInt(row[2]);
		r.charges = Strings::ToInt(row[4]);

		auto item = db.GetItem(r.item_id);
		if (!item) {
			continue;
		}

		uint32 aug_slot_1 = Strings::ToUnsignedInt(row[11]);
		uint32 aug_slot_2 = Strings::ToUnsignedInt(row[12]);
		uint32 aug_slot_3 = Strings::ToUnsignedInt(row[13]);
		uint32 aug_slot_4 = Strings::ToUnsignedInt(row[14]);
		uint32 aug_slot_5 = Strings::ToUnsignedInt(row[15]);
		uint32 aug_slot_6 = Strings::ToUnsignedInt(row[16]);

		std::unique_ptr<EQ::ItemInstance> inst(
			db.CreateItem(
				item,
				r.charges,
				aug_slot_1,
				aug_slot_2,
				aug_slot_3,
				aug_slot_4,
				aug_slot_5,
				aug_slot_6
			)
		);

		if (!inst->GetItem()) {
			continue;
		}

		r.count                   = Strings::ToInt(row[0]);
		r.trader_id               = Strings::ToInt(row[1]);
		r.serial_number           = Strings::ToInt(row[3]);
		r.cost                    = Strings::ToInt(row[5]);
		r.slot_id                 = Strings::ToInt(row[6]);
		r.sum_charges             = Strings::ToInt(row[7]);
		r.stackable               = item->Stackable;
		r.icon_id                 = item->Icon;
		r.trader_zone_id          = Strings::ToInt(row[8]);
		r.trader_zone_instance_id = Strings::ToInt(row[17]);
		r.trader_entity_id        = Strings::ToInt(row[9]);
		r.serial_number_RoF       = fmt::format("{:016}\0", Strings::ToInt(row[3]));
		r.item_name               = fmt::format("{:.63}\0", item->Name);
		r.trader_name             = fmt::format("{:.63}\0", std::string(row[10]).c_str());

		LogTradingDetail(
			"Searching against item [{}] ({}) for trader [{}]",
			item->Name,
			item->ID,
			r.trader_name
		);

		// item stat searches
		std::map<uint32, uint32> item_stat_searches = {

			{STAT_AC,                  inst->GetItemArmorClass(true)},
			{STAT_AGI,                 static_cast<uint32>(inst->GetItemAgi(true))},
			{STAT_CHA,                 static_cast<uint32>(inst->GetItemCha(true))},
			{STAT_DEX,                 static_cast<uint32>(inst->GetItemDex(true))},
			{STAT_INT,                 static_cast<uint32>(inst->GetItemInt(true))},
			{STAT_STA,                 static_cast<uint32>(inst->GetItemSta(true))},
			{STAT_STR,                 static_cast<uint32>(inst->GetItemStr(true))},
			{STAT_WIS,                 static_cast<uint32>(inst->GetItemWis(true))},
			{STAT_COLD,                static_cast<uint32>(inst->GetItemCR(true))},
			{STAT_DISEASE,             static_cast<uint32>(inst->GetItemDR(true))},
			{STAT_FIRE,                static_cast<uint32>(inst->GetItemFR(true))},
			{STAT_MAGIC,               static_cast<uint32>(inst->GetItemMR(true))},
			{STAT_POISON,              static_cast<uint32>(inst->GetItemPR(true))},
			{STAT_HP,                  static_cast<uint32>(inst->GetItemHP(true))},
			{STAT_MANA,                static_cast<uint32>(inst->GetItemMana(true))},
			{STAT_ENDURANCE,           static_cast<uint32>(inst->GetItemEndur(true))},
			{STAT_ATTACK,              static_cast<uint32>(inst->GetItemAttack(true))},
			{STAT_HP_REGEN,            static_cast<uint32>(inst->GetItemRegen(true))},
			{STAT_MANA_REGEN,          static_cast<uint32>(inst->GetItemManaRegen(true))},
			{STAT_HASTE,               static_cast<uint32>(inst->GetItemHaste(true))},
			{STAT_DAMAGE_SHIELD,       static_cast<uint32>(inst->GetItemDamageShield(true))},
			{STAT_DS_MITIGATION,       static_cast<uint32>(inst->GetItemDSMitigation(true))},
			{STAT_HEAL_AMOUNT,         static_cast<uint32>(inst->GetItemHealAmt(true))},
			{STAT_SPELL_DAMAGE,        static_cast<uint32>(inst->GetItemSpellDamage(true))},
			{STAT_CLAIRVOYANCE,        static_cast<uint32>(inst->GetItemClairvoyance(true))},
			{STAT_HEROIC_AGILITY,      static_cast<uint32>(inst->GetItemHeroicAgi(true))},
			{STAT_HEROIC_CHARISMA,     static_cast<uint32>(inst->GetItemHeroicCha(true))},
			{STAT_HEROIC_DEXTERITY,    static_cast<uint32>(inst->GetItemHeroicDex(true))},
			{STAT_HEROIC_INTELLIGENCE, static_cast<uint32>(inst->GetItemHeroicInt(true))},
			{STAT_HEROIC_STAMINA,      static_cast<uint32>(inst->GetItemHeroicSta(true))},
			{STAT_HEROIC_STRENGTH,     static_cast<uint32>(inst->GetItemHeroicStr(true))},
			{STAT_HEROIC_WISDOM,       static_cast<uint32>(inst->GetItemHeroicWis(true))},
			{STAT_BASH,                static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillBash, true))},
			{STAT_BACKSTAB,            static_cast<uint32>(inst->GetItemBackstabDamage(true))},
			{STAT_DRAGON_PUNCH,        static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillDragonPunch, true))},
			{STAT_EAGLE_STRIKE,        static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillEagleStrike, true))},
			{STAT_FLYING_KICK,         static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillFlyingKick, true))},
			{STAT_KICK,                static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillKick, true))},
			{STAT_ROUND_KICK,          static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillRoundKick, true))},
			{STAT_TIGER_CLAW,          static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillTigerClaw, true))},
			{STAT_FRENZY,              static_cast<uint32>(inst->GetItemSkillsStat(EQ::skills::SkillFrenzy, true))},
		};

		r.item_stat = item_stat_searches.contains(search.item_stat) ? item_stat_searches[search.item_stat] : 0;
		if (item_stat_searches.contains(search.item_stat) && item_stat_searches[search.item_stat] <= 0) {
			continue;
		}

		static std::map<uint8, uint32> item_slot_searches = {
			{EQ::invslot::slotCharm,       1},
			{EQ::invslot::slotEar1,        2},
			{EQ::invslot::slotHead,        4},
			{EQ::invslot::slotFace,        8},
			{EQ::invslot::slotEar2,        16},
			{EQ::invslot::slotNeck,        32},
			{EQ::invslot::slotShoulders,   64},
			{EQ::invslot::slotArms,        128},
			{EQ::invslot::slotBack,        256},
			{EQ::invslot::slotWrist1,      512},
			{EQ::invslot::slotWrist2,      1024},
			{EQ::invslot::slotRange,       2048},
			{EQ::invslot::slotHands,       4096},
			{EQ::invslot::slotPrimary,     8192},
			{EQ::invslot::slotSecondary,   16384},
			{EQ::invslot::slotFinger1,     32768},
			{EQ::invslot::slotFinger2,     65536},
			{EQ::invslot::slotChest,       131072},
			{EQ::invslot::slotLegs,        262144},
			{EQ::invslot::slotFeet,        524288},
			{EQ::invslot::slotWaist,       1048576},
			{EQ::invslot::slotPowerSource, 2097152},
			{EQ::invslot::slotAmmo,        4194304},
		};

		auto GetEquipmentSlotBit = [&](uint32 slot) -> uint32 {
			return item_slot_searches.contains(slot) ? item_slot_searches[slot] : 0;
		};

		auto FindItemAugSlot = [&]() -> bool {
			for (auto const &s: inst->GetItem()->AugSlotType) {
				return s == search.augment;
			}
			return false;
		};

		// item type searches
		std::vector<ItemSearchType> item_search_types = {
			{EQ::item::ItemType::ItemTypeAll,                  true},
			{EQ::item::ItemType::ItemTypeBook,                 item->ItemClass == EQ::item::ItemType::ItemTypeBook},
			{EQ::item::ItemType::ItemTypeContainer,            item->ItemClass == EQ::item::ItemType::ItemTypeContainer ||
			                                                   item->IsClassBag()},
			{EQ::item::ItemType::ItemTypeAllEffects,           item->Scroll.Effect > 0 && item->Scroll.Effect < 65000},
			{EQ::item::ItemType::ItemTypeUnknown9,             item->Worn.Effect == 998},
			{EQ::item::ItemType::ItemTypeUnknown10,            item->Worn.Effect >= 1298 && item->Worn.Effect <= 1307},
			{EQ::item::ItemType::ItemTypeFocusEffect,          item->Focus.Effect > 0},
			{EQ::item::ItemType::ItemTypeArmor,                item->ItemType == EQ::item::ItemType::ItemTypeArmor},
			{EQ::item::ItemType::ItemType1HBlunt,              item->ItemType == EQ::item::ItemType::ItemType1HBlunt},
			{EQ::item::ItemType::ItemType1HPiercing,           item->ItemType == EQ::item::ItemType::ItemType1HPiercing},
			{EQ::item::ItemType::ItemType1HSlash,              item->ItemType == EQ::item::ItemType::ItemType1HSlash},
			{EQ::item::ItemType::ItemType2HBlunt,              item->ItemType == EQ::item::ItemType::ItemType2HBlunt},
			{EQ::item::ItemType::ItemType2HSlash,              item->ItemType == EQ::item::ItemType::ItemType2HSlash},
			{EQ::item::ItemType::ItemTypeBow,                  item->ItemType == EQ::item::ItemType::ItemTypeBow},
			{EQ::item::ItemType::ItemTypeShield,               item->ItemType == EQ::item::ItemType::ItemTypeShield},
			{EQ::item::ItemType::ItemTypeMisc,                 item->ItemType == EQ::item::ItemType::ItemTypeMisc},
			{EQ::item::ItemType::ItemTypeFood,                 item->ItemType == EQ::item::ItemType::ItemTypeFood},
			{EQ::item::ItemType::ItemTypeDrink,                item->ItemType == EQ::item::ItemType::ItemTypeDrink},
			{EQ::item::ItemType::ItemTypeLight,                item->ItemType == EQ::item::ItemType::ItemTypeLight},
			{EQ::item::ItemType::ItemTypeCombinable,           item->ItemType == EQ::item::ItemType::ItemTypeCombinable},
			{EQ::item::ItemType::ItemTypeBandage,              item->ItemType == EQ::item::ItemType::ItemTypeBandage},
			{EQ::item::ItemType::ItemTypeSmallThrowing,        item->ItemType == EQ::item::ItemType::ItemTypeSmallThrowing ||
			                                                   item->ItemType == EQ::item::ItemType::ItemTypeLargeThrowing},
			{EQ::item::ItemType::ItemTypeSpell,                item->ItemType == EQ::item::ItemType::ItemTypeSpell},
			{EQ::item::ItemType::ItemTypePotion,               item->ItemType == EQ::item::ItemType::ItemTypePotion},
			{EQ::item::ItemType::ItemTypeBrassInstrument,      item->ItemType == EQ::item::ItemType::ItemTypeBrassInstrument},
			{EQ::item::ItemType::ItemTypeWindInstrument,       item->ItemType == EQ::item::ItemType::ItemTypeWindInstrument},
			{EQ::item::ItemType::ItemTypeStringedInstrument,   item->ItemType == EQ::item::ItemType::ItemTypeStringedInstrument},
			{EQ::item::ItemType::ItemTypePercussionInstrument, item->ItemType == EQ::item::ItemType::ItemTypePercussionInstrument},
			{EQ::item::ItemType::ItemTypeArrow,                item->ItemType == EQ::item::ItemType::ItemTypeArrow},
			{EQ::item::ItemType::ItemTypeJewelry,              item->ItemType == EQ::item::ItemType::ItemTypeJewelry},
			{EQ::item::ItemType::ItemTypeNote,                 item->ItemType == EQ::item::ItemType::ItemTypeNote},
			{EQ::item::ItemType::ItemTypeKey,                  item->ItemType == EQ::item::ItemType::ItemTypeKey},
			{EQ::item::ItemType::ItemType2HPiercing,           item->ItemType == EQ::item::ItemType::ItemType2HPiercing},
			{EQ::item::ItemType::ItemTypeAlcohol,              item->ItemType == EQ::item::ItemType::ItemTypeAlcohol},
			{EQ::item::ItemType::ItemTypeMartial,              item->ItemType == EQ::item::ItemType::ItemTypeMartial},
			{EQ::item::ItemType::ItemTypeAugmentation,         item->ItemType == EQ::item::ItemType::ItemTypeAugmentation},
			{EQ::item::ItemType::ItemTypeAlternateAbility,     item->ItemType == EQ::item::ItemType::ItemTypeAlternateAbility},
			{EQ::item::ItemType::ItemTypeCount,                item->ItemType == EQ::item::ItemType::ItemTypeCount},
			{EQ::item::ItemType::ItemTypeCollectible,          item->ItemType == EQ::item::ItemType::ItemTypeCollectible}
			};

		bool met_filter = false;
		bool has_filter = false;

		for (auto &i: item_search_types) {
			if (i.type == search.type) {
				has_filter = true;
				if (i.condition) {
					LogTradingDetail("Item [{}] met search criteria for type [{}]", item->Name, uint8(i.type));
					met_filter = true;
					break;
				}
			}
		}
		if (has_filter && !met_filter) {
			continue;
		}

		// TODO: Add catch-all item type filter for specific item types

		// item additive searches
		std::vector<AddititiveSearchCriteria> item_additive_searches = {
			{
				.should_check = search.min_level != 1 && inst->GetItemRequiredLevel(true) > 0,
				.condition = inst->GetItemRequiredLevel(true) >= search.min_level
			},
			{
				.should_check = search.max_level != 1 && inst->GetItemRequiredLevel(true) > 0,
				.condition = inst->GetItemRequiredLevel(true) <= search.max_level
			},
			{
				.should_check = !std::string(search.item_name).empty(),
				.condition = Strings::ContainsLower(item->Name, search.item_name)
			},
			{
				.should_check = search._class != 0xFFFFFFFF,
				.condition = static_cast<bool>(item->Classes & GetPlayerClassBit(search._class))
			},
			{
				.should_check = search.race != 0xFFFFFFFF,
				.condition = static_cast<bool>(item->Races & GetPlayerRaceBit(GetRaceIDFromPlayerRaceValue(search.race)))
			},
			{
				.should_check = search.augment != 0,
				.condition = FindItemAugSlot()
			},
			{
				.should_check = search.slot != 0xFFFFFFFF,
				.condition = static_cast<bool>(item->Slots & GetEquipmentSlotBit(search.slot))
			},
		};

		bool should_add = true;

		for (auto &i: item_additive_searches) {
			LogTradingDetail(
				"Checking item [{}] for search criteria - should_check [{}] condition [{}]",
				item->Name,
				i.should_check,
				i.condition
			);
			if (i.should_check && !i.condition) {
				should_add = false;
				continue;
			}
		}

		if (!should_add) {
			continue;
		}

		LogTradingDetail("Found item [{}] meeting search criteria.", r.item_name);
		if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
			if (convert || (r.trader_zone_id == Zones::BAZAAR && r.trader_zone_instance_id != char_zone_instance_id)) {
				r.trader_id = TraderRepository::TRADER_CONVERT_ID + r.trader_zone_instance_id;
			}
		}

		all_entries.push_back(r);
	}

	if (all_entries.size() > search.max_results) {
		all_entries.resize(search.max_results);
	}

	LogTrading("Returning [{}] items from search results", all_entries.size());

	return all_entries;
}

std::vector<BazaarSearchResultsFromDB_Struct>
Bazaar::GetSearchResultsNew(
	Database &db,
	Database &content_db,
	BazaarSearchCriteria_Struct search,
	uint32 char_zone_id,
	int32 char_zone_instance_id
)
{
	LogTrading(
		"Searching for items with search criteria - item_name [{}] min_cost [{}] max_cost [{}] min_level [{}] "
		"max_level [{}] max_results [{}] prestige [{}] augment [{}] trader_entity_id [{}] trader_id [{}] "
		"search_scope [{}] char_zone_id [{}], char_zone_instance_id [{}]",
		search.item_name,
		search.min_cost,
		search.max_cost,
		search.min_level,
		search.max_level,
		search.max_results,
		search.prestige,
		search.augment,
		search.trader_entity_id,
		search.trader_id,
		search.search_scope,
		char_zone_id,
		char_zone_instance_id
	);

	static std::map<uint8, uint32> item_slot_searches_new = {
		{EQ::invslot::slotCharm,       1},
		{EQ::invslot::slotEar1,        2},
		{EQ::invslot::slotHead,        4},
		{EQ::invslot::slotFace,        8},
		{EQ::invslot::slotEar2,        16},
		{EQ::invslot::slotNeck,        32},
		{EQ::invslot::slotShoulders,   64},
		{EQ::invslot::slotArms,        128},
		{EQ::invslot::slotBack,        256},
		{EQ::invslot::slotWrist1,      512},
		{EQ::invslot::slotWrist2,      1024},
		{EQ::invslot::slotRange,       2048},
		{EQ::invslot::slotHands,       4096},
		{EQ::invslot::slotPrimary,     8192},
		{EQ::invslot::slotSecondary,   16384},
		{EQ::invslot::slotFinger1,     32768},
		{EQ::invslot::slotFinger2,     65536},
		{EQ::invslot::slotChest,       131072},
		{EQ::invslot::slotLegs,        262144},
		{EQ::invslot::slotFeet,        524288},
		{EQ::invslot::slotWaist,       1048576},
		{EQ::invslot::slotPowerSource, 2097152},
		{EQ::invslot::slotAmmo,        4194304},
	};

	struct ItemSearchType {
		EQ::item::ItemType type;
		std::string        condition;
	};

	std::vector<ItemSearchType> item_search_types_new = {
		{EQ::item::ItemType::ItemTypeBook,                 " AND (items.itemclass = 2 or items.itemclass = 31)"},
		{EQ::item::ItemType::ItemTypeContainer,            " AND (items.itemclass = 1 or items.itemclass = 67)"},
		{EQ::item::ItemType::ItemTypeAllEffects,           " AND (items.scrolleffect > 0 && items.scrolleffect < 65000)"},
		{EQ::item::ItemType::ItemTypeUnknown9,             " AND items.worneffect = 998"},
		{EQ::item::ItemType::ItemTypeUnknown10,            " AND (items.worneffect >= 1298 && items.worneffect <= 1307)"},
		{EQ::item::ItemType::ItemTypeFocusEffect,          " AND items.focuseffect > 0"},
		{EQ::item::ItemType::ItemTypeArmor,                " AND items.itemtype = 10"},
		{EQ::item::ItemType::ItemType1HBlunt,              " AND items.itemtype = 3"},
		{EQ::item::ItemType::ItemType1HPiercing,           " AND items.itemtype = 2"},
		{EQ::item::ItemType::ItemType1HSlash,              " AND items.itemtype = 0"},
		{EQ::item::ItemType::ItemType2HBlunt,              " AND items.itemtype = 4"},
		{EQ::item::ItemType::ItemType2HSlash,              " AND items.itemtype = 1"},
		{EQ::item::ItemType::ItemTypeBow,                  " AND items.itemtype = 5"},
		{EQ::item::ItemType::ItemTypeShield,               " AND items.itemtype = 8"},
		{EQ::item::ItemType::ItemTypeMisc,                 " AND items.itemtype = 11"},
		{EQ::item::ItemType::ItemTypeFood,                 " AND items.itemtype = 14"},
		{EQ::item::ItemType::ItemTypeDrink,                " AND items.itemtype = 15"},
		{EQ::item::ItemType::ItemTypeLight,                " AND items.itemtype = 16"},
		{EQ::item::ItemType::ItemTypeCombinable,           " AND items.itemtype = 17"},
		{EQ::item::ItemType::ItemTypeBandage,              " AND items.itemtype = 18"},
		{EQ::item::ItemType::ItemTypeSmallThrowing,        " AND (items.itemtype = 19 OR items.itemtype = 7)"},
		{EQ::item::ItemType::ItemTypeSpell,                " AND items.itemtype = 20"},
		{EQ::item::ItemType::ItemTypePotion,               " AND items.itemtype = 21"},
		{EQ::item::ItemType::ItemTypeBrassInstrument,      " AND items.itemtype = 25"},
		{EQ::item::ItemType::ItemTypeWindInstrument,       " AND items.itemtype = 23"},
		{EQ::item::ItemType::ItemTypeStringedInstrument,   " AND items.itemtype = 24"},
		{EQ::item::ItemType::ItemTypePercussionInstrument, " AND items.itemtype = 26"},
		{EQ::item::ItemType::ItemTypeArrow,                " AND items.itemtype = 27"},
		{EQ::item::ItemType::ItemTypeJewelry,              " AND items.itemtype = 29"},
		{EQ::item::ItemType::ItemTypeNote,                 " AND items.itemtype = 32"},
		{EQ::item::ItemType::ItemTypeKey,                  " AND items.itemtype = 33"},
		{EQ::item::ItemType::ItemType2HPiercing,           " AND items.itemtype = 35"},
		{EQ::item::ItemType::ItemTypeAlcohol,              " AND items.itemtype = 38"},
		{EQ::item::ItemType::ItemTypeMartial,              " AND items.itemtype = 45"},
		{EQ::item::ItemType::ItemTypeAugmentation,         " AND items.itemtype = 54"},
		{EQ::item::ItemType::ItemTypeAlternateAbility,     " AND items.itemtype = 57"},
		{EQ::item::ItemType::ItemTypeCount,                " AND items.itemtype = 65"},
		{EQ::item::ItemType::ItemTypeCollectible,          " AND items.itemtype = 66"}
	};

	// item stat searches
	struct ItemStatSearch {
		std::string           query_string;
		EQ::skills::SkillType skill_type;
	};

	std::map<uint32, ItemStatSearch> item_stat_searches_new = {
		{STAT_AC,                  {" items.ac" ,          static_cast<EQ::skills::SkillType>(0)} },
		{STAT_AGI,                 {" items.aagi",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_CHA,                 {" items.acha",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DEX,                 {" items.adex",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_INT,                 {" items.aint",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_STA,                 {" items.asta",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_STR,                 {" items.astr",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_WIS,                 {" items.awis",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_COLD,                {" items.cr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DISEASE,             {" items.dr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_FIRE,                {" items.fr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_MAGIC,               {" items.mr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_POISON,              {" items.pr",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HP,                  {" items.hp",           static_cast<EQ::skills::SkillType>(0)} },
		{STAT_MANA,                {" items.mana",         static_cast<EQ::skills::SkillType>(0)} },
		{STAT_ENDURANCE,           {" items.endur",        static_cast<EQ::skills::SkillType>(0)} },
		{STAT_ATTACK,              {" items.attack",       static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HP_REGEN,            {" items.regen",        static_cast<EQ::skills::SkillType>(0)} },
		{STAT_MANA_REGEN,          {" items.manaregen",    static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HASTE,               {" items.haste",        static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DAMAGE_SHIELD,       {" items.damageshield", static_cast<EQ::skills::SkillType>(0)} },
		{STAT_DS_MITIGATION,       {" items.dsmitigation", static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEAL_AMOUNT,         {" items.healamt",      static_cast<EQ::skills::SkillType>(0)} },
		{STAT_SPELL_DAMAGE,        {" items.spelldmg",     static_cast<EQ::skills::SkillType>(0)} },
		{STAT_CLAIRVOYANCE,        {" items.clairvoyance", static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_AGILITY,      {" items.heroic_agi",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_CHARISMA,     {" items.heroic_cha",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_DEXTERITY,    {" items.heroic_dex",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_INTELLIGENCE, {" items.heroic_int",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_STAMINA,      {" items.heroic_sta",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_STRENGTH,     {" items.heroic_str",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_HEROIC_WISDOM,       {" items.heroic_wis",   static_cast<EQ::skills::SkillType>(0)} },
		{STAT_BASH,                {" items.skillmodvalue", EQ::skills::SkillBash}                },
		{STAT_BACKSTAB,            {" items.backstabdmg",   EQ::skills::SkillBackstab}            },
		{STAT_DRAGON_PUNCH,        {" items.skillmodvalue", EQ::skills::SkillDragonPunch}         },
		{STAT_EAGLE_STRIKE,        {" items.skillmodvalue", EQ::skills::SkillEagleStrike}         },
		{STAT_FLYING_KICK,         {" items.skillmodvalue", EQ::skills::SkillFlyingKick}          },
		{STAT_KICK,                {" items.skillmodvalue", EQ::skills::SkillKick}                },
		{STAT_ROUND_KICK,          {" items.skillmodvalue", EQ::skills::SkillRoundKick}           },
		{STAT_TIGER_CLAW,          {" items.skillmodvalue", EQ::skills::SkillTigerClaw}           },
		{STAT_FRENZY,              {" items.skillmodvalue", EQ::skills::SkillFrenzy}              },
	};

	bool        convert = false;
	std::string search_criteria_trader("TRUE");
	std::string field_criteria_items("FALSE");
	std::string where_criteria_items(" TRUE ");

	if (search.search_scope == NonRoFBazaarSearchScope) {
		search_criteria_trader.append(
			fmt::format(
				" AND trader.char_entity_id = {} AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
				search.trader_entity_id,
				Zones::BAZAAR,
				char_zone_instance_id
			)
		);
	}
	else if (search.search_scope == Local_Scope) {
		search_criteria_trader.append(fmt::format(
			" AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
			char_zone_id,
			char_zone_instance_id)
		);
	}
	else if (search.trader_id > 0) {
		if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
			if (search.trader_id >= TraderRepository::TRADER_CONVERT_ID) {
				convert = true;
				search_criteria_trader.append(fmt::format(
					" AND trader.char_zone_id = {} AND trader.char_zone_instance_id = {}",
					Zones::BAZAAR,
					search.trader_id - TraderRepository::TRADER_CONVERT_ID)
				);
			}
			else {
				search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
			}
		}
		else {
			search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
		}
	}

	if (search.min_cost != 0) {
		search_criteria_trader.append(fmt::format(" AND trader.item_cost >= {}", search.min_cost * 1000));
	}
	if (search.max_cost != 0) {
		search_criteria_trader.append(fmt::format(" AND trader.item_cost <= {}", (uint64) search.max_cost * 1000));
	}

	if (search.slot != std::numeric_limits<uint32>::max()) {
		if (item_slot_searches_new.contains(search.slot)) {
			where_criteria_items.append(
				fmt::format(" AND items.slots & {0} = {0}", item_slot_searches_new[search.slot]));
		}
	}

	if (search.type != std::numeric_limits<uint32>::max()) {
		for (auto const &[type, condition]: item_search_types_new) {
			if (type == search.type) {
				where_criteria_items.append(condition);
				break;
			}
		}
	}

	if (search.race != std::numeric_limits<uint32>::max()) {
		where_criteria_items.append(
			fmt::format(" AND items.races & {0} = {0}", GetPlayerRaceBit(GetRaceIDFromPlayerRaceValue(search.race))));
	}

	if (search._class != std::numeric_limits<uint32>::max()) {
		where_criteria_items.append(fmt::format(" AND items.classes & {0} = {0}", GetPlayerClassBit(search._class)));
	}

	if (search.item_stat != std::numeric_limits<uint32>::max()) {
		if (item_stat_searches_new.contains(search.item_stat)) {
			field_criteria_items = fmt::format("{}", item_stat_searches_new[search.item_stat].query_string);
			if (item_stat_searches_new[search.item_stat].skill_type) {
				where_criteria_items.append(
					fmt::format(" AND items.skillmodtype = {} ", item_stat_searches_new[search.item_stat].skill_type));
			}
			else {
				where_criteria_items.append(
					fmt::format(" AND {} > 0 ", item_stat_searches_new[search.item_stat].query_string));
			}
		}
	}

	if (search.augment) {
		where_criteria_items.append(fmt::format(
			" AND (items.augslot1type = {0} OR "
			"items.augslot2type = {0} OR "
			"items.augslot3type = {0} OR "
			"items.augslot4type = {0} OR "
			"items.augslot5type = {0} OR "
			"items.augslot6type = {0})",
			search.augment)
		);
	}

	if (search.min_level != 1) {
		where_criteria_items.append(fmt::format(" AND items.reclevel >= {}", search.min_level));
	}

	if (search.max_level != 100) {
		where_criteria_items.append(fmt::format(" AND items.reclevel <= {}", search.max_level));
	}

	std::vector<BazaarSearchResultsFromDB_Struct> all_entries;
	std::vector<std::string>                      trader_items_ids{};

	auto const trader_results = TraderRepository::GetBazaarTraderDetails(db, search_criteria_trader);
	if (trader_results.empty()) {
		LogTradingDetail("Bazaar - No traders found in bazaar search.");
		return all_entries;
	}

	for (auto const &i: trader_results) {
		trader_items_ids.push_back(std::to_string(i.trader.item_id));
	}

	auto const item_results = ItemsRepository::GetItemsForBazaarSearch(
		content_db,
		trader_items_ids,
		std::string(search.item_name),
		field_criteria_items,
		where_criteria_items
	);

	if (item_results.empty()) {
		LogError("Bazaar - No items found in bazaar search.");
		return all_entries;
	}

	all_entries.reserve(trader_results.size());

	for (auto const& t:trader_results) {
		if (!item_results.contains(t.trader.item_id)) {
			continue;
		}

		BazaarSearchResultsFromDB_Struct r{};
		r.count                   = 1;
		r.trader_id               = t.trader.char_id;
		r.serial_number           = t.trader.item_sn;
		r.cost                    = t.trader.item_cost;
		r.slot_id                 = t.trader.slot_id;
		r.sum_charges             = t.trader.item_charges;
		r.stackable               = item_results.at(t.trader.item_id).stackable;
		r.icon_id                 = item_results.at(t.trader.item_id).icon;
		r.trader_zone_id          = t.trader.char_zone_id;
		r.trader_zone_instance_id = t.trader.char_zone_instance_id;
		r.trader_entity_id        = t.trader.char_entity_id;
		r.serial_number_RoF       = fmt::format("{:016}\0", t.trader.item_sn);
		r.item_name               = fmt::format("{:.63}\0", item_results.at(t.trader.item_id).name);
		r.trader_name             = fmt::format("{:.63}\0", t.trader_name);
		r.item_stat               = item_results.at(t.trader.item_id).stats;

		if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
			if (convert || (r.trader_zone_id == Zones::BAZAAR && r.trader_zone_instance_id != char_zone_instance_id)) {
				r.trader_id = TraderRepository::TRADER_CONVERT_ID + r.trader_zone_instance_id;
			}
		}

		all_entries.push_back(r);
	}

	if (all_entries.size() > search.max_results) {
		all_entries.resize(search.max_results);
	}

	LogTrading("Returning [{}] items from search results", all_entries.size());

	return all_entries;
}
