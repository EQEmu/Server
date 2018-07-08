/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "global_define.h"
#include "eqemu_logsys.h"

#include "types.h"
#include "dbcore.h"
#include "linked_list.h"
#include "eq_packet_structs.h"

#include <cmath>
#include <string>
#include <vector>
#include <map>

//atoi is not uint32 or uint32 safe!!!!
#define atoul(str) strtoul(str, nullptr, 10)

class MySQLRequestResult;
class Client;

namespace EQEmu
{
	class InventoryProfile;
}

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

struct npcDecayTimes_Struct {
	uint16 minlvl;
	uint16 maxlvl;
	uint32 seconds;
};


struct VarCache_Struct {
	std::map<std::string, std::string> m_cache;
	uint32 last_update;
	VarCache_Struct() : last_update(0) { }
	void Add(const std::string &key, const std::string &value) { m_cache[key] = value; }
	const std::string *Get(const std::string &key) {
		auto it = m_cache.find(key);
		return (it != m_cache.end() ? &it->second : nullptr);
	}
};

class PTimerList;

#ifdef _WINDOWS
#if _MSC_VER > 1700 // greater than 2012 (2013+)
#	define _ISNAN_(a) std::isnan(a)
#else
#	include <float.h>
#	define _ISNAN_(a) _isnan(a)
#endif
#else
#	define _ISNAN_(a) std::isnan(a)
#endif

class Database : public DBcore {
public:
	Database();
	Database(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	bool Connect(const char* host, const char* user, const char* passwd, const char* database, uint32 port);
	~Database();

	/* Character Creation */

	bool	AddToNameFilter(const char* name);
	bool	CreateCharacter(uint32 account_id, char* name, uint16 gender, uint16 race, uint16 class_, uint8 str, uint8 sta, uint8 cha, uint8 dex, uint8 int_, uint8 agi, uint8 wis, uint8 face);
	bool	DeleteCharacter(char* name);
	bool	MoveCharacterToZone(const char* charname, const char* zonename);
	bool	MoveCharacterToZone(const char* charname, const char* zonename,uint32 zoneid);
	bool	MoveCharacterToZone(uint32 iCharID, const char* iZonename);
	bool	ReserveName(uint32 account_id, char* name);
	bool	SaveCharacterCreate(uint32 character_id, uint32 account_id, PlayerProfile_Struct* pp);
	bool	SetHackerFlag(const char* accountname, const char* charactername, const char* hacked);
	bool	SetMQDetectionFlag(const char* accountname, const char* charactername, const char* hacked, const char* zone);
	bool	StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, EQEmu::InventoryProfile* inv);
	bool	UpdateName(const char* oldname, const char* newname);

	/* General Information Queries */

	bool	AddBannedIP(char* bannedIP, const char* notes); //Add IP address to the Banned_IPs table.
	bool	AddGMIP(char* ip_address, char* name);
	bool	CheckBannedIPs(const char* loginIP); //Check incoming connection against banned IP table.
	bool	CheckGMIPs(const char* loginIP, uint32 account_id);
	bool	CheckNameFilter(const char* name, bool surname = false);
	bool	CheckUsedName(const char* name);

	uint32	GetAccountIDByChar(const char* charname, uint32* oCharID = 0);
	uint32	GetAccountIDByChar(uint32 char_id);
	uint32	GetAccountIDByName(const char* accname, int16* status = 0, uint32* lsid = 0);
	uint32	GetCharacterID(const char *name);
	uint32	GetCharacterInfo(const char* iName, uint32* oAccID = 0, uint32* oZoneID = 0, uint32* oInstanceID = 0, float* oX = 0, float* oY = 0, float* oZ = 0);
	uint32	GetGuildIDByCharID(uint32 char_id);

	void	GetAccountName(uint32 accountid, char* name, uint32* oLSAccountID = 0);
	void	GetCharName(uint32 char_id, char* name);
	void	LoginIP(uint32 AccountID, const char* LoginIP);

	/* Instancing */

