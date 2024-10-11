#include "zone_copy.h"


void ZoneCopier::ZoneCopy()
{
	LogSys.log_settings[Logs::MySQLQuery].is_category_enabled = 1;
	LogSys.log_settings[Logs::MySQLQuery].log_to_console      = 1;

	LoadZoneToCopy();

}

Database *ZoneCopier::GetDatabase() const
{
	return m_database;
}

ZoneCopier *ZoneCopier::SetDatabase(Database *database)
{
	ZoneCopier::m_database = database;

	return this;
}

Database *ZoneCopier::GetContentDatabase() const
{
	return m_content_database;
}

ZoneCopier *ZoneCopier::SetContentDatabase(Database *database)
{
	ZoneCopier::m_content_database = database;

	return this;
}

const std::string &ZoneCopier::GetShortName() const
{
	return m_source_short_name;
}

void ZoneCopier::SetSourceShortName(const std::string &short_name)
{
	ZoneCopier::m_source_short_name = short_name;
}

int ZoneCopier::GetSourceVersion() const
{
	return m_source_version;
}

void ZoneCopier::SetSourceVersion(int source_version)
{
	ZoneCopier::m_source_version = source_version;
}

int ZoneCopier::GetDestinationVersion() const
{
	return m_destination_version;
}

void ZoneCopier::SetDestinationVersion(int destination_version)
{
	ZoneCopier::m_destination_version = destination_version;
}

void ZoneCopier::LoadZoneToCopy()
{
	auto zones = ZoneRepository::GetWhere(
		*m_content_database,
		fmt::format("short_name = '{}' and version = 0", m_source_short_name)
	);

	if (zones.empty()) {
		LogError("Zone [{}] not found in content database", m_source_short_name);
		return;
	}

	m_zone = zones.front();
	m_zone.id      = 0;
	m_zone.version = m_destination_version;
	m_zone_id = m_zone.zoneidnumber;

	LogInfo(
		"Zone [{}] copied from version [{}] to version [{}]",
		m_source_short_name,
		m_source_version,
		m_destination_version
	);

	m_spawn2s = Spawn2Repository::GetWhere(
		*m_content_database,
		fmt::format("zone = '{}' and version = {}", m_source_short_name, m_source_version)
	);

	if (m_spawn2s.empty()) {
		LogError("No spawn2 entries found for zone [{}] version [{}]", m_source_short_name, m_source_version);
		return;
	}

	std::vector<std::string> spawn_group_ids;

	for (auto &spawn: m_spawn2s) {
		auto id = fmt::format("{}", spawn.spawngroupID);
		if (std::find(spawn_group_ids.begin(), spawn_group_ids.end(), id) == spawn_group_ids.end()) {
			spawn_group_ids.push_back(id);
		}
	}

	m_spawngroups = SpawngroupRepository::GetWhere(
		*m_content_database,
		fmt::format("id IN ({})", Strings::Join(spawn_group_ids, ","))
	);

	m_spawnentries = SpawnentryRepository::GetWhere(
		*m_content_database,
		fmt::format("spawngroupID IN ({})", Strings::Join(spawn_group_ids, ","))
	);

	m_zone_points = ZonePointsRepository::GetWhere(
		*m_content_database,
		fmt::format("zone = '{}'", m_source_short_name)
	);

	std::vector<std::string> npc_type_ids;
	for (auto                &e: m_spawnentries) {
		auto id = fmt::format("{}", e.npcID);
		if (std::find(npc_type_ids.begin(), npc_type_ids.end(), id) == npc_type_ids.end()) {
			npc_type_ids.push_back(id);
		}
	}

	m_npc_types = NpcTypesRepository::GetWhere(
		*m_content_database,
		fmt::format("id IN ({})", Strings::Join(npc_type_ids, ","))
	);

	m_doors = DoorsRepository::GetWhere(
		*m_content_database,
		fmt::format("LOWER(zone) = '{}' and version = {}", m_source_short_name, m_source_version)
	);

	m_ground_spawns = GroundSpawnsRepository::GetWhere(
		*m_content_database,
		fmt::format("zoneid = {} and version = {}", m_zone_id, m_source_version)
	);

	m_objects = ObjectRepository::GetWhere(
		*m_content_database,
		fmt::format("zoneid = {} and version = {}", m_zone_id, m_source_version)
	);

	LogInfo(
		"Found zone short_name [{}] long_name [{}] id [{}] version [{}]",
		m_zone.short_name,
		m_zone.long_name,
		m_zone.zoneidnumber,
		m_zone.version
	);

	LogInfo("Found [{}] doors", m_doors.size());
	LogInfo("Found [{}] ground_spawns", m_ground_spawns.size());
	LogInfo("Found [{}] npc_types", m_npc_types.size());
	LogInfo("Found [{}] objects", m_objects.size());
	LogInfo("Found [{}] spawn entries", m_spawnentries.size());
	LogInfo("Found [{}] spawn groups", m_spawngroups.size());
	LogInfo("Found [{}] spawn2 entries", m_spawn2s.size());
	LogInfo("Found [{}] zone_points", m_zone_points.size());
}
