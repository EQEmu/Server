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

#include "world/party_state.h"

#include "common/party_state.h"
#include "common/repositories/bot_data_repository.h"
#include "common/strings.h"

#include "world/cliententry.h"
#include "world/clientlist.h"
#include "world/worlddb.h"
#include "world/zoneserver.h"
#include "world/zonelist.h"

#include <algorithm>

#include "common/repositories/character_leadership_abilities_repository.h"

PartyStateManager& PartyStateManager::Instance()
{
	static PartyStateManager instance;
	return instance;
}

static bool IsCharacterOnline(std::string_view character_name)
{
	const auto* cle = ClientList::Instance()->FindCharacter(character_name);
	return cle != nullptr && (cle->Online() == InZone || cle->Online() == Zoning);
}

void PartyStateManager::HandleCharacterOffline(std::string_view character_name)
{
	WorldGroup* group = FindGroupByCharacter(character_name);
	if (group != nullptr)
		group->SetMemberOffline(character_name);

	WorldRaid* raid = FindRaidByCharacter(character_name);
	if (raid != nullptr) {
		WorldRaidMember* m = raid->FindMember(character_name);
		if (m != nullptr) {
			auto next_online = std::ranges::find_if(raid->members, [&character_name](const WorldRaidMember& nm) {
				return nm.InUse() && nm.kind != RaidMemberKind::RaidMemberBot
					&& !Strings::EqualFold(nm.name, character_name) && IsCharacterOnline(nm.name);
			});

			if (next_online != raid->members.end()) {
				if (m->roles & RaidBitmaskLeader) {
					m->roles &= ~RaidBitmaskLeader;
					next_online->roles |= RaidBitmaskLeader;
					raid->leader_name = next_online->name;
					raid->PushLeaderChanged();
				}

				raid->PushMemberFieldsChanged(*m);
			} else {
				raid->Disband();
			}
		}
	}
}

void PartyStateManager::HandleCharacterOnline(std::string_view character_name)
{
	const WorldGroup* group = FindGroupByCharacter(character_name);
	if (group != nullptr)
		group->SetMemberOnline(character_name);

	WorldRaid* raid = FindRaidByCharacter(character_name);
	if (raid != nullptr) {
		WorldRaidMember* m = raid->FindMember(character_name);
		if (m != nullptr)
			raid->PushMemberFieldsChanged(*m);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Group-side
// ---------------------------------------------------------------------------------------------------------------------

WorldGroup* PartyStateManager::FindGroupByCharacter(std::string_view character_name)
{
	WorldGroup* found = nullptr;

	auto idx_it = m_character_group_id.find(character_name);
	if (idx_it != m_character_group_id.end()) {
		auto group_it = m_groups.find(idx_it->second);
		if (group_it != m_groups.end())
			found = &group_it->second;
	}

	return found;
}

const WorldGroup* PartyStateManager::FindGroupByCharacter(std::string_view character_name) const
{
	const WorldGroup* found = nullptr;

	auto idx_it = m_character_group_id.find(character_name);
	if (idx_it != m_character_group_id.end()) {
		auto group_it = m_groups.find(idx_it->second);
		if (group_it != m_groups.end())
			found = &group_it->second;
	}

	return found;
}

void PartyStateManager::IndexMember(std::string_view name, uint32 gid)
{
	auto it = m_character_group_id.find(name);
	if (it != m_character_group_id.end())
		it->second = gid;
	else
		m_character_group_id.emplace(name, gid);
}

void PartyStateManager::EraseFromIndex(const std::vector<std::string_view>& names)
{
	for (const auto& name : names) {
		auto it = m_character_group_id.find(name);
		if (it != m_character_group_id.end())
			m_character_group_id.erase(it);
	}
}

void PartyStateManager::EraseGroupAndIndex(uint32 gid, const std::vector<std::string_view>& departed_names)
{
	EraseFromIndex(departed_names);
	m_groups.erase(gid);
}

uint32 PartyStateManager::GetGroupIdOf(std::string_view character_name) const
{
	auto idx_it = m_character_group_id.find(character_name);
	return idx_it != m_character_group_id.end() ? idx_it->second : 0;
}

bool PartyStateManager::IsGroupLeader(std::string_view character_name) const
{
	const WorldGroup* group = FindGroupByCharacter(character_name);
	return group != nullptr && Strings::EqualFold(group->leader_name, character_name);
}

std::vector<const WorldGroupMember*> PartyStateManager::GetGroupMembersOf(std::string_view character_name) const
{
	std::vector<const WorldGroupMember*> result;
	const WorldGroup* group = FindGroupByCharacter(character_name);
	if (group != nullptr)
		for (const auto& m : group->members)
			if (m.InUse())
				result.push_back(&m);

	return result;
}

std::string_view PartyStateManager::GetGroupLeaderName(uint32 gid) const
{
	std::string_view leader_name;

	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		leader_name = it->second.leader_name;

	return leader_name;
}

bool PartyStateManager::IsMemberOf(uint32 gid, std::string_view character_name) const
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		return it->second.IsMemberOf(character_name);

	return false;
}

uint32 PartyStateManager::GetGroupMemberCount(uint32 gid) const
{
	auto it = m_groups.find(gid);
	return it != m_groups.end() ? it->second.Count() : 0;
}

void PartyStateManager::ForceDisbandGroup(uint32 gid)
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		it->second.Disband();
}

void PartyStateManager::SetRaidLink(uint32 gid, const RaidLink &link)
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		it->second.SetRaidLink(link);
}

void PartyStateManager::SetGroupLocked(uint32 gid, bool locked)
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		it->second.SetLocked(locked);
}

uint32 PartyStateManager::AllocateGroupId()
{
	uint32 start = 0, end = 0;
	ZSList::Instance()->NextGroupIDs(start, end);
	return start;
}

WorldGroup& PartyStateManager::InstallGroup(uint32 gid, WorldGroup group)
{
	WorldGroup& installed = m_groups[gid] = std::move(group);
	installed.SetID(gid);
	return installed;
}

WorldGroup* PartyStateManager::GetGroupById(uint32 gid)
{
	auto it = m_groups.find(gid);
	return it != m_groups.end() ? &it->second : nullptr;
}

void PartyStateManager::AddMemberToGroup(uint32 gid, WorldGroupMember member)
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		it->second.AddMember(std::move(member));
}

// TODO: far to many arguments here
void PartyStateManager::AddMemberToGroup(uint32 gid, std::string_view name, uint8 class_, uint8 level, bool is_bot,
	uint32 owner_character_id, bool is_merc, std::string_view owner_name, uint32 zone_id, uint16 instance_id)
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end()) {
		GroupMemberKind kind = is_bot ? GroupMemberKind::GroupMemberBot
			: (is_merc ? GroupMemberKind::GroupMemberMerc : GroupMemberKind::GroupMemberPlayer);

		it->second.AddMember(name, class_, level, kind, owner_character_id, owner_name, zone_id, instance_id);
	}
}

void PartyStateManager::RemoveMemberFromGroup(uint32 gid, std::string_view name)
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		it->second.RemoveMember(name);
}

bool PartyStateManager::GroupExists(uint32 gid) const
{
	return m_groups.contains(gid);
}

void PartyStateManager::HandleGroupStateRequest(std::string_view character_name, ZoneServer* requester)
{
	WorldGroup* group = FindGroupByCharacter(character_name);
	if (group != nullptr)
		group->HandleStateRequest(character_name, requester);
}

void PartyStateManager::HandleRefreshRequest(uint32 gid) const
{
	auto it = m_groups.find(gid);
	if (it != m_groups.end())
		it->second.PushFull();
}

void PartyStateManager::HandleGroupAction(const ServerPacket *pack)
{
	if (pack->size >= sizeof(ServerGroupActionHeader_Struct)) {
		const auto *header = reinterpret_cast<ServerGroupActionHeader_Struct*>(pack->pBuffer);

		switch (header->action) {
		case GroupActionType::GroupActionFollow:
			if (pack->size == sizeof(ServerGroupActionFollow_Struct))
				HandleFollow(*reinterpret_cast<ServerGroupActionFollow_Struct*>(pack->pBuffer));
			break;
		case GroupActionType::GroupActionAddMember:
			if (pack->size == sizeof(ServerGroupActionAddMember_Struct))
				HandleAddMember(*reinterpret_cast<ServerGroupActionAddMember_Struct*>(pack->pBuffer));
			break;
		case GroupActionType::GroupActionDisband:
			if (pack->size == sizeof(ServerGroupActionTarget_Struct))
				HandleDisband(*reinterpret_cast<ServerGroupActionTarget_Struct*>(pack->pBuffer));
			break;
		case GroupActionType::GroupActionMentor:
			if (pack->size == sizeof(ServerGroupActionMentor_Struct))
				HandleMentor(*reinterpret_cast<ServerGroupActionMentor_Struct*>(pack->pBuffer));
			break;
		case GroupActionType::GroupActionRefresh:
			HandleRefreshRequest(header->gid);
			break;
		case GroupActionType::GroupActionMakeLeader:
		case GroupActionType::GroupActionDelegateMainTank:
		case GroupActionType::GroupActionUndelegateMainTank:
		case GroupActionType::GroupActionDelegateMainAssist:
		case GroupActionType::GroupActionUndelegateMainAssist:
		case GroupActionType::GroupActionDelegatePuller:
		case GroupActionType::GroupActionUndelegatePuller:
		case GroupActionType::GroupActionDelegateMasterLooter:
		case GroupActionType::GroupActionUndelegateMasterLooter:
		case GroupActionType::GroupActionDelegateMarkNPC:
		case GroupActionType::GroupActionUndelegateMarkNPC:
			if (pack->size == sizeof(ServerGroupActionTarget_Struct)) {
				const auto &in = *reinterpret_cast<ServerGroupActionTarget_Struct*>(pack->pBuffer);
				HandleLeadership(in.header, in.target_name, header->action);
			}
			break;
		case GroupActionType::GroupActionUpdateAA:
			if (pack->size == sizeof(ServerGroupActionBasic_Struct)) {
				const auto &in = *reinterpret_cast<ServerGroupActionBasic_Struct*>(pack->pBuffer);
				HandleLeadership(in.header, "", header->action);
			}
			break;
		}
	}
}

uint32 PartyStateManager::EffectiveRaidIdOf(std::string_view character_name) const
{
	const WorldGroup* group = FindGroupByCharacter(character_name);
	return group != nullptr && group->raid_link.IsLinked() ? group->raid_link.raid_id : GetRaidIdOf(character_name);
}

