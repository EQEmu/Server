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

#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/zonelist.h"
#include "world/zoneserver.h"

#include <unordered_set>

class ServerPacket;
class ZoneServer;

struct WorldPartyMember : PartyMemberBase
{
	static WorldPartyMember Make(std::string_view name, uint8 class_, uint8 level,
		uint32 owner_character_id, std::string_view owner_name, uint32 zone_id, uint32 instance_id)
	{
		return WorldPartyMember{
			PartyMemberBase{
				.name               = std::string(name),
				.class_             = class_,
				.level              = level,
				.owner_character_id = owner_character_id,
				.owner_name         = std::string(owner_name),
			},
			zone_id,
			instance_id,
		};
	}

	uint32 zone_id = 0;
    uint32 instance_id = 0;
};

inline void SendPartyActionFailure(uint32 requestor_character_id, std::string_view reason)
{
    if (requestor_character_id != 0) {
        auto* cle = ClientList::Instance()->FindCLEByCharacterID(requestor_character_id);
        if (cle != nullptr && cle->Server() != nullptr) {
            ServerPartyActionFailed_Struct fail{};
            fail.requestor_character_id = requestor_character_id;
            Strings::Copy(fail.reason, reason);

            auto pack = new ServerPacket(ServerOP_PartyActionFailed, sizeof(ServerPartyActionFailed_Struct));
            memcpy(pack->pBuffer, &fail, sizeof(ServerPartyActionFailed_Struct));
            cle->Server()->SendPacket(pack);
            safe_delete(pack);
        }
    }
}

template <typename Party, uint16 STATE_OPCODE>
requires std::derived_from<typename Party::MemberType, WorldPartyMember>
class WorldParty : public Party
{
protected:
    using Member = Party::MemberType;

    void RelayToMembers(ServerPacket* pack) const
    {
        std::unordered_set<uint32> sent_to;
        this->ForEachMember([&sent_to, &pack](const Member& member) {
            ZoneServer* zs = ZSList::Instance()->FindByZoneIDAndInstanceID(member.zone_id, member.instance_id);
            if (zs != nullptr && sent_to.insert(zs->GetID()).second)
                zs->SendPacket(pack);
        });
    }

    // Group's wire headers key off .gid, raid's off .rid -- both just mean
    // "this party's id", so pick whichever the header struct actually has
    // instead of forcing every wire struct onto one field name.
    template <typename Header>
    static void SetHeaderId(Header& header, uint32 id)
    {
        if constexpr (requires { header.gid; })
            header.gid = id;
        else if constexpr (requires { header.rid; })
            header.rid = id;
    }

    template <typename T, typename UpdateType>
    ServerPacket* BuildPacket(T&& payload, UpdateType type) const
    {
        payload.header.update_type = type;
        SetHeaderId(payload.header, this->GetID());

        auto pack = new ServerPacket(STATE_OPCODE, sizeof(T));
        memcpy(pack->pBuffer, &payload, sizeof(T));

        return pack;
    }

    template <typename T, typename UpdateType>
    void PushDelta(T&& payload, UpdateType type) const
    {
        ServerPacket* pack = BuildPacket(std::forward<T>(payload), type);
        RelayToMembers(pack);
        safe_delete(pack);
    }

    template <typename T, typename UpdateType>
    void PushDelta(T&& payload, UpdateType type, ZoneServer* zs) const
    {
        if (zs != nullptr) {
            ServerPacket* pack = BuildPacket(std::forward<T>(payload), type);
            zs->SendPacket(pack);
            safe_delete(pack);
        }
    }
};

struct WorldGroupMember : GroupMemberBase<WorldPartyMember>
{
	static WorldGroupMember Make(std::string_view name, uint8 class_, uint8 level, GroupMemberKind kind,
		uint32 owner_character_id, std::string_view owner_name, uint32 zone_id, uint16 instance_id)
	{
		return WorldGroupMember{
			WorldPartyMember::Make(name, class_, level, owner_character_id, owner_name, zone_id, instance_id),
			kind,
		};
	}
};

struct RaidLink
{
	uint32 raid_id = 0;
	uint8 slot = 0xFF;
	bool locked = false;
	std::function<void(uint32 raid_id, std::string_view character_name)> on_member_departed;
	std::function<void(uint32 raid_id, const WorldGroupMember& new_member)> on_member_joined;

