#ifndef EQEMU_TRADER_REPOSITORY_H
#define EQEMU_TRADER_REPOSITORY_H

#include "../../common/shareddb.h"
#include "../strings.h"
#include "base/base_trader_repository.h"
#include "items_repository.h"
#include "../../common/item_data.h"
#include "../../common/races.h"
#include "../cereal/include/cereal/archives/binary.hpp"
#include "../cereal/include/cereal/types/string.hpp"

class TraderRepository : public BaseTraderRepository {
public:
	static constexpr uint32 TRADER_CONVERT_ID = 4000000000;

	struct DistinctTraders_Struct {
		uint32      trader_id;
		uint32      zone_id;
		uint32      zone_instance_id;
		uint32      entity_id;
		std::string trader_name;
	};

	struct BulkTraders_Struct {
		uint32                              count{0};
		uint32                              name_length{0};
		std::vector<DistinctTraders_Struct> traders{};
	};

	struct BazaarTraderSearch_Struct {
		Trader      trader;
		std::string trader_name;
		std::string name;
		bool        stackable;
		uint32      icon;
		uint32      stats;
	};

	struct WelcomeData_Struct {
		uint32 count_of_traders;
		uint32 count_of_items;
	};

	static std::vector<BazaarSearchResultsFromDB_Struct>
	GetBazaarSearchResults(
		SharedDatabase &db,
		BazaarSearchCriteria_Struct search,
		uint32 char_zone_id,
		int32 char_zone_instance_id
	);

	static BulkTraders_Struct GetDistinctTraders(
		Database &db,
		uint32 char_zone_instance_id,
		uint32 max_results = std::numeric_limits<uint32>::max()
		)
	{
		BulkTraders_Struct                  all_entries{};
		std::vector<DistinctTraders_Struct> distinct_traders;
		MySQLRequestResult                  results;

		if (RuleB(Bazaar, UseAlternateBazaarSearch)) {
			results = db.QueryDatabase(fmt::format(
				"SELECT DISTINCT(t.character_id), t.char_zone_id, t.char_zone_instance_id, t.char_entity_id, c.name "
				"FROM trader AS t "
				"JOIN character_data AS c ON t.character_id = c.id "
				"WHERE t.char_zone_instance_id = {} "
				"ORDER BY t.char_zone_instance_id ASC "
				"LIMIT {}",
				char_zone_instance_id,
				max_results)
			);
		}
		else {
			results = db.QueryDatabase(
				fmt::format(
					"SELECT DISTINCT(t.character_id), t.char_zone_id, t.char_zone_instance_id, t.char_entity_id, c.name "
					"FROM trader AS t "
					"JOIN character_data AS c ON t.character_id = c.id "
					"ORDER BY t.char_zone_instance_id ASC "
					"LIMIT {}",
					max_results)
			);
		}

		distinct_traders.reserve(results.RowCount());

		for (auto row: results) {
			DistinctTraders_Struct e{};

			e.trader_id        = Strings::ToInt(row[0]);
			e.zone_id          = Strings::ToInt(row[1]);
			e.zone_instance_id = Strings::ToInt(row[2]);
			e.entity_id        = Strings::ToInt(row[3]);
			e.trader_name      = row[4] ? row[4] : "";
			all_entries.name_length += e.trader_name.length() + 1;

			all_entries.traders.push_back(e);
		}
		all_entries.count = results.RowCount();
		return all_entries;
	}

	static WelcomeData_Struct GetWelcomeData(Database &db)
	{
		WelcomeData_Struct e{};

		auto results = db.QueryDatabase("SELECT COUNT(DISTINCT character_id), count(character_id) FROM trader;");

		if (!results.RowCount()) {
			return e;
		}

		auto r = results.begin();
		e.count_of_traders = Strings::ToInt(r[0]);
		e.count_of_items   = Strings::ToInt(r[1]);
		return e;
	}