void PartyStateManager::MergeIntoGroup(
	std::string_view inviter_name, std::string_view target_name,
	uint32 requestor_character_id, uint8 inviter_class, uint8 inviter_level,
	const GroupLeadershipAA_Struct *new_group_leadership_aa
)
{
	WorldGroup *target_group = FindGroupByCharacter(target_name);
	if (!Strings::EqualFold(target_group->leader_name, target_name)) {
		SendPartyActionFailure(requestor_character_id, "You can only invite an ungrouped player or group leader.");
	} else {
		WorldGroup *inviter_group = FindGroupByCharacter(inviter_name);
		if (inviter_group == target_group) {
			SendPartyActionFailure(requestor_character_id, "You are already in a party with that person.");
		} else {
			bool inviter_locked = inviter_group != nullptr && inviter_group->raid_link.locked;
			uint32 inviter_size = inviter_group != nullptr ? inviter_group->Count() : 1;
			bool target_linked  = target_group->raid_link.IsLinked();

			uint32 inviter_rid = EffectiveRaidIdOf(inviter_name);

			bool same_raid = target_linked && inviter_rid != 0 && inviter_rid == target_group->raid_link.raid_id;
			bool needs_new_attach = !same_raid && inviter_group == nullptr && inviter_rid != 0;

			if (target_linked && !same_raid) {
				SendPartyActionFailure(requestor_character_id, "You cannot invite someone from another raid group.");
			} else if (inviter_locked || target_group->raid_link.locked) {
				SendPartyActionFailure(requestor_character_id, "You cannot merge groups while one of them is locked.");
			} else if (inviter_size + target_group->Count() > MAX_GROUP_MEMBERS) {
				SendPartyActionFailure(requestor_character_id, "That group is full.");
			} else if (needs_new_attach && !CanAttachNewGroup(inviter_rid)) {
				SendPartyActionFailure(requestor_character_id, "Your raid has no room for a new group.");
			} else if (same_raid || inviter_group != nullptr) {
				// Survivor already exists live -- merge loser's members into
				// it one at a time, same push-per-join as any ordinary
				// post-creation invite accept.
				WorldGroup *survivor = same_raid ? target_group : inviter_group;
				WorldGroup *loser    = same_raid ? inviter_group : target_group; // loser may be null (same_raid, inviter ungrouped)

				std::vector<WorldGroupMember> moved_in;
				if (loser != nullptr) {
					loser->ForEachMember([&moved_in](const WorldGroupMember& member) { moved_in.push_back(member); });
					loser->Disband();
				} else {
					WorldGroupMember solo_inviter;
					solo_inviter.name   = inviter_name;
					solo_inviter.class_ = inviter_class;
					solo_inviter.level  = inviter_level;
					moved_in.push_back(solo_inviter);
				}

				for (auto &mm : moved_in)
					survivor->AddMember(std::move(mm));
			} else {
				// Inviter is ungrouped and there's no existing slot to reuse
				// -- build the whole new group (inviter as leader + target's
				// entire membership) with one GroupBuilder and commit it
				// once, instead of standing up a 1-person group and then
				// drip-feeding joins.
				std::vector<WorldGroupMember> moved_in;
				target_group->ForEachMember([&moved_in](const WorldGroupMember& member) { moved_in.push_back(member); });
				target_group->Disband();

				GroupBuilder builder = NewGroup();
				builder.AddMember(inviter_name, inviter_class, inviter_level, GroupMemberKind::GroupMemberPlayer, 0, "", 0, 0);
				for (const auto& mm : moved_in)
					builder.AddMember(mm.name, mm.class_, mm.level, mm.kind, mm.owner_character_id, mm.owner_name, mm.zone_id, mm.instance_id);
				if (new_group_leadership_aa != nullptr)
					builder.SetLeadershipAA(*new_group_leadership_aa);

				uint32 gid = builder.Push();
				if (needs_new_attach)
					AttachExistingGroup(inviter_rid, gid);
			}
		}
	}
}

void PartyStateManager::HandleFollow(const ServerGroupActionFollow_Struct& in)
{
	std::string inviter_name = in.inviter_name;
	std::string accepter_name = in.header.requestor_name;

	if (FindGroupByCharacter(accepter_name) != nullptr) {
		MergeIntoGroup(inviter_name, accepter_name, in.header.requestor_character_id, in.inviter_class, in.inviter_level, nullptr);
	} else {
		uint32 inviter_rid = EffectiveRaidIdOf(inviter_name);
		uint32 accepter_rid = GetRaidIdOf(accepter_name);

		if (accepter_rid != 0 && accepter_rid != inviter_rid) {
			SendPartyActionFailure(in.header.requestor_character_id, "You cannot invite someone from another raid group.");
		} else if (inviter_rid != 0 && !CanAttachNewGroup(inviter_rid)) {
			SendPartyActionFailure(in.header.requestor_character_id, "Your raid has no room for a new group.");
		} else {
			uint32 gid = NewGroup()
				.AddMember(inviter_name, in.inviter_class, in.inviter_level, GroupMemberKind::GroupMemberPlayer, 0, "", 0, 0)
				.AddMember(accepter_name, in.accepter_class, in.accepter_level, GroupMemberKind::GroupMemberPlayer, 0, "", in.header.zoneid, in.header.instance_id)
				.Push();

			if (inviter_rid != 0)
				AttachExistingGroup(inviter_rid, gid);
		}
	}
}

void PartyStateManager::HandleAddMember(const ServerGroupActionAddMember_Struct& in)
{
	std::string requestor_name = in.header.requestor_name;
	std::string target_name = in.target_name;

	if (FindGroupByCharacter(target_name) != nullptr) {
		MergeIntoGroup(requestor_name, target_name, in.header.requestor_character_id, in.requestor_class, in.requestor_level, &in.requestor_group_aa);
	} else {
		uint32 requestor_rid = EffectiveRaidIdOf(requestor_name);
		uint32 target_rid = GetRaidIdOf(target_name);

		if (target_rid != 0 && target_rid != requestor_rid) {
			SendPartyActionFailure(in.header.requestor_character_id, "You cannot invite someone from another raid group.");
		} else if (requestor_rid != 0 && !CanAttachNewGroup(requestor_rid)) {
			SendPartyActionFailure(in.header.requestor_character_id, "Your raid has no room for a new group.");
		} else {
			GroupMemberKind target_kind = in.target_is_bot ? GroupMemberKind::GroupMemberBot
				: (in.target_is_merc ? GroupMemberKind::GroupMemberMerc : GroupMemberKind::GroupMemberPlayer);

			uint32 gid = NewGroup()
				.AddMember(requestor_name, in.requestor_class, in.requestor_level, GroupMemberKind::GroupMemberPlayer, 0, "", in.header.zoneid, in.header.instance_id)
				.AddMember(target_name, in.target_class, in.target_level, target_kind, in.target_owner_character_id, in.target_owner_name, in.header.zoneid, in.header.instance_id)
				.SetLeadershipAA(in.requestor_group_aa)
				.Push();

			if (requestor_rid != 0)
				AttachExistingGroup(requestor_rid, gid);
		}
	}
}

void PartyStateManager::HandleDisband(const ServerGroupActionTarget_Struct& in)
{
	if (in.target_name[0] == '\0') {
		ForceDisbandGroup(in.header.gid);
	} else {
		auto idx_it = m_character_group_id.find(in.target_name);
		if (idx_it != m_character_group_id.end() && idx_it->second == in.header.gid)
			RemoveMemberFromGroup(idx_it->second, in.target_name);
	}
}

void PartyStateManager::HandleLeadership(const ServerGroupActionHeader_Struct& header, std::string_view target_name, GroupActionType action)
{
	const auto it = m_groups.find(header.gid);
	if (it != m_groups.end())
		it->second.HandleLeadershipAction(header, target_name, action);
}

