#ifndef EQEMU_DB_STR_REPOSITORY_H
#define EQEMU_DB_STR_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_db_str_repository.h"

class DbStrRepository: public BaseDbStrRepository {
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
     * DbStrRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * DbStrRepository::GetWhereNeverExpires()
     * DbStrRepository::GetWhereXAndY()
     * DbStrRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static std::vector<std::string> GetDBStrFileLines(Database& db)
	{
		std::vector<std::string> lines;

		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT CONCAT(CONCAT_WS('^', {}), '^0') FROM {} ORDER BY `id`, `type` ASC",
				ColumnsRaw(),
				TableName()
			)
		);



		for (auto row : results) {
			lines.emplace_back(row[0]);
		}

		return lines;
	}

	static std::vector<std::string> GetDBStrFileLinesMulticlass(Database& db, Database& content_db)
	{
		std::vector<std::string> lines;

		// Query to get the data from the primary db
		auto results = db.QueryDatabase(
			fmt::format(
				"SELECT CONCAT(CONCAT_WS('^', {}), '^0') FROM {} ORDER BY `id`, `type` ASC",
				ColumnsRaw(),
				TableName()
			)
		);

		// Class bitmask to class name map
		const std::vector<std::pair<int, std::string>> class_map = {
			{1, "WAR"}, {2, "CLR"}, {4, "PAL"}, {8, "RNG"}, {16, "SHD"},
			{32, "DRU"}, {64, "MNK"}, {128, "BRD"}, {256, "ROG"}, {512, "SHM"},
			{1024, "NEC"}, {2048, "WIZ"}, {4096, "MAG"}, {8192, "ENC"},
			{16384, "BST"}, {32768, "BER"}
		};

		// Process each row from the query result
		for (auto row : results) {
			std::string line = row[0];
			std::stringstream ss(line);
			std::string item;
			std::vector<std::string> columns;

			// Split the line by '^' and push to the columns vector
			while (std::getline(ss, item, '^')) {
				columns.push_back(item);
			}

			// Perform modifications
			// Only apply logic if columns[1] == "1"
			if (columns[1] == "4") {
				// Query content_db to get the `aa_ability.classes` for the matching title_sid (columns[0])
				auto content_result = content_db.QueryDatabase(
					fmt::format(
						"SELECT aa_ability.classes FROM aa_ability "
						"JOIN aa_ranks ON aa_ability.first_rank_id = aa_ranks.id "
						"WHERE aa_ranks.desc_sid = {}",
						columns[0] // The title_sid from columns[0]
					)
				);

				if (content_result.RowCount() > 0) {
					// Access the first row's first column
					auto row = content_result.begin();
					int aa_classes = std::stoi(row[0]); // Convert the class bitmask to an integer

				if (aa_classes != 65535) {
					std::vector<std::string> class_tags;

					// Track 'has_X' and 'block_X' for each class
					bool has_war = false, block_war = false;
					bool has_clr = false, block_clr = false;
					bool has_pal = false, block_pal = false;
					bool has_rng = false, block_rng = false;
					bool has_shd = false, block_shd = false;
					bool has_dru = false, block_dru = false;
					bool has_mnk = false, block_mnk = false;
					bool has_brd = false, block_brd = false;
					bool has_rog = false, block_rog = false;
					bool has_shm = false, block_shm = false;
					bool has_nec = false, block_nec = false;
					bool has_wiz = false, block_wiz = false;
					bool has_mag = false, block_mag = false;
					bool has_enc = false, block_enc = false;
					bool has_bst = false, block_bst = false;
					bool has_ber = false, block_ber = false;

					// Parse the bitmask and mark 'has_X' for each class
					for (const auto& [bitmask, class_name] : class_map) {
						if (aa_classes & bitmask) {
							if (class_name == "WAR") has_war = true;
							else if (class_name == "CLR") has_clr = true;
							else if (class_name == "PAL") has_pal = true;
							else if (class_name == "RNG") has_rng = true;
							else if (class_name == "SHD") has_shd = true;
							else if (class_name == "DRU") has_dru = true;
							else if (class_name == "MNK") has_mnk = true;
							else if (class_name == "BRD") has_brd = true;
							else if (class_name == "ROG") has_rog = true;
							else if (class_name == "SHM") has_shm = true;
							else if (class_name == "NEC") has_nec = true;
							else if (class_name == "WIZ") has_wiz = true;
							else if (class_name == "MAG") has_mag = true;
							else if (class_name == "ENC") has_enc = true;
							else if (class_name == "BST") has_bst = true;
							else if (class_name == "BER") has_ber = true;
						}
					}

					// Add leftover class tags that are not blocked
					if (has_war && !block_war) class_tags.push_back("WAR");
					if (has_clr && !block_clr) class_tags.push_back("CLR");
					if (has_pal && !block_pal) class_tags.push_back("PAL");
					if (has_rng && !block_rng) class_tags.push_back("RNG");
					if (has_shd && !block_shd) class_tags.push_back("SHD");
					if (has_dru && !block_dru) class_tags.push_back("DRU");
					if (has_mnk && !block_mnk) class_tags.push_back("MNK");
					if (has_brd && !block_brd) class_tags.push_back("BRD");
					if (has_rog && !block_rog) class_tags.push_back("ROG");
					if (has_shm && !block_shm) class_tags.push_back("SHM");
					if (has_nec && !block_nec) class_tags.push_back("NEC");
					if (has_wiz && !block_wiz) class_tags.push_back("WIZ");
					if (has_mag && !block_mag) class_tags.push_back("MAG");
					if (has_enc && !block_enc) class_tags.push_back("ENC");
					if (has_bst && !block_bst) class_tags.push_back("BST");
					if (has_ber && !block_ber) class_tags.push_back("BER");

					// Join the class tags into a single string separated by spaces
					std::string class_tags_str = "(" + Strings::Join(class_tags, " ") + ")";
					columns[2] = class_tags_str + "<br>" + columns[2];  // Append class tags to the name in columns[2]
				} else {
					// If aa_classes is 65535, it means "ALL" classes
					columns[2] = "(ALL)<br>" + columns[2];
				}

					//LogDebug("Produced AA Name: [{}]", columns[2]);
				}
			}

			// Reconstruct the modified line
			std::string modified_line;
			for (size_t i = 0; i < columns.size(); ++i) {
				if (i > 0) {
					modified_line += '^'; // Re-add the delimiter
				}
				modified_line += columns[i]; // Add the column data back
			}

			lines.emplace_back(modified_line); // Add the modified line to the result
		}

		return lines; // Return the final vector of modified lines
	}


};

#endif //EQEMU_DB_STR_REPOSITORY_H
