/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef EQEMU_EXPEDITIONS_REPOSITORY_H
#define EQEMU_EXPEDITIONS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"
#include "base/base_expeditions_repository.h"

class ExpeditionsRepository: public BaseExpeditionsRepository {
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
     * ExpeditionsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * ExpeditionsRepository::GetWhereNeverExpires()
     * ExpeditionsRepository::GetWhereXAndY()
     * ExpeditionsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	struct CharacterExpedition
	{
		uint32_t    id;
		std::string name;
		uint32_t    expedition_id;
	};

	static std::vector<CharacterExpedition> GetCharactersWithExpedition(
		Database& db, const std::vector<std::string>& character_names)
	{
		if (character_names.empty())
		{
			return {};
		}

		std::vector<CharacterExpedition> entries;

		auto joined_character_names = fmt::format("'{}'", fmt::join(character_names, "','"));

		auto results = db.QueryDatabase(fmt::format(SQL(
			SELECT
				character_data.id,
				character_data.name,
				MAX(expeditions.id)
			FROM character_data
				LEFT JOIN expedition_members
					ON character_data.id = expedition_members.character_id
					AND expedition_members.is_current_member = TRUE
				LEFT JOIN expeditions
					ON expedition_members.expedition_id = expeditions.id
			WHERE character_data.name IN ({})
			GROUP BY character_data.id
			ORDER BY FIELD(character_data.name, {})
		),
			joined_character_names,
			joined_character_names
		));

		if (results.Success())
		{
			entries.reserve(results.RowCount());

			for (auto row = results.begin(); row != results.end(); ++row)
			{
				CharacterExpedition entry{};
				entry.id            = std::strtoul(row[0], nullptr, 10);
				entry.name          = row[1];
				entry.expedition_id = row[2] ? std::strtoul(row[2], nullptr, 10) : 0;

				entries.emplace_back(std::move(entry));
			}
		}

		return entries;
	}
};

#endif //EQEMU_EXPEDITIONS_REPOSITORY_H