	static int UpdateItem(Database &db, uint32 character_id, uint32 new_price, uint32 item_id, uint32 item_charges)
	{
		std::vector<BaseTraderRepository::Trader> items{};
		if (item_charges == 0) {
			items = GetWhere(
				db,
				fmt::format(
					"character_id = '{}' AND item_id = '{}'",
					character_id,
					item_id
				)
			);
		}
		else {
			items = GetWhere(
				db,
				fmt::format(
					"character_id = '{}' AND item_id = '{}' AND item_charges = '{}'",
					character_id,
					item_id,
					item_charges
				)
			);
		}

		if (items.empty()) {
			return 0;
		}

		for (auto &i: items) {
			i.item_cost    = new_price;
			i.listing_date = time(nullptr);
		}

		return ReplaceMany(db, items);
	}

	static Trader GetTraderItem(Database &db, uint32 trader_id, uint32 item_id, uint32 item_cost)
	{
		Trader item{};

		auto query   = fmt::format(
			"SELECT t.character_id, t.item_id, t.item_unique.id, t.charges, t.item_cost, t.slot_id, t.entity_id FROM trader AS t "
			"WHERE t.entity_id = '{}' AND t.item_id = '{}' AND t.item_cost = '{}' "
			"LIMIT 1;",
			trader_id,
			item_id,
			item_cost
		);
		auto results = db.QueryDatabase(query);

		if (results.RowCount() == 0) {
			return item;
		}

		auto row            = results.begin();
		item.character_id   = Strings::ToInt(row[0]);
		item.item_id        = Strings::ToInt(row[1]);
		item.item_unique_id = row[2] ? row[2] : "";
		item.item_charges   = Strings::ToInt(row[3]);
		item.item_cost      = Strings::ToInt(row[4]);
		item.slot_id        = Strings::ToInt(row[5]);

		return item;
	}

	static int UpdateQuantity(Database &db, const std::string &item_unique_id, int16 quantity)
	{
		const auto trader_item = GetWhere(
			db,
			fmt::format("`item_unique_id` = '{}' ", item_unique_id)
		);

		if (trader_item.empty() || trader_item.size() > 1) {
			return 0;
		}

		auto m         = trader_item[0];
		m.item_charges = quantity;
		m.listing_date = time(nullptr);

		return UpdateOne(db, m);
	}

	static std::vector<Trader> UpdatePrice(Database &db, const std::string &item_unique_id, uint32 price)
	{
		std::vector<Trader> all_entries{};

		const auto query = fmt::format(
			"UPDATE trader t1 SET t1.`item_cost` = '{}', t1.`listing_date` = FROM_UNIXTIME({}) WHERE t1.`item_id` = "
			"(SELECT t2.`item_id` FROM trader t2 WHERE t2.`item_unique_id` = '{}')",
			price,
			time(nullptr),
			item_unique_id
		);

		auto results = db.QueryDatabase(query);
		if (results.RowsAffected() == 0) {
			return all_entries;
		}

		all_entries = GetWhere(
			db,
			fmt::format(
				"`item_id` = (SELECT t1.`item_id` FROM trader t1 WHERE t1.`item_unique_id` = '{}');",
				item_unique_id
			)
		);

		return all_entries;
	}

	static Trader GetItemByItemUniqueNumber(Database &db, std::string &item_unique_id)
	{
		Trader     e{};
		const auto trader_item = GetWhere(
			db,
			fmt::format("`item_unique_id` = '{}' LIMIT 1", item_unique_id)
		);

		if (trader_item.empty()) {
			return e;
		}

		return trader_item.at(0);
	}

	static Trader GetItemByItemUniqueNumber(Database &db, const char* item_unique_id)
	{
		Trader     e{};
		const auto trader_item = GetWhere(
			db,
			fmt::format("`item_unique_id` = '{}' LIMIT 1", item_unique_id)
		);

		if (trader_item.empty()) {
			return e;
		}

		return trader_item.at(0);
	}

