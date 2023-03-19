#ifndef EQEMU_INSTANCE_LIST_PLAYER_REPOSITORY_H
#define EQEMU_INSTANCE_LIST_PLAYER_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_instance_list_player_repository.h"

class InstanceListPlayerRepository: public BaseInstanceListPlayerRepository {
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
     * InstanceListPlayerRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * InstanceListPlayerRepository::GetWhereNeverExpires()
     * InstanceListPlayerRepository::GetWhereXAndY()
     * InstanceListPlayerRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static int InsertOrUpdateMany(Database& db,
		const std::vector<InstanceListPlayer>& instance_list_player_entries)
	{
		std::vector<std::string> insert_chunks;

		for (auto &instance_list_player_entry: instance_list_player_entries)
		{
			std::vector<std::string> insert_values;

			insert_values.push_back(std::to_string(instance_list_player_entry.id));
			insert_values.push_back(std::to_string(instance_list_player_entry.charid));

			insert_chunks.push_back("(" + Strings::Implode(",", insert_values) + ")");
		}

		std::vector<std::string> insert_values;

		auto results = db.QueryDatabase(
			fmt::format(
				"INSERT INTO {} ({}) VALUES {} ON DUPLICATE KEY UPDATE id = VALUES(id)",
				TableName(),
				ColumnsRaw(),
				Strings::Implode(",", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static bool ReplaceOne(Database& db, InstanceListPlayer e)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.charid));

		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO {} VALUES ({})",
				TableName(),
				Strings::Implode(",", v)
			)
		);
		if (results.Success()) {
			return true;
		}

		return false;
	}
};

#endif //EQEMU_INSTANCE_LIST_PLAYER_REPOSITORY_H
