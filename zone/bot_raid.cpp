/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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

#include "bot.h"
#include "object.h"
#include "raids.h"
#include "doors.h"
#include "quest_parser_collection.h"
#include "../common/data_verification.h"

extern volatile bool is_zone_loaded;
extern bool Critical; 

std::vector<RaidMember> Raid::GetRaidGroupMembers(uint32 gid) 
{
	std::vector<RaidMember> raid_group_members;
	raid_group_members.clear();

	for (int i = 0; i < MAX_RAID_MEMBERS; ++i)
	{
		if (members[i].member && members[i].GroupNumber == gid)
		{
			raid_group_members.push_back(members[i]);
		}
	}
	return raid_group_members;
}

// Returns Bot members that are in the raid
// passing in owner will return all Bots that have the same owner.
std::vector<Bot*> Raid::GetRaidBotMembers(uint32 owner)
{
	std::vector<Bot*> raid_members_bots;
	raid_members_bots.clear();

	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (
			members[i].member &&
			members[i].member->IsBot()
			) {
			auto b_member = members[i].member->CastToBot();
			if (owner && b_member->GetBotOwnerCharacterID() == owner) {
				raid_members_bots.emplace_back(b_member);
			} else if (!owner) {
				raid_members_bots.emplace_back(b_member);
			}
		}
	}

	return raid_members_bots;
}

// Returns Bot members that are in the group specified
// passing in owner will return only Bots that have the same owner.
std::vector<Bot*> Raid::GetRaidGroupBotMembers(uint32 gid)
{
	std::vector<Bot*> raid_members_bots;
	raid_members_bots.clear();

	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (
			members[i].member &&
			members[i].member->IsBot() &&
			members[i].GroupNumber == gid
		) {
			auto b_member = members[i].member->CastToBot();
			raid_members_bots.emplace_back(b_member);
			raid_members_bots.emplace_back(b_member);
		}
	}

	return raid_members_bots;
}

void Raid::HandleBotGroupDisband(uint32 owner, uint32 gid)
{
	auto raid_members_bots = gid != RAID_GROUPLESS ? GetRaidGroupBotMembers(gid) : GetRaidBotMembers(owner);

	// If any of the bots are a group leader then re-create the botgroup on disband, dropping any clients
	for (auto& bot_iter: raid_members_bots) {

		// Remove the entire BOT group in this case
		if (
			bot_iter &&
			gid != RAID_GROUPLESS &&
			IsRaidMember(bot_iter->GetName()) &&
			IsGroupLeader(bot_iter->GetName())
			) {
			auto r_group_members = GetRaidGroupMembers(GetGroup(bot_iter->GetName()));
			auto group_inst = new Group(bot_iter);
			entity_list.AddGroup(group_inst);
			database.SetGroupID(bot_iter->GetCleanName(), group_inst->GetID(), bot_iter->GetBotID());
			database.SetGroupLeaderName(group_inst->GetID(), bot_iter->GetName());

			for (auto member_iter: r_group_members) {
				if (member_iter.member->IsBot()) {
					auto b_member = member_iter.member->CastToBot();
					if (strcmp(b_member->GetName(), bot_iter->GetName()) == 0) {
						bot_iter->SetFollowID(owner);
					} else {
						Bot::AddBotToGroup(b_member, group_inst);
					}
					Bot::RemoveBotFromRaid(b_member);
				}
			}
		} else {
			Bot::RemoveBotFromRaid(bot_iter);
		}
	}
}

uint8 Bot::GetNumberNeedingHealedInRaidGroup(uint8& need_healed, uint8 hpr, bool includePets, Raid* raid) {

	if (raid) {
		uint32 r_group = raid->GetGroup(GetName());
		auto raid_group_members = raid->GetRaidGroupMembers(r_group);

		for (auto& m: raid_group_members) {
			if (m.member && !m.member->qglobal) {
				if (m.member->GetHPRatio() <= hpr) {
					need_healed++;
				}

				if (includePets) {
					if (m.member->GetPet() && m.member->GetPet()->GetHPRatio() <= hpr) {
						need_healed++;
					}
				}
			}
		}
	}
	return need_healed;
}

