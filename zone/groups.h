/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "common/eq_packet_structs.h"
#include "common/party_state.h"
#include "common/types.h"
#include "zone/mob.h"
#include "zone/xtargetautohaters.h"

#include <type_traits>

class Client;
class EQApplicationPacket;
class Mob;
class ServerPacket;
struct ServerGroupState_Struct;

struct GroupMember : GroupMemberBase<PartyMemberBase> {
	Mob* mob = nullptr;
	bool offline = false;
};

class Group : public GroupBase<GroupMember> {
public:
	explicit Group(uint32 gid);
	explicit Group(Mob* leader);
	~Group() override;

	void	SendLeadershipAAUpdate() const;
	void	ApplyDisbandFromWorld();
	void	RequestDisband() const;
	void	RequestDisband(uint32 requestor_character_id, const std::string& requestor_name, const std::string& target_name = "") const;
	static void RequestAddMember(const Client* requestor, const Mob* target);
	static void RequestFollow(const Client* accepter, const char* inviter_name);
	void	RequestRoleChange(uint32 requestor_character_id, const std::string& requestor_name, uint8 action, const std::string& target_name) const;
	void	RequestMakeLeader(uint32 requestor_character_id, const std::string& requestor_name, const std::string& target_name) const;
	void	RequestMakeLeader(const Mob* target) const;
	void	RequestMentor(uint32 requestor_character_id, bool clear, const std::string& name, int percent) const;
	static void RequestInvite(const GroupInvite_Struct* invite);
	static void RequestCancelInvite(const GroupCancel_Struct* cancel);
	// Explicit desync recovery (/refreshgroup) -- asks world to broadcast a
	// real GroupUpdateType::Full to every zone with a member present. See
	// ServerOP_GroupAction's GroupActionRefresh comment in common/servertalk.h.
	void	RequestRefresh() const;

