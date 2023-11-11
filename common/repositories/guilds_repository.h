#ifndef EQEMU_GUILDS_REPOSITORY_H
#define EQEMU_GUILDS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_guilds_repository.h"

class GuildsRepository : public BaseGuildsRepository {
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
	 * GuildsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * GuildsRepository::GetWhereNeverExpires()
	 * GuildsRepository::GetWhereXAndY()
	 * GuildsRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */

	 // Custom extended repository methods here

	static int ReplaceOne(
		Database& db,
		const Guilds& e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(std::to_string(e.id));
		v.push_back("'" + Strings::Escape(e.name) + "'");
		v.push_back(std::to_string(e.leader));
		v.push_back(std::to_string(e.minstatus));
		v.push_back("'" + Strings::Escape(e.motd) + "'");
		v.push_back(std::to_string(e.tribute));
		v.push_back("'" + Strings::Escape(e.motd_setter) + "'");
		v.push_back("'" + Strings::Escape(e.channel) + "'");
		v.push_back("'" + Strings::Escape(e.url) + "'");
		v.push_back(std::to_string(e.favor));

		auto results = db.QueryDatabase(
			fmt::format(
				"REPLACE INTO {} ({}) VALUES({})",
				TableName(),
				ColumnsRaw(),
				Strings::Implode(", ", v)
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateFavor(Database& db, uint32 guild_id, uint32 favor)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET `favor` = '{}' WHERE `id` = {}",
				TableName(),
				favor,
				guild_id
				)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}
};

#endif //EQEMU_GUILDS_REPOSITORY_H