	[[nodiscard]] bool IsLinked() const
	{
		return raid_id != 0 && static_cast<bool>(on_member_departed) && static_cast<bool>(on_member_joined);
	}
};

class WorldGroup : public WorldParty<GroupBase<WorldGroupMember>, ServerOP_GroupState>
{
protected:
	friend class PartyStateManager; // TODO: get rid of this
	friend class GroupBuilder;
	RaidLink raid_link;

	// Slot-fill only -- no push, no index update. Used by AddMember (which
	// adds both) and by GroupBuilder (which defers both until Push()).
	WorldGroupMember* PlaceMember(WorldGroupMember member);

public:
	// ---- Outbound pushes (world->zone)
	// TODO: I don't like that full is defaulted here
	[[nodiscard]] ServerPacket* BuildFullPacket(GroupUpdateType type = GroupUpdateType::Full, std::string_view targeted_character_name = "") const;
	void PushFull() const;
	void PushFullTo(const WorldGroupMember& member, GroupUpdateType type) const;

	void AddMember(std::string_view name, uint8 class_, uint8 level, GroupMemberKind kind,
		uint32 owner_character_id, std::string_view owner_name, uint32 zone_id, uint16 instance_id);
	void AddMember(WorldGroupMember member);

	void RemoveMember(std::string_view character_name);
	void Disband() const;

	// Returns true if the group should now be disbanded (no eligible next leader).
	void SetMemberOffline(std::string_view character_name);
	void SetMemberOnline(std::string_view character_name) const;

	void SetRaidLink(const RaidLink& link);
	void SetLocked(bool locked);
	void SetMentor(std::string_view mentoree_name, uint8 mentor_percent);

	void HandleStateRequest(std::string_view character_name, ZoneServer* requester);
	void HandleLeadershipAction(const ServerGroupActionHeader_Struct& header, std::string_view target_name, GroupActionType action);

private:
	void PushMemberJoined(const WorldGroupMember& new_member) const;
	void PushMemberLeft(std::string_view departed_name, std::string_view new_leader_name, ZoneServer* departing_zs) const;
	void PushGroupDisbanded() const;
	void PushLeaderChanged() const;
	void PushRoleChanged(uint8 role_id, std::string_view target_name) const;
	void PushOfflineChanged(std::string_view member_name, bool is_offline) const;
	void PushMentorChanged() const;
	void PushAAChanged() const;

	[[nodiscard]] std::string_view FindNextLeader(std::string_view offline_name) const;
};

struct WorldRaidMember;
class GroupBuilder
{
public:
	GroupBuilder& AddMember(WorldGroupMember member);
	GroupBuilder& AddMember(std::string_view name, uint8 class_, uint8 level, GroupMemberKind kind,
		uint32 owner_character_id, std::string_view owner_name, uint32 zone_id, uint16 instance_id);
	GroupBuilder& AddMember(const WorldRaidMember& member);
	GroupBuilder& SetLeadershipAA(const GroupLeadershipAA_Struct& aa);
	GroupBuilder& LinkToRaid(const RaidLink& link);

	uint32 Push();

private:
	WorldGroup m_group;
};

struct WorldRaidMember : RaidMemberBase<WorldPartyMember>
{
	static WorldRaidMember Make(std::string_view name, uint8 class_, uint8 level, RaidMemberKind kind,
		uint32 owner_character_id, uint32 zone_id, uint32 instance_id)
	{
		return WorldRaidMember{
			RaidMemberBase<WorldPartyMember>{
				WorldPartyMember::Make(name, class_, level, owner_character_id, "", zone_id, instance_id),
				"",
				0,
				0,
				kind,
			}
		};
	}
};

class WorldRaid : public WorldParty<RaidBase<WorldRaidMember>, ServerOP_RaidUpdate>
{
protected:
	friend class PartyStateManager; // TODO: get rid of this
	friend class RaidBuilder;
	bool group_on_invite = false; // never sent to client; pure server-side placement policy

	// Slot-fill only -- no push, no index update. Used by AddMember (which
	// adds both) and by RaidBuilder (which defers both until Push()).
	WorldRaidMember* PlaceMember(WorldRaidMember member);

public:
	[[nodiscard]] uint8 GroupCount(uint32 group_number) const;
	[[nodiscard]] uint32 FindMemberGroupNumber(std::string_view character_name) const;
	[[nodiscard]] uint32 GetFreeGroup() const;
	[[nodiscard]] uint32 GetGroupWithRoom() const;
	void JoinRealSubgroup(uint32 group_number, std::string_view name, uint8 class_, uint8 level,
		bool is_bot, uint32 owner_character_id, uint32 zone_id, uint32 instance_id);

