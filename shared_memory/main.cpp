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

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include "../common/shareddb.h"
#include "../common/eqemu_config.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/rulesys.h"
#include "../common/eqemu_exception.h"
#include "../common/string_util.h"
#include "items.h"
#include "npc_faction.h"
#include "loot.h"
#include "skill_caps.h"
#include "spells.h"
#include "base_data.h"
#include "../common/content/world_content_service.h"

EQEmuLogSys LogSys;
WorldContentService content_service;

#ifdef _WINDOWS
#include <direct.h>
#else

#include <unistd.h>

#endif

#include <sys/stat.h>

inline bool MakeDirectory(const std::string &directory_name)
{
#ifdef _WINDOWS
	struct _stat st;
	if (_stat(directory_name.c_str(), &st) == 0) {
		return false;
	}
	else {
		_mkdir(directory_name.c_str());
		return true;
	}

#else
	struct stat st;
	if (stat(directory_name.c_str(), &st) == 0) {
		return false;
	}
	else {
		mkdir(directory_name.c_str(), 0755);
		return true;
	}

#endif
	return false;
}

int main(int argc, char **argv)
{
	RegisterExecutablePlatform(ExePlatformSharedMemory);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();

	LogInfo("Shared Memory Loader Program");
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
		content_db.SetMysql(database.getMySQL());
	}

	LogSys.SetDatabase(&database)
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	std::string shared_mem_directory = Config->SharedMemDir;
	if (MakeDirectory(shared_mem_directory)) {
		LogInfo("Shared Memory folder doesn't exist, so we created it [{}]", shared_mem_directory.c_str());
	}

	database.LoadVariables();

	/* If we're running shared memory and hotfix has no custom name, we probably want to start from scratch... */
	std::string db_hotfix_name;
	if (database.GetVariable("hotfix_name", db_hotfix_name)) {
		if (!db_hotfix_name.empty() && strcasecmp("hotfix_", db_hotfix_name.c_str()) == 0) {
			LogInfo("Current hotfix in variables is the default [{}], clearing out variable", db_hotfix_name.c_str());
			std::string query = StringFormat("UPDATE `variables` SET `value`='' WHERE (`varname`='hotfix_name')");
			database.QueryDatabase(query);
		}
	}

	/**
	 * Rules: TODO: Remove later
	 */
	{
		std::string tmp;
		if (database.GetVariable("RuleSet", tmp)) {
			LogInfo("Loading rule set [{}]", tmp.c_str());
			if (!RuleManager::Instance()->LoadRules(&database, tmp.c_str(), false)) {
				LogError("Failed to load ruleset [{}], falling back to defaults", tmp.c_str());
			}
		}
		else {
			if (!RuleManager::Instance()->LoadRules(&database, "default", false)) {
				LogInfo("No rule set configured, using default rules");
			}
			else {
				LogInfo("Loaded default rule set 'default'");
			}
		}

		EQ::InitializeDynamicLookups();
		LogInfo("Initialized dynamic dictionary entries");
	}


	content_service.SetCurrentExpansion(RuleI(Expansion, CurrentExpansion));
	content_service.SetDatabase(&database)
		->SetExpansionContext()
		->ReloadContentFlags();

	LogInfo(
		"Current expansion is [{}] ({})",
		content_service.GetCurrentExpansion(),
		content_service.GetCurrentExpansionName()
	);

	std::string hotfix_name = "";

	bool load_all        = true;
	bool load_items      = false;
	bool load_factions   = false;
	bool load_loot       = false;
	bool load_skill_caps = false;
	bool load_spells     = false;
	bool load_bd         = false;

	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			switch (argv[i][0]) {
				case 'b':
					if (strcasecmp("base_data", argv[i]) == 0) {
						load_bd  = true;
						load_all = false;
					}
					break;

				case 'i':
					if (strcasecmp("items", argv[i]) == 0) {
						load_items = true;
						load_all   = false;
					}
					break;

				case 'f':
					if (strcasecmp("factions", argv[i]) == 0) {
						load_factions = true;
						load_all      = false;
					}
					break;

				case 'l':
					if (strcasecmp("loot", argv[i]) == 0) {
						load_loot = true;
						load_all  = false;
					}
					break;

				case 's':
					if (strcasecmp("skill_caps", argv[i]) == 0) {
						load_skill_caps = true;
						load_all        = false;
					}
					else if (strcasecmp("spells", argv[i]) == 0) {
						load_spells = true;
						load_all    = false;
					}
					break;
				case '-': {
					auto split = SplitString(argv[i], '=');
					if (split.size() >= 2) {
						auto command  = split[0];
						auto argument = split[1];
						if (strcasecmp("-hotfix", command.c_str()) == 0) {
							hotfix_name = argument;
							load_all    = true;
						}
					}
					break;
				}
			}
		}
	}

	if (hotfix_name.length() > 0) {
		LogInfo("Writing data for hotfix [{}]", hotfix_name.c_str());
	}

	if (load_all || load_items) {
		LogInfo("Loading items");
		try {
			LoadItems(&content_db, hotfix_name);
		} catch (std::exception &ex) {
			LogError("{}", ex.what());
			return 1;
		}
	}

	if (load_all || load_factions) {
		LogInfo("Loading factions");
		try {
			LoadFactions(&content_db, hotfix_name);
		} catch (std::exception &ex) {
			LogError("{}", ex.what());
			return 1;
		}
	}

	if (load_all || load_loot) {
		LogInfo("Loading loot");
		try {
			LoadLoot(&content_db, hotfix_name);
		} catch (std::exception &ex) {
			LogError("{}", ex.what());
			return 1;
		}
	}

	if (load_all || load_skill_caps) {
		LogInfo("Loading skill caps");
		try {
			LoadSkillCaps(&content_db, hotfix_name);
		} catch (std::exception &ex) {
			LogError("{}", ex.what());
			return 1;
		}
	}

	if (load_all || load_spells) {
		LogInfo("Loading spells");
		try {
			LoadSpells(&content_db, hotfix_name);
		} catch (std::exception &ex) {
			LogError("{}", ex.what());
			return 1;
		}
	}

	if (load_all || load_bd) {
		LogInfo("Loading base data");
		try {
			LoadBaseData(&content_db, hotfix_name);
		} catch (std::exception &ex) {
			LogError("{}", ex.what());
			return 1;
		}
	}

	LogSys.CloseFileLogs();
	return 0;
}
