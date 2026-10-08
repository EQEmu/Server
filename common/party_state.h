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
#include "common/servertalk.h"
#include "common/strings.h"
#include "common/types.h"

#include <algorithm>
#include <array>
#include <string>

constexpr uint8 MAX_GROUP_MEMBERS = 6;
constexpr uint8 MAX_RAID_GROUPS = 12;
constexpr uint8 MAX_RAID_MEMBERS = 72;
constexpr uint32 RAID_GROUPLESS = 0xFFFFFFFF;

constexpr uint8 MAX_RAID_MAIN_ASSISTERS = 3;
constexpr uint8 MAX_RAID_MAIN_MARKERS = 3;
constexpr uint8 MAX_MARKED_NPCS = 3;
constexpr uint8 MAX_RAID_LOOTERS = 9;

#pragma region Message Structs

struct ServerRaidMessage_Struct {
	uint32 rid;
	uint32 group_number; // RAID_GROUPLESS for a raid-wide say
	uint8 language;
	uint8 lang_skill;
	char from[64];
	char message[0];
};

enum class RaidUpdateType : uint8 {
	Full                = 0, // ServerRaidFull_Struct
	MemberJoined        = 1, // ServerRaidMemberJoined_Struct
	MemberLeft          = 2, // ServerRaidMemberLeft_Struct
	MemberFieldsChanged = 3, // ServerRaidMemberFieldsChanged_Struct
	GroupMoved          = 4, // ServerRaidGroupMoved_Struct
	LeaderChanged       = 5, // ServerRaidLeaderChanged_Struct
	GroupLeaderChanged  = 6, // ServerRaidGroupLeaderChanged_Struct
	StateChanged        = 7, // ServerRaidStateChanged_Struct
	Disbanded           = 8, // header only
	AAChanged           = 9, // ServerRaidAAChanged_Struct
	RoleChanged         = 10, // ServerRaidRoleChanged_Struct
};

struct ServerRaidUpdateHeader_Struct {
	RaidUpdateType  update_type; // RaidUpdateType
	uint32 rid;
};

struct ServerRaidRoleChanged_Struct {
	ServerRaidUpdateHeader_Struct header;
	uint8 role_id;
	uint8 slot;
	bool  clearing;
	char  target_name[64];
};

enum class RaidMemberKind : uint8 { RaidMemberPlayer = 0, RaidMemberBot = 1 };

struct ServerRaidMemberFields_Struct {
	char           member_name[64];
	uint32         group_number; // RAID_GROUPLESS if ungrouped
	uint8          _class;
	uint8          level;
	char           note[64];
	bool           is_group_leader;
	uint8          roles; // RaidRoleBitmask (common/party_state.h) -- looter/master-looter/leader
	uint8          main_marker;
	uint8          main_assister;
	RaidMemberKind kind; // common/party_state.h -- same enum both ends of the wire already share
	bool           is_offline; // world-computed via PartyStateManager::IsCharacterOnline; bots are never offline independently
};

struct ServerRaidFull_Struct {
	ServerRaidUpdateHeader_Struct header;
	uint8  member_count;
	ServerRaidMemberFields_Struct members[MAX_RAID_MEMBERS];
	char   leader_name[64];
	bool   locked;
	uint32 loot_type;
	char   motd[1024];
	char   main_assist_names[MAX_RAID_MAIN_ASSISTERS][64];
	char   main_marker_names[MAX_RAID_MAIN_MARKERS][64];
	char   master_looter_name[64];
	char   looter_names[MAX_RAID_LOOTERS][64];
	RaidLeadershipAA_Struct raid_aa;
};

struct ServerRaidMemberJoined_Struct {
	ServerRaidUpdateHeader_Struct header;
	ServerRaidMemberFields_Struct member;
};

struct ServerRaidMemberLeft_Struct {
	ServerRaidUpdateHeader_Struct header;
	char member_name[64];
};

struct ServerRaidMemberFieldsChanged_Struct {
	ServerRaidUpdateHeader_Struct header;
	ServerRaidMemberFields_Struct member;
};

struct ServerRaidGroupMoved_Struct {
	ServerRaidUpdateHeader_Struct header;
	char   member_name[64];
	uint32 new_group_number;
};

struct ServerRaidLeaderChanged_Struct {
	ServerRaidUpdateHeader_Struct header;
	char leader_name[64];
};

