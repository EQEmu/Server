#include "../common/debug.h"
#include "../common/types.h"
#include "../common/eq_packet_structs.h"
#include <stdio.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

#include "../common/dbcore.h"

#define MAX_CONVERT_STEPS 10

int convert_profile_once(char *src, char *dst, int len);

int main() {
	
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
	
	vector<char *> updates;
	
	MYSQL m;
	MYSQL out;

	mysql_init(&m);
	mysql_init(&out);
	
	if(!mysql_real_connect(&m, host, user, passwd, database, 0, NULL, 0)) {
		printf("Unable to connect 1: %s.\n", mysql_error(&m));
		return(1);
	}
	if(!mysql_real_connect(&out, host, user, passwd, database, 0, NULL, 0)) {
		printf("Unable to connect 2: %s.\n", mysql_error(&m));
		return(1);
	}

	if(mysql_query(&m, "SELECT id,name,profile FROM character_") != 0) {
		printf("Unable to query.\n");
		return(1);
	}
	
	MYSQL_RES *res = mysql_use_result(&m);
	if(res == NULL) {
		printf("Unable to use res.\n");
		return(1);
	}
	
	char *inbuffer = new char[sizeof(PlayerProfile_Struct)];
	PlayerProfile_Struct *in_pp = (PlayerProfile_Struct *) inbuffer;
	
	MYSQL_ROW row;
	unsigned long *lengths;
	
	int convert_count = 0;
	int correct_count = 0;
	int failed_count = 0;

	while((row = mysql_fetch_row(res))) {
		lengths = mysql_fetch_lengths(res);
		unsigned long id = atoi(row[0]);
		
		int curlen = lengths[2];
		
		if(curlen != sizeof(PlayerProfile_Struct)) {
			fprintf(stderr, "Char '%s' has the wrong size. Expected %d, got %d\n", row[1], sizeof(PlayerProfile_Struct), curlen);
			failed_count++;
			continue;
		}
		
		
		//make a copy of the version in the DB
		memcpy(inbuffer, row[2], curlen);

		bool change = false;
		int r;
		for(r = 0; r < MAX_PP_SKILL; r++) {
fprintf(stderr, "Char '%s' skill %d = %d\n", row[1], r, in_pp->skills[r]);
			if(in_pp->skills[r] == 254) {
				in_pp->skills[r] = 0;
				change = true;
			}
		}
		if(!change) {
			correct_count++;
			continue;
		}
		
		fprintf(stderr, "Converting char %lu: %s...", id, row[1]);
		convert_count++;
		
		char *outbuffer = new char[2*sizeof(PlayerProfile_Struct) + 512];
		
		
		//the correct profile ends up in inbuffer, so we can escape it into outbuffer
		char *bptr = outbuffer;
		bptr += snprintf(bptr, 128, "UPDATE character_ SET profile='");
		bptr += mysql_real_escape_string(&m, bptr, (const char *) inbuffer, sizeof(PlayerProfile_Struct));
		snprintf(bptr, 128, "' WHERE id=%lu", id);
		
//		printf("Query: '%s'\n", outbuffer);
/*		if(mysql_query(&out, outbuffer) != 0) {
			failed_count++;
			printf(" Error updating char id %lu: %s\n", id, mysql_error(&m));
			continue;
		}
*/		
updates.push_back(outbuffer);
		fprintf(stderr, " done.\n");
//		convert_count++;
	}
//	mysql_free_result(res);
	

	vector<char *>::iterator cur, end;
	cur = updates.begin();
	end = updates.end();
	printf("Querying.");
	for(; cur != end; cur++) {
		/*if(mysql_query(&out, *cur) != 0) {
			failed_count++;
			printf(" Error updating some char: %s\n", mysql_error(&m));
			continue;
		}
printf(".");
fflush(stdout);*/
		delete[] *cur;
		
		convert_count++;
	}

	fprintf(stderr, "%d chars converted, %d errors, %d chars were up to date.\n", convert_count, failed_count, correct_count);
	
	mysql_close(&m);
	mysql_close(&out);
	return(0);
}