	// ---- Outbound pushes (world->zone). group_number/is_group_leader come
	// from PartyStateManager's own group map -- resolved via
	// PartyStateManager::Instance() inside FillMemberFields, same as
	// WorldGroup calls PartyStateManager::Instance() for its own index
	// bookkeeping. Once group and raid share one manager this is a same-
	// object call, not a cross-singleton hop.
	void PushFullTo(ZoneServer* target) const;
	void PushMemberJoined(const WorldRaidMember& m) const;
	// departing_zs: the departed character's own zone -- by the time this
	// runs the member's slot is already cleared, so RelayToMembers can no
	// longer reach it through the member array.
	void PushMemberLeft(std::string_view character_name, ZoneServer* departing_zs) const;
	void PushMemberFieldsChanged(const WorldRaidMember& m) const;
	void PushGroupMoved(std::string_view character_name, uint32 new_group_number) const;
	void PushLeaderChanged() const;
	void PushStateChanged() const;
	void PushAAChanged() const;
	void PushRoleChanged(uint8 role_id, uint8 slot, std::string_view target_name, bool clearing) const;
	void PushDisbanded() const;
	void AddMember(WorldRaidMember member);
	void AddMember(const WorldGroupMember& member);

	// ---- Mutations
	WorldRaidMember* AddMember(std::string_view name, uint8 class_, uint8 level, bool raid_leader, bool looter,
		RaidMemberKind kind, uint32 owner_character_id, uint32 zone_id, uint32 instance_id);

	// Removes character_name (and cascades to any bots they own, or -- if
	// they were the group leader of a bot-only subgroup -- that subgroup's
	// bots), promoting/disbanding as needed. Disbanding and index/subgroup
	// teardown happen from inside here, same guarantee as WorldGroup's
	// Disband()/RemoveMember().
	void RemoveMember(std::string_view character_name);

	// Single disband chokepoint -- tears down every subgroup and erases the
	// raid from m_raids. Index/member-notification cleanup itself is
	// guaranteed by WorldRaid::Disband() (mirrors WorldGroup's own
	// Disband()); this only does the parts WorldRaid can't reach itself.
	void Disband() const;
	void DisbandAllSubgroups() const;

	void RecomputeMasterLooter();
	void RecomputeLooterNames();

	// Raid-local leadership/role actions -- everything HandleRaidAction
	// dispatches that only touches this raid's own state (plus the group
	// map, via PartyStateManager::Instance(), for subgroup-locking and
	// group_number resolution -- see the class comment above).
	void MakeLeader(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name);
	void UpdateLeadershipAA(uint32 requestor_character_id);
	void Lock(std::string_view requestor_name, uint32 requestor_character_id);
	void Unlock(std::string_view requestor_name, uint32 requestor_character_id);
	void SetLootType(std::string_view requestor_name, uint32 requestor_character_id, uint32 loot_type);
	void AddLooter(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name);
	void RemoveLooter(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name);
	void SetGroupOnInvite(std::string_view requestor_name, bool value);
	void DelegateAssist(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name);
	void UndelegateAssist(std::string_view target_name);
	void DelegateMark(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name);
	void UndelegateMark(std::string_view target_name);
	void DelegateMasterLooter(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name);
	void UndelegateMasterLooter(std::string_view target_name);
	void SetMotd(std::string_view requestor_name, uint32 requestor_character_id, std::string_view motd);
	void SetNote(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name, std::string_view note);
	void UpdateMemberLevel(std::string_view target_name, uint8 level);
	void HandleStateRequest(std::string_view character_name, ZoneServer* requester);

	void FillMemberFields(ServerRaidMemberFields_Struct& out, const WorldRaidMember& m) const;

private:
	[[nodiscard]] ServerPacket* BuildFullPacket() const;
};

