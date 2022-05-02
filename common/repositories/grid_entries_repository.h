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

#ifndef EQEMU_GRID_ENTRIES_REPOSITORY_H
#define EQEMU_GRID_ENTRIES_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class GridEntriesRepository {
public:
	struct GridEntry {
		int   gridid;
		int   zoneid;
		int   number;
		float x;
		float y;
		float z;
		float heading;
		int   pause;
		int8  centerpoint;
	};

	static std::vector<std::string> Columns()
	{
		return {
			"gridid",
			"zoneid",
			"number",
			"x",
			"y",
			"z",
			"heading",
			"pause",
			"centerpoint",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("grid_entries");
	}

	static std::string BaseSelect()
	{
		return std::string(
			fmt::format(
				"SELECT {} FROM {}",
				ColumnsRaw(),
				TableName()
			)
		);
	}

	static GridEntry NewEntity()
	{
		GridEntry entry{};

		entry.gridid      = 0;
		entry.zoneid      = 0;
		entry.number      = 0;
		entry.x           = 0;
		entry.y           = 0;
		entry.z           = 0;
		entry.heading     = 0;
		entry.pause       = 0;
		entry.centerpoint = 0;

		return entry;
	}

	static std::vector<GridEntry> GetZoneGridEntries(int zone_id)
	{
		std::vector<GridEntry> grid_entries;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE zoneid = {} ORDER BY gridid, number",
				BaseSelect(),
				zone_id
			)
		);

		for (auto row = results.begin(); row != results.end(); ++row) {
			GridEntry entry{};

			entry.gridid      = atoi(row[0]);
			entry.zoneid      = atoi(row[1]);
			entry.number      = atoi(row[2]);
			entry.x           = static_cast<float>(atof(row[3]));
			entry.y           = static_cast<float>(atof(row[4]));
			entry.z           = static_cast<float>(atof(row[5]));
			entry.heading     = static_cast<float>(atof(row[6]));
			entry.pause       = atoi(row[7]);
			entry.centerpoint = atoi(row[8]);

			grid_entries.push_back(entry);
		}

		return grid_entries;
	}
};

#endif
