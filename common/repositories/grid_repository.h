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

#ifndef EQEMU_GRID_REPOSITORY_H
#define EQEMU_GRID_REPOSITORY_H

#include "../database.h"
#include "../string_util.h"

class GridRepository {
public:
	struct Grid {
		int id;
		int zoneid;
		int type;
		int type2;
	};

	static std::vector<std::string> Columns()
	{
		return {
			"id",
			"zoneid",
			"type",
			"type2",
		};
	}

	static std::string ColumnsRaw()
	{
		return std::string(implode(", ", Columns()));
	}

	static std::string TableName()
	{
		return std::string("grid");
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

	static Grid NewEntity()
	{
		Grid entry{};

		entry.id     = 0;
		entry.zoneid = 0;
		entry.type   = 0;
		entry.type2  = 0;

		return entry;
	}

	static std::vector<Grid> GetZoneGrids(int zone_id)
	{
		std::vector<Grid> grids;

		auto results = content_db.QueryDatabase(
			fmt::format(
				"{} WHERE zoneid = {}",
				BaseSelect(),
				zone_id
			)
		);

		for (auto row = results.begin(); row != results.end(); ++row) {
			Grid entry{};

			entry.id     = atoi(row[0]);
			entry.zoneid = atoi(row[1]);
			entry.type   = atoi(row[2]);
			entry.type2  = atoi(row[3]);

			grids.push_back(entry);
		}

		return grids;
	}

	static Grid GetGrid(
		const std::vector<Grid>& grids,
		int grid_id
	)
	{
		for (auto &row : grids) {
			if (row.id == grid_id) {
				return row;
			}
		}

		return NewEntity();
	}

};

#endif
