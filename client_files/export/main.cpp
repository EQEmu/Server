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
#include "../../common/eqemu_config.h"
#include "../../common/platform.h"
#include "../../common/crash.h"
#include "../../common/rulesys.h"
#include "../../common/string_util.h"

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

	const std::string query = "SELECT * FROM spells_new ORDER BY id";
	auto results = db->QueryDatabase(query);

	if(results.Success()) {
        for (auto row = results.begin(); row != results.end(); ++row) {
			std::string line;
			unsigned int fields = results.ColumnCount();
			for(unsigned int i = 0; i < fields; ++i) {
				if(i != 0) {
					line.push_back('^');
				}

				if(row[i] != nullptr) {
					line += row[i];
				}
			}

			fprintf(f, "%s\n", line.c_str());
		}
	} else {
		LogFile->write(EQEMuLog::Error, "Error in ExportSpells query '%s' %s", query.c_str(), results.ErrorMessage().c_str());
	}

	fclose(f);
}

bool SkillUsable(SharedDatabase *db, int skill_id, int class_id) {

	bool res = false;

	std::string query = StringFormat("SELECT max(cap) FROM skill_caps WHERE class=%d AND skillID=%d",
                                    class_id, skill_id);
	auto results = db->QueryDatabase(query);
	if(!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in skill_usable query '%s' %s", query.c_str(), results.ErrorMessage().c_str());
        return false;
    }

    if (results.RowCount() == 0)
        return false;

    auto row = results.begin();
    if(row[0] && atoi(row[0]) > 0)
        return true;

    return false;
}

int GetSkill(SharedDatabase *db, int skill_id, int class_id, int level) {

	std::string query = StringFormat("SELECT cap FROM skill_caps WHERE class=%d AND skillID=%d AND level=%d",
                                    class_id, skill_id, level);
    auto results = db->QueryDatabase(query);
    if (!results.Success()) {
        LogFile->write(EQEMuLog::Error, "Error in get_skill query '%s' %s", query.c_str(), results.ErrorMessage().c_str());
        return 0;
    }

    if (results.RowCount() == 0)
        return 0;

    auto row = results.begin();
	return atoi(row[0]);
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

	const std::string query = "SELECT * FROM base_data ORDER BY level, class";
	auto results = db->QueryDatabase(query);
	if(results.Success()) {
        for (auto row = results.begin();row != results.end();++row) {
			std::string line;
			unsigned int fields = results.ColumnCount();
			for(unsigned int rowIndex = 0; rowIndex < fields; ++rowIndex) {
				if(rowIndex != 0)
					line.push_back('^');

				if(row[rowIndex] != nullptr) {
					line += row[rowIndex];
				}
			}

			fprintf(f, "%s\n", line.c_str());
		}
	} else {
		LogFile->write(EQEMuLog::Error, "Error in ExportBaseData query '%s' %s", query.c_str(), results.ErrorMessage().c_str());
	}

	fclose(f);
}

