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
#ifndef EQEMU_DATABASE_H
#define EQEMU_DATABASE_H

#define AUTHENTICATION_TIMEOUT	60
#define INVALID_ID				0xFFFFFFFF

#include "debug.h"
#include "types.h"
#include "dbcore.h"
#include "linked_list.h"
#include "eq_packet_structs.h"
/*#include "EQStream.h"
#include "guilds.h"
#include "MiscFunctions.h"
#include "Mutex.h"
#include "Item.h"
#include "extprofile.h"*/
#include <string>
#include <vector>
#include <map>

//atoi is not uint32 or uint32 safe!!!!
#define atoul(str) strtoul(str, nullptr, 10)

//class Spawn;
class Corpse;
class Spawn2;
class NPC;
class SpawnGroupList;
class Petition;
class Client;
class Merc;
struct Combine_Struct;
//struct Faction;
//struct FactionMods;
//struct FactionValue;
struct ZonePoint;
struct NPCType;
class Inventory;
class ItemInst;

struct EventLogDetails_Struct {
	uint32	id;
	char	accountname[64];
	uint32	account_id;
	int16	status;
	char	charactername[64];
	char	targetname[64];
	char	timestamp[64];
	char	descriptiontype[64];
	char	details[128];
};

struct CharacterEventLog_Struct {
uint32	count;
uint8	eventid;
EventLogDetails_Struct eld[255];
};


// Added By Hogie
// INSERT into variables (varname,value) values('decaytime [minlevel] [maxlevel]','[number of seconds]');
// IE: decaytime 1 54 = Levels 1 through 54
// decaytime 55 100 = Levels 55 through 100
// It will always put the LAST time for the level (I think) from the Database
struct npcDecayTimes_Struct {
	uint16 minlvl;
	uint16 maxlvl;
	uint32 seconds;
};
// Added By Hogie -- End

struct VarCache_Struct {
	char varname[26];	// varname is char(25) in database
	char value[0];
};

struct PlayerProfile_Struct;
struct GuildRankLevel_Struct;
struct GuildRanks_Struct;
struct ExtendedProfile_Struct;
struct GuildMember_Struct;
class PTimerList;

