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

#include "../../common/debug.h"
#include "../../common/shareddb.h"
#include "../../common/EQEmuConfig.h"
#include "../../common/platform.h"
#include "../../common/crash.h"
#include "../../common/rulesys.h"
#include "../../common/StringUtil.h"

void ImportSpells(SharedDatabase *db);
void ImportSkillCaps(SharedDatabase *db);
void ImportBaseData(SharedDatabase *db);

int main(int argc, char **argv) {
	RegisterExecutablePlatform(ExePlatformClientImport);
	set_exception_handler();
	
	LogFile->write(EQEMuLog::Status, "Client Files Import Utility");
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

	ImportSpells(&database);
	ImportSkillCaps(&database);
	ImportBaseData(&database);
	
	return 0;
}

int GetSpellColumns(SharedDatabase *db) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = "DESCRIBE spells_new";
	MYSQL_RES *result;
	MYSQL_ROW row;
	int res = 0;
	if(db->RunQuery(query, strlen(query), errbuf, &result)) {
		while(row = mysql_fetch_row(result)) {
			++res;
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in GetSpellColumns query '%s' %s", query, errbuf);
	}

	return res;
}

void ImportSpells(SharedDatabase *db) {
	LogFile->write(EQEMuLog::Status, "Importing Spells...");
	FILE *f = fopen("import/spells_us.txt", "r");
	if(!f) {
		LogFile->write(EQEMuLog::Error, "Unable to open import/spells_us.txt to read, skipping.");
		return;
	}

	int columns = GetSpellColumns(db);

	char buffer[2048];
	while(fgets(buffer, 2048, f)) {
		auto split = SplitString(buffer, '^');
	}

	fclose(f);
}

void ImportSkillCaps(SharedDatabase *db) {
	LogFile->write(EQEMuLog::Status, "Importing Skill Caps...");
	FILE *f = fopen("import/SkillCaps.txt", "r");
	if(!f) {
		LogFile->write(EQEMuLog::Error, "Unable to open import/SkillCaps.txt to read, skipping.");
		return;
	}

	fclose(f);
}

void ImportBaseData(SharedDatabase *db) {
	LogFile->write(EQEMuLog::Status, "Importing Base Data...");
	FILE *f = fopen("import/BaseData.txt", "r");
	if(!f) {
		LogFile->write(EQEMuLog::Error, "Unable to open import/BaseData.txt to read, skipping.");
		return;
	}

	fclose(f);
}