#include <filesystem>
#include "database_update.h"
#include "../eqemu_logsys.h"
#include "../database.h"
#include "../strings.h"
#include "../rulesys.h"
#include "../http/httplib.h"

// don't remove these
#include "database_update_manifest_old.cpp"
#include "database_migrations.cpp"
#include "database_migrations_bots.cpp"
#include "database_update_manifest.cpp"
#include "database_update_manifest_bots.cpp"
#include "database_dump_service.h"

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

void DatabaseUpdate::CheckDbUpdates()
{
	auto v = GetDatabaseVersions();
	auto b = GetBinaryDatabaseVersions();

	LogInfo(
		"DB Version | database [{}] server [{}] {}",
		v.server_database_version,
		b.server_database_version,
		(v.server_database_version == b.server_database_version) ? "up to date" : "checking updates"
	);

	if (b.bots_database_version > 0) {
		LogInfo(
			"Bots DB Version | database [{}] server [{}]",
			v.bots_database_version,
			b.bots_database_version
		);
	}

//	RunConversion();

	if (CheckManifest(manifest_entries, v.server_database_version, b.server_database_version)) {
		LogInfo(
			"Updates ran successfully, setting database version to [{}] from [{}]",
			b.server_database_version,
			v.server_database_version
		);
		m_database->QueryDatabase(fmt::format("UPDATE `db_version` SET `version` = {}", b.server_database_version));
	}

	if (b.bots_database_version > 0) {
		if (CheckManifest(bot_manifest_entries, v.bots_database_version, b.bots_database_version)) {
			LogInfo(
				"Updates ran successfully, setting database version to [{}] from [{}]",
				b.bots_database_version,
				v.bots_database_version
			);
			m_database->QueryDatabase(fmt::format("UPDATE `db_version` SET `bots_version` = {}", b.bots_database_version));
		}
	}

	std::exit(0);
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

//# Example: Version|Filename.sql|Query_to_Check_Condition_For_Needed_Update|match type|text to match
//#	0 = Database Version
//#	1 = Filename.sql
//#	2 = Query_to_Check_Condition_For_Needed_Update
//#	3 = Match Type - If condition from match type to Value 4 is true, update will flag for needing to be ran
//#		contains = If query results contains text from 4th value
//#		match = If query results matches text from 4th value
//#		missing = If query result is missing text from 4th value
//#		empty = If the query results in no results
//#		not_empty = If the query is not empty
//#	4 = Text to match
//#
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
void DatabaseUpdate::RunConversion()
{
	auto lines = Strings::Split(db_manifest, "\n");

	std::vector<ManifestEntry> entries = {};
	entries.reserve(lines.size());

	int lines_threshold = 12;

	std::vector<std::string> new_entries     = {};
	std::vector<std::string> migration_files = {};
	for (auto                &l: lines) {
		if (l[0] != '#') {
			auto c = Strings::Split(l, "|");
			if (!c.empty() && Strings::ToInt(c[0]) != 0) {
//				LogInfo("Size [{}]", c.size());

				entries.emplace_back(
					ManifestEntry{
						.version = Strings::ToInt(c[0]),
						.description = c.size() >= 2 ? c[1] : "",
						.query = c.size() >= 3 ? c[2] : "",
						.condition = c.size() >= 4 ? c[3] : "",
						.match = c.size() >= 5 ? c[4] : "",
					}
				);

				int version = Strings::ToInt(c[0]);
				if (version < 9000) {
					continue;
				}

				std::filesystem::path path("/home/eqemu/code/utils/sql/git/bots/required");
				std::filesystem::path filename(c[1]);
				std::filesystem::path filepath = path / filename;
				std::ifstream         t(filepath.string());
				std::stringstream     buffer;
				if (!t.is_open()) {
					LogError("Failed to open file [{}] version [{}]", filepath.string(), version);
					std::exit(0);
				}
				buffer << t.rdbuf();

				auto sql_lines = Strings::Split(buffer.str(), ";");
//				LogInfo("[{}] Lines [{}]", filepath.string(), sql_lines.size());

				std::string statement;
				if (sql_lines.size() < lines_threshold) {
					statement = buffer.str();
				}

				// common/database/migrations/{}



				// buffer.str()

				std::string sql = fmt::format("R\"(\n{}\n)\\\\\\\\\"", statement);
				if (sql_lines.size() >= lines_threshold) {
					std::string new_file       = Strings::ToLower(Strings::Replace(filename.string(), ".sql", ".cpp"));
					std::string migration_var  =
									"_" + Strings::ToLower(Strings::Replace(filename.string(), ".sql", ""));
					std::string migration_file = fmt::format(
						"/home/eqemu/code/common/database/large-migrations-bot/{}",
						new_file
					);

					sql = migration_var;

					std::string inject_sql              = fmt::format("R\"(\n{}\n)\\\\\\\\\"", buffer.str());
					std::string migration_file_contents = fmt::format(
						"std::string {} = {};",
						migration_var,
						inject_sql
					);
					migration_file_contents = Strings::Replace(migration_file_contents, "\\\\\\\\", "");

					std::ofstream f;
					f.open(migration_file);
					f << migration_file_contents;
					f.close();

					migration_files.emplace_back(fmt::format("#include \"./large-migrations-bot/{}\"", new_file));
				}

				std::string new_entry = fmt::format(
					R"(	ManifestEntry{{
		.version = {},
		.description = "{}",
		.query = "{}",
		.condition = "{}",
		.match = "{}",
		.sql = {},
	}})",
					Strings::ToInt(c[0]),
					c.size() >= 2 ? Strings::ToLower(c[1]) : "",
					c.size() >= 3 ? c[2] : "",
					c.size() >= 4 ? c[3] : "",
					c.size() >= 5 ? c[4] : "",
					sql
				);


				new_entry = Strings::Replace(new_entry, "\\\\\\\\", "");

				new_entries.emplace_back(new_entry);
			}
		}
	}

	// write to manifest file
	std::string   manifest_write = "/home/eqemu/code/common/database/database_update_manifest_bots.cpp";
	std::ofstream f;
	f.open(manifest_write);
	f << fmt::format("std::vector<ManifestEntry> bot_manifest_entries = {{\n{}\n}};", Strings::Join(new_entries, ",\n"));
	f.close();


	// write to migration include file
	std::string   migration_include_file = "/home/eqemu/code/common/database/database_migrations_bots.cpp";
	std::ofstream mf;
	mf.open(migration_include_file);
	mf << Strings::Join(migration_files, "\n");
	mf.close();
}

