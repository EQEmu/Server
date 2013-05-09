// Doors
#ifdef SHAREMEM
int32 Database::GetDoorsCount(uint32* oMaxID) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(id), count(*) FROM doors");
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete(query);
		row = mysql_fetch_row(result);
		if (row && row[1]) {
			int32 ret = atoi(row[1]);
			if (oMaxID) {
				if (row[0])
					*oMaxID = atoi(row[0]);
				else
					*oMaxID = 0;
			}
			mysql_free_result(result);
			return ret;
		}
	}
	else {
		cerr << "Error in GetDoorsCount query '" << query << "' " << errbuf << endl;
		delete[] query;
		return -1;
	}

	return -1;
}

extern "C" bool extDBLoadDoors(uint32 iDoorCount, uint32 iMaxDoorID) { return database.DBLoadDoors(iDoorCount, iMaxDoorID); }
const Door* Database::GetDoor(uint8 door_id, const char* zone_name) {
	for(uint32 i=0; i<max_door_type; i++) {
		const Door* door = GetDoorDBID(i);
		if(door && door->door_id == door_id && strcasecmp(door->zone_name, zone_name) == 0)
			return door;
	}
	return 0;
}

const Door* Database::GetDoorDBID(uint32 db_id) {
	return EMuShareMemDLL.Doors.GetDoor(db_id);
}

bool Database::LoadDoors() {
	if (!EMuShareMemDLL.Load())
		return false;
	int32 tmp_max_door_type = -1;
	uint32 tmp = 0;
	tmp_max_door_type = GetDoorsCount(&tmp);
	if (tmp_max_door_type < 0) {
		cout << "Error: Database::LoadDoors-ShareMem: GetDoorsCount() returned < 0" << endl;
		return false;
	}
	max_door_type = tmp_max_door_type;
	bool ret = EMuShareMemDLL.Doors.DLLLoadDoors(&extDBLoadDoors, sizeof(Door), max_door_type, tmp);
	return ret;
}

bool Database::DBLoadDoors(uint32 iDoorCount, uint32 iMaxDoorID) {
	cout << "Loading Doors from database..." << endl;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(id), Count(*) FROM doors");
	if (RunQuery(query, strlen(query), errbuf, &result))
	{
		safe_delete(query);
		row = mysql_fetch_row(result);
		if (row && row[0]) {
			if (atoi(row[0]) > iMaxDoorID) {
				cout << "Error: Insufficient shared memory to load doors." << endl;
				cout << "Max(id): " << atoi(row[0]) << ", iMaxDoorID: " << iMaxDoorID << endl;
				cout << "Fix this by increasing the MMF_MAX_Door_ID define statement" << endl;
				return false;
			}
			if (atoi(row[1]) != iDoorCount) {
				cout << "Error: Insufficient shared memory to load doors." << endl;
				cout << "Count(*): " << atoi(row[1]) << ", iDoorCount: " << iDoorCount << endl;
				return false;
			}
			max_door_type = atoi(row[0]);
			mysql_free_result(result);
			Door tmpDoor;
				MakeAnyLenString(&query, "SELECT id,doorid,zone,name,pos_x,pos_y,pos_z,heading,opentype,guild,lockpick,keyitem,triggerdoor,triggertype from doors");//WHERE zone='%s'", zone_name
				if (RunQuery(query, strlen(query), errbuf, &result))
				{
					safe_delete(query);
					while((row = mysql_fetch_row(result))) {
						memset(&tmpDoor, 0, sizeof(Door));
						tmpDoor.db_id = atoi(row[0]);
						tmpDoor.door_id = atoi(row[1]);
						strn0cpy(tmpDoor.zone_name,row[2],32);
						strn0cpy(tmpDoor.door_name,row[3],32);
						tmpDoor.pos_x = (float)atof(row[4]);
						tmpDoor.pos_y = (float)atof(row[5]);
						tmpDoor.pos_z = (float)atof(row[6]);
						tmpDoor.heading = atoi(row[7]);
						tmpDoor.opentype = atoi(row[8]);
						tmpDoor.guild_id = atoi(row[9]);
						tmpDoor.lockpick = atoi(row[10]);
						tmpDoor.keyitem = atoi(row[11]);
						tmpDoor.trigger_door = atoi(row[12]);
						tmpDoor.trigger_type = atoi(row[13]);
						EMuShareMemDLL.Doors.cbAddDoor(tmpDoor.db_id, &tmpDoor);
						Sleep(0);
					}
					mysql_free_result(result);
				}
				else
				{
					cerr << "Error in DBLoadDoors query '" << query << "' " << errbuf << endl;
					delete[] query;
					return false;
				}
			}
		}
	return true;
}
#endif

