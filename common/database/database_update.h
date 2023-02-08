#ifndef EQEMU_DATABASE_UPDATE_H
#define EQEMU_DATABASE_UPDATE_H

#include "../database.h"

struct ManifestEntry {
	int         version{};
	std::string description{};
	std::string query{};
	std::string condition{};
	std::string match{};
	std::string sql{};
};

struct DatabaseVersion {
	int server_database_version;
	int bots_database_version;
};

class DatabaseUpdate {
public:
	DatabaseVersion GetDatabaseVersions();
	DatabaseVersion GetBinaryDatabaseVersions();
	void CheckDbUpdates();
	std::string GetQueryResult(std::string query);
	static bool ShouldRunMigration(ManifestEntry &e, std::string query_result);
	void RunConversion();
	bool CheckManifest(std::vector<ManifestEntry> entries, int version_low, int version_high);

	DatabaseUpdate *SetDatabase(Database *db);
private:
	Database *m_database;
};

#endif //EQEMU_DATABASE_UPDATE_H
