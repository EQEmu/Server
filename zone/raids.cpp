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
#include "common/eq_constants.h"
#include "common/events/player_event_logs.h"
#include "common/raid.h"
#include "common/repositories/character_expedition_lockouts_repository.h"
#include "common/strings.h"
#include "zone/bot.h"
#include "zone/client.h"
#include "zone/dynamic_zone.h"
#include "zone/entity.h"
#include "zone/groups.h"
#include "zone/mob.h"
#include "zone/queryserv.h"
#include "zone/raids.h"
#include "zone/worldserver.h"

extern QueryServ  *QServ;

Raid::Raid(uint32 raidID)
{
	SetID(raidID);
	loot_type = 4; // matches world/raid_state.cpp's own default for a freshly-created raid
}

Raid::Raid(Client* nLeader)
	: leader(nLeader)
{
	leader_name = nLeader->GetName();
	loot_type = 4;
}

Raid::~Raid()
{
	for (auto gid : subgroup_group_ids)
		if (gid != 0)
			if (const Group *g = entity_list.GetGroupByID(gid))
				entity_list.RemoveGroup(g->GetID());
}

void Raid::Process() {}

// ---------------------------------------------------------------------------------------------------------------------
// Raid Game Functionality
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// Passthrough to world
// ---------------------------------------------------------------------------------------------------------------------

