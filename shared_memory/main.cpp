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
#include "../common/debug.h"
#include "../common/shareddb.h"
#include "../common/EQEmuConfig.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/rulesys.h"
#include "../common/eqemu_exception.h"
#include "items.h"
#include "npc_faction.h"
#include "loot.h"
#include "skill_caps.h"
#include "spells.h"

int main(int argc, char **argv) {
	RegisterExecutablePlatform(ExePlatformSharedMemory);
	set_exception_handler();

	LogFile->write(EQEMuLog::Status, "Shared Memory Loader Program");
	if(!EQEmuConfig::LoadConfig()) {
		LogFile->write(EQEMuLog::Error, "Unable to load configuration file.");
		return 1;
	}

	const EQEmuConfig *config = EQEmuConfig::get();
	if(!load_log_settings(config->LogSettingsFile.c_str())) {
		LogFile->write(EQEMuLog::Error, "Warning: unable to read %s.", config->LogSettingsFile.c_str());
	}

	SharedDatabase database;
	LogFile->write(EQEMuLog::Status, "Connecting to database...");
	if(!database.Connect(config->DatabaseHost.c_str(), config->DatabaseUsername.c_str(),
		config->DatabasePassword.c_str(), config->DatabaseDB.c_str(), config->DatabasePort)) {
		LogFile->write(EQEMuLog::Error, "Unable to connect to the database, cannot continue without a "
			"database connection");
		return 1;
	}

	bool load_all = true;
	bool load_items = true;
	bool load_factions = true;
	bool load_loot = true;
	bool load_skill_caps = true;
	bool load_spells = true;
	if(argc > 1) {
		load_all = false;
		load_items = false;
		load_factions = false;
		load_loot = false;
		load_skill_caps = false;
		load_spells = false;

		for(int i = 1; i < argc; ++i) {
			switch(argv[i][0]) {
			case 'a':
				if(strcasecmp("all", argv[i]) == 0) {
					load_all = true;
				}
				break;

			case 'i':
				if(strcasecmp("items", argv[i]) == 0) {
					load_items = true;
				}
				break;

			case 'f':
				if(strcasecmp("factions", argv[i]) == 0) {
					load_factions = true;
				}
				break;

			case 'l':
				if(strcasecmp("loot", argv[i]) == 0) {
					load_loot = true;
				}
				break;

			case 's':
				if(strcasecmp("skill_caps", argv[i]) == 0) {
					load_skill_caps = true;
				} else if(strcasecmp("spells", argv[i]) == 0) {
					load_spells = true;
				}
				break;
			}
		}
	}

	if(load_all || load_items) {
		LogFile->write(EQEMuLog::Status, "Loading items...");
		try {
			LoadItems(&database);
		} catch(std::exception &ex) {
			LogFile->write(EQEMuLog::Error, "%s", ex.what());
			return 1;
		}
	}

	if(load_all || load_factions) {
		LogFile->write(EQEMuLog::Status, "Loading factions...");
		try {
			LoadFactions(&database);
		} catch(std::exception &ex) {
			LogFile->write(EQEMuLog::Error, "%s", ex.what());
			return 1;
		}
	}

	if(load_all || load_loot) {
		LogFile->write(EQEMuLog::Status, "Loading loot...");
		try {
			LoadLoot(&database);
		} catch(std::exception &ex) {
			LogFile->write(EQEMuLog::Error, "%s", ex.what());
			return 1;
		}
	}

	if(load_all || load_skill_caps) {
		LogFile->write(EQEMuLog::Status, "Loading skill caps...");
		try {
			LoadSkillCaps(&database);
		} catch(std::exception &ex) {
			LogFile->write(EQEMuLog::Error, "%s", ex.what());
			return 1;
		}
	}

	if(load_all || load_spells) {
		LogFile->write(EQEMuLog::Status, "Loading spells...");
		try {
			LoadSpells(&database);
		} catch(std::exception &ex) {
			LogFile->write(EQEMuLog::Error, "%s", ex.what());
			return 1;
		}
	}

	return 0;
}
