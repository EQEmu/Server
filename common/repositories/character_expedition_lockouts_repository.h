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

#ifndef EQEMU_CHARACTER_EXPEDITION_LOCKOUTS_REPOSITORY_H
#define EQEMU_CHARACTER_EXPEDITION_LOCKOUTS_REPOSITORY_H

#include "../database.h"
#include "../expedition_lockout_timer.h"
#include "../string_util.h"
#include "base/base_character_expedition_lockouts_repository.h"
#include <unordered_map>

class CharacterExpeditionLockoutsRepository: public BaseCharacterExpeditionLockoutsRepository {
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
     * CharacterExpeditionLockoutsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * CharacterExpeditionLockoutsRepository::GetWhereNeverExpires()
     * CharacterExpeditionLockoutsRepository::GetWhereXAndY()
     * CharacterExpeditionLockoutsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	struct CharacterExpeditionLockoutsTimeStamp {
		int         id;
		int         character_id;
		std::string expedition_name;
		std::string event_name;
		time_t      expire_time;
		int         duration;
		std::string from_expedition_uuid;
	};

	static ExpeditionLockoutTimer GetExpeditionLockoutTimerFromEntry(
		CharacterExpeditionLockoutsTimeStamp&& entry)
	{
		ExpeditionLockoutTimer lockout_timer{
			std::move(entry.from_expedition_uuid),
			std::move(entry.expedition_name),
			std::move(entry.event_name),
			static_cast<uint64_t>(entry.expire_time),
			static_cast<uint32_t>(entry.duration)
		};

		return lockout_timer;
	}

	static std::unordered_map<uint32_t, std::vector<ExpeditionLockoutTimer>> GetManyCharacterLockoutTimers(
		Database& db, const std::vector<uint32_t>& character_ids,
		const std::string& expedition_name, const std::string& ordered_event_name)
	{
		auto joined_character_ids = fmt::join(character_ids, ",");

		auto results = db.QueryDatabase(fmt::format(SQL(
			SELECT
				character_id,
				UNIX_TIMESTAMP(expire_time),
				duration,
				event_name,
				from_expedition_uuid
			FROM character_expedition_lockouts
			WHERE
				character_id IN ({})
				AND expire_time > NOW()
				AND expedition_name = '{}'
			ORDER BY
				FIELD(character_id, {}),
				FIELD(event_name, '{}') DESC
		),
			joined_character_ids,
			EscapeString(expedition_name),
			joined_character_ids,
			EscapeString(ordered_event_name)
		));

		std::unordered_map<uint32_t, std::vector<ExpeditionLockoutTimer>> lockouts;

		for (auto row = results.begin(); row != results.end(); ++row)
		{
			CharacterExpeditionLockoutsTimeStamp entry{};

			int col = 0;
			entry.character_id         = std::strtoul(row[col++], nullptr, 10);
			entry.expire_time          = std::strtoull(row[col++], nullptr, 10);
			entry.duration             = std::strtoul(row[col++], nullptr, 10);
			entry.event_name           = row[col++];
			entry.expedition_name      = expedition_name;
			entry.from_expedition_uuid = row[col++];

			auto lockout = GetExpeditionLockoutTimerFromEntry(std::move(entry));
			lockouts[entry.character_id].emplace_back(std::move(lockout));
		}

		return lockouts;
	}
};

#endif //EQEMU_CHARACTER_EXPEDITION_LOCKOUTS_REPOSITORY_H