	[[nodiscard]] bool IsGroupMember(const Mob* c) const;
	[[nodiscard]] bool IsGroupMember(const char* name);
	void	Process();
	void	CastGroupSpell(Mob* caster,int32 spellid) const;
	void	SplitExp(ExpSource exp_source, uint64 exp, Mob* other);
	void	GroupMessage(const Mob* sender,uint8 language,uint8 lang_skill,const char* message) const;
	void	GroupMessageString(const Mob* sender, uint32 type, uint32 string_id, const char* message,
		const char* message2 = nullptr, const char* message3 = nullptr, const char* message4 = nullptr,
		const char* message5 = nullptr, const char* message6 = nullptr, const char* message7 = nullptr,
		const char* message8 = nullptr, const char* message9 = nullptr) const;
	[[nodiscard]] uint32 GetTotalGroupDamage(Mob* other) const;
	void	SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, const Client *splitter = nullptr, bool share = false) const;
	void	SetLeader(Mob* c){ leader = c; };
	[[nodiscard]] Mob* GetLeader() const { return leader; };
	[[nodiscard]] std::string GetLeaderName() const;
	void	SendHPManaEndPacketsTo(Client* target) const;
	void	SendHPPacketsFrom(Mob* source) const;
	void	SendManaPacketFrom(Mob* source) const;
	void	SendEndurancePacketFrom(Mob* source) const;
	void	SendHPManaEndPacketsFrom(Mob* source) const;
	void	UpdatePlayer(Mob* update);
	void	MemberZoned(const Mob* removemob);
	[[nodiscard]] bool IsLeader(const char* name) const;
	[[nodiscard]] bool IsLeader(const Mob* m) const { return m == leader; };
	[[nodiscard]] uint8 GroupCount() const;
	[[nodiscard]] uint32 GetHighestLevel() const;
	[[nodiscard]] uint32 GetLowestLevel() const;
	[[nodiscard]] uint16 GetAvgLevel() const;
	void	TeleportGroup(const Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading) const;
	void	BalanceHP(int32 penalty, float range = 0, const Mob* caster = nullptr, int32 limit = 0) const;
	void	BalanceMana(int32 penalty, float range = 0, const Mob* caster = nullptr, int32 limit = 0) const;
	void	HealGroup(uint32 heal_amt, Mob* caster, float range = 0) const;
	[[nodiscard]] int GetNumberNeedingHealedInGroup(int hpr, bool include_pets) const;
	void	GetGroupAAs(GroupLeadershipAA_Struct *Into) const { memcpy(Into, &leadership_aa, sizeof(GroupLeadershipAA_Struct)); }
	void	RequestGroupAAs() const;
	static void RequestPartyState(const std::string& character_name);
	void	MarkNPC(const Mob* Target, uint32 Number);
	void	UpdateMemberTarget(const Mob* member, const Mob* target);
	bool	HasRole(const Mob *m, uint8 Role) const;
	[[nodiscard]] uint8 GetMemberRole(const Mob* m) const;
	[[nodiscard]] uint8 GetMemberRole(const char* name) const;
	void	NotifyAssistTarget(Client *c) const;
	void	NotifyTankTarget(Client *c) const;
	void	NotifyPullerTarget(Client *c) const;
	[[nodiscard]] const char* GetMainTankName() const { return main_tank_name.c_str(); }
	[[nodiscard]] const char* GetMainAssistName() const { return main_assist_name.c_str(); }
	[[nodiscard]] const char* GetPullerName() const { return puller_name.c_str(); }
	[[nodiscard]] const char* GetMasterLooterName() const { return master_looter_name.c_str(); }
	[[nodiscard]] const char* GetNPCMarkerName() const { return npc_marker_name.c_str(); }
	[[nodiscard]] bool AmIMainTank(const char *mob_name) const;
	[[nodiscard]] bool AmIMainAssist(const char *mob_name) const;
	[[nodiscard]] bool AmIPuller(const char *mob_name) const;
	void	UnMarkNPC(uint16 ID);
	void	NotifyMarkedNPC(Client* c, uint16 mark, bool clear = false) const;
	[[nodiscard]] uint32 GetLeadershipAA(int AAID) const { return leadership_aa.ranks[AAID]; }
	void	ClearAllNPCMarks();
	void	QueueHPPacketsForNPCHealthAA(const Mob* sender, const EQApplicationPacket* app) const;
	void	QueueClientsInRange(const Mob *sender, const EQApplicationPacket *app, bool ack_required = true, bool ignore_sender = true, float distance = 0) const;
	void	QueueClients(const EQApplicationPacket* app, bool ack_required = true) const;
	void	SetDirtyAutoHaters() const;
	void	MergeAutoHaters(XTargetAutoHaters& other) { m_autohatermgr.merge(other); }
	void	DemergeAutoHaters(XTargetAutoHaters& other) { other.demerge(m_autohatermgr); }
	void	IncrementAutoHater(Mob* m) { m_autohatermgr.increment_count(m); }
	void	DecrementAutoHater(Mob* m) { m_autohatermgr.decrement_count(m); }
	[[nodiscard]] const XTargetAutoHaters* GetXTargetAutoMgr() const { return &m_autohatermgr; }
	[[nodiscard]] int GetMentorPercent() const { return mentor_percent; }
	[[nodiscard]] Client* GetMentoree() const { return mentoree; }

	[[nodiscard]] bool AnyMemberHasDzLockout(const std::string& expedition, const std::string& event) const;

	void	SendFullGroupUpdate(Client* client) const;
	static void SendInvite(Client* recipient, const GroupInvite_Struct* invite);
	static void SendCancelInvite(Client* recipient, const GroupCancel_Struct* cancel);
	static void SendLeaderChangePacket(Client* c, const std::string& leader_name);
	static void SendJoinAcknowledge(Client* c);
	// Explicit desync recovery only (/refreshgroup) -- re-applies the whole
	// roster/leader/role/mentor state and notifies every resident client
	// unconditionally. Everything else (zone-in) never calls this.
	void	ApplyFullFromWorld(const ServerGroupState_Struct &gs);
	// The common case: this zone already tracks the group (some other
	// member is already resident), so nothing about its state changed --
	// just resolve this one already-known member's Mob* into their
	// existing roster slot and give them (only them) the current picture.
	void	AttachZonedMember(const std::string &name);
	void	ApplyMemberJoinedFromWorld(const ServerGroupStateMember_Struct &member);
	void	ApplyMemberLeftFromWorld(const std::string &name);
	void	ApplyLeaderChangedFromWorld(const std::string &leader_name);
	void	ApplyRoleChangedFromWorld(uint8 role_id, const std::string &target_name);
	void	ApplyOfflineChangedFromWorld(const std::string &name, bool is_offline);
	void	ApplyMentorChangedFromWorld(const std::string &mentoree_name, int32 mentor_percent);
	void	ApplyAAChangedFromWorld(const GroupLeadershipAA_Struct &new_leader_aa);

	[[nodiscard]] Mob* GetMember(uint32 index) const { return members[index].mob; }
	[[nodiscard]] const char* GetMemberName(uint32 index) const { return members[index].name.c_str(); }
	bool HasMemberNamed(const char* name) const;
	[[nodiscard]] uint8 GetMemberRoles(uint32 index) const { return members[index].roles; }
	[[nodiscard]] bool IsMemberMerc(const std::string& name) const;

	template <typename Fn>
	void ForEachMemberName(Fn &&fn) const
	{
		for (const auto& member : members) {
			if (member.InUse()) {
				if constexpr (std::is_invocable_r_v<bool, Fn, const char*>) {
					if (!fn(member.name.c_str())) return;
				} else {
					fn(member.name.c_str());
				}
			}
		}
	}

	template <typename Fn>
	void ForEachMember(Fn &&fn) const
	{
		for (const auto& member : members) {
			if (member.mob != nullptr) {
				if constexpr (std::is_invocable_r_v<bool, Fn, Mob*>) {
					if (!fn(member.mob)) return;
				} else {
					fn(member.mob);
				}
			}
		}
	}

	template <typename Fn>
	void ForEachClient(Fn &&fn) const
	{
		ForEachMember([&](Mob *m) -> bool {
			if (!m->IsClient()) return true;
			if constexpr (std::is_invocable_r_v<bool, Fn, Client*>) {
				return fn(m->CastToClient());
			} else {
				fn(m->CastToClient());
				return true;
			}
		});
	}

	template <typename Fn>
	void ForEachBot(Fn &&fn) const
	{
		ForEachMember([&](Mob *m) -> bool {
			if (!m->IsBot()) return true;
			if constexpr (std::is_invocable_r_v<bool, Fn, Bot*>) {
				return fn(m->CastToBot());
			} else {
				fn(m->CastToBot());
				return true;
			}
		});
	}

	[[nodiscard]] uint32 GetRosterRevision() const { return roster_revision; }

