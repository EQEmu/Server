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

#include "common/party_state.h"
#include "common/servertalk.h" // RaidActionType, used by the RequestAction()/SendRaidAction() template declarations below
#include "common/types.h"
#include "zone/groups.h"
#include "zone/xtargetautohaters.h"
#include "zone/client.h"

class Client;
class EQApplicationPacket;
class Mob;
class ServerPacket;
struct ServerRaidFull_Struct;
struct ServerRaidMemberJoined_Struct;
struct ServerRaidMemberLeft_Struct;
struct ServerRaidMemberFieldsChanged_Struct;
struct ServerRaidGroupMoved_Struct;
struct ServerRaidLeaderChanged_Struct;
struct ServerRaidGroupLeaderChanged_Struct;
struct ServerRaidStateChanged_Struct;

// DelegateAbility values for OP_RaidDelegateAbility packet
enum RaidDelegateAbility {
	RaidDelegateMainAssist   = 3,
	RaidDelegateMainMarker   = 4,
	RaidDelegateMasterLooter = 5
};

// 0-indexed slots into main_assist_names[]/main_marker_names[] and their
// MainAssist/Marker*Update arrays. The wire's main_assist_number/mark_target
// fields are 1-indexed (slot + 1), not a separate named constant -- there's
// only one slot value per role, not two.
enum MainAssistSlot {
	MAIN_ASSIST_1_SLOT = 0,
	MAIN_ASSIST_2_SLOT = 1,
	MAIN_ASSIST_3_SLOT = 2,
};

enum MainMarkerSlot {
	MAIN_MARKER_1_SLOT = 0,
	MAIN_MARKER_2_SLOT = 1,
	MAIN_MARKER_3_SLOT = 2,
};

// Action value for DelegateAbility_Struct::Action (OP_RaidDelegateAbility).
enum {
	ClearDelegate = 1,
	SetDelegate = 0,
};

struct RaidMember : public RaidMemberBase<PartyMemberBase> {
	Mob* member = nullptr;
	uint32 group_number = RAID_GROUPLESS;
	bool is_group_leader = false;
	bool offline = false;
};

class Raid : public RaidBase<RaidMember> {
public:
	using RaidBase::GetID;

	explicit Raid(Client *nLeader);
	explicit Raid(uint32 raidID);
	~Raid() override;

	// leader is Client*, not Mob*: no /bot command lets a bot owner make
	// their bot issue a raid/group request as itself -- every raid action
	// always originates from a real Client -- so a bot becoming raid leader
	// is never legitimate. World rejects RaidLeadershipMakeLeader when the
	// target is a bot (see world/raid_state.cpp), so this field should never
	// need to hold anything but a Client*; keep it that way rather than
	// widening to Mob* and pushing the IsClient() check onto every caller.
	void SetLeader(Client* c) { leader = c; }
	[[nodiscard]] Client* GetLeader() const { return leader; }
	[[nodiscard]] std::string GetLeaderName() const { return leader_name; }
	bool IsLeader(const Client* c) const { return c == leader; }
	bool IsLeader(const char* name) const { return Strings::EqualFold(leader_name, name); }

	void Process();

	void	RaidMessage(const char* msg, const char* from, uint8 language, uint8 lang_skill) const;
	[[nodiscard]] bool IsEngaged() const;

	// Mirrors Group::RequestInvite(const GroupInvite_Struct*), but takes the
	// inviter directly and derives class/level from it internally -- callers
	// never touch ServerRaidActionInvite_Struct or its individual fields.
	// invitee_name is the only thing identifying the target: whether it's a
	// bot (and if so, its owner/class/level) is resolved by world itself
	// from the bot_data table, not by zone -- see
	// ServerRaidActionInvite_Struct's comment in common/servertalk.h. No
	// live Raid* needed -- the invitee/inviter may not be in a raid yet.
	static void RequestInvite(const Client* inviter, const std::string& invitee_name);

