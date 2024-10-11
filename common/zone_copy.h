#ifndef EQEMU_ZONECOPY_H
#define EQEMU_ZONECOPY_H

#include "database.h"
#include "repositories/zone_repository.h"
#include "repositories/spawn2_repository.h"
#include "repositories/spawngroup_repository.h"
#include "repositories/spawnentry_repository.h"
#include "repositories/zone_points_repository.h"
#include "repositories/npc_types_repository.h"
#include "repositories/doors_repository.h"
#include "repositories/ground_spawns_repository.h"
#include "repositories/object_repository.h"

class ZoneCopier {
public:
	void ZoneCopy();

	// setters and getters
	ZoneCopier *SetDatabase(Database *database);
	Database *GetDatabase() const;
	ZoneCopier *SetContentDatabase(Database *database);
	Database *GetContentDatabase() const;
	const std::string &GetShortName() const;
	void SetSourceShortName(const std::string &short_name);
	int GetSourceVersion() const;
	void SetSourceVersion(int source_version);
	int GetDestinationVersion() const;
	void SetDestinationVersion(int destination_version);
private:
	void LoadZoneToCopy();

	// reference to database
	Database *m_database;
	Database *m_content_database;

	// inputs
	std::string m_source_short_name;
	int m_source_version;
	int m_destination_version;

	// hold the data
	ZoneRepository::Zone m_zone;
	int m_zone_id;
	std::vector<Spawn2Repository::Spawn2> m_spawn2s;
	std::vector<SpawngroupRepository::Spawngroup> m_spawngroups;
	std::vector<SpawnentryRepository::Spawnentry> m_spawnentries;
	std::vector<ZonePointsRepository::ZonePoints> m_zone_points;
	std::vector<NpcTypesRepository::NpcTypes> m_npc_types;
	std::vector<DoorsRepository::Doors> m_doors;
	std::vector<GroundSpawnsRepository::GroundSpawns> m_ground_spawns;
	std::vector<ObjectRepository::Object> m_objects;
};


#endif //EQEMU_ZONECOPY_H
