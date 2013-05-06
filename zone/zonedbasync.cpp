#include "../common/debug.h"
#include <iostream>
using namespace std;
#include "entity.h"
#include "masterentity.h"
#include "../common/MiscFunctions.h"
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
					char errbuf[MYSQL_ERRMSG_SIZE];
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
			cout << "Error: DispatchFinishedDBAsync(): Unknown workpt.b4: " << (int) workpt.b4() << ", workpt=" << workpt << endl;
			break;
		}
	}
	safe_delete(dbaw);
}

#define MAX_TO_DELETE	10
#define MAX_BACKUPS		5
bool DBAsyncCB_CharacterBackup(DBAsyncWork* iWork) { // return true means delete data
	char errbuf[MYSQL_ERRMSG_SIZE] = "dbaq == 0";
	MYSQL_RES* result = 0;
	MYSQL_ROW row;
	char* query = 0;
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
	if (dbaq && dbaq->GetAnswer(errbuf, &result)) {
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
			AppendAnyLenString(&query, &size, &len, "Delete from character_backup where id=%u", ToDelete[0]);
			for (uint8 i=1; i<ToDeleteIndex; i++) {
				AppendAnyLenString(&query, &size, &len, " or id=%u", ToDelete[i]);
			}
			if (!database.RunQuery(query, len, errbuf)) {
				LogFile->write(EQEMuLog::Error, "Error in DBAsyncCB_CharacterBackup query2 '%s' %s", query, errbuf);
				safe_delete_array(query);
				return true;
			}
			safe_delete_array(query);
		}
		bool needtoinsert = false;
		for (i=0; i<MAX_BACKUPS; i++) {
			if (BackupAges[i] != 0 && !FoundBackup[i])
				needtoinsert = true;
		}
		if (needtoinsert) {
			if (!database.RunQuery(query, MakeAnyLenString(&query, 
				"Insert Delayed into character_backup (charid, account_id, name, profile, level, class, x, y, z, zoneid) "
				"select id, account_id, name, profile, level, class, x, y, z, zoneid "
				"from character_ where id=%u", iWork->WPT()), errbuf)) {
				cout << "Error in DBAsyncCB_CharacterBackup query3 '" << query << "' " << errbuf << endl;
				safe_delete_array(query);
				return true;
			}
			safe_delete_array(query);
		}
	}
	else {
//		cout << "Error in DBAsyncCB_CharacterBackup query1 '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return true;
	}
	return true;
}