	// Mirrors Group::RequestFollow(accepter, inviter_name): a client
	// accepting a raid invite. inviter_name is identity only -- neither the
	// accepter's nor the inviter's class/level travels on the wire, world
	// resolves both from character_data itself (see
	// ServerRaidActionAcceptInvite_Struct's comment in common/servertalk.h).
	// No live Raid* needed, the accepter isn't in one yet.
	static void RequestFollow(const Client* accepter, const std::string& inviter_name);

	// Mirrors Group::ApplyDisbandFromWorld(): clears every member's IsRaided()
	// and calls entity_list.RemoveRaid(GetID()) itself -- the one true
	// chokepoint for tearing down a live raid, called only from world's
	// RaidUpdateType::Disbanded push (zone/worldserver.cpp) or locally via
	// Raid::~Raid()'s own leftover-subgroup cleanup path. Unlike the old
	// shape, this does NOT
	// disband subgroups itself -- world already independently pushes a
	// normal GroupUpdateType::Disbanded for each subgroup (via
	// GroupStateManager::DisbandAllSubgroups) whenever a raid disbands, so
	// zone-side subgroup teardown happens through the ordinary
	// Group::ApplyDisbandFromWorld() path, not a second, redundant local call
	// here. EntityList::RemoveRaid() itself no longer calls this (mirrors
	// EntityList::RemoveGroup() never calling Group::ApplyDisbandFromWorld()) --
	// removal-from-tracking and the full disband notification are two
	// different directions of the same relationship now, not the same call.
	void	ApplyDisbandFromWorld();

	// Leadership/role requests -- one narrow method per verb, mirroring
	// Group's RequestRoleChange/RequestMakeLeader/etc. Each one builds
	// whichever specific ServerRaidAction*_Struct its action needs and
	// calls the private RequestAction()/SendRaidAction() templates -- callers
	// never touch a wire struct directly, building one outside Raid would
	// leak the wire protocol's shape into caller code.
	// No RequestRemoveGroupLeader -- RaidLeadershipRemoveGroupLeader is
	// retired/a confirmed no-op on live (see its comment in
	// world/raid_state.cpp); RaidCommandRemoveGroupLeader's client_packet.cpp
	// handler sends nothing at all rather than round-tripping a request the
	// server would just discard.
	// Every requestor is a live Client* -- id/name are pulled from it
	// internally (no caller-side CharacterID()/GetName() calls, and no extra
	// world-side lookup needed to resolve who's asking).
	void	RequestLock(const Client* requestor) const;
	void	RequestUnlock(const Client* requestor) const;
	void	RequestSetLootType(const Client* requestor, uint32 loot_type) const;
	// adding: true for RaidCommandAddLooter[2], false for RaidCommandRemoveLooter[2].
	void	RequestSetLooter(const Client* requestor, const std::string& target_name, bool adding) const;
	// new_leader_raid_aa is resolved internally from GetLeader() (matches
	// RequestRaidAAs()'s pattern) -- callers never need to snapshot it themselves.
	void	RequestMakeLeader(const Client* requestor, const std::string& target_name) const;
	void	RequestSetMotd(const Client* requestor, const std::string& new_motd) const;
	void	RequestSetNote(const Client* requestor, const std::string& target_name, const std::string& note) const;
	void	RequestSetGroupOnInvite(const Client* requestor, bool enabled) const;
	// Toggle-style, matching the client's single OP_RaidDelegateAbility button
	// per role (server decides delegate-vs-undelegate from current state) --
	// RaidRoleUndelegate* is never sent by any zone code, so no separate
	// Request*/Undelegate method exists for it. Mirrors
	// Group::RequestRoleChange's convention -- action is the resolved
	// zone->world wire action (RaidRoleDelegateAssist/DelegateMark/
	// DelegateMasterLooter), not the client's own DelegateAbility_Struct
	// value; the caller (client_packet.cpp) maps one to the other, same as
	// it already does for Group's DelegateAbility_Struct handling.
	void	RequestDelegateRole(const Client* requestor, RaidActionType action, const std::string& target_name) const;

