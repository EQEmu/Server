#include <stdlib.h>
#include "lfguild.h"
#include "database.h"
#include "worldserver.h"
#include "../common/StringUtil.h"
#include "../common/packet_dump.h"
#include "../common/rulesys.h"

extern WorldServer *worldserver;
extern Database database;

PlayerLookingForGuild::PlayerLookingForGuild(char *Name, char *Comments, uint32 Level, uint32 Class, uint32 AACount, uint32 Timezone, uint32 TimePosted)
{
	this->Name = Name;
	this->Comments = Comments;
	this->Level = Level;
	this->Class = Class;
	this->AACount = AACount;
	this->TimeZone = Timezone;
	this->TimePosted = TimePosted;
}

GuildLookingForPlayers::GuildLookingForPlayers(char *Name, char *Comments, uint32 FromLevel, uint32 ToLevel, uint32 Classes, uint32 AACount, uint32 Timezone, uint32 TimePosted)
{
	this->Name = Name;
	this->Comments = Comments;
	this->FromLevel = FromLevel;
	this->ToLevel = ToLevel;
	this->Classes = Classes;
	this->AACount = AACount;
	this->TimeZone = Timezone;
	this->TimePosted = TimePosted;
}

bool LFGuildManager::LoadDatabase()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!database.RunQuery(query,MakeAnyLenString(&query, "SELECT `type`,`name`,`comment`, `fromlevel`, `tolevel`, `classes`, `aacount`, `timezone`, `timeposted` FROM `lfguild`"),errbuf,&result)){

		_log(QUERYSERV__ERROR, "Failed to load LFGuild info from database. %s %s", query, errbuf);
		safe_delete_array(query);

		return false;
	}

	safe_delete_array(query);

	while((row = mysql_fetch_row(result))) {

		uint32 type = atoul(row[0]);
		if(type == 0)
		{
			PlayerLookingForGuild p(row[1], row[2], atoul(row[3]), atoul(row[5]), atoul(row[6]), atoul(row[7]), atoul(row[8]));
			Players.push_back(p);
		}
		else
		{
			GuildLookingForPlayers g(row[1], row[2], atoul(row[3]), atoul(row[4]), atoul(row[5]), atoul(row[6]), atoul(row[7]), atoul(row[8]));
			Guilds.push_back(g);
		}
	}

	mysql_free_result(result);

	return true;
}

void LFGuildManager::HandlePacket(ServerPacket *pack)
{
	char From[64];

	pack->SetReadPosition(0);
	uint32 FromZoneID = pack->ReadUInt32();
	uint32 FromInstanceID = pack->ReadUInt32();
	pack->ReadString(From);
	pack->ReadSkipBytes(4);
	uint32 SubType = pack->ReadUInt32();

	switch(SubType)
	{
		case QSG_LFGuild_PlayerMatches:
		{
			uint32 FromLevel = pack->ReadUInt32();
			uint32 ToLevel = pack->ReadUInt32();
			uint32 MinAA = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();
			uint32 Classes = pack->ReadUInt32();

			SendPlayerMatches(FromZoneID, FromInstanceID, From, FromLevel, ToLevel, MinAA, TimeZone, Classes);
			break;
		}

		case QSG_LFGuild_UpdatePlayerInfo:
		{
			char Comments[257];
			uint32 Class = pack->ReadUInt32();
			uint32 Level = pack->ReadUInt32();
			uint32 AAPoints = pack->ReadUInt32();
			pack->ReadString(Comments);
			uint32 Toggle = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();

			TogglePlayer(FromZoneID, FromInstanceID, From, Class, Level, AAPoints, Comments, Toggle, TimeZone);

			break;
		}
		case QSG_LFGuild_RequestPlayerInfo:
		{
			SendPlayerStatus(FromZoneID, FromInstanceID, From);
			break;
		}
		case QSG_LFGuild_UpdateGuildInfo:
		{
			char GuildName[33], Comments[257];

			pack->ReadString(GuildName);
			pack->ReadString(Comments);

			uint32 FromLevel = pack->ReadUInt32();
			uint32 ToLevel = pack->ReadUInt32();
			uint32 Classes = pack->ReadUInt32();
			uint32 AACount = pack->ReadUInt32();
			uint32 Toggle = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();

			ToggleGuild(FromZoneID, FromInstanceID, From, GuildName, Comments, FromLevel, ToLevel, Classes, AACount, Toggle, TimeZone);


			break;
		}
		case QSG_LFGuild_GuildMatches:
		{
			uint32 Level = pack->ReadUInt32();
			uint32 AAPoints = pack->ReadUInt32();
			uint32 TimeZone = pack->ReadUInt32();
			uint32 Class = pack->ReadUInt32();

			SendGuildMatches(FromZoneID, FromInstanceID, From, Level, AAPoints, TimeZone, Class);
			break;
		}
		case QSG_LFGuild_RequestGuildInfo:
		{
			char GuildName[33];
			pack->ReadString(GuildName);

			SendGuildStatus(FromZoneID, FromInstanceID, From, GuildName);
			break;
		}

		default:
			break;
	}


}