void PartyStateManager::HandleMentor(const ServerGroupActionMentor_Struct& in)
{
	auto it = m_groups.find(in.header.gid);
	if (it != m_groups.end()) {
		if (in.clear)
			it->second.SetMentor("", 0);
		else
			it->second.SetMentor(in.mentoree_name, in.mentor_percent);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Raid-side
// ---------------------------------------------------------------------------------------------------------------------

// A raid invite's target could be anywhere -- zone fills in
// ServerRaidActionInvite_Struct's fields itself when it can resolve
// target_name locally (a live Client or Bot found in that zone); when it
// can't (`!in.resolved`), world enriches from its own live data instead of
// asking a second time: ClientList for a player (always fresh, zone-
// reported on every connect/update, see cliententry.cpp), or bot_data for a
// bot's identity/ownership only. There's no live registry of bots the way
// ClientList tracks connected characters (bots aren't network sessions), so
// a currently-unspawned bot's class/level simply aren't knowable -- they're
// left at 0/0, exactly like an offline player invitee also never gets
// class/level resolved. Either way -- zone-resolved or world-enriched --
// the result feeds the exact same AddPartyToRaid call.
namespace {
struct ResolvedInviteTarget {
	bool           found = false;
	RaidMemberKind kind = RaidMemberKind::RaidMemberPlayer;
	uint32         owner_character_id = 0;
	uint8          class_ = 0;
	uint8          level = 0;
};

ResolvedInviteTarget ResolveInviteTarget(const ServerRaidActionInvite_Struct& in)
{
	ResolvedInviteTarget target;

	if (in.resolved) {
		target.found              = true;
		target.kind               = in.kind;
		target.owner_character_id = in.owner_character_id;
		target.class_             = in.target_class;
		target.level              = in.target_level;
	} else if (auto* cle = ClientList::Instance()->FindCharacter(in.target_name)) {
		target.found  = true;
		target.kind   = RaidMemberKind::RaidMemberPlayer;
		target.class_ = cle->class_();
		target.level  = cle->level();
	} else {
		auto rows = BotDataRepository::GetWhere(database, fmt::format("`name` = '{}'", Strings::Escape(std::string_view(in.target_name))));
		if (!rows.empty()) {
			target.found              = true;
			target.kind               = RaidMemberKind::RaidMemberBot;
			target.owner_character_id = rows[0].owner_id;
			// class_/level stay 0/0 -- unknown, matches offline-player treatment.
		}
	}

	return target;
}
}

void PartyStateManager::IndexRaidMember(std::string_view name, uint32 rid)
{
	auto it = m_character_raid_id.find(name);
	if (it != m_character_raid_id.end())
		it->second = rid;
	else
		m_character_raid_id.emplace(name, rid);
}

void PartyStateManager::EraseFromRaidIndex(const std::vector<std::string_view>& names)
{
	for (const auto& name : names) {
		auto it = m_character_raid_id.find(name);
		if (it != m_character_raid_id.end())
			m_character_raid_id.erase(it);
	}
}

void PartyStateManager::EraseRaidAndIndex(uint32 rid, const std::vector<std::string_view>& departed_names)
{
	EraseFromRaidIndex(departed_names);
	m_raids.erase(rid);
}

WorldRaid* PartyStateManager::FindRaidByCharacter(std::string_view character_name)
{
	WorldRaid* found = nullptr;

	auto idx_it = m_character_raid_id.find(character_name);
	if (idx_it != m_character_raid_id.end()) {
		auto raid_it = m_raids.find(idx_it->second);
		if (raid_it != m_raids.end())
			found = &raid_it->second;
	}

	return found;
}

uint32 PartyStateManager::AllocateRaidId()
{
	uint32 start = 0, end = 0;
	ZSList::Instance()->NextGroupIDs(start, end);
	return start;
}

WorldRaid& PartyStateManager::InstallRaid(uint32 rid, WorldRaid raid)
{
	WorldRaid& installed = m_raids[rid] = std::move(raid);
	installed.SetID(rid);
	return installed;
}

bool PartyStateManager::IsRaidMember(std::string_view character_name) const
{
	return m_character_raid_id.contains(character_name);
}

uint32 PartyStateManager::GetRaidIdOf(std::string_view character_name) const
{
	auto it = m_character_raid_id.find(character_name);
	return it != m_character_raid_id.end() ? it->second : 0;
}

RaidLink PartyStateManager::MakeRaidLink(uint32 rid, uint32 group_number, bool locked)
{
	RaidLink link;
	link.raid_id = rid;
	link.slot    = static_cast<uint8>(group_number);
	link.locked  = locked;
	link.on_member_departed = [this](uint32 owner_rid, std::string_view character_name) {
		HandleSubgroupMemberDeparted(owner_rid, character_name);
	};
	link.on_member_joined = [this](uint32 owner_rid, const WorldGroupMember &new_member) {
		HandleSubgroupMemberJoined(owner_rid, new_member);
	};

	return link;
}

void PartyStateManager::LinkSubgroup(uint32 gid, uint32 rid, uint32 group_number, bool locked)
{
	SetRaidLink(gid, MakeRaidLink(rid, group_number, locked));
}

bool PartyStateManager::CanAttachNewGroup(uint32 rid) const
{
	auto it = m_raids.find(rid);
	return it != m_raids.end() && it->second.GetFreeGroup() < MAX_RAID_GROUPS;
}

void PartyStateManager::AttachExistingGroup(uint32 rid, uint32 gid)
{
	auto it = m_raids.find(rid);
	if (it != m_raids.end()) {
		WorldRaid &raid = it->second;
		uint32 free_group_id = raid.GetFreeGroup();
		if (free_group_id < MAX_RAID_GROUPS) {
			raid.subgroup_group_ids[free_group_id] = gid;
			LinkSubgroup(gid, rid, free_group_id, raid.locked);
		}
	}
}

void PartyStateManager::HandleRaidStateRequest(std::string_view character_name, ZoneServer* requester)
{
	WorldRaid* raid = FindRaidByCharacter(character_name);
	if (raid != nullptr)
		raid->HandleStateRequest(character_name, requester);
}

void PartyStateManager::HandleSubgroupMemberDeparted(uint32 rid, std::string_view character_name)
{
	auto it = m_raids.find(rid);
	if (it != m_raids.end()) {
		WorldRaid &raid = it->second;

		WorldRaidMember *m = raid.FindMember(character_name);
		if (m != nullptr) {
			for (uint32 &gid : raid.subgroup_group_ids)
				if (gid != 0 && !GroupExists(gid))
					gid = 0;

			raid.PushMemberFieldsChanged(*m);
		}
	}
}

void PartyStateManager::HandleSubgroupMemberJoined(uint32 rid, const WorldGroupMember &new_member)
{
	auto it = m_raids.find(rid);
	if (it != m_raids.end()) {
		WorldRaid &raid = it->second;
		if (raid.FindMember(new_member.name) == nullptr)
			raid.AddMember(new_member);
	}
}

void PartyStateManager::AddGroupToRaid(uint32 requestor_id, std::string_view inviter_name, std::string_view invitee_name)
{
	if (IsRaidMember(invitee_name)) {
		WorldRaid* raid = FindRaidByCharacter(inviter_name);
		if (raid == nullptr) {
			// no raid yet, need to create it
			NewRaid()
			.AddMember(inviter_name, class_, level, RaidMemberKind::RaidMemberPlayer, 0, zone_id, instance_id);
		}
	} else {
		SendPartyActionFailure(requestor_id, fmt::format("{} is already in a raid.", invitee_name));
	}
}

void PartyStateManager::AddPartyToRaid(
	std::string_view inviter_name,
	std::string_view accepter_name, uint8 accepter_class, uint8 accepter_level,
	bool accepter_is_bot, uint32 accepter_bot_owner_character_id,
	uint8 inviter_class, uint8 inviter_level,
	uint32 accepter_zone_id, uint32 accepter_instance_id
)
{
	WorldRaid *raid = FindRaidByCharacter(inviter_name);

	if (raid != nullptr) {
		// Topology 1: inviter already has a raid.
		std::vector<const WorldGroupMember*> accepter_members = GetGroupMembersOf(accepter_name);
		uint32 incoming_count = accepter_members.empty() ? 1 : static_cast<uint32>(accepter_members.size());
		if (incoming_count + raid->Count() <= MAX_RAID_MEMBERS) {
			if (!accepter_members.empty() && IsGroupLeader(accepter_name)) {
				// Bringing a whole group in needs an actual empty raid
				// subgroup slot -- MAX_RAID_MEMBERS headroom (checked
				// above) isn't the same thing as slot availability. The
				// group itself already exists (however it was created) --
				// this just attaches it, the same ordinary way anything
				// attaches an existing group to a raid.
				if (raid->GetFreeGroup() < MAX_RAID_GROUPS) {
					uint32 accepter_gid = GetGroupIdOf(accepter_name);
					AttachExistingGroup(raid->GetID(), accepter_gid);
					for (const auto &member : accepter_members)
						raid->AddMember(*member);
				}
			} else {
				uint32 dest_group = raid->group_on_invite ? raid->GetGroupWithRoom() : RAID_GROUPLESS;
				WorldRaidMember* slot = raid->AddMember(
					accepter_name, accepter_class, accepter_level, false, false,
					accepter_is_bot ? RaidMemberKind::RaidMemberBot : RaidMemberKind::RaidMemberPlayer,
					accepter_bot_owner_character_id, accepter_zone_id, accepter_instance_id
				);

				if (slot != nullptr && dest_group < MAX_RAID_GROUPS)
					raid->JoinRealSubgroup(dest_group, accepter_name, accepter_class, accepter_level, accepter_is_bot,
						accepter_bot_owner_character_id, accepter_zone_id, accepter_instance_id);
			}
		}
	} else {
		// Topology 2: inviter has no raid -- create one, led by the inviter.
		// Any existing group involved is adopted wholesale (AdoptGroup);
		// neither one is built by hand here.
		std::vector<const WorldGroupMember*> inviter_members  = GetGroupMembersOf(inviter_name);
		std::vector<const WorldGroupMember*> accepter_members = GetGroupMembersOf(accepter_name);

		RaidBuilder builder = NewRaid();
		builder.SetLootType(4); // matches Raid::Raid()'s default LootType

		if (!inviter_members.empty()) {
			builder.AdoptGroup(GetGroupIdOf(inviter_name));
			builder.SetLeader(inviter_name);
		} else {
			builder.AddMember(inviter_name, inviter_class, inviter_level, RaidMemberKind::RaidMemberPlayer, 0, 0, 0);
			builder.SetLeader(inviter_name);
		}

		if (!accepter_members.empty() && IsGroupLeader(accepter_name)) {
			builder.AdoptGroup(GetGroupIdOf(accepter_name));
		} else {
			builder.AddMember(
				accepter_name, accepter_class, accepter_level,
				accepter_is_bot ? RaidMemberKind::RaidMemberBot : RaidMemberKind::RaidMemberPlayer,
				accepter_bot_owner_character_id, accepter_zone_id, accepter_instance_id
			);
		}

		builder.Commit();
	}
}

void PartyStateManager::PushInvite(uint32 invitee_character_id, std::string_view invitee_name, std::string_view inviter_name)
{
	if (invitee_character_id != 0) {
		auto *cle = ClientList::Instance()->FindCLEByCharacterID(invitee_character_id);
		if (cle != nullptr && cle->Server() != nullptr) {
			ServerRaidInvitePushed_Struct out{};
			out.invitee_character_id = invitee_character_id;
			Strings::Copy(out.invitee_name, invitee_name);
			Strings::Copy(out.inviter_name, inviter_name);

			auto pack = new ServerPacket(ServerOP_RaidInvitePushed, sizeof(ServerRaidInvitePushed_Struct));
			memcpy(pack->pBuffer, &out, sizeof(ServerRaidInvitePushed_Struct));
			cle->Server()->SendPacket(pack);
			safe_delete(pack);
		}
	}
}

void PartyStateManager::HandleActionDisband(const ServerRaidActionBasic_Struct &in)
{
	auto it = m_raids.find(in.header.rid);
	if (it != m_raids.end())
		it->second.Disband();
}

void PartyStateManager::HandleActionInvite(const ServerRaidActionInvite_Struct &in)
{
	std::string inviter_name = in.header.requestor_name;
	std::string invitee_name = in.target_name;

	// /raidinvite is leader-only -- a regular /invite issued by a raid
	// member (grouped or ungrouped) is a *group* action instead, routed
	// through HandleFollow/HandleAddMember, never here.
	WorldRaid *requestor_raid = FindRaidByCharacter(inviter_name);
	ResolvedInviteTarget target = ResolveInviteTarget(in);
	if (requestor_raid != nullptr && !Strings::EqualFold(requestor_raid->leader_name, inviter_name)) {
		SendPartyActionFailure(in.header.requestor_character_id, "Only the raid leader can invite to the raid.");
	} else if (IsRaidMember(invitee_name)) {
		SendPartyActionFailure(in.header.requestor_character_id, fmt::format("{} is already in a raid.", invitee_name));
	} else if (target.found && target.kind == RaidMemberKind::RaidMemberBot) {
		// A bot can't accept/decline for itself -- unlike a player, this
		// adds it to the raid immediately, subject only to the same
		// ownership/room checks as any other bot-add.
		WorldRaid *raid = requestor_raid;
		bool owner_ok = (target.owner_character_id == in.header.requestor_character_id);
		if (!owner_ok && raid != nullptr) {
			owner_ok = std::ranges::any_of(raid->members, [&target](const WorldRaidMember& m) {
				if (!m.InUse() || m.kind == RaidMemberKind::RaidMemberBot)
					return false;
				auto *cle = ClientList::Instance()->FindCharacter(m.name);
				return cle != nullptr && cle->CharID() == target.owner_character_id;
			});
		}

		if (!owner_ok) {
			SendPartyActionFailure(in.header.requestor_character_id, fmt::format("{}'s owner needs to be in your raid to be able to invite them.", invitee_name));
		} else if (raid != nullptr && raid->Count() >= MAX_RAID_MEMBERS) {
			SendPartyActionFailure(in.header.requestor_character_id, "The raid is full.");
		} else {
			// The requestor is the one issuing this packet -- always
			// online, so their CLE (kept live-fresh by zone on every
			// connect/update) always has current class/level, no DB
			// read needed.
			auto *inviter_cle = ClientList::Instance()->FindCLEByCharacterID(in.header.requestor_character_id);
			uint8 inviter_class = inviter_cle != nullptr ? inviter_cle->class_() : 0;
			uint8 inviter_level = inviter_cle != nullptr ? inviter_cle->level() : 0;
			AddPartyToRaid(inviter_name, invitee_name, target.class_, target.level, true, target.owner_character_id, inviter_class, inviter_level, in.header.zoneid, in.header.instance_id);
		}
	} else if (!target.found) {
		// Doesn't exist as a player or a bot anywhere -- silently a no-op,
		// same as inviting an offline player already was.
	} else {
		std::vector<const WorldGroupMember*> invitee_members = GetGroupMembersOf(invitee_name);
		bool inviter_in_group = std::ranges::any_of(invitee_members, [&inviter_name](const WorldGroupMember* m) {
			return Strings::EqualFold(m->name, inviter_name);
		});

		if (inviter_in_group) {
			SendPartyActionFailure(in.header.requestor_character_id, "You are already in a party with that person.");
		} else if (!invitee_members.empty() && !IsGroupLeader(invitee_name)) {
			SendPartyActionFailure(in.header.requestor_character_id, "You can only invite an ungrouped player or group leader to join your raid.");
		} else {
			auto *invitee_cle = ClientList::Instance()->FindCharacter(invitee_name);
			if (invitee_cle != nullptr) {
				m_pending_invite_inviter_name[invitee_cle->CharID()] = inviter_name;
				PushInvite(invitee_cle->CharID(), invitee_name, inviter_name);
			}
		}
	}
}

void PartyStateManager::HandleActionAcceptInvite(const ServerRaidActionTarget_Struct &in)
{
	std::string accepter_name = in.header.requestor_name;

	if (IsRaidMember(accepter_name)) {
		SendPartyActionFailure(in.header.requestor_character_id, "You are already in a raid.");
	} else {
		auto pending_it = m_pending_invite_inviter_name.find(in.header.requestor_character_id);
		if (pending_it == m_pending_invite_inviter_name.end()) {
			SendPartyActionFailure(in.header.requestor_character_id, "You do not have a pending raid invite.");
		} else {
			std::string inviter_name = pending_it->second;
			m_pending_invite_inviter_name.erase(pending_it);

			// The accepter is the one issuing this packet -- always online,
			// so their CLE always has current class/level.
			auto *accepter_cle = ClientList::Instance()->FindCLEByCharacterID(in.header.requestor_character_id);
			uint8 accepter_class = accepter_cle != nullptr ? accepter_cle->class_() : 0;
			uint8 accepter_level = accepter_cle != nullptr ? accepter_cle->level() : 0;

			// The inviter's own class/level, best-effort: if they're not
			// currently online, this degrades to 0/0 the same as it always
			// has -- but works regardless of which zone they're in.
			uint8 inviter_class = 0, inviter_level = 0;
			if (auto *inviter_cle = ClientList::Instance()->FindCharacter(inviter_name)) {
				inviter_class = inviter_cle->class_();
				inviter_level = inviter_cle->level();
			}

			AddPartyToRaid(inviter_name, accepter_name, accepter_class, accepter_level, false, 0, inviter_class,
				inviter_level, in.header.zoneid, in.header.instance_id);
		}
	}
}

void PartyStateManager::HandleActionRemoveMember(const ServerRaidActionTarget_Struct &in)
{
	auto it = m_raids.find(in.header.rid);
	if (it != m_raids.end()) {
		WorldRaid& raid = it->second;
		std::string target_name = in.target_name;

		WorldRaidMember* target_member = raid.FindMember(target_name);
		bool allowed = Strings::EqualFold(in.header.requestor_name, raid.leader_name)
			|| Strings::EqualFold(in.header.requestor_name, target_name);
		if (!allowed && target_member != nullptr && target_member->kind == RaidMemberKind::RaidMemberBot) {
			auto *cle = ClientList::Instance()->FindCLEByCharacterID(in.header.requestor_character_id);
			allowed = (cle != nullptr && cle->CharID() == target_member->owner_character_id);
		}

		if (!allowed) {
			SendPartyActionFailure(in.header.requestor_character_id, "You do not have permission to remove that member.");
		} else if (target_member != nullptr) {
			raid.RemoveMember(target_name);

			auto refreshed = m_raids.find(in.header.rid);
			if (refreshed != m_raids.end()) {
				// Disband if the raid is now empty, or if the departed
				// member was the raid leader and RemoveMember couldn't find
				// a non-bot member to promote -- a raid with only bots left
				// has no one who could ever lead it again. FindMember
				// returns nullptr here exactly in that case: RemoveMember
				// only ever reassigns leader_name away from a departing
				// leader, never clears it, so a stale name that matches no
				// current member is the signal.
				bool leaderless = refreshed->second.FindMember(refreshed->second.leader_name) == nullptr;
				if (refreshed->second.Count() == 0 || leaderless)
					refreshed->second.Disband();
			}
		}
	}
}

void PartyStateManager::HandleActionMoveGroup(const ServerRaidActionTargetValue_Struct &in)
{
	auto it = m_raids.find(in.header.rid);
	if (it != m_raids.end()) {
		WorldRaid& raid = it->second;
		std::string target_name = in.target_name;

		// Leader can move anyone; a member can also move themselves.
		bool allowed = Strings::EqualFold(in.header.requestor_name, raid.leader_name)
			|| Strings::EqualFold(in.header.requestor_name, target_name);
		if (!allowed) {
			SendPartyActionFailure(in.header.requestor_character_id, "You do not have permission to move that raid member.");
		} else {
			WorldRaidMember* m = raid.FindMember(target_name);
			if (m != nullptr) {
				uint32 old_group = raid.FindMemberGroupNumber(target_name);
				uint32 dest_group = (in.value >= MAX_RAID_GROUPS) ? RAID_GROUPLESS : in.value;
				if (dest_group != old_group) {
					if (dest_group < MAX_RAID_GROUPS && raid.GroupCount(dest_group) >= MAX_GROUP_MEMBERS) {
						SendPartyActionFailure(in.header.requestor_character_id, "That group is full.");
					} else {
						if (old_group < MAX_RAID_GROUPS && raid.subgroup_group_ids[old_group] != 0) {
							uint32 old_gid = raid.subgroup_group_ids[old_group];
							RemoveMemberFromGroup(old_gid, target_name);
							if (!GroupExists(old_gid))
								raid.subgroup_group_ids[old_group] = 0;
						}

						if (dest_group < MAX_RAID_GROUPS)
							raid.JoinRealSubgroup(dest_group, target_name, m->class_, m->level, m->kind == RaidMemberKind::RaidMemberBot,
								m->owner_character_id, m->zone_id, m->instance_id);

						raid.PushGroupMoved(target_name, dest_group);
					}
				}
			}
		}
	}
}

void PartyStateManager::HandleRaidAction(const ServerPacket *pack)
{
	if (pack->size >= sizeof(ServerRaidActionHeader_Struct)) {
		auto *header = reinterpret_cast<ServerRaidActionHeader_Struct*>(pack->pBuffer);

		// These 5 don't all require an existing raid (Invite/AcceptInvite
		// may be forming a brand-new one, so header->rid is 0/unused for
		// them) -- dispatched here, ahead of the m_raids.find(rid) gate
		// below, each resolving whatever raid state it needs by name
		// instead.
		switch (header->action) {
		case RaidActionType::RaidActionDisband:
			if (pack->size == sizeof(ServerRaidActionBasic_Struct))
				HandleActionDisband(*reinterpret_cast<ServerRaidActionBasic_Struct*>(pack->pBuffer));
			break;
		case RaidActionType::RaidActionInvite:
			if (pack->size == sizeof(ServerRaidActionInvite_Struct))
				HandleActionInvite(*reinterpret_cast<ServerRaidActionInvite_Struct*>(pack->pBuffer));
			break;
		case RaidActionType::RaidActionAcceptInvite:
			if (pack->size == sizeof(ServerRaidActionTarget_Struct))
				HandleActionAcceptInvite(*reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer));
			break;
		case RaidActionType::RaidActionRemoveMember:
			if (pack->size == sizeof(ServerRaidActionTarget_Struct))
				HandleActionRemoveMember(*reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer));
			break;
		case RaidActionType::RaidActionMoveGroup:
			if (pack->size == sizeof(ServerRaidActionTargetValue_Struct))
				HandleActionMoveGroup(*reinterpret_cast<ServerRaidActionTargetValue_Struct*>(pack->pBuffer));
			break;
		default: {
			auto it = m_raids.find(header->rid);
			if (it != m_raids.end()) {
				WorldRaid& raid = it->second;
				bool locked_out = false;

				if (raid.locked) {
					switch (header->action) {
					case RaidActionType::RaidRoleDelegateAssist:
					case RaidActionType::RaidRoleUndelegateAssist:
					case RaidActionType::RaidRoleDelegateMark:
					case RaidActionType::RaidRoleUndelegateMark:
					case RaidActionType::RaidRoleDelegateMasterLooter:
					case RaidActionType::RaidRoleUndelegateMasterLooter:
						SendPartyActionFailure(header->requestor_character_id, "You cannot change raid roles while the raid is locked.");
						locked_out = true;
						break;
					default:
						break;
					}
				}

				if (!locked_out) {
					switch (header->action) {
					case RaidActionType::RaidLeadershipMakeLeader:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.MakeLeader(in.header.requestor_name, in.header.requestor_character_id, in.target_name);
						}
						break;
					case RaidActionType::RaidLeadershipUpdateAA:
						if (pack->size == sizeof(ServerRaidActionBasic_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionBasic_Struct*>(pack->pBuffer);
							raid.UpdateLeadershipAA(in.header.requestor_character_id);
						}
						break;
					// RaidLeadershipRemoveGroupLeader: retired, confirmed a
					// no-op on live too -- deliberately left out of this
					// switch (falls to default). The enum value itself
					// stays in common/servertalk.h; it's meaningful wire
					// history even though nothing handles it.
					case RaidActionType::RaidLeadershipLock:
						if (pack->size == sizeof(ServerRaidActionBasic_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionBasic_Struct*>(pack->pBuffer);
							raid.Lock(in.header.requestor_name, in.header.requestor_character_id);
						}
						break;
					case RaidActionType::RaidLeadershipUnlock:
						if (pack->size == sizeof(ServerRaidActionBasic_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionBasic_Struct*>(pack->pBuffer);
							raid.Unlock(in.header.requestor_name, in.header.requestor_character_id);
						}
						break;
					case RaidActionType::RaidLeadershipSetLootType:
						if (pack->size == sizeof(ServerRaidActionValue_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionValue_Struct*>(pack->pBuffer);
							raid.SetLootType(in.header.requestor_name, in.header.requestor_character_id, in.value);
						}
						break;
					case RaidActionType::RaidLeadershipAddLooter:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.AddLooter(in.header.requestor_name, in.header.requestor_character_id, in.target_name);
						}
						break;
					case RaidActionType::RaidLeadershipRemoveLooter:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.RemoveLooter(in.header.requestor_name, in.header.requestor_character_id, in.target_name);
						}
						break;
					case RaidActionType::RaidLeadershipSetGroupOnInvite:
						if (pack->size == sizeof(ServerRaidActionValue_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionValue_Struct*>(pack->pBuffer);
							raid.SetGroupOnInvite(in.header.requestor_name, in.value != 0);
						}
						break;
					case RaidActionType::RaidRoleDelegateAssist:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.DelegateAssist(in.header.requestor_name, in.header.requestor_character_id, in.target_name);
						}
						break;
					case RaidActionType::RaidRoleUndelegateAssist:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.UndelegateAssist(in.target_name);
						}
						break;
					case RaidActionType::RaidRoleDelegateMark:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.DelegateMark(in.header.requestor_name, in.header.requestor_character_id, in.target_name);
						}
						break;
					case RaidActionType::RaidRoleUndelegateMark:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.UndelegateMark(in.target_name);
						}
						break;
					case RaidActionType::RaidRoleDelegateMasterLooter:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.DelegateMasterLooter(in.header.requestor_name, in.header.requestor_character_id, in.target_name);
						}
						break;
					case RaidActionType::RaidRoleUndelegateMasterLooter:
						if (pack->size == sizeof(ServerRaidActionTarget_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTarget_Struct*>(pack->pBuffer);
							raid.UndelegateMasterLooter(in.target_name);
						}
						break;
					// RaidRoleMarkNPC/RaidRoleClearNPCMarks used to live
					// here -- removed. NPC marking is zone-local only now.
					case RaidActionType::RaidRoleSetMotd:
						if (pack->size == sizeof(ServerRaidActionMotd_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionMotd_Struct*>(pack->pBuffer);
							raid.SetMotd(in.header.requestor_name, in.header.requestor_character_id, in.motd);
						}
						break;
					case RaidActionType::RaidRoleSetNote:
						if (pack->size == sizeof(ServerRaidActionNote_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionNote_Struct*>(pack->pBuffer);
							raid.SetNote(in.header.requestor_name, in.header.requestor_character_id, in.target_name, in.note);
						}
						break;
					case RaidActionType::RaidLeadershipUpdateLevel:
						if (pack->size == sizeof(ServerRaidActionTargetValue_Struct)) {
							auto &in = *reinterpret_cast<ServerRaidActionTargetValue_Struct*>(pack->pBuffer);
							raid.UpdateMemberLevel(in.target_name, static_cast<uint8>(in.value));
						}
						break;
					default:
						break;
					}
				}
			}
			break;
		}
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Group Methods
// ---------------------------------------------------------------------------------------------------------------------

static GroupLeadershipAA_Struct FetchGroupLeadershipAAFromDB(uint32 character_id)
{
	GroupLeadershipAA_Struct result{};

	const auto rows =  CharacterLeadershipAbilitiesRepository::GetWhere(database,
		fmt::format("`id` = {} AND `slot` < {}", character_id, MAX_GROUP_LEADERSHIP_AA_ARRAY));
	for (const auto &row : rows)
		result.ranks[row.slot] = row.rank_;

	return result;
}

ServerPacket* WorldGroup::BuildFullPacket(GroupUpdateType type, std::string_view targeted_character_name) const
{
	const auto pack = new ServerPacket(ServerOP_GroupState, sizeof(ServerGroupState_Struct));
	const auto gs = reinterpret_cast<ServerGroupState_Struct*>(pack->pBuffer);

	gs->header.update_type = type;
	Strings::Copy(gs->character_name, targeted_character_name);
	gs->header.gid = GetID();
	uint8 out_i = 0;
	for (const auto& m : members) {
		if (m.InUse()) {
			Strings::Copy(gs->members[out_i].name, m.name);
			gs->members[out_i].class_ = m.class_;
			gs->members[out_i].level = m.level;
			gs->members[out_i].kind = m.kind;
			gs->members[out_i].owner_character_id = m.owner_character_id;
			Strings::Copy(gs->members[out_i].owner_name, m.owner_name);
			gs->members[out_i].is_offline = m.kind == GroupMemberKind::GroupMemberPlayer && !IsCharacterOnline(m.name);
			gs->members[out_i].roles = m.roles;
			++out_i;
		}
	}

	gs->member_count = out_i;
	Strings::Copy(gs->leader_name, leader_name);
	Strings::Copy(gs->main_tank_name, main_tank_name);
	Strings::Copy(gs->main_assist_name, main_assist_name);
	Strings::Copy(gs->puller_name, puller_name);
	Strings::Copy(gs->npc_marker_name, npc_marker_name);
	Strings::Copy(gs->master_looter_name, EffectiveMasterLooter());
	Strings::Copy(gs->mentoree_name, mentoree_name);
	gs->mentor_percent = mentor_percent;
	gs->leadership_aa = leadership_aa;
	gs->raid_id = raid_link.raid_id;
	gs->raid_slot = raid_link.slot;

	return pack;
}

void WorldGroup::PushFull() const
{
	auto pack = BuildFullPacket();
	RelayToMembers(pack);
	safe_delete(pack);
}

void WorldGroup::PushFullTo(const WorldGroupMember& member, GroupUpdateType type) const
{
	ZoneServer* zs = ZSList::Instance()->FindByZoneIDAndInstanceID(member.zone_id, member.instance_id);
	if (zs != nullptr) {
		auto pack = BuildFullPacket(type, member.name);
		zs->SendPacket(pack);
		safe_delete(pack);
	}
}

void WorldGroup::PushMemberJoined(const WorldGroupMember& new_member) const
{
	ServerGroupMemberJoined_Struct msg{};
	Strings::Copy(msg.member.name, new_member.name);
	msg.member.class_ = new_member.class_;
	msg.member.level = new_member.level;
	msg.member.kind = new_member.kind;
	msg.member.owner_character_id = new_member.owner_character_id;
	Strings::Copy(msg.member.owner_name, new_member.owner_name);
	msg.member.is_offline = new_member.kind == GroupMemberKind::GroupMemberPlayer && !IsCharacterOnline(new_member.name);
	msg.member.roles = new_member.roles;

	PushDelta(msg, GroupUpdateType::MemberJoined);

	size_t zone_count = std::ranges::count_if(members, [&new_member](const WorldGroupMember& member) {
		return new_member.instance_id == member.instance_id && new_member.zone_id == member.zone_id;
	});

	// self zone should always count as 1, save a string comparison this way
	if (zone_count <= 1)
		PushFullTo(new_member, GroupUpdateType::MemberZoned);
}

void WorldGroup::PushMemberLeft(std::string_view departed_name, std::string_view new_leader_name, ZoneServer* departing_zs) const
{
	ServerGroupMemberLeft_Struct msg{};
	Strings::Copy(msg.member_name, departed_name);

	bool departing_zone_covered = departing_zs != nullptr && std::ranges::any_of(members,
		[&departing_zs](const WorldGroupMember& member) {
			return member.instance_id != 0
				? departing_zs->GetInstanceID() == member.instance_id
				: departing_zs->GetZoneID() == member.zone_id;
		});

	PushDelta(msg, GroupUpdateType::MemberLeft);
	if (departing_zs != nullptr && !departing_zone_covered)
		PushDelta(msg, GroupUpdateType::MemberLeft, departing_zs);

	if (!new_leader_name.empty()) {
		ServerGroupLeaderChanged_Struct lmsg{};
		Strings::Copy(lmsg.leader_name, new_leader_name);
		PushDelta(lmsg, GroupUpdateType::LeaderChanged);
		if (departing_zs != nullptr && !departing_zone_covered)
			PushDelta(lmsg, GroupUpdateType::LeaderChanged, departing_zs);
	}
}

void WorldGroup::PushLeaderChanged() const
{
	ServerGroupLeaderChanged_Struct msg{};
	Strings::Copy(msg.leader_name, leader_name);
	PushDelta(msg, GroupUpdateType::LeaderChanged);
}

void WorldGroup::PushRoleChanged(uint8 role_id, std::string_view target_name) const
{
	ServerGroupRoleChanged_Struct msg{};
	msg.role_id = role_id;
	Strings::Copy(msg.target_name, target_name);
	PushDelta(msg, GroupUpdateType::RoleChanged);
}

void WorldGroup::PushOfflineChanged(std::string_view member_name, bool is_offline) const
{
	ServerGroupOfflineChanged_Struct msg{};
	Strings::Copy(msg.member_name, member_name);
	msg.is_offline = is_offline;
	PushDelta(msg, GroupUpdateType::OfflineChanged);
}

void WorldGroup::PushMentorChanged() const
{
	ServerGroupMentorChanged_Struct msg{};
	Strings::Copy(msg.mentoree_name, mentoree_name);
	msg.mentor_percent = mentor_percent;
	PushDelta(msg, GroupUpdateType::MentorChanged);
}

void WorldGroup::PushAAChanged() const
{
	ServerGroupAAChanged_Struct msg{};
	msg.leadership_aa = leadership_aa;
	PushDelta(msg, GroupUpdateType::AAChanged);
}

void WorldGroup::PushGroupDisbanded() const
{
	PushDelta(ServerGroupDisbanded_Struct{}, GroupUpdateType::Disbanded);
}

WorldGroupMember* WorldGroup::PlaceMember(WorldGroupMember member)
{
	WorldGroupMember* slot = nullptr;

	uint8 count = Count();
	if (FindMember(member.name) == nullptr && count < MAX_GROUP_MEMBERS) {
		uint8 group_role = count == 0 ? GroupBitmaskLeader : 0;
		if ((group_role & GroupBitmaskLeader) != 0)
			leader_name = member.name;
		member.roles = group_role;

		slot = FindFreeSlot();
		if (slot != nullptr)
			*slot = std::move(member);
	}

	return slot;
}

void WorldGroup::AddMember(WorldGroupMember member)
{
	WorldGroupMember* slot = PlaceMember(std::move(member));
	if (slot != nullptr) {
		PushMemberJoined(*slot);
		if (raid_link.IsLinked())
			raid_link.on_member_joined(raid_link.raid_id, *slot);

		PartyStateManager::Instance().IndexMember(slot->name, GetID());
	}
}

void WorldGroup::AddMember(std::string_view name, uint8 class_, uint8 level, GroupMemberKind kind,
	uint32 owner_character_id, std::string_view owner_name, uint32 zone_id, uint16 instance_id)
{
	AddMember(WorldGroupMember::Make(name, class_, level, kind, owner_character_id, owner_name, zone_id, instance_id));
}

// ---------------------------------------------------------------------------------------------------------------------
// GroupBuilder
// ---------------------------------------------------------------------------------------------------------------------

GroupBuilder& GroupBuilder::AddMember(WorldGroupMember member)
{
	m_group.PlaceMember(std::move(member));
	return *this;
}

GroupBuilder& GroupBuilder::AddMember(std::string_view name, uint8 class_, uint8 level, GroupMemberKind kind,
	uint32 owner_character_id, std::string_view owner_name, uint32 zone_id, uint16 instance_id)
{
	return AddMember(WorldGroupMember::Make(name, class_, level, kind, owner_character_id, owner_name, zone_id, instance_id));
}

GroupBuilder& GroupBuilder::AddMember(const WorldRaidMember& member)
{
	return AddMember(member.name, member.class_, member.level,
		member.kind == RaidMemberKind::RaidMemberBot ? GroupMemberKind::GroupMemberBot : GroupMemberKind::GroupMemberPlayer,
		member.owner_character_id, member.owner_name,
		member.zone_id, member.instance_id);
}

GroupBuilder& GroupBuilder::SetLeadershipAA(const GroupLeadershipAA_Struct& aa)
{
	m_group.leadership_aa = aa;
	return *this;
}

GroupBuilder& GroupBuilder::LinkToRaid(const RaidLink& link)
{
	m_group.raid_link = link;
	return *this;
}

uint32 GroupBuilder::Push()
{
	PartyStateManager& mgr_ = PartyStateManager::Instance();
	uint32 gid = mgr_.AllocateGroupId();
	WorldGroup& installed = mgr_.InstallGroup(gid, std::move(m_group));

	installed.ForEachMember([&mgr = mgr_, gid](const WorldGroupMember& m) {
		mgr.IndexMember(m.name, gid);
	});

	installed.PushFull();

	return gid;
}

void WorldGroup::RemoveMember(std::string_view character_name)
{
	const auto found_it = std::ranges::find(members, character_name, &WorldGroupMember::name);
	if (found_it != members.end()) {
		if (Count() <= 2) {
			Disband();
		} else {
			bool departing_was_leader = Strings::EqualFold(leader_name, character_name);
			bool all_remaining_are_bots = std::ranges::none_of(members, [](const WorldGroupMember& m) {
				return m.InUse() && m.kind != GroupMemberKind::GroupMemberBot;
			});

			if (departing_was_leader && !raid_link.IsLinked() && all_remaining_are_bots) {
				Disband();
			} else {
				if (Strings::EqualFold(mentoree_name, character_name)) {
					mentoree_name.clear();
					mentor_percent = 0;
					PushMentorChanged();
				}

				std::string new_leader_name;
				if (departing_was_leader) {
					auto promoted = std::ranges::find_if(
						members,
						[](const WorldGroupMember& m) {
							return m.InUse() && m.kind != GroupMemberKind::GroupMemberBot;
						});

					if (promoted == members.end())
						promoted = std::ranges::find_if(members, [](const WorldGroupMember& m) { return m.InUse(); });
					leader_name = promoted->name;
					new_leader_name = leader_name;
					SetRoleHolder(GroupBitmaskLeader, leader_name);
				}

				ZoneServer* departing_zs = ZSList::Instance()->FindByZoneIDAndInstanceID(found_it->zone_id, found_it->instance_id);
				PushMemberLeft(character_name, new_leader_name, departing_zs);

				if (!new_leader_name.empty() && master_looter_name.empty()) {
					SetRoleHolder(GroupBitmaskMasterLooter, EffectiveMasterLooter());
					PushRoleChanged(GroupRoleMasterLooter, new_leader_name);
				}

				if (raid_link.IsLinked())
					raid_link.on_member_departed(raid_link.raid_id, character_name);

				PartyStateManager::Instance().EraseFromIndex({character_name});
			}
		}
	}
}

void WorldGroup::Disband() const
{
	PushGroupDisbanded();

	std::vector<std::string_view> departed_names;
	for (const auto& m : members)
		if (m.InUse())
			departed_names.push_back(m.name);

	if (raid_link.IsLinked())
		for (const auto& name : departed_names)
			raid_link.on_member_departed(raid_link.raid_id, name);

	PartyStateManager::Instance().EraseGroupAndIndex(GetID(), departed_names);
}

std::string_view WorldGroup::FindNextLeader(std::string_view offline_name) const
{
	auto online = std::ranges::find_if(members, [&offline_name](const WorldGroupMember& m) {
		return m.InUse() &&
			m.kind == GroupMemberKind::GroupMemberPlayer &&
			!Strings::EqualFold(m.name, offline_name) &&
			IsCharacterOnline(m.name);
	});

	if (online == members.end() && raid_link.IsLinked())
		online = std::ranges::find_if(members, [&offline_name](const WorldGroupMember& m) {
			return m.InUse() &&
				m.kind == GroupMemberKind::GroupMemberBot &&
				!Strings::EqualFold(m.name, offline_name);
		});

	std::string_view next_leader;
	if (online != members.end())
		next_leader = online->name;

	return next_leader;
}

void WorldGroup::SetMemberOffline(std::string_view character_name)
{
	if (!Strings::EqualFold(leader_name, character_name)) {
		// not the leader, safe to just set offline
		PushOfflineChanged(character_name, true);
	} else {
		std::string_view next_leader = FindNextLeader(character_name);
		if (next_leader.empty()) {
			PartyStateManager::Instance().ForceDisbandGroup(GetID());
		} else {
			leader_name = next_leader;
			SetRoleHolder(GroupBitmaskLeader, leader_name);
			PushLeaderChanged();
			PushOfflineChanged(character_name, true);
		}
	}
}

void WorldGroup::SetMemberOnline(std::string_view character_name) const
{
	PushOfflineChanged(character_name, false);
}

void WorldGroup::SetRaidLink(const RaidLink& link)
{
	raid_link = link;
	PushFull();
}

void WorldGroup::SetLocked(bool locked)
{
	raid_link.locked = locked;
}

void WorldGroup::SetMentor(std::string_view mentoree, uint8 percent)
{
	mentoree_name = mentoree;
	mentor_percent = mentoree.empty() ? 0 : percent;
	PushMentorChanged();
}

void WorldGroup::HandleStateRequest(std::string_view character_name, ZoneServer* requester)
{
	if (requester != nullptr) {
		if (WorldGroupMember* m = FindMember(character_name)) {
			m->zone_id = requester->GetZoneID();
			m->instance_id = requester->GetInstanceID();
		}

		auto pack = BuildFullPacket(GroupUpdateType::MemberZoned, character_name);
		requester->SendPacket(pack);
		safe_delete(pack);
	}
}

void WorldGroup::HandleLeadershipAction(const ServerGroupActionHeader_Struct& header, std::string_view target_name, GroupActionType action)
{
	auto blocked = [locked = raid_link.locked, action, req = header.requestor_character_id] {
		if (locked) {
			switch (action) {
			case GroupActionType::GroupActionDelegateMainTank:
			case GroupActionType::GroupActionUndelegateMainTank:
			case GroupActionType::GroupActionDelegateMainAssist:
			case GroupActionType::GroupActionUndelegateMainAssist:
			case GroupActionType::GroupActionDelegatePuller:
			case GroupActionType::GroupActionUndelegatePuller:
			case GroupActionType::GroupActionDelegateMasterLooter:
			case GroupActionType::GroupActionUndelegateMasterLooter:
			case GroupActionType::GroupActionDelegateMarkNPC:
			case GroupActionType::GroupActionUndelegateMarkNPC:
				SendPartyActionFailure(req, "You cannot change group roles while the raid is locked.");
				return true;
			case GroupActionType::GroupActionMakeLeader:
				SendPartyActionFailure(req, "You cannot change the group leader while the raid is locked.");
				return true;
			default:
				return false;
			}
		}

		return false;
	};

	if (!blocked()) {
		switch (action) {
		case GroupActionType::GroupActionMakeLeader: {
			WorldGroupMember* new_leader = FindMember(target_name);
			bool bot_leader_ok = raid_link.IsLinked();
			if (Strings::EqualFold(header.requestor_name, leader_name) && new_leader != nullptr) {
				if (new_leader->kind == GroupMemberKind::GroupMemberBot && !bot_leader_ok) {
					SendPartyActionFailure(header.requestor_character_id, "You cannot make a bot the leader of a standalone group.");
				} else {
					leader_name = target_name;
					SetRoleHolder(GroupBitmaskLeader, leader_name);

					leadership_aa = GroupLeadershipAA_Struct{};
					if (new_leader->kind != GroupMemberKind::GroupMemberBot)
						if (auto *new_leader_cle = ClientList::Instance()->FindCharacter(target_name))
							leadership_aa = FetchGroupLeadershipAAFromDB(new_leader_cle->CharID());

					PushLeaderChanged();

					SetRoleHolder(GroupBitmaskMasterLooter, EffectiveMasterLooter());
					if (master_looter_name.empty())
						PushRoleChanged(GroupRoleMasterLooter, target_name);
				}
			}
			break;
		}
		case GroupActionType::GroupActionDelegateMainTank:
			if (FindMember(target_name) != nullptr) {
				main_tank_name = Strings::EqualFold(main_tank_name, target_name) ? "" : target_name;
				SetRoleHolder(GroupBitmaskMainTank, main_tank_name);
				PushRoleChanged(GroupRoleMainTank, main_tank_name);
			}
			break;
		case GroupActionType::GroupActionUndelegateMainTank:
			main_tank_name.clear();
			SetRoleHolder(GroupBitmaskMainTank, "");
			PushRoleChanged(GroupRoleMainTank, "");
			break;
		case GroupActionType::GroupActionDelegateMainAssist:
			if (FindMember(target_name) != nullptr) {
				main_assist_name = Strings::EqualFold(main_assist_name, target_name) ? "" : target_name;
				SetRoleHolder(GroupBitmaskMainAssist, main_assist_name);
				PushRoleChanged(GroupRoleMainAssist, main_assist_name);
			}
			break;
		case GroupActionType::GroupActionUndelegateMainAssist:
			main_assist_name.clear();
			SetRoleHolder(GroupBitmaskMainAssist, "");
			PushRoleChanged(GroupRoleMainAssist, "");
			break;
		case GroupActionType::GroupActionDelegatePuller:
			if (FindMember(target_name) != nullptr) {
				puller_name = Strings::EqualFold(puller_name, target_name) ? "" : target_name;
				SetRoleHolder(GroupBitmaskPuller, puller_name);
				PushRoleChanged(GroupRolePuller, puller_name);
			}
			break;
		case GroupActionType::GroupActionUndelegatePuller:
			puller_name.clear();
			SetRoleHolder(GroupBitmaskPuller, "");
			PushRoleChanged(GroupRolePuller, "");
			break;
		case GroupActionType::GroupActionDelegateMasterLooter:
			if (FindMember(target_name) != nullptr) {
				master_looter_name = Strings::EqualFold(master_looter_name, target_name) ? "" : target_name;
				SetRoleHolder(GroupBitmaskMasterLooter, EffectiveMasterLooter());
				PushRoleChanged(GroupRoleMasterLooter, EffectiveMasterLooter());
			}
			break;
		case GroupActionType::GroupActionUndelegateMasterLooter:
			master_looter_name.clear();
			SetRoleHolder(GroupBitmaskMasterLooter, EffectiveMasterLooter());
			PushRoleChanged(GroupRoleMasterLooter, EffectiveMasterLooter());
			break;
		case GroupActionType::GroupActionDelegateMarkNPC:
			if (FindMember(target_name) != nullptr) {
				npc_marker_name = Strings::EqualFold(npc_marker_name, target_name) ? "" : target_name;
				SetRoleHolder(GroupBitmaskMarkNPC, npc_marker_name);
				PushRoleChanged(GroupRoleMarkNPC, npc_marker_name);
			}
			break;
		case GroupActionType::GroupActionUndelegateMarkNPC:
			npc_marker_name.clear();
			SetRoleHolder(GroupBitmaskMarkNPC, "");
			PushRoleChanged(GroupRoleMarkNPC, "");
			break;
		case GroupActionType::GroupActionUpdateAA: {
			auto *leader_cle = ClientList::Instance()->FindCharacter(leader_name);
			if (leader_cle != nullptr) {
				leadership_aa = FetchGroupLeadershipAAFromDB(leader_cle->CharID());
				PushAAChanged();
			} else {
				SendPartyActionFailure(header.requestor_character_id, "Could not resolve the group leader to update leadership AAs.");
			}
			break;
		}
		default:
			break;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Raid Methods
// ---------------------------------------------------------------------------------------------------------------------

static RaidLeadershipAA_Struct FetchRaidLeadershipAAFromDB(uint32 character_id)
{
	RaidLeadershipAA_Struct result{};

	const auto rows =  CharacterLeadershipAbilitiesRepository::GetWhere(database,
		fmt::format("`id` = {} AND `slot` >= {} AND `slot` < {}", character_id, MAX_GROUP_LEADERSHIP_AA_ARRAY, MAX_RAID_LEADERSHIP_AA_ARRAY));
	for (const auto &row : rows)
		result.ranks[row.slot - MAX_GROUP_LEADERSHIP_AA_ARRAY] = row.rank_;

	return result;
}

void WorldRaid::FillMemberFields(ServerRaidMemberFields_Struct& out, const WorldRaidMember& m) const
{
	Strings::Copy(out.member_name, m.name);
	out.group_number      = FindMemberGroupNumber(m.name);
	out._class            = m.class_;
	out.level             = m.level;
	Strings::Copy(out.note, m.note);
	out.is_group_leader   = PartyStateManager::Instance().IsGroupLeader(m.name);
	out.roles             = m.roles;
	out.main_marker       = m.main_marker;
	out.main_assister     = m.main_assister;
	out.kind              = m.kind;
	out.is_offline        = m.kind != RaidMemberKind::RaidMemberBot && !IsCharacterOnline(m.name);
}

ServerPacket* WorldRaid::BuildFullPacket() const
{
	auto pack = new ServerPacket(ServerOP_RaidUpdate, sizeof(ServerRaidFull_Struct));
	const auto rs = reinterpret_cast<ServerRaidFull_Struct*>(pack->pBuffer);
	rs->header.update_type = RaidUpdateType::Full;
	rs->header.rid         = GetID();

	rs->member_count = 0;
	for (const auto &m : members) {
		if (m.InUse() && rs->member_count < MAX_RAID_MEMBERS) {
			FillMemberFields(rs->members[rs->member_count], m);
			++rs->member_count;
		}
	}

	Strings::Copy(rs->leader_name, leader_name);
	rs->locked    = locked;
	rs->loot_type = loot_type;
	Strings::Copy(rs->motd, motd);

	for (int i = 0; i < MAX_RAID_MAIN_ASSISTERS; ++i)
		Strings::Copy(rs->main_assist_names[i], main_assist_names[i]);
	for (int i = 0; i < MAX_RAID_MAIN_MARKERS; ++i)
		Strings::Copy(rs->main_marker_names[i], main_marker_names[i]);

	Strings::Copy(rs->master_looter_name, master_looter_name);
	for (int i = 0; i < MAX_RAID_LOOTERS; ++i)
		Strings::Copy(rs->looter_names[i], looter_names[i]);
	rs->raid_aa = leadership_aa;

	return pack;
}

uint8 WorldRaid::GroupCount(uint32 group_number) const
{
	if (group_number < MAX_RAID_GROUPS) {
		uint32 real_gid = subgroup_group_ids[group_number];
		if (real_gid != 0)
			return PartyStateManager::Instance().GetGroupMemberCount(real_gid);

		return 0;
	}

	return std::ranges::count_if(members, [this](const WorldRaidMember& m) {
		return m.InUse() && FindMemberGroupNumber(m.name) == RAID_GROUPLESS;
	});
}

uint32 WorldRaid::FindMemberGroupNumber(std::string_view character_name) const
{
	for (uint32 i = 0; i < MAX_RAID_GROUPS; ++i) {
		uint32 gid = subgroup_group_ids[i];
		if (gid != 0 && PartyStateManager::Instance().IsMemberOf(gid, character_name))
			return i;
	}

	return RAID_GROUPLESS;
}

uint32 WorldRaid::GetFreeGroup() const
{
	for (uint32 i = 0; i < MAX_RAID_GROUPS; ++i)
		if (GroupCount(i) == 0)
			return i;

	return RAID_GROUPLESS;
}

uint32 WorldRaid::GetGroupWithRoom() const
{
	for (uint32 i = 0; i < MAX_RAID_GROUPS; i++) {
		uint8 count = GroupCount(i);
		if (count > 0 && count < MAX_GROUP_MEMBERS)
			return i;
	}

	return GetFreeGroup();
}

void WorldRaid::JoinRealSubgroup(uint32 group_number, std::string_view name, uint8 class_, uint8 level, bool is_bot,
	uint32 owner_character_id, uint32 zone_id, uint32 instance_id)
{
	if (group_number < MAX_RAID_GROUPS) {
		uint32 &gid = subgroup_group_ids[group_number];
		GroupMemberKind kind = is_bot ? GroupMemberKind::GroupMemberBot : GroupMemberKind::GroupMemberPlayer;

		if (gid == 0) {
			// A raid subgroup is created exactly the way any other group is
			// -- via GroupBuilder -- the only raid-specific part is linking
			// it to this raid before the single creation push goes out.
			gid = PartyStateManager::Instance().NewGroup()
				.AddMember(name, class_, level, kind, owner_character_id, "", zone_id, instance_id)
				.LinkToRaid(PartyStateManager::Instance().MakeRaidLink(GetID(), group_number, locked))
				.Push();
		} else {
			PartyStateManager::Instance().AddMemberToGroup(gid, name, class_, level, is_bot, owner_character_id, false, "", zone_id, instance_id);
		}
	}
}

void WorldRaid::PushFullTo(ZoneServer* target) const
{
	if (target != nullptr) {
		auto pack = BuildFullPacket();
		target->SendPacket(pack);
		safe_delete(pack);
	}
}

void WorldRaid::PushMemberJoined(const WorldRaidMember& m) const
{
	ServerRaidMemberJoined_Struct msg{};
	FillMemberFields(msg.member, m);
	PushDelta(msg, RaidUpdateType::MemberJoined);
}

void WorldRaid::PushMemberLeft(std::string_view character_name, ZoneServer* departing_zs) const
{
	ServerRaidMemberLeft_Struct msg{};
	Strings::Copy(msg.member_name, character_name);

	PushDelta(msg, RaidUpdateType::MemberLeft);

	if (departing_zs != nullptr)
		PushDelta(msg, RaidUpdateType::MemberLeft, departing_zs);
}

void WorldRaid::PushMemberFieldsChanged(const WorldRaidMember& m) const
{
	ServerRaidMemberFieldsChanged_Struct msg{};
	FillMemberFields(msg.member, m);
	PushDelta(msg, RaidUpdateType::MemberFieldsChanged);
}

void WorldRaid::PushGroupMoved(std::string_view character_name, uint32 new_group_number) const
{
	ServerRaidGroupMoved_Struct msg{};
	Strings::Copy(msg.member_name, character_name);
	msg.new_group_number = new_group_number;
	PushDelta(msg, RaidUpdateType::GroupMoved);
}

void WorldRaid::PushLeaderChanged() const
{
	ServerRaidLeaderChanged_Struct msg{};
	Strings::Copy(msg.leader_name, leader_name);
	PushDelta(msg, RaidUpdateType::LeaderChanged);
}

void WorldRaid::PushStateChanged() const
{
	ServerRaidStateChanged_Struct msg{};
	msg.locked    = locked;
	msg.loot_type = loot_type;
	Strings::Copy(msg.motd, motd);

	for (int i = 0; i < MAX_RAID_MAIN_ASSISTERS; ++i)
		Strings::Copy(msg.main_assist_names[i], main_assist_names[i]);
	for (int i = 0; i < MAX_RAID_MAIN_MARKERS; ++i)
		Strings::Copy(msg.main_marker_names[i], main_marker_names[i]);

	Strings::Copy(msg.master_looter_name, master_looter_name);
	for (int i = 0; i < MAX_RAID_LOOTERS; ++i)
		Strings::Copy(msg.looter_names[i], looter_names[i]);
	msg.raid_aa = leadership_aa;

	PushDelta(msg, RaidUpdateType::StateChanged);
}

void WorldRaid::PushAAChanged() const
{
	ServerRaidAAChanged_Struct msg{};
	msg.raid_aa = leadership_aa;
	PushDelta(msg, RaidUpdateType::AAChanged);
}

void WorldRaid::PushRoleChanged(uint8 role_id, uint8 slot, std::string_view target_name, bool clearing) const
{
	ServerRaidRoleChanged_Struct msg{};
	msg.role_id  = role_id;
	msg.slot     = slot;
	msg.clearing = clearing;
	Strings::Copy(msg.target_name, target_name);
	PushDelta(msg, RaidUpdateType::RoleChanged);
}

void WorldRaid::PushDisbanded() const
{
	PushDelta(ServerRaidUpdateDisbanded_Struct{}, RaidUpdateType::Disbanded);
}

WorldRaidMember* WorldRaid::PlaceMember(WorldRaidMember member)
{
	WorldRaidMember* slot = FindFreeSlot();
	if (slot != nullptr)
		*slot = std::move(member);

	return slot;
}

void WorldRaid::AddMember(WorldRaidMember member)
{
	WorldRaidMember* slot = PlaceMember(std::move(member));
	if (slot != nullptr) {
		PushMemberJoined(*slot);
		PartyStateManager::Instance().IndexRaidMember(slot->name, GetID());
	}
}

// Roles never carry over -- a group role (main tank, puller, ...) has no
// raid-side equivalent, and raid-side roles (leader, looter, ...) are
// always granted explicitly (RaidBuilder::SetLeader, HandleActionInvite's
// AddLooter), never inherited from whatever group the member came from.
void WorldRaid::AddMember(const WorldGroupMember& member)
{
	WorldRaidMember raid_member;
	raid_member.name               = member.name;
	raid_member.class_             = member.class_;
	raid_member.level              = member.level;
	raid_member.owner_character_id = member.owner_character_id;
	raid_member.zone_id            = member.zone_id;
	raid_member.instance_id        = member.instance_id;
	raid_member.kind               = member.kind == GroupMemberKind::GroupMemberBot ? RaidMemberKind::RaidMemberBot : RaidMemberKind::RaidMemberPlayer;

	AddMember(raid_member);
}

WorldRaidMember* WorldRaid::AddMember(std::string_view name, uint8 class_, uint8 level, bool raid_leader, bool looter,
	RaidMemberKind kind, uint32 owner_character_id, uint32 zone_id, uint32 instance_id)
{
	WorldRaidMember member = WorldRaidMember::Make(name, class_, level, kind, owner_character_id, zone_id, instance_id);
	member.roles = (raid_leader ? RaidBitmaskLeader : 0) | (looter ? RaidBitmaskLooter : 0);

	WorldRaidMember* slot = PlaceMember(std::move(member));
	if (slot != nullptr) {
		PushMemberJoined(*slot);
		PartyStateManager::Instance().IndexRaidMember(name, GetID());
	}

	return slot;
}

void WorldRaid::RemoveMember(std::string_view character_name)
{
	WorldRaidMember* m = FindMember(character_name);
	if (m != nullptr) {
		uint32 old_group         = FindMemberGroupNumber(character_name);
		bool   was_group_leader  = PartyStateManager::Instance().IsGroupLeader(character_name);
		bool   was_raid_leader   = (m->roles & RaidBitmaskLeader) != 0;
		bool   was_master_looter = (m->roles & RaidBitmaskMasterLooter) != 0;
		bool   was_looter        = (m->roles & RaidBitmaskLooter) != 0;
		bool   was_bot           = m->kind == RaidMemberKind::RaidMemberBot;

		std::vector<std::string> bots_to_cascade;
		if (!was_bot) {
			// A departing real character's own bots always leave with them.
			auto *cle = ClientList::Instance()->FindCharacter(character_name);
			if (cle != nullptr) {
				uint32 owner_id = cle->CharID();
				for (const auto &bm : members)
					if (bm.InUse() && bm.kind == RaidMemberKind::RaidMemberBot && bm.owner_character_id == owner_id)
						bots_to_cascade.push_back(bm.name);
			}
		} else if (was_group_leader && old_group < MAX_RAID_GROUPS) {
			// A departing bot group-leader (only possible via its owner
			// leaving the raid entirely while a different bot led the
			// subgroup) takes the rest of that subgroup's bots with it.
			for (const auto &bm : members)
				if (bm.InUse() && bm.kind == RaidMemberKind::RaidMemberBot && bm.name != m->name && FindMemberGroupNumber(bm.name) == old_group)
					bots_to_cascade.push_back(bm.name);
		}

		if (old_group < MAX_RAID_GROUPS && subgroup_group_ids[old_group] != 0) {
			uint32 old_gid = subgroup_group_ids[old_group];
			PartyStateManager::Instance().RemoveMemberFromGroup(old_gid, character_name);
			if (!PartyStateManager::Instance().GroupExists(old_gid))
				subgroup_group_ids[old_group] = 0;
		}

		ZoneServer* departing_zs = nullptr;
		if (auto* cle = ClientList::Instance()->FindCharacter(character_name))
			departing_zs = cle->Server();

		PartyStateManager::Instance().EraseFromRaidIndex({character_name});
		*m = WorldRaidMember{};

		PushMemberLeft(character_name, departing_zs);

		if (was_master_looter)
			RecomputeMasterLooter();

		if (was_looter)
			RecomputeLooterNames();

		if (was_raid_leader) {
			auto promoted = std::ranges::find_if(members, [](const WorldRaidMember& nm) {
				return nm.InUse() && nm.kind != RaidMemberKind::RaidMemberBot;
			});

			if (promoted != members.end()) {
				promoted->roles |= RaidBitmaskLeader;
				leader_name = promoted->name;
				PushLeaderChanged();
			}
		}

		for (const auto &bot_name : bots_to_cascade)
			RemoveMember(bot_name);
	}
}

void WorldRaid::Disband() const
{
	DisbandAllSubgroups();
	PushDisbanded();

	std::vector<std::string_view> departed_names;
	for (const auto& m : members)
		if (m.InUse())
			departed_names.push_back(m.name);

	PartyStateManager::Instance().EraseRaidAndIndex(GetID(), departed_names);
}

void WorldRaid::DisbandAllSubgroups() const
{
	for (const auto &gid : subgroup_group_ids)
		if (gid != 0)
			PartyStateManager::Instance().ForceDisbandGroup(gid);
}

// ---------------------------------------------------------------------------------------------------------------------
// RaidBuilder
// ---------------------------------------------------------------------------------------------------------------------

RaidBuilder& RaidBuilder::AddMember(std::string_view name, uint8 class_, uint8 level, RaidMemberKind kind,
	uint32 owner_character_id, uint32 zone_id, uint32 instance_id)
{
	raid_.PlaceMember(WorldRaidMember::Make(name, class_, level, kind, owner_character_id, zone_id, instance_id));
	return *this;
}

RaidBuilder& RaidBuilder::SetLeader(std::string_view name)
{
	WorldRaidMember* m = raid_.FindMember(name);
	if (m != nullptr) {
		m->roles |= (RaidBitmaskLeader | RaidBitmaskLooter);
		raid_.leader_name = name;
	}

	return *this;
}

RaidBuilder& RaidBuilder::AdoptGroup(uint32 gid)
{
	WorldGroup* group = PartyStateManager::Instance().GetGroupById(gid);
	if (group != nullptr) {
		group->ForEachMember([this](const WorldGroupMember& gm) {
			RaidMemberKind kind = gm.kind == GroupMemberKind::GroupMemberBot ? RaidMemberKind::RaidMemberBot : RaidMemberKind::RaidMemberPlayer;
			raid_.PlaceMember(WorldRaidMember::Make(gm.name, gm.class_, gm.level, kind, gm.owner_character_id, gm.zone_id, gm.instance_id));
		});

		pending_subgroup_gids_.push_back(gid);
	}

	return *this;
}

RaidBuilder& RaidBuilder::SetLootType(uint32 loot_type)
{
	raid_.loot_type = loot_type;
	return *this;
}

uint32 RaidBuilder::Commit()
{
	PartyStateManager& mgr = PartyStateManager::Instance();
	uint32 rid = mgr.AllocateRaidId();
	WorldRaid& installed = mgr.InstallRaid(rid, std::move(raid_));

	installed.ForEachMember([&mgr, rid](const WorldRaidMember& m) {
		mgr.IndexRaidMember(m.name, rid);
	});

	for (size_t i = 0; i < pending_subgroup_gids_.size(); ++i) {
		uint32 gid = pending_subgroup_gids_[i];
		installed.subgroup_group_ids[i] = gid;
		mgr.SetRaidLink(gid, mgr.MakeRaidLink(rid, static_cast<uint32>(i), installed.locked));
	}

	// A member adopted (or added) with the looter bit already needs the
	// looter_names array populated to match -- SetLeader grants it to the
	// founding leader, same as live's make-leader/found-raid behavior.
	installed.RecomputeLooterNames();

	installed.PushLeaderChanged();
	installed.PushStateChanged();

	return rid;
}

void WorldRaid::RecomputeMasterLooter()
{
	master_looter_name.clear();
	auto holder = std::ranges::find_if(members, [](const WorldRaidMember& m) {
		return m.InUse() && (m.roles & RaidBitmaskMasterLooter) != 0;
	});

	if (holder != members.end())
		master_looter_name = holder->name;
}

void WorldRaid::RecomputeLooterNames()
{
	for (auto &name : looter_names)
		name.clear();

	uint8 slot = 0;
	for (const auto &m : members) {
		if (m.InUse() && (m.roles & RaidBitmaskLooter) && slot < MAX_RAID_LOOTERS) {
			looter_names[slot] = m.name;
			++slot;
		}
	}
}

void WorldRaid::MakeLeader(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name)
{
	if (Strings::EqualFold(requestor_name, leader_name)) {
		WorldRaidMember* was_lead = FindMember(leader_name);
		WorldRaidMember* new_lead = FindMember(target_name);
		// No /bot command lets a bot owner make their bot issue a raid
		// request as itself -- every raid action always originates from a
		// real Client -- so a bot becoming raid leader is never legitimate.
		if (new_lead != nullptr && new_lead->kind == RaidMemberKind::RaidMemberBot) {
			SendPartyActionFailure(requestor_character_id, "You cannot make a bot the raid leader.");
		} else if (new_lead != nullptr) {
			if (was_lead != nullptr)
				was_lead->roles &= ~RaidBitmaskLeader;
			new_lead->roles |= RaidBitmaskLeader;
			leader_name = target_name;

			// The new leader's own leadership AA ranks become the raid's
			// active ranks -- read fresh from the DB rather than trusting a
			// zone-sent snapshot. new_lead->kind == RaidMemberBot is already
			// ruled out above, so this always resolves to a real character.
			leadership_aa = RaidLeadershipAA_Struct{};
			if (auto *new_leader_cle = ClientList::Instance()->FindCharacter(target_name))
				leadership_aa = FetchRaidLeadershipAAFromDB(new_leader_cle->CharID());

			PushLeaderChanged();
			PushStateChanged();
		}
	}
}

void WorldRaid::UpdateLeadershipAA(uint32 requestor_character_id)
{
	// Resolve the character to query from world's own record of who leads
	// this raid, not from anything the zone claims.
	auto *leader_cle = ClientList::Instance()->FindCharacter(leader_name);
	if (leader_cle == nullptr) {
		SendPartyActionFailure(requestor_character_id, "Could not resolve the raid leader to update leadership AAs.");
	} else {
		leadership_aa = FetchRaidLeadershipAAFromDB(leader_cle->CharID());
		PushAAChanged();
	}
}

void WorldRaid::Lock(std::string_view requestor_name, uint32 requestor_character_id)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can lock the raid.");
	} else {
		locked = true;
		auto& psm = PartyStateManager::Instance();
		for (uint32 gid : subgroup_group_ids)
			if (gid != 0)
				psm.SetGroupLocked(gid, true);

		PushStateChanged();
	}
}

void WorldRaid::Unlock(std::string_view requestor_name, uint32 requestor_character_id)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can unlock the raid.");
	} else {
		locked = false;
		auto& psm = PartyStateManager::Instance();
		for (uint32 gid : subgroup_group_ids)
			if (gid != 0)
				psm.SetGroupLocked(gid, false);

		PushStateChanged();
	}
}

void WorldRaid::SetLootType(std::string_view requestor_name, uint32 requestor_character_id, uint32 new_loot_type)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can change the loot type.");
	} else {
		loot_type = new_loot_type;
		if (new_loot_type != RaidLootType::LeaderSelected) {
			for (auto& m : members)
				if (m.InUse())
					m.roles &= ~RaidBitmaskLooter;
			RecomputeLooterNames();
		}

		PushStateChanged();
	}
}