// Same idea as GroupBuilder -- stages a brand-new raid (members, leader,
// adopted subgroups) with no network traffic until Push(), which fires
// one PushLeaderChanged + PushStateChanged instead of a push per member/
// per subgroup attach.
class RaidBuilder
{
public:
	RaidBuilder& AddMember(std::string_view name, uint8 class_, uint8 level, RaidMemberKind kind,
		uint32 owner_character_id, uint32 zone_id, uint32 instance_id);
	// Marks an already-added member as raid leader (also grants looter,
	// matching the live make-leader/found-raid behavior).
	RaidBuilder& SetLeader(std::string_view name);
	// Pulls gid's entire membership in as raid members (translated to
	// WorldRaidMember, no roles carried over) and queues it to become a
	// linked subgroup once the raid has a real id -- the group itself was
	// created the ordinary way (GroupBuilder), this just adopts it.
	RaidBuilder& AdoptGroup(uint32 gid);
	RaidBuilder& SetLootType(uint32 loot_type);

	uint32 Commit();

private:
	WorldRaid raid_;
	std::vector<uint32> pending_subgroup_gids_;
};

class PartyStateManager
{
public:
	static PartyStateManager& Instance();

	void HandleGroupStateRequest(std::string_view character_name, ZoneServer* requester);
	void HandleGroupAction(const ServerPacket* pack);
	void HandleRaidStateRequest(std::string_view character_name, ZoneServer* requester);
	void HandleRaidAction(const ServerPacket* pack);

	// The one and only way anything creates a group or a raid -- see
	// GroupBuilder/RaidBuilder's own comments.
	GroupBuilder NewGroup() { return GroupBuilder{}; }
	RaidBuilder NewRaid() { return RaidBuilder{}; }

	// ---- Group-side queries/mutations
	uint32 GetGroupIdOf(std::string_view character_name) const; // 0 if ungrouped
	bool IsGroupLeader(std::string_view character_name) const; // false if ungrouped
	std::vector<const WorldGroupMember*> GetGroupMembersOf(std::string_view character_name) const; // empty if ungrouped
	std::string_view GetGroupLeaderName(uint32 gid) const; // empty if no such group
	bool IsMemberOf(uint32 gid, std::string_view character_name) const;
	uint32 GetGroupMemberCount(uint32 gid) const; // 0 if no such group

	void ForceDisbandGroup(uint32 gid);
	void AddMemberToGroup(uint32 gid, WorldGroupMember member);
	void AddMemberToGroup(uint32 gid, std::string_view name, uint8 class_, uint8 level, bool is_bot,
		uint32 owner_character_id, bool is_merc = false, std::string_view owner_name = "", uint32 zone_id = 0,
		uint16 instance_id = 0);
	void RemoveMemberFromGroup(uint32 gid, std::string_view name);
	bool GroupExists(uint32 gid) const;
	void SetRaidLink(uint32 gid, const RaidLink& link);
	void SetGroupLocked(uint32 gid, bool locked);

	void IndexMember(std::string_view name, uint32 gid);
	void EraseFromIndex(const std::vector<std::string_view>& names);
	void EraseGroupAndIndex(uint32 gid, const std::vector<std::string_view>& departed_names);

	// ---- Raid-side index bookkeeping, same guarantee as the group-side
	// three above -- called by WorldRaid itself from AddMember/RemoveMember/
	// Disband, never left as a separate step for a caller to remember.
	void IndexRaidMember(std::string_view name, uint32 rid);
	void EraseFromRaidIndex(const std::vector<std::string_view>& names);
	void EraseRaidAndIndex(uint32 rid, const std::vector<std::string_view>& departed_names);

	void HandleCharacterOffline(std::string_view character_name); // group+raid offline transition
	void HandleCharacterOnline(std::string_view character_name); // group+raid online transition

	// ---- Raid-side queries
	bool IsRaidMember(std::string_view character_name) const;
	uint32 GetRaidIdOf(std::string_view character_name) const; // 0 if not a raid member
	bool CanAttachNewGroup(uint32 rid) const;
	void AttachExistingGroup(uint32 rid, uint32 gid);

	// Callback target for RaidLink -- a plain group invite/follow into a
	// raid-linked group never goes through any raid request handler, so
	// these are the only way raid learns about it.
	void HandleSubgroupMemberDeparted(uint32 rid, std::string_view character_name);
	void HandleSubgroupMemberJoined(uint32 rid, const WorldGroupMember& new_member);