void LFGuildManager::SendPlayerMatches(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 FromLevel, uint32 ToLevel, uint32 MinAA, uint32 TimeZone, uint32 Classes)
{
	std::list<PlayerLookingForGuild>::iterator it;
	std::list<PlayerLookingForGuild> Matches;

	uint32 PacketSize = strlen(From) + 21, NumberOfMatches = 0;

	for(it = Players.begin(); it != Players.end(); ++it)
	{
		uint32 bitmask = 1 << (*it).Class ;

		if(((*it).Level >= FromLevel) && ((*it).Level <= ToLevel) && ((*it).AACount >= MinAA) && (bitmask & Classes) && ((TimeZone == 0xFFFFFFFF) || (TimeZone == (*it).TimeZone)))
		{
			++NumberOfMatches;
			Matches.push_back(*it);
			PacketSize += (*it).Name.length() + (*it).Comments.length() + 18;
		}

	}

	ServerPacket *pack = new ServerPacket(ServerOP_QueryServGeneric, PacketSize);

	pack->WriteUInt32(FromZoneID);
	pack->WriteUInt32(FromInstanceID);
	pack->WriteString(From);
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_PlayerMatches);
	pack->WriteUInt32(NumberOfMatches);

	for(it = Matches.begin(); it != Matches.end(); ++it)
	{
		pack->WriteString((*it).Name.c_str());
		pack->WriteString((*it).Comments.c_str());
		pack->WriteUInt32((*it).Level);
		pack->WriteUInt32((*it).Class);
		pack->WriteUInt32((*it).AACount);
		pack->WriteUInt32((*it).TimeZone);

	}
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void LFGuildManager::SendGuildMatches(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 Level, uint32 AAPoints, uint32 TimeZone, uint32 Class)
{
	std::list<GuildLookingForPlayers>::iterator it;
	std::list<GuildLookingForPlayers> Matches;

	uint32 bitmask = 1 << Class ;

	uint32 PacketSize = strlen(From) + 21, NumberOfMatches = 0;

	for(it = Guilds.begin(); it != Guilds.end(); ++it)
	{
		if((Level >= (*it).FromLevel) && (Level <= (*it).ToLevel) && (AAPoints >= (*it).AACount) && (bitmask & (*it).Classes) && (((*it).TimeZone == 0xFFFFFFFF) || (TimeZone == 0xFFFFFFFF) || (TimeZone == (*it).TimeZone)))
		{
			++NumberOfMatches;
			Matches.push_back(*it);
			PacketSize += (*it).Name.length() + (*it).Comments.length() + 6;
		}

	}

	ServerPacket *pack = new ServerPacket(ServerOP_QueryServGeneric, PacketSize);

	pack->WriteUInt32(FromZoneID);
	pack->WriteUInt32(FromInstanceID);
	pack->WriteString(From);
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_GuildMatches);
	pack->WriteUInt32(NumberOfMatches);

	for(it = Matches.begin(); it != Matches.end(); ++it)
	{
		pack->WriteString((*it).Name.c_str());
		pack->WriteUInt32((*it).TimeZone);
		pack->WriteString((*it).Comments.c_str());
	}
	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void LFGuildManager::TogglePlayer(uint32 FromZoneID, uint32 FromInstanceID, char *From, uint32 Class, uint32 Level, uint32 AAPoints, char *Comments, uint32 Toggle, uint32 TimeZone)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	std::list<PlayerLookingForGuild>::iterator it;

	for(it = Players.begin(); it != Players.end(); ++it)
	{
		if(!strcasecmp((*it).Name.c_str(), From))
		{
			Players.erase(it);

			break;
		}
	}

	if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `lfguild` WHERE `type` = 0 AND `name` = '%s'", From), errbuf, 0, 0))
		_log(QUERYSERV__ERROR, "Error removing player from LFGuild table, query was %s, %s", query, errbuf);

	safe_delete_array(query);

	uint32 Now = time(nullptr);

	if(Toggle == 1)
	{
		PlayerLookingForGuild p(From, Comments, Level, Class, AAPoints, TimeZone, Now);
		Players.push_back(p);
		if(!database.RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `lfguild` (`type`, `name`, `comment`, `fromlevel`, `tolevel`, `classes`, `aacount`, `timezone`, `timeposted`) VALUES(0, '%s', '%s', %u, 0, %u, %u, %u, %u)", From, Comments, Level, Class, AAPoints, TimeZone, Now), errbuf, 0, 0))
			_log(QUERYSERV__ERROR, "Error inserting player into LFGuild table, query was %s, %s", query, errbuf);

		safe_delete_array(query);

	}

	ServerPacket *pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(From) + strlen(Comments) + 30);

	pack->WriteUInt32(FromZoneID);
	pack->WriteUInt32(FromInstanceID);
	pack->WriteString(From);
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_RequestPlayerInfo);
	pack->WriteString(Comments);
	pack->WriteUInt32(TimeZone);
	pack->WriteUInt32(Now);
	pack->WriteUInt32(Toggle);

	worldserver->SendPacket(pack);
	safe_delete(pack);

}