struct ServerRaidGroupLeaderChanged_Struct {
	ServerRaidUpdateHeader_Struct header;
	uint32 group_number;
	char   old_leader_name[64];
	char   new_leader_name[64];
};

struct ServerRaidStateChanged_Struct {
	ServerRaidUpdateHeader_Struct header;
	bool   locked;
	uint32 loot_type;
	char   motd[1024];
	char   main_assist_names[MAX_RAID_MAIN_ASSISTERS][64];
	char   main_marker_names[MAX_RAID_MAIN_MARKERS][64];
	char   master_looter_name[64];
	char   looter_names[MAX_RAID_LOOTERS][64];
	RaidLeadershipAA_Struct raid_aa;
};

struct ServerRaidUpdateDisbanded_Struct {
	ServerRaidUpdateHeader_Struct header;
};

struct ServerRaidAAChanged_Struct {
	ServerRaidUpdateHeader_Struct header;
	RaidLeadershipAA_Struct raid_aa;
};

enum class GroupMemberKind : uint8 { GroupMemberPlayer = 0, GroupMemberMerc = 1, GroupMemberBot = 2 };

struct ServerGroupStateMember_Struct {
	char            name[64];
	uint8           class_;
	uint8           level;
	GroupMemberKind kind; // common/party_state.h -- same enum both ends of the wire already share
	uint32          owner_character_id; // only meaningful if kind != GroupMemberPlayer
	char            owner_name[64];     // only meaningful if kind != GroupMemberPlayer
	uint8           roles; // GroupRoleBitmask (common/party_state.h)
	bool            is_offline; // world-computed via ClientList; bots/mercs are never offline independently
};

enum class GroupUpdateType : uint8 {
	Full           = 0, // ServerGroupState_Struct
	MemberJoined   = 1, // ServerGroupMemberJoined_Struct
	MemberLeft     = 2, // ServerGroupMemberLeft_Struct
	LeaderChanged  = 3, // ServerGroupLeaderChanged_Struct
	RoleChanged    = 4, // ServerGroupRoleChanged_Struct
	OfflineChanged = 5, // ServerGroupOfflineChanged_Struct
	MentorChanged  = 6, // ServerGroupMentorChanged_Struct
	AAChanged      = 7, // ServerGroupAAChanged_Struct
	Disbanded      = 8, // ServerGroupDisbanded_Struct -- no payload beyond the header itself
	MemberZoned    = 9, // ServerGroupState_Struct
};

struct ServerGroupUpdateHeader_Struct {
	GroupUpdateType  update_type;
	uint32 gid;
};

struct ServerGroupState_Struct {
	ServerGroupUpdateHeader_Struct header; // header.update_type == GroupUpdateType::Full
	uint8  member_count;
	ServerGroupStateMember_Struct members[MAX_GROUP_MEMBERS];
	char   leader_name[64];
	char   main_tank_name[64];
	char   main_assist_name[64];
	char   puller_name[64];
	char   npc_marker_name[64];
	char   master_looter_name[64];
	char   mentoree_name[64];
	int32  mentor_percent;
	GroupLeadershipAA_Struct leadership_aa;
	uint32 raid_id;   // 0 = standalone group
	uint8  raid_slot; // 0xFF = not a raid subgroup
	char   character_name[64]; // Only meaningful when header.update_type == GroupUpdateType::MemberZoned
};

struct ServerGroupMemberJoined_Struct {
	ServerGroupUpdateHeader_Struct header;
	ServerGroupStateMember_Struct  member;
};

struct ServerGroupMemberLeft_Struct {
	ServerGroupUpdateHeader_Struct header;
	char member_name[64];
};

struct ServerGroupLeaderChanged_Struct {
	ServerGroupUpdateHeader_Struct header;
	char leader_name[64];
};

struct ServerGroupRoleChanged_Struct {
	ServerGroupUpdateHeader_Struct header;
	uint8 role_id;
	char  target_name[64];
};

struct ServerGroupOfflineChanged_Struct {
	ServerGroupUpdateHeader_Struct header;
	char member_name[64];
	bool is_offline;
};

struct ServerGroupMentorChanged_Struct {
	ServerGroupUpdateHeader_Struct header;
	char  mentoree_name[64];
	int32 mentor_percent;
};

struct ServerGroupAAChanged_Struct {
	ServerGroupUpdateHeader_Struct header;
	GroupLeadershipAA_Struct       leadership_aa;
};

struct ServerGroupDisbanded_Struct {
	ServerGroupUpdateHeader_Struct header;
};

