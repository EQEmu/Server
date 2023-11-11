#ifndef EQEMU_GUILD_PERMISSIONS_REPOSITORY_H
#define EQEMU_GUILD_PERMISSIONS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_guild_permissions_repository.h"

class GuildPermissionsRepository: public BaseGuildPermissionsRepository {
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
     * GuildPermissionsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * GuildPermissionsRepository::GetWhereNeverExpires()
     * GuildPermissionsRepository::GetWhereXAndY()
     * GuildPermissionsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static int UpdateOne(
		Database& db,
		const GuildPermissions& e
	)
	{
		std::vector<std::string> v;

		v.push_back(std::to_string(e.id));
		v.push_back(std::to_string(e.perm_id));
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.permission));

		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO {} ({}) VALUES({})",
				TableName(),
				ColumnsRaw(),
//				Strings::Implode(", ", columns),
				Strings::Implode(", ", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int ReplaceMany(
		Database& db,
		const std::vector<GuildPermissions>& entries
	)
	{
		std::vector<std::string> insert_chunks;

		for (auto& e : entries) {
			std::vector<std::string> v;

			v.push_back(std::to_string(e.id));
			v.push_back(std::to_string(e.perm_id));
			v.push_back(std::to_string(e.guild_id));
			v.push_back(std::to_string(e.permission));

			insert_chunks.push_back("(" + Strings::Implode(",", v) + ")");
		}

		std::vector<std::string> v;

		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO {} ({}) VALUES{}",
				TableName(),
				ColumnsRaw(),
					Strings::Implode(", ", insert_chunks)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};

#endif //EQEMU_GUILD_PERMISSIONS_REPOSITORY_H
