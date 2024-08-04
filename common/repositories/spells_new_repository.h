#ifndef EQEMU_SPELLS_NEW_REPOSITORY_H
#define EQEMU_SPELLS_NEW_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_spells_new_repository.h"

class SpellsNewRepository: public BaseSpellsNewRepository {
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
     * SpellsNewRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * SpellsNewRepository::GetWhereNeverExpires()
     * SpellsNewRepository::GetWhereXAndY()
     * SpellsNewRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static std::vector<std::string> GetSpellFileLines(Database& db)
	{
		std::vector<std::string> lines;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT CONCAT_WS('^', {}) FROM {} ORDER BY {} ASC",
				ColumnsRaw(),
				TableName(),
				PrimaryKey()
			)
		);

		for (auto row : results) {
			std::string line = row[0];
			std::stringstream ss(line);
			std::string item;
			std::vector<std::string> columns;

			while (std::getline(ss, item, '^')) {
				columns.push_back(item);
			}


			if (columns[98] == "14" || columns[98] == "38") {
				columns[98] = "6";
			}

			// Reconstruct the line
			std::string modified_line;
			for (size_t i = 0; i < columns.size(); ++i) {
				if (i > 0) {
					modified_line += '^';
				}
				modified_line += columns[i];
			}

			lines.emplace_back(modified_line);
		}

		return lines;
	}
};

#endif //EQEMU_SPELLS_NEW_REPOSITORY_H
