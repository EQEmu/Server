#ifndef EQEMU_GROUP_LEADERS_REPOSITORY_H
#define EQEMU_GROUP_LEADERS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_group_leaders_repository.h"

class GroupLeadersRepository: public BaseGroupLeadersRepository {
public:
	struct GroupLeadersFix
	{
		int32_t     gid;
		std::string leadername;
		std::string marknpc;
		char leadershipaa[sizeof(GroupLeadershipAA_Struct)];
		std::string maintank;
		std::string assist;
		std::string puller;
		std::string mentoree;
		int32_t     mentor_percent;
	};

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
     * GroupLeadersRepository::GetByZoneAndVersion(int zone_id, int zone_version)
     * GroupLeadersRepository::GetWhereNeverExpires()
     * GroupLeadersRepository::GetWhereXAndY()
     * GroupLeadersRepository::DeleteWhereXAndY()
     *
     * Most of the above could be covered by base methods, but if you as a developer
     * find yourself re-using logic for other parts of the code, its best to just make a
     * method that can be re-used easily elsewhere especially if it can use a base repository
     * method and encapsulate filters there
     */

	// Custom extended repository methods here
	static void ClearAllGroupLeaders(Database& db)
	{
		db.QueryDatabase(
			fmt::format(
				"DELETE FROM `{}`",
				TableName()
			)
		);
	}

	static GroupLeadersFix GetGroupLeaderFix(
		Database &db,
		int group_leaders_id
	)
	{
		auto results = db.QueryDatabase(
			fmt::format(
				"{} WHERE {} = {} LIMIT 1",
				BaseSelect(),
				PrimaryKey(),
				group_leaders_id
			)
		);

		GroupLeadersFix tmp{};

		auto            row = results.begin();
        if (results.RowCount() == 1) {
            GroupLeadersFix e{};

            e.gid        = row[0] ? static_cast<int32_t>(atoi(row[0])) : 0;
            e.leadername = row[1] ? row[1] : "";
            e.marknpc    = row[2] ? row[2] : "";
            if (results.LengthOfColumn(3) == sizeof(GroupLeadershipAA_Struct)) {
                memcpy(e.leadershipaa, row[3], sizeof(e.leadershipaa));
            }
            e.maintank       = row[4] ? row[4] : "";
            e.assist         = row[5] ? row[5] : "";
            e.puller         = row[6] ? row[6] : "";
            e.mentoree       = row[7] ? row[7] : "";
            e.mentor_percent = row[8] ? static_cast<int32_t>(atoi(row[8])) : 0;

            return e;
        }

        return tmp;
    }
};

#endif //EQEMU_GROUP_LEADERS_REPOSITORY_H
