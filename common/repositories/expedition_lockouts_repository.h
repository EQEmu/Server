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

#ifndef EQEMU_EXPEDITION_LOCKOUTS_REPOSITORY_H
#define EQEMU_EXPEDITION_LOCKOUTS_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"
#include "base/base_expedition_lockouts_repository.h"

class ExpeditionLockoutsRepository: public BaseExpeditionLockoutsRepository {
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
     * ExpeditionLockoutsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * ExpeditionLockoutsRepository::GetWhereNeverExpires()
     * ExpeditionLockoutsRepository::GetWhereXAndY()
     * ExpeditionLockoutsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here

	struct ExpeditionLockoutsWithTimestamp {
		uint32_t    id;
		uint32_t    expedition_id;
		std::string event_name;
		time_t      expire_time;
		int         duration;
		std::string from_expedition_uuid;
	};

	static std::vector<ExpeditionLockoutsWithTimestamp> GetWithTimestamp(
		Database& db, const std::vector<uint32_t>& expedition_ids)
	{
		if (expedition_ids.empty())
		{
			return {};
		}

		std::vector<ExpeditionLockoutsWithTimestamp> all_entries;

		auto results = db.QueryDatabase(fmt::format(SQL(
			SELECT
				id,
				expedition_id,
				event_name,
				UNIX_TIMESTAMP(expire_time),
				duration,
				from_expedition_uuid
			FROM expedition_lockouts
			WHERE expedition_id IN ({})
		),
			fmt::join(expedition_ids, ",")
		));

		all_entries.reserve(results.RowCount());

		for (auto row = results.begin(); row != results.end(); ++row)
		{
			ExpeditionLockoutsWithTimestamp entry{};

			int col = 0;
			entry.id                   = strtoul(row[col++], nullptr, 10);
			entry.expedition_id        = strtoul(row[col++], nullptr, 10);
			entry.event_name           = row[col++];
			entry.expire_time          = strtoull(row[col++], nullptr, 10);
			entry.duration             = strtol(row[col++], nullptr, 10);
			entry.from_expedition_uuid = row[col++];

			all_entries.emplace_back(std::move(entry));
		}

		return all_entries;
	}
};

#endif //EQEMU_EXPEDITION_LOCKOUTS_REPOSITORY_H
