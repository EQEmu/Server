#ifndef EQEMU_GUILD_MEMBERS_REPOSITORY_H
#define EQEMU_GUILD_MEMBERS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_guild_members_repository.h"

class GuildMembersRepository : public BaseGuildMembersRepository {
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
	 * GuildMembersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * GuildMembersRepository::GetWhereNeverExpires()
	 * GuildMembersRepository::GetWhereXAndY()
	 * GuildMembersRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */

	 // Custom extended repository methods here

	struct GuildMembersWithTributeOn {
		int32_t     char_id;
		uint32_t    guild_id;
		uint8_t     tribute_enable;
		std::string char_name;
		uint32_t     char_level;
	};

	static int UpdateMemberRank(
		Database& db,
		uint32	char_id,
		uint32	rank_id
	) {
		auto results = db.QueryDatabase(
			fmt::format("UPDATE `{}` SET `rank`= '{}' WHERE `char_id` = '{}';",
				TableName(),
				rank_id,
				char_id
			)
		);

		return results.Success() ? results.RowsAffected() : 0;
	}

	static int ReplaceOne(
		Database& db,
		const GuildMembers& e
	)
	{
		std::vector<std::string> v;

		auto columns = Columns();

		v.push_back(std::to_string(e.char_id));
		v.push_back(std::to_string(e.guild_id));
		v.push_back(std::to_string(e.rank));
		v.push_back(std::to_string(e.tribute_enable));
		v.push_back(std::to_string(e.total_tribute));
		v.push_back(std::to_string(e.last_tribute));
		v.push_back(std::to_string(e.banker));
		v.push_back("'" + Strings::Escape(e.public_note) + "'");
		v.push_back(std::to_string(e.alt));

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

	static int UpdateEnabled(Database& db, uint32 guild_id, uint32 char_id, uint32 enabled)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET `tribute_enable` = '{}' WHERE `char_id` = '{}' AND `guild_id` = '{}'",
				TableName(),
				enabled,
				char_id,
				guild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static std::vector<GuildMembersWithTributeOn> GetMembersWithTributeOn(Database& db, uint32 guild_id)
	{
		std::vector<GuildMembersWithTributeOn> all_entries;

		auto results = db.QueryDatabase(
			fmt::format("SELECT gm.char_id, gm.guild_id, gm.tribute_enable, cd.`name`, cd.`level`, cd.deleted_at "
				"FROM guild_members gm "
				"JOIN character_data cd ON cd.id = gm.char_id "
				"WHERE ISNULL(cd.deleted_at) AND gm.tribute_enable = 1 AND gm.guild_id = '{}';",
				guild_id
				)
		);

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row) {
			GuildMembersWithTributeOn e{};

			e.char_id		 = static_cast<int32_t>(atoi(row[0]));
			e.guild_id		 = static_cast<uint32_t>(strtoul(row[1], nullptr, 10));
			e.tribute_enable = static_cast<uint8_t>(strtoul(row[2], nullptr, 10));
			e.char_name		 = row[3] ? row[3] : "";
			e.char_level	 = static_cast<uint32_t>(strtoul(row[4], nullptr, 10));
			
			all_entries.push_back(e);
		}

		return all_entries;
	}

	static int UpdateFavor(Database& db, uint32 guild_id, uint32 char_id, uint32 favor)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET `total_tribute` = '{}', `last_tribute` = '{}' WHERE `char_id` = '{}' AND `guild_id` = '{}'",
				TableName(),
				favor,
				time(nullptr),
				char_id,
				guild_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

	static int UpdateOnline(Database& db, uint32 char_id, uint32 status)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"UPDATE {} SET `online` = '{}' WHERE `char_id` = '{}'",
				TableName(),
				status,
				char_id
			)
		);

		return (results.Success() ? results.RowsAffected() : 0);
	}

};
#endif //EQEMU_GUILD_MEMBERS_REPOSITORY_H