	// ---- Cross-party helpers used by WorldRaid's own methods (group_number
	// resolution, subgroup creation/teardown) -- WorldRaid never touches
	// m_groups itself, it always comes back through here.
	void LinkSubgroup(uint32 gid, uint32 rid, uint32 group_number, bool locked);
	RaidLink MakeRaidLink(uint32 rid, uint32 group_number, bool locked);

private:
	friend class GroupBuilder;
	friend class RaidBuilder;

	uint32 AllocateGroupId(); // ZSList-assigned id, not yet installed anywhere
	WorldGroup& InstallGroup(uint32 gid, WorldGroup group); // installs into m_groups, sets its id
	WorldGroup* GetGroupById(uint32 gid);

	uint32 AllocateRaidId();
	WorldRaid& InstallRaid(uint32 rid, WorldRaid raid);

	std::unordered_map<uint32, WorldGroup> m_groups;
	TransparentStringMap<uint32> m_character_group_id; // recovery/routing index

	std::unordered_map<uint32, WorldRaid> m_raids;
	TransparentStringMap<uint32> m_character_raid_id; // recovery/routing index
	std::unordered_map<uint32, std::string> m_pending_invite_inviter_name;

	// ---- Group-side internals
	WorldGroup* FindGroupByCharacter(std::string_view character_name);
	const WorldGroup* FindGroupByCharacter(std::string_view character_name) const;

	void HandleRefreshRequest(uint32 gid) const;
	void HandleFollow(const ServerGroupActionFollow_Struct& in);
	void HandleAddMember(const ServerGroupActionAddMember_Struct& in);
	void HandleDisband(const ServerGroupActionTarget_Struct& in);
	void HandleLeadership(const ServerGroupActionHeader_Struct& header, std::string_view target_name,
		GroupActionType action);
	void HandleMentor(const ServerGroupActionMentor_Struct& in);

	uint32 EffectiveRaidIdOf(std::string_view character_name) const;

	// Shared by HandleFollow/HandleAddMember: called only once target_name
	// is already confirmed grouped. Group-initiated invites always ignore
	// group_on_invite (that's a /raidinvite-only concept) and instead
	// attempt a merge -- inviting a group's own leader combines the two
	// groups (matches live: the other group's members join, subject to a
	// combined-size check), same as inviting an ungrouped player, just with
	// more than one person moving.
	//
	// The inviter's side is always the survivor (existing group, or a
	// freshly created one if they were ungrouped) -- an external link only
	// ever transfers forward into that survivor, never the other way.
	// Someone outside a raid can never pull a raid-linked target's group
	// out of it via a plain invite/merge; only a raid-affiliated inviter
	// absorbing an outsider is allowed, and even then only into the same
	// raid (two different raid-linked sides never merge). Also fails if
	// target isn't their group's leader, either side is locked, or the
	// combined size would exceed MAX_GROUP_MEMBERS.
	//
	// NOTE: live also blocks this merge if anyone involved has an active
	// mercenary out -- world has no visibility into that (it's ephemeral
	// zone/Client state, never replicated here), so that check can't be
	// enforced yet and is deliberately left as a TODO for when zone sends
	// that fact through.
	void MergeIntoGroup(
		std::string_view inviter_name, std::string_view target_name,
		uint32 requestor_character_id, uint8 inviter_class, uint8 inviter_level,
		const GroupLeadershipAA_Struct* new_group_leadership_aa
	);

	// ---- Raid-side internals
	WorldRaid* FindRaidByCharacter(std::string_view character_name);

	void HandleActionDisband(const ServerRaidActionBasic_Struct& in);
	void HandleActionInvite(const ServerRaidActionInvite_Struct& in);
	void HandleActionAcceptInvite(const ServerRaidActionTarget_Struct& in);
	void HandleActionRemoveMember(const ServerRaidActionTarget_Struct& in);
	void HandleActionMoveGroup(const ServerRaidActionTargetValue_Struct& in);

	static void PushInvite(uint32 invitee_character_id, std::string_view invitee_name, std::string_view inviter_name);

	void AddPartyToRaid(
		std::string_view inviter_name,
		std::string_view accepter_name, uint8 accepter_class, uint8 accepter_level,
		bool accepter_is_bot, uint32 accepter_bot_owner_character_id,
		uint8 inviter_class, uint8 inviter_level, uint32 accepter_zone_id, uint32 accepter_instance_id
	);
};
