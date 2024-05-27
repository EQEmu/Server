/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../../common/eqemu_logsys.h"
#include "../../common/global_define.h"
#include "../../common/shareddb.h"
#include "../../common/eqemu_config.h"
#include "../../common/platform.h"
#include "../../common/crash.h"
#include "../../common/content/world_content_service.h"
#include "../../common/zone_store.h"
#include "../../common/events/player_event_logs.h"

EQEmuLogSys         LogSys;
WorldContentService content_service;
ZoneStore           zone_store;
PathManager         path;
PlayerEventLogs     player_event_logs;

int main(int argc, char **argv) {
	RegisterExecutablePlatform(ExePlatformClientImport);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();

	path.LoadPaths();

	LogInfo("Client Files Import Utility");
	if(!EQEmuConfig::LoadConfig()) {
		LogError("Unable to load configuration file.");
		return 1;
	}

	auto Config = EQEmuConfig::get();

	SharedDatabase database;
	SharedDatabase content_db;

	LogInfo("Connecting to database");
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort
	)) {
		LogError("Unable to connect to the database, cannot continue without a database connection");
		return 1;
	}

	/**
	 * Multi-tenancy: Content database
	 */
	if (!Config->ContentDbHost.empty()) {
		if (!content_db.Connect(
			Config->ContentDbHost.c_str() ,
			Config->ContentDbUsername.c_str(),
			Config->ContentDbPassword.c_str(),
			Config->ContentDbName.c_str(),
			Config->ContentDbPort
		)) {
			LogError("Cannot continue without a content database connection");
			return 1;
		}
	} else {
		content_db.SetMySQL(database);
	}

	LogSys.SetDatabase(&database)
		->SetLogPath(path.GetLogPath())
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	std::vector<std::string> file_names = {
		"BaseData.txt",
		"dbstr_us.txt",
		"SkillCaps.txt",
		"spells_us.txt"
	};

	std::ifstream file;

	for (const auto& file_name : file_names) {
		const std::string& full_file_name = fmt::format(
			"{}/import/{}",
			path.GetServerPath(),
			file_name
		);

		file.open(full_file_name);
		std::string line;
		if (!file || !file.is_open()) {
			LogError("Unable to open {} to read, skipping.", full_file_name);
			continue;
		}

		std::string table_name;
		std::string type;

		if (Strings::EqualFold(file_name, "BaseData.txt")) {
			table_name = "base_data";
			type       = "base data value";
		} else if (Strings::EqualFold(file_name, "dbstr_us.txt")) {
			table_name = "db_str";
			type       = "database string";
		} else if (Strings::EqualFold(file_name, "SkillCaps.txt")) {
			table_name = "skill_caps";
			type       = "skill cap";
		} else if (Strings::EqualFold(file_name, "spells_us.txt")) {
			table_name = "spells_new";
			type       = "spell";
		}

		// skill_caps has an AUTO_INCREMENT primary key that is not part of its exported file, unlike other tables
		// due to this we need to attach an id of '0' to utilize auto increment
		const std::string& query_prefix = Strings::EqualFold(table_name, "skill_caps") ? "0," : "";

		std::vector<std::string> insert_chunks;
		while (file) {
			std::getline(file, line);

			const std::string& insert_chunk = fmt::format(
				"({}{})",
				query_prefix,
				Strings::Replace(Strings::Escape(line), "^", ", ")
			);

			insert_chunks.emplace_back(insert_chunk);
		}

		if (insert_chunks.empty()) {
			LogInfo("There are no {}s to insert, skipping.", type);
			continue;
		}

		SharedDatabase& db = (!Strings::EqualFold(table_name, "db_str") ? content_db : database);

		// Empty table before attempting to insert rows
		db.QueryDatabase(
			fmt::format(
				"TRUNCATE `{}`",
				table_name
			)
		);

		const std::string& query = fmt::format(
			"INSERT INTO `{}` VALUES {}",
			table_name,
			Strings::Implode(", ", insert_chunks)
		);

		// db_str using content_db
		// base_data, skill_caps, and spells_new use database
		db.QueryDatabase(query);

		LogInfo(
			"[{}] {}{} inserted from {}",
			insert_chunks.size(),
			type,
			insert_chunks.size() != 1 ? "s" : "",
			file_name
		);
	}

	LogSys.CloseFileLogs();

	return 0;
}
