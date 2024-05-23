#ifndef EQEMU_TRADER_REPOSITORY_H
#define EQEMU_TRADER_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_trader_repository.h"
#include "items_repository.h"

//#include "../../common/eq_constants.h"
#include "../../common/item_data.h"
//#include "../eq_packet_structs.h"
#include "../cereal/include/cereal/archives/binary.hpp"
#include "../cereal/include/cereal/types/string.hpp"

class TraderRepository: public BaseTraderRepository {
public:

	struct DistinctTraders_Struct {
		uint32      trader_id;
		uint32      zone_id;
		uint32      entity_id;
		std::string trader_name;
	};

	struct BulkTraders_Struct {
		uint32                              count {0};
		uint32                              name_length {0};
		std::vector<DistinctTraders_Struct> traders {};
	};

	struct WelcomeData_Struct {
		uint32 count_of_traders;
		uint32 count_of_items;
	};

	/**
	 * This file was auto generated and can be modified and extended upon
	 *
	 * Base repository methods are automatically
	 * generated in the "base" version of this repository. The base repository
	 * is immutable and to be left untouched, while methods in this class
	 * are used as extension methods for more specific persistence-layer
	 * accessors or mutators.
	 *
	 * Base Methods (Subject to be expanded upon in time)
	 *
	 * Note: Not all tables are designed appropriately to fit functionality with all base methods
	 *
	 * InsertOne
	 * UpdateOne
	 * DeleteOne
	 * FindOne
	 * GetWhere(std::string where_filter)
	 * DeleteWhere(std::string where_filter)
	 * InsertMany
	 * All
	 *
	 * Example custom methods in a repository
	 *
	 * TraderRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * TraderRepository::GetWhereNeverExpires()
	 * TraderRepository::GetWhereXAndY()
	 * TraderRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */

	// Custom extended repository methods here