struct ServerPartyStateRequest_Struct {
	uint32 zoneid;
	uint16 instance_id;
	char   character_name[64];
};

enum class RaidActionType {
	RaidLeadershipMakeLeader = 0,
	RaidLeadershipRemoveGroupLeader,
	RaidLeadershipLock,
	RaidLeadershipUnlock,
	RaidLeadershipSetLootType,
	RaidLeadershipAddLooter,
	RaidLeadershipRemoveLooter,
	RaidLeadershipSetGroupOnInvite,
	RaidLeadershipUpdateLevel,
	RaidLeadershipUpdateAA,

	RaidRoleDelegateAssist,
	RaidRoleUndelegateAssist,
	RaidRoleDelegateMark,
	RaidRoleUndelegateMark,
	RaidRoleDelegateMasterLooter,
	RaidRoleUndelegateMasterLooter,
	RaidRoleSetMotd,
	RaidRoleSetNote,

	RaidActionDisband,
	RaidActionInvite,
	RaidActionAcceptInvite,
	RaidActionRemoveMember,
	RaidActionMoveGroup,
};

struct ServerRaidActionHeader_Struct {
	RaidActionType  action;
	uint32 zoneid;
	uint16 instance_id;
	uint32 rid;
	uint32 requestor_character_id;
	char   requestor_name[64];
};

struct ServerRaidActionBasic_Struct {
	ServerRaidActionHeader_Struct header;
};

struct ServerRaidActionTarget_Struct {
	ServerRaidActionHeader_Struct header;
	char target_name[64];
};

// RaidActionInvite only. The invitee could be anywhere (or a bot that isn't
// even spawned), so the fields below are populated by zone only when it can
// resolve target_name locally (a live Client or Bot found in that zone) --
// left zeroed otherwise. Class 0 / level 0 are never valid values in EQ
// (classes start at 1, minimum level is 1), so they double as "unknown"
// sentinels; `resolved` still says explicitly whether zone found anything
// at all (a bot's owner_character_id can legitimately be non-zero while
// class/level stay unknown). When `resolved` is false, world enriches from
// its own live data (ClientList for a player, bot_data for a bot's
// identity/ownership only, never class/level -- see world/party_state.cpp)
// before proceeding -- either way the result goes through the exact same
// AddPartyToRaid call.
struct ServerRaidActionInvite_Struct {
	ServerRaidActionHeader_Struct header;
	char           target_name[64];
	bool           resolved;
	RaidMemberKind kind; // meaningless unless resolved
	uint32         owner_character_id; // only meaningful if kind == RaidMemberBot
	char           owner_name[64];     // only meaningful if kind == RaidMemberBot
	uint8          target_class;
	uint8          target_level;
};

struct ServerRaidActionTargetValue_Struct {
	ServerRaidActionHeader_Struct header;
	char   target_name[64];
	uint32 value;
};

struct ServerRaidActionValue_Struct {
	ServerRaidActionHeader_Struct header;
	uint32 value;
};

struct ServerRaidActionMotd_Struct {
	ServerRaidActionHeader_Struct header;
	char motd[1024];
};

struct ServerRaidActionNote_Struct {
	ServerRaidActionHeader_Struct header;
	char target_name[64];
	char note[64];
};

enum class GroupActionType {
	GroupActionMakeLeader = 0,
	GroupActionDelegateMainTank,
	GroupActionUndelegateMainTank,
	GroupActionDelegateMainAssist,
	GroupActionUndelegateMainAssist,
	GroupActionDelegatePuller,
	GroupActionUndelegatePuller,
	GroupActionDelegateMasterLooter,
	GroupActionUndelegateMasterLooter,
	GroupActionDelegateMarkNPC,
	GroupActionUndelegateMarkNPC,
	GroupActionUpdateAA,

	GroupActionMentor,

	GroupActionDisband, // target_name empty = disband all
	GroupActionFollow,
	GroupActionAddMember,

	GroupActionRefresh,
};

struct ServerGroupActionHeader_Struct {
	GroupActionType  action;
	uint32 zoneid;
	uint16 instance_id;
	uint32 gid;
	uint32 requestor_character_id;
	char   requestor_name[64];
};

struct ServerGroupActionBasic_Struct {
	ServerGroupActionHeader_Struct header;
};

struct ServerGroupActionTarget_Struct {
	ServerGroupActionHeader_Struct header;
	char target_name[64];
};