	// Remove a member from the raid (RaidCommandDisband despite the name --
	// it's a per-member kick, not Raid::RequestDisband()'s whole-raid
	// teardown).
	void	RequestRemoveMember(const Client* requestor, const std::string& target_name) const;
	// Move a member to a different raid subgroup.
	void	RequestMoveGroup(const Client* requestor, const std::string& target_name, uint32 dest_group_number) const;

	// World->zone pushes, applied thinly (mutate local state + fire the
	// client-facing packet(s), no further round-trip). Mirrors Group's
	// Apply*FromWorld naming. Dispatched from zone/worldserver.cpp's
	// ServerOP_RaidUpdate handler, keyed on RaidUpdateType (see
	// common/servertalk.h).
	void	ApplyFullFromWorld(const ServerRaidFull_Struct &rs);
	void	ApplyMemberJoinedFromWorld(const ServerRaidMemberJoined_Struct &msg);
	void	ApplyMemberLeftFromWorld(const ServerRaidMemberLeft_Struct &msg);
	void	ApplyMemberFieldsChangedFromWorld(const ServerRaidMemberFieldsChanged_Struct &msg);
	void	ApplyGroupMovedFromWorld(const ServerRaidGroupMoved_Struct &msg);
	void	ApplyLeaderChangedFromWorld(const ServerRaidLeaderChanged_Struct &msg);
	void	ApplyGroupLeaderChangedFromWorld(const ServerRaidGroupLeaderChanged_Struct &msg);
	void	ApplyStateChangedFromWorld(const ServerRaidStateChanged_Struct &msg);
	// Narrow AA-only push -- mirrors Group::ApplyAAChangedFromWorld exactly.
	void	ApplyAAChangedFromWorld(const RaidLeadershipAA_Struct &new_leader_aa);
	// Narrow role-delta push (main assist/marker/master looter changed) --
	// mirrors Group::ApplyRoleChangedFromWorld, plus a slot index (raid's
	// assist/marker roles are multi-slot where Group's are singular) and an
	// explicit clearing flag (target_name always names the affected member,
	// even when clearing -- see ServerRaidRoleChanged_Struct's comment).
	void	ApplyRoleChangedFromWorld(uint8 role_id, uint8 slot, const std::string &target_name, bool clearing);

	[[nodiscard]] Group* GetGroupObject(uint32 group_number) const;
	void	SetSubgroupId(uint32 group_number, uint32 gid) { if (group_number < MAX_RAID_GROUPS) subgroup_group_ids[group_number] = gid; }
	bool	IsGroupLeader(const char* name) const;
	bool	IsGroupLeader(const Client *c) const;
	bool	IsRaidMember(const char* name) const;
	bool	IsRaidMember(const Client *c) const;
	void	RequestUpdateLevel(const Client *requestor, const std::string& target_name, int level) const;
	bool    IsAssister(const char* who) const;
	bool    IsMarker(const char* who) const;

	[[nodiscard]] uint8		RaidCount() const;
	[[nodiscard]] uint32	GetHighestLevel() const;
	[[nodiscard]] uint32	GetLowestLevel() const;
	[[nodiscard]] uint32	GetGroupNumber(const char *name) const;
	[[nodiscard]] uint32	GetGroupNumber(const Client *c) const;
	[[nodiscard]] uint16	GetAvgLevel() const;

	[[nodiscard]] uint32	  GetLootType() const { return loot_type; }
	[[nodiscard]] const char* GetRaidMasterLooter() const;
	// slot is 0-indexed (MainAssistSlot/MainMarkerSlot above) -- caller's
	// responsibility to bounds-check, same convention as GetMemberByIndex.
	[[nodiscard]] const std::string& GetMainAssistName(uint8 slot) const { return main_assist_names[slot]; }

	inline void	SetRaidMOTD(const std::string& in_motd) { motd = in_motd; };

	//for perl interface
	[[nodiscard]] Client *GetClientByIndex(uint16 index) const;
	[[nodiscard]] const char *GetClientNameByIndex(uint8 index) const;
	// Bounds-checked by caller, same convention as the two above -- for
	// Lua/Perl's index-based member accessors.
	[[nodiscard]] const RaidMember& GetMemberByIndex(uint16 index) const { return members[index]; }