void WorldRaid::AddLooter(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can set looters.");
	} else if (loot_type != RaidLootType::LeaderSelected) {
		// Looter only means anything in Leader Selected loot mode -- matches
		// the client's own gating, enforced here too so a stale/forged
		// request can't set it under a different mode.
		SendPartyActionFailure(requestor_character_id, "You can only set looters while using the Leader Selected loot type.");
	} else {
		WorldRaidMember* m = FindMember(target_name);
		if (m != nullptr && m->kind != RaidMemberKind::RaidMemberBot && !(m->roles & RaidBitmaskLooter)) {
			uint32 looter_count = std::ranges::count_if(members, [](const WorldRaidMember& rm) {
				return rm.InUse() && (rm.roles & RaidBitmaskLooter) != 0;
			});

			if (looter_count >= MAX_RAID_LOOTERS) {
				SendPartyActionFailure(requestor_character_id, "Maximum number of looters reached.");
			} else {
				m->roles |= RaidBitmaskLooter;
				RecomputeLooterNames();
				PushMemberFieldsChanged(*m);
			}
		}
	}
}

void WorldRaid::RemoveLooter(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can set looters.");
	} else {
		WorldRaidMember* m = FindMember(target_name);
		if (m != nullptr && m->kind != RaidMemberKind::RaidMemberBot) {
			m->roles &= ~RaidBitmaskLooter;
			RecomputeLooterNames();
			PushMemberFieldsChanged(*m);
		}
	}
}

