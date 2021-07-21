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

#ifndef EQEMU_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H
#define EQEMU_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"
#include "base/base_character_instance_safereturns_repository.h"

class CharacterInstanceSafereturnsRepository: public BaseCharacterInstanceSafereturnsRepository {
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
     * CharacterInstanceSafereturnsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterInstanceSafereturnsRepository::GetWhereNeverExpires()
     * CharacterInstanceSafereturnsRepository::GetWhereXAndY()
     * CharacterInstanceSafereturnsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	static CharacterInstanceSafereturns InsertOneOrUpdate(
		Database& db, CharacterInstanceSafereturns& character_instance_safereturns_entry)
	{
		std::vector<std::string> insert_values;

		insert_values.push_back(std::to_string(character_instance_safereturns_entry.id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.character_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.instance_zone_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.instance_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_zone_id));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_x));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_y));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_z));
		insert_values.push_back(std::to_string(character_instance_safereturns_entry.safe_heading));

		auto results = db.QueryDatabase(fmt::format(SQL(
			{} VALUES ({})
			ON DUPLICATE KEY UPDATE
				instance_zone_id = VALUES(instance_zone_id),
				instance_id      = VALUES(instance_id),
				safe_zone_id     = VALUES(safe_zone_id),
				safe_x           = VALUES(safe_x),
				safe_y           = VALUES(safe_y),
				safe_z           = VALUES(safe_z),
				safe_heading     = VALUES(safe_heading)
		),
			BaseInsert(),
			implode(",", insert_values)
		));

		if (results.Success())
		{
			character_instance_safereturns_entry.id = results.LastInsertedID();
			return character_instance_safereturns_entry;
		}

		return NewEntity();
	}
};

#endif //EQEMU_CHARACTER_INSTANCE_SAFERETURNS_REPOSITORY_H
