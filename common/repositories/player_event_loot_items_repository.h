#ifndef EQEMU_PLAYER_EVENT_LOOT_ITEMS_REPOSITORY_H
#define EQEMU_PLAYER_EVENT_LOOT_ITEMS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_player_event_loot_items_repository.h"

class PlayerEventLootItemsRepository: public BasePlayerEventLootItemsRepository {
public:

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
	 * PlayerEventLootItemsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * PlayerEventLootItemsRepository::GetWhereNeverExpires()
	 * PlayerEventLootItemsRepository::GetWhereXAndY()
	 * PlayerEventLootItemsRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */
	static int InsertManyFromStdAny(Database &db, const std::vector<std::any> &entries)
	{
		std::vector<std::string> insert_chunks;

		for (auto &r: entries) {
			auto                     e = std::any_cast<PlayerEventLootItems>(r);
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.item_id));
			v.push_back("'" + Strings::Escape(e.item_name) + "'");
			v.push_back(std::to_string(e.charges));
			v.push_back(std::to_string(e.npc_id));
			v.push_back("'" + Strings::Escape(e.corpse_name) + "'");

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results =
			db.QueryDatabase(fmt::format("{} VALUES {}", BaseInsert(), Strings::Implode(",", insert_chunks)));

		return results.Success() ? results.RowsAffected() : 0;
	}
};

#endif //EQEMU_PLAYER_EVENT_LOOT_ITEMS_REPOSITORY_H
