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

		auto results = db.QueryDatabase(fmt::format(
			"SELECT DISTINCT(t.char_id), t.char_zone_id, t.char_zone_instance_id, t.char_entity_id, c.name "
			"FROM trader AS t "
			"JOIN character_data AS c ON t.char_id = c.id "
			"WHERE t.char_zone_instance_id = {} "
			"ORDER BY t.char_zone_instance_id ASC "
			"LIMIT {}",
			char_zone_instance_id,
			max_results)
		);

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

	static Trader GetItemBySerialNumber(Database &db, uint32 serial_number, uint32 trader_id)
	{
		Trader     e{};
		const auto trader_item = GetWhere(
			db,
			fmt::format("`char_id` = '{}' AND `item_sn` = '{}' LIMIT 1", trader_id, serial_number)
		);

		if (trader_item.empty()) {
			return e;
		}

		return trader_item.at(0);
	}

	static Trader GetItemBySerialNumber(Database &db, std::string serial_number, uint32 trader_id)
	{
		Trader     e{};
		auto       sn          = Strings::ToUnsignedBigInt(serial_number);
		const auto trader_item = GetWhere(
			db,
			fmt::format("`char_id` = '{}' AND `item_sn` = '{}' LIMIT 1", trader_id, sn)
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

	static DistinctTraders_Struct GetTraderByInstanceAndSerialnumber(
		Database &db,
		uint32 instance_id,
		const char *serial_number
		)
	{
		DistinctTraders_Struct trader{};

		auto query = fmt::format(
			"SELECT t.id, t.char_id, c.name "
			"FROM trader AS t "
			"JOIN character_data AS c ON c.id = t.char_id "
			"WHERE t.char_zone_id = 151 AND t.char_zone_instance_id = {} AND t.item_sn = {} LIMIT 1",
			instance_id,
			serial_number
		);

		auto results = db.QueryDatabase(query);

		if (results.RowCount() == 0) {
			return trader;
		}

		auto        row    = results.begin();
		std::string name   = row[2];
		trader.trader_id   = Strings::ToUnsignedInt(row[1]);
		trader.trader_name = row[2] ? row[2] : "";

		return trader;
	}
};

#endif //EQEMU_TRADER_REPOSITORY_H