private:
	Mob*	AddMember(const ServerGroupStateMember_Struct& new_member_def);
	void	RemoveMember(const std::string& name);
	static void	ClearMember(GroupMember& slot);
	[[nodiscard]] bool IsNPCMarked(uint32 EntityID) const;
	[[nodiscard]] uint32 GetNPCMarkerID() const { return NPCMarkerID; }
	[[nodiscard]] Mob* GetMember(const std::string& name) const;
	void	SetGroupAssistTarget(const Mob *m);
	void	SetGroupTankTarget(const Mob *m);
	void	SetGroupPullerTarget(const Mob *m);
	void	SetGroupLeaderRole(const std::string &new_leader_name);
	void	SetMainTank(const char *NewMainTankName);
	void	SetMainAssist(const char *NewMainAssistName);
	void	SetPuller(const char *NewPullerName);
	void	SetNPCMarker(const char *NewNPCMarkerName);
	void	SetMasterLooter(const char *NewMasterLooterName);
	void	NotifyMainTank(Client *c, bool toggle) const;
	void	NotifyMainAssist(Client *c, uint8 toggle) const;
	void	NotifyPuller(Client *c, uint8 toggle) const;
	void	NotifyMarkNPC(Client *c, bool toggle) const;
	void	NotifyMasterLooter(Client *c, bool toggle) const;
	void	UpdateXTargetMarkedNPC(uint32 Number, const Mob *m) const;

	void	SendJoinInfo(Mob* new_member) const;
	void	SendJoinPacket(const Mob* new_member, const ServerGroupStateMember_Struct& new_member_def) const;
	void	SendLeavePacket(const std::string& name) const;
	void	SendLeadershipPacket(const std::string& leader_name) const;
	void	SendGroupRolePacket(Client *c, const char *name, uint8 role_number, bool toggle) const;
	void	SendMarkedNPCsToMember(Client *c, bool Clear = false) const;
	void	SendDisbandGroup() const;

	// leader_name, main_tank_name, main_assist_name, puller_name,
	// npc_marker_name, master_looter_name, mentoree_name, mentor_percent are
	// all inherited (privately) from GroupBase<Group> (common/party_state.h).
	// members itself is not -- see the private GroupMember/members
	// declarations above.
	uint32	roster_revision = 0;

	Mob*	leader = nullptr;
	uint16	NPCMarkerID = 0;
	uint16	AssistTargetID = 0;
	uint16	TankTargetID = 0;
	uint16	PullerTargetID = 0;
	uint16	MarkedNPCs[MAX_MARKED_NPCS] = {};

	Client *mentoree = nullptr;

	XTargetAutoHaters m_autohatermgr;
};
