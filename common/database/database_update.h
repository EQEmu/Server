#ifndef EQEMU_DATABASE_UPDATE_H
#define EQEMU_DATABASE_UPDATE_H

#include "../database.h"

struct ManifestEntry {
	int         version{};     // database version of the migration
	std::string description{}; // description of the migration ex: "add_new_table" or "add_index_to_table"
	std::string check{};       // query that checks against the condition
	std::string condition{};   // condition or "match_type" - Possible values [contains|match|missing|empty|not_empty]
	std::string match{};       // match field that is not always used, but works in conjunction with "condition" values [missing|match|contains]
	std::string sql{};         // the SQL DDL that gets ran when the condition is true
	bool        content_schema_update{}; // if true, this migration is a content schema update and should be ran against the content database
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
	std::string GetQueryResult(const ManifestEntry& e);
	static bool ShouldRunMigration(ManifestEntry &e, std::string query_result);
	bool UpdateManifest(std::vector<ManifestEntry> entries, int version_low, int version_high);

	DatabaseUpdate *SetDatabase(Database *db);
	DatabaseUpdate *SetContentDatabase(Database *db);
	bool HasPendingUpdates();
private:
	Database *m_database;
	Database *m_content_database;
	static bool CheckVersionsUpToDate(DatabaseVersion v, DatabaseVersion b);
	void InjectBotsVersionColumn();
};

#endif //EQEMU_DATABASE_UPDATE_H
