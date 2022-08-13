#ifndef EQEMU_GRID_REPOSITORY_H
#define EQEMU_GRID_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_grid_repository.h"

class GridRepository : public BaseGridRepository {
public:

	/**
	 * This file was auto generated on Apr 5, 2020 and can be modified and extended upon
	 *
	 * Base repository methods are automatically
	 * generated in the "base" version of this repository. The base repository
	 * is immutable and to be left untouched, while methods in this class
	 * are used as extension methods for more specific persistence-layer
	 * accessors or mutators
	 *
	 * Base Methods (Subject to be expanded upon in time)
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
	 * GridRepository::GetByZoneAndVersion(int zone_id, int zone_version)
	 * GridRepository::GetWhereNeverExpires()
	 * GridRepository::GetWhereXAndY()
	 * GridRepository::DeleteWhereXAndY()
	 *
	 * Most of the above could be covered by base methods, but if you as a developer
	 * find yourself re-using logic for other parts of the code, its best to just make a
	 * method that can be re-used easily elsewhere especially if it can use a base repository
	 * method and encapsulate filters there
	 */

	// Custom extended repository methods here

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
		const std::vector<Grid> &grids,
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

#endif //EQEMU_GRID_REPOSITORY_H
