#ifndef EQEMU_ITEMS_REPOSITORY_H
#define EQEMU_ITEMS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_items_repository.h"

class ItemsRepository: public BaseItemsRepository {
public:
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


};

#endif //EQEMU_ITEMS_REPOSITORY_H
