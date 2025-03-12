#ifndef EQEMU_ZONE_STATE_SPAWNS_REPOSITORY_H
#define EQEMU_ZONE_STATE_SPAWNS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_zone_state_spawns_repository.h"

class ZoneStateSpawnsRepository: public BaseZoneStateSpawnsRepository {
public:
	// Custom extended repository methods here
	static void PurgeInvalidZoneStates(Database &database)
	{
		std::string query = R"(
SELECT zone_id, instance_id
FROM zone_state_spawns
GROUP BY zone_id, instance_id
HAVING COUNT(*) = SUM(
    CASE
        WHEN hp = 0
         AND mana = 0
         AND endurance = 0
         AND (loot_data IS NULL OR loot_data = '')
         AND (entity_variables IS NULL OR entity_variables = '')
         AND (buffs IS NULL OR buffs = '')
        THEN 1 ELSE 0
    END
);
)";

		auto results = database.QueryDatabase(query);
		if (!results.Success()) {
			return;
		}

		for (auto row: results) {
			uint32 zone_id     = std::stoul(row[0]);
			uint32 instance_id = std::stoul(row[1]);

			ZoneStateSpawnsRepository::DeleteWhere(
				database,
				fmt::format(
					"`zone_id` = {} AND `instance_id` = {}",
					zone_id,
					instance_id
				)
			);

			LogInfo("Purged invalid zone state data for zone [{}] instance [{}]", zone_id, instance_id);
		}
	}

};

#endif //EQEMU_ZONE_STATE_SPAWNS_REPOSITORY_H