void WorldRaid::SetGroupOnInvite(std::string_view requestor_name, bool value)
{
	if (Strings::EqualFold(requestor_name, leader_name))
		group_on_invite = value;
}

void WorldRaid::DelegateAssist(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can delegate main assist.");
	} else {
		WorldRaidMember* m = FindMember(target_name);
		if (m != nullptr) {
			if (m->main_assister != 0) {
				uint8 slot = m->main_assister - 1;
				main_assist_names[slot].clear();
				m->main_assister = 0;
				m->roles &= ~RaidBitmaskMainAssist;
				PushMemberFieldsChanged(*m);
				PushRoleChanged(RaidBitmaskMainAssist, slot, target_name, true);
			} else {
				auto free_slot = std::ranges::find(main_assist_names, std::string());
				if (free_slot == main_assist_names.end()) {
					SendPartyActionFailure(requestor_character_id, "Maximum number of raid main assisters reached.");
				} else {
					auto slot = static_cast<uint8>(std::distance(main_assist_names.begin(), free_slot));
					*free_slot = target_name;
					m->main_assister = slot + 1;
					m->roles |= RaidBitmaskMainAssist;
					PushMemberFieldsChanged(*m);
					PushRoleChanged(RaidBitmaskMainAssist, slot, target_name, false);
				}
			}
		}
	}
}

