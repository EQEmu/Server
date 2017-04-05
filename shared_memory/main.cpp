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

EQEmuLogSys LogSys;

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

int main(int argc, char **argv) {
	RegisterExecutablePlatform(ExePlatformSharedMemory);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();

	Log(Logs::General, Logs::Status, "Shared Memory Loader Program");
	if(!EQEmuConfig::LoadConfig()) {
		Log(Logs::General, Logs::Error, "Unable to load configuration file.");
		return 1;
	}

	auto Config = EQEmuConfig::get();

	SharedDatabase database;
	Log(Logs::General, Logs::Status, "Connecting to database...");
	if(!database.Connect(Config->DatabaseHost.c_str(), Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(), Config->DatabaseDB.c_str(), Config->DatabasePort)) {
		Log(Logs::General, Logs::Error, "Unable to connect to the database, cannot continue without a "
			"database connection");
		return 1;
	}

	/* Register Log System and Settings */
	database.LoadLogSettings(LogSys.log_settings);
	LogSys.StartFileLogs();

	std::string shared_mem_directory = Config->SharedMemDir;
	if (MakeDirectory(shared_mem_directory)) {
		Log(Logs::General, Logs::Status, "Shared Memory folder doesn't exist, so we created it... '%s'", shared_mem_directory.c_str());
	}

	database.LoadVariables();

	/* If we're running shared memory and hotfix has no custom name, we probably want to start from scratch... */
	std::string db_hotfix_name;
	if (database.GetVariable("hotfix_name", db_hotfix_name)) {
		if (!db_hotfix_name.empty() && strcasecmp("hotfix_", db_hotfix_name.c_str()) == 0) {
			Log(Logs::General, Logs::Status, "Current hotfix in variables is the default %s, clearing out variable", db_hotfix_name.c_str());
			std::string query = StringFormat("UPDATE `variables` SET `value`='' WHERE (`varname`='hotfix_name')");
			database.QueryDatabase(query);
		}
	}

	std::string hotfix_name = "";
	bool load_all = true;
	bool load_items = false;
	bool load_factions = false;
	bool load_loot = false;
	bool load_skill_caps = false;
	bool load_spells = false;
	bool load_bd = false;
	if(argc > 1) {
		for(int i = 1; i < argc; ++i) {
			switch(argv[i][0]) {	
			case 'b':
				if(strcasecmp("base_data", argv[i]) == 0) {
					load_bd = true;
					load_all = false;
				}
				break;
	
			case 'i':
				if(strcasecmp("items", argv[i]) == 0) {
					load_items = true;
					load_all = false;
				}
				break;
	
			case 'f':
				if(strcasecmp("factions", argv[i]) == 0) {
					load_factions = true;
					load_all = false;
				}
				break;
	
			case 'l':
				if(strcasecmp("loot", argv[i]) == 0) {
					load_loot = true;
					load_all = false;
				}
				break;
	
			case 's':
				if(strcasecmp("skill_caps", argv[i]) == 0) {
					load_skill_caps = true;
					load_all = false;
				} else if(strcasecmp("spells", argv[i]) == 0) {
					load_spells = true;
					load_all = false;
				}
				break;
			case '-': {
				auto split = SplitString(argv[i], '=');
				if(split.size() >= 2) {
					auto command = split[0];
					auto argument = split[1];
					if(strcasecmp("-hotfix", command.c_str()) == 0) {
						hotfix_name = argument;
						load_all = true;
					}
				}
				break;
			}
			}
		}
	}

	if(hotfix_name.length() > 0) {
		Log(Logs::General, Logs::Status, "Writing data for hotfix '%s'", hotfix_name.c_str());
	}
	
	if(load_all || load_items) {
		Log(Logs::General, Logs::Status, "Loading items...");
		try {
			LoadItems(&database, hotfix_name);
		} catch(std::exception &ex) {
			Log(Logs::General, Logs::Error, "%s", ex.what());
			return 1;
		}
	}
	
	if(load_all || load_factions) {
		Log(Logs::General, Logs::Status, "Loading factions...");
		try {
			LoadFactions(&database, hotfix_name);
		} catch(std::exception &ex) {
			Log(Logs::General, Logs::Error, "%s", ex.what());
			return 1;
		}
	}
	
	if(load_all || load_loot) {
		Log(Logs::General, Logs::Status, "Loading loot...");
		try {
			LoadLoot(&database, hotfix_name);
		} catch(std::exception &ex) {
			Log(Logs::General, Logs::Error, "%s", ex.what());
			return 1;
		}
	}
	
	if(load_all || load_skill_caps) {
		Log(Logs::General, Logs::Status, "Loading skill caps...");
		try {
			LoadSkillCaps(&database, hotfix_name);
		} catch(std::exception &ex) {
			Log(Logs::General, Logs::Error, "%s", ex.what());
			return 1;
		}
	}
	
	if(load_all || load_spells) {
		Log(Logs::General, Logs::Status, "Loading spells...");
		try {
			LoadSpells(&database, hotfix_name);
		} catch(std::exception &ex) {
			Log(Logs::General, Logs::Error, "%s", ex.what());
			return 1;
		}
	}
	
	if(load_all || load_bd) {
		Log(Logs::General, Logs::Status, "Loading base data...");
		try {
			LoadBaseData(&database, hotfix_name);
		} catch(std::exception &ex) {
			Log(Logs::General, Logs::Error, "%s", ex.what());
			return 1;
		}
	}
	
	LogSys.CloseFileLogs();
	return 0;
}