class Database : public DBcore {
public:
	Database();
	Database(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	bool Connect(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	~Database();

	/*
	* General Character Related Stuff
	*/
	bool	MoveCharacterToZone(const char* charname, const char* zonename);
	bool	MoveCharacterToZone(const char* charname, const char* zonename,uint32 zoneid);
	bool	MoveCharacterToZone(uint32 iCharID, const char* iZonename);
	bool	UpdateName(const char* oldname, const char* newname);
	bool	SetHackerFlag(const char* accountname, const char* charactername, const char* hacked);
	bool	SetMQDetectionFlag(const char* accountname, const char* charactername, const char* hacked, const char* zone);
	bool	AddToNameFilter(const char* name);
	bool	ReserveName(uint32 account_id, char* name);
	bool	CreateCharacter(uint32 account_id, char* name, uint16 gender, uint16 race, uint16 class_, uint8 str, uint8 sta, uint8 cha, uint8 dex, uint8 int_, uint8 agi, uint8 wis, uint8 face);
	bool	StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, Inventory* inv, ExtendedProfile_Struct *ext);
	bool	DeleteCharacter(char* name);
	uint8	CopyCharacter(const char* oldname, const char* newname, uint32 acctid);

	/*
	* General Information Getting Queries
	*/
	bool	CheckNameFilter(const char* name, bool surname = false);
	bool	CheckUsedName(const char* name);
	uint32	GetAccountIDByChar(const char* charname, uint32* oCharID = 0);
	uint32	GetAccountIDByChar(uint32 char_id);
	uint32	GetAccountIDByName(const char* accname, int16* status = 0, uint32* lsid = 0);
	uint32	GetGuildDBIDByCharID(uint32 char_id);
	void	GetAccountName(uint32 accountid, char* name, uint32* oLSAccountID = 0);
	void	GetCharName(uint32 char_id, char* name);
	uint32	GetCharacterInfo(const char* iName, uint32* oAccID = 0, uint32* oZoneID = 0, uint32* oInstanceID = 0,float* oX = 0, float* oY = 0, float* oZ = 0);
	uint32	GetCharacterID(const char *name);
	bool	CheckBannedIPs(const char* loginIP); //Lieka Edit: Check incomming connection against banned IP table.
	bool	AddBannedIP(char* bannedIP, const char* notes); //Lieka Edit: Add IP address to the Banned_IPs table.
	bool	CheckGMIPs(const char* loginIP, uint32 account_id);
	bool	AddGMIP(char* ip_address, char* name);
	void	LoginIP(uint32 AccountID, const char* LoginIP);

	/*
	* Instancing Stuff
	*/
	bool VerifyZoneInstance(uint32 zone_id, uint16 instance_id);
	bool VerifyInstanceAlive(uint16 instance_id, uint32 char_id);
	bool CharacterInInstanceGroup(uint16 instance_id, uint32 char_id);
	void DeleteInstance(uint16 instance_id);
	bool CheckInstanceExpired(uint16 instance_id);
	uint32 ZoneIDFromInstanceID(uint16 instance_id);
	uint32 VersionFromInstanceID(uint16 instance_id);
	uint32 GetTimeRemainingInstance(uint16 instance_id, bool &is_perma);
	bool GetUnusedInstanceID(uint16 &instance_id);
	bool CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration);
	void PurgeExpiredInstances();
	bool AddClientToInstance(uint16 instance_id, uint32 char_id);
	bool RemoveClientFromInstance(uint16 instance_id, uint32 char_id);
	bool RemoveClientsFromInstance(uint16 instance_id);
	bool CheckInstanceExists(uint16 instance_id);
	void BuryCorpsesInInstance(uint16 instance_id);
	uint16 GetInstanceVersion(uint16 instance_id);
	uint16 GetInstanceID(const char* zone, uint32 charid, int16 version);
	uint16 GetInstanceID(uint32 zone, uint32 charid, int16 version);
	void AssignGroupToInstance(uint32 gid, uint32 instance_id);
	void AssignRaidToInstance(uint32 rid, uint32 instance_id);
	void FlagInstanceByGroupLeader(uint32 zone, int16 version, uint32 charid, uint32 gid);
	void FlagInstanceByRaidLeader(uint32 zone, int16 version, uint32 charid, uint32 rid);
	void SetInstanceDuration(uint16 instance_id, uint32 new_duration);
	bool GlobalInstance(uint16 instance_id);

	/*
	* Adventure related.
	*/
	void UpdateAdventureStatsEntry(uint32 char_id, uint8 theme, bool win);
	bool GetAdventureStats(uint32 char_id, uint32 &guk_w, uint32 &mir_w, uint32 &mmc_w, uint32 &ruj_w, uint32 &tak_w,
		uint32 &guk_l, uint32 &mir_l, uint32 &mmc_l, uint32 &ruj_l, uint32 &tak_l);

	/*
	* Account Related
	*/
	uint32	GetMiniLoginAccount(char* ip);
	void	GetAccountFromID(uint32 id, char* oAccountName, int16* oStatus);
	uint32	CheckLogin(const char* name, const char* password, int16* oStatus = 0);
	int16	CheckStatus(uint32 account_id);
	uint32	CreateAccount(const char* name, const char* password, int16 status, uint32 lsaccount_id = 0);
	bool	DeleteAccount(const char* name);
	bool	SetAccountStatus(const char* name, int16 status);
	bool	SetLocalPassword(uint32 accid, const char* password);
	uint32	GetAccountIDFromLSID(uint32 iLSID, char* oAccountName = 0, int16* oStatus = 0);
	bool	UpdateLiveChar(char* charname,uint32 lsaccount_id);
	bool	GetLiveChar(uint32 account_id, char* cname);
	uint8	GetAgreementFlag(uint32 acctid);
	void	SetAgreementFlag(uint32 acctid);