	[[nodiscard]] bool IsLocked() const { return locked; }

	//Actual Implementation Stuff
	void	RaidMessageString(const Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2=nullptr,const char* message3=nullptr,const char* message4=nullptr,const char* message5=nullptr,const char* message6=nullptr,const char* message7=nullptr,const char* message8=nullptr,const char* message9=nullptr, uint32 distance = 0) const;
	// Zone-local only, same treatment as Group's assist/tank/puller targets --
	// never reported to world, see marked_npcs' own comment. Not const:
	// mutates marked_npcs directly (matches Group's SendJoinInfo precedent --
	// no const_cast, the method just isn't const). Same leader-or-delegated-
	// marker permission check as ClearNPCMarks() -- requester identifies who's
	// asking, same as that method.
	void MarkNPC(Client* requester, const Mob* target, uint32 number);
	void ClearNPCMarks(Client* requester);
	void	SplitExp(ExpSource exp_source, uint64 exp, Mob* other);
	uint32	GetTotalRaidDamage(Mob* other) const;

	void	TeleportRaid(const Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading);

	// is_new_member distinguishes a genuine new-roster-slot join (fires
	// OnAddToRaid) from every other caller -- cold bootstrap, zone-in
	// re-resolve, /refreshraid-style full rebuild (fires OnZonedInWithRaid
	// instead, since membership itself isn't changing). Defaults to false;
	// only ApplyMemberJoinedFromWorld passes true.
	void	AttachLocalMember(RaidMember &m, bool is_new_member = false);
	void	MemberZonedIn(const Client *c);
	void	MemberZoned(const Mob* removemob);

	// Both halves of the older, coarser zone<->world raid sync mechanism --
	// ServerOP_RaidMemberUpsert/RaidState and their Apply*FromWire methods --
	// were confirmed dead (no sender anywhere for either opcode) and have
	// been removed. Everything they used to cover is already handled by the
	// precise, per-purpose ServerOP_RaidUpdate variants above
	// (ApplyMemberJoinedFromWorld/ApplyMemberFieldsChangedFromWorld/
	// ApplyGroupMovedFromWorld/ApplyLeaderChangedFromWorld/etc.) and
	// ApplyFullFromWorld() for the state case.
	void	RaidMessage(const char *msg, const Client *c, uint8 language, uint8 lang_skill) const;
	void    UpdateXTargetType(XTargetType Type, const Mob* m, const char* name = (const char*)nullptr) const;
	void	UpdateXTargetMarkedNPC(uint32 number, const Mob* m) const;
	void    UpdateXtargetMarkedNPC() const;
	void	UpdateRaidXTargets() const;

	//Packet Functions
	// Only what's actually called from outside Raid (client_packet.cpp,
	// worldserver.cpp, mob.cpp) lives here, mirroring Group's convention
	// (SendFullGroupUpdate/SendInvite/etc. public, SendJoinPacket/
	// SendLeavePacket/etc. private) -- everything else moved to the private
	// section below.
	void	SendRaidAddAll(const char *who) const;
	void	SendClassLevelUpdateAll(const char *who) const;
	void	SendRaidGroupChangeAll(const char *who, uint32 new_group) const;
	void    SendRaidAssistTarget() const;

	void	QueueClients(const EQApplicationPacket *app, bool ack_req = true) const;

	// Leadership
	// Mirrors Group::RequestGroupAAs() exactly: snapshots the leader's live
	// AA ranks and reports the fact to world via ServerOP_RaidAction
	// (RaidLeadershipUpdateAA), which relays the narrow AAChanged push back
	// out to every zone (including this one) via ApplyAAChangedFromWorld.
	void	RequestRaidAAs() const;
	void	SendLeadershipAAUpdate() const;
	[[nodiscard]] uint32 GetLeadershipAA(int AAID) const;
	void SetRaidAAs(const RaidLeadershipAA_Struct* rlaa) { memcpy(&leadership_aa, rlaa, sizeof(RaidLeadershipAA_Struct)); }