struct ServerGroupActionFollow_Struct {
	ServerGroupActionHeader_Struct header;
	char   inviter_name[64];
	uint8  accepter_class;
	uint8  accepter_level;
	GroupLeadershipAA_Struct accepter_group_aa; // live-PP snapshot, mirrored not validated
	uint8  inviter_class;
	uint8  inviter_level;
};

struct ServerGroupActionAddMember_Struct {
	ServerGroupActionHeader_Struct header;
	uint8  requestor_class;
	uint8  requestor_level;
	GroupLeadershipAA_Struct requestor_group_aa; // live-PP snapshot, mirrored not validated
	char   target_name[64];        // bot/merc name to add
	bool   target_is_bot;
	bool   target_is_merc;
	uint32 target_owner_character_id; // == header.requestor_character_id
	char   target_owner_name[64];     // == header.requestor_name
	uint8  target_class; // zone resolves the live Bot*/Merc* directly
	uint8  target_level;
};

struct ServerGroupActionMentor_Struct {
	ServerGroupActionHeader_Struct header;
	bool   clear;
	char   mentoree_name[64];
	int32  mentor_percent;
};

struct ServerPartyActionFailed_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 requestor_character_id;
	char   reason[128];
};

struct ServerRaidInvitePushed_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 invitee_character_id;
	char   invitee_name[64];
	char   inviter_name[64];
};

struct ServerLFGMatchesRequest_Struct {
	uint32	FromID;
	uint8	QuerierLevel;
	char	FromName[64];
	uint8	FromLevel;
	uint8	ToLevel;
	uint32	Classes;
};

struct ServerLFGMatchesResponse_Struct {
	char Name[64];
	uint8 Level;
	uint8 Class_;
	uint16 Zone;
	uint16 GuildID;
	uint16 Anon;
	char Comments[64];
};

struct ServerLFPUpdate_Struct {
	uint32 LeaderID;
	uint8 Action;
	uint8 MatchFilter;
	uint32	FromLevel;
	uint32	ToLevel;
	uint32	Classes;
	char	Comments[64];
	GroupLFPMemberEntry Members[MAX_GROUP_MEMBERS];
};

struct ServerLFPMatchesResponse_Struct {
	uint32	FromLevel;
	uint32	ToLevel;
	uint32	Classes;
	GroupLFPMemberEntry Members[MAX_GROUP_MEMBERS];
	char Comments[64];
};

struct ServerLFPMatchesRequest_Struct {
	uint32	FromID;
	uint8	FromLevel;
	uint8	ToLevel;
	uint8	QuerierClass;
	uint8	QuerierLevel;
	char	FromName[64];
};

#pragma endregion

struct PartyMemberBase
{
	std::string name;
	uint8 class_ = 0;
	uint8 level = 0;
	uint8 roles = 0;
	uint32 owner_character_id = 0;
	std::string owner_name;

	[[nodiscard]] bool InUse() const { return !name.empty(); }
};

class PartyIDHolder {
public:
	virtual ~PartyIDHolder() = default;
	[[nodiscard]] uint32 GetID() const { return id; }
protected:
	// these two classes are where lifetime management lives
	friend class EntityList;
	friend class PartyStateManager;
	void SetID(uint32 set_id) { id = set_id; }
private:
	uint32 id = 0;
};

template <typename Member, uint8 MAX_PARTY_MEMBERS>
requires std::derived_from<Member, PartyMemberBase>
class PartyBase : public PartyIDHolder
{
public:
	static constexpr uint8 MaxMembers = MAX_PARTY_MEMBERS;

protected:
	std::array<Member, MAX_PARTY_MEMBERS> members{};
	std::string leader_name;
	std::string master_looter_name;

	[[nodiscard]] bool IsMemberOf(std::string_view name) const
	{
		return std::ranges::any_of(members, [&name](const Member& member) {
			return member.InUse() && Strings::EqualFold(member.name, name);
		});
	}

	[[nodiscard]] uint8 Count() const
	{
		return std::ranges::count_if(members, [](const Member& member) {  return member.InUse(); });
	}

	Member* FindFreeSlot()
	{
		auto slot = std::ranges::find_if(members, [](const Member& member) { return !member.InUse(); });
		return slot != members.end() ? std::to_address(slot) : nullptr;
	}

	Member* FindMember(std::string_view character_name)
	{
		auto member =  std::ranges::find_if(members, [character_name](const Member& member) {
			return Strings::EqualFold(member.name, character_name);
		});

		return member != members.end() ? std::to_address(member) : nullptr;
	}