// return true if we ran updates
bool DatabaseUpdate::CheckManifest(
	std::vector<ManifestEntry> entries,
	int version_low,
	int version_high
)
{
	std::vector<int> missing_migrations = {};
	if (version_low != version_high) {
		for (int version = version_low; version <= version_high; ++version) {
			for (auto &e: entries) {
				if (e.version == version) {
					bool        has_migration = true;
					std::string r             = GetQueryResult(e.query);
					if (ShouldRunMigration(e, r)) {
						has_migration = false;
						missing_migrations.emplace_back(e.version);
					}

					LogInfo(
						"[{}] [{}] [{}] query [{}] condition [{}] match [{}]",
						e.version,
						has_migration ? "ok" : "missing",
						e.description,
						e.query,
						e.condition,
						e.match
					);
				}
			}
		}

		if (!missing_migrations.empty()) {
			auto s = new DatabaseDumpService();
			s->SetDumpAllTables(true);
			s->SetDumpWithCompression(true);
			s->DatabaseDump();
			delete s;
		}

		for (auto &m: missing_migrations) {
			for (auto &e: entries) {
				if (e.version == m) {
					LogInfo(
						"Running [{}] [{}] query [{}] condition [{}] match [{}]",
						e.version,
						e.description,
						e.query,
						e.condition,
						e.match
					);

					std::string sql = e.sql;

					// don't remove these even if you think you know what you're doing
					sql = Strings::Replace(sql, "; --", ";\n --");
					sql = Strings::Replace(sql, ";   ", ";\n");
					sql = Strings::Replace(sql, ";  ", ";\n");
					sql = Strings::Replace(sql, "; ", ";\n");
					bool errored_migration = false;
					for (auto &s: Strings::Split(sql, ";\n")) {
						if (Strings::Contains(s, ";")) {
							LogError("Found [;] in statement [{}] in version [{}]", e.sql, e.version);
						}
						else {
							auto results = m_database->QueryDatabase(s);
							// ignore empty query result "errors"
							if (results.ErrorNumber() != 1065 && !results.ErrorMessage().empty()) {
								LogError("[{}]", results.ErrorMessage());
								errored_migration = true;
							}
						}
					}

					if (errored_migration) {
						LogError("[Fatal] Database migration [{}] failed to run", e.description);
						std::exit(1);
					}
				}
			}
		}

		return true;
	}

	return false;
}

DatabaseUpdate *DatabaseUpdate::SetDatabase(Database *db)
{
	m_database = db;

	return this;
}
