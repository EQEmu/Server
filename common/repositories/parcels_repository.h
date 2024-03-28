#ifndef EQEMU_PARCELS_REPOSITORY_H
#define EQEMU_PARCELS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_parcels_repository.h"

class ParcelsRepository: public BaseParcelsRepository {
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
     * ParcelsRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * ParcelsRepository::GetWhereNeverExpires()
     * ParcelsRepository::GetWhereXAndY()
     * ParcelsRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	struct ParcelCountAndCharacterName
	{
		std::string character_name;
		uint32      parcel_count;
	};

	static std::vector<ParcelCountAndCharacterName> GetParcelCountAndCharacterName(Database &db, const std::string &character_name)
	{
		std::vector<ParcelCountAndCharacterName> all_entries;

		auto results = db.QueryDatabase(
			fmt::format(
			"SELECT c.name, COUNT(p.id) FROM character_data c "
			"JOIN parcels p ON p.to_name = c.name "
			"WHERE c.name = '{}' "
			"LIMIT 1",
			character_name)
		);

		all_entries.reserve(results.RowCount());

		for(auto row = results.begin(); row != results.end(); ++row) {
			ParcelCountAndCharacterName e {};

			e.character_name = row[0] ? row[0] : "";
			e.parcel_count = row[1] ? Strings::ToUnsignedInt(row[1]) : 0;

			all_entries.push_back(e);
		}

		return all_entries;
	}
};

#endif //EQEMU_PARCELS_REPOSITORY_H