	bool AddClientToInstance(uint16 instance_id, uint32 char_id);
	bool CharacterInInstanceGroup(uint16 instance_id, uint32 char_id);
	bool CheckInstanceExists(uint16 instance_id);
	bool CheckInstanceExpired(uint16 instance_id);
	bool CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration);
	bool GetUnusedInstanceID(uint16 &instance_id);
	bool GlobalInstance(uint16 instance_id);
	bool RemoveClientFromInstance(uint16 instance_id, uint32 char_id);
	bool RemoveClientsFromInstance(uint16 instance_id);
	bool VerifyInstanceAlive(uint16 instance_id, uint32 char_id);
	bool VerifyZoneInstance(uint32 zone_id, uint16 instance_id);

	uint16 GetInstanceID(const char* zone, uint32 charid, int16 version);
	uint16 GetInstanceID(uint32 zone, uint32 charid, int16 version);
	uint16 GetInstanceVersion(uint16 instance_id);
	uint32 GetTimeRemainingInstance(uint16 instance_id, bool &is_perma);
	uint32 VersionFromInstanceID(uint16 instance_id);
	uint32 ZoneIDFromInstanceID(uint16 instance_id);

	void AssignGroupToInstance(uint32 gid, uint32 instance_id);
	void AssignRaidToInstance(uint32 rid, uint32 instance_id);
	void BuryCorpsesInInstance(uint16 instance_id);
	void DeleteInstance(uint16 instance_id);
	void FlagInstanceByGroupLeader(uint32 zone, int16 version, uint32 charid, uint32 gid);
	void FlagInstanceByRaidLeader(uint32 zone, int16 version, uint32 charid, uint32 rid);
	void GetCharactersInInstance(uint16 instance_id, std::list<uint32> &charid_list);
	void PurgeExpiredInstances();
	void SetInstanceDuration(uint16 instance_id, uint32 new_duration);

	/* Adventure related. */

	void UpdateAdventureStatsEntry(uint32 char_id, uint8 theme, bool win);
	bool GetAdventureStats(uint32 char_id, AdventureStats_Struct *as);

	/* Account Related */

	bool	DeleteAccount(const char* name);
	bool	GetLiveChar(uint32 account_id, char* cname);
	bool	SetAccountStatus(const char* name, int16 status);
	bool	SetLocalPassword(uint32 accid, const char* password);
	bool	UpdateLiveChar(char* charname, uint32 lsaccount_id);

	int16	CheckStatus(uint32 account_id);

	uint32	CheckLogin(const char* name, const char* password, int16* oStatus = 0);
	uint32	CreateAccount(const char* name, const char* password, int16 status, uint32 lsaccount_id = 0);
	uint32	GetAccountIDFromLSID(uint32 iLSID, char* oAccountName = 0, int16* oStatus = 0);
	uint32	GetMiniLoginAccount(char* ip);
	uint8	GetAgreementFlag(uint32 acctid);

	void	GetAccountFromID(uint32 id, char* oAccountName, int16* oStatus);
	void	SetAgreementFlag(uint32 acctid);
	
	int		GetIPExemption(std::string account_ip);

	int		GetInstanceID(uint32 char_id, uint32 zone_id);


	/* Groups */
	
	char*	GetGroupLeaderForLogin(const char* name,char* leaderbuf);
	char*	GetGroupLeadershipInfo(uint32 gid, char* leaderbuf, char* maintank = nullptr, char* assist = nullptr, char* puller = nullptr, char *marknpc = nullptr, char *mentoree = nullptr, int *mentor_percent = nullptr, GroupLeadershipAA_Struct* GLAA = nullptr);
	
	uint32	GetGroupID(const char* name);
	
	void	ClearGroup(uint32 gid = 0);
	void	ClearGroupLeader(uint32 gid = 0);
	void	SetGroupID(const char* name, uint32 id, uint32 charid, uint32 ismerc = false);
	void	SetGroupLeaderName(uint32 gid, const char* name);

	/* Raids */

	const char *GetRaidLeaderName(uint32 rid);

	uint32	GetRaidID(const char* name);

	void	ClearRaid(uint32 rid = 0);
	void	ClearRaidDetails(uint32 rid = 0);
	void	ClearRaidLeader(uint32 gid = 0xFFFFFFFF, uint32 rid = 0);
	void	GetGroupLeadershipInfo(uint32 gid, uint32 rid, char* maintank = nullptr, char* assist = nullptr, char* puller = nullptr, char *marknpc = nullptr, char *mentoree = nullptr, int *mentor_percent = nullptr, GroupLeadershipAA_Struct* GLAA = nullptr);
	void	GetRaidLeadershipInfo(uint32 rid, char* maintank = nullptr, char* assist = nullptr, char* puller = nullptr, char *marknpc = nullptr, RaidLeadershipAA_Struct* RLAA = nullptr);
	void	SetRaidGroupLeaderInfo(uint32 gid, uint32 rid);

	void    PurgeAllDeletedDataBuckets();

	/* Database Conversions 'database_conversions.cpp' */

	bool	CheckDatabaseConversions();
	bool	CheckDatabaseConvertCorpseDeblob();
	bool	CheckDatabaseConvertPPDeblob();

	/* Database Variables */

	bool	GetVariable(std::string varname, std::string &varvalue);
	bool	SetVariable(const std::string varname, const std::string &varvalue);
	bool	LoadVariables();

	/* General Queries */

	bool	GetSafePoints(const char* short_name, uint32 version, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, int16* minstatus = 0, uint8* minlevel = 0, char *flag_needed = nullptr);
	bool	GetSafePoints(uint32 zoneID, uint32 version, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, int16* minstatus = 0, uint8* minlevel = 0, char *flag_needed = nullptr) { return GetSafePoints(GetZoneName(zoneID), version, safe_x, safe_y, safe_z, minstatus, minlevel, flag_needed); }
	bool	GetZoneGraveyard(const uint32 graveyard_id, uint32* graveyard_zoneid = 0, float* graveyard_x = 0, float* graveyard_y = 0, float* graveyard_z = 0, float* graveyard_heading = 0);
	bool	GetZoneLongName(const char* short_name, char** long_name, char* file_name = 0, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, uint32* graveyard_id = 0, uint32* maxclients = 0);
	bool	LoadPTimers(uint32 charid, PTimerList &into);
	bool	LoadZoneNames();

	const char*	GetZoneName(uint32 zoneID, bool ErrorUnknown = false);

	uint32	GetZoneGraveyardID(uint32 zone_id, uint32 version);
	uint32	GetZoneID(const char* zonename);

	uint8	GetPEQZone(uint32 zoneID, uint32 version);
	uint8	GetRaceSkill(uint8 skillid, uint8 in_race);
	uint8	GetServerType();
	uint8	GetSkillCap(uint8 skillid, uint8 in_race, uint8 in_class, uint16 in_level);

	void	AddReport(std::string who, std::string against, std::string lines);
	struct TimeOfDay_Struct		LoadTime(time_t &realtime);
	bool	SaveTime(int8 minute, int8 hour, int8 day, int8 month, int16 year);
	void	ClearMerchantTemp();
	void	ClearPTimers(uint32 charid);
	void	SetFirstLogon(uint32 CharID, uint8 firstlogon);
	void	SetLFG(uint32 CharID, bool LFG);
	void	SetLFP(uint32 CharID, bool LFP);
	void	SetLoginFlags(uint32 CharID, bool LFP, bool LFG, uint8 firstlogon);

	void	ClearInvSnapshots(bool use_rule = true);

	/* EQEmuLogSys */
	void	LoadLogSettings(EQEmuLogSys::LogSettings* log_settings);

private:
	std::map<uint32,std::string>	zonename_array;

	Mutex Mvarcache;
	VarCache_Struct varcache;

	/* Groups, utility methods. */
	void    ClearAllGroupLeaders();
	void    ClearAllGroups();

	/* Raid, utility methods. */
	void ClearAllRaids();
	void ClearAllRaidDetails();
	void ClearAllRaidLeaders();
};

#endif
