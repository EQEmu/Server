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
#include "groups.h"

#include "common/eqemu_logsys.h"
#include "common/events/player_event_logs.h"
#include "common/patches/tob_structs.h"
#include "common/repositories/character_expedition_lockouts_repository.h"
#include "zone/bot_structs.h"
#include "zone/dynamic_zone.h"
#include "zone/masterentity.h"
#include "zone/queryserv.h"
#include "zone/string_ids.h"
#include "zone/worldserver.h"

extern QueryServ  *QServ;

/*
note about how groups work:
A group contains 2 list, a list of pointers to members and a
list of member names. All members of a group should have their
name in the member_name array, whether they are in the zone or not.
Only members in this zone will have non-null pointers in the
members array.
*/

Group::Group(uint32 gid)
{
	SetID(gid);
}

//creating a new group
Group::Group(Mob* leader)
{
	members[0].mob = leader;
	members[0].name = leader->GetName();
	members[0].roles = GroupBitmaskLeader;

	if (leader->IsClient())
		leader->CastToClient()->UpdateGroupMembers(this);

	leader_name = leader->GetName();
}

Group::~Group()
{
	for (const unsigned short MarkedNPC : MarkedNPCs)
		if (Mob * m = entity_list.GetMob(MarkedNPC))
			m->IsTargeted(-1);
}

void Group::Process() {}

// ---------------------------------------------------------------------------------------------------------------------
// Group Game Functionality
// ---------------------------------------------------------------------------------------------------------------------

//Split money used in OP_Split (/split and /autosplit).
void Group::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, const Client *splitter, bool share) const
{
	if ((copper > 0 || silver > 0 || gold > 0 || platinum > 0) && splitter != nullptr) {
		bool  splitter_in_group = false;

		std::vector<Client*> groupClients;
		ForEachClient([&](Client* c) {
			groupClients.push_back(c);
			if (c == splitter)
				splitter_in_group = true;
		});

		if (!groupClients.empty() && splitter_in_group) {
			const int member_count = static_cast<int>(groupClients.size());
			const Client* random_member = groupClients[zone->random.Int(0, member_count - 1)];

			const uint32 copper_split       = copper / member_count;
			const uint32 copper_remainder   = copper % member_count;
			const uint32 silver_split       = silver / member_count;
			const uint32 silver_remainder   = silver % member_count;
			const uint32 gold_split         = gold / member_count;
			const uint32 gold_remainder     = gold % member_count;
			const uint32 platinum_split     = platinum / member_count;
			const uint32 platinum_remainder = platinum % member_count;

			for (const auto client : groupClients) {
				uint32 receive_copper = copper_split;
				uint32 receive_silver = silver_split;
				uint32 receive_gold = gold_split;
				uint32 receive_platinum = platinum_split;

				// if /split is used then splitter gets the remainder + split.
				// if /autosplit is used then random players in the group will get the remainder + split.
				if (share ? client == splitter : client == random_member) {
					receive_copper += copper_remainder;
					receive_silver += silver_remainder;
					receive_gold += gold_remainder;
					receive_platinum += platinum_remainder;
				}

				if (share && client != splitter)
					client->MessageString(Chat::MoneySplit, SHARE_MONEY, splitter->GetCleanName());

				// Check if there are any coins to add to the player's purse.
				if (receive_copper > 0 || receive_silver > 0 || receive_gold > 0 || receive_platinum > 0) {
					client->AddMoneyToPP(receive_copper, receive_silver, receive_gold, receive_platinum, true);
					client->MessageString(Chat::MoneySplit, YOU_RECEIVE_AS_SPLIT,
						Strings::Money(receive_platinum, receive_gold, receive_silver, receive_copper).c_str()
					);
				}

				// If logging of player money transactions is enabled, record the transaction.
				if (PlayerEventLogs::Instance()->IsEventEnabled(PlayerEvent::SPLIT_MONEY)) {
					const auto e = PlayerEvent::SplitMoneyEvent{
						.copper               = receive_copper,
						.silver               = receive_silver,
						.gold                 = receive_gold,
						.platinum             = receive_platinum,
						.player_money_balance = client->GetCarriedMoney(),
					};
					RecordPlayerEventLogWithClient(client, PlayerEvent::SPLIT_MONEY, e);
				}
			}
		}
	}
}

void Group::SendHPManaEndPacketsTo(Client* target) const
{
	if (target != nullptr) {
		EQApplicationPacket hpapp;

		static_assert(sizeof(MobManaUpdate_Struct) == sizeof(MobEnduranceUpdate_Struct), "Mana and Endurance struct sizes differ");
		EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

		for (const auto& member : members) {
			if(member.mob != nullptr && member.mob != target) {
				member.mob->CreateHPPacket(&hpapp);
				target->QueuePacket(&hpapp, false);

				if (target->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
					outapp.SetOpcode(OP_MobManaUpdate);
					const auto mana_update = reinterpret_cast<MobManaUpdate_Struct*>(outapp.pBuffer);
					mana_update->spawn_id = member.mob->GetID();
					mana_update->mana = member.mob->GetManaPercent();
					target->QueuePacket(&outapp, false);

					outapp.SetOpcode(OP_MobEnduranceUpdate);
					const auto endurance_update = reinterpret_cast<MobEnduranceUpdate_Struct*>(outapp.pBuffer);
					endurance_update->endurance = member.mob->GetEndurancePercent();
					target->QueuePacket(&outapp, false);
				}
			}
		}
	}
}

void Group::SendHPPacketsFrom(Mob *source) const
{
	if (source != nullptr) {
		EQApplicationPacket hp_app;
		source->CreateHPPacket(&hp_app);

		static_assert(sizeof(MobManaUpdate_Struct) == sizeof(MobEnduranceUpdate_Struct), "Mana and Endurance struct sizes differ");
		EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

		ForEachClient([&](Client* c) {
			if (c != source) {
				c->QueuePacket(&hp_app);
				if (c->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
					outapp.SetOpcode(OP_MobManaUpdate);
					const auto mana_update = reinterpret_cast<MobManaUpdate_Struct*>(outapp.pBuffer);
					mana_update->spawn_id = source->GetID();
					mana_update->mana = source->GetManaPercent();
					c->QueuePacket(&outapp, false);

					outapp.SetOpcode(OP_MobEnduranceUpdate);
					const auto endurance_update = reinterpret_cast<MobEnduranceUpdate_Struct*>(outapp.pBuffer);
					endurance_update->endurance = source->GetEndurancePercent();
					c->QueuePacket(&outapp, false);
				}
			}
		});
	}
}

void Group::SendManaPacketFrom(Mob *source) const
{
	if (source != nullptr) {
		EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

		ForEachClient([&](Client* c) {
			if (c != source) {
				if (c->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
					const auto mana_update = reinterpret_cast<MobManaUpdate_Struct*>(outapp.pBuffer);
					mana_update->spawn_id = source->GetID();
					mana_update->mana = source->GetManaPercent();
					c->QueuePacket(&outapp, false);
				}
			}
		});
	}
}

