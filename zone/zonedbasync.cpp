#include "../common/debug.h"
#include <iostream>
#include "entity.h"
#include "masterentity.h"
#include "../common/StringUtil.h"
#include "../common/breakdowns.h"
#include <stdlib.h>

extern EntityList entity_list;

void DispatchFinishedDBAsync(DBAsyncWork* dbaw) {
	uint32_breakdown workpt;
	workpt = dbaw->WPT();
	switch (workpt.b4()) {
/*		case DBA_b4_Main: {
			switch (workpt.i24_1()) {
				case DBA_i24_1_Main_LoadVariables: {
					std::string errbuf;
					MYSQL_RES* result;
					DBAsyncQuery* dbaq = dbaw->PopAnswer();
					if (dbaq->GetAnswer(errbuf, result))
						database.LoadVariables_result(result);
					else
						cout << "Async DB.LoadVariables() failed: '" << errbuf << "'" << endl;
					break;
				}
				default: {
					cout << "Error: DispatchFinishedDBAsync(): Unknown workpt.b4" << endl;
					break;
				}
			}
		}*/
		case DBA_b4_Zone: {
			if(zone == nullptr)
				break;
			zone->DBAWComplete(workpt.b1(), dbaw);
			break;
		}
		case DBA_b4_Entity: {
			Entity* entity = entity_list.GetID(workpt.w2_3());
			if (!entity)
				break;
			entity->DBAWComplete(workpt.b1(), dbaw);
			break;
		}
		default: {
			std::cout << "Error: DispatchFinishedDBAsync(): Unknown workpt.b4: " << (int) workpt.b4() << ", workpt=" << workpt << std::endl;
			break;
		}
	}
	safe_delete(dbaw);
}

#define MAX_TO_DELETE	10
#define MAX_BACKUPS		5
bool DBAsyncCB_CharacterBackup(DBAsyncWork* iWork) { // return true means delete data
	std::string errbuf = "dbaq == 0";
	MYSQL_RES* result = 0;
	MYSQL_ROW row;
	std::string query;
	uint32 i;
	uint8 ToDeleteIndex = 0;
	uint32 ToDelete[MAX_TO_DELETE];
	memset(ToDelete, 0, sizeof(ToDelete));

	uint32 BackupAges[MAX_BACKUPS]; // must be sorted, highest value in lowest index
	memset(BackupAges, 0, sizeof(BackupAges));

	bool FoundBackup[MAX_BACKUPS];
	memset(FoundBackup, 0, sizeof(FoundBackup));

	BackupAges[0] = 86400;
	BackupAges[1] = 3600;

	DBAsyncQuery* dbaq = iWork->PopAnswer();
	if (dbaq && dbaq->GetAnswer(&errbuf, &result)) {
		while ((row = mysql_fetch_row(result))) {
			for (i=0; i<MAX_BACKUPS; i++) {
				if (BackupAges[i] == 0 || (uint32)atoi(row[1]) > BackupAges[i])
					i = MAX_BACKUPS;
				else if (!FoundBackup[i]) {
					FoundBackup[i] = true;
					break;
				}
			}
			if (i >= MAX_BACKUPS)
				ToDelete[ToDeleteIndex++] = atoi(row[0]);
			if (ToDeleteIndex >= MAX_TO_DELETE)
				break;
		}
		if (ToDelete[0]) {
			uint32 len = 0, size = 0;
			
			std::string toAppend;
			
			StringFormat(toAppend, "Delete from character_backup where id=%u", ToDelete[0]);
			query.append(toAppend);
			
			for (uint8 i=1; i<ToDeleteIndex; i++) {
				
				StringFormat(toAppend, " or id=%u", ToDelete[i]);
				query.append(toAppend);
				
			}
			if (!database.RunQuery(query, &errbuf)) {
				LogFile->write(EQEMuLog::Error, "Error in DBAsyncCB_CharacterBackup query2 '%s' %s", query.c_str(), errbuf.c_str());
				return true;
			}
		}
		bool needtoinsert = false;
		for (i=0; i<MAX_BACKUPS; i++) {
			if (BackupAges[i] != 0 && !FoundBackup[i])
				needtoinsert = true;
		}
		if (needtoinsert) {
			
			StringFormat(query, "Insert Delayed into character_backup "
								"(charid, account_id, name, profile, level, class, x, y, z, zoneid) "
								"select id, account_id, name, profile, level, class, x, y, z, zoneid "
								"from character_ where id=%u", iWork->WPT());
			
			if (!database.RunQuery(query, &errbuf)) {
				std::cout << "Error in DBAsyncCB_CharacterBackup query3 '" << query << "' " << errbuf << std::endl;
				return true;
			}
		}
	}
	else {
//		cout << "Error in DBAsyncCB_CharacterBackup query1 '" << query << "' " << errbuf << endl;
		return true;
	}
	return true;
}
