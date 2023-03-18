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

#include "../common/strings.h"
#include "../common/events/player_event_logs.h"

#include "client.h"
#include "entity.h"
#include "expedition.h"
#include "groups.h"
#include "mob.h"
#include "raids.h"
#include "string_ids.h"
#include "bot.h"

#include "worldserver.h"

extern EntityList entity_list;
extern WorldServer worldserver;

Raid::Raid(uint32 raidID)
: GroupIDConsumer(raidID)
{
	memset(members ,0, (sizeof(RaidMember)*MAX_RAID_MEMBERS));
	memset(&raid_aa, 0, sizeof(RaidLeadershipAA_Struct));
	memset(group_aa, 0, sizeof(GroupLeadershipAA_Struct) * MAX_RAID_GROUPS);
	for (auto& gm : group_mentor) {
		gm.mentor_percent = 0;
		gm.mentoree = nullptr;
	}
	leader = nullptr;
	memset(leadername, 0, 64);
	locked = false;
	LootType = 4;

	m_autohatermgr.SetOwner(nullptr, nullptr, this);
}

Raid::Raid(Client* nLeader)
: GroupIDConsumer()
{
	memset(members ,0, (sizeof(RaidMember)*MAX_RAID_MEMBERS));
	memset(&raid_aa, 0, sizeof(RaidLeadershipAA_Struct));
	memset(group_aa, 0, sizeof(GroupLeadershipAA_Struct) * MAX_RAID_GROUPS);
	for (auto& gm : group_mentor) {
		gm.mentor_percent = 0;
		gm.mentoree = nullptr;
	}
	leader = nLeader;
	memset(leadername, 0, 64);
	strn0cpy(leadername, nLeader->GetName(), 64);
	locked = false;
	LootType = 4;

	m_autohatermgr.SetOwner(nullptr, nullptr, this);
}

Raid::~Raid()
{
}

bool Raid::Process()
{
	if (forceDisband)
		return false;
	if (disbandCheck)
	{
		int count = 0;
		for (const auto& m : members)
		{
			if (strlen(m.membername) == 0)
				continue;
			else
				count++;
		}
		if (count == 0)
			return false;
	}
	return true;
}

