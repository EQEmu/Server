/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
#ifndef RAIDS_H
#define RAIDS_H

#include "../common/types.h"
#include "groups.h"
#include "xtargetautohaters.h"

class Client;
class EQApplicationPacket;
class Mob;

enum {	//raid packet types:
	raidAdd = 0,
	raidRemove2 = 1,	//parameter=0
	raidMemberNameChange	= 2,
	raidRemove1 = 3,	//parameter=0xFFFFFFFF
	raidNoLongerLeader		= 4,
	raidDisband				= 5,
	raidMembers = 6,	//len 395+, details + members list
	raidNoAssignLeadership	= 7,
	raidCreate = 8,		//len 72
	raidUnknown				= 9, // unused?
	raidNoRaid = 10,		//parameter=0
	raidChangeLootType		= 11,
	raidStringID			= 12,
	raidChangeGroupLeader = 13,	//136 raid leader, new group leader, group_id?
	raidSetLeaderAbilities	= 14,	//472
	raidSetLeaderData		= 15,	// 14,15 SoE names, not sure on difference, 14 packet has 0x100 bytes 15 0x214 in addition to raid general
	raidChangeGroup = 16,	//?? len 136 old leader, new leader, 0 (preceeded with a remove2)
	raidLock = 17,		//len 136 leader?, leader, 0
	raidUnlock = 18,		//len 136 leader?, leader, 0
	raidRedStringID			= 19,
	raidSetLeader			= 20,	//len 388, contains 'details' struct without members; also used for "invite to raid"
	raidMakeLeader			= 30,
	raidSetMotd				= 35,
	raidSetNote				= 36,
};


enum { //raid command types
	RaidCommandInviteIntoExisting = 0, //in use
	RaidCommandAcceptInvite = 1, //in use
	RaidCommandInvite = 3, //in use
	RaidCommandDisband = 5, //in use
	RaidCommandMoveGroup = 6, //in use
	RaidCommandRemoveGroupLeader = 7,
	RaidCommandRaidLock = 8, //in use
	RaidCommandRaidUnlock = 9, //in use
	RaidCommandLootType = 20, //in use
	RaidCommandAddLooter = 21, //in use
	RaidCommandRemoveLooter = 22, //in use
	RaidCommandMakeLeader = 30,
	RaidCommandInviteFail = 31, //already in raid, waiting on invite from other raid, etc
	RaidCommandLootType2 = 32, //in use
	RaidCommandAddLooter2 = 33, //in use
	RaidCommandRemoveLooter2 = 34, //in use
	RaidCommandSetMotd = 35,
	RaidCommandSetNote = 36,
};

#define MAX_RAID_GROUPS 12
#define MAX_RAID_MEMBERS 72
const uint32 RAID_GROUPLESS = 0xFFFFFFFF;

struct RaidMember{
	char membername[64];
	Client *member;
	uint32 GroupNumber;
	uint8 _class;
	uint8 level;
	bool IsGroupLeader;
	bool IsRaidLeader;
	bool IsLooter;
};

struct GroupMentor {
	std::string name;
	Client *mentoree;
	int mentor_percent;
};

class Raid : public GroupIDConsumer {
public:
	Raid(Client *nLeader);
	Raid(uint32 raidID);
	~Raid();

	void SetLeader(Client *newLeader) { leader = newLeader; }
	Client* GetLeader() { return leader; }
	bool IsLeader(Client *c) { return leader==c; }
	bool IsLeader(const char* name) { return (strcmp(leadername, name)==0); }
	void SetRaidLeader(const char *wasLead, const char *name);

	bool	Process();
	bool	IsRaid() { return true; }

	void	AddMember(Client *c, uint32 group = 0xFFFFFFFF, bool rleader=false, bool groupleader=false, bool looter=false);
	void	RemoveMember(const char *c);
	void	DisbandRaid();
	void	MoveMember(const char *name, uint32 newGroup);
	void	SetGroupLeader(const char *who, bool glFlag = true);
	Client	*GetGroupLeader(uint32 group_id);
	void	RemoveGroupLeader(const char *who);
	bool	IsGroupLeader(const char *who);
	bool	IsRaidMember(const char *name);
	void	UpdateLevel(const char *name, int newLevel);

	uint32	GetFreeGroup();
	uint8	GroupCount(uint32 gid);
	uint8	RaidCount();
	uint32	GetHighestLevel();
	uint32	GetLowestLevel();
	uint32	GetGroup(const char *name);
	uint32	GetGroup(Client *c);
	uint16	GetAvgLevel();

	uint32	GetLootType() { return LootType; }
	void	ChangeLootType(uint32 type);
	void	AddRaidLooter(const char* looter);
	void	RemoveRaidLooter(const char* looter);

	inline void	SetRaidMOTD(std::string in_motd) { motd = in_motd; };

	//util func
	//keeps me from having to keep iterating through the list
	//when I want lots of data from the same entry
	uint32	GetPlayerIndex(const char *name);
	uint32	GetPlayerIndex(Client *c);
	//for perl interface
	Client *GetClientByIndex(uint16 index);
	const char *GetClientNameByIndex(uint8 index);

	void	LockRaid(bool lockFlag);
	bool	IsLocked() { return locked; }

	//Actual Implementation Stuff