	void SetDirtyAutoHaters() const;
	// See Group::MergeAutoHaters/DemergeAutoHaters/IncrementAutoHater/
	// DecrementAutoHater's comments (zone/groups.h) -- same reasoning here.
	void	MergeAutoHaters(XTargetAutoHaters& other) { m_autohatermgr.merge(other); }
	void	DemergeAutoHaters(XTargetAutoHaters& other) { other.demerge(m_autohatermgr); }
	void	IncrementAutoHater(Mob* m) { m_autohatermgr.increment_count(m); }
	void	DecrementAutoHater(Mob* m) { m_autohatermgr.decrement_count(m); }
	[[nodiscard]] const XTargetAutoHaters* GetXTargetAutoMgr() const { return &m_autohatermgr; }

	void	QueueClientsInRange(const Mob *sender, const EQApplicationPacket *app, bool ack_required = true, bool ignore_sender = true, float distance = 0) const;

	[[nodiscard]] bool AnyMemberHasDzLockout(const std::string& expedition, const std::string& event) const;

	// The one deliberate exception to the callback-iteration convention
	// below -- expedition_request.cpp needs a materialized, sortable
	// snapshot (ordering members by group number for expedition creation),
	// which a callback can't support. GetRaidGroupMembers()/
	// GetRaidGroupBotMembers()/GetRaidBotMembers() used to exist alongside
	// this as a second, vector-returning convention, but had no caller
	// anywhere -- removed; use ForEachMember()/ForEachBot() with a
	// group_number/owner filter at the call site instead.
	[[nodiscard]] std::vector<RaidMember> GetMembers() const;

	// Every raid roster slot with a name, whether or not that member is
	// physically present in this zone (member may be nullptr). Mirrors
	// Group::ForEachMember's early-exit convention: return bool from fn to
	// keep going/stop, or return nothing to always visit every slot.
	template <typename Fn>
	void ForEachMember(Fn &&fn) const
	{
		for (const auto& m : members) {
			if (m.InUse()) {
				if constexpr (std::is_invocable_r_v<bool, Fn, const RaidMember&>) {
					if (!fn(m)) return;
				} else {
					fn(m);
				}
			}
		}
	}

	// Name-only visit, mirroring Group::ForEachMemberName -- cheaper than
	// ForEachMember() when the caller only needs the name.
	template <typename Fn>
	void ForEachMemberName(Fn &&fn) const
	{
		for (const auto& m : members) {
			if (m.InUse()) {
				if constexpr (std::is_invocable_r_v<bool, Fn, const char*>) {
					if (!fn(m.name.c_str())) return;
				} else {
					fn(m.name.c_str());
				}
			}
		}
	}

	// Same, but only slots with a live local Mob* (physically present in
	// this zone) -- the common case for anything that needs to actually
	// touch the member, not just know their name.
	template <typename Fn>
	void ForEachLocalMember(Fn &&fn) const
	{
		for (const auto& m : members) {
			if (m.member != nullptr) {
				if constexpr (std::is_invocable_r_v<bool, Fn, const RaidMember&>) {
					if (!fn(m)) return;
				} else {
					fn(m);
				}
			}
		}
	}

	template <typename Fn>
	void ForEachClient(Fn &&fn) const
	{
		ForEachLocalMember([&](const RaidMember& m) -> bool {
			if (!m.member->IsClient()) return true;
			if constexpr (std::is_invocable_r_v<bool, Fn, Client*>) {
				return fn(m.member->CastToClient());
			} else {
				fn(m.member->CastToClient());
				return true;
			}
		});
	}

	template <typename Fn>
	void ForEachBot(Fn &&fn) const
	{
		ForEachLocalMember([&](const RaidMember& m) -> bool {
			if (!m.member->IsBot()) return true;
			if constexpr (std::is_invocable_r_v<bool, Fn, Bot*>) {
				return fn(m.member->CastToBot());
			} else {
				fn(m.member->CastToBot());
				return true;
			}
		});
	}