	static int UpdateActiveTransaction(Database &db, uint32 id, bool status)
	{
		auto e = FindOne(db, id);
		if (!e.id) {
			return 0;
		}

		e.active_transaction = status == true ? 1 : 0;
		e.listing_date       = time(nullptr);

		return UpdateOne(db, e);
	}

	static int DeleteMany(Database &db, const std::vector<Trader> &entries)
	{
		std::vector<std::string> delete_ids;

		for (auto const &e: entries) {
			delete_ids.push_back(std::to_string(e.id));
		}

		if (delete_ids.empty()) {
			return 0;
		}

		return DeleteWhere(db, fmt::format("`id` IN({})", Strings::Implode(",", delete_ids)));
	}

	static DistinctTraders_Struct GetTraderByItemUniqueNumber(Database &db, std::string &item_unique_id)
	{
		DistinctTraders_Struct trader{};

		auto query = fmt::format(
			"SELECT t.id, t.character_id, c.name "
			"FROM trader AS t "
			"JOIN character_data AS c ON c.id = t.character_id "
			"WHERE t.item_unique_id = '{}' LIMIT 1",
			item_unique_id
		);

		auto results = db.QueryDatabase(query);

		if (results.RowCount() == 0) {
			return trader;
		}

		auto        row    = results.begin();
		trader.trader_id   = Strings::ToUnsignedInt(row[1]);
		trader.trader_name = row[2] ? row[2] : "";

		return trader;
	}