	const Member* FindMember(std::string_view character_name) const
	{
		auto member = std::ranges::find_if(members, [character_name](const Member& member) {
			return Strings::EqualFold(member.name, character_name);
		});

		return member != members.end() ? std::to_address(member) : nullptr;
	}

	[[nodiscard]] std::string_view EffectiveMasterLooter() const
	{
		return master_looter_name.empty() ? leader_name : master_looter_name;
	}

	void SetRoleHolder(uint8 role_bit, std::string_view holder_name)
	{
		for (auto& m : members)
			if (m.InUse())
				m.roles &= ~role_bit;

		if (!holder_name.empty()) {
			Member* holder = FindMember(holder_name);
			if (holder != nullptr)
				holder->roles |= role_bit;
		}
	}

	template <typename Fn>
	void ForEachMember(Fn &&fn) const
	{
		for (const Member& member : members) {
			if (member.InUse()) {
				if constexpr (std::is_invocable_r_v<bool, Fn, const Member&>) {
					if (!fn(member)) return;
				} else {
					fn(member);
				}
			}
		}
	}
};

enum GroupRoleNumber : uint8 {
	GroupRoleMainTank     = 1,
	GroupRoleMainAssist   = 2,
	GroupRolePuller       = 3,
	GroupRoleMarkNPC      = 4,
	GroupRoleMasterLooter = 5,
};

enum GroupRoleBitmask : uint8
{
	GroupBitmaskMainTank     = 0x01,
	GroupBitmaskMainAssist   = 0x02,
	GroupBitmaskPuller       = 0x04,
	GroupBitmaskMarkNPC      = 0x08,
	GroupBitmaskMasterLooter = 0x10,
	GroupBitmaskLeader       = 0x20,
};

template <typename B>
requires std::derived_from<B, PartyMemberBase>
struct GroupMemberBase : B {
	GroupMemberKind kind   = GroupMemberKind::GroupMemberPlayer;
};

template <typename T>
concept DerivedFromGroupMemberBase = requires (const T& t) { []<typename B>(const GroupMemberBase<B>&) {}(t); };

template <typename Member>
requires DerivedFromGroupMemberBase<Member>
class GroupBase : public PartyBase<Member, MAX_GROUP_MEMBERS>
{
public:
	using MemberType = Member;

protected:
	GroupLeadershipAA_Struct leadership_aa{};
	std::string main_tank_name;
	std::string main_assist_name;
	std::string puller_name;
	std::string npc_marker_name;
	std::string mentoree_name;
	int32       mentor_percent = 0;
};

// TODO: do I need Raid Role Number as well?
enum RaidRoleBitmask : uint8
{
	RaidBitmaskMainAssist   = 0x01,
	RaidBitmaskMainMarker   = 0x02,
	RaidBitmaskMasterLooter = 0x04,
	RaidBitmaskLooter       = 0x08,
	RaidBitmaskLeader       = 0x10,
};

template <typename B>
requires std::derived_from<B, PartyMemberBase>
struct RaidMemberBase : B {
	std::string    note;
	uint8          main_marker      = 0; // TODO: I think it's safe to consolidate these into roles even though they are mutually exclusive. It'll make math simpler
	uint8          main_assister    = 0; // TODO: I think it's safe to consolidate these into roles even though they are mutually exclusive. It'll make math simpler
	RaidMemberKind kind             = RaidMemberKind::RaidMemberPlayer;
};

template <typename T>
concept DerivedFromRaidMemberBase = requires (const T& t) { []<typename B>(const RaidMemberBase<B>&) {}(t); };

template <typename Member>
requires DerivedFromRaidMemberBase<Member>
class RaidBase : public PartyBase<Member, MAX_RAID_MEMBERS> {
public:
	using MemberType = Member;

protected:
	RaidLeadershipAA_Struct leadership_aa{};
	std::string motd;
	bool        locked = false;
	uint32      loot_type = 0;
	std::array<std::string, MAX_RAID_MAIN_ASSISTERS> main_assist_names{};
	std::array<std::string, MAX_RAID_MAIN_MARKERS>   main_marker_names{};
	std::string master_looter_name;
	std::array<std::string, MAX_RAID_LOOTERS> looter_names{};
	std::array<uint32, MAX_RAID_GROUPS> subgroup_group_ids{};
};