template <typename T>
static void SendRaidAction(const Client* requestor, T&& payload, uint32 raid_id, RaidActionType action)
{
	if (requestor != nullptr) {
		payload.header.rid = raid_id;
		payload.header.requestor_character_id = requestor->CharacterID();
		strn0cpy(payload.header.requestor_name, requestor->GetCleanName(), sizeof(payload.header.requestor_name));

		payload.header.action = static_cast<uint8>(action);
		payload.header.zoneid = zone->GetZoneID();
		payload.header.instance_id = zone->GetInstanceID();

		auto pack = new ServerPacket(ServerOP_RaidAction, sizeof(T));
		memcpy(pack->pBuffer, &payload, sizeof(T));
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

template <typename T>
void Raid::RequestAction(const Client* requestor, T&& payload, RaidActionType action) const
{
	SendRaidAction(requestor, std::forward<T>(payload), GetID(), action);
}

static ServerRaidActionBasic_Struct ActionPayload() { return ServerRaidActionBasic_Struct{}; }

static ServerRaidActionValue_Struct ActionPayload(uint32 value)
{
	return ServerRaidActionValue_Struct{.value = value};
}

static ServerRaidActionTarget_Struct ActionPayload(const std::string& target_name)
{
	ServerRaidActionTarget_Struct payload{};
	strn0cpy(payload.target_name, target_name.c_str(), sizeof(payload.target_name));
	return payload;
}

static ServerRaidActionTargetValue_Struct ActionPayload(const std::string& target_name, uint32 value)
{
	ServerRaidActionTargetValue_Struct payload{.value = value};
	strn0cpy(payload.target_name, target_name.c_str(), sizeof(payload.target_name));
	return payload;
}

void Raid::RequestLock(const Client* requestor) const
{
	RequestAction(requestor, ActionPayload(), RaidLeadershipLock);
}

void Raid::RequestUnlock(const Client* requestor) const
{
	RequestAction(requestor, ActionPayload(), RaidLeadershipUnlock);
}

void Raid::RequestSetLootType(const Client* requestor, uint32 loot_type) const
{
	RequestAction(requestor, ActionPayload(loot_type), RaidLeadershipSetLootType);
}

void Raid::RequestSetLooter(const Client* requestor, const std::string& target_name, bool adding) const
{
	RequestAction(requestor, ActionPayload(target_name), adding ? RaidLeadershipAddLooter : RaidLeadershipRemoveLooter);
}

void Raid::RequestMakeLeader(const Client* requestor, const std::string& target_name) const
{
	RequestAction(requestor, ActionPayload(target_name), RaidLeadershipMakeLeader);
}

void Raid::RequestSetMotd(const Client* requestor, const std::string& new_motd) const
{
	ServerRaidActionMotd_Struct payload{};
	strn0cpy(payload.motd, new_motd.c_str(), sizeof(payload.motd));
	RequestAction(requestor, payload, RaidRoleSetMotd);
}

void Raid::RequestSetNote(const Client* requestor, const std::string& target_name, const std::string& note) const
{
	ServerRaidActionNote_Struct payload{};
	strn0cpy(payload.target_name, target_name.c_str(), sizeof(payload.target_name));
	strn0cpy(payload.note, note.c_str(), sizeof(payload.note));
	RequestAction(requestor, payload, RaidRoleSetNote);
}

void Raid::RequestSetGroupOnInvite(const Client* requestor, bool enabled) const
{
	RequestAction(requestor, ActionPayload(enabled ? 1u : 0u), RaidLeadershipSetGroupOnInvite);
}

void Raid::RequestDelegateRole(const Client* requestor, RaidActionType action, const std::string& target_name) const
{
	RequestAction(requestor, ActionPayload(target_name), action);
}

void Raid::RequestRemoveMember(const Client* requestor, const std::string& target_name) const
{
	RequestAction(requestor, ActionPayload(target_name), RaidActionRemoveMember);
}

void Raid::RequestMoveGroup(const Client* requestor, const std::string& target_name, uint32 dest_group_number) const
{
	RequestAction(requestor, ActionPayload(target_name, dest_group_number), RaidActionMoveGroup);
}

void Raid::RequestRaidAAs() const
{
	RequestAction(GetLeader(), ActionPayload(), RaidLeadershipUpdateAA);
}

void Raid::RequestUpdateLevel(const Client* requestor, const std::string& target_name, int level) const
{
	RequestAction(requestor, ActionPayload(target_name, level), RaidLeadershipUpdateLevel);
}

// The invitee is typed by name, not targeted, so unlike every other raid
// action's target (always something already resolved locally) they might
// not be anywhere near this zone -- or, for a bot, might not even be
// spawned anywhere. Resolve them here when possible and hand world a fully
// populated struct; leave it zeroed (`resolved` stays false) when not, so
// world knows to fall back to its own live data (ClientList/bot_data)
// instead of silently treating "not found" as "is a player with no info."
static ServerRaidActionInvite_Struct InviteActionPayload(const std::string& target_name)
{
	ServerRaidActionInvite_Struct payload{};
	strn0cpy(payload.target_name, target_name.c_str(), sizeof(payload.target_name));

	if (const Client* local_client = entity_list.GetClientByName(target_name.c_str())) {
		payload.resolved     = true;
		payload.kind         = RaidMemberKind::RaidMemberPlayer;
		payload.target_class = local_client->GetClass();
		payload.target_level = local_client->GetLevel();
	} else if (Bot* local_bot = entity_list.GetBotByBotName(target_name)) {
		payload.resolved           = true;
		payload.kind               = RaidMemberKind::RaidMemberBot;
		payload.owner_character_id = local_bot->GetOwnerCharacterID();
		payload.target_class       = local_bot->GetClass();
		payload.target_level       = local_bot->GetLevel();
		if (const Mob* bot_owner = local_bot->GetBotOwner())
			strn0cpy(payload.owner_name, bot_owner->GetCleanName(), sizeof(payload.owner_name));
	}

	return payload;
}

void Raid::RequestInvite(const Client* inviter, const std::string& invitee_name)
{
	SendRaidAction(inviter, InviteActionPayload(invitee_name), 0, RaidActionInvite);
}

void Raid::RequestFollow(const Client* accepter, const std::string& inviter_name)
{
	SendRaidAction(accepter, ActionPayload(inviter_name), 0, RaidActionAcceptInvite);
}

// ---------------------------------------------------------------------------------------------------------------------
// Apply State From World
// ---------------------------------------------------------------------------------------------------------------------

void Raid::ApplyFullFromWorld(const ServerRaidFull_Struct &rs)
{
	leader_name = rs.leader_name;
	SetLeader(entity_list.GetClientByName(leader_name.c_str()));

	locked    = rs.locked;
	loot_type = rs.loot_type;
	motd      = rs.motd;

	for (int i = 0; i < MAX_RAID_MAIN_ASSISTERS; ++i)
		main_assist_names[i] = rs.main_assist_names[i];
	for (int i = 0; i < MAX_RAID_MAIN_MARKERS; ++i)
		main_marker_names[i] = rs.main_marker_names[i];

	master_looter_name = rs.master_looter_name;
	for (int i = 0; i < MAX_RAID_LOOTERS; ++i)
		looter_names[i] = rs.looter_names[i];

	leadership_aa = rs.raid_aa;

	// Diff against the roster as it stood before this call -- only a name
	// that's genuinely new or genuinely gone fires OnAddToRaid/
	// OnRemoveFromRaid via AttachLocalMember/entity_list.SetRaid; anyone
	// still present just gets re-resolved via AttachLocalMember's Zoned
	// path, since their membership itself didn't change.
	std::vector<std::string> previous_names;
	for (const auto &m : members)
		if (m.InUse())
			previous_names.push_back(m.name);

	std::vector<std::string> incoming_names;

	for (uint8 i = 0; i < rs.member_count && i < MAX_RAID_MEMBERS; ++i) {
		const auto &mf = rs.members[i];
		RaidMember &slot = members[i];
		incoming_names.push_back(mf.member_name);

		// Captured before AttachLocalMember re-resolves slot.member below --
		// this is how we tell "just became locally resolvable this call"
		// apart from "was already resolved, nothing changed for them" below,
		// without needing any per-client pending-sync flag.
		Mob* previously_resolved = slot.member;

		slot.name           = mf.member_name;
		slot.group_number   = mf.group_number;
		slot.class_         = mf._class;
		slot.level          = mf.level;
		slot.note           = mf.note;
		slot.is_group_leader  = mf.is_group_leader;
		slot.roles            = mf.roles;
		slot.main_marker      = mf.main_marker;
		slot.main_assister    = mf.main_assister;
		slot.kind             = mf.kind;
		slot.offline          = mf.is_offline;

		const bool was_present = std::ranges::any_of(previous_names, [&mf](const std::string& n) {
			return Strings::EqualFold(n, mf.member_name);
		});

		AttachLocalMember(slot, !was_present);

		// Only notify (and spawn raid bots for) a member that just became
		// locally resolvable this call -- an already-resolved member's raid
		// info hasn't changed just because some OTHER member's connect
		// triggered this Full. This is also this member's one-time,
		// exactly-once-per-zone-entry signal (MemberZoned nulls slot.member
		// back out on zone-out, so the next zone-in sees this transition
		// again) -- no p_pending_raid_sync flag needed.
		if (previously_resolved == nullptr && slot.member != nullptr && slot.member->IsClient()) {
			Client* c = slot.member->CastToClient();
			SendBulkRaidInfoTo(c);

			if (RuleB(Bots, Enabled))
				c->SpawnRaidBotsOnConnect(this);
		}
	}

	// Anyone who was here before but isn't in the new roster at all
	// genuinely left the raid.
	for (const auto& name : previous_names) {
		const bool still_present = std::ranges::any_of(incoming_names, [&name](const std::string& n) {
			return Strings::EqualFold(n, name);
		});

		if (!still_present) {
			if (Mob* m = entity_list.GetMob(name))
				entity_list.SetRaid(m, nullptr);
		}
	}

	if (locked)
		SendRaidLock();
	else
		SendRaidUnlock();
}

void Raid::ApplyMemberJoinedFromWorld(const ServerRaidMemberJoined_Struct &msg)
{
	RaidMember *slot = nullptr;
	for (auto &m : members) {
		if (!m.InUse()) {
			slot = &m;
			break;
		}
	}

	if (slot == nullptr) {
		LogError("Raid::ApplyMemberJoinedFromWorld() -- no free raid member slot for [{}]", msg.member.member_name);
		return;
	}

	slot->name             = msg.member.member_name;
	slot->group_number     = msg.member.group_number;
	slot->class_           = msg.member._class;
	slot->level            = msg.member.level;
	slot->note             = msg.member.note;
	slot->is_group_leader  = msg.member.is_group_leader;
	slot->roles            = msg.member.roles;
	slot->main_marker      = msg.member.main_marker;
	slot->main_assister    = msg.member.main_assister;
	slot->kind             = msg.member.kind;
	slot->offline          = msg.member.is_offline;

	AttachLocalMember(*slot, true);

	if (slot->member && slot->member->IsClient())
		SendBulkRaidInfoTo(slot->member->CastToClient());

	for (auto &other : members)
		if (other.member && other.member != slot->member && other.member->IsClient())
			SendRaidAdd(slot->name.c_str(), other.member->CastToClient());
}

void Raid::ApplyMemberLeftFromWorld(const ServerRaidMemberLeft_Struct &msg)
{
	SendRaidRemoveAll(msg.member_name);

	Client *rem = entity_list.GetClientByName(msg.member_name);
	if (rem) {
		entity_list.SetRaid(rem, nullptr);
		SendRaidDisband(rem);
	}

	if (RuleB(Bots, Enabled)) {
		Bot *rem_bot = entity_list.GetBotByBotName(msg.member_name);
		if (rem_bot) {
			if (rem_bot->GetOwner()) {
				rem_bot->SetFollowID(rem_bot->GetOwner()->GetID());
			}
			rem_bot->SetTarget(nullptr);
			rem_bot->SetVerifiedRaid(false);
			entity_list.SetRaid(rem_bot, nullptr);
		}
	}

	for (auto &m : members) {
		if (Strings::EqualFold(m.name, msg.member_name)) {
			m.name.clear();
			m.member = nullptr;
			m.group_number = RAID_GROUPLESS;
			m.class_ = 0;
			m.level = 0;
			m.is_group_leader = false;
			m.roles = 0;
			m.main_marker = 0;
			m.main_assister = 0;
			m.kind = RaidMemberPlayer;
			m.note.clear();
			break;
		}
	}
}

void Raid::ApplyMemberFieldsChangedFromWorld(const ServerRaidMemberFieldsChanged_Struct &msg)
{
	for (auto &m : members) {
		if (Strings::EqualFold(m.name, msg.member.member_name)) {
			m.class_           = msg.member._class;
			m.level            = msg.member.level;
			m.note             = msg.member.note;
			m.roles            = msg.member.roles;
			m.main_marker      = msg.member.main_marker;
			m.main_assister    = msg.member.main_assister;
			m.offline          = msg.member.is_offline;

			AttachLocalMember(m);

			SendRaidAddAll(m.name.c_str());
			SendClassLevelUpdateAll(m.name.c_str());
			break;
		}
	}
}

void Raid::ApplyGroupMovedFromWorld(const ServerRaidGroupMoved_Struct &msg)
{
	for (auto &m : members) {
		if (Strings::EqualFold(m.name, msg.member_name)) {
			m.group_number = msg.new_group_number;
			AttachLocalMember(m);
			SendRaidGroupChangeAll(m.name.c_str(), msg.new_group_number);
			break;
		}
	}
}

void Raid::ApplyLeaderChangedFromWorld(const ServerRaidLeaderChanged_Struct &msg)
{
	for (auto &m : members) {
		if (Strings::EqualFold(m.name, msg.leader_name))
			m.roles |= RaidBitmaskLeader;
		else
			m.roles &= ~RaidBitmaskLeader;
	}

	SendMakeLeaderPacket(msg.leader_name);
}

void Raid::ApplyGroupLeaderChangedFromWorld(const ServerRaidGroupLeaderChanged_Struct &msg)
{
	for (auto &m : members) {
		if (m.group_number == msg.group_number)
			m.is_group_leader = Strings::EqualFold(m.name, msg.new_leader_name);
	}

	SendGroupLeaderChange(msg.group_number, msg.old_leader_name, msg.new_leader_name);
}

void Raid::ApplyStateChangedFromWorld(const ServerRaidStateChanged_Struct &msg)
{
	locked    = msg.locked;
	loot_type = msg.loot_type;
	motd      = msg.motd;

	for (int i = 0; i < MAX_RAID_MAIN_ASSISTERS; ++i)
		main_assist_names[i] = msg.main_assist_names[i];
	for (int i = 0; i < MAX_RAID_MAIN_MARKERS; ++i)
		main_marker_names[i] = msg.main_marker_names[i];
	master_looter_name = msg.master_looter_name;
	for (int i = 0; i < MAX_RAID_LOOTERS; ++i)
		looter_names[i] = msg.looter_names[i];
	// marked_npcs deliberately untouched -- see ApplyFullFromWorld's comment.

	leadership_aa = msg.raid_aa;

	if (locked)
		SendRaidLock();
	else
		SendRaidUnlock();

	for (const auto &m : members)
		if (m.member && m.member->IsClient())
			SendRaidMOTD(m.member->CastToClient());
}

void Raid::ApplyAAChangedFromWorld(const RaidLeadershipAA_Struct &new_leader_aa)
{
	leadership_aa = new_leader_aa;
	SendLeadershipAAUpdate();
}

void Raid::ApplyRoleChangedFromWorld(uint8 role_id, uint8 slot, const std::string &target_name, bool clearing)
{
	switch (role_id) {
		case RaidBitmaskMainAssist:
			if (slot < MAX_RAID_MAIN_ASSISTERS) {
				if (clearing)
					main_assist_names[slot].clear();
				else
					main_assist_names[slot] = target_name;
				SendRaidAssister(slot + 1, clearing, target_name.c_str());
			}
			break;
		case RaidBitmaskMainMarker:
			if (slot < MAX_RAID_MAIN_MARKERS) {
				if (clearing)
					main_marker_names[slot].clear();
				else
					main_marker_names[slot] = target_name;
				SendRaidMarker(slot + 1, clearing, target_name.c_str());
			}
			break;
		case RaidBitmaskMasterLooter:
			SendRaidMasterLooter(clearing, target_name.c_str());
			break;
		default:
			break;
	}
}

void Raid::QueueClientsInRange(const Mob *sender, const EQApplicationPacket *app, bool ack_required /*= true*/, bool ignore_sender /*= true*/, float distance /*= 0*/) const
{
	if (sender != nullptr && sender->IsClient()) {
		for (const auto& m : members) {
			if (m.member != nullptr && (!ignore_sender || m.member != sender)) {
				/* If we don't have a distance requirement - send to all members */
				if (distance == 0 ||
					/* If negative distance - we check if current distance is greater than X */
					(distance < 0 && DistanceSquared(sender->GetPosition(), m.member->GetPosition()) >= distance * distance) ||
					/* If positive distance - we check if current distance is less than X */
					(distance > 0 && DistanceSquared(sender->GetPosition(), m.member->GetPosition()) <= distance * distance))
					m.member->QueuePacket(app, ack_required);
			}
		}
	}
}

void Raid::QueueClients(const EQApplicationPacket *app, bool ack_req) const
{
	ForEachLocalMember([&](const RaidMember& m) { m.member->QueuePacket(app, ack_req); });
}

void Raid::RaidMessage(const char *msg, const Client *c, uint8 language, uint8 lang_skill) const
{
	if (c != nullptr)
		RaidMessage(msg, c->GetCleanName(), language, lang_skill);
}

void Raid::RaidMessageString(const Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9, uint32 distance) const
{
	for (const auto& m : members)
		if (m.member != nullptr && m.member != sender)
			m.member->MessageString(type, string_id, message, message2, message3, message4, message5, message6,
				message7, message8, message9, distance);
}

// ---------------------------------------------------------------------------------------------------------------------
// Raid member logistics
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// Raid Role Functionality
// ---------------------------------------------------------------------------------------------------------------------

void Raid::MarkNPC(Client* requester, const Mob* target, uint32 number)
{
	if (requester == nullptr)
		return;

	// Same permission rule ClearNPCMarks() enforces -- this was previously
	// missing entirely (any raid member could mark, regardless of whether
	// they'd actually been delegated the ability), which defeats the whole
	// point of RaidRoleDelegateMark.
	const bool allowed = IsLeader(requester->GetCleanName()) || IsMarker(requester->GetCleanName());
	if (!allowed) {
		requester->Message(Chat::Red, "You have not been delegated the ability to mark NPCs.");
		return;
	}

	if (target != nullptr && number > 0 && number <= MAX_MARKED_NPCS) {
		const uint32 EntityID = target->GetID();
		if (EntityID > 0 && marked_npcs[number - 1] != EntityID) {
			for (int i = 0; i < MAX_MARKED_NPCS; ++i) {
				if (marked_npcs[i] == EntityID || (i == static_cast<int>(number - 1) && marked_npcs[i] > 0)) {
					if (Mob* m = entity_list.GetMob(marked_npcs[i]))
						m->IsTargeted(-1);

					UpdateXtargetMarkedNPC();
					UpdateXTargetMarkedNPC(i + 1, nullptr);
					marked_npcs[i] = 0;
				}
			}

			marked_npcs[number - 1] = EntityID;
			if (Mob* m = entity_list.GetMob(EntityID))
				m->IsTargeted(1);

			auto outapp = new EQApplicationPacket(OP_MarkRaidNPC, sizeof(MarkNPC_Struct));
			const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
			mnpcs->TargetID = EntityID;
			mnpcs->Number = number;

			const Mob* m = entity_list.GetMob(EntityID);
			if (m != nullptr)
				strn0cpy(mnpcs->Name, m->GetCleanName(), sizeof(mnpcs->Name));

			QueueClients(outapp);
			safe_delete(outapp);

			UpdateXTargetMarkedNPC(number, m);
		}
	}
}

// Same permission rule the old world-authoritative RaidRoleClearNPCMarks
// action enforced (leader or one of the delegated markers), now checked
// locally like MarkNPC() -- marked_npcs is zone-local only, never touches
// world at all.
void Raid::ClearNPCMarks(Client* requester)
{
	if (requester == nullptr)
		return;

	const bool allowed = IsLeader(requester->GetCleanName()) || IsMarker(requester->GetCleanName());
	if (!allowed) {
		requester->Message(Chat::Red, "You have not been delegated the ability to mark NPCs.");
		return;
	}

	for (int i = 0; i < MAX_MARKED_NPCS; ++i) {
		if (marked_npcs[i] != 0) {
			if (Mob* m = entity_list.GetMob(marked_npcs[i]))
				m->IsTargeted(-1);

			marked_npcs[i] = 0;
			UpdateXTargetMarkedNPC(i + 1, nullptr);

			auto outapp = new EQApplicationPacket(OP_MarkRaidNPC, sizeof(MarkNPC_Struct));
			const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
			mnpcs->TargetID = 0;
			mnpcs->Number   = i + 1;
			QueueClients(outapp);
			safe_delete(outapp);
		}
	}
}

void Raid::ApplyDisbandFromWorld()
{
	for (const auto &m : members)
		if (m.member && m.InUse())
			entity_list.SetRaid(m.member, nullptr);

	for (auto &gid : subgroup_group_ids)
		gid = 0;

	SendRaidDisbandAll();

	entity_list.RemoveRaid(GetID()); // deletes this Raid
}

Group* Raid::GetGroupObject(uint32 group_number) const
{
	if (group_number < MAX_RAID_GROUPS) {
		uint32 gid = subgroup_group_ids[group_number];
		if (gid != 0)
			return entity_list.GetGroupByID(gid);
	}

	return nullptr;
}

bool Raid::IsGroupLeader(const char* name) const
{
	if (name)
		for (const auto& m : members)
			if (m.name == name)
				return m.is_group_leader;

	return false;
}

bool Raid::IsGroupLeader(const Client* c) const
{
	if (c)
		for (const auto& m : members)
			if (m.member == c)
				return true;

	return false;
}

uint8 Raid::RaidCount() const
{
	int count = 0;
	for (const auto& m : members)
		if (m.InUse())
			++count;

	return count;
}

uint32 Raid::GetGroupNumber(const char *name) const
{
	for (const auto& m : members)
		if (m.name == name)
			return m.group_number;

	return RAID_GROUPLESS;
}

uint32 Raid::GetGroupNumber(const Client *c) const
{
	for (const auto& m : members)
		if (m.member == c)
			return m.group_number;

	return RAID_GROUPLESS;
}

Client* Raid::GetClientByIndex(uint16 index) const
{
	if (index < MAX_RAID_MEMBERS) {
		Mob* m = members[index].member;
		if (m != nullptr && m->IsClient())
			return m->CastToClient();
	}

	return nullptr;
}

uint32 Raid::GetTotalRaidDamage(Mob* other) const
{
	uint32 total = 0;

	for (const auto& m : members)
		if (m.member != nullptr && other->CheckAggro(m.member))
			total += other->GetHateAmount(m.member, true);

	return total;
}

void Raid::TeleportRaid(const Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading)
{
	for (const auto& m : members)
		if (m.member != nullptr && m.member != sender && m.member->IsClient())
			m.member->CastToClient()->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
}

const char *Raid::GetRaidMasterLooter() const
{
	for (const auto &m : members)
		if (m.roles & RaidBitmaskMasterLooter)
			return m.name.c_str();

	return "";
}

bool Raid::IsRaidMember(const char *name) const
{
	if (name != nullptr && name[0] != '\0')
		for (const auto &m: members)
			if (Strings::EqualFold(m.name, name))
				return true;

	return false;
}

bool Raid::IsRaidMember(const Client* c) const
{
	if (c != nullptr)
		for (const auto &m: members)
			if (m.member == c)
				return true;

	return false;
}

uint32 Raid::GetHighestLevel() const
{
	uint32 highlvl = 0;
	for (const auto& m : members)
		if (m.InUse() && m.level > highlvl)
			highlvl = m.level;

	return highlvl;
}

uint32 Raid::GetLowestLevel() const
{
	uint32 lowlvl = std::numeric_limits<uint32>::max();
	for (const auto& m : members)
		if (m.InUse() && m.level < lowlvl)
			lowlvl = m.level;

	return lowlvl;
}

/*
 * Packet Functions Start
 */
void Raid::SendRaidCreate(Client *to) const
{
	if (to != nullptr) {
		auto outapp = new EQApplicationPacket(OP_RaidJoin, sizeof(RaidCreate_Struct));
		const auto rc = reinterpret_cast<RaidCreate_Struct*>(outapp->pBuffer);
		rc->action = raidCreate;
		strn0cpy(rc->leader_name, leader_name.c_str(), 64);
		rc->leader_id = GetLeader() ? GetLeader()->GetID() : 0;
		to->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidMembers(Client *to) const
{
	if (to != nullptr) {
		RaidMembersUpdate_Struct snapshot{};
		snapshot.action = raidMembers;
		strn0cpy(snapshot.leader_name, leader_name.c_str(), sizeof(snapshot.leader_name));
		snapshot.raid_id = GetID();
		strn0cpy(snapshot.motd, motd.c_str(), sizeof(snapshot.motd));
		snapshot.locked = IsLocked() ? 1 : 0;

		// Same opportunistic entity-id resolution as
		// SendRaidAssister/SendRaidMarker/SendRaidMasterLooter -- 0 if the
		// named member isn't physically resolvable in this zone.
		for (int i = 0; i < MAX_RAID_MAIN_ASSISTERS; ++i) {
			strn0cpy(snapshot.main_assist_name[i], main_assist_names[i].c_str(), sizeof(snapshot.main_assist_name[i]));
			if (const auto* mob = entity_list.GetMob(main_assist_names[i].c_str()))
				snapshot.main_assist_id[i] = mob->GetID();
		}
		for (int i = 0; i < MAX_RAID_MAIN_MARKERS; ++i) {
			if (const auto* mob = entity_list.GetMob(main_marker_names[i].c_str()))
				snapshot.main_marker_id[i] = mob->GetID();
		}
		if (const auto* mob = entity_list.GetMob(GetRaidMasterLooter()))
			snapshot.master_looter_id = mob->GetID();

		uint32 count = 0;
		for (const auto& m : members) {
			if (m.InUse() && count < std::size(snapshot.members)) {
				auto& out = snapshot.members[count];
				strn0cpy(out.name, m.name.c_str(), sizeof(out.name));
				strn0cpy(out.note, m.note.c_str(), sizeof(out.note));
				out.level = m.level;
				out._class = m.class_;
				out.is_raid_leader = (m.roles & RaidBitmaskLeader) ? 1 : 0;
				out.is_group_leader = m.is_group_leader ? 1 : 0;
				out.is_assist = m.main_assister != 0 ? 1 : 0;
				out.is_marker = m.main_marker != 0 ? 1 : 0;
				out.is_master_looter = (m.roles & RaidBitmaskMasterLooter) ? 1 : 0;
				out.group_number = m.group_number;

				if (m.is_group_leader && m.group_number < std::size(snapshot.group_leader_names))
					strn0cpy(snapshot.group_leader_names[m.group_number], m.name.c_str(), sizeof(snapshot.group_leader_names[0]));

				++count;
			}
		}
		snapshot.member_count = count;

		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidMembersUpdate_Struct));
		memcpy(outapp->pBuffer, &snapshot, sizeof(RaidMembersUpdate_Struct));
		to->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidAdd(const char *who, Client *to) const
{
	if (to != nullptr && who != nullptr && who[0] != '\0') {
		for (const auto& m : members) {
			if (m.InUse() && Strings::EqualFold(m.name, who)) {
				auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
				const auto ram = reinterpret_cast<RaidAddMember_Struct*>(outapp->pBuffer);
				ram->raidGen.action = raidAdd;
				ram->raidGen.parameter = m.group_number;
				strn0cpy(ram->raidGen.leader_name, m.name.c_str(), sizeof(ram->raidGen.leader_name));
				strn0cpy(ram->raidGen.player_name, m.name.c_str(), sizeof(ram->raidGen.player_name));
				ram->_class = m.class_;
				ram->level = m.level;
				ram->isGroupLeader = m.is_group_leader;
				to->QueuePacket(outapp);
				safe_delete(outapp);

				if (m.main_assister != 0)
					SendRaidAssisterTo(m.main_assister, m.name.c_str(), to);

				if (m.main_marker != 0)
					SendRaidMarkerTo(m.main_marker, m.name.c_str(), to);

				if (m.roles & RaidBitmaskMasterLooter)
					SendRaidMasterLooterTo(m.name.c_str(), to);

				return;
			}
		}
	}
}

void Raid::SendRaidAddAll(const char *who) const
{
	if (who != nullptr && who[0] != '\0') {
		for (const auto& m : members) {
			if (m.InUse() && Strings::EqualFold(m.name, who)) {
				auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
				const auto ram = reinterpret_cast<RaidAddMember_Struct*>(outapp->pBuffer);
				ram->raidGen.action = raidAdd;
				ram->raidGen.parameter = m.group_number;
				strn0cpy(ram->raidGen.leader_name, m.name.c_str(), sizeof(ram->raidGen.leader_name));
				strn0cpy(ram->raidGen.player_name, m.name.c_str(), sizeof(ram->raidGen.player_name));
				ram->isGroupLeader = m.is_group_leader;
				ram->_class = m.class_;
				ram->level = m.level;

				for (const auto &other : members)
					if (other.member && !Strings::EqualFold(other.name, who))
						other.member->QueuePacket(outapp);

				safe_delete(outapp);

				if (m.main_assister != 0)
					SendRaidAssister(m.main_assister, false, m.name.c_str());

				if (m.main_marker != 0)
					SendRaidMarker(m.main_marker, false, m.name.c_str());

				if (m.roles & RaidBitmaskMasterLooter)
					SendRaidMasterLooter(false, m.name.c_str());

				return;
			}
		}
	}
}

void Raid::SendClassLevelUpdate(const char *who, Client *to) const
{
	if (to != nullptr && who != nullptr && who[0] != '\0') {
		for (const auto& m : members) {
			if (m.InUse() && Strings::EqualFold(m.name, who)) {
				auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidUpdateClassLevel_Struct));
				const auto ruc = reinterpret_cast<RaidUpdateClassLevel_Struct*>(outapp->pBuffer);
				ruc->general.action = raidUpdateClassLevel;
				strn0cpy(ruc->general.leader_name, m.name.c_str(), sizeof(ruc->general.leader_name));
				ruc->_class = m.class_;
				ruc->level = m.level;
				to->QueuePacket(outapp);
				safe_delete(outapp);
				return;
			}
		}
	}
}

void Raid::SendClassLevelUpdateAll(const char *who) const
{
	if (who != nullptr && who[0] != '\0') {
		for (const auto& m : members) {
			if (m.InUse() && Strings::EqualFold(m.name, who)) {
				auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidUpdateClassLevel_Struct));
				const auto ruc = reinterpret_cast<RaidUpdateClassLevel_Struct*>(outapp->pBuffer);
				ruc->general.action = raidUpdateClassLevel;
				strn0cpy(ruc->general.leader_name, m.name.c_str(), sizeof(ruc->general.leader_name));
				ruc->_class = m.class_;
				ruc->level = m.level;

				QueueClients(outapp);
				safe_delete(outapp);
				return;
			}
		}
	}
}