	static std::vector<BazaarTraderSearch_Struct> GetBazaarTraderDetails(
		Database &db,
		const std::string &search_criteria_trader,
		const std::string &name,
		const std::string &field_criteria_items,
		const std::string &where_criteria_items,
		uint32 max_results
		)
	{
		std::vector<BazaarTraderSearch_Struct> all_entries{};

		// auto query_2 = fmt::format(
		// "WITH ranked_trader_items AS ("
		// 	"SELECT trader.id, trader.character_id, trader.item_id, trader.item_unique_id, trader.augment_one, "
		// 	"trader.augment_two, trader.augment_three, trader.augment_four, trader.augment_five, trader.augment_six, "
		// 	"trader.item_charges, trader.item_cost, trader.slot_id, trader.char_entity_id, trader.char_zone_id, "
		// 	"trader.char_zone_instance_id, trader.active_transaction, c.`name`, "
		// 	"items.name AS n1, items.stackable, items.icon, {}, "
		// 	"ROW_NUMBER() OVER (PARTITION BY trader.character_id) AS row_num "
		// 	"FROM trader "
		// 	"INNER JOIN character_data AS c ON trader.character_id = c.id "
		// 	"JOIN peq642024_content.items AS items ON trader.item_id = items.id "
		// 	"WHERE items.`name` LIKE '%{}%' AND {} AND {}"
		// ") "
		// "SELECT * FROM ranked_trader_items "
		// "WHERE row_num <= '{}';",
		// field_criteria_items,
		// Strings::Escape(name),
		// where_criteria_items,
		// search_criteria_trader,
		// max_results
		// );

		auto query = fmt::format(
			"SELECT trader.id, trader.character_id, trader.item_id, trader.item_unique_id, trader.augment_one, "
			"trader.augment_two, trader.augment_three, trader.augment_four, trader.augment_five, trader.augment_six, "
			"trader.item_charges, trader.item_cost, trader.slot_id, trader.char_entity_id, trader.char_zone_id, "
			"trader.char_zone_instance_id, trader.active_transaction, c.`name` FROM `trader` "
			"INNER JOIN character_data AS c ON trader.character_id = c.id "
			"WHERE {} "
			"ORDER BY trader.character_id ASC",
			search_criteria_trader
		);

		auto results = db.QueryDatabase(query);

		if (results.RowCount() == 0) {
			return all_entries;
		}

		all_entries.reserve(results.RowCount());
		for (auto row = results.begin(); row != results.end(); ++row) {
			BazaarTraderSearch_Struct e{};

			e.trader.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
			e.trader.character_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
			e.trader.item_id               = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
			e.trader.item_unique_id        = row[3] ? row[3] : std::string("");
			e.trader.augment_one           = row[4] ? static_cast<uint32_t>(strtoul(row[4], nullptr, 10)) : 0;
			e.trader.augment_two           = row[5] ? static_cast<uint32_t>(strtoul(row[5], nullptr, 10)) : 0;
			e.trader.augment_three         = row[6] ? static_cast<uint32_t>(strtoul(row[6], nullptr, 10)) : 0;
			e.trader.augment_four          = row[7] ? static_cast<uint32_t>(strtoul(row[7], nullptr, 10)) : 0;
			e.trader.augment_five          = row[8] ? static_cast<uint32_t>(strtoul(row[8], nullptr, 10)) : 0;
			e.trader.augment_six           = row[9] ? static_cast<uint32_t>(strtoul(row[9], nullptr, 10)) : 0;
			e.trader.item_charges          = row[10] ? static_cast<int32_t>(atoi(row[10])) : 0;
			e.trader.item_cost             = row[11] ? static_cast<uint32_t>(strtoul(row[11], nullptr, 10)) : 0;
			e.trader.slot_id               = row[12] ? static_cast<uint8_t>(strtoul(row[12], nullptr, 10)) : 0;
			e.trader.char_entity_id        = row[13] ? static_cast<uint32_t>(strtoul(row[13], nullptr, 10)) : 0;
			e.trader.char_zone_id          = row[14] ? static_cast<uint32_t>(strtoul(row[14], nullptr, 10)) : 0;
			e.trader.char_zone_instance_id = row[15] ? static_cast<int32_t>(atoi(row[15])) : 0;
			e.trader.active_transaction    = row[16] ? static_cast<uint8_t>(strtoul(row[16], nullptr, 10)) : 0;
			e.trader_name                  = row[17] ? row[17] : std::string("");
			// e.name                         = row[18] ? row[18] : "";
			// e.stackable                    = atoi(row[19]) ? true : false;
			// e.icon                         = row[20] ? static_cast<int32_t>(atoi(row[20])) : 0;
			// e.stats                        = row[21] ? static_cast<int32_t>(atoi(row[21])) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}

	static Trader GetAccountZoneIdAndInstanceIdByAccountId(Database &db, uint32 account_id)
	{
		auto trader_query = fmt::format(
			"SELECT t.id, t.character_id, t.char_zone_id, t.char_zone_instance_id "
			"FROM trader AS t "
			"WHERE t.character_id IN(SELECT c.id FROM character_data AS c WHERE c.account_id = '{}') "
			"LIMIT 1;",
			account_id
		);

		auto buyer_query = fmt::format(
			"SELECT t.id, t.char_id, t.char_zone_id, t.char_zone_instance_id "
			"FROM buyer AS t "
			"WHERE t.char_id IN(SELECT c.id FROM character_data AS c WHERE c.account_id = '{}') "
			"LIMIT 1;",
			account_id
		);

		Trader e{};

		auto trader_results = db.QueryDatabase(trader_query);
		auto buyer_results  = db.QueryDatabase(buyer_query);
		if (trader_results.RowCount() == 0 && buyer_results.RowCount() == 0) {
			return e;
		}

		MySQLRequestRow row;
		if (trader_results.RowCount() > 0) {
			row = trader_results.begin();
		}

		if (buyer_results.RowCount() > 0) {
			row = buyer_results.begin();
		}

		e.id                    = row[0] ? strtoull(row[0], nullptr, 10) : 0;
		e.character_id          = row[1] ? static_cast<uint32_t>(strtoul(row[1], nullptr, 10)) : 0;
		e.char_zone_id          = row[2] ? static_cast<uint32_t>(strtoul(row[2], nullptr, 10)) : 0;
		e.char_zone_instance_id = row[3] ? static_cast<int32_t>(atoi(row[3])) : 0;

		return e;
	}
};

#endif //EQEMU_TRADER_REPOSITORY_H