	static std::vector<BazaarSearchResultsFromDB_Struct>
    GetBazaarSearchResults(Database &db, BazaarSearchCriteria_Struct search, uint32 char_zone_id)
	{
		std::string search_values {};
		std::string search_criteria_trader("TRUE ");
		std::string search_criteria_items {};

		if(search.search_scope == UFBazaarSearchScope) {
			search_criteria_trader.append(fmt::format(
				" AND trader.char_entity_id = {} AND trader.char_zone_id = {}",
				search.trader_entity_id,
				Zones::BAZAAR)
			);
		} else if(search.search_scope == Local_Scope) {
			search_criteria_trader.append(fmt::format(" AND trader.char_zone_id = {}", char_zone_id));
		}
		else if(search.trader_id > 0) {
			search_criteria_trader.append(fmt::format(" AND trader.char_id = {}", search.trader_id));
		}

		if (search.min_cost != 0) {
			search_criteria_trader.append(fmt::format(" AND trader.item_cost >= {}", search.min_cost));
		}

		if (search.max_cost != 0) {
			search_criteria_trader.append(fmt::format(" AND trader.item_cost <= {}", (uint64)search.max_cost * 1000));
		}

		if (search.min_level != 1) {
			search_criteria_items.append(fmt::format(" AND items.reclevel >= {}", search.min_level));
		}

		if (search.max_level != 1) {
			search_criteria_items.append(fmt::format(" AND items.reclevel <= {}", search.max_level));
		}

		if (!std::string(search.item_name).empty()) {
			search_criteria_items.append(fmt::format(" AND items.name LIKE \"\%{}\%\" ", search.item_name));
		}

		if (search._class != 0xFFFFFFFF) {
			search_criteria_items.append(fmt::format(" AND MID(REVERSE(BIN(items.classes)), {}, 1) = 1", search._class));
		}

		if (search.race != 0xFFFFFFFF) {
			search_criteria_items.append(fmt::format(" AND MID(REVERSE(BIN(items.races)), {}, 1) = 1", search.race));
		}

		if (search.slot != 0xFFFFFFFF) {
			search_criteria_items.append(fmt::format(" AND MID(REVERSE(BIN(items.slots)), {}, 1) = 1", search.slot + 1));
		}

		// not yet implemented
		//	if (search.prestige != 0) {
		// 	   0xffffffff prestige only, 0xfffffffe non-prestige, 0 all
		//		search_criteria.append(fmt::format(" AND items.type = {} ", search.prestige));
		//	}

		if (search.augment != 0) {
			search_criteria_items.append(fmt::format(" AND MID(REVERSE(BIN(items.augtype)), {}, 1) = 1", search.augment));
		}

		switch (search.type) {
			case EQ::item::ItemType::ItemTypeAll: {
				break;
			}
			case EQ::item::ItemType::ItemTypeBook: {
				search_criteria_items.append(" AND items.itemclass = 2");
				break;
			}
			case EQ::item::ItemType::ItemTypeContainer: {
				search_criteria_items.append(" AND items.itemclass = 1");
				break;
			}
			case EQ::item::ItemType::ItemTypeAllEffects: {
				search_criteria_items.append(" AND items.scrolleffect > 0 AND items.scrolleffect < 65000");
				break;
			}
			case EQ::item::ItemType::ItemTypeUnknown9: {
				search_criteria_items.append(" AND items.worneffect = 998");
				break;
			}
			case EQ::item::ItemType::ItemTypeUnknown10: {
				search_criteria_items.append(" AND items.worneffect >= 1298 AND items.worneffect <= 1307");
				break;
			}
			case EQ::item::ItemType::ItemTypeFocusEffect: {
				search_criteria_items.append(" AND items.focuseffect > 0");
				break;
			}
			default: {
				search_criteria_items.append(fmt::format(" AND items.itemtype = {}", search.type));
			}
		}

		switch (search.item_stat) {
			case STAT_AC: {
				search_criteria_items.append(" AND items.ac > 0");
				search_values.append(", items.ac");
				break;
			}
			case STAT_AGI: {
				search_criteria_items.append(" AND items.aagi > 0");
				search_values.append(", items.aagi");
				break;
			}
			case STAT_CHA: {
				search_criteria_items.append(" AND items.acha > 0");
				search_values.append(", items.acha");
				break;
			}
			case STAT_DEX: {
				search_criteria_items.append(" AND items.adex > 0");
				search_values.append(", items.adex");
				break;
			}
			case STAT_INT: {
				search_criteria_items.append(" AND items.aint > 0");
				search_values.append(", items.aint");
				break;
			}
			case STAT_STA: {
				search_criteria_items.append(" AND items.asta > 0");
				search_values.append(", items.asta");
				break;
			}
			case STAT_STR: {
				search_criteria_items.append(" AND items.astr > 0");
				search_values.append(", items.astr");
				break;
			}
			case STAT_WIS: {
				search_criteria_items.append(" AND items.awis > 0");
				search_values.append(", items.awis");
				break;
			}
			case STAT_COLD: {
				search_criteria_items.append(" AND items.cr > 0");
				search_values.append(", items.cr");
				break;
			}
			case STAT_DISEASE: {
				search_criteria_items.append(" AND items.dr > 0");
				search_values.append(", items.dr");
				break;
			}
			case STAT_FIRE: {
				search_criteria_items.append(" AND items.fr > 0");
				search_values.append(", items.fr");
				break;
			}
			case STAT_MAGIC: {
				search_criteria_items.append(" AND items.mr > 0");
				search_values.append(", items.mr");
				break;
			}
			case STAT_POISON: {
				search_criteria_items.append(" AND items.pr > 0");
				search_values.append(", items.pr");
				break;
			}
			case STAT_HP: {
				search_criteria_items.append(" AND items.hp > 0");
				search_values.append(", items.hp");
				break;
			}
			case STAT_MANA: {
				search_criteria_items.append(" AND items.mana > 0");
				search_values.append(", items.mana");
				break;
			}
			case STAT_ENDURANCE: {
				search_criteria_items.append(" AND items.endur > 0");
				search_values.append(", items.endur");
				break;
			}
			case STAT_ATTACK: {
				search_criteria_items.append(" AND items.attack > 0");
				search_values.append(", items.attack");
				break;
			}
			case STAT_HP_REGEN: {
				search_criteria_items.append(" AND items.regen > 0");
				search_values.append(", items.regen");
				break;
			}
			case STAT_MANA_REGEN: {
				search_criteria_items.append(" AND items.manaregen > 0");
				search_values.append(", items.manaregen");
				break;
			}
			case STAT_HASTE: {
				search_criteria_items.append(" AND items.haste > 0");
				search_values.append(", items.haste");
				break;
			}
			case STAT_DAMAGE_SHIELD: {
				search_criteria_items.append(" AND items.damageshield > 0");
				search_values.append(", items.damageshield");
				break;
			}
			default: {
				search_values.append(", 0");
				break;
			}
		}

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

		auto trader_item_details = ItemsRepository::GetTraderItemDetails(
			content_db,
			results,
			search_values,
			search_criteria_items
		);

		results = db.QueryDatabase(query);
		for (auto row : results) {
			BazaarSearchResultsFromDB_Struct data{};

			data.item_id = Strings::ToInt(row[2]);
			auto it = std::find_if(
				trader_item_details.cbegin(),
				trader_item_details.cend(),
				[&](ItemsRepository::TraderItemDetails const x) { return x.item_id == data.item_id; }
			);

			if (it == std::end(trader_item_details)) {
				continue;
			}

			data.count             = Strings::ToInt(row[0]);
			data.trader_id         = Strings::ToInt(row[1]);
			data.serial_number     = Strings::ToInt(row[3]);
			data.charges           = Strings::ToInt(row[4]);
			data.cost              = Strings::ToInt(row[5]);
			data.slot_id           = Strings::ToInt(row[6]);
			data.sum_charges       = Strings::ToInt(row[7]);
			data.stackable         = it != std::end(trader_item_details) ? it->stackable : 0;
			data.icon_id           = it != std::end(trader_item_details) ? it->icon : 0;
			data.trader_zone_id    = Strings::ToInt(row[8]);
			data.trader_entity_id  = Strings::ToInt(row[9]);
			data.item_stat         = it != std::end(trader_item_details) ? it->item_stat : 0;
			data.serial_number_RoF = fmt::format("{:016}\0", Strings::ToInt(row[3]));
			data.item_name         = fmt::format("{:.63}\0", it != std::end(trader_item_details) ? it->name : std::string());
			data.trader_name       = fmt::format("{:.63}\0", std::string(row[10]).c_str());
			all_entries.push_back(data);
		}

		return all_entries;
	}

	static BulkTraders_Struct GetDistinctTraders(Database &db)
	{
		BulkTraders_Struct                  all_entries {};
		std::vector<DistinctTraders_Struct> distinct_traders;

		auto results = db.QueryDatabase("SELECT DISTINCT(t.char_id), t.char_zone_id, t.char_entity_id, c.name "
										"FROM trader AS t "
										"JOIN character_data AS c ON t.char_id = c.id;"
										);

		distinct_traders.reserve(results.RowCount());

		for (auto row : results)
		{
			DistinctTraders_Struct e {};

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

		for (auto &i : items) {
			i.item_cost = new_price;
		}

		return ReplaceMany(db, items);
	}

	static Trader GetTraderItem(Database& db, uint32 trader_id, uint32 item_id, uint32 item_cost)
	{
		Trader item{};

		auto query = fmt::format("SELECT t.char_id, t.item_id, t.serialnumber, t.charges, t.item_cost, t.slot_id, t.entity_id FROM trader AS t "
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

		auto row          = results.begin();
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
		Trader e{};
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
		Trader e{};
		auto sn = Strings::ToUnsignedBigInt(serial_number);
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