void WorldRaid::UndelegateAssist(std::string_view target_name)
{
	WorldRaidMember* m = FindMember(target_name);
	if (m != nullptr && m->main_assister != 0) {
		uint8 slot = m->main_assister - 1;
		main_assist_names[slot].clear();
		m->main_assister = 0;
		m->roles &= ~RaidBitmaskMainAssist;
		PushMemberFieldsChanged(*m);
		PushRoleChanged(RaidBitmaskMainAssist, slot, target_name, true);
	}
}

void WorldRaid::DelegateMark(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can delegate NPC marking.");
	} else {
		WorldRaidMember* m = FindMember(target_name);
		if (m != nullptr) {
			if (m->main_marker != 0) {
				uint8 slot = m->main_marker - 1;
				main_marker_names[slot].clear();
				m->main_marker = 0;
				m->roles &= ~RaidBitmaskMainMarker;
				PushMemberFieldsChanged(*m);
				PushRoleChanged(RaidBitmaskMainMarker, slot, target_name, true);
			} else {
				auto free_slot = std::ranges::find(main_marker_names, std::string());
				if (free_slot == main_marker_names.end()) {
					SendPartyActionFailure(requestor_character_id, "Maximum number of raid main markers reached.");
				} else {
					auto slot = static_cast<uint8>(std::distance(main_marker_names.begin(), free_slot));
					*free_slot = target_name;
					m->main_marker = slot + 1;
					m->roles |= RaidBitmaskMainMarker;
					PushMemberFieldsChanged(*m);
					PushRoleChanged(RaidBitmaskMainMarker, slot, target_name, false);
				}
			}
		}
	}
}

