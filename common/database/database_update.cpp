#include <filesystem>
#include "database_update.h"
#include "../eqemu_logsys.h"
#include "../database.h"
#include "../strings.h"
#include "../rulesys.h"
#include "../http/httplib.h"

#include "database_update_manifest.cpp"
#include "database_update_manifest_bots.cpp"
#include "database_dump_service.h"

constexpr int BREAK_LENGTH = 70;

DatabaseVersion DatabaseUpdate::GetDatabaseVersions()
{
	auto results = m_database->QueryDatabase("SELECT `version`, `bots_version` FROM `db_version` LIMIT 1");
	if (!results.Success() || !results.RowCount()) {
		LogError("Failed to read from [db_version] table!");
		return DatabaseVersion{};
	}

	auto r = results.begin();

	return DatabaseVersion{
		.server_database_version = Strings::ToInt(r[0]),
		.bots_database_version = Strings::ToInt(r[1]),
	};
}

DatabaseVersion DatabaseUpdate::GetBinaryDatabaseVersions()
{
	return DatabaseVersion{
		.server_database_version = CURRENT_BINARY_DATABASE_VERSION,
		.bots_database_version = (RuleB(Bots, Enabled) ? CURRENT_BINARY_BOTS_DATABASE_VERSION : 0),
	};
}

// the amount of versions we look-back to ensure we have all migrations
// we may not want to force these, but just warn about the look-backs
constexpr int LOOK_BACK_AMOUNT = 10;

// this check will take action
void DatabaseUpdate::CheckDbUpdates()
{
	InjectBotsVersionColumn();
	auto v = GetDatabaseVersions();
	auto b = GetBinaryDatabaseVersions();
	if (CheckVersionsUpToDate(v, b)) {
		return;
	}

	if (UpdateManifest(manifest_entries, v.server_database_version, b.server_database_version)) {
		LogInfo(
			"Updates ran successfully, setting database version to [{}] from [{}]",
			b.server_database_version,
			v.server_database_version
		);
		m_database->QueryDatabase(fmt::format("UPDATE `db_version` SET `version` = {}", b.server_database_version));
	}

	if (b.bots_database_version > 0) {
		if (UpdateManifest(bot_manifest_entries, v.bots_database_version, b.bots_database_version)) {
			LogInfo(
				"Updates ran successfully, setting database version to [{}] from [{}]",
				b.bots_database_version,
				v.bots_database_version
			);
			m_database->QueryDatabase(
				fmt::format(
					"UPDATE `db_version` SET `bots_version` = {}",
					b.bots_database_version
				)
			);
		}
	}
}

std::string DatabaseUpdate::GetQueryResult(std::string query)
{
	auto results = m_database->QueryDatabase(query);

	std::vector<std::string> result_lines = {};

	for (auto row = results.begin(); row != results.end(); ++row) {
		std::vector<std::string> cols;

		int field_count = results.ColumnCount();
		cols.reserve(field_count);
		for (int i = 0; i < field_count; ++i) {
			if (row[i] != nullptr) {
				cols.emplace_back(row[i]);
			}
		}

		result_lines.emplace_back(Strings::Join(cols, " "));
	}

	return Strings::Join(result_lines, "\n");
}

bool DatabaseUpdate::ShouldRunMigration(ManifestEntry &e, std::string query_result)
{
	std::string r = Strings::Trim(query_result);
	if (e.condition == "contains") {
		return Strings::Contains(r, e.match);
	}
	else if (e.condition == "match") {
		return r == e.match;
	}
	else if (e.condition == "missing") {
		return !Strings::Contains(r, e.match);
	}
	else if (e.condition == "empty") {
		return r.empty();
	}
	else if (e.condition == "not_empty") {
		return !r.empty();
	}

	return false;
}

