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
	
	MYSQL_ROW row;
	unsigned long *lengths;
	
	int convert_count = 0;
	int correct_count = 0;
	int failed_count = 0;

	while((row = mysql_fetch_row(res))) {
		lengths = mysql_fetch_lengths(res);
		unsigned long id = atoi(row[0]);
		
		int curlen = lengths[2];
		
		if(curlen == sizeof(PlayerProfile_Struct)) {
			correct_count++;
			continue;	//allready current.
		}
		
		fprintf(stderr, "Converting char %lu: %s...", id, row[1]);
		
		//make a copy of the version in the DB
		memcpy(inbuffer, row[2], curlen);
		
		char *outbuffer = new char[2*sizeof(PlayerProfile_Struct) + 512];
		
		int steps;
		for(steps = 0; steps < MAX_CONVERT_STEPS && curlen != sizeof(PlayerProfile_Struct); steps++) {
			//clear outbuffer
			memset(outbuffer, 0, sizeof(PlayerProfile_Struct));
			
			fprintf(stderr, " |");
			fflush(stderr);
			
			//convert inbuffer one step closer to the current profile, into outbuffer
			curlen = convert_profile_once(inbuffer, outbuffer, curlen);
			if(curlen == 0)
				break;
			
			//copy outbuffer into inbuffer for the next convert step
			memcpy(inbuffer, outbuffer, curlen);
		}
		if(curlen != sizeof(PlayerProfile_Struct)) {
			failed_count++;
			fprintf(stderr, " failed.\n");
			delete[] outbuffer;
			continue;
		}
		fprintf(stderr, " *");
		fflush(stderr);
		
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
		if(mysql_query(&out, *cur) != 0) {
			failed_count++;
			printf(" Error updating some char: %s\n", mysql_error(&m));
			continue;
		}
printf(".");
fflush(stdout);
		delete[] *cur;
		
		convert_count++;
	}

	fprintf(stderr, "%d chars converted, %d errors, %d chars were up to date.\n", convert_count, failed_count, correct_count);
	
	mysql_close(&m);
	mysql_close(&out);
	return(0);
}


