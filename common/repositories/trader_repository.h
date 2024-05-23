#ifndef EQEMU_TRADER_REPOSITORY_H
#define EQEMU_TRADER_REPOSITORY_H

#include "../../common/shareddb.h"
#include "../strings.h"
#include "base/base_trader_repository.h"
#include "items_repository.h"

//#include "../../common/eq_constants.h"
#include "../../common/item_data.h"
#include "../../common/races.h"
//#include "../eq_packet_structs.h"
#include "../cereal/include/cereal/archives/binary.hpp"
#include "../cereal/include/cereal/types/string.hpp"

class TraderRepository : public BaseTraderRepository {
public:

	struct DistinctTraders_Struct {
		uint32      trader_id;
		uint32      zone_id;
		uint32      entity_id;
		std::string trader_name;
	};

	struct BulkTraders_Struct {
		uint32                              count{0};
		uint32                              name_length{0};
		std::vector<DistinctTraders_Struct> traders{};
	};

	struct WelcomeData_Struct {
		uint32 count_of_traders;
		uint32 count_of_items;
	};

	static std::vector<BazaarSearchResultsFromDB_Struct>
	GetBazaarSearchResults(
		SharedDatabase &db,
		BazaarSearchCriteria_Struct search,
		uint32 char_zone_id
	)
	{

		LogTrading(
			"Searching for items with search criteria - item_name [{}] min_cost [{}] max_cost [{}] min_level [{}] "
			"max_level [{}] max_results [{}] prestige [{}] augment [{}] trader_entity_id [{}] trader_id [{}] "
			"search_scope [{}] char_zone_id [{}]",
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
			char_zone_id
		);

		std::string search_criteria_trader("TRUE ");

		if (search.search_scope == UFBazaarSearchScope) {
			search_criteria_trader.append(
				fmt::format(
					" AND trader.char_entity_id = {} AND trader.char_zone_id = {}",
					search.trader_entity_id,
					Zones::BAZAAR
				)
			);
		}
		else if (search.search_scope == Local_Scope) {
			search_criteria_trader.append(fmt::format(" AND trader.char_zone_id = {}", char_zone_id));
		}
		else if (search.trader_id > 0) {
			search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
		}
		if (search.min_cost != 0) {
			search_criteria_trader.append(fmt::format(" AND trader.item_cost >= {}", search.min_cost));
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
			"trader.slot_id, SUM(trader.item_charges), trader.char_zone_id, trader.char_entity_id, character_data.name "
			"FROM trader, character_data "
			"WHERE {} AND trader.char_id = character_data.id "
			"GROUP BY trader.item_sn, trader.item_charges, trader.char_id LIMIT {}",
			search_criteria_trader.c_str(),
			search.max_results
		);

		std::vector<BazaarSearchResultsFromDB_Struct> all_entries;

		auto results = db.QueryDatabase(query);

		if (!results.Success()) {
			return all_entries;
		}

		struct ItemStatSearch {
			uint32 stat;
			uint32 value;
		};

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

			auto item = db.GetItem(r.item_id);
			if (!item) {
				continue;
			}

			r.count             = Strings::ToInt(row[0]);
			r.trader_id         = Strings::ToInt(row[1]);
			r.serial_number     = Strings::ToInt(row[3]);
			r.charges           = Strings::ToInt(row[4]);
			r.cost              = Strings::ToInt(row[5]);
			r.slot_id           = Strings::ToInt(row[6]);
			r.sum_charges       = Strings::ToInt(row[7]);
			r.stackable         = item->Stackable;
			r.icon_id           = item->Icon;
			r.trader_zone_id    = Strings::ToInt(row[8]);
			r.trader_entity_id  = Strings::ToInt(row[9]);
			r.serial_number_RoF = fmt::format("{:016}\0", Strings::ToInt(row[3]));
			r.item_name         = fmt::format("{:.63}\0", item->Name);
			r.trader_name       = fmt::format("{:.63}\0", std::string(row[10]).c_str());

			LogTradingDetail(
				"Searching against item [{}] ({}) for trader [{}]",
				item->Name,
				item->ID,
				r.trader_name
			);

			// item stat searches
			std::vector<ItemStatSearch> item_stat_searches = {
				{STAT_AC,            static_cast<uint32>(item->AC)},
				{STAT_AGI,           static_cast<uint32>(item->AAgi)},
				{STAT_CHA,           static_cast<uint32>(item->ACha)},
				{STAT_DEX,           static_cast<uint32>(item->ADex)},
				{STAT_INT,           static_cast<uint32>(item->AInt)},
				{STAT_STA,           static_cast<uint32>(item->ASta)},
				{STAT_STR,           static_cast<uint32>(item->AStr)},
				{STAT_WIS,           static_cast<uint32>(item->AWis)},
				{STAT_COLD,          static_cast<uint32>(item->CR)},
				{STAT_DISEASE,       static_cast<uint32>(item->DR)},
				{STAT_FIRE,          static_cast<uint32>(item->FR)},
				{STAT_MAGIC,         static_cast<uint32>(item->MR)},
				{STAT_POISON,        static_cast<uint32>(item->PR)},
				{STAT_HP,            static_cast<uint32>(item->HP)},
				{STAT_MANA,          static_cast<uint32>(item->Mana)},
				{STAT_ENDURANCE,     static_cast<uint32>(item->Endur)},
				{STAT_ATTACK,        static_cast<uint32>(item->Attack)},
				{STAT_HP_REGEN,      static_cast<uint32>(item->Regen)},
				{STAT_MANA_REGEN,    static_cast<uint32>(item->ManaRegen)},
				{STAT_HASTE,         static_cast<uint32>(item->Haste)},
				{STAT_DAMAGE_SHIELD, static_cast<uint32>(item->DamageShield)}
			};

			r.item_stat = 0;
			for (auto &i: item_stat_searches) {
				if (i.stat == search.item_stat) {
					r.item_stat = i.value;
				}
			}

			// item type searches
			std::vector<ItemSearchType> item_search_types = {
				{EQ::item::ItemType::ItemTypeAll,         true},
				{EQ::item::ItemType::ItemTypeBook,        item->ItemClass == EQ::item::ItemType::ItemTypeBook},
				{EQ::item::ItemType::ItemTypeContainer,   item->ItemClass == EQ::item::ItemType::ItemTypeContainer},
				{EQ::item::ItemType::ItemTypeAllEffects,  item->Scroll.Effect > 0 && item->Scroll.Effect < 65000},
				{EQ::item::ItemType::ItemTypeUnknown9,    item->Worn.Effect == 998},
				{EQ::item::ItemType::ItemTypeUnknown10,   item->Worn.Effect >= 1298 && item->Worn.Effect <= 1307},
				{EQ::item::ItemType::ItemTypeFocusEffect, item->Focus.Effect > 0},
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
					.should_check = search.min_level != 1 && item->RecLevel > 0,
					.condition = item->RecLevel >= search.min_level
				},
				{
					.should_check = search.max_level != 1 && item->RecLevel > 0,
					.condition = item->RecLevel <= search.max_level
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
					.condition = static_cast<bool>(item->Races & GetPlayerRaceBit(search.race))
				},
//				{
//					.should_check = search.augment != 0,
//					.condition = !(item->AugType & GetAugTypeBit(search.augment))
//				},
//				{
//					.should_check = search.slot != 0xFFFFFFFF,
//					.condition = item->Slots & GetEquipmentSlotBit(search.slot)
//				},
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

			all_entries.push_back(r);
		}

