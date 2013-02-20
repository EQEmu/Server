/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2013  EQEMu Development Team (http://eqemulator.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include "../common/debug.h"
#include "../common/shareddb.h"
#include "../common/EQEmuConfig.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/rulesys.h"
#include "../common/eqemu_exception.h"
#include "spells.h"
#include "skill_caps.h"

//blah global variables =(
RuleManager *rules = new RuleManager();

int main(int argc, char **argv) {
    RegisterExecutablePlatform(ExePlatformSharedMemory);
    set_exception_handler();

    LogFile->write(EQEMuLog::Status, "Shared Memory Loader Program");
    if(!EQEmuConfig::LoadConfig()) {
        LogFile->write(EQEMuLog::Error, "Unable to load configuration file.");
        return 0;
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
		return 0;
	}

    bool load_all = true;
    bool load_spells = true;
    bool load_skill_caps = true;
    
    if(load_all || load_spells) {
        LogFile->write(EQEMuLog::Status, "Loading spells...");
        try {
            LoadSpells(&database);
        } catch(std::exception &ex) {
            LogFile->write(EQEMuLog::Error, "%s", ex.what());
            return 0;
        }
    }
    
    if(load_all || load_skill_caps) {
        LogFile->write(EQEMuLog::Status, "Loading skill caps...");
        try {
            LoadSkillCaps(&database);
        } catch(std::exception &ex) {
            LogFile->write(EQEMuLog::Error, "%s", ex.what());
            return 0;
        }
    }

    return 0;
}