void WorldRaid::UndelegateMark(std::string_view target_name)
{
	WorldRaidMember* m = FindMember(target_name);
	if (m != nullptr && m->main_marker != 0) {
		uint8 slot = m->main_marker - 1;
		main_marker_names[slot].clear();
		m->main_marker = 0;
		m->roles &= ~RaidBitmaskMainMarker;
		PushMemberFieldsChanged(*m);
		PushRoleChanged(RaidBitmaskMainMarker, slot, target_name, true);
	}
}

void WorldRaid::DelegateMasterLooter(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can delegate master looter.");
	} else {
		WorldRaidMember* m = FindMember(target_name);
		if (m != nullptr) {
			if (m->roles & RaidBitmaskMasterLooter) {
				m->roles &= ~RaidBitmaskMasterLooter;
				master_looter_name.clear();
				PushMemberFieldsChanged(*m);
				PushRoleChanged(RaidBitmaskMasterLooter, 0, target_name, true);
			} else {
				for (auto& member : members) {
					if (member.InUse()) {
						bool is_new_master_looter = Strings::EqualFold(member.name, target_name);
						if (is_new_master_looter)
							member.roles |= RaidBitmaskMasterLooter;
						else
							member.roles &= ~RaidBitmaskMasterLooter;
						if (is_new_master_looter || Strings::EqualFold(member.name, master_looter_name))
							PushMemberFieldsChanged(member);
					}
				}

				master_looter_name = target_name;
				PushRoleChanged(RaidBitmaskMasterLooter, 0, target_name, false);
			}
		}
	}
}