// return true if we ran updates
bool DatabaseUpdate::UpdateManifest(
	std::vector<ManifestEntry> entries,
	int version_low,
	int version_high
)
{
	std::vector<int> missing_migrations = {};
	if (version_low != version_high) {

		LogSys.DisableMySQLErrorLogs();
		for (int version = version_low + 1; version <= version_high; ++version) {
			for (auto &e: entries) {
				if (e.version == version) {
					bool        has_migration = true;
					std::string r             = GetQueryResult(e.check);
					if (ShouldRunMigration(e, r)) {
						has_migration = false;
						missing_migrations.emplace_back(e.version);
					}

					std::string prefix = fmt::format(
						"[{}]",
						has_migration ? "ok" : "missing"
					);

					LogInfo(
						"[{}] {:>10} | [{}]",
						e.version,
						prefix,
						e.description
					);
				}
			}
		}
		LogSys.EnableMySQLErrorLogs();
		LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

		if (!missing_migrations.empty()) {
			LogInfo("Automatically backing up database before applying updates");
			LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));
			auto s = DatabaseDumpService();
			s.SetDumpAllTables(true);
			s.SetDumpWithCompression(true);
			s.DatabaseDump();
			LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));
		}

		if (!missing_migrations.empty()) {
			LogInfo("Running database migrations. Please wait...");
			LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));
		}

		for (auto &m: missing_migrations) {
			for (auto &e: entries) {
				if (e.version == m) {
					bool errored_migration = false;

					auto r = m_database->QueryDatabaseMulti(e.sql);

					// ignore empty query result "errors"
					if (r.ErrorNumber() != 1065 && !r.ErrorMessage().empty()) {
						LogError("(#{}) [{}]", r.ErrorNumber(), r.ErrorMessage());
						errored_migration = true;

						LogInfo("Required database update failed. This could be a problem");
						LogInfo("Would you like to skip this update? [y/n] (Timeout 60s)");

						// user input
						std::string input;
						bool        gave_input        = false;
						time_t      start_time        = time(nullptr);
						time_t      wait_time_seconds = 60;

						// spawn a concurrent thread that waits for input from std::cin
						std::thread t1(
							[&]() {
								std::cin >> input;
								gave_input = true;
							}
						);
						t1.detach();

						// check the inputReceived flag once every 50ms for 10 seconds
						while (time(nullptr) < start_time + wait_time_seconds && !gave_input) {
							std::this_thread::sleep_for(std::chrono::milliseconds(50));
						}

						// prompt for user skip
						if (Strings::Trim(input) == "y") {
							errored_migration = false;
							LogInfo("Skipping update [{}] [{}]", e.version, e.description);
						}
					}

					LogInfo(
						"[{}] [{}] [{}]",
						e.version,
						e.description,
						(errored_migration ? "error" : "ok")
					);

					if (errored_migration) {
						LogError("Fatal | Database migration [{}] failed to run", e.description);
						LogError("Fatal | Shutting down");
						std::exit(1);
					}
				}
			}
		}

		LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

		return true;
	}

	return false;
}

DatabaseUpdate *DatabaseUpdate::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}

bool DatabaseUpdate::CheckVersionsUpToDate(DatabaseVersion v, DatabaseVersion b)
{
	LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

	LogInfo(
		"{:>8} | database [{}] binary [{}] {}",
		"Server",
		v.server_database_version,
		b.server_database_version,
		(v.server_database_version == b.server_database_version) ? "up to date" : "checking updates"
	);

	if (RuleB(Bots, Enabled) && b.bots_database_version > 0) {
		LogInfo(
			"{:>8} | database [{}] binary [{}] {}",
			"Bots",
			v.bots_database_version,
			b.bots_database_version,
			(v.bots_database_version == b.bots_database_version) ? "up to date" : "checking updates"
		);
	}

	LogInfo("{:>8} | [server.auto_database_updates] [<green>true]", "Config");

	LogInfo("{}", Strings::Repeat("-", BREAK_LENGTH));

	// server database version is required
	bool server_up_to_date = v.server_database_version >= b.server_database_version;
	// bots database version is optional, if not enabled then it is always up-to-date
	bool bots_up_to_date   = RuleB(Bots, Enabled) ? v.bots_database_version >= b.bots_database_version : true;

	return server_up_to_date && bots_up_to_date;
}

// checks to see if there are pending updates
// used by zone to prevent launch or boot loop until updates are applied
bool DatabaseUpdate::HasPendingUpdates()
{
	auto v = GetDatabaseVersions();
	auto b = GetBinaryDatabaseVersions();

	return !CheckVersionsUpToDate(v, b);
}

void DatabaseUpdate::InjectBotsVersionColumn()
{
	auto r = m_database->QueryDatabase("show columns from db_version where Field like '%bots_version%'");
	if (r.RowCount() == 0) {
		m_database->QueryDatabase("ALTER TABLE db_version ADD bots_version int(11) DEFAULT '0' AFTER version");
	}
}
