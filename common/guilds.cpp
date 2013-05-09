/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/debug.h"
#include "MiscFunctions.h"
#include "guilds.h"
#include "database.h"
#include "eq_packet_structs.h"


#ifndef WIN32
#include <netinet/in.h>	//for htonl
#endif

/*
void Database::GetGuildMembers(uint32 guild_id, GuildMember_Struct* gms){
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 count=0;
	uint32 length=0;
	if (RunQuery(query, MakeAnyLenString(&query, "Select name,profile,timelaston,guildrank,publicnote from character_ where guild=%i", guild_id), errbuf, &result)) {
		safe_delete_array(query);
		while( ( row = mysql_fetch_row(result) ) ){
			strcpy(gms->member[count].name,row[0]);
			length+=strlen(row[0])+strlen(row[4]);
			PlayerProfile_Struct* pps=(PlayerProfile_Struct*)row[1];
			gms->member[count].level=htonl(pps->level);
			gms->member[count].zoneid=(pps->zone_id*256);
			gms->member[count].timelaston=htonl(atol(row[2]));
			gms->member[count].class_=htonl(pps->class_);
			gms->member[count].rank=atoi(row[3]);
			strcpy(gms->member[count].publicnote,row[4]);
			count++;
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in GetGuildMembers query '%s': %s", query, errbuf);
		safe_delete_array(query);
	}
	gms->count=count;
	gms->length=length;
}

uint32 Database::NumberInGuild(uint32 guild_id) {
		char errbuf[MYSQL_ERRMSG_SIZE];
		char *query = 0;
		MYSQL_RES *result;
		MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "Select count(id) from character_ where guild=%i", guild_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			uint32 ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in NumberInGuild query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return 0;
	}
	return 0;
}
bool Database::SetGuild(char* name, uint32 guild_id, uint8 guildrank) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET guild=%i, guildrank=%i WHERE name='%s'", guild_id, guildrank, name), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1)
			return true;
		else
			return false;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in SetGuild query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	return false;
}

bool Database::SetGuild(uint32 charid, uint32 guild_id, uint8 guildrank) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "UPDATE character_ SET guild=%i, guildrank=%i WHERE id=%i", guild_id, guildrank, charid), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1)
			return true;

		else
			return false;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in SetGuild query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	return false;
}

bool Database::DeleteGuild(uint32 guild_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	char *query2 = 0;
	uint32 affected_rows = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "DELETE FROM guilds WHERE id=%i;", guild_id), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1) {
			if(!RunQuery(query2, MakeAnyLenString(&query2, "update character_ set guild=0,guildrank=0 where guild=%i", guild_id), errbuf, 0, &affected_rows))
				LogFile->write(EQEMuLog::Error, "Error in DeleteGuild cleanup query '%s': %s", query2, errbuf);
			safe_delete_array(query2);
			return true;
		}
		else
			return false;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in DeleteGuild query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	return false;
}

bool Database::RenameGuild(uint32 guild_id, const char* name) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;
	char buf[65];
	DoEscapeString(buf, name, strlen(name)) ;

	if (RunQuery(query, MakeAnyLenString(&query, "Update guilds set name='%s' WHERE id=%i;", buf, guild_id), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1)
			return true;
		else
			return false;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in RenameGuild query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	return false;
}



bool Database::EditGuild(uint32 guild_id, uint8 ranknum, GuildRankLevel_Struct* grl)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	int chars = 0;
	uint32 affected_rows = 0;
	char buf[203];
	char buf2[8];
	DoEscapeString(buf, grl->rankname, strlen(grl->rankname)) ;
	buf2[GUILD_HEAR] = grl->heargu + '0';
	buf2[GUILD_SPEAK] = grl->speakgu + '0';
	buf2[GUILD_INVITE] = grl->invite + '0';
	buf2[GUILD_REMOVE] = grl->remove + '0';
	buf2[GUILD_PROMOTE] = grl->promote + '0';
	buf2[GUILD_DEMOTE] = grl->demote + '0';
	buf2[GUILD_MOTD] = grl->motd + '0';
	buf2[GUILD_WARPEACE] = grl->warpeace + '0';

	if (ranknum == 0)
		chars = MakeAnyLenString(&query, "Update guilds set rank%ititle='%s' WHERE id=%i;", ranknum, buf, guild_id);
	else
		chars = MakeAnyLenString(&query, "Update guilds set rank%ititle='%s', rank%i='%s' WHERE id=%i;", ranknum, buf, ranknum, buf2, guild_id);

	if (RunQuery(query, chars, errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1)
			return true;
		else
			return false;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in EditGuild query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	return false;
}

bool Database::GetGuildNameByID(uint32 guild_id, char * name) {
	if (!name || !guild_id) return false;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "select name from guilds where id='%i'", guild_id), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row[0])
			sprintf(name,"%s",row[0]);
		mysql_free_result(result);
		return true;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in GetGuildNameByID query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	return false;
}

uint32 Database::GetGuildIDbyLeader(uint32 leader)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT id FROM guilds WHERE leader=%i", leader), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1)
		{
			row = mysql_fetch_row(result);
			uint32 tmp = atoi(row[0]);
			mysql_free_result(result);
			return tmp;
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in Getguild_idbyLeader query '%s': %s", query, errbuf);
		safe_delete_array(query);
	}

	return 0;
}

bool Database::SetGuildLeader(uint32 guild_id, uint32 leader)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;

	if (RunQuery(query, MakeAnyLenString(&query, "UPDATE guilds SET leader=%i WHERE id=%i", leader, guild_id), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		if (affected_rows == 1)
			return true;
		else
			return false;
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in SetGuildLeader query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	return false;
}

bool Database::SetGuildMOTD(uint32 guild_id, const char* motd) {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	char* motdbuf = 0;
	uint32 affected_rows = 0;

	motdbuf = new char[(strlen(motd)*2)+3];

	DoEscapeString(motdbuf, motd, strlen(motd)) ;

	if (RunQuery(query, MakeAnyLenString(&query, "Update guilds set motd='%s' WHERE id=%i;", motdbuf, guild_id), errbuf, 0, &affected_rows)) {
		safe_delete_array(query);
		delete motdbuf;
		if (affected_rows == 1)
			return true;
		else
			return false;
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in SetGuildMOTD query '%s': %s", query, errbuf);
		safe_delete_array(query);
		delete motdbuf;
		return false;
	}

	return false;
}

string Database::GetGuildMOTD(uint32 guild_id)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	string motd_str;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT motd FROM guilds WHERE id=%i", guild_id), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			if (row[0])
				motd_str = row[0];
		}
		mysql_free_result(result);
	}
	else {
		LogFile->write(EQEMuLog::Error, "Error in GetGuildMOTD query '%s': %s", query, errbuf);
		safe_delete_array(query);
	}
	return motd_str;
}
*/