void WorldRaid::UndelegateMasterLooter(std::string_view target_name)
{
	WorldRaidMember* m = FindMember(target_name);
	if (m != nullptr) {
		m->roles &= ~RaidBitmaskMasterLooter;
		master_looter_name.clear();
		PushMemberFieldsChanged(*m);
		PushRoleChanged(RaidBitmaskMasterLooter, 0, target_name, true);
	}
}

void WorldRaid::SetMotd(std::string_view requestor_name, uint32 requestor_character_id, std::string_view new_motd)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can set the message of the day.");
	} else {
		motd = new_motd;
		PushStateChanged();
	}
}

void WorldRaid::SetNote(std::string_view requestor_name, uint32 requestor_character_id, std::string_view target_name, std::string_view note)
{
	if (!Strings::EqualFold(requestor_name, leader_name)) {
		SendPartyActionFailure(requestor_character_id, "Only the raid leader can set member notes.");
	} else {
		WorldRaidMember* m = FindMember(target_name);
		if (m != nullptr) {
			m->note = note;
			PushMemberFieldsChanged(*m);
		}
	}
}

void WorldRaid::UpdateMemberLevel(std::string_view target_name, uint8 level)
{
	// Not a membership/role decision -- just recording a fact zone reported
	// about its own live character. No lock gate, no permission check:
	// matches the old zone-local behavior exactly (a level-up was never
	// blockable).
	WorldRaidMember* m = FindMember(target_name);
	if (m != nullptr) {
		m->level = level;
		PushMemberFieldsChanged(*m);
	}
}

void WorldRaid::HandleStateRequest(std::string_view character_name, ZoneServer* requester)
{
	if (requester != nullptr) {
		// The single canonical update point for zone_id/instance_id -- every
		// member kind (player, bot, merc) sends this on every connect/zone-
		// transfer/spawn, so this is always kept current without ever
		// needing to resolve through ClientList.
		if (WorldRaidMember* m = FindMember(character_name)) {
			m->zone_id     = requester->GetZoneID();
			m->instance_id = requester->GetInstanceID();
		}

		PushFullTo(requester);
	}
}