void Raid::AddMember(Client *c, uint32 group, bool rleader, bool groupleader, bool looter)
{
	if (!c) {
		return;
	}

	const auto query = fmt::format(
		"REPLACE INTO raid_members SET raidid = {}, charid = {}, bot_id = 0, "
		"groupid = {}, _class = {}, level = {}, name = '{}', "
		"isgroupleader = {}, israidleader = {}, islooter = {}",
		GetID(),
		c->CharacterID(),
		group,
		c->GetClass(),
		c->GetLevel(),
		c->GetName(),
		groupleader,
		rleader,
		looter
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		LogError("Error inserting into raid members: [{}]", results.ErrorMessage().c_str());
	}

	LearnMembers();
	VerifyRaid();

	if (rleader && group != RAID_GROUPLESS) {
		database.SetRaidGroupLeaderInfo(group, GetID());
		UpdateRaidAAs();
	} else if (rleader) {
		database.SetRaidGroupLeaderInfo(RAID_GROUPLESS, GetID());
		UpdateRaidAAs();
	}

	if (group != RAID_GROUPLESS && groupleader) {
		database.SetRaidGroupLeaderInfo(group, GetID());
		UpdateGroupAAs(group);
	}

	if (group < MAX_RAID_GROUPS) {
		GroupUpdate(group);
	} else { // get raid AAs, GroupUpdate will handles it otherwise
		SendGroupLeadershipAA(c, RAID_GROUPLESS);
	}

	SendRaidAddAll(c->GetName());

	c->SetRaidGrouped(true);
	SendRaidMOTD(c);

	if (group == RAID_GROUPLESS) {
		if (rleader) {
			GetXTargetAutoMgr()->merge(*c->GetXTargetAutoMgr());
			c->GetXTargetAutoMgr()->clear();
			c->SetXTargetAutoMgr(GetXTargetAutoMgr());
		} else {
			if (!c->GetXTargetAutoMgr()->empty()) {
				GetXTargetAutoMgr()->merge(*c->GetXTargetAutoMgr());
				c->GetXTargetAutoMgr()->clear();
				c->RemoveAutoXTargets();
			}

			c->SetXTargetAutoMgr(GetXTargetAutoMgr());

			if (!c->GetXTargetAutoMgr()->empty())
				c->SetDirtyAutoHaters();
		}
	}

	auto* raid_update = c->GetRaid();
	if (raid_update) {
		raid_update->SendHPManaEndPacketsTo(c);
		raid_update->SendHPManaEndPacketsFrom(c);
	}

	auto pack = new ServerPacket(ServerOP_RaidAdd, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*) pack->pBuffer;
	strn0cpy(rga->playername, c->GetName(), sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}



void Raid::AddBot(Bot* b, uint32 group, bool rleader, bool groupleader, bool looter)
{
	if (!b) {
		return;
	}

	const auto query = fmt::format(
		"REPLACE INTO raid_members SET raidid = {}, "
		"charid = 0, bot_id = {}, groupid = {}, _class = {}, level = {}, name = '{}', "
		"isgroupleader = {}, israidleader = {}, islooter = {}",
		GetID(),
		b->GetBotID(),
		group,
		b->GetClass(),
		b->GetLevel(),
		b->GetName(),
		groupleader,
		rleader,
		looter
	);

	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();

	if (group < MAX_RAID_GROUPS) {
		GroupUpdate(group);
	} else { // get raid AAs, GroupUpdate will handle it otherwise
		SendGroupLeadershipAA(b->GetOwner()->CastToClient(), RAID_GROUPLESS);
	}

	SendRaidAddAll(b->GetName());

	b->SetRaidGrouped(true);
	b->p_raid_instance = this;


	auto pack = new ServerPacket(ServerOP_RaidAdd, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*) pack->pBuffer;
	strn0cpy(rga->playername, b->GetName(), sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}


void Raid::RemoveMember(const char *characterName)
{
	std::string query = StringFormat("DELETE FROM raid_members where name='%s'", characterName);
	auto results = database.QueryDatabase(query);

	auto* b = entity_list.GetBotByBotName(characterName);
	auto* c = entity_list.GetClientByName(characterName);

	if (RuleB(Bots, Enabled) && b) {
		b = entity_list.GetBotByBotName(characterName);
		b->SetFollowID(b->GetOwner()->CastToClient()->GetID());
		b->SetTarget(nullptr);
		b->SetRaidGrouped(false);
	}

	disbandCheck = true;
	SendRaidRemoveAll(characterName);
	SendRaidDisband(c);
	LearnMembers();
	VerifyRaid();

	if (c) {
		c->SetRaidGrouped(false);
		c->LeaveRaidXTargets(this);
		c->p_raid_instance = nullptr;
	}

	auto pack = new ServerPacket(ServerOP_RaidRemove, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid         = GetID();
	rga->instance_id = zone->GetInstanceID();
	rga->zoneid      = zone->GetZoneID();
	strn0cpy(rga->playername, characterName, sizeof(rga->playername));
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::DisbandRaid()
{
	const auto query = fmt::format(
		"DELETE FROM raid_members WHERE raidid = {}",
		GetID()
	);
	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();
	SendRaidDisbandAll();

	auto pack = new ServerPacket(ServerOP_RaidDisband, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	strn0cpy(rga->playername, " ", sizeof(rga->playername));
	worldserver.SendPacket(pack);
	safe_delete(pack);

	forceDisband = true;
}

void Raid::MoveMember(const char *name, uint32 newGroup)
{

	const auto query = fmt::format(
		"UPDATE raid_members SET groupid = {} WHERE name = '{}'",
		newGroup,
		name
		);
	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();
	SendRaidMoveAll(name);
	
	auto pack = new ServerPacket(ServerOP_RaidChangeGroup, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*) pack->pBuffer;
	strn0cpy(rga->playername, name, sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetGroupLeader(const char *who, bool glFlag)
{
	const auto query = fmt::format(
		"UPDATE raid_members SET isgroupleader = {} WHERE name = '{}'",
		glFlag,
		who
	);
	auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();

	auto pack = new ServerPacket(ServerOP_RaidGroupLeader, sizeof(ServerRaidGeneralAction_Struct));
	auto* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	strn0cpy(rga->playername, who, sizeof(rga->playername));
	rga->rid         = GetID();
	rga->zoneid      = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

Client *Raid::GetGroupLeader(uint32 group_id)
{
	if (group_id == RAID_GROUPLESS)
		return nullptr;

	for (const auto& m : members) {
		if (m.member && m.IsGroupLeader && m.GroupNumber == group_id) {
			return m.member;
		}
	}

	return nullptr;
}

void Raid::SetRaidLeader(const char *wasLead, const char *name)
{
	std::string query = StringFormat("UPDATE raid_members SET israidleader = 0 WHERE name = '%s'", wasLead);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("Set Raid Leader error: [{}]\n", results.ErrorMessage().c_str());

	query = StringFormat("UPDATE raid_members SET israidleader = 1 WHERE name = '%s'", name);
	results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("Set Raid Leader error: [{}]\n", results.ErrorMessage().c_str());

	strn0cpy(leadername, name, 64);

	Client *c = entity_list.GetClientByName(name);
	if (c)
		SetLeader(c);
	else
		SetLeader(nullptr); //sanity check, should never get hit but we want to prefer to NOT crash if we do VerifyRaid and leader never gets set there (raid without a leader?)

	LearnMembers();
	VerifyRaid();
	SendMakeLeaderPacket(name);

	auto pack = new ServerPacket(ServerOP_RaidLeader, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	strn0cpy(rga->playername, name, 64);
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SaveGroupLeaderAA(uint32 gid)
{
	auto queryBuffer = new char[sizeof(GroupLeadershipAA_Struct) * 2 + 1];
	database.DoEscapeString(queryBuffer, (char*)&group_aa[gid], sizeof(GroupLeadershipAA_Struct));

	std::string query = "UPDATE raid_leaders SET leadershipaa = '";
	query += queryBuffer;
	query +=  StringFormat("' WHERE gid = %lu AND rid = %lu LIMIT 1", gid, GetID());
	safe_delete_array(queryBuffer);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("Unable to store LeadershipAA: [{}]\n", results.ErrorMessage().c_str());
}

void Raid::SaveRaidLeaderAA()
{
	auto queryBuffer = new char[sizeof(RaidLeadershipAA_Struct) * 2 + 1];
	database.DoEscapeString(queryBuffer, (char*)&raid_aa, sizeof(RaidLeadershipAA_Struct));

	std::string query = "UPDATE raid_leaders SET leadershipaa = '";
	query += queryBuffer;
	query +=  StringFormat("' WHERE gid = %lu AND rid = %lu LIMIT 1", RAID_GROUPLESS, GetID());
	safe_delete_array(queryBuffer);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("Unable to store LeadershipAA: [{}]\n", results.ErrorMessage().c_str());
}

void Raid::UpdateGroupAAs(uint32 gid)
{

	if (gid > MAX_RAID_GROUPS) {
		return;
	}

	Client *gl = GetGroupLeader(gid);

	if (gl)
		gl->GetGroupAAs(&group_aa[gid]);
	else
		memset(&group_aa[gid], 0, sizeof(GroupLeadershipAA_Struct));

	SaveGroupLeaderAA(gid);
}

void Raid::UpdateRaidAAs()
{
	Client *rl = GetLeader();

	if (rl)
		rl->GetRaidAAs(&raid_aa);
	else
		memset(&raid_aa, 0, sizeof(RaidLeadershipAA_Struct));

	SaveRaidLeaderAA();
}

bool Raid::IsGroupLeader(const char* name)
{
	if (name) {
		for (const auto &m: members) {
			if (!strcmp(m.membername, name)) {
				return m.IsGroupLeader;
			}
		}
	}

	return false;
}

bool Raid::IsGroupLeader(Client *c)
{
	if (c) {
		for (const auto &m: members) {
			if (m.member == c) {
				return true;
			}
		}
	}

	return false;
}

void Raid::UpdateLevel(const char *name, int newLevel)
{
	std::string query = StringFormat("UPDATE raid_members SET level = %lu WHERE name = '%s'",
                                    (unsigned long)newLevel, name);
    auto results = database.QueryDatabase(query);

	LearnMembers();
	VerifyRaid();
}

uint32 Raid::GetFreeGroup()
{
	//check each group return the first one with 0 members assigned...
	for (int x = 0; x < MAX_RAID_GROUPS; x++)
	{
		int count = 0;
		for (const auto& m : members)
		{
			if (m.GroupNumber == x && (strlen(m.membername)>0))
				count++;
		}
		if (count == 0)
			return x;
	}
	//if we get to here then there were no free groups so we added the group as free floating members.
	return RAID_GROUPLESS;
}

uint8 Raid::GroupCount(uint32 gid)
{
	uint8 count = 0;
	if (gid < MAX_RAID_GROUPS)
	{
		for (const auto& m : members)
		{
			if (m.GroupNumber == gid && strlen(m.membername)>0)
			{
				count++;
			}
		}
	}
	else
	{
		for (const auto& m : members)
		{
			if (m.GroupNumber > 11 && strlen(m.membername)>0)
			{
				count++;
			}
		}
	}
	return count;
}

uint8 Raid::RaidCount()
{
	int count = 0;
	for (const auto& m : members)
	{
		if (strlen(m.membername) > 0)
			count++;
	}
	return count;
}

uint32 Raid::GetGroup(const char *name)
{
	for (const auto& m : members)
	{
		if (strcmp(m.membername, name) == 0)
			return m.GroupNumber;
	}
	return RAID_GROUPLESS;
}

uint32 Raid::GetGroup(Client *c)
{
	for (const auto& m : members)
	{
		if (m.member == c)
			return m.GroupNumber;
	}
	return RAID_GROUPLESS;
}

void Raid::RaidSay(const char *msg, Client *c, uint8 language, uint8 lang_skill)
{
	if (!c)
		return;

	auto pack = new ServerPacket(ServerOP_RaidSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	ServerRaidMessage_Struct *rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = RAID_GROUPLESS;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::RaidGroupSay(const char *msg, Client *c, uint8 language, uint8 lang_skill)
{
	if (!c)
		return;

	uint32 group_id_to_use = GetGroup(c->GetName());

	if (group_id_to_use > 11)
		return;

	auto pack = new ServerPacket(ServerOP_RaidGroupSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	auto rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = group_id_to_use;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint32 Raid::GetPlayerIndex(const char *name)
{
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(name, members[x].membername) == 0) {
			return x;
		}
	}
	return 0; //should never get to here if we do everything else right, set it to 0 so we never crash things that rely on it.
}

uint32 Raid::GetPlayerIndex(Client *c)
{
	for (int i = 0; i < MAX_RAID_MEMBERS; ++i)
		if (c == members[i].member)
			return i;
	return 0xFFFFFFFF; // return sentinel value, make sure you check it unlike the above function
}

Client *Raid::GetClientByIndex(uint16 index)
{
	if (index > MAX_RAID_MEMBERS) {
		return nullptr;
	}

	return members[index].member;
}

void Raid::CastGroupSpell(Mob* caster, uint16 spellid, uint32 gid)
{
	float range, distance;

	if (!caster)
		return;

	range = caster->GetAOERange(spellid);

	float range2 = range*range;

	for (const auto& m : members)
	{
		if (m.member == caster) {
			caster->SpellOnTarget(spellid, caster);
#ifdef GROUP_BUFF_PETS
			if (spells[spellid].target_type != ST_GroupNoPets && caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
				caster->SpellOnTarget(spellid, caster->GetPet());
#endif
		}
		else if (m.member != nullptr)
		{
			if (m.GroupNumber == gid) {
				distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
				if (distance <= range2) {
					caster->SpellOnTarget(spellid, m.member);
#ifdef GROUP_BUFF_PETS
					if (spells[spellid].target_type != ST_GroupNoPets && m.member->GetPet() && m.member->HasPetAffinity() && !m.member->GetPet()->IsCharmed())
						caster->SpellOnTarget(spellid, m.member->GetPet());
#endif
				}
				else{
					LogSpells("Raid spell: [{}] is out of range [{}] at distance [{}] from [{}]", m.member->GetName(), range, distance, caster->GetName());
				}
			}
		}
	}
}


uint32 Raid::GetTotalRaidDamage(Mob* other)
{
	uint32 total = 0;

	for (const auto& m : members) {
		if (!m.member)
			continue;
		if (other->CheckAggro(m.member))
			total += other->GetHateAmount(m.member,true);
	}
	return total;
}

void Raid::HealGroup(uint32 heal_amt, Mob* caster, uint32 gid, float range)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	float distance;
	float range2 = range*range;

	int numMem = 0;
	for (const auto& m : members)
	{
		if (m.member && m.GroupNumber == gid) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
			if (distance <= range2) {
				numMem += 1;
			}
		}
	}

	heal_amt /= numMem;
	for (const auto& m : members)
	{
		if (m.member && m.GroupNumber == gid) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
			if (distance <= range2) {
				m.member->SetHP(m.member->GetHP() + heal_amt);
				m.member->SendHPUpdate();
			}
		}
	}
}


void Raid::BalanceHP(int32 penalty, uint32 gid, float range, Mob* caster, int32 limit)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	int dmgtaken = 0, numMem = 0, dmgtaken_tmp = 0;

	float distance;
	float range2 = range*range;

	for (const auto& m : members)
	{
		if (m.member && m.GroupNumber == gid) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
			if (distance <= range2) {

				dmgtaken_tmp = m.member->GetMaxHP() - m.member->GetHP();
				if (limit && (dmgtaken_tmp > limit))
					dmgtaken_tmp = limit;

				dmgtaken += (dmgtaken_tmp);
				numMem += 1;
			}
		}
	}

	dmgtaken += dmgtaken * penalty / 100;
	dmgtaken /= numMem;
	for (const auto& m : members)
	{
		if (m.member && m.GroupNumber == gid) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
			if (distance <= range2) {
				if ((m.member->GetMaxHP() - dmgtaken) < 1) {//this way the ability will never kill someone
					m.member->SetHP(1);					//but it will come darn close
					m.member->SendHPUpdate();
				}
				else{
					m.member->SetHP(m.member->GetMaxHP() - dmgtaken);
					m.member->SendHPUpdate();
				}
			}
		}
	}
}

void Raid::BalanceMana(int32 penalty, uint32 gid, float range, Mob* caster, int32 limit)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	float distance;
	float range2 = range*range;

	int manataken = 0;
	int numMem = 0;
	int manataken_tmp = 0;

	for (const auto& m : members)
	{
		if (m.member && !m.IsBot) {
			if (m.GroupNumber == gid)
			{
				if (m.member->GetMaxMana() > 0) {
					distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
					if (distance <= range2) {

						manataken_tmp = m.member->GetMaxMana() - m.member->GetMana();
						if (limit && (manataken_tmp > limit))
							manataken_tmp = limit;

						manataken += (manataken_tmp);
						numMem += 1;
					}
				}
			}
		}
	}

	manataken += manataken * penalty / 100;
	manataken /= numMem;

	for (const auto& m : members)
	{
		if (m.member && m.GroupNumber == gid) {
			distance = DistanceSquared(caster->GetPosition(), m.member->GetPosition());
			if (distance <= range2) {
				if ((m.member->GetMaxMana() - manataken) < 1) {
					m.member->SetMana(1);
					if (m.member->IsClient())
						m.member->CastToClient()->SendManaUpdate();
				}
				else {
					m.member->SetMana(m.member->GetMaxMana() - manataken);
					if (m.member->IsClient())
						m.member->CastToClient()->SendManaUpdate();
				}
			}
		}
	}
}

//basically the same as Group's version just with more people like a lot of non group specific raid stuff
//this only functions if the member has a group in the raid. This does not support /autosplit?
void Raid::SplitMoney(uint32 gid, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter)
{
	//avoid unneeded work
	if (gid == RAID_GROUPLESS) {
		return;
	}

	if (
		!copper &&
		!silver &&
		!gold &&
		!platinum
	) {
		return;
	}

	uint8 member_count = 0;
	for (const auto& m : members) {
		if (m.member && m.GroupNumber == gid && m.member->IsClient()) {
			member_count++;
		}
	}

	if (!member_count) {
		return;
	}

	uint32 modifier;
	if (member_count > 1) {
		modifier = platinum % member_count;

		if (modifier) {
			platinum -= modifier;
			gold += 10 * modifier;
		}

		modifier = gold % member_count;

		if (modifier) {
			gold -= modifier;
			silver += 10 * modifier;
		}

		modifier = silver % member_count;

		if (modifier) {
			silver -= modifier;
			copper += 10 * modifier;
		}
	}

	auto copper_split = copper / member_count;
	auto silver_split = silver / member_count;
	auto gold_split = gold / member_count;
	auto platinum_split = platinum / member_count;

	for (const auto& m : members) {
		if (m.member && m.GroupNumber == gid && m.member->IsClient()) { // If Group Member is Client
			m.member->AddMoneyToPP(
				copper_split,
				silver_split,
				gold_split,
				platinum_split,
				true
			);

			if (player_event_logs.IsEventEnabled(PlayerEvent::SPLIT_MONEY)) {
				auto e = PlayerEvent::SplitMoneyEvent{
					.copper = copper_split,
					.silver = silver_split,
					.gold = gold_split,
					.platinum = platinum_split,
					.player_money_balance = m.member->GetCarriedMoney(),
				};

				RecordPlayerEventLogWithClient(m.member, PlayerEvent::SPLIT_MONEY, e);
			}

			m.member->MessageString(
				Chat::MoneySplit,
				YOU_RECEIVE_AS_SPLIT,
				Strings::Money(
					platinum_split,
					gold_split,
					silver_split,
					copper_split
				).c_str()
			);
		}
	}
}

void Raid::TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading, uint32 gid)
{
	for (const auto& m : members)
	{
		if (m.member && m.GroupNumber == gid && m.member->IsClient()) {
			m.member->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
		}
	}
}

void Raid::TeleportRaid(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading)
{
	for (const auto& m : members)
	{
		if (m.member && m.member->IsClient())
		{
			m.member->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
		}
	}
}

void Raid::ChangeLootType(uint32 type)
{
	std::string query = StringFormat("UPDATE raid_details SET loottype = %lu WHERE raidid = %lu",
                                    (unsigned long)type, (unsigned long)GetID());
    auto results = database.QueryDatabase(query);
	LootType = type;
}

void Raid::AddRaidLooter(const char* looter)
{
	std::string query = StringFormat("UPDATE raid_members SET islooter = 1 WHERE name = '%s'", looter);
	auto results = database.QueryDatabase(query);

	for (auto& m : members)
	{
		if (strcmp(looter, m.membername) == 0)
		{
			m.IsLooter = true;
			break;
		}
	}
	auto pack = new ServerPacket(ServerOP_DetailsChange, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::RemoveRaidLooter(const char* looter)
{
	std::string query = StringFormat("UPDATE raid_members SET islooter = 0 WHERE name = '%s'", looter);
	auto results = database.QueryDatabase(query);

	for (auto& m : members)
		if (strcmp(looter, m.membername) == 0) {
			m.IsLooter = false;
			break;
		}

	auto pack = new ServerPacket(ServerOP_DetailsChange, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

bool Raid::IsRaidMember(const char *name)
{
	if (name) {
		for (const auto &m: members) {
			if (!strcmp(m.membername, name)) {
				return true;
			}
		}
	}

	return false;
}

bool Raid::IsRaidMember(Client* c)
{
	if (c) {
		for (const auto &m: members) {
			if (m.member == c) {
				return true;
			}
		}
	}

	return false;
}

uint32 Raid::GetHighestLevel()
{
	uint32 highlvl = 0;
	for (const auto& m : members)
	{
		if (strlen(m.membername) && m.level > highlvl)
			highlvl = m.level;
	}
	return highlvl;
}

uint32 Raid::GetLowestLevel()
{
	uint32 lowlvl = 1000;
	for (const auto& m : members)
	{
		if (strlen(m.membername) && m.level < lowlvl)
			lowlvl = m.level;
	}
	return lowlvl;
}

/*
 * Packet Functions Start
 */
void Raid::SendRaidCreate(Client *to)
{
	if (!to)
		return;

	auto outapp = new EQApplicationPacket(OP_RaidJoin, sizeof(RaidCreate_Struct));
	auto rc = (RaidCreate_Struct*)outapp->pBuffer;
	rc->action = raidCreate;
	strn0cpy(rc->leader_name, leadername, 64);
	rc->leader_id = (GetLeader()?GetLeader()->GetID():0);
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidAdd(const char *who, Client *to)
{
	if (!to)
		return;

	std::vector<RaidMember> rm = GetMembers();

	for (const auto& m : rm) 
	{
		if (strcmp(m.membername, who) == 0)
		{
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
			auto ram = (RaidAddMember_Struct*)outapp->pBuffer;
			ram->raidGen.action = raidAdd;
			ram->raidGen.parameter = m.GroupNumber;
			strn0cpy(ram->raidGen.leader_name, m.membername, 64);
			strn0cpy(ram->raidGen.player_name, m.membername, 64);
			ram->_class = m._class;
			ram->level = m.level;
			ram->isGroupLeader = m.IsGroupLeader;
			to->QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidAddAll(const char *who)
{
	std::vector<RaidMember> rm = GetMembers();

	for (const auto& m : rm) {
		if (strcmp(m.membername, who) == 0)
		{
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
			auto ram = (RaidAddMember_Struct*)outapp->pBuffer;
			ram->raidGen.action = raidAdd;
			ram->raidGen.parameter = m.GroupNumber;
			strcpy(ram->raidGen.leader_name, m.membername);
			strcpy(ram->raidGen.player_name, m.membername);
			ram->isGroupLeader = m.IsGroupLeader;
			ram->_class = m._class;
			ram->level = m.level;

			QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidRemove(const char *who, Client *to)
{
	if (!to)
		return;

	for (const auto& m : members)
	{
		if (strcmp(m.membername, who) == 0)
		{
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
			rg->action = raidRemove2;
			strn0cpy(rg->leader_name, who, 64);
			strn0cpy(rg->player_name, who, 64);
			rg->parameter = 0;
			to->QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidRemoveAll(const char *who)
{
	for (const auto& m : members)
	{
		if (strcmp(m.membername, who) == 0)
		{
			auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
			rg->action = raidRemove2;
			strn0cpy(rg->leader_name, who, 64);
			strn0cpy(rg->player_name, who, 64);
			rg->parameter = 0;
			QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidDisband(Client *to)
{
	if (!to) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidDisband;
	strn0cpy(rg->leader_name, to->GetName(), 64);
	strn0cpy(rg->player_name, to->GetName(), 64);
	rg->parameter = 0;
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidDisbandAll()
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidDisband;
	strn0cpy(rg->leader_name, "RaidMember", 64);
	strn0cpy(rg->player_name, "RaidMember", 64);
	rg->parameter = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidMove(const char* who, Client *to)
{
	if (!to)
		return;

	Client *c = entity_list.GetClientByName(who);
	if (c && c == to) {
		SendRaidCreate(c);
		SendMakeLeaderPacketTo(leadername, c);
	}
	SendRaidRemove(who, to);
	SendRaidAdd(who, to);
	if (c && c == to) {
		SendBulkRaid(c);
		if (IsLocked()) {
			SendRaidLockTo(c);
		}
	}
}

void Raid::SendRaidMoveAll(const char* who)
{
	Client *c = entity_list.GetClientByName(who);
	SendRaidRemoveAll(who);
	if (c)
		SendRaidCreate(c);
	if (c) {
		SendBulkRaid(c);
	if (IsLocked()) { SendRaidLockTo(c); }
	}
	SendRaidAddAll(who);
	SendMakeLeaderPacket(leadername);

}

void Raid::SendBulkRaid(Client *to)
{
	if (!to)
		return;

	for (const auto& m : members)
	{
		if (strlen(m.membername) > 0 && (strcmp(m.membername, to->GetName()) != 0)) //don't send ourself
		{
			SendRaidAdd(m.membername, to);
		}
	}
}

void Raid::QueuePacket(const EQApplicationPacket *app, bool ack_req)
{
	for (const auto& m : members)
	{
		if (m.member && m.member->IsClient())
		{
			m.member->QueuePacket(app, ack_req);
		}
	}
}

void Raid::SendMakeLeaderPacket(const char *who)
{

	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(who)) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	auto rg = (RaidLeadershipUpdate_Struct*)outapp->pBuffer;
	rg->action = raidMakeLeader;
	strn0cpy(rg->leader_name, who, 64);
	strn0cpy(rg->player_name, who, 64);
	memcpy(&rg->raid, &raid_aa, sizeof(RaidLeadershipAA_Struct));
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendMakeLeaderPacketTo(const char *who, Client *to)
{
	if (!to) {
		return;
	}

	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(who)) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	auto rg = (RaidLeadershipUpdate_Struct*)outapp->pBuffer;
	rg->action = raidMakeLeader;
	strn0cpy(rg->leader_name, who, 64);
	strn0cpy(rg->player_name, who, 64);
	memcpy(&rg->raid, &raid_aa, sizeof(RaidLeadershipAA_Struct));
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

//nyi
void Raid::SendMakeGroupLeaderPacketAll()
{
}

void Raid::SendMakeGroupLeaderPacket(const char *who) //13
{
}

void Raid::SendMakeGroupLeaderPacketTo(const char *who, Client *to)
{
	if (!to)
		return;
}

void Raid::SendGroupUpdate(Client *to)
{
	if (!to) {
		return;
	}

	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(to->GetName())) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	auto gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = groupActUpdate;
	int index = 0;
	uint32 grp = GetGroup(to->GetName());
	if (grp > 11)
	{
		safe_delete(outapp);
		return;
	}
	for (const auto& m : members)
	{
		if (m.GroupNumber == grp && strlen(m.membername) > 0)
		{
			if (m.IsGroupLeader) {
				strn0cpy(gu->leadersname, m.membername, 64);
				if (strcmp(to->GetName(), m.membername) != 0) {
					strn0cpy(gu->membername[index], m.membername, 64);
					index++;
				}
			}
			else{
				if (strcmp(to->GetName(), m.membername) != 0) {
					strn0cpy(gu->membername[index], m.membername, 64);
					index++;
				}
			}
		}
	}
	if (strlen(gu->leadersname) < 1) {
		strn0cpy(gu->leadersname, to->GetName(), 64);
	}
	strn0cpy(gu->yourname, to->GetName(), 64);
	memcpy(&gu->leader_aas, &group_aa[grp], sizeof(GroupLeadershipAA_Struct));

	to->FastQueuePacket(&outapp);
}

void Raid::GroupUpdate(uint32 gid, bool initial)
{
	if (gid > 11) {//ungrouped member doesn't need grouping.
		return;
	}
	for (const auto& m : members)
	{
		if (strlen(m.membername) > 0) {
			if (m.GroupNumber == gid) {
				if (m.member) {
					SendGroupUpdate(m.member);
					SendGroupLeadershipAA(m.member, gid);
				}
			}
		}
	}
	if (initial) {
		auto pack = new ServerPacket(ServerOP_UpdateGroup, sizeof(ServerRaidGeneralAction_Struct));
		auto  rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
		rga->gid = gid;
		rga->rid = GetID();
		rga->zoneid = zone->GetZoneID();
		rga->instance_id = zone->GetInstanceID();
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Raid::SendRaidLock()
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidLock;
	strn0cpy(rg->leader_name, leadername, 64);
	strn0cpy(rg->player_name, leadername, 64);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidUnlock()
{
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidUnlock;
	strn0cpy(rg->leader_name, leadername, 64);
	strn0cpy(rg->player_name, leadername, 64);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidLockTo(Client *c)
{
	if (!c)
		return;

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidLock;
	strn0cpy(rg->leader_name, c->GetName(), 64);
	strn0cpy(rg->player_name, c->GetName(), 64);
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidUnlockTo(Client *c)
{
	if (!c)
		return;

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	auto rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidUnlock;
	strn0cpy(rg->leader_name, c->GetName(), 64);
	strn0cpy(rg->player_name, c->GetName(), 64);
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendGroupDisband(Client *to)
{
	if (!to)
		return;

	auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
	auto gu = (GroupUpdate_Struct*) outapp->pBuffer;
	gu->action = groupActDisband;
	strn0cpy(gu->leadersname, leadername, 64);
	strn0cpy(gu->yourname, to->GetName(), 64);
	to->FastQueuePacket(&outapp);
}

void Raid::SendRaidGroupAdd(const char *who, uint32 gid)
{
	auto pack = new ServerPacket(ServerOP_RaidGroupAdd, sizeof(ServerRaidGroupAction_Struct));
	auto rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = gid;
	strn0cpy(rga->membername, who, 64);
	safe_delete(pack);
}

void Raid::SendRaidGroupRemove(const char *who, uint32 gid)
{
	auto pack = new ServerPacket(ServerOP_RaidGroupRemove, sizeof(ServerRaidGroupAction_Struct));
	auto rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = gid;
	strn0cpy(rga->membername, who, 64);
	safe_delete(pack);
}

void Raid::SendRaidMOTD(Client *c)
{
	if (!c || motd.empty())
		return;

	if (entity_list.GetBotByBotName(c->GetName())) {
		return;
	}

	size_t size = motd.size() + 1;
	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidMOTD_Struct) + size);
	auto rmotd = (RaidMOTD_Struct *)outapp->pBuffer;
	rmotd->general.action = raidSetMotd;
	strn0cpy(rmotd->general.player_name, c->GetName(), 64);
	strn0cpy(rmotd->motd, motd.c_str(), size);
	c->FastQueuePacket(&outapp);
}

void Raid::SendRaidMOTD()
{
	if (motd.empty())
		return;

	for (const auto& m : members)
		if (m.member)
			SendRaidMOTD(m.member);
}

void Raid::SendRaidMOTDToWorld()
{
	if (motd.empty())
		return;

	size_t size = motd.size() + 1;
	auto pack = new ServerPacket(ServerOP_RaidMOTD, sizeof(ServerRaidMOTD_Struct) + size);
	auto smotd = (ServerRaidMOTD_Struct *)pack->pBuffer;
	smotd->rid = GetID();
	strn0cpy(smotd->motd, motd.c_str(), size);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SendGroupLeadershipAA(Client *c, uint32 gid)
{
	if (RuleB(Bots, Enabled) && entity_list.GetBotByBotName(c->GetName())) {
		return;
	}

	auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidLeadershipUpdate_Struct));
	auto rlaa = (RaidLeadershipUpdate_Struct *)outapp->pBuffer;
	rlaa->action = raidSetLeaderAbilities;
	strn0cpy(rlaa->leader_name, c->GetName(), 64);
	strn0cpy(rlaa->player_name, c->GetName(), 64);
	if (gid != RAID_GROUPLESS)
		memcpy(&rlaa->group, &group_aa[gid], sizeof(GroupLeadershipAA_Struct));
	memcpy(&rlaa->raid, &raid_aa, sizeof(RaidLeadershipAA_Struct));
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendGroupLeadershipAA(uint32 gid)
{
	for (const auto& m : members) {
		if (m.member && m.GroupNumber == gid && !m.IsBot) {
			SendGroupLeadershipAA(m.member, gid);
		}
	}
}

void Raid::SendAllRaidLeadershipAA()
{
	for (const auto& m : members) {
		if (m.member && !m.IsBot) {
			SendGroupLeadershipAA(m.member, m.GroupNumber);
		}
	}
}


void Raid::LockRaid(bool lockFlag)
{
	std::string query = StringFormat("UPDATE raid_details SET locked = %d WHERE raidid = %lu",
                                    lockFlag, (unsigned long)GetID());
    auto results = database.QueryDatabase(query);

	locked = lockFlag;
	if (lockFlag)
		SendRaidLock();
	else
		SendRaidUnlock();

	auto pack = new ServerPacket(ServerOP_RaidLockFlag, sizeof(ServerRaidGeneralAction_Struct));
	auto rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->gid = lockFlag;
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetRaidDetails()
{
	std::string query = StringFormat("INSERT INTO raid_details SET raidid = %lu, loottype = 4, locked = 0, motd = ''",
                                    (unsigned long)GetID());
    auto results = database.QueryDatabase(query);
}

void Raid::GetRaidDetails()
{
	std::string query = StringFormat("SELECT locked, loottype, motd FROM raid_details WHERE raidid = %lu",
                                    (unsigned long)GetID());
    auto results = database.QueryDatabase(query);
    if (!results.Success())
        return;

	if (results.RowCount() == 0) {
		LogError(
			"Error getting raid details for raid [{}]: [{}]",
			(unsigned long) GetID(),
			results.ErrorMessage().c_str()
		);
		return;
	}

    auto row = results.begin();

    locked = Strings::ToInt(row[0]);
    LootType = Strings::ToInt(row[1]);
	motd = std::string(row[2]);
}

void Raid::SaveRaidMOTD()
{
	std::string query = StringFormat("UPDATE raid_details SET motd = '%s' WHERE raidid = %lu",
			Strings::Escape(motd).c_str(), (unsigned long)GetID());

	auto results = database.QueryDatabase(query);
}

bool Raid::LearnMembers()
{
	memset(members, 0, (sizeof(RaidMember) * MAX_RAID_MEMBERS));

	const auto query = fmt::format(
		"SELECT name, groupid, _class, level, "
		"isgroupleader, israidleader, islooter, bot_id "
		"FROM raid_members WHERE raidid = {} ORDER BY groupid",
		GetID()
	);

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	if (!results.RowCount()) {
		LogError("Error getting raid members for raid [{}]: [{}]", GetID(), results.ErrorMessage());
		disbandCheck = true;
		return false;
	}

	int index = 0;
	for (auto row: results) {
		if (!row[0]) {
			continue;
		}

		members[index].member = nullptr;
		strn0cpy(members[index].membername, row[0], sizeof(members[index].membername));
		uint32 group_id = Strings::ToUnsignedInt(row[1]);

		if (group_id > 11) {
			members[index].GroupNumber = RAID_GROUPLESS;
		} else {
			members[index].GroupNumber = group_id;
		}

		members[index]._class        = Strings::ToUnsignedInt(row[2]);
		members[index].level         = Strings::ToUnsignedInt(row[3]);
		members[index].IsGroupLeader = Strings::ToBool(row[4]);
		members[index].IsRaidLeader  = Strings::ToBool(row[5]);
		members[index].IsLooter      = Strings::ToBool(row[6]);
		members[index].IsBot         = Strings::ToBool(row[7]) > 0;
		++index;
	}
  
	return true;
}

void Raid::VerifyRaid()
{
	for (auto& m : members) {
		if (strlen(m.membername) == 0) {
			m.member = nullptr;
		} else {
			auto* c = entity_list.GetClientByName(m.membername);
			auto* b = entity_list.GetBotByBotName(m.membername);

			if (c) {
				m.member = c;
				m.IsBot = false;
			} else if (RuleB(Bots, Enabled) && b) {
				//Raid requires client* we are forcing it here to be a BOT.  Care is needed here as any client function that
				//does not exist within the Bot Class will likely corrupt memory for the member object. Good practice to test the IsBot
				//attribute before calling a client function or casting to client.
				b = entity_list.GetBotByBotName(m.membername);
				m.member = b->CastToClient();
				m.IsBot = true; //Used to identify those members who are Bots
			} else {
				m.member = nullptr;
				m.IsBot = false;
			}
		}

		if (m.IsRaidLeader) {
			if (strlen(m.membername) > 0) {
				SetLeader(m.member);
				strn0cpy(leadername, m.membername, sizeof(leadername));
			} else {
				SetLeader(nullptr);
			}
		}
	}
}

void Raid::MemberZoned(Client *c)
{
	if (!c)
		return;

	if (leader == c)
	{
		leader = nullptr;
	}

	// Raid::GetGroup() goes over the members as well, this way we go over once
	uint32 gid = RAID_GROUPLESS;
	for (auto& m : members)
	{
		if (m.member == c)
		{
			m.member = nullptr;
			gid = m.GroupNumber;
		}
	}

	if (gid < MAX_RAID_GROUPS && group_mentor[gid].mentoree == c)
		group_mentor[gid].mentoree = nullptr;
}

void Raid::SendHPManaEndPacketsTo(Client *client)
{
	if (!client)
		return;

	uint32 group_id = GetGroup(client);

	EQApplicationPacket hp_packet;
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	for (const auto& m : members) {
		if (m.member && !m.IsBot && (m.member != client) && (m.GroupNumber == group_id)) {

			m.member->CreateHPPacket(&hp_packet);
			client->QueuePacket(&hp_packet, false);
			safe_delete_array(hp_packet.pBuffer);

			hp_packet.size = 0;
			if (client->ClientVersion() >= EQ::versions::ClientVersion::SoD) {

				outapp.SetOpcode(OP_MobManaUpdate);
				MobManaUpdate_Struct *mana_update = (MobManaUpdate_Struct *)outapp.pBuffer;
				mana_update->spawn_id = m.member->GetID();
				mana_update->mana = m.member->GetManaPercent();
				client->QueuePacket(&outapp, false);

				outapp.SetOpcode(OP_MobEnduranceUpdate);
				MobEnduranceUpdate_Struct *endurance_update = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				endurance_update->endurance = m.member->GetEndurancePercent();
				client->QueuePacket(&outapp, false);
			}
		}
	}
}

void Raid::SendHPManaEndPacketsFrom(Mob *mob)
{
	if (!mob)
		return;

	uint32 group_id = 0;

	if (mob->IsClient())
		group_id = GetGroup(mob->CastToClient());

	EQApplicationPacket hpapp;
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	mob->CreateHPPacket(&hpapp);

	for (const auto& m : members) {
		if (m.member && !m.IsBot &&
			(!mob->IsClient() || ((m.member != mob->CastToClient()) && (m.GroupNumber == group_id)))) {
			m.member->QueuePacket(&hpapp, false);
			if (m.member->ClientVersion() >= EQ::versions::ClientVersion::SoD) {
				outapp.SetOpcode(OP_MobManaUpdate);
				MobManaUpdate_Struct *mana_update = (MobManaUpdate_Struct *)outapp.pBuffer;
				mana_update->spawn_id = mob->GetID();
				mana_update->mana = mob->GetManaPercent();
				m.member->QueuePacket(&outapp, false);

				outapp.SetOpcode(OP_MobEnduranceUpdate);
				MobEnduranceUpdate_Struct *endurance_update = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				endurance_update->endurance = mob->GetEndurancePercent();
				m.member->QueuePacket(&outapp, false);
			}
		}
	}
}

void Raid::SendManaPacketFrom(Mob *mob)
{
	if (!mob)
		return;

	uint32 group_id = 0;

	if (mob->IsClient())
		group_id = GetGroup(mob->CastToClient());

	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	for (const auto& m : members) {
		if (m.member && !m.IsBot) {
			if ((!mob->IsClient() || ((m.member != mob->CastToClient()) && (m.GroupNumber == group_id))) &&
				m.member->ClientVersion() >= EQ::versions::ClientVersion::SoD
			) {
				outapp.SetOpcode(OP_MobManaUpdate);
				MobManaUpdate_Struct *mana_update = (MobManaUpdate_Struct *)outapp.pBuffer;
				mana_update->spawn_id = mob->GetID();
				mana_update->mana = mob->GetManaPercent();
				m.member->QueuePacket(&outapp, false);
			}
		}
	}
}

void Raid::SendEndurancePacketFrom(Mob *mob)
{
	if (!mob)
		return;

	uint32 group_id = 0;

	if (mob->IsClient())
		group_id = GetGroup(mob->CastToClient());

	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	for (const auto& m : members) {
		if (m.member && !m.IsBot) {
			if ((!mob->IsClient() || ((m.member != mob->CastToClient()) && (m.GroupNumber == group_id))) &&
				m.member->ClientVersion() >= EQ::versions::ClientVersion::SoD
			) {
				outapp.SetOpcode(OP_MobEnduranceUpdate);
				MobEnduranceUpdate_Struct *endurance_update = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				endurance_update->spawn_id = mob->GetID();
				endurance_update->endurance = mob->GetEndurancePercent();
				m.member->QueuePacket(&outapp, false);
			}
		}
	}
}

uint16 Raid::GetAvgLevel()
{
	double levelHolder = 0;
	uint8 i = 0;
	uint8 numMem = 0;
	while(i < MAX_RAID_MEMBERS)
	{
		if (strlen(members[i].membername))
		{
			levelHolder = levelHolder + members[i].level;
			numMem++;
		}
		i++;
	}
	levelHolder = ((levelHolder/(numMem))+.5); // total levels divided by num of characters
	return (uint16(levelHolder));
}

const char *Raid::GetClientNameByIndex(uint8 index)
{
	return members[index].membername;
}

void Raid::RaidMessageString(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9, uint32 distance)
{
	for (const auto& m : members) {
		if (m.member && m.member->IsClient() && m.member != sender) {
			m.member->MessageString(type, string_id, message, message2, message3, message4, message5, message6,
									message7, message8, message9, distance);
		}
	}
}

void Raid::LoadLeadership()
{
	database.GetRaidLeadershipInfo(GetID(), nullptr, nullptr, nullptr, nullptr, &raid_aa);

	char mentor_name[64];
	for (uint32 group_id = 0; group_id < MAX_RAID_GROUPS; group_id++) {
		database.GetGroupLeadershipInfo(group_id, GetID(), nullptr, nullptr, nullptr, nullptr,
				mentor_name, &group_mentor[group_id].mentor_percent, &group_aa[group_id]);
		if (strlen(mentor_name)) {
			group_mentor[group_id].name = mentor_name;
			mentor_name[0] = '\0';
		}
	}
}

void Raid::SetGroupMentor(uint32 group_id, int percent, char *name)
{
	if (group_id > 11)
		return;
	group_mentor[group_id].name = name;
	group_mentor[group_id].mentor_percent = percent;
	Client *client = entity_list.GetClientByName(name);
	group_mentor[group_id].mentoree = client ? client : nullptr;

	std::string query = StringFormat("UPDATE raid_leaders SET mentoree = '%s', mentor_percent = %i WHERE gid = %i AND rid = %i LIMIT 1",
			name, percent, group_id, GetID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("Unable to set raid group mentor: [{}]\n", results.ErrorMessage().c_str());
}

void Raid::ClearGroupMentor(uint32 group_id)
{
	if (group_id > 11)
		return;
	group_mentor[group_id].name.clear();
	group_mentor[group_id].mentor_percent = 0;
	group_mentor[group_id].mentoree = nullptr;

	std::string query = StringFormat("UPDATE raid_leaders SET mentoree = '', mentor_percent = 0 WHERE gid = %i AND rid = %i LIMIT 1",
			group_id, GetID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		LogError("Unable to clear raid group mentor: [{}]\n", results.ErrorMessage().c_str());
}

// there isn't a nice place to add this in another function, unlike groups
// so we do it here instead
void Raid::CheckGroupMentor(uint32 group_id, Client *c)
{
	if (!c || group_id > 11)
		return;

	if (group_mentor[group_id].name == c->GetName())
		group_mentor[group_id].mentoree = c;
}

void Raid::SetDirtyAutoHaters()
{
	for (const auto& m : members)
		if (m.member && m.IsBot)
		{
			m.member->CastToBot()->SetDirtyAutoHaters();
		}
		else if (m.member && !m.IsBot)
		{
			m.member->SetDirtyAutoHaters();
		}
}

void Raid::QueueClients(Mob *sender, const EQApplicationPacket *app, bool ack_required /*= true*/, bool ignore_sender /*= true*/, float distance /*= 0*/, bool group_only /*= true*/)
{
	if (sender && sender->IsClient()) {

		uint32 group_id = GetGroup(sender->CastToClient());

		/* If this is a group only packet and we're not in a group -- return */
		if (group_id == RAID_GROUPLESS && group_only)
			return;

		for (const auto& m : members) {
			if (!m.member) {
				continue;
			}
			if (!m.member->IsClient()) {
				continue;
			}
			if (m.IsBot) {
				continue;
			}
			if (ignore_sender && m.member == sender) {
				continue;
			}
			if (group_only && m.GroupNumber != group_id) {
				continue;
			}
			/* If we don't have a distance requirement - send to all members */
			if (distance == 0) {
				m.member->CastToClient()->QueuePacket(app, ack_required);
			}
			else {
				/* If negative distance - we check if current distance is greater than X */
				if (distance <= 0 && DistanceSquared(sender->GetPosition(), m.member->GetPosition()) >= (distance * distance)) {
					m.member->CastToClient()->QueuePacket(app, ack_required);
				}
				/* If positive distance - we check if current distance is less than X */
				else if (distance >= 0 && DistanceSquared(sender->GetPosition(), m.member->GetPosition()) <= (distance * distance)) {
					m.member->CastToClient()->QueuePacket(app, ack_required);
				}
			}
		}
	}
}

std::vector<RaidMember> Raid::GetMembers() const
{
	std::vector<RaidMember> raid_members;
	for (const auto& m : members)
	{
		if (m.membername[0])
		{
			raid_members.emplace_back(m);
		}
	}
	return raid_members;
}

bool Raid::DoesAnyMemberHaveExpeditionLockout(const std::string& expedition_name, const std::string& event_name, int max_check_count)
{
	auto raid_members = GetMembers();

	if (max_check_count > 0)
	{
		// priority is leader, group number, then ungrouped members
		std::sort(raid_members.begin(), raid_members.end(),
			[&](const RaidMember& lhs, const RaidMember& rhs) {
				if (lhs.IsRaidLeader) {
					return true;
				} else if (rhs.IsRaidLeader) {
					return false;
				}
				return lhs.GroupNumber < rhs.GroupNumber;
			});

		raid_members.resize(max_check_count);
	}

	return std::any_of(raid_members.begin(), raid_members.end(), [&](const RaidMember& raid_member) {
		return Expedition::HasLockoutByCharacterName(raid_member.membername, expedition_name, event_name);
	});
}

Mob* Raid::GetRaidMainAssist()
{
	for (const auto& m : GetMembers())
	{
		if (m.IsRaidMainAssistOne) {
			return m.member->CastToMob();
		}
	}
	return nullptr;
}

bool Raid::IsEngaged()
{

	for (const auto& m : GetMembers()) {
		if (m.member && !m.IsBot && (m.member->IsEngaged() || m.member->GetAggroCount() > 0)) {
			return true;
		}
	}
	return false;
}

void Raid::RaidGroupSay(const char* msg, const char* from, uint8 language, uint8 lang_skill)
{
	if (!from) {
		return;
	}

	uint32 group_id_to_use = GetGroup(from);

	if (group_id_to_use >= MAX_RAID_GROUPS) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_RaidGroupSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	auto rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = group_id_to_use;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, from, 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
void Raid::RaidSay(const char* msg, const char* from, uint8 language, uint8 lang_skill)
{
	if (!from) {
		return;
	}

	auto pack = new ServerPacket(ServerOP_RaidSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	auto rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = RAID_GROUPLESS;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, from, 64);

	strcpy(rga->message, msg);

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetNewRaidLeader(uint32 i)
{
	if (members[i].IsRaidLeader) {
		for (const auto& m : members) {
			if (m.member && !m.IsBot && strlen(m.membername) > 0 && strcmp(m.membername, m.membername) != 0) {
				SetRaidLeader(members[i].membername, m.membername);
				UpdateRaidAAs();
				SendAllRaidLeadershipAA();
				break;
			}
		}
	}
}