//dst is garunteed to be all 0's
int convert_profile_once(char *src, char *dst, int len) {
	switch(len) {
	case sizeof(BeforeFeb18_PlayerProfile_Struct): {
		BeforeFeb18_PlayerProfile_Struct* ops = (BeforeFeb18_PlayerProfile_Struct*) src;
		memcpy(dst, ops, sizeof(BeforeFeb18_PlayerProfile_Struct));
		len = sizeof(BeforeApril14th_PlayerProfile_Struct);
		break;
	}
	case sizeof(BeforeApril14th_PlayerProfile_Struct): {
		BeforeApril14th_PlayerProfile_Struct* ops = (BeforeApril14th_PlayerProfile_Struct*) src;
		memcpy(dst, ops, sizeof(BeforeApril14th_PlayerProfile_Struct));
		len = sizeof(BeforeApr21st_PlayerProfile_Struct);
		break;
	}
	case sizeof(BeforeApr21st_PlayerProfile_Struct): {
		BeforeApr21st_PlayerProfile_Struct* ops = (BeforeApr21st_PlayerProfile_Struct*)src;
		memcpy(dst, ops, sizeof(BeforeApr21st_PlayerProfile_Struct));
		len = sizeof(BeforeMay5th_PlayerProfile_Struct);
		break;
	}
	case sizeof(BeforeMay5th_PlayerProfile_Struct): {
		BeforeMay5th_PlayerProfile_Struct* ops = (BeforeMay5th_PlayerProfile_Struct*)src;
		memcpy(dst, ops, sizeof(BeforeMay5th_PlayerProfile_Struct));
		len = sizeof(PlayerProfile_Struct_Before_May26th);
		break;
	}
	case sizeof(PlayerProfile_Struct_Before_May26th): {
		PlayerProfile_Struct_Before_May26th* ops = (PlayerProfile_Struct_Before_May26th*)src;
		memcpy(dst, ops, sizeof(PlayerProfile_Struct_Before_May26th));
		len = sizeof(Before_Aug13th_PlayerProfile_Struct);
		break;
	}
	case sizeof(Before_Aug13th_PlayerProfile_Struct): {
		Before_Aug13th_PlayerProfile_Struct* ops = (Before_Aug13th_PlayerProfile_Struct*)src;
		Before_Sep14th_PlayerProfile_Struct* pp = (Before_Sep14th_PlayerProfile_Struct*)dst;
		uchar* newpps = new uchar[sizeof(Before_Sep14th_PlayerProfile_Struct)];
		memset(newpps, 0, sizeof(Before_Sep14th_PlayerProfile_Struct));
		uchar* ptr_old=(uchar*)newpps;
		uchar* c_ptr=(uchar*)ops;
		memcpy(ptr_old,c_ptr,588);
		c_ptr+=588;
		ptr_old+=1304;
		memcpy(ptr_old,c_ptr,4440);

		uchar* ptr=(uchar*)dst;
		uchar* s_ptr=(uchar*)newpps;
		memcpy(ptr,s_ptr,124);
		s_ptr+=140;
		ptr+=220;
		memcpy(ptr,s_ptr,88);
		s_ptr+=88;
		ptr+=92;
		memcpy(ptr,s_ptr,1140);
		s_ptr+=1140;
		ptr+=1176;
		memcpy(ptr,s_ptr,8);
		s_ptr+=8;
		ptr+=12;
		memcpy(ptr,s_ptr,3700);
		
		pp->bind_x=((Before_Sep14th_PlayerProfile_Struct*)newpps)->bind_x;
		pp->bind_y=((Before_Sep14th_PlayerProfile_Struct*)newpps)->bind_y;
		pp->bind_z=((Before_Sep14th_PlayerProfile_Struct*)newpps)->bind_z;
		
		len = sizeof(Before_Sep14th_PlayerProfile_Struct);
		break;
	}
	case sizeof(Before_Sep14th_PlayerProfile_Struct): {
		Before_Sep14th_PlayerProfile_Struct* oldpp =(Before_Sep14th_PlayerProfile_Struct*)src;
		Before_Dec15th_PlayerProfile_Struct* pp =(Before_Dec15th_PlayerProfile_Struct*)dst;
		uchar* ptr=(uchar*)dst;
		uchar* s_ptr=(uchar*)oldpp;
		memcpy(ptr,s_ptr,124);
		s_ptr+=140;
		ptr+=220;
		memcpy(ptr,s_ptr,88);
		s_ptr+=88;
		ptr+=92;
		memcpy(ptr,s_ptr,1140);
		s_ptr+=1140;
		ptr+=1176;
		memcpy(ptr,s_ptr,8);
		s_ptr+=8;
		ptr+=12;
		memcpy(ptr,s_ptr,3700);
		//s_ptr+=3020;
		//ptr+=3024;
		//memcpy(ptr,s_ptr,680);
		pp->bind_x[0]=oldpp->bind_x;
		pp->bind_y[0]=oldpp->bind_y;
		pp->bind_z[0]=oldpp->bind_z;
		
		len = sizeof(Before_Dec15th_PlayerProfile_Struct);
		break;
	}
	case sizeof(Before_Dec15th_PlayerProfile_Struct): {
#define StructDist(in, f1, f2) (uint32(&in->f2)-uint32(&in->f1))
		Before_Dec15th_PlayerProfile_Struct* ops = (Before_Dec15th_PlayerProfile_Struct*)src;
		Before_June29th_PlayerProfile_Struct* pp = (Before_June29th_PlayerProfile_Struct*)dst;
		
		//start with the basics
		memcpy(dst, ops, sizeof(Before_June29th_PlayerProfile_Struct));
		
		pp->anon = ops->anon;
		pp->guildrank = ops->guildrank;
		memcpy(pp->unknown0245, &ops->fatigue, StructDist(ops, fatigue, guildid2));
		//just zero out buffs and groups
		memset(pp->buffs, 0, StructDist(pp, buffs, unknown6392));
		//shift down everything after that.
		memcpy(pp->unknown6392, &ops->unknown5248, StructDist(ops, unknown5248, unknown11376));
		//put the tribute block in the right place
		memcpy(&pp->tribute_time_remaining, &ops->tribute_time_remaining, StructDist(ops, tribute_time_remaining, unknown5764));
		
		//copy over things that maybe moved, but I havent figure out how yet
		pp->aapoints = ops->aapoints;
		pp->aapoints_spent = ops->aapoints_spent;
		
		len = sizeof(Before_June29th_PlayerProfile_Struct);
	}
	case sizeof(Before_June29th_PlayerProfile_Struct): {
		Before_June29th_PlayerProfile_Struct* ops = (Before_June29th_PlayerProfile_Struct*)src;
		memcpy(dst, ops, sizeof(Before_June29th_PlayerProfile_Struct));
		
		len = sizeof(Before_May12_PlayerProfile_Struct);
	}
	case sizeof(Before_May12_PlayerProfile_Struct): {
		Before_May12_PlayerProfile_Struct* ops = (Before_May12_PlayerProfile_Struct*)src;
		PlayerProfile_Struct* pp = (PlayerProfile_Struct*)dst;
		memcpy(dst, ops, sizeof(Before_May12_PlayerProfile_Struct));
		
		memcpy(&pp->checksum, &ops->checksum, StructDist(ops, checksum, haircolor));
		memcpy(&pp->haircolor, &ops->haircolor, StructDist(ops, haircolor, unknown0310[0]));
		memcpy(&pp->item_material[0], &ops->item_material[0], StructDist(ops, item_material[0], servername[0]));
		memcpy(&pp->servername[0], &ops->servername[0], StructDist(ops, servername[0], skills[0]));
		memcpy(&pp->skills[0], &ops->skills[0], StructDist(ops, skills[0], zone_change_count));
		memcpy(&pp->zone_change_count, &ops->zone_change_count, StructDist(ops, zone_change_count, unknown4436[0]));
		memcpy(&pp->expAA, &ops->expAA, StructDist(ops, expAA, unknown4484));
		memcpy(&pp->expansion, &ops->expansion, StructDist(ops, expansion, buffs[0]));
		memcpy(&pp->ldon_points_guk, &ops->ldon_points_guk, StructDist(ops, ldon_points_guk, tribute_time_remaining));
		memcpy(&pp->tribute_time_remaining, &ops->tribute_time_remaining, StructDist(ops, tribute_time_remaining, unknown6860));
		memcpy(&pp->leader_abilities, &ops->leader_abilities, StructDist(ops, leader_abilities, unknown6932[0]));
		memcpy(&pp->air_remaining, &ops->air_remaining, StructDist(ops, air_remaining, unknown18492));
		
		/*
		 *  This is the last statement in this switch.
		 */
		len = sizeof(PlayerProfile_Struct);
		break;
	}
	default:
		fprintf(stderr, "Unknown length of player profile: %d, %d\n", len, sizeof(Before_June29th_PlayerProfile_Struct));
		len = 0;
		break;
	}
	return(len);
}