void Bot::ProcessRaidInvite(Mob* invitee, Client* invitor, bool group_invite) {

	if (!invitee || !invitor) {
		return;
	}

	if (invitee->IsBot() && invitee->CastToBot()->GetBotOwnerCharacterID() != invitor->CharacterID()) {
		invitor->Message(
			Chat::Red,
			fmt::format(
				"{} is not your Bot. You can only invite your Bots, or players grouped with bots.",
				invitee->GetCleanName()
			).c_str()
		);
		return;
	}

	Raid* raid = entity_list.GetRaidByClient(invitor);

	Bot::CreateBotRaid(invitee, invitor, group_invite, raid);
}

void Bot::CreateBotRaid(Mob* invitee, Client* invitor, bool group_invite, Raid* raid) {

	Group* g_invitee = invitee->GetGroup();
	Group* g_invitor = invitor->GetGroup();

	if (g_invitee && invitor->IsClient()) {
		if (!g_invitee->IsLeader(invitee)) {
			invitor->Message(
				Chat::Red,
				fmt::format(
					"You can only invite group leaders or ungrouped bots. Try {} instead.",
					g_invitee->GetLeader()->GetCleanName()
				).c_str()
			);
			return;
		}
	}

	bool new_raid = false;
	if (!raid) {
		new_raid = true;
		raid = new Raid(invitor);
		entity_list.AddRaid(raid);
		raid->SetRaidDetails();
	}

	// Add Invitor to new raid
	if (new_raid) {
		if (g_invitor) {
			ProcessBotGroupAdd(g_invitor, raid, nullptr, true, true);
		} else {
			raid->SendRaidCreate(invitor);
			raid->AddMember(invitor, 0xFFFFFFFF, true, false, true);
			raid->SendMakeLeaderPacketTo(invitor->GetName(), invitor);
			if (raid->IsLocked()) {
				raid->SendRaidLockTo(invitor);
			}
		}
	}

	// Add Bot Group or Client Bot Group to raid
	if (g_invitee) {
		ProcessBotGroupAdd(g_invitee, raid);

	// Add individual client to raid
	} else if (invitee->IsClient()) {
		ProcessBotGroupAdd(g_invitee, raid, invitee->CastToClient());

	// Add individual bot to raid
	} else {
		auto gid = raid->GetGroup(invitor->GetName());
		auto b = invitee->CastToBot();

		// gives us a choice to either invite directly into the clients Raid Group, or just into the Raid
		if (group_invite && raid->GroupCount(gid) < MAX_GROUP_MEMBERS) {
			raid->AddBot(b, gid);
		} else {
			raid->AddBot(b);
		}

		if (new_raid) {
			invitee->SetFollowID(invitor->GetID());
		}
	}
}

void Bot::ProcessBotGroupAdd(Group* group, Raid* raid, Client* client, bool new_raid, bool initial) {

	uint32 raid_free_group_id = raid->GetFreeGroup();
	if (group) {
		for (int x = 0; x < MAX_GROUP_MEMBERS; x++) {
			if (group->members[x]) {
				Client* c = nullptr;
				Bot* b = nullptr;

				if (group->members[x] && group->members[x]->IsBot()) {
					b = group->members[x]->CastToBot();
					raid->AddBot(b, raid_free_group_id, false, x == 0, false);
				} else if (group->members[x] && group->members[x]->IsClient()) {
					c = group->members[x]->CastToClient();
					raid->SendRaidCreate(c);
					raid->AddMember(
						c,
						raid_free_group_id,
						new_raid,
						x == 0,
						false
					);
					raid->SendMakeLeaderPacketTo(raid->leadername, c);
					raid->SendBulkRaid(c);
				}
			}
		}
		group->JoinRaidXTarget(raid, initial);
		group->DisbandGroup(true);
	} else if (client) {
		raid->SendRaidCreate(client);
		raid->AddMember(client, RAID_GROUPLESS, false, false, true);
		raid->SendMakeLeaderPacketTo(raid->leadername, client);
		raid->SendBulkRaid(client);
		if (raid->IsLocked()) {
			raid->SendRaidLockTo(client);
		}
	}

	raid->GroupUpdate(raid_free_group_id);
}