void LFGuildManager::ToggleGuild(uint32 FromZoneID, uint32 FromInstanceID, char *From, char* GuildName, char *Comments, uint32 FromLevel, uint32 ToLevel, uint32 Classes, uint32 AACount, uint32 Toggle, uint32 TimeZone)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	std::list<GuildLookingForPlayers>::iterator it;

	for(it = Guilds.begin(); it != Guilds.end(); ++it)
	{
		if(!strcasecmp((*it).Name.c_str(), GuildName))
		{
			Guilds.erase(it);
			break;
		}
	}

	if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE FROM `lfguild` WHERE `type` = 1 AND `name` = '%s'", GuildName), errbuf, 0, 0))
		_log(QUERYSERV__ERROR, "Error removing guild from LFGuild table, query was %s, %s", query, errbuf);

	safe_delete_array(query);

	uint32 Now = time(nullptr);

	if(Toggle == 1)
	{
		GuildLookingForPlayers g(GuildName, Comments, FromLevel, ToLevel, Classes, AACount, TimeZone, Now);
		Guilds.push_back(g);
		if(!database.RunQuery(query, MakeAnyLenString(&query, "INSERT INTO `lfguild` (`type`, `name`, `comment`, `fromlevel`, `tolevel`, `classes`, `aacount`, `timezone`, `timeposted`) VALUES(1, '%s', '%s', %u, %u, %u, %u, %u, %u)", GuildName, Comments, FromLevel, ToLevel, Classes, AACount, TimeZone, Now), errbuf, 0, 0))
			_log(QUERYSERV__ERROR, "Error inserting guild into LFGuild table, query was %s, %s", query, errbuf);

		safe_delete_array(query);

	}
	ServerPacket *pack = new ServerPacket(ServerOP_LFGuildUpdate, strlen(GuildName) + strlen(Comments) + 30);

	pack->WriteString(GuildName);
	pack->WriteString(Comments);
	pack->WriteUInt32(FromLevel);
	pack->WriteUInt32(ToLevel);
	pack->WriteUInt32(Classes);
	pack->WriteUInt32(AACount);
	pack->WriteUInt32(TimeZone);
	pack->WriteUInt32(Now);
	pack->WriteUInt32(Toggle);

	worldserver->SendPacket(pack);
	safe_delete(pack);
}