	void	RaidMessageString(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, uint32 distance = 0);
	void	CastGroupSpell(Mob* caster,uint16 spellid, uint32 gid);
	void	SplitExp(uint32 exp, Mob* other);
	uint32	GetTotalRaidDamage(Mob* other);
	void	BalanceHP(int32 penalty, uint32 gid, float range = 0, Mob* caster = nullptr, int32 limit = 0);
	void	BalanceMana(int32 penalty, uint32 gid,  float range = 0, Mob* caster = nullptr, int32 limit = 0);
	void	HealGroup(uint32 heal_amt, Mob* caster, uint32 gid, float range = 0);
	void	SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter = nullptr);
	void	GroupBardPulse(Mob* caster, uint16 spellid, uint32 gid);

	void	TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading, uint32 gid);
	void	TeleportRaid(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading);

	//updates the list of Client* objects based on who's in and not in the zone.
	//also learns raid structure based on db.
	void	SetRaidDetails();
	void	GetRaidDetails();
	void	SaveRaidMOTD();
	bool	LearnMembers();
	void	VerifyRaid();
	void	MemberZoned(Client *c);
	void	SendHPManaEndPacketsTo(Client *c);
	void	SendHPManaEndPacketsFrom(Mob *mob);
	void	SendManaPacketFrom(Mob *mob);
	void	SendEndurancePacketFrom(Mob *mob);
	void	RaidSay(const char *msg, Client *c, uint8 language, uint8 lang_skill);
	void	RaidGroupSay(const char *msg, Client *c, uint8 language, uint8 lang_skill);

	//Packet Functions
	void	SendRaidCreate(Client *to);
	void	SendRaidAdd(const char *who, Client *to);
	void	SendRaidAddAll(const char *who);
	void	SendRaidRemove(const char *who, Client *to);
	void	SendRaidRemoveAll(const char *who);
	void	SendRaidDisband(Client *to);
	void	SendRaidDisbandAll();
	void	SendRaidMove(const char* who, Client *to);
	void	SendRaidMoveAll(const char* who);
	void	SendBulkRaid(Client *to);

	void	GroupUpdate(uint32 gid, bool initial = true);
	void	SendGroupUpdate(Client *to);
	void	SendGroupDisband(Client *to);
	void	SendRaidLock();
	void	SendRaidUnlock();
	void	SendRaidLockTo(Client *c);
	void	SendRaidUnlockTo(Client *c);
	void	SendRaidGroupAdd(const char *who, uint32 gid);
	void	SendRaidGroupRemove(const char *who, uint32 gid);
	void	SendMakeLeaderPacket(const char *who); //30
	void	SendMakeLeaderPacketTo(const char *who, Client *to);
	void	SendMakeGroupLeaderPacketAll();
	void	SendMakeGroupLeaderPacket(const char *who); //13
	void	SendMakeGroupLeaderPacketTo(const char *who, Client *to);
	void	SendRaidMOTD(Client *c);
	void	SendRaidMOTD();
	void	SendRaidMOTDToWorld();

	void	QueuePacket(const EQApplicationPacket *app, bool ack_req = true);

	// Leadership
	void	UpdateGroupAAs(uint32 gid);
	void	SaveGroupLeaderAA(uint32 gid);
	void	UpdateRaidAAs();
	void	SaveRaidLeaderAA();
	void	SendGroupLeadershipAA(Client *c, uint32 gid);
	void	SendGroupLeadershipAA(uint32 gid);
	void	SendAllRaidLeadershipAA();
	void	LoadLeadership();
	inline int GetLeadershipAA(int AAID, uint32 gid = 0)
		{ if (AAID >= 16) return raid_aa.ranks[AAID - 16]; else return group_aa[gid].ranks[AAID]; }
	inline void SetGroupAAs(uint32 gid, GroupLeadershipAA_Struct *glaa)
		{ memcpy(&group_aa[gid], glaa, sizeof(GroupLeadershipAA_Struct)); }
	inline void SetRaidAAs(RaidLeadershipAA_Struct *rlaa)
		{ memcpy(&raid_aa, rlaa, sizeof(RaidLeadershipAA_Struct)); }

	void	SetGroupMentor(uint32 group_id, int percent, char *name);
	void	ClearGroupMentor(uint32 group_id);
	void	CheckGroupMentor(uint32 group_id, Client *c); // this just checks if we should be fixing the pointer in group mentor struct on zone
	inline int GetMentorPercent(uint32 group_id) { return group_mentor[group_id].mentor_percent; }
	inline Client *GetMentoree(uint32 group_id) { return group_mentor[group_id].mentoree; }

	void SetDirtyAutoHaters();
	inline XTargetAutoHaters *GetXTargetAutoMgr() { return &m_autohatermgr; }

	void	QueueClients(Mob *sender, const EQApplicationPacket *app, bool ack_required = true, bool ignore_sender = true, float distance = 0, bool group_only = true);

	RaidMember members[MAX_RAID_MEMBERS];
	char leadername[64];
protected:
	Client *leader;
	bool locked;
	uint16 numMembers;
	uint32 LootType;
	bool disbandCheck;
	bool forceDisband;
	std::string motd;
	RaidLeadershipAA_Struct raid_aa;
	GroupLeadershipAA_Struct group_aa[MAX_RAID_GROUPS];

	GroupMentor group_mentor[MAX_RAID_GROUPS];

	XTargetAutoHaters m_autohatermgr;
};


#endif

