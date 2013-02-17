#include "../common/types.h"
//#include "../common/eq_packet_structs.h"
#include <stdio.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
using namespace std;

#include "../common/dbcore.h"

#include "char_format.h"

void usage() {
	printf("Usage: export_char [list|charid] [outfile]\n");
	printf("  if charid is 'list' then all char IDs will be listed with names\n");
	printf("  note: this will not work on char ID 0\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	
	const char *outfile;
	
	if(argc != 3)
		usage();
	int charid = atoi(argv[1]);
	if(charid == 0)
		usage();
	outfile = argv[2];
	
	char *query = new char[1024];
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
/*	FactionValueRecord *factions;
	InventoryEntry *inventory;
	InventoryEntry *sharedbank;
	QuestGlobalEntry *qglobals;*/
	
	sprintf(query, "SELECT name,profile,extprofile,x,y,z,zonename,zoneid FROM character_ WHERE id=%d", charid);
	if(mysql_query(&m, query) != 0) {
		printf("Unable to query.\n");
		return(1);
	}
	
	MYSQL_RES *res = mysql_use_result(&m);
	if(res == NULL) {
		printf("Unable to use character_ res.\n");
		return(1);
	}
	MYSQL_ROW row;
	row = mysql_fetch_row(res);
	if(row == NULL) {
		printf("Unable to find a char with id %d\n", charid);
		return(1);
	}
	unsigned long* lengths;
	lengths = mysql_fetch_lengths(res);
	
	header.char_magic = CHAR_MAGIC;
	strcpy(header.name, row[0]);
	header.profile_length = lengths[1];
	header.extprofile_length = lengths[2];
	header.x = atof(row[3]);
	header.y = atof(row[4]);
	header.z = atof(row[5]);
	strcpy(header.zone_name, row[6]);
	header.zone_id = atoi(row[7]);
	//copy in blobs
	ppbuffer = new char[header.profile_length];
	eppbuffer = new char[header.extprofile_length];
	memcpy(ppbuffer, row[1], header.profile_length);
	memcpy(eppbuffer, row[2], header.extprofile_length);
	
	mysql_free_result(res);
	
	//query faction
	sprintf(query, "SELECT faction_id,current_value FROM faction_values WHERE char_id=%d", charid);
	if(mysql_query(&m, query) != 0) {
		printf("Unable to query faction: %s\n", mysql_error(&m));
		return(1);
	}
	res = mysql_use_result(&m);
	if(res == NULL) {
		printf("Unable to use faction res.\n");
		return(1);
	}
	vector<FactionValueRecord> fr;
	while((row = mysql_fetch_row(res))) {
		FactionValueRecord r;
		r.faction_id = atoi(row[0]);
		r.current_value = atoi(row[1]);
		fr.push_back(r);
	}
	mysql_free_result(res);
  
	//query inventory
	sprintf(query, "SELECT slotid,itemid,charges,color,augslot1,augslot2,augslot3,augslot4,augslot5,instnodrop FROM inventory WHERE charid=%d", charid);
	if(mysql_query(&m, query) != 0) {
		printf("Unable to query inventory: %s\n", mysql_error(&m));
		return(1);
	}
	res = mysql_use_result(&m);
	if(res == NULL) {
		printf("Unable to use inventory res.\n");
		return(1);
	}
	vector<InventoryEntry> inv;
	while((row = mysql_fetch_row(res))) {
		InventoryEntry r;
		r.slotid = atoi(row[0]);
		r.itemid = atoi(row[1]);
		r.charges = atoi(row[2]);
		r.colors = atoi(row[3]);
		r.augs[0] = atoi(row[4]);
		r.augs[1] = atoi(row[5]);
		r.augs[2] = atoi(row[6]);
		r.augs[3] = atoi(row[7]);
		r.augs[4] = atoi(row[8]);
		inv.push_back(r);
	}
	mysql_free_result(res);
  
	//query shared bank
	sprintf(query, "SELECT slotid,itemid,charges,augslot1,augslot2,augslot3,augslot4,augslot5 FROM sharedbank,character_ WHERE sharedbank.acctid = character_.account_id AND character_.id=%d", charid);
	if(mysql_query(&m, query) != 0) {
		printf("Unable to query shared bank: %s\n", mysql_error(&m));
		return(1);
	}
	res = mysql_use_result(&m);
	if(res == NULL) {
		printf("Unable to use shared bank res.\n");
		return(1);
	}
	vector<InventoryEntry> sb;
	while((row = mysql_fetch_row(res))) {
		InventoryEntry r;
		r.slotid = atoi(row[0]);
		r.itemid = atoi(row[1]);
		r.charges = atoi(row[2]);
		r.augs[0] = atoi(row[3]);
		r.augs[1] = atoi(row[4]);
		r.augs[2] = atoi(row[5]);
		r.augs[3] = atoi(row[6]);
		r.augs[4] = atoi(row[7]);
		sb.push_back(r);
	}
	mysql_free_result(res);
  
	//query quest_globals
	sprintf(query, "SELECT npcid,zoneid,name,value,expdate FROM quest_globals WHERE charid=%d", charid);
	if(mysql_query(&m, query) != 0) {
		printf("Unable to query quest globals: %s\n", mysql_error(&m));
		return(1);
	}
	res = mysql_use_result(&m);
	if(res == NULL) {
		printf("Unable to use quest globals res.\n");
		return(1);
	}
	vector<QuestGlobalEntry> qg;
	while((row = mysql_fetch_row(res))) {
		QuestGlobalEntry r;
		r.npcid = atoi(row[0]);
		r.zoneid = atoi(row[1]);
		strcpy(r.name, row[2]);
		strcpy(r.value, row[3]);
		r.expdate = atoi(row[4]);
		qg.push_back(r);
	}
	mysql_free_result(res);
	
	mysql_close(&m);
	
	
	FILE *outf = fopen(outfile, "wb");
	if(outf == NULL) {
		printf("Unable to open output file %s\n", outfile);
		return(1);
	}
	
	//fill in our counters
	header.faction_value_count = fr.size();
	header.inventory_count = inv.size();
	header.sharedbank_count = sb.size();
	header.quest_globals_count = qg.size();
	
	//write header
	fwrite(&header, 1, sizeof(header), outf);
	
	//write out pp and epp
	fwrite(ppbuffer, 1, header.profile_length, outf);
	fwrite(eppbuffer, 1, header.extprofile_length, outf);
	
	//write out our variable length segments
	vector<FactionValueRecord>::iterator curfr, endfr;
	curfr = fr.begin();
	endfr = fr.end();
	for(; curfr != endfr; curfr++) {
		FactionValueRecord &r = *curfr;
		fwrite(&r, 1, sizeof(FactionValueRecord), outf);
	}
	
	vector<InventoryEntry>::iterator curi, endi;
	curi = inv.begin();
	endi = inv.end();
	for(; curi != endi; curi++) {
		InventoryEntry &i = *curi;
		fwrite(&i, 1, sizeof(InventoryEntry), outf);
	}
	
	curi = sb.begin();
	endi = sb.end();
	for(; curi != endi; curi++) {
		InventoryEntry &i = *curi;
		fwrite(&i, 1, sizeof(InventoryEntry), outf);
	}
	
	vector<QuestGlobalEntry>::iterator curqg, endqg;
	curqg = qg.begin();
	endqg = qg.end();
	for(; curqg != endqg; curqg++) {
		QuestGlobalEntry &r = *curqg;
		fwrite(&r, 1, sizeof(QuestGlobalEntry), outf);
	}
	
	fclose(outf);
	
	return(0);
}







