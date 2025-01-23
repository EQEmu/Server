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

#include <stdio.h>

#include "../../common/eqemu_logsys.h"
#include "../../common/global_define.h"
#include "../../common/shareddb.h"
#include "../../common/eqemu_config.h"
#include "../../common/platform.h"
#include "../../common/crash.h"
#include "../../common/rulesys.h"
#include "../../common/strings.h"
#include "../../common/content/world_content_service.h"
#include "../../common/zone_store.h"
#include "../../common/path_manager.h"
#include "../../common/repositories/base_data_repository.h"
#include "../../common/repositories/db_str_repository.h"
#include "../../common/repositories/skill_caps_repository.h"
#include "../../common/repositories/spells_new_repository.h"
#include "../../common/file.h"
#include "../../common/events/player_event_logs.h"
#include "../../common/skill_caps.h"
#include "../../common/evolving_items.h"

EQEmuLogSys          LogSys;
WorldContentService  content_service;
ZoneStore            zone_store;
PathManager          path;
PlayerEventLogs      player_event_logs;
EvolvingItemsManager evolving_items_manager;

void ExportSpells(SharedDatabase *db);
void ExportSkillCaps(SharedDatabase *db);
void ExportBaseData(SharedDatabase *db);
void ExportDBStrings(SharedDatabase *db, SharedDatabase *content_db);

int main(int argc, char **argv)
{
	RegisterExecutablePlatform(ExePlatformClientExport);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();

	path.LoadPaths();

	LogInfo("Client Files Export Utility");
	if (!EQEmuConfig::LoadConfig()) {
		LogError("Unable to load configuration file");
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

	RuleManager::Instance()->LoadRules(&database, "default", false);

	LogSys.SetDatabase(&database)
		->SetLogPath(path.GetLogPath())
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	std::string export_type;

	if (argv[1]) {
		export_type = argv[1];
	}

	if (Strings::EqualFold(export_type, "spells")) {
		ExportSpells(&content_db);
		return 0;
	} else if (Strings::EqualFold(export_type, "skills")) {
		ExportSkillCaps(&content_db);
		return 0;
	} else if (Strings::EqualFold(export_type, "basedata") || Strings::EqualFold(export_type, "base_data")) {
		ExportBaseData(&content_db);
		return 0;
	} else if (Strings::EqualFold(export_type, "dbstr") || Strings::EqualFold(export_type, "dbstring")) {
		ExportDBStrings(&database, &content_db);
		return 0;
	}

	ExportSpells(&content_db);
	ExportSkillCaps(&content_db);
	ExportBaseData(&content_db);
	ExportDBStrings(&database, &content_db);

	LogSys.CloseFileLogs();

	return 0;
}

void ExportSpells(SharedDatabase* db)
{
	std::ofstream file(fmt::format("{}/export/spells_us.txt", path.GetServerPath()));
	if (!file || !file.is_open()) {
		LogError("Unable to open export/spells_us.txt to write, skipping.");
		return;
	}

	std::vector<std::string> lines;
	if (!RuleB(Custom, MulticlassingEnabled)) {
		lines = SpellsNewRepository::GetSpellFileLines(*db);
	} else {
		lines = SpellsNewRepository::GetSpellFileLinesMulticlass(*db);
	}

	const std::string& file_string = Strings::Implode("\n", lines);

	file << file_string;

	file.close();

	LogInfo("Exported [{}] Spell{}", lines.size(), lines.size() != 1 ? "s" : "");
}

void ExportSkillCaps(SharedDatabase* db)
{
	std::ofstream file(fmt::format("{}/export/SkillCaps.txt", path.GetServerPath()));
	if (!file || !file.is_open()) {
		LogError("Unable to open export/SkillCaps.txt to write, skipping.");
		return;
	}

	std::vector<std::string> lines;
	if (!RuleB(Custom, MulticlassingEnabled)) {
		lines = SkillCapsRepository::GetSkillCapFileLines(*db);
	} else {
		LogInfo("Multiclassing Enabled. Exporting Multiclass Skillcaps.");
		lines = SkillCapsRepository::GetSkillCapFileLinesMulticlass(*db);
	}

	const std::string& file_string = Strings::Implode("\n", lines);

	file << file_string;

	file.close();

	LogInfo("Exported [{}] Skill Cap{}", lines.size(), lines.size() != 1 ? "s" : "");
}

void ExportBaseData(SharedDatabase *db)
{
	std::ofstream file(fmt::format("{}/export/BaseData.txt", path.GetServerPath()));
	if (!file || !file.is_open()) {
		LogError("Unable to open export/BaseData.txt to write, skipping.");
		return;
	}

	const auto& lines = BaseDataRepository::GetBaseDataFileLines(*db);


	const std::string& file_string = Strings::Implode("\n", lines);

	file << file_string;

	file.close();

	LogInfo("Exported [{}] Base Data Entr{}", lines.size(), lines.size() != 1 ? "ies" : "y");
}

void ExportDBStrings(SharedDatabase *db, SharedDatabase *content_db)
{
	std::ofstream file(fmt::format("{}/export/dbstr_us.txt", path.GetServerPath()));
	if (!file || !file.is_open()) {
		LogError("Unable to open export/dbstr_us.txt to write, skipping.");
		return;
	}

	std::vector<std::string> lines;
	if (!RuleB(Custom, MulticlassingEnabled)) {
		lines = DbStrRepository::GetDBStrFileLines(*db);
	} else {
		lines = DbStrRepository::GetDBStrFileLinesMulticlass(*db, *content_db);
	}


	const std::string& file_string = Strings::Implode("\n", lines);

	file << file_string;

	file.close();

	LogInfo("Exported [{}] Database String{}", lines.size(), lines.size() != 1 ? "s" : "");
}