	// TODO: all brances of the originating XTarget packet in client_packet need to be functions called on group and raid
	uint16 marked_npcs[MAX_MARKED_NPCS] = {};

private:
	// Instance convenience over SendRaidAction() for every Request* method
	// that does have a live raid -- also stamps header.rid = GetID().
	template <typename T>
	void RequestAction(const Client* requestor, T&& payload, RaidActionType action) const;

	// Shared builder for SendRaidAssister[To]/SendRaidMarker[To]/
	// SendRaidMasterLooter[To] -- resolves EntityID opportunistically (0 if
	// the named member isn't physically resolvable in this zone; the client
	// still needs the packet for the chat message even then, so this no
	// longer aborts the way the old per-ability functions did) and returns
	// nullptr only if name itself is empty. Caller owns/deletes the result.
	static EQApplicationPacket* BuildDelegateAbilityPacket(uint32 ability, uint32 member_number, bool clearing, const char* name);

	// Packet functions with no caller outside Raid itself -- see the public
	// //Packet Functions block's comment for why these live here instead.
	void	SendRaidCreate(Client *to) const;
	// RoF2/TOB single-packet bulk join (action=6/raidMembers on both clients,
	// the raid analogue of Group's OP_GroupUpdateB) -- see
	// RaidMembersUpdate_Struct's comment in common/eq_packet_structs.h for
	// what's confirmed vs. zero-filled on the wire. Both clients read this
	// same internal struct via their own client-specific
	// ENCODE(OP_RaidUpdate) raidMembers case (common/patches/rof2.cpp,
	// common/patches/tob.cpp) -- each serializes it into that client's own,
	// structurally different wire format, so this method itself never touches
	// wire bytes. Earlier client versions still use
	// SendRaidCreate()+SendRaidAdd() per member instead.
	void	SendRaidMembers(Client *to) const;
	// Only callers left are ApplyMemberJoinedFromWorld()/ApplyFullFromWorld()
	// (formerly also the now-deleted ServerOP_RaidState/RaidMemberUpsert
	// handlers' own cold-start-completion logic in zone/worldserver.cpp).
	void	SendBulkRaidInfoTo(Client *c) const;
	void	SendRaidAdd(const char *who, Client *to) const;
	void	SendClassLevelUpdate(const char *who, Client *to) const;
	void	SendRaidRemoveAll(const char *who) const;
	static void	SendRaidDisband(Client *to);
	void	SendRaidDisbandAll() const;
	void	SendBulkRaid(Client *to) const;
	void    SendRaidNotes() const;
	void    SendAssistTarget(Client* c) const;
	void	SendGroupLeaderChange(uint32 group_number, const char *old_leader_name, const char *new_leader_name) const;
	void	SendRaidLock() const;
	void	SendRaidUnlock() const;
	static void SendRaidLockTo(Client *c);
	static void SendRaidUnlockTo(Client *c);
	void	SendMakeLeaderPacket(const char *who) const;
	void	SendMakeLeaderPacketTo(const char *who, Client *to) const;
	void	SendRaidMOTD(Client* c) const;
	void	SendRaidMOTD() const;
	// slot is the 1-indexed MemberNumber the verified OP_RaidDelegateAbility
	// wire format requires (tob/packet_analysis/OP_RaidDelegateAbility.md) --
	// unused by the client for MasterLooter (always pass 0), but load-bearing
	// for MainAssist/MainMarker: the client indexes its own 3-slot arrays by
	// it, so a hardcoded 0 silently corrupts slots 2/3.
	static void SendRaidAssisterTo(uint32 slot, const char* assister, Client* to);
	void    SendRaidAssister(uint32 slot, bool clearing, const char* assister) const;
	static void SendRaidMarkerTo(uint32 slot, const char* marker, Client* to);
	void    SendRaidMarker(uint32 slot, bool clearing, const char* marker) const;
	static void SendRaidMasterLooterTo(const char* looter, Client* to);
	void    SendRaidMasterLooter(bool clearing, const char* looter) const;
	void    SendMarkTargets(const Client* c) const;

	Client *leader = nullptr;
	XTargetAutoHaters m_autohatermgr;
};
