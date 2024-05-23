#ifndef EQEMU_ITEMS_REPOSITORY_H
#define EQEMU_ITEMS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_items_repository.h"

class ItemsRepository: public BaseItemsRepository {
public:
	struct TraderItemDetails{
		std::string name;
		uint32      item_id;
		int32       stackable;
		int32       icon;
		int32       item_stat;
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
     * ItemsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * ItemsRepository::GetWhereNeverExpires()
     * ItemsRepository::GetWhereXAndY()
     * ItemsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static std::vector<int32> GetItemIDsBySearchCriteria(
		Database& db,
		std::string search_string,
		int query_limit = 0
	)
	{
		auto query = fmt::format(
			"SELECT `id` FROM {} WHERE LOWER(`name`) LIKE '%%{}%%' ORDER BY id ASC",
			TableName(),
			Strings::Escape(search_string)
		);

		if (query_limit >= 1) {
			query += fmt::format(" LIMIT {}", query_limit);
		}

		std::vector<int32> item_id_list;

		auto results = db.QueryDatabase(query);
		if (!results.Success() || !results.RowCount()) {
			return item_id_list;
		}

		for (auto row : results) {
			item_id_list.emplace_back(Strings::ToInt(row[0]));
		}

		return item_id_list;
	}

	static std::vector<TraderItemDetails> GetTraderItemDetails(
		Database &db,
		MySQLRequestResult &trader_result,
		std::string &search_values,
		std::string &search_criteria_items
	)
	{
		std::vector<std::string> ids{};

		for (auto row: trader_result) {
			ids.push_back(row[2]);
		}

		auto query         = fmt::format(
			"SELECT name, id, stackable, icon {} FROM items WHERE id IN({}) {}",
			search_values,
			Strings::Implode(",", ids),
			search_criteria_items
		);
		auto results_items = content_db.QueryDatabase(query);

		std::vector<TraderItemDetails> all_entries{};
		if (!results_items.Success()) {
			return all_entries;
		}

		for (auto r: results_items) {
			TraderItemDetails e{};
			e.name      = r[0];
			e.item_id   = r[1] ? static_cast<uint32_t>(atoi(r[1])) : 0;
			e.stackable = r[2] ? static_cast<int32_t>(atoi(r[2])) : 0;
			e.icon      = r[3] ? static_cast<int32_t>(atoi(r[3])) : 0;
			e.item_stat = r[4] ? static_cast<int32_t>(atoi(r[4])) : 0;
			all_entries.push_back(e);
		}

		return all_entries;
	}
};

#endif //EQEMU_ITEMS_REPOSITORY_H
