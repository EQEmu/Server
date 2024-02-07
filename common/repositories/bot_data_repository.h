#ifndef EQEMU_BOT_DATA_REPOSITORY_H
#define EQEMU_BOT_DATA_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_bot_data_repository.h"

class BotDataRepository: public BaseBotDataRepository {
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
     * BotDataRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * BotDataRepository::GetWhereNeverExpires()
     * BotDataRepository::GetWhereXAndY()
     * BotDataRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static bool SaveAllHelmAppearances(Database& db, const uint32 owner_id, const bool show_flag)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `show_helm` = {} WHERE `owner_id` = {}",
				TableName(),
				show_flag ? 1 : 0,
				owner_id
			)
		);

		return results.Success();
	}

	static bool ToggleAllHelmAppearances(Database& db, const uint32 owner_id)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `show_helm` = (`show_helm` XOR '1') WHERE `owner_id` = {}",
				TableName(),
				owner_id
			)
		);

		return results.Success();
	}

	static bool SaveAllFollowDistances(Database& db, const uint32 owner_id, const uint32 follow_distance)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE `{}` SET `follow_distance` = {} WHERE `owner_id` = {}",
				TableName(),
				follow_distance,
				owner_id
			)
		);

		return results.Success();
	}
};

#endif //EQEMU_BOT_DATA_REPOSITORY_H