	/*
	* Groups
	*/
	uint32	GetGroupID(const char* name);
	void	SetGroupID(const char* name, uint32 id, uint32 charid, uint32 ismerc = false);
	void	ClearGroup(uint32 gid = 0);
	char*	GetGroupLeaderForLogin(const char* name,char* leaderbuf);

	void	SetGroupLeaderName(uint32 gid, const char* name);
	char*	GetGroupLeadershipInfo(uint32 gid, char* leaderbuf, char* maintank = nullptr, char* assist = nullptr, char* puller = nullptr, char *marknpc = nullptr,
						GroupLeadershipAA_Struct* GLAA = nullptr);
	void	ClearGroupLeader(uint32 gid = 0);

	/*
	* Raids
	*/
	void	ClearRaid(uint32 rid = 0);
	void	ClearRaidDetails(uint32 rid = 0);
	uint32	GetRaidID(const char* name);
	const char *GetRaidLeaderName(uint32 rid);

	/*
	* Database Variables
	*/
	bool	GetVariable(const char* varname, char* varvalue, uint16 varvalue_len);
	bool	SetVariable(const char* varname, const char* varvalue);
	bool	LoadVariables();
	uint32	LoadVariables_MQ(char** query);
	bool	LoadVariables_result(MYSQL_RES* result);

	/*
	* General Queries
	*/
	bool	LoadZoneNames();
	bool	GetZoneLongName(const char* short_name, char** long_name, char* file_name = 0, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, uint32* graveyard_id = 0, uint32* maxclients = 0);
	bool	GetZoneGraveyard(const uint32 graveyard_id, uint32* graveyard_zoneid = 0, float* graveyard_x = 0, float* graveyard_y = 0, float* graveyard_z = 0, float* graveyard_heading = 0);
	uint32	GetZoneGraveyardID(uint32 zone_id, uint32 version);
	uint32	GetZoneID(const char* zonename);
	uint8	GetPEQZone(uint32 zoneID, uint32 version);
	const char*	GetZoneName(uint32 zoneID, bool ErrorUnknown = false);
	uint8	GetServerType();
	bool	GetSafePoints(const char* short_name, uint32 version, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, int16* minstatus = 0, uint8* minlevel = 0, char *flag_needed = nullptr);
	bool	GetSafePoints(uint32 zoneID, uint32 version, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, int16* minstatus = 0, uint8* minlevel = 0, char *flag_needed = nullptr) { return GetSafePoints(GetZoneName(zoneID), version, safe_x, safe_y, safe_z, minstatus, minlevel, flag_needed); }
	uint8	GetSkillCap(uint8 skillid, uint8 in_race, uint8 in_class, uint16 in_level);
	uint8	GetRaceSkill(uint8 skillid, uint8 in_race);
	bool	LoadPTimers(uint32 charid, PTimerList &into);
	void	ClearPTimers(uint32 charid);
	void	ClearMerchantTemp();
	void	SetLFP(uint32 CharID, bool LFP);
	void	SetLFG(uint32 CharID, bool LFG);
	void	SetFirstLogon(uint32 CharID, uint8 firstlogon);
	void	SetLoginFlags(uint32 CharID, bool LFP, bool LFG, uint8 firstlogon);
	void	AddReport(std::string who, std::string against, std::string lines);


protected:
	void	HandleMysqlError(uint32 errnum);

private:
	void DBInitVars();

	std::map<uint32,std::string>	zonename_array;

	Mutex				Mvarcache;
	uint32				varcache_max;
	VarCache_Struct**	varcache_array;
	uint32				varcache_lastupdate;
};

bool	FetchRowMap(MYSQL_RES *result, std::map<std::string,std::string> &rowmap);
#endif