void Group::SendEndurancePacketFrom(Mob* source) const
{
	if (source != nullptr) {
		EQApplicationPacket outapp(OP_MobEnduranceUpdate, sizeof(MobManaUpdate_Struct));

		ForEachClient([&](Client* c) {
			if (c != source) {
				if (c->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
					const auto endurance_update = reinterpret_cast<MobEnduranceUpdate_Struct*>(outapp.pBuffer);
					endurance_update->spawn_id = source->GetID();
					endurance_update->endurance = source->GetEndurancePercent();
					c->QueuePacket(&outapp, false);
				}
			}
		});
	}
}

void Group::SendHPManaEndPacketsFrom(Mob *source) const
{
	SendHPPacketsFrom(source);
	SendManaPacketFrom(source);
	SendEndurancePacketFrom(source);
}

void Group::CastGroupSpell(Mob* caster, int32 spell_id) const
{
	if (caster != nullptr) {
		float range = caster->GetAOERange(spell_id);
		float range2 = range*range;
		float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;

		for (const auto& member : members) {
			if (member.mob == caster) {
				caster->SpellOnTarget(spell_id, caster);
#ifdef GROUP_BUFF_PETS
				if(spells[spell_id].target_type != ST_GroupNoPets && caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
					caster->SpellOnTarget(spell_id, caster->GetPet());
#endif
			} else if (member.mob != nullptr) {
				float distance = DistanceSquared(caster->GetPosition(), member.mob->GetPosition());
				if (distance <= range2 && distance >= min_range2) {
					member.mob->CalcSpellPowerDistanceMod(spell_id, distance);
					caster->SpellOnTarget(spell_id, member.mob);
#ifdef GROUP_BUFF_PETS
					if(spells[spell_id].target_type != ST_GroupNoPets && member.mob->GetPet() && member.mob->HasPetAffinity() && !member.mob->GetPet()->IsCharmed())
						caster->SpellOnTarget(spell_id, member.mob->GetPet());
#endif
				} else
					LogSpells("Group spell: [{}] is out of range [{}] at distance [{}] from [{}]", member.mob->GetName(), range, distance, caster->GetName());
			}
		}
	}
}

uint32 Group::GetTotalGroupDamage(Mob* other) const
{
	uint32 total = 0;

	for (const auto& member : members) {
		if (member.mob != nullptr && other->CheckAggro(member.mob))
			total += other->GetHateAmount(member.mob,true);
	}

	return total;
}

int Group::GetNumberNeedingHealedInGroup(int hpr, bool include_pets) const
{
	int need_healed = 0;

	for (const auto& member : members) {
		if (member.mob != nullptr && !member.mob->qglobal) {
			if (member.mob->GetIntHPRatio() <= hpr)
				need_healed++;

			if (include_pets)
				if (member.mob->GetPet() && member.mob->GetPet()->GetIntHPRatio() <= hpr)
					need_healed++;
		}
	}

	return need_healed;
}

void Group::HealGroup(uint32 heal_amt, Mob* caster, float range) const
{
	if (caster != nullptr) {
		if (range < std::numeric_limits<float>::epsilon())
			range = 200;

		const float range2 = range * range;

		std::vector<Mob*> membersInRange;
		for (const auto& member : members)
			if (member.mob != nullptr && DistanceSquared(caster->GetPosition(), member.mob->GetPosition()) <= range2)
				membersInRange.push_back(member.mob);

		heal_amt /= membersInRange.size();
		for (const auto member : membersInRange) {
			member->HealDamage(heal_amt, caster);
			member->SendHPUpdate();
		}
	}
}

void Group::BalanceHP(int32 penalty, float range, const Mob* caster, int32 limit) const
{
	if (caster != nullptr) {
		if (range < std::numeric_limits<float>::epsilon())
			range = 200;

		const float range2 = range*range;
		int64 dmgtaken = 0;

		std::vector<Mob*> membersInRange;
		for (const auto& member : members) {
			if (member.mob != nullptr && DistanceSquared(caster->GetPosition(), member.mob->GetPosition()) <= range2) {
				int64 dmgtaken_tmp = member.mob->GetMaxHP() - member.mob->GetHP();
				if (limit > 0 && dmgtaken_tmp > limit)
					dmgtaken_tmp = limit;

				dmgtaken += dmgtaken_tmp;
				membersInRange.push_back(member.mob);
			}
		}

		dmgtaken += dmgtaken * penalty / 100;
		dmgtaken /= static_cast<int64>(membersInRange.size());
		for (const auto member : membersInRange) {
			member->SetHP(std::max(member->GetMaxHP() - dmgtaken, 1L));
			member->SendHPUpdate();
		}
	}
}

void Group::BalanceMana(int32 penalty, float range, const Mob* caster, int32 limit) const
{
	if (caster != nullptr) {
		if (range < std::numeric_limits<float>::epsilon())
			range = 200;

		const float range2 = range * range;
		int64 manataken = 0;

		std::vector<Mob*> membersInRange;
		for (const auto& member : members) {
			if (member.mob != nullptr && member.mob->GetMaxMana() > 0 && DistanceSquared(caster->GetPosition(), member.mob->GetPosition()) <= range2) {
				int64 manataken_tmp = member.mob->GetMaxMana() - member.mob->GetMana();
				if (limit > 0 && manataken_tmp > limit)
					manataken_tmp = limit;

				manataken += manataken_tmp;
				membersInRange.push_back(member.mob);
			}
		}

		manataken += manataken * penalty / 100;
		manataken /= static_cast<int64>(membersInRange.size());

		if (limit > 0 && manataken > limit)
			manataken = limit;

		for (const auto member : membersInRange) {
			member->SetMana(std::max(member->GetMaxMana() - manataken, 1L));
			if (member->IsClient())
				member->CastToClient()->SendManaUpdate();
		}
	}
}

void Group::QueueHPPacketsForNPCHealthAA(const Mob* sender, const EQApplicationPacket* app) const
{
	if (sender != nullptr && app != nullptr && GetLeadershipAA(groupAANPCHealth) > 0) {
		const uint16 SenderID = sender->GetID();
		if (SenderID == AssistTargetID || IsNPCMarked(SenderID))
			ForEachClient([&](Client* c) {
				if (c->GetTarget() == nullptr || c->GetTarget()->GetID() != SenderID)
					c->QueuePacket(app);
			});
	}
}

bool Group::HasMemberNamed(const char* name) const
{
	return std::ranges::any_of(members, [&name](const GroupMember& member) {
		return member.InUse() && strcasecmp(member.name.c_str(), name) == 0;
	});
}

bool Group::IsMemberMerc(const std::string& name) const
{
	for (const auto& member : members)
		if (Strings::EqualFold(member.name, name))
			return member.kind == GroupMemberMerc;

	return false;
}

uint8 Group::GroupCount() const
{
	uint8 MemberCount = 0;
	for (const auto & member : members)
		if(member.InUse())
			++MemberCount;

	return MemberCount;
}

uint32 Group::GetHighestLevel() const
{
	uint32 level = 1;
	for (const auto& member : members)
		if (member.mob != nullptr && member.mob->GetLevel() > level)
				level = member.mob->GetLevel();

	return level;
}

uint32 Group::GetLowestLevel() const
{
	uint32 level = std::numeric_limits<uint32>::max();
	for (const auto& member : members)
		if (member.mob != nullptr && member.mob->GetLevel() < level)
				level = member.mob->GetLevel();

	return level;
}

uint16 Group::GetAvgLevel() const
{
	double levelHolder = 0;
	uint8 numMem = 0;

	for (const auto& member : members) {
		if (member.mob != nullptr) {
			++numMem;
			levelHolder += member.mob->GetLevel();
		}
	}

	return static_cast<uint16>(std::round(levelHolder / numMem));
}

void Group::TeleportGroup(const Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading) const
{
	ForEachClient([&](Client* c) {
		if (c != sender)
			c->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
	});
}

void Group::SetDirtyAutoHaters() const
{
	ForEachMember([](Mob* m) { m->SetDirtyAutoHaters(); });
}

bool Group::AnyMemberHasDzLockout(const std::string& expedition, const std::string& event) const
{
	std::vector<std::string> names;
	for (const auto& member : members) {
		if (member.mob == nullptr && member.InUse()) {
			names.emplace_back(member.name);
		} else if (member.mob != nullptr && member.mob->HasDzLockout(expedition, event)) {
			return true;
		}
	}

	return !CharacterExpeditionLockoutsRepository::GetLockouts(database, names, expedition, event).empty();
}

// ---------------------------------------------------------------------------------------------------------------------
// Passthrough to world
// ---------------------------------------------------------------------------------------------------------------------

// Passthrough to world -- one opcode (ServerOP_GroupAction) + one shared
// header for every zone->world group mutation request, mirroring
// zone/raids.cpp's SendRaidAction/RequestAction exactly.
template <typename T>
static void SendGroupAction(uint32 requestor_character_id, const std::string& requestor_name, T&& payload, uint32 gid, GroupActionType action)
{
	payload.header.action = static_cast<uint8>(action);
	payload.header.zoneid = zone->GetZoneID();
	payload.header.instance_id = zone->GetInstanceID();
	payload.header.gid = gid;
	payload.header.requestor_character_id = requestor_character_id;
	strn0cpy(payload.header.requestor_name, requestor_name.c_str(), sizeof(payload.header.requestor_name));

	auto pack = new ServerPacket(ServerOP_GroupAction, sizeof(T));
	memcpy(pack->pBuffer, &payload, sizeof(T));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Group::RequestDisband() const
{
	std::string requestor_name = GetLeaderName();
	uint32 requestor_character_id = (leader && leader->IsClient()) ? leader->CastToClient()->CharacterID() : 0;

	RequestDisband(requestor_character_id, requestor_name);
}

void Group::RequestDisband(uint32 requestor_character_id, const std::string& requestor_name, const std::string& target_name) const
{
	ServerGroupActionTarget_Struct payload{};
	strn0cpy(payload.target_name, target_name.c_str(), sizeof(payload.target_name));
	SendGroupAction(requestor_character_id, requestor_name, payload, GetID(), GroupActionDisband);
}

void Group::RequestAddMember(const Client* requestor, const Mob* target)
{
	if (requestor != nullptr) {
		ServerGroupActionAddMember_Struct payload{};
		payload.requestor_class = requestor->GetClass();
		payload.requestor_level = requestor->GetLevel();
		requestor->GetGroupAAs(&payload.requestor_group_aa);
		strn0cpy(payload.target_name, target->GetCleanName(), sizeof(payload.target_name));
		payload.target_is_bot = target->IsBot();
		payload.target_is_merc = target->IsMerc();
		payload.target_owner_character_id = target->GetOwnerCharacterID();
		strn0cpy(payload.target_owner_name, requestor->GetName(), sizeof(payload.target_owner_name));
		payload.target_class = target->GetClass();
		payload.target_level = target->GetLevel();
		SendGroupAction(requestor->CharacterID(), requestor->GetName(), payload, 0, GroupActionAddMember);
	}
}

void Group::RequestFollow(const Client* accepter, const char* inviter_name)
{
	if (accepter != nullptr && inviter_name != nullptr && inviter_name[0] != '\0') {
		ServerGroupActionFollow_Struct payload{};

		if (const Client* local_inviter = entity_list.GetClientByName(inviter_name)) {
			strn0cpy(payload.inviter_name, local_inviter->GetCleanName(), sizeof(payload.inviter_name));
			payload.inviter_class = local_inviter->GetClass();
			payload.inviter_level = local_inviter->GetLevel();
		} else {
			strn0cpy(payload.inviter_name, inviter_name, sizeof(payload.inviter_name));
			payload.inviter_class = 0;
			payload.inviter_level = 0;
		}

		payload.accepter_class = accepter->GetClass();
		payload.accepter_level = accepter->GetLevel();
		accepter->GetGroupAAs(&payload.accepter_group_aa);
		SendGroupAction(accepter->CharacterID(), accepter->GetName(), payload, 0, GroupActionFollow);
	}
}

void Group::RequestRoleChange(uint32 requestor_character_id, const std::string& requestor_name, uint8 action, const std::string& target_name) const
{
	ServerGroupActionTarget_Struct payload{};
	strn0cpy(payload.target_name, target_name.c_str(), sizeof(payload.target_name));
	SendGroupAction(requestor_character_id, requestor_name, payload, GetID(), static_cast<GroupActionType>(action));
}

void Group::RequestMakeLeader(uint32 requestor_character_id, const std::string& requestor_name, const std::string& target_name) const
{
	ServerGroupActionTarget_Struct payload{};
	strn0cpy(payload.target_name, target_name.c_str(), sizeof(payload.target_name));
	SendGroupAction(requestor_character_id, requestor_name, payload, GetID(), GroupActionMakeLeader);
}

void Group::RequestMakeLeader(const Mob* target) const
{
	if (target != nullptr && leader != nullptr && leader->IsClient())
		RequestMakeLeader(leader->CastToClient()->CharacterID(), leader->CastToClient()->GetName(), target->GetCleanName());
}

void Group::RequestMentor(uint32 requestor_character_id, bool clear, const std::string& name, int percent) const
{
	ServerGroupActionMentor_Struct payload{};
	payload.clear = clear;
	strn0cpy(payload.mentoree_name, name.c_str(), sizeof(payload.mentoree_name));
	payload.mentor_percent = percent;
	SendGroupAction(requestor_character_id, "", payload, GetID(), GroupActionMentor);
}

void Group::RequestInvite(const GroupInvite_Struct* invite)
{
	auto pack = new ServerPacket(ServerOP_GroupInvite, sizeof(GroupInvite_Struct));
	memcpy(pack->pBuffer, invite, sizeof(GroupInvite_Struct));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Group::RequestCancelInvite(const GroupCancel_Struct* cancel)
{
	auto pack = new ServerPacket(ServerOP_GroupCancelInvite, sizeof(GroupCancel_Struct));
	memcpy(pack->pBuffer, cancel, sizeof(GroupCancel_Struct));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Group::RequestRefresh() const
{
	ServerGroupActionBasic_Struct payload{};
	SendGroupAction(0, "", payload, GetID(), GroupActionRefresh);
}

void Group::RequestGroupAAs() const
{
	uint32 requestor_character_id = 0;
	std::string requestor_name;

	const Mob* m = GetLeader();
	if (m != nullptr && m->IsClient()) {
		requestor_character_id = m->CastToClient()->CharacterID();
		requestor_name = m->GetCleanName();
	}

	ServerGroupActionBasic_Struct payload{};
	SendGroupAction(requestor_character_id, requestor_name, payload, GetID(), GroupActionUpdateAA);
}

// TODO: This will get Group _and_ Raid responses
void Group::RequestPartyState(const std::string& character_name)
{
	auto pack = new ServerPacket(ServerOP_PartyStateRequest, sizeof(ServerPartyStateRequest_Struct));
	const auto psr = reinterpret_cast<ServerPartyStateRequest_Struct*>(pack->pBuffer);
	psr->zoneid = zone->GetZoneID();
	psr->instance_id = zone->GetInstanceID();
	strn0cpy(psr->character_name, character_name.c_str(), sizeof(psr->character_name));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

// ---------------------------------------------------------------------------------------------------------------------
// Apply State From World
// ---------------------------------------------------------------------------------------------------------------------

void Group::AttachZonedMember(const std::string& name)
{
	Mob* m = entity_list.GetMob(name.c_str());
	if (m != nullptr) {
		UpdatePlayer(m);

		if (m->IsClient())
			SendFullGroupUpdate(m->CastToClient());

		SendHPPacketsFrom(m);
	}
}

void Group::ApplyFullFromWorld(const ServerGroupState_Struct &gs)
{
	std::vector<std::string> previous_names;
	for (auto& member : members) {
		if (member.InUse())
			previous_names.push_back(member.name);

		member = GroupMember{};
	}

	std::vector<std::string> incoming_names;

	int slot_id = 0;
	for (const auto& member : gs.members) {
		incoming_names.emplace_back(member.name);

		members[slot_id].name               = member.name;
		members[slot_id].class_             = member.class_;
		members[slot_id].level              = member.level;
		members[slot_id].kind               = member.kind;
		members[slot_id].owner_character_id = member.owner_character_id;
		members[slot_id].owner_name         = member.owner_name;
		members[slot_id].offline            = member.is_offline;
		members[slot_id].roles              = member.roles;
		members[slot_id].mob                = entity_list.GetMob(member.name);

		if (members[slot_id].mob != nullptr) {
			const bool was_present = std::ranges::any_of(previous_names, [&member](const std::string& n) {
				return Strings::EqualFold(n, member.name);
			});

			if (was_present)
				entity_list.SetGroupZoned(members[slot_id].mob, this);
			else
				entity_list.SetGroup(members[slot_id].mob, this);
		}

		++slot_id;
	}

	// Anyone who was here before but isn't in the new roster at all
	// genuinely left the group.
	for (const auto& name : previous_names) {
		const bool still_present = std::ranges::any_of(incoming_names, [&name](const std::string& n) {
			return Strings::EqualFold(n, name);
		});

		if (!still_present) {
			if (Mob* m = entity_list.GetMob(name.c_str()))
				entity_list.SetGroup(m, nullptr);
		}
	}

	++roster_revision;

	SetGroupLeaderRole(gs.leader_name);
	SetMainTank(gs.main_tank_name);
	SetMainAssist(gs.main_assist_name);
	SetPuller(gs.puller_name);
	SetNPCMarker(gs.npc_marker_name);
	SetMasterLooter(gs.master_looter_name);
	SetLeader(entity_list.GetClientByName(leader_name.c_str()));
	leadership_aa = gs.leadership_aa;

	mentoree_name = gs.mentoree_name;
	mentor_percent = gs.mentor_percent;
	mentoree = entity_list.GetClientByName(mentoree_name.c_str());

	ForEachClient([this](Client* c) { SendFullGroupUpdate(c); });
}

void Group::ApplyMemberJoinedFromWorld(const ServerGroupStateMember_Struct &member)
{
	Mob* m = AddMember(member);
	SendJoinPacket(m, member);
	SendJoinInfo(m);
}

void Group::ApplyMemberLeftFromWorld(const std::string &name)
{
	SendLeavePacket(name);
	RemoveMember(name);
}

void Group::ApplyDisbandFromWorld()
{
	SendDisbandGroup();

	Client *Leader = nullptr;
	for (auto& member : members) {
		if (member.mob != nullptr && member.mob->IsClient() && IsLeader(member.mob))
				Leader = member.mob->CastToClient();

		ClearMember(member);
	}

	ClearAllNPCMarks();

	if(Leader && (Leader->IsLFP()))
		Leader->UpdateLFP();

	SetLeader(nullptr);
	entity_list.RemoveGroup(GetID()); // deletes this Group
}

void Group::ApplyLeaderChangedFromWorld(const std::string &new_leader_name)
{
	leader_name = new_leader_name;
	SetLeader(entity_list.GetClientByName(leader_name.c_str()));
	SetGroupLeaderRole(leader_name);
	SendLeadershipPacket(new_leader_name);
}

void Group::ApplyRoleChangedFromWorld(uint8 role_id, const std::string &target_name)
{
	const bool clearing = target_name.empty();

	switch (role_id) {
		case GroupRoleMainTank:
			if (clearing) ForEachClient([this](Client* c) { NotifyMainTank(c, false); });
			SetMainTank(target_name.c_str());
			if (!clearing) ForEachClient([this](Client* c) { NotifyMainTank(c, true); });
			break;
		case GroupRoleMainAssist:
			if (clearing) ForEachClient([this](Client* c) { NotifyMainAssist(c, false); });
			SetMainAssist(target_name.c_str());
			if (!clearing) ForEachClient([this](Client* c) { NotifyMainAssist(c, true); });
			break;
		case GroupRolePuller:
			if (clearing) ForEachClient([this](Client* c) { NotifyPuller(c, false); });
			SetPuller(target_name.c_str());
			if (!clearing) ForEachClient([this](Client* c) { NotifyPuller(c, true); });
			break;
		case GroupRoleMarkNPC:
			if (clearing) ForEachClient([this](Client* c) { NotifyMarkNPC(c, false); });
			SetNPCMarker(target_name.c_str());
			if (!clearing) ForEachClient([this](Client* c) { NotifyMarkNPC(c, true); });
			break;
		case GroupRoleMasterLooter:
			if (clearing) ForEachClient([this](Client* c) { NotifyMasterLooter(c, false); });
			SetMasterLooter(target_name.c_str());
			if (!clearing) ForEachClient([this](Client* c) { NotifyMasterLooter(c, true); });
			break;
		default:
			break;
	}
}

void Group::ApplyOfflineChangedFromWorld(const std::string &name, bool is_offline)
{
	for (auto& member : members) {
		if (member.InUse() && Strings::EqualFold(member.name, name)) {
			member.offline = is_offline;
			break;
		}
	}

	// Cannot find a delta packet for this update, have to send a full
	ForEachClient([this](Client* c) { SendFullGroupUpdate(c); });
}

void Group::ApplyMentorChangedFromWorld(const std::string &mentoree_name_in, int32 mentor_percent_in)
{
	mentoree_name  = mentoree_name_in;
	mentor_percent = mentor_percent_in;
	mentoree       = entity_list.GetClientByName(mentoree_name.c_str());
}

void Group::ApplyAAChangedFromWorld(const GroupLeadershipAA_Struct &new_leader_aa)
{
	leadership_aa = new_leader_aa;
	SendLeadershipAAUpdate();
}

void Group::QueueClientsInRange(
	const Mob* sender, const EQApplicationPacket* app, bool ack_required /*= true*/,
	bool ignore_sender /*= true*/, float distance /*= 0*/) const
{
	if (sender != nullptr) {
		ForEachClient([&](Client* c) {
			if (!ignore_sender || c != sender) {
				/* If we don't have a distance requirement - send to all members */
				if (distance ==  0) {
					c->QueuePacket(app, ack_required);
				} else {
					float maxDistance2 = distance * distance;
					float distance2 = DistanceSquared(sender->GetPosition(), c->GetPosition());
					/* If negative distance - we check if current distance is greater than X */
					/* If positive distance - we check if current distance is less than X */
					if ((distance <= 0 && distance2 >= maxDistance2) || (distance >= 0 && distance2 <= maxDistance2))
						c->QueuePacket(app, ack_required);
				}
			}
		});
	}
}

void Group::QueueClients(const EQApplicationPacket* app, bool ack_required) const
{
	ForEachClient([&](Client* c) { c->QueuePacket(app, ack_required); });
}

// TODO: I think this (and the next) needs to not send things in zone and needs to consolidate to a single world message with an apply back
void Group::GroupMessage(const Mob* sender, uint8 language, uint8 lang_skill, const char* message) const
{
	ForEachClient([&](Client* c) {
		if (c->GetFilter(FilterGroupChat) != 0)
			c->ChannelMessageSend(sender->GetName(), c->GetName(), ChatChannel_Group, language, lang_skill, message);
	});

	auto pack = new ServerPacket(ServerOP_OOZGroupMessage,
		sizeof(ServerGroupChannelMessage_Struct) + strlen(message) + 1);

	const auto gcm = reinterpret_cast<ServerGroupChannelMessage_Struct*>(pack->pBuffer);
	gcm->zoneid = zone->GetZoneID();
	gcm->groupid = GetID();
	gcm->instanceid = zone->GetInstanceID();
	strcpy(gcm->from, sender->GetCleanName());
	strcpy(gcm->message, message);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Group::GroupMessageString(const Mob* sender, uint32 type, uint32 string_id, const char* message,
	const char* message2, const char* message3, const char* message4, const char* message5, const char* message6,
	const char* message7, const char* message8, const char* message9) const
{
	ForEachClient([&](Client* c) {
		if (c != sender)
			c->MessageString(type, string_id, message, message2, message3, message4, message5, message6, message7,
				message8, message9, 0);
	});
}

// ---------------------------------------------------------------------------------------------------------------------
// Group member logistics
// ---------------------------------------------------------------------------------------------------------------------

Mob* Group::GetMember(const std::string& name) const
{
	for (const auto& member : members)
		if (Strings::EqualFold(member.name, name))
			return member.mob;

	return nullptr;
}

bool Group::IsLeader(const char* name) const
{
	return name != nullptr && !strcasecmp(leader_name.c_str(), name);
}

std::string Group::GetLeaderName() const
{
	return leader_name;
}

void Group::SendJoinInfo(Mob* new_member) const
{
	if (new_member != nullptr) {
		if (new_member->IsClient()) {
			Client *nc = new_member->CastToClient();
			nc->Save();

			if (nc->ClientVersion() >= EQ::versions::ClientVersion::SoD)
				SendJoinAcknowledge(nc);

			SendFullGroupUpdate(nc);
		}

		SendHPPacketsFrom(new_member);
	}
}

Mob* Group::AddMember(const ServerGroupStateMember_Struct& new_member_def)
{
	Mob* new_member = entity_list.GetMob(new_member_def.name);
	if (GroupCount() < MAX_GROUP_MEMBERS && !IsGroupMember(new_member_def.name)) {
		for (auto& member : members) {
			if (!member.InUse()) {
				member.mob = new_member;
				member.name = new_member_def.name;
				member.roles = 0;
				member.offline = new_member_def.is_offline;
				member.kind = new_member_def.kind;
				member.level = new_member_def.level;
				member.class_ = new_member_def.class_;
				member.owner_character_id = new_member_def.owner_character_id;
				member.owner_name = new_member_def.owner_name;
				break;
			}
		}

		entity_list.SetGroup(new_member, this);

		++roster_revision; // TODO: This is stupid, what is the point of this

		return new_member;
	}

	return nullptr;
}

void Group::RemoveMember(const std::string& name)
{
	Mob* departing = GetMember(name);
	for (auto& member : members)
		if ((departing != nullptr && member.mob == departing) || Strings::EqualFold(member.name, name))
			ClearMember(member);
}

void Group::ClearMember(GroupMember& slot)
{
	entity_list.SetGroup(slot.mob, nullptr);
	slot = GroupMember{};
}

void Group::UpdatePlayer(Mob* update)
{
	if (update != nullptr) {
		for (auto& member : members) {
			if (member.InUse() && Strings::EqualFold(member.name, update->GetCleanName()) && member.mob != update) {
				member.mob = update;
				entity_list.SetGroupZoned(update, this);
				break;
			}
		}

		if (update->IsClient()) {
			if (Strings::EqualFold(update->GetCleanName(), npc_marker_name))
				NPCMarkerID = update->GetID();

			if (mentoree == nullptr && !mentoree_name.empty() && Strings::EqualFold(update->GetCleanName(), mentoree_name))
				mentoree = update->CastToClient();
		}

		++roster_revision;
	}
}

void Group::MemberZoned(const Mob* removemob) {
	if (removemob != nullptr) {
		if (removemob == GetLeader())
			SetLeader(nullptr);

		for (auto& member : members) {
			if (member.mob != nullptr && (member.mob->GetOwner() == removemob || member.mob == removemob)) {
				entity_list.SetGroupZoned(member.mob, nullptr);
				member.mob = nullptr;
			}
		}

		UpdateMemberTarget(removemob, nullptr);

		if (removemob->GetID() == NPCMarkerID)
			NPCMarkerID = 0;

		if (removemob->IsClient() && removemob == mentoree)
			mentoree = nullptr;

		++roster_revision;
	}
}

bool Group::IsGroupMember(const Mob* c) const
{
	if (c)
		for (const auto &member: members)
			if (member.mob == c)
				return true;

	return false;
}

bool Group::IsGroupMember(const char *name)
{
	return std::ranges::any_of(members, [&name](const GroupMember& member) { return member.InUse() && member.name == name; });
}

void Group::SendDisbandGroup() const
{
	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
	const auto gu = reinterpret_cast<GroupUpdate_Struct*>(outapp->pBuffer);
	gu->action = groupActDisband;

	ForEachClient([&outapp, &gu](Client* c) {
		strn0cpy(gu->yourname, c->GetCleanName(), sizeof(gu->yourname));
		c->QueuePacket(outapp);
	});

	safe_delete(outapp);
}

void Group::SendFullGroupUpdate(Client* client) const
{
	if (client != nullptr) {
		client->UpdateGroupMembers(this);

		if (client->ClientVersion() >= EQ::versions::ClientVersion::RoF2) {
			// TODO: This is probably true for SoD+, but previous clients would need testing
			std::vector<const GroupMember*> entries;
			entries.reserve(MAX_GROUP_MEMBERS);
			for (const auto& member : members) {
				if (member.InUse() && !Strings::EqualFold(member.name, client->GetCleanName()))
					entries.push_back(&member);
				else if (member.InUse())
					entries.insert(entries.begin(), &member); // client must always be the first entry
			}

			auto outapp = new EQApplicationPacket(OP_GroupUpdateB, sizeof(GroupMembersUpdate_Struct));
			const auto gu = reinterpret_cast<GroupMembersUpdate_Struct*>(outapp->pBuffer);
			strn0cpy(gu->leader_name, GetLeaderName().c_str(), sizeof(gu->leader_name));
			gu->member_count = std::min<uint32>(static_cast<uint32>(entries.size()), std::size(gu->members));
			for (uint32 i = 0; i < gu->member_count; ++i) {
				const GroupMember* entry = entries[i];
				auto& out = gu->members[i];
				strn0cpy(out.name, entry->name.c_str(), sizeof(out.name));
				strn0cpy(out.owner_name, entry->owner_name.c_str(), sizeof(out.owner_name));
				out.type = entry->kind;
				out.level = entry->level;
				out.is_tank = (entry->roles & GroupBitmaskMainTank) != 0 ? 1 : 0;
				out.is_assist = (entry->roles & GroupBitmaskMainAssist) != 0 ? 1 : 0;
				out.is_puller = (entry->roles & GroupBitmaskPuller) != 0 ? 1 : 0;
				out.is_mark = (entry->roles & GroupBitmaskMarkNPC) != 0 ? 1 : 0;
				out.is_master_looter = (entry->roles & GroupBitmaskMasterLooter) != 0 ? 1 : 0;
				out.offline = entry->offline ? 1 : 0;
			}

			client->QueuePacket(outapp);
			safe_delete(outapp);
		} else {
			auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
			const auto gu = reinterpret_cast<GroupUpdate2_Struct*>(outapp->pBuffer);
			gu->action = groupActUpdate;
			strn0cpy(gu->yourname, client->GetCleanName(), sizeof(gu->yourname));
			strn0cpy(gu->leadersname, GetLeaderName().c_str(), sizeof(gu->leadersname));
			GetGroupAAs(&gu->leader_aas);
			gu->NPCMarkerID = GetNPCMarkerID();

			int slot_id = 0;
			for (const auto& member : members) {
				if (member.InUse()
					&& !Strings::EqualFold(member.name, client->GetCleanName())
					&& slot_id < static_cast<int>(std::size(gu->membername))) {
					strn0cpy(gu->membername[slot_id], member.name.c_str(), sizeof(gu->membername[slot_id]));
					++slot_id;
				}
			}

			client->QueuePacket(outapp);

			safe_delete(outapp);

			NotifyMainTank(client, true);
			NotifyMainAssist(client, true);
			NotifyPuller(client, true);
			NotifyMarkNPC(client, true);
			NotifyMasterLooter(client, true);
		}

		NotifyTankTarget(client);
		NotifyAssistTarget(client);
		NotifyPullerTarget(client);
		SendMarkedNPCsToMember(client);
		SendHPManaEndPacketsTo(client);
	}
}

void Group::SendJoinPacket(const Mob* new_member, const ServerGroupStateMember_Struct& new_member_def) const
{
	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	const auto gj = reinterpret_cast<GroupJoin_Struct*>(outapp->pBuffer);
	strcpy(gj->membername, new_member_def.name);
	gj->action     = groupActJoin;
	gj->leader_aas = leadership_aa;

	ForEachClient([&](Client* c) {
		if (c != new_member) {
			strn0cpy(gj->yourname, c->GetCleanName(), sizeof(gj->yourname));
			c->QueuePacket(outapp);
		}

		c->UpdateGroupMembers(this);
	});

	safe_delete(outapp);
}

void Group::SendLeavePacket(const std::string& name) const
{
	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	const auto gu = reinterpret_cast<GroupJoin_Struct*>(outapp->pBuffer);
	gu->action = groupActLeave;
	strn0cpy(gu->membername, name.c_str(), sizeof(gu->membername));
	gu->leader_aas = leadership_aa;

	ForEachClient([&](Client* c) {
		strn0cpy(gu->yourname, c->GetCleanName(), sizeof(gu->yourname));
		c->QueuePacket(outapp);
	});

	safe_delete(outapp);
}

void Group::SendGroupRolePacket(Client *c, const char *name, uint8 role_number, bool toggle) const
{
	auto outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));
	const auto grs = reinterpret_cast<GroupRole_Struct*>(outapp->pBuffer);

	strn0cpy(grs->Name1, name, sizeof(grs->Name1));
	strn0cpy(grs->Name2, GetLeaderName().c_str(), sizeof(grs->Name2));
	grs->RoleNumber = role_number;
	grs->Toggle = toggle ? 1 : 0;

	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Group::SendLeadershipPacket(const std::string& leader_name) const
{
	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	const auto gu = reinterpret_cast<GroupJoin_Struct*>(outapp->pBuffer);
	gu->action = groupActMakeLeader;
	strn0cpy(gu->membername, leader_name.c_str(), sizeof(gu->membername));
	gu->leader_aas = leadership_aa;

	ForEachClient([&](Client* c) {
		strn0cpy(gu->yourname, c->GetCleanName(), sizeof(gu->yourname));
		if (c->ClientVersion() >= EQ::versions::ClientVersion::SoD)
			SendLeaderChangePacket(c, leader_name);

		c->QueuePacket(outapp);
	});

	safe_delete(outapp);
}

void Group::SendLeadershipAAUpdate() const
{
	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	const auto gu = reinterpret_cast<GroupJoin_Struct*>(outapp->pBuffer);
	gu->action = groupActAAUpdate;
	gu->leader_aas = leadership_aa;
	gu->NPCMarkerID = GetNPCMarkerID();

	ForEachClient([&](Client* c) {
		strn0cpy(gu->yourname, c->GetName(), sizeof(gu->yourname));
		strn0cpy(gu->membername, c->GetName(), sizeof(gu->membername));
		c->QueuePacket(outapp);
	});

	safe_delete(outapp);
}

void Group::SendInvite(Client* recipient, const GroupInvite_Struct* invite)
{
	if (recipient != nullptr && invite != nullptr) {
		auto outapp = new EQApplicationPacket(OP_GroupInvite, sizeof(GroupInvite_Struct));
		memcpy(outapp->pBuffer, invite, sizeof(GroupInvite_Struct));
		recipient->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Group::SendCancelInvite(Client* recipient, const GroupCancel_Struct* cancel)
{
	if (recipient != nullptr && cancel != nullptr) {
		auto outapp = new EQApplicationPacket(OP_GroupCancelInvite, sizeof(GroupCancel_Struct));
		memcpy(outapp->pBuffer, cancel, sizeof(GroupCancel_Struct));
		recipient->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Group::SendLeaderChangePacket(Client* c, const std::string& leader_name)
{
	if (c != nullptr) {
		auto outapp = new EQApplicationPacket(OP_GroupLeaderChange, sizeof(GroupLeaderChange_Struct));
		const auto glcs = reinterpret_cast<GroupLeaderChange_Struct*>(outapp->pBuffer);
		strn0cpy(glcs->LeaderName, leader_name.c_str(), sizeof(glcs->LeaderName));
		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Group::SendJoinAcknowledge(Client* c)
{
	if (c != nullptr) {
		auto outapp = new EQApplicationPacket(OP_GroupAcknowledge, 4);
		c->QueuePacket(outapp);
		safe_delete(outapp);
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Group Role Functionality
// ---------------------------------------------------------------------------------------------------------------------

bool Group::IsNPCMarked(uint32 EntityID) const
{
	return std::ranges::any_of(MarkedNPCs, [EntityID](const auto& npc) { return npc == EntityID; });
}

// TODO: no permission check here -- any group member can mark, regardless of
// GroupRoleMarkNPC/NPCMarkerName delegation. Same class of bug fixed on
// Raid::MarkNPC() (which now requires IsLeader() || IsMarker()); needs the
// analogous fix here plus a covering test (see group_test/tests.lua's own
// TODO on the "NPC mark TARGET" block).
void Group::MarkNPC(const Mob* Target, uint32 Number)
{
	if (Target != nullptr && Number > 0 && Number <= MAX_MARKED_NPCS) {
		const uint16 EntityID = Target->GetID();
		if (EntityID > 0 && MarkedNPCs[Number - 1] != EntityID) {
			for (int i = 0; i < MAX_MARKED_NPCS; ++i) {
				if (MarkedNPCs[i] == EntityID || (i == Number - 1 && MarkedNPCs[i] > 0)) {
					if (Mob* m = entity_list.GetMob(MarkedNPCs[i]))
						m->IsTargeted(-1);

					UpdateXTargetMarkedNPC(i + 1, nullptr);
					MarkedNPCs[i] = 0;
				}
			}

			MarkedNPCs[Number - 1] = EntityID;
			if (Mob* m = entity_list.GetMob(Target->GetID()))
				m->IsTargeted(1);

			auto outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));
			const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
			mnpcs->TargetID = EntityID;
			mnpcs->Number = Number;

			const Mob* m = entity_list.GetMob(EntityID);
			if (m != nullptr)
				strn0cpy(mnpcs->Name, m->GetCleanName(), sizeof(mnpcs->Name));

			QueueClients(outapp);
			safe_delete(outapp);

			UpdateXTargetMarkedNPC(Number, m);
		}
	}
}

void Group::UnMarkNPC(uint16 ID)
{
	if (AssistTargetID == ID)
		AssistTargetID = 0;

	if (TankTargetID == ID)
		TankTargetID = 0;

	if (PullerTargetID == ID)
		PullerTargetID = 0;

	for (int i = 0; i < MAX_MARKED_NPCS; ++i) {
		if (MarkedNPCs[i] == ID) {
			MarkedNPCs[i] = 0;
			UpdateXTargetMarkedNPC(i + 1, nullptr);
		}
	}
}

void Group::UpdateXTargetMarkedNPC(uint32 Number, const Mob* m) const
{
	ForEachClient([&](Client* c) {
		c->UpdateXTargetType(
			Number == 1 ? GroupMarkTarget1 :
			Number == 2 ? GroupMarkTarget2 :
			GroupMarkTarget3, m);
	});
}

void Group::SendMarkedNPCsToMember(Client* c, bool Clear) const
{
	for (int mark = 1; mark <= MAX_MARKED_NPCS; ++mark)
		NotifyMarkedNPC(c, mark, Clear);
}

void Group::NotifyMainTank(Client* c, bool toggle) const
{
	if (c != nullptr && !main_tank_name.empty() && c->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
		SendGroupRolePacket(c, main_tank_name.c_str(), GroupRoleMainTank, toggle);
		NotifyTankTarget(c);
	}
}

void Group::NotifyMainAssist(Client* c, uint8 toggle) const
{
	if (c!= nullptr && !main_assist_name.empty()) {
		if (c->ClientVersion() < EQ::versions::ClientVersion::SoD) {
			auto outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));
			const auto das = reinterpret_cast<DelegateAbility_Struct*>(outapp->pBuffer);
			das->DelegateAbility = 0;
			das->MemberNumber = 0;
			das->Action = 0;
			das->EntityID = 0;
			strn0cpy(das->Name, main_assist_name.c_str(), sizeof(das->Name));
			c->QueuePacket(outapp);
			safe_delete(outapp);
		} else {
			SendGroupRolePacket(c, main_assist_name.c_str(), GroupRoleMainAssist, toggle);
		}

		NotifyAssistTarget(c);
	}
}

void Group::NotifyPuller(Client* c, uint8 toggle) const
{
	if (c != nullptr && !puller_name.empty() && c->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
		SendGroupRolePacket(c, puller_name.c_str(), GroupRolePuller, toggle);
		NotifyPullerTarget(c);
	}
}

void Group::NotifyMarkNPC(Client* c, bool toggle) const
{
	if (c != nullptr && !npc_marker_name.empty()) {
		if (c->ClientVersion() < EQ::versions::ClientVersion::TOB) {
			auto outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));
			const auto das = reinterpret_cast<DelegateAbility_Struct*>(outapp->pBuffer);

			das->DelegateAbility = 1;
			das->MemberNumber = 0;
			das->Action = 0;
			das->EntityID = GetNPCMarkerID();
			strn0cpy(das->Name, npc_marker_name.c_str(), sizeof(das->Name));

			c->QueuePacket(outapp);
			safe_delete(outapp);
		} else {
			SendGroupRolePacket(c, npc_marker_name.c_str(), GroupRoleMarkNPC, toggle);
		}
	}
}

void Group::NotifyMasterLooter(Client *c, bool toggle) const
{
	if (c != nullptr && !master_looter_name.empty() && c->ClientVersion() >= EQ::versions::ClientVersion::TOB)
		SendGroupRolePacket(c, master_looter_name.c_str(), GroupRoleMasterLooter, toggle);
}

void Group::UpdateMemberTarget(const Mob* member, const Mob* target)
{
	if (member != nullptr && member->IsClient()) {
		if (HasRole(member, GroupBitmaskMainAssist))
			SetGroupAssistTarget(target);

		if (HasRole(member, GroupBitmaskMainTank))
			SetGroupTankTarget(target);

		if (HasRole(member, GroupBitmaskPuller))
			SetGroupPullerTarget(target);
	}
}

void Group::SetGroupAssistTarget(const Mob* m)
{
	AssistTargetID = m != nullptr ? m->GetID() : 0;
	ForEachClient([this](Client* c) { NotifyAssistTarget(c); });
}

void Group::SetGroupTankTarget(const Mob* m)
{
	TankTargetID = m != nullptr ? m->GetID() : 0;
	ForEachClient([this](Client* c) { NotifyTankTarget(c); });
}

void Group::SetGroupPullerTarget(const Mob* m)
{
	PullerTargetID = m != nullptr ? m->GetID() : 0;
	ForEachClient([this](Client* c) { NotifyPullerTarget(c); });
}

void Group::NotifyAssistTarget(Client *c) const
{
	if (c != nullptr) {
		auto outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));
		const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
		mnpcs->TargetID = AssistTargetID;
		mnpcs->Number = 0;
		c->QueuePacket(outapp);
		safe_delete(outapp);

		c->UpdateXTargetType(GroupAssistTarget, entity_list.GetMob(AssistTargetID));
	}

}

void Group::NotifyTankTarget(Client *c) const
{
	if (c != nullptr)
		c->UpdateXTargetType(GroupTankTarget, entity_list.GetMob(TankTargetID));
}

void Group::NotifyPullerTarget(Client *c) const
{
	if (c != nullptr)
		c->UpdateXTargetType(PullerTarget, entity_list.GetMob(PullerTargetID));
}

void Group::NotifyMarkedNPC(Client* c, uint16 mark, bool clear) const
{
	uint16 mark_idx = mark - 1;
	if (c != nullptr && mark > 0 && mark <= MAX_MARKED_NPCS && MarkedNPCs[mark_idx] > 0) {
		auto outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));
		const auto mnpcs = reinterpret_cast<MarkNPC_Struct*>(outapp->pBuffer);
		mnpcs->TargetID = MarkedNPCs[mark_idx];

		const Mob* m = entity_list.GetMob(MarkedNPCs[mark_idx]);
		if (m != nullptr)
			sprintf(mnpcs->Name, "%s", m->GetCleanName());

		if (!clear)
			mnpcs->Number = mark;
		else
			mnpcs->Number = 0;

		c->QueuePacket(outapp);
		c->UpdateXTargetType(mark == 1 ? GroupMarkTarget1 : mark == 2 ? GroupMarkTarget2 : GroupMarkTarget3, m);

		safe_delete(outapp);
	}
}

void Group::ClearAllNPCMarks()
{
	ForEachClient([this](Client* c) { SendMarkedNPCsToMember(c, true); });

	for (unsigned short MarkedNPC : MarkedNPCs) {
		if (MarkedNPC > 0)
			if (Mob* m = entity_list.GetMob(MarkedNPC))
				m->IsTargeted(-1);

		MarkedNPC = 0;
	}
}

void Group::SetGroupLeaderRole(const std::string &new_leader_name)
{
	leader_name = new_leader_name;

	for (auto& member : members) {
		if (Strings::EqualFold(member.name, new_leader_name))
			member.roles |= GroupBitmaskLeader;
		else
			member.roles &= ~GroupBitmaskLeader;
	}
}

void Group::SetMainTank(const char *NewMainTankName)
{
	main_tank_name = NewMainTankName;

	for (auto& member : members) {
		if (Strings::EqualFold(member.name, NewMainTankName))
			member.roles |= GroupBitmaskMainTank;
		else
			member.roles &= ~GroupBitmaskMainTank;
	}
}

void Group::SetMainAssist(const char* NewMainAssistName)
{
	main_assist_name = NewMainAssistName;

	for (auto& member : members) {
		if (Strings::EqualFold(member.name, NewMainAssistName))
			member.roles |= GroupBitmaskMainAssist;
		else
			member.roles &= ~GroupBitmaskMainAssist;
	}
}

void Group::SetPuller(const char* NewPullerName)
{
	puller_name = NewPullerName;

	for (auto& member : members) {
		if (Strings::EqualFold(member.name, NewPullerName))
			member.roles |= GroupBitmaskPuller;
		else
			member.roles &= ~GroupBitmaskPuller;
	}
}

void Group::SetNPCMarker(const char* NewNPCMarkerName)
{
	npc_marker_name = NewNPCMarkerName;

	if (const Client* m = entity_list.GetClientByName(npc_marker_name.c_str()))
		NPCMarkerID = m->GetID();
	else
		NPCMarkerID = 0;

	for (auto& member : members) {
		if (Strings::EqualFold(member.name, NewNPCMarkerName))
			member.roles |= GroupBitmaskMarkNPC;
		else
			member.roles &= ~GroupBitmaskMarkNPC;
	}
}

void Group::SetMasterLooter(const char *NewMasterLooterName)
{
	master_looter_name = NewMasterLooterName;

	for (auto& member : members) {
		if (Strings::EqualFold(member.name, NewMasterLooterName))
			member.roles |= GroupBitmaskMasterLooter;
		else
			member.roles &= ~GroupBitmaskMasterLooter;
	}
}

bool Group::AmIMainTank(const char *mob_name) const
{
	return mob_name != nullptr && Strings::EqualFold(mob_name, main_tank_name);
}

bool Group::AmIMainAssist(const char *mob_name) const
{
	return mob_name != nullptr && Strings::EqualFold(mob_name, main_assist_name);
}

bool Group::AmIPuller(const char *mob_name) const
{
	return mob_name != nullptr && Strings::EqualFold(mob_name, puller_name);
}

bool Group::HasRole(const Mob* m, uint8 Role) const
{
	if (m != nullptr)
		for (const auto& member: members)
			if (m == member.mob && member.roles & Role)
				return true;

	return false;
}

uint8 Group::GetMemberRole(const Mob* m) const
{
	if (m != nullptr)
		for (const auto& member: members)
			if (m == member.mob)
				return member.roles;

	return 0;
}

uint8 Group::GetMemberRole(const char* name) const
{
	if (name != nullptr) {
		for (const auto& member : members) {
			if (Strings::EqualFold(member.name, name))
				return member.roles;
		}
	}

	return 0;
}

// Ordinary (non-template) definition -- lives here because this is where
// Group is actually complete, unlike mob.h. Since GroupMembership isn't a
// template, this is the one and only definition needed regardless of how
// many concrete Mob-derived classes mix HasGroup<M> in -- no explicit
// instantiation list to keep in sync.
void GroupMembership::SetGroup(Group* group)
{
	m_group = group;
	m_group_id = group != nullptr ? group->GetID() : 0;
}

