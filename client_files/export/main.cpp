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
#include "../../common/debug.h"
#include "../../common/shareddb.h"
#include "../../common/EQEmuConfig.h"
#include "../../common/platform.h"
#include "../../common/crash.h"
#include "../../common/rulesys.h"
#include "../../common/StringUtil.h"

void ExportSpells(SharedDatabase *db);
void ExportSkillCaps(SharedDatabase *db);
void ExportBaseData(SharedDatabase *db);

int main(int argc, char **argv) {
	RegisterExecutablePlatform(ExePlatformClientExport);
	set_exception_handler();
	
	LogFile->write(EQEMuLog::Status, "Client Files Export Utility");
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
	
	ExportSpells(&database);
	ExportSkillCaps(&database);
	ExportBaseData(&database);
	
	return 0;
}

void ExportSpells(SharedDatabase *db) {
	LogFile->write(EQEMuLog::Status, "Exporting Spells...");

	FILE *f = fopen("export/spells_us.txt", "w");
	if(!f) {
		LogFile->write(EQEMuLog::Error, "Unable to open export/spells_us.txt to write, skipping.");
		return;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = "SELECT * FROM spells_new ORDER BY id";
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(db->RunQuery(query, strlen(query), errbuf, &result)) {
		while(row = mysql_fetch_row(result)) {
			std::string line;
			unsigned int fields = mysql_num_fields(result);
			for(unsigned int i = 0; i < fields; ++i) {
				if(i != 0) {
					line.push_back('^');
				}
				
				line += row[i];
			}
			
			fprintf(f, "%s\n", line.c_str());
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in ExportSpells query '%s' %s", query, errbuf);
	}
	
	fclose(f);
}

bool SkillUsable(SharedDatabase *db, int skill_id, int class_id) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = nullptr;
	MYSQL_RES *result;
	MYSQL_ROW row;
	bool res = false;
	if(db->RunQuery(query, MakeAnyLenString(&query, "SELECT max(cap) FROM skill_caps WHERE class=%d AND skillID=%d",
		class_id, skill_id), errbuf, &result)) {
		if(row = mysql_fetch_row(result)) {
			if(row[0] && atoi(row[0]) > 0) {
				res = true;
			}
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in skill_usable query '%s' %s", query, errbuf);
	}

	safe_delete_array(query);
	return res;
}

int GetSkill(SharedDatabase *db, int skill_id, int class_id, int level) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = nullptr;
	MYSQL_RES *result;
	MYSQL_ROW row;
	int res = 0;
	if(db->RunQuery(query, MakeAnyLenString(&query, "SELECT cap FROM skill_caps WHERE class=%d AND skillID=%d AND level=%d",
		class_id, skill_id, level), errbuf, &result)) {
		if(row = mysql_fetch_row(result)) {
			res = atoi(row[0]);
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in get_skill query '%s' %s", query, errbuf);
	}

	safe_delete_array(query);
	return res;
}

void ExportSkillCaps(SharedDatabase *db) {
	LogFile->write(EQEMuLog::Status, "Exporting Skill Caps...");

	FILE *f = fopen("export/SkillCaps.txt", "w");
	if(!f) {
		LogFile->write(EQEMuLog::Error, "Unable to open export/SkillCaps.txt to write, skipping.");
		return;
	}

	for(int cl = 1; cl <= 16; ++cl) {
		for(int skill = 0; skill <= 77; ++skill) {
			if(SkillUsable(db, skill, cl)) {
				int previous_cap = 0;
				for(int level = 1; level <= 100; ++level) {
					int cap = GetSkill(db, skill, cl, level);
					if(cap < previous_cap) {
						cap = previous_cap;
					}

					fprintf(f, "%d^%d^%d^%d^0\n", cl, skill, level, cap);
					previous_cap = cap;
				}
			}
		}
	}

	fclose(f);
}

void ExportBaseData(SharedDatabase *db) {
	LogFile->write(EQEMuLog::Status, "Exporting Base Data...");

	FILE *f = fopen("export/BaseData.txt", "w");
	if(!f) {
		LogFile->write(EQEMuLog::Error, "Unable to open export/BaseData.txt to write, skipping.");
		return;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = "SELECT * FROM base_data ORDER BY level, class";
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(db->RunQuery(query, strlen(query), errbuf, &result)) {
		while(row = mysql_fetch_row(result)) {
			std::string line;
			unsigned int fields = mysql_num_fields(result);
			for(unsigned int i = 0; i < fields; ++i) {
				if(i != 0) {
					line.push_back('^');
				}

				line += row[i];
			}

			fprintf(f, "%s\n", line.c_str());
		}
		mysql_free_result(result);
	} else {
		LogFile->write(EQEMuLog::Error, "Error in ExportBaseData query '%s' %s", query, errbuf);
	}

	fclose(f);
}