void Raid::SendRaidRemoveAll(const char* who) const
{
	for (const auto& m : members) {
		if (m.InUse() && Strings::EqualFold(m.name, who)) {
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
			rg->action = raidRemove2;
			strn0cpy(rg->leader_name, who, 64);
			strn0cpy(rg->player_name, who, 64);
			rg->parameter = 0;
			QueueClients(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidDisband(Client *to)
{
	if (to != nullptr) {
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
		const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
		rg->action = raidDisband;
		strn0cpy(rg->leader_name, to->GetName(), 64);
		strn0cpy(rg->player_name, to->GetName(), 64);
		rg->parameter = 0;
		to->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidDisbandAll() const
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
	rg->action = raidDisband;
	strn0cpy(rg->leader_name, "RaidMember", 64);
	strn0cpy(rg->player_name, "RaidMember", 64);
	rg->parameter = 0;
	QueueClients(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidGroupChangeAll(const char *who, uint32 new_group) const
{
	if (who != nullptr && who[0] != '\0') {
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
		const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
		rg->action = raidMoveGroup;
		strn0cpy(rg->leader_name, who, sizeof(rg->leader_name));
		strn0cpy(rg->player_name, who, sizeof(rg->player_name));
		rg->parameter = new_group;
		QueueClients(outapp);
		safe_delete(outapp);
	}
}

// Called from multiple contexts on purpose: a direct local call on warm
// zone-in (client_packet.cpp, raid data already cached here), cold-start
// completion once world's reply lands (worldserver.cpp), and from the
// Apply*FromWorld handlers when a new member joins. This actually matches
// Group's real equivalent (SendFullGroupUpdate() is also called directly at
// zone-in, no world round-trip) -- raid additionally needs the cold/async
// path because raid membership is more likely to span zones than group
// membership.
void Raid::SendBulkRaidInfoTo(Client *c) const
{
	if (c != nullptr) {
		if (c->ClientVersion() >= EQ::versions::ClientVersion::RoF2) {
			// TODO: This is probably true for SoD+, but previous clients would need testing
			SendRaidMembers(c);
		} else {
			SendRaidCreate(c);
			SendRaidAdd(c->GetName(), c);
			SendBulkRaid(c);
			SendRaidMOTD(c);
			SendMakeLeaderPacketTo(leader_name.c_str(), c);
			if (IsLocked())
				SendRaidLockTo(c);
		}

		// technically this is in the bulk packet, but it's not reversed. gap is probably acceptable
		SendLeadershipAAUpdate();

		SendAssistTarget(c);
		SendMarkTargets(c);
	}
}

void Raid::SendBulkRaid(Client *to) const
{
	if (to != nullptr) {
		for (const auto& m : members)
			if (m.InUse() && !Strings::EqualFold(m.name, to->GetName()))
				SendRaidAdd(m.name.c_str(), to);

		SendRaidNotes();
	}
}

void Raid::SendMakeLeaderPacket(const char *who) const
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	const auto rg = reinterpret_cast<RaidLeadershipUpdate_Struct*>(outapp->pBuffer);
	rg->action = raidMakeLeader;
	strn0cpy(rg->leader_name, who, 64);
	strn0cpy(rg->player_name, who, 64);
	memcpy(&rg->raid, &leadership_aa, sizeof(RaidLeadershipAA_Struct));
	QueueClients(outapp);
	safe_delete(outapp);
}

void Raid::SendMakeLeaderPacketTo(const char *who, Client *to) const
{
	if (to != nullptr) {
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
		const auto rg = reinterpret_cast<RaidLeadershipUpdate_Struct*>(outapp->pBuffer);
		rg->action = raidMakeLeader;
		strn0cpy(rg->leader_name, who, 64);
		strn0cpy(rg->player_name, who, 64);
		memcpy(&rg->raid, &leadership_aa, sizeof(RaidLeadershipAA_Struct));
		to->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidLock() const
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
	rg->action = raidLock;
	strn0cpy(rg->leader_name, leader_name.c_str(), 64);
	strn0cpy(rg->player_name, leader_name.c_str(), 64);
	QueueClients(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidUnlock() const
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
	rg->action = raidUnlock;
	strn0cpy(rg->leader_name, leader_name.c_str(), 64);
	strn0cpy(rg->player_name, leader_name.c_str(), 64);
	QueueClients(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidLockTo(Client *c)
{
	if (c != nullptr) {
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
		const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
		rg->action = raidLock;
		strn0cpy(rg->leader_name, c->GetName(), 64);
		strn0cpy(rg->player_name, c->GetName(), 64);
		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidUnlockTo(Client *c)
{
	if (c != nullptr) {
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
		const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
		rg->action = raidUnlock;
		strn0cpy(rg->leader_name, c->GetName(), 64);
		strn0cpy(rg->player_name, c->GetName(), 64);
		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

// Raid doesn't decide group leadership -- this only fires from
// ApplyGroupLeaderChangedFromWorld(), reacting to a fact Group already
// decided and reported to world. This just updates the raid window's own
// bookkeeping/packet for it.
void Raid::SendGroupLeaderChange(uint32 group_number, const char *old_leader_name, const char *new_leader_name) const
{
	if (group_number < MAX_RAID_GROUPS) {
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
		const auto rg = reinterpret_cast<RaidGeneral_Struct*>(outapp->pBuffer);
		rg->action = raidChangeGroupLeader;
		rg->parameter = group_number;
		strn0cpy(rg->player_name, old_leader_name != nullptr ? old_leader_name : "", sizeof(rg->player_name));
		strn0cpy(rg->leader_name, new_leader_name != nullptr ? new_leader_name : "", sizeof(rg->leader_name));
		QueueClients(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidMOTD(Client* c) const
{
	if (c != nullptr && !motd.empty() && !c->IsBot()) {
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidMOTD_Struct));
		const auto data = reinterpret_cast<RaidMOTD_Struct*>(outapp->pBuffer);

		data->general.action = raidSetMotd;
		data->general.parameter = 0;
		data->general.unknown1 = 0;
		strn0cpy(data->general.leader_name, c->GetName(), sizeof(c->GetName()));
		strn0cpy(data->general.player_name, GetLeaderName().c_str(), 64);
		strn0cpy(data->motd, motd.c_str(), sizeof(data->motd));

		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidMOTD() const
{
	if (!motd.empty())
		for (const auto& m: members)
			if (m.member != nullptr && m.member->IsClient())
				SendRaidMOTD(m.member->CastToClient());
}

void Raid::SendLeadershipAAUpdate() const
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	const auto rlaa = reinterpret_cast<RaidLeadershipUpdate_Struct*>(outapp->pBuffer);
	rlaa->action = raidSetLeaderAbilities;
	rlaa->raid = leadership_aa;

	ForEachClient([&rlaa, &outapp](Client* c) {
		strn0cpy(rlaa->leader_name, c->GetName(), 64);
		strn0cpy(rlaa->player_name, c->GetName(), 64);
		c->QueuePacket(outapp);
	});

	safe_delete(outapp);
}

uint32 Raid::GetLeadershipAA(int AAID) const
{
	if (AAID >= raidAAMarkNPC)
		return leadership_aa.ranks[AAID - raidAAMarkNPC];

	return  0;
}

void Raid::AttachLocalMember(RaidMember &m, bool is_new_member)
{
	Mob* previous = m.member;

	if (!m.InUse()) {
		m.member = nullptr;
	} else {
		auto* c = entity_list.GetClientByName(m.name.c_str());
		auto* b = entity_list.GetBotByBotName(m.name.c_str());

		if (c != nullptr) {
			m.member = c;
			m.kind = RaidMemberPlayer;
		} else if (RuleB(Bots, Enabled) && b != nullptr) {
			m.member = b;
			m.kind = RaidMemberBot;
		} else {
			m.member = nullptr;
		}
	}

	// IsRaided() now derives straight from the cached Client::m_raid/
	// Bot::m_raid pointer (HasRaid<M>::IsRaided() { return m_raid != nullptr; }),
	// so this is the single point where m.member is ever reassigned away
	// from a previous value, and the single point a live Mob* actually
	// starts/stops occupying a raid slot for the join/re-resolve paths
	// (ApplyMemberJoinedFromWorld, cold-start, zone-transfer re-attach);
	// ApplyMemberLeftFromWorld/ApplyDisbandFromWorld clear it themselves
	// since they don't route through here.
	if (previous != nullptr && previous != m.member)
		entity_list.SetRaidZoned(previous, nullptr);

	if (m.member != nullptr) {
		if (is_new_member)
			entity_list.SetRaid(m.member, this);
		else
			entity_list.SetRaidZoned(m.member, this);
	}

	for (int i = 0; i < MAX_RAID_MAIN_MARKERS; i++)
		if (m.main_marker == i + 1)
			main_marker_names[i] = m.name;

	for (int i = 0; i < MAX_RAID_MAIN_ASSISTERS; i++)
		if (m.main_assister == i + 1)
			main_assist_names[i] = m.name;

	if (m.roles & RaidBitmaskLeader) {
		if (m.InUse()) {
			// World rejects RaidLeadershipMakeLeader when the target is a
			// bot, so m.member here should always be a Client in practice --
			// still guard rather than assume, since this also runs during
			// cold-start application of world data.
			SetLeader(m.member && m.member->IsClient() ? m.member->CastToClient() : nullptr);
			leader_name = m.name;
		} else {
			SetLeader(nullptr);
		}
	}
}

void Raid::MemberZonedIn(const Client *c)
{
	if (c != nullptr) {
		for (auto &m : members) {
			if (Strings::EqualFold(m.name, c->GetCleanName())) {
				AttachLocalMember(m);
				break;
			}
		}
	}
}

void Raid::MemberZoned(const Mob* removemob)
{
	if (removemob != nullptr) {
		if (leader == removemob)
			leader = nullptr;

		for (auto& m : members) {
			if (m.member == removemob) {
				// Same reasoning as Group::MemberZoned -- membership is
				// unchanged, just detaching a live Mob* on zone-out, but the
				// cached pointer on the (about-to-be-destroyed) Mob object
				// should still go to nullptr.
				entity_list.SetRaidZoned(m.member, nullptr);
				m.member = nullptr;
				break;
			}
		}
	}
}

uint16 Raid::GetAvgLevel() const
{
	double levelHolder = 0;
	uint8 numMem = 0;
	for (const auto& m : members) {
		if (m.InUse()) {
			levelHolder += m.level;
			++numMem;
		}
	}

	return static_cast<uint16>(std::round(levelHolder/numMem)); // total levels divided by num of characters
}

const char *Raid::GetClientNameByIndex(uint8 index) const
{
	return members[index].name.c_str();
}

void Raid::SetDirtyAutoHaters() const
{
	ForEachLocalMember([](const RaidMember& m) { m.member->SetDirtyAutoHaters(); });
}

std::vector<RaidMember> Raid::GetMembers() const
{
	std::vector<RaidMember> raid_members;
	for (const auto& m : members)
		if (m.InUse())
			raid_members.emplace_back(m);
	return raid_members;
}

bool Raid::AnyMemberHasDzLockout(const std::string& expedition, const std::string& event) const
{
	std::vector<std::string> names;
	for (const auto& mbr : members) {
		if (mbr.kind != RaidMemberBot) {
			if (mbr.member == nullptr && mbr.InUse()) {
				names.emplace_back(mbr.name); // out of zone member
			} else if (mbr.member != nullptr && mbr.member->HasDzLockout(expedition, event)) {
				return true;
			}
		}
	}

	return !CharacterExpeditionLockoutsRepository::GetLockouts(database, names, expedition, event).empty();
}

bool Raid::IsEngaged() const
{
	bool engaged = false;
	ForEachLocalMember([&](const RaidMember& m) -> bool {
		engaged = m.member->IsEngaged() || m.member->GetAggroCount() > 0;
		return !engaged;
	});

	return engaged;
}

void Raid::RaidMessage(const char* msg, const char* from, uint8 language, uint8 lang_skill) const
{
	if (from != nullptr && from[0] != '\0') {
		auto pack = new ServerPacket(ServerOP_RaidSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
		const auto rga = reinterpret_cast<ServerRaidMessage_Struct*>(pack->pBuffer);
		rga->rid = GetID();
		rga->group_number = RAID_GROUPLESS;
		rga->language = language;
		rga->lang_skill = lang_skill;
		strn0cpy(rga->from, from, 64);
		strcpy(rga->message, msg);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Raid::SendRaidNotes() const
{
	for (const auto& c : members) {
		if (!c.note.empty()) {
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidNote_Struct));
			const auto data = reinterpret_cast<RaidNote_Struct*>(outapp->pBuffer);

			data->general.action = raidSetNote;
			data->general.parameter = 0;
			data->general.unknown1 = 0;
			strn0cpy(data->general.leader_name, c.name.c_str(), sizeof(data->general.leader_name));
			strn0cpy(data->general.player_name, GetLeaderName().c_str(), GetLeaderName().length());
			strn0cpy(data->note, c.note.c_str(), sizeof(data->note));

			QueueClients(outapp);
			safe_delete(outapp);
		}
	}
}

void Raid::UpdateRaidXTargets() const
{
	struct AssistUpdate {
		XTargetType assist_type;
		XTargetType assist_target_type;
		int32       slot;
	};

	std::vector<AssistUpdate> assist_updates = {
		AssistUpdate{.assist_type = RaidAssist1, .assist_target_type = RaidAssist1Target, .slot = MAIN_ASSIST_1_SLOT},
		AssistUpdate{.assist_type = RaidAssist2, .assist_target_type = RaidAssist2Target, .slot = MAIN_ASSIST_2_SLOT},
		AssistUpdate{.assist_type = RaidAssist3, .assist_target_type = RaidAssist3Target, .slot = MAIN_ASSIST_3_SLOT},
	};

	for (const auto& u : assist_updates) {
		if (!main_assist_names[u.slot].empty()) {
			auto m = entity_list.GetMob(main_assist_names[u.slot].c_str());
			if (m != nullptr) {
				UpdateXTargetType(u.assist_type, m, m->GetName());
				auto n = m->GetTarget();
				if (n != nullptr && n->GetHP() > 0)
					UpdateXTargetType(u.assist_target_type, n, n->GetName());
				else
					UpdateXTargetType(u.assist_target_type, nullptr);
			}
		} else {
			UpdateXTargetType(u.assist_type, nullptr);
			UpdateXTargetType(u.assist_target_type, nullptr);
		}
	}

	struct MarkedUpdate {
		XTargetType mark_target;
		int32       slot;
	};

	std::vector<MarkedUpdate> marked_updates = {
		MarkedUpdate{.mark_target = RaidMarkTarget1, .slot = MAIN_MARKER_1_SLOT},
		MarkedUpdate{.mark_target = RaidMarkTarget2, .slot = MAIN_MARKER_2_SLOT},
		MarkedUpdate{.mark_target = RaidMarkTarget3, .slot = MAIN_MARKER_3_SLOT},
	};

	for (auto& u : marked_updates) {
		if (marked_npcs[u.slot] != 0) {
			auto m = entity_list.GetMob(marked_npcs[u.slot]);
			if (m != nullptr && m->GetHP() > 0)
				UpdateXTargetType(u.mark_target, m, m->GetName());
			else
				UpdateXTargetType(u.mark_target, nullptr);
		} else {
			UpdateXTargetType(u.mark_target, nullptr);
		}
	}
}

void Raid::UpdateXTargetType(XTargetType Type, const Mob *m, const char *name) const
{
	ForEachLocalMember([&](const RaidMember& rm) { rm.member->UpdateXTargetType(Type, m, name); });
}

void Raid::UpdateXTargetMarkedNPC(uint32 number, const Mob* m) const
{
	ForEachClient([&number, &m](Client* c) {
		c->UpdateXTargetType(
			number == 1 ? RaidMarkTarget1 :
			number == 2 ? RaidMarkTarget2 :
			RaidMarkTarget3, m);
	});
}

void Raid::UpdateXtargetMarkedNPC() const
{
	for (int i = 0; i < MAX_MARKED_NPCS; i++) {
		const auto mm = entity_list.GetNPCByID(marked_npcs[i]);
		if (mm)
			UpdateXTargetType(static_cast<XTargetType>(RaidMarkTarget1 + i), mm->CastToMob(), mm->CastToMob()->GetName());
		else
			UpdateXTargetType(static_cast<XTargetType>(RaidMarkTarget1 + i), nullptr);
	}
}

// Send a packet to the entire raid notifying them of the group target selected by the Main Assist.
void Raid::SendRaidAssistTarget() const
{
	uint16 assist_target_id = 0;
	uint16 number = 0;
	Mob* target = nullptr;

	struct AssistTypes {
		MainAssistSlot main_assist_type_slot;
		uint32 main_assist_number; // 1-indexed wire slot number (main_assist_type_slot + 1)
	};

	std::vector<AssistTypes> assist_types = {
		{.main_assist_type_slot = MAIN_ASSIST_1_SLOT, .main_assist_number = MAIN_ASSIST_1_SLOT + 1},
		{.main_assist_type_slot = MAIN_ASSIST_2_SLOT, .main_assist_number = MAIN_ASSIST_2_SLOT + 1},
		{.main_assist_type_slot = MAIN_ASSIST_3_SLOT, .main_assist_number = MAIN_ASSIST_3_SLOT + 1}
	};

	for (auto &a: assist_types) {
		if (!main_assist_names[a.main_assist_type_slot].empty()) {
			auto player = entity_list.GetMob(main_assist_names[a.main_assist_type_slot].c_str());
			if (player != nullptr) {
				target = player->GetTarget();
				if (target != nullptr) {
					assist_target_id = target->GetID();
					number           = a.main_assist_number;
					break;
				}
			}
		}
	}

	if (assist_target_id) {
		auto outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));
		const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
		mnpcs->TargetID = assist_target_id;
		mnpcs->Number = number;

		for (const auto& m : members)
			if (m.member != nullptr)
				m.member->QueuePacket(outapp);

		safe_delete(outapp);
	}
}

void Raid::SendAssistTarget(Client *c) const
{
	if (c != nullptr) {
		uint32 assist_target_id = 0;
		uint32 number           = 0;

		struct AssistTypes {
			MainAssistSlot main_assist_type_slot;
			uint32 main_assist_number; // 1-indexed wire slot number (main_assist_type_slot + 1)
		};

		std::vector<AssistTypes> assist_types = {
			{.main_assist_type_slot = MAIN_ASSIST_1_SLOT, .main_assist_number = MAIN_ASSIST_1_SLOT + 1},
			{.main_assist_type_slot = MAIN_ASSIST_2_SLOT, .main_assist_number = MAIN_ASSIST_2_SLOT + 1},
			{.main_assist_type_slot = MAIN_ASSIST_3_SLOT, .main_assist_number = MAIN_ASSIST_3_SLOT + 1}
		};

		for (const auto& a : assist_types) {
			if (!main_assist_names[a.main_assist_type_slot].empty()) {
				const auto player = entity_list.GetMob(main_assist_names[a.main_assist_type_slot].c_str());
				if (player != nullptr) {
					const Mob* target = player->GetTarget();
					if (target != nullptr) {
						assist_target_id = target->GetID();
						number           = a.main_assist_number;
						break;
					}
				}
			}
		}

		if (assist_target_id != 0) {
			auto outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));
			const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
			mnpcs->TargetID = assist_target_id;
			mnpcs->Number   = number;
			c->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

bool Raid::IsAssister(const char* who) const
{
	for (const auto & main_assist_name : main_assist_names) {
		if (Strings::EqualFold(main_assist_name, who)) {
			return true;
		}
	}

	return false;
}

// Verified against tob/packet_analysis/OP_RaidDelegateAbility.md: this is a
// direct passthrough on TOB (no ENCODE override), so DelegateAbility_Struct's
// fields travel to the client unchanged. Doesn't require a resolvable Mob --
// the client needs Name for its chat message ("X is now/no longer...") even
// when the delegatee isn't physically present in this zone; EntityID (used
// for the icon target) just falls back to 0 in that case.
EQApplicationPacket* Raid::BuildDelegateAbilityPacket(uint32 ability, uint32 member_number, bool clearing, const char* name)
{
	if (name == nullptr || name[0] == '\0')
		return nullptr;

	auto outapp = new EQApplicationPacket(OP_RaidDelegateAbility, sizeof(DelegateAbility_Struct));
	const auto das = reinterpret_cast<DelegateAbility_Struct*>(outapp->pBuffer);
	das->DelegateAbility = ability;
	das->MemberNumber    = member_number;
	das->Action          = clearing ? ClearDelegate : SetDelegate;
	if (const auto* mob = entity_list.GetMob(name))
		das->EntityID = mob->GetID();
	strn0cpy(das->Name, name, sizeof(das->Name));
	return outapp;
}

void Raid::SendRaidAssisterTo(uint32 slot, const char* assister, Client* to)
{
	if (to == nullptr)
		return;

	if (auto* outapp = BuildDelegateAbilityPacket(RaidDelegateMainAssist, slot, false, assister)) {
		to->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidAssister(uint32 slot, bool clearing, const char* assister) const
{
	if (auto* outapp = BuildDelegateAbilityPacket(RaidDelegateMainAssist, slot, clearing, assister)) {
		QueueClients(outapp);
		safe_delete(outapp);
	}
}

bool Raid::IsMarker(const char* who) const
{
	return std::ranges::any_of(main_marker_names, [&who](const auto& marker) {
		return Strings::EqualFold(marker, who);
	});
}

void Raid::SendRaidMarkerTo(uint32 slot, const char* marker, Client* to)
{
	if (to == nullptr)
		return;

	if (auto* outapp = BuildDelegateAbilityPacket(RaidDelegateMainMarker, slot, false, marker)) {
		to->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidMarker(uint32 slot, bool clearing, const char* marker) const
{
	if (auto* outapp = BuildDelegateAbilityPacket(RaidDelegateMainMarker, slot, clearing, marker)) {
		QueueClients(outapp);
		safe_delete(outapp);
	}
}

// Analog of SendRaidAssisterTo/SendRaidMarkerTo -- was never written at all
// (the client-facing OP_RaidDelegateAbility push for master looter simply
// didn't exist), so master looter delegation was previously silent: state
// changed on world and in each member's roster row, but no client ever
// learned who currently holds it except via a cold full resync. MemberNumber
// is always 0 -- the client hardcodes an internal slot for this ability type
// (see the packet analysis doc), it doesn't read the field for MasterLooter.
void Raid::SendRaidMasterLooterTo(const char* looter, Client* to)
{
	if (to == nullptr)
		return;

	if (auto* outapp = BuildDelegateAbilityPacket(RaidDelegateMasterLooter, 0, false, looter)) {
		to->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendRaidMasterLooter(bool clearing, const char* looter) const
{
	if (auto* outapp = BuildDelegateAbilityPacket(RaidDelegateMasterLooter, 0, clearing, looter)) {
		QueueClients(outapp);
		safe_delete(outapp);
	}
}

void Raid::SendMarkTargets(const Client* c) const
{
	if (c != nullptr) {
		for (int i = 0; i < MAX_MARKED_NPCS; i++) {
			if (marked_npcs[i] > 0) {
				const auto marked_mob = entity_list.GetMob(marked_npcs[i]);
				if (marked_mob != nullptr) {
					auto outapp = new EQApplicationPacket(OP_MarkRaidNPC, sizeof(MarkNPC_Struct));
					const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
					mnpcs->TargetID = marked_mob->GetID();
					mnpcs->Number = i + 1;
					strcpy(mnpcs->Name, marked_mob->GetCleanName());
					QueueClients(outapp);
					safe_delete(outapp);
				}
			}
		}
		UpdateXtargetMarkedNPC();
	}
}

// Ordinary (non-template) definition -- lives here because this is where
// Raid is actually complete, unlike mob.h. Since RaidMembership isn't a
// template, this is the one and only definition needed regardless of how
// many concrete Mob-derived classes mix HasRaid<M> in -- no explicit
// instantiation list to keep in sync.
void RaidMembership::SetRaid(Raid* raid)
{
	m_raid = raid;
	m_raid_id = raid != nullptr ? raid->GetID() : 0;
}

