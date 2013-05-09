/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#ifndef GROUPS_H
#define GROUPS_H

#include "../common/types.h"
#include "../common/linked_list.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "entity.h"
#include "mob.h"
#include "../common/features.h"
#include "../common/servertalk.h"

#define MAX_MARKED_NPCS 3

enum { RoleAssist = 1, RoleTank = 2, RolePuller = 4 };

class GroupIDConsumer {
public:
	GroupIDConsumer() { id = 0; }
	GroupIDConsumer(uint32 gid) { id = gid; }
	inline const uint32 GetID()	const { return id; }

protected:
	friend class EntityList;
	//use of this function is highly discouraged
	inline void SetID(uint32 set_id) { id = set_id; }
private:
	uint32 id;
};

class Group : public GroupIDConsumer {
public:
	Group(Mob* leader);
	Group(uint32 gid);
	~Group();

	bool	AddMember(Mob* newmember, const char* NewMemberName = nullptr, uint32 CharacterID = 0);
	void	AddMember(const char* NewMemberName);
	void	SendUpdate(uint32 type,Mob* member);
	void	SendLeadershipAAUpdate();
	void	SendWorldGroup(uint32 zone_id,Mob* zoningmember);
	bool	DelMemberOOZ(const char *Name);
	bool	DelMember(Mob* oldmember,bool ignoresender = false);
	void	DisbandGroup();
	bool	IsGroupMember(Mob* client);
	bool	IsGroupMember(const char *Name);
	bool	Process();
	bool	IsGroup()			{ return true; }
	void	CastGroupSpell(Mob* caster,uint16 spellid);
	void	GroupBardPulse(Mob* caster,uint16 spellid);
	void	SplitExp(uint32 exp, Mob* other);
	void	GroupMessage(Mob* sender,uint8 language,uint8 lang_skill,const char* message);
	void	GroupMessage_StringID(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, uint32 distance = 0);
	uint32	GetTotalGroupDamage(Mob* other);
	void	SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter = nullptr);
	inline	void SetLeader(Mob* newleader){ leader=newleader; };
	inline	Mob* GetLeader(){ return leader; };
	char*	GetLeaderName() { return membername[0]; };
	void	SendHPPacketsTo(Mob* newmember);
	void	SendHPPacketsFrom(Mob* newmember);
	bool	UpdatePlayer(Mob* update);
	void	MemberZoned(Mob* removemob);
	inline	bool IsLeader(Mob* leadertest) { return leadertest==leader; };
	uint8	GroupCount();
	uint32	GetHighestLevel();
	uint32	GetLowestLevel();
	void	QueuePacket(const EQApplicationPacket *app, bool ack_req = true);
	void	TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading);
	uint16	GetAvgLevel();
	bool	LearnMembers();
	void	VerifyGroup();
	void	BalanceHP(int32 penalty);
	void	BalanceMana(int32 penalty);
	void	HealGroup(uint32 heal_amt, Mob* caster);
	inline	void SetGroupAAs(GroupLeadershipAA_Struct *From) { memcpy(&LeaderAbilities, From, sizeof(GroupLeadershipAA_Struct)); }
	inline	void GetGroupAAs(GroupLeadershipAA_Struct *Into) { memcpy(Into, &LeaderAbilities, sizeof(GroupLeadershipAA_Struct)); }
	void	UpdateGroupAAs();
	void	SaveGroupLeaderAA();
	void	MarkNPC(Mob* Target, int Number);
	int8	GetNumberNeedingHealedInGroup(int8 hpr, bool includePets);
	void	DelegateMainTank(const char *NewMainAssistName, uint8 toggle = 0);
	void	DelegateMainAssist(const char *NewMainAssistName, uint8 toggle = 0);
	void	DelegatePuller(const char *NewMainAssistName, uint8 toggle = 0);
	void	UnDelegateMainTank(const char *OldMainAssistName, uint8 toggle = 0);
	void	UnDelegateMainAssist(const char *OldMainAssistName, uint8 toggle = 0);
	void	UnDelegatePuller(const char *OldMainAssistName, uint8 toggle = 0);
	bool	IsNPCMarker(Client *c);
	void	SetGroupAssistTarget(Mob *m);
	void	SetGroupTankTarget(Mob *m);
	void	SetGroupPullerTarget(Mob *m);
	bool	HasRole(Mob *m, uint8 Role);
	void	NotifyAssistTarget(Client *c);
	void	NotifyTankTarget(Client *c);
	void	NotifyPullerTarget(Client *c);
	void	DelegateMarkNPC(const char *NewNPCMarkerName);
	void	UnDelegateMarkNPC(const char *OldNPCMarkerName);
	void	NotifyMainTank(Client *c, uint8 toggle = 0);
	void	NotifyMainAssist(Client *c, uint8 toggle = 0);
	void	NotifyPuller(Client *c, uint8 toggle = 0);
	void	NotifyMarkNPC(Client *c);
	inline	uint32 GetNPCMarkerID() { return NPCMarkerID; }
	void	SetMainTank(const char *NewMainTankName);
	void	SetMainAssist(const char *NewMainAssistName);
	void	SetPuller(const char *NewPullerName);
	const char *GetMainTankName() { return MainTankName.c_str(); }
	const char *GetMainAssistName() { return MainAssistName.c_str(); }
	const char *GetPullerName() { return PullerName.c_str(); }
	void	SetNPCMarker(const char *NewNPCMarkerName);
	void	UnMarkNPC(uint16 ID);
	void	SendMarkedNPCsToMember(Client *c, bool Clear = false);
	inline int GetLeadershipAA(int AAID) { return LeaderAbilities.ranks[AAID]; }
	void	ClearAllNPCMarks();
	void	QueueHPPacketsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app);
	void	ChangeLeader(Mob* newleader);
	const char *GetClientNameByIndex(uint8 index);
	void UpdateXTargetMarkedNPC(uint32 Number, Mob *m);

	Mob* members[MAX_GROUP_MEMBERS];
	char	membername[MAX_GROUP_MEMBERS][64];
	uint8	MemberRoles[MAX_GROUP_MEMBERS];
	bool	disbandcheck;
	bool	castspell;

private:
	Mob*	leader;
	GroupLeadershipAA_Struct LeaderAbilities;
	string	MainTankName;
	string	MainAssistName;
	string	PullerName;
	string	NPCMarkerName;
	uint16	NPCMarkerID;
	uint16	AssistTargetID;
	uint16	TankTargetID;
	uint16	PullerTargetID;
	uint16	MarkedNPCs[MAX_MARKED_NPCS];

};

#endif
