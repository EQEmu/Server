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

			// Split the line by '^' and push to the columns vector
			while (std::getline(ss, item, '^')) {
				columns.push_back(item);
			}

			// Fix Pet 'Targets'
			if (columns[98] == "14" || columns[98] == "38") {
				columns[98] = "6";
			}

			//

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

	static std::vector<std::string> GetSpellFileLinesMulticlass(Database& db)
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

			// Split the line by '^' and push to the columns vector
			while (std::getline(ss, item, '^')) {
				columns.push_back(item);
			}

			// Fix Pet 'Targets'
			if (columns[98] == "14" || columns[98] == "38") {
				columns[98] = "6";
			}

			// Adjust disc timers (Why are discs -1 here)
			if (columns[168] == "-1") {
				// Check how many of the class columns (104-119) are <= 70
				int valid_class_count = 0;
				int valid_class_id = -1;

				for (int i = 104; i <= 119; ++i) {
					if (std::stoi(columns[i]) <= 70) {
						valid_class_count++;
						if (valid_class_count == 1) {
							valid_class_id = i - 104; // Adjust the index to reflect the class ID
						}
					}
				}

				// If exactly one valid class, update the timer_id with offset (valid_class_id + 1)
				if (valid_class_count == 1 && valid_class_id != -1) {
					//LogDebug("Found a discipline name [{}], updating [{}] to [{}]", columns[1], columns[167], std::to_string(Strings::ToInt(columns[167]) + (20 * (valid_class_id + 1))));
					columns[167] = std::to_string(Strings::ToInt(columns[167]) + (20 * (valid_class_id + 1)));
				}
			} else {
				// Not a disc
				if (Strings::ToInt(columns[20]) < 0 && columns[86] == "0" && Strings::ToInt(columns[17]) == 0) { // it is a nuke
					if (Strings::ToInt(columns[13]) > 3000) { // cast time > 3 sec
						columns[15] = fmt::to_string(Strings::ToInt(columns[15]) + (Strings::ToInt(columns[13]) - 3000)).c_str();
						columns[13] = "3000";
					}
				}
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
