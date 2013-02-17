#ifndef ZONEDBASYNC_H
#define ZONEDBASYNC_H

#include "../common/dbasync.h"
void DispatchFinishedDBAsync(DBAsyncWork* iDBAW);
bool DBAsyncCB_CharacterBackup(DBAsyncWork* iWork);

#define DBA_b4_Main			1
#define DBA_b4_Worldserver	2
#define DBA_b4_Zone			3
#define DBA_b4_Entity		4

#define DBA_b1_Entity_SeeQPT				0
#define DBA_b1_Entity_Client_InfoForLogin	1
#define DBA_b1_Entity_Client_Save			2
#define DBA_b1_Entity_Client_Backup			3
#define DBA_b1_Entity_Corpse_Backup			4
#define DBA_b1_Zone_MerchantLists			5
#define DBA_b1_Zone_MerchantListsTemp		6

#endif

