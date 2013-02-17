#include "../common/types.h"
//#include "../common/eq_packet_structs.h"
#include <stdio.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>

#include "../common/dbcore.h"

#include "char_format.h"

void usage() {
	printf("Usage: import_char [accountid] [infile]\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	
	const char *infile;
	
	if(argc != 3)
		usage();
	int accountid = atoi(argv[1]);
	if(accountid == 0)
		usage();
	infile = argv[2];
	
	char *query = new char[1024000];
	char *cursor;
	char host[200], user[200], passwd[200], database[200];
	int32 port=0;
	bool compression = false;
	bool items[6] = {false, false, false, false, false, false};
	
	if(!DBcore::ReadDBINI(host, user, passwd, database, port, compression, items)) {
		exit(1);
	}
	
	if (!items[0] || !items[1] || !items[2] || !items[3])
	{
		printf ("Incomplete DB.INI file.\n");
		exit (1);
	}
	
	MYSQL m;

	mysql_init(&m);
	
	if(!mysql_real_connect(&m, host, user, passwd, database, 0, NULL, 0)) {
		printf("Unable to connect 1: %s.\n", mysql_error(&m));
		return(1);
	}
	
	CharHeader header;
	char *ppbuffer;
	char *eppbuffer;
	FactionValueRecord *factions;
	InventoryEntry *inventory;
	InventoryEntry *sharedbank;
	QuestGlobalEntry *qglobals;
	
	FILE *inf = fopen(infile, "rb");
	if(inf == NULL) {
		printf("Unable to open infile %s\n", infile);
		return(1);
	}
	
	if(fread(&header, sizeof(header), 1, inf) != 1) {
		printf("Error reading header.\n");
		return(1);
	}
	
	ppbuffer = new char[header.profile_length];
	eppbuffer = new char[header.extprofile_length];
	factions = new FactionValueRecord[header.faction_value_count];
	inventory = new InventoryEntry[header.inventory_count];
	sharedbank = new InventoryEntry[header.sharedbank_count];
	qglobals = new QuestGlobalEntry[header.quest_globals_count];
	
	//read all the shit in
	if(fread(ppbuffer, header.profile_length, 1, inf) != 1) {
		printf("Error reading pp.\n");
		return(1);
	}
	if(fread(eppbuffer, header.extprofile_length, 1, inf) != 1) {
		printf("Error reading epp.\n");
		return(1);
	}
	if(fread(factions, sizeof(FactionValueRecord), header.faction_value_count, inf) != header.faction_value_count) {
		printf("Error reading factions.\n");
		return(1);
	}
	if(fread(inventory, sizeof(InventoryEntry), header.inventory_count, inf) != header.inventory_count) {
		printf("Error reading inventory.\n");
		return(1);
	}
	if(fread(sharedbank, sizeof(InventoryEntry), header.sharedbank_count, inf) != header.sharedbank_count) {
		printf("Error reading sharedbank.\n");
		return(1);
	}
	if(fread(qglobals, sizeof(QuestGlobalEntry), header.quest_globals_count, inf) != header.quest_globals_count) {
		printf("Error reading quest globals.\n");
		return(1);
	}
	
	fclose(inf);
	
	cursor = query;
	cursor += sprintf(cursor, "INSERT INTO character_ "
				"(account_id,name,profile,extprofile,x,y,z,zonename,zoneid)"
				"VALUES(%d,'%s','", accountid, header.name );
	cursor += mysql_real_escape_string(&m, cursor, (const char *) ppbuffer, header.profile_length);
	cursor += sprintf(cursor, "','");
	cursor += mysql_real_escape_string(&m, cursor, (const char *) eppbuffer, header.extprofile_length);
	sprintf(cursor, "',%f,%f,%f,'%s',%d)", header.x, header.y, header.z, header.zone_name, header.zone_id);
	if(mysql_query(&m, query) != 0) {
		printf("Unable to insert character: %s\n", mysql_error(&m));
		return(1);
	}
	int charid = mysql_insert_id(&m);
	uint32 r;
	
	//faction
	for(r = 0; r < header.faction_value_count; r++) {
		sprintf(query, "INSERT INTO faction_values (char_id,faction_id,current_value)"
			" VALUES(%d,%d,%d)", charid, factions[r].faction_id, factions[r].current_value);
		if(mysql_query(&m, query) != 0) {
			printf("Unable to insert faction: %s\n", mysql_error(&m));
			return(1);
		}
	}
	
	//inventory
	for(r = 0; r < header.inventory_count; r++) {
		sprintf(query, "INSERT INTO inventory (charid,slotid,itemid,charges,color,augslot1,augslot2,augslot3,augslot4,augslot5,instnodrop)"
			" VALUES(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)", charid,
			inventory[r].slotid, inventory[r].itemid, inventory[r].charges, inventory[r].colors,
			inventory[r].augs[0], inventory[r].augs[1], inventory[r].augs[2], inventory[r].augs[3], 
			inventory[r].augs[4], inventory[r].instnodrop
			);
		if(mysql_query(&m, query) != 0) {
			printf("Unable to insert faction: %s\n", mysql_error(&m));
			return(1);
		}
	}
	
	//shared bank
	//this is far from perfect since this is per-account, works great with empty shard bank...
	for(r = 0; r < header.sharedbank_count; r++) {
		sprintf(query, "INSERT INTO sharedbank (acctid,slotid,itemid,charges,augslot1,augslot2,augslot3,augslot4,augslot5)"
			" VALUES(%d,%d,%d,%d,%d,%d,%d,%d,%d)", accountid,
			sharedbank[r].slotid, sharedbank[r].itemid, sharedbank[r].charges,
			sharedbank[r].augs[0], sharedbank[r].augs[1], sharedbank[r].augs[2], sharedbank[r].augs[3], sharedbank[r].augs[4]
			);
		if(mysql_query(&m, query) != 0) {
		//ignore errors on insert
			printf("Shared bank warning: %s\n", mysql_error(&m));
		//	return(1);
		}
	}
  
	//quest globals
	for(r = 0; r < header.quest_globals_count; r++) {
		cursor = query;
		cursor += sprintf(cursor, "INSERT INTO quest_globals (charid,npcid,zoneid,name,value,expdate)"
			" VALUES(%d,%d,%d,'", charid, qglobals[r].npcid, qglobals[r].zoneid);
		cursor += mysql_real_escape_string(&m, cursor, (const char *) qglobals[r].name, strlen(qglobals[r].name));
		cursor += sprintf(cursor, "','");
		cursor += mysql_real_escape_string(&m, cursor, (const char *) qglobals[r].value, strlen(qglobals[r].value));
		sprintf(cursor, "',%d)", qglobals[r].expdate);
		
		if(mysql_query(&m, query) != 0) {
			printf("Unable to insert quest global: %s\n", mysql_error(&m));
			return(1);
		}
	}
	
	mysql_close(&m);
	
	printf("Successfully imported %s as character id %d\n", header.name, charid);
	
	return(0);
}