		LogTrading("Returning [{}] items from search results", all_entries.size());

		return all_entries;
	}

	static BulkTraders_Struct GetDistinctTraders(Database &db)
	{
		BulkTraders_Struct                  all_entries{};
		std::vector<DistinctTraders_Struct> distinct_traders;

		auto results = db.QueryDatabase(
			"SELECT DISTINCT(t.char_id), t.char_zone_id, t.char_entity_id, c.name "
			"FROM trader AS t "
			"JOIN character_data AS c ON t.char_id = c.id;"
		);

		distinct_traders.reserve(results.RowCount());

		for (auto row: results) {
			DistinctTraders_Struct e{};

			e.trader_id   = Strings::ToInt(row[0]);
			e.zone_id     = Strings::ToInt(row[1]);
			e.entity_id   = Strings::ToInt(row[2]);
			e.trader_name = row[3] ? row[3] : "";
			all_entries.name_length += e.trader_name.length() + 1;

			all_entries.traders.push_back(e);
		}
		all_entries.count = results.RowCount();
		return all_entries;
	}

	static WelcomeData_Struct GetWelcomeData(Database &db)
	{
		WelcomeData_Struct e{};

		auto results = db.QueryDatabase("SELECT COUNT(DISTINCT char_id), count(char_id) FROM trader;");

		if (!results.RowCount()) {
			return e;
		}

		auto r = results.begin();
		e.count_of_traders = Strings::ToInt(r[0]);
		e.count_of_items   = Strings::ToInt(r[1]);
		return e;
	}

	static int UpdateItem(Database &db, uint32 char_id, uint32 new_price, uint32 item_id, uint32 item_charges)
	{
		std::vector<BaseTraderRepository::Trader> items{};
		if (item_charges == 0) {
			items = GetWhere(
				db,
				fmt::format(
					"char_id = '{}' AND item_id = '{}'",
					char_id,
					item_id
				)
			);
		}
		else {
			items = GetWhere(
				db,
				fmt::format(
					"char_id = '{}' AND item_id = '{}' AND item_charges = '{}'",
					char_id,
					item_id,
					item_charges
				)
			);
		}

		if (items.empty()) {
			return 0;
		}

		for (auto &i: items) {
			i.item_cost = new_price;
		}

		return ReplaceMany(db, items);
	}

	static Trader GetTraderItem(Database &db, uint32 trader_id, uint32 item_id, uint32 item_cost)
	{
		Trader item{};

		auto query   = fmt::format(
			"SELECT t.char_id, t.item_id, t.serialnumber, t.charges, t.item_cost, t.slot_id, t.entity_id FROM trader AS t "
			"WHERE t.entity_id = {} AND t.item_id = {} AND t.item_cost = {} "
			"LIMIT 1;",
			trader_id,
			item_id,
			item_cost
		);
		auto results = db.QueryDatabase(query);

		if (results.RowCount() == 0) {
			return item;
		}

		auto row = results.begin();
		item.char_id      = Strings::ToInt(row[0]);
		item.item_id      = Strings::ToInt(row[1]);
		item.item_sn      = Strings::ToInt(row[2]);
		item.item_charges = Strings::ToInt(row[3]);
		item.item_cost    = Strings::ToInt(row[4]);
		item.slot_id      = Strings::ToInt(row[5]);

		return item;
	}

	static int UpdateQuantity(Database &db, uint32 char_id, uint32 serial_number, int16 quantity)
	{
		const auto trader_item = GetWhere(
			db,
			fmt::format("char_id = '{}' AND item_sn = '{}' ", char_id, serial_number)
		);

		if (trader_item.empty() || trader_item.size() > 1) {
			return 0;
		}

		auto m = trader_item[0];
		m.item_charges = quantity;

		return UpdateOne(db, m);
	}

	static Trader GetItemBySerialNumber(Database &db, uint32 serial_number)
	{
		Trader     e{};
		const auto trader_item = GetWhere(
			db,
			fmt::format("`item_sn` = '{}' LIMIT 1", serial_number)
		);

		if (trader_item.empty()) {
			return e;
		}
		else {
			return trader_item.at(0);
		}
	}

	static Trader GetItemBySerialNumber(Database &db, std::string serial_number)
	{
		Trader     e{};
		auto       sn          = Strings::ToUnsignedBigInt(serial_number);
		const auto trader_item = GetWhere(
			db,
			fmt::format("`item_sn` = '{}' LIMIT 1", sn)
		);

		if (trader_item.empty()) {
			return e;
		}
		else {
			return trader_item.at(0);
		}
	}

	static int UpdateActiveTransaction(Database &db, uint32 id, bool status)
	{
		auto e = FindOne(db, id);
		if (!e.id) {
			return 0;
		}

		e.active_transaction = status == true ? 1 : 0;

		return UpdateOne(db, e);
	}

	static int DeleteMany(Database &db, const std::vector<Trader> &entries)
	{
		std::vector<std::string> delete_ids;

		for (auto const &e: entries) {
			delete_ids.push_back(std::to_string(e.id));
		}

		return DeleteWhere(db, fmt::format("`id` IN({})", Strings::Implode(",", delete_ids)));
	}
};

#endif //EQEMU_TRADER_REPOSITORY_H