void LFGuildManager::ExpireEntries()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;

	std::list<PlayerLookingForGuild>::iterator it;
	std::list<GuildLookingForPlayers>::iterator it2;

	for(it = Players.begin(); it != Players.end(); ++it)
	{
		if((*it).TimePosted + 604800 <= (uint32)time(nullptr))
		{
			if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE from `lfguild` WHERE `type` = 0 AND `name` = '%s'", (*it).Name.c_str()), errbuf, 0, 0))
				_log(QUERYSERV__ERROR, "Error expiring player LFGuild entry, query was %s, %s", query, errbuf);

			safe_delete_array(query);

			it = Players.erase(it);
		}
	}

	for(it2 = Guilds.begin(); it2 != Guilds.end(); ++it2)
	{
		if((*it2).TimePosted + 2592000 <= time(nullptr))
		{
			if(!database.RunQuery(query, MakeAnyLenString(&query, "DELETE from `lfguild` WHERE `type` = 1 AND `name` = '%s'", (*it2).Name.c_str()), errbuf, 0, 0))
				_log(QUERYSERV__ERROR, "Error removing guild LFGuild entry, query was %s, %s", query, errbuf);

			safe_delete_array(query);

			it2 = Guilds.erase(it2);
		}
	}
}

void LFGuildManager::SendPlayerStatus(uint32 FromZoneID, uint32 FromInstanceID, char *From)
{

	std::list<PlayerLookingForGuild>::iterator it;

	for(it = Players.begin(); it != Players.end(); ++it)
	{
		if(!strcasecmp((*it).Name.c_str(), From))
		{
			ServerPacket *pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(From) + (*it).Comments.length() + 30);

			pack->WriteUInt32(FromZoneID);
			pack->WriteUInt32(FromInstanceID);
			pack->WriteString(From);
			pack->WriteUInt32(QSG_LFGuild);
			pack->WriteUInt32(QSG_LFGuild_RequestPlayerInfo);
			pack->WriteString((*it).Comments.c_str());
			pack->WriteUInt32((*it).TimeZone);
			pack->WriteUInt32((*it).TimePosted);
			pack->WriteUInt32(1);

			worldserver->SendPacket(pack);
			safe_delete(pack);
			break;
		}
	}
}

void LFGuildManager::SendGuildStatus(uint32 FromZoneID, uint32 FromInstanceID, char *From, char *GuildName)
{
	std::list<GuildLookingForPlayers>::iterator it;

	for(it = Guilds.begin(); it != Guilds.end(); ++it)
	{
		if(!strcasecmp((*it).Name.c_str(), GuildName))
		{
			ServerPacket *pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(From) + (*it).Comments.length() + 42);

			pack->WriteUInt32(FromZoneID);
			pack->WriteUInt32(FromInstanceID);
			pack->WriteString(From);
			pack->WriteUInt32(QSG_LFGuild);
			pack->WriteUInt32(QSG_LFGuild_RequestGuildInfo);
			pack->WriteString((*it).Comments.c_str());
			pack->WriteUInt32((*it).FromLevel);
			pack->WriteUInt32((*it).ToLevel);
			pack->WriteUInt32((*it).Classes);
			pack->WriteUInt32((*it).AACount);
			pack->WriteUInt32((*it).TimeZone);
			pack->WriteUInt32((*it).TimePosted);

			worldserver->SendPacket(pack);
			safe_delete(pack);
			break;
		}
	}
}

