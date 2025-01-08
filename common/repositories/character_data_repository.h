#ifndef EQEMU_CHARACTER_DATA_REPOSITORY_H
#define EQEMU_CHARACTER_DATA_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_character_data_repository.h"



class CharacterDataRepository: public BaseCharacterDataRepository {
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
     * CharacterDataRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterDataRepository::GetWhereNeverExpires()
     * CharacterDataRepository::GetWhereXAndY()
     * CharacterDataRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static uint32 GetSecondsSinceLastLogin(Database &db, const std::string& name)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT (UNIX_TIMESTAMP(NOW()) - last_login) FROM {} WHERE name = '{}'",
				TableName(),
				Strings::Escape(name)
			)
		);

		if (!results.RowCount() || !results.Success()) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}

	static CharacterData FindByName(
		Database& db,
		const std::string& character_name
	)
	{
		auto l = CharacterDataRepository::GetWhere(
			db,
			fmt::format(
				"`name` = '{}' LIMIT 1",
				Strings::Escape(character_name)
			)
		);

		return l.empty() ? CharacterDataRepository::NewEntity() : l.front();
	}

	struct InstancePlayerCount {
		int32_t instance_id;
		uint32_t zone_id;
		uint32_t player_count;
	};

	static std::vector<InstancePlayerCount> GetInstanceZonePlayerCounts(Database& db, int zone_id) {
		std::vector<InstancePlayerCount> zone_player_counts;

		auto query = fmt::format(SQL(
			SELECT
				zone_id,
				0 AS instance_id,
				COUNT(id) AS player_count
				FROM
				character_data
			WHERE
				zone_instance = 0
				AND zone_id = {}
				AND last_login >= UNIX_TIMESTAMP(NOW()) - 600
			GROUP BY
				zone_id
			ORDER BY
				zone_id, player_count DESC
		), zone_id);

		auto results = db.QueryDatabase(query);
		for (auto row = results.begin(); row != results.end(); ++row) {
			InstancePlayerCount e{};
			e.zone_id      = std::stoi(row[0]);
			e.instance_id  = 0;
			e.player_count = std::stoi(row[2]);
			zone_player_counts.push_back(e);
		}

		if (zone_player_counts.empty()) {
			InstancePlayerCount e{};
			e.zone_id      = zone_id;
			e.instance_id  = 0;
			e.player_count = 0;
			zone_player_counts.push_back(e);
		}

		query = fmt::format(
			SQL(
				SELECT
				i.id AS instance_id,
				i.zone AS zone_id,
				COUNT(c.id) AS player_count
				FROM
				instance_list  i
				LEFT           JOIN
				character_data c
				ON
				i.zone = c.zone_id
				AND i.id = c.zone_instance
				AND c.last_login >= UNIX_TIMESTAMP(NOW()) - 600
				WHERE
				i.zone IS NOT NULL AND i.zone = {}
				GROUP BY
				i.id, i.zone, i.version
				ORDER BY
				i.id ASC;
			), zone_id
		);

		results = db.QueryDatabase(query);
		if (!results.Success() || results.RowCount() == 0) {
			return zone_player_counts;
		}

		for (auto row = results.begin(); row != results.end(); ++row) {
			InstancePlayerCount e{};
			e.instance_id  = std::stoi(row[0]);
			e.zone_id      = std::stoi(row[1]);
			e.player_count = std::stoi(row[2]);
			zone_player_counts.push_back(e);
		}

		return zone_player_counts;
	}
};

#endif //EQEMU_CHARACTER_DATA_REPOSITORY_H
