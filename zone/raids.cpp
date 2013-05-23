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
#include "../common/debug.h"
#include "masterentity.h"
#include "NpcAI.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/StringUtil.h"
#include "worldserver.h"
extern EntityList entity_list;
extern WorldServer worldserver;

Raid::Raid(uint32 raidID)
: GroupIDConsumer(raidID)
{
	memset(members ,0, (sizeof(RaidMember)*MAX_RAID_MEMBERS));
	leader = nullptr;
	memset(leadername, 0, 64);
	locked = false;
	LootType = 4;
}

Raid::Raid(Client* nLeader)
: GroupIDConsumer()
{
	memset(members ,0, (sizeof(RaidMember)*MAX_RAID_MEMBERS));
	leader = nLeader;
	memset(leadername, 0, 64);
	strn0cpy(leadername, nLeader->GetName(), 64);
	locked = false;
	LootType = 4;
}

Raid::~Raid()
{
}

bool Raid::Process()
{
	if(forceDisband)
		return false;
	if(disbandCheck)
	{
		int count = 0;
		for(int x = 0; x < MAX_RAID_MEMBERS; x++)
		{
			if(strlen(members[x].membername) == 0)
				continue;
			else
				count++;
		}
		if(count == 0)
			return false;
	}
	return true;
}

void Raid::AddMember(Client *c, uint32 group, bool rleader, bool groupleader, bool looter){
	if(!c)
		return;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "INSERT INTO raid_members SET raidid=%lu, charid=%lu, groupid=%lu, _class=%d, level=%d, name='%s', isgroupleader=%d, israidleader=%d, islooter=%d", (unsigned long)GetID(), (unsigned long)c->CharacterID(), (unsigned long)group, c->GetClass(), c->GetLevel(), c->GetName(), groupleader, rleader, looter ),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
	LearnMembers();
	VerifyRaid();
	if(group < 12)
		GroupUpdate(group);
	SendRaidAddAll(c->GetName());

	c->SetRaidGrouped(true);

	ServerPacket *pack = new ServerPacket(ServerOP_RaidAdd, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	strn0cpy(rga->playername, c->GetName(), 64);
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::RemoveMember(const char *c)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "DELETE FROM raid_members where name='%s'", c ),errbuf,&result)){
		mysql_free_result(result);
	}

	Client *m = entity_list.GetClientByName(c);
	safe_delete_array(query);
	disbandCheck = true;
	SendRaidRemoveAll(c);
	SendRaidDisband(m);
	LearnMembers();
	VerifyRaid();

	if(m){
		m->SetRaidGrouped(false);
	}

	ServerPacket *pack = new ServerPacket(ServerOP_RaidRemove, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->instance_id = zone->GetInstanceID();
	strn0cpy(rga->playername, c, 64);
	rga->zoneid = zone->GetZoneID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::DisbandRaid()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "DELETE FROM raid_members WHERE raidid=%lu", (unsigned long)GetID()),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
	LearnMembers();
	VerifyRaid();
	SendRaidDisbandAll();

	ServerPacket *pack = new ServerPacket(ServerOP_RaidDisband, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	strn0cpy(rga->playername, " ", 64);
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);

	forceDisband = true;
}

void Raid::MoveMember(const char *name, uint32 newGroup)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_members SET groupid=%lu WHERE name='%s'", (unsigned long)newGroup, name),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
	LearnMembers();
	VerifyRaid();
	SendRaidMoveAll(name);

	ServerPacket *pack = new ServerPacket(ServerOP_RaidChangeGroup, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	strn0cpy(rga->playername, name, 64);
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetGroupLeader(const char *who, bool glFlag)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_members SET isgroupleader=%lu WHERE name='%s'", (unsigned long)glFlag, who),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
	LearnMembers();
	VerifyRaid();

	//if(glFlag == true){ //we're setting the flag
		//this->SendMakeGroupLeaderPacket(who);
	//}

	ServerPacket *pack = new ServerPacket(ServerOP_RaidGroupLeader, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	strn0cpy(rga->playername, who, 64);
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetRaidLeader(const char *wasLead, const char *name)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (!database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_members SET israidleader=0 WHERE name='%s'", wasLead),errbuf,&result)){
		printf("Set Raid Leader error: %s\n", errbuf);
	}
	else
		mysql_free_result(result);

	safe_delete_array(query);
	query = 0;

	if (!database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_members SET israidleader=1 WHERE name='%s'", name),errbuf,&result)){
		printf("Set Raid Leader error: %s\n", errbuf);
	}
	else
		mysql_free_result(result);

	safe_delete_array(query);

	strn0cpy(leadername, name, 64);

	Client *c = entity_list.GetClientByName(name);
	if(c)
		SetLeader(c);

	LearnMembers();
	VerifyRaid();
	SendMakeLeaderPacket(name);

	ServerPacket *pack = new ServerPacket(ServerOP_RaidLeader, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	strn0cpy(rga->playername, name, 64);
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

bool Raid::IsGroupLeader(const char *who)
{
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(who, members[x].membername) == 0){
			return members[x].IsGroupLeader;
		}
	}

	return false;
}

void Raid::UpdateLevel(const char *name, int newLevel)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_members SET level=%lu WHERE name='%s'", (unsigned long)newLevel, name),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
	LearnMembers();
	VerifyRaid();
}

uint32 Raid::GetFreeGroup()
{
	//check each group return the first one with 0 members assigned...
	for(int x = 0; x < MAX_RAID_GROUPS; x++)
	{
		int count = 0;
		for(int y = 0; y < MAX_RAID_MEMBERS; y++)
		{
			if(members[y].GroupNumber == x && (strlen(members[y].membername)>0))
				count++;
		}
		if(count == 0)
			return x;
	}
	//if we get to here then there were no free groups so we added the group as free floating members.
	return 0xFFFFFFFF;
}

uint8 Raid::GroupCount(uint32 gid)
{
	uint8 count = 0;
	if(gid < 12)
	{
		for(int x = 0; x < MAX_RAID_MEMBERS; x++)
		{
			if(members[x].GroupNumber == gid && strlen(members[x].membername)>0)
			{
				count++;
			}
		}
	}
	else
	{
		for(int x = 0; x < MAX_RAID_MEMBERS; x++)
		{
			if(members[x].GroupNumber > 11 && strlen(members[x].membername)>0)
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
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strlen(members[x].membername) > 0)
			count++;
	}
	return count;
}

uint32 Raid::GetGroup(const char *name)
{
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(members[x].membername, name) == 0)
			return members[x].GroupNumber;
	}
	return 0xFFFFFFFF;
}

uint32 Raid::GetGroup(Client *c)
{
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(members[x].member == c)
			return members[x].GroupNumber;
	}
	return 0xFFFFFFFF;
}

void Raid::RaidSay(const char *msg, Client *c)
{
	if(!c)
		return;

	ServerPacket *pack = new ServerPacket(ServerOP_RaidSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	ServerRaidMessage_Struct *rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = 0xFFFFFFFF;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::RaidGroupSay(const char *msg, Client *c)
{
	if(!c)
		return;

	uint32 groupToUse = GetGroup(c->GetName());

	if(groupToUse > 11)
		return;

	ServerPacket *pack = new ServerPacket(ServerOP_RaidGroupSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	ServerRaidMessage_Struct *rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = groupToUse;
	strn0cpy(rga->from, c->GetName(), 64);

	strcpy(rga->message, msg); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint32 Raid::GetPlayerIndex(const char *name){
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(name, members[x].membername) == 0){
			return x;
		}
	}
	return 0; //should never get to here if we do everything else right, set it to 0 so we never crash things that rely on it.
}

Client *Raid::GetClientByIndex(uint16 index)
{
	if(index > MAX_RAID_MEMBERS)
		return nullptr;

	return members[index].member;
}

void Raid::CastGroupSpell(Mob* caster, uint16 spellid, uint32 gid)
{
	float range, distance;

	if(!caster)
		return;

	range = caster->GetAOERange(spellid);

	float range2 = range*range;

	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(members[x].member == caster) {
			caster->SpellOnTarget(spellid, caster);
#ifdef GROUP_BUFF_PETS
			if(caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
				caster->SpellOnTarget(spellid, caster->GetPet());
#endif
		}
		else if(members[x].member != nullptr)
		{
			if(members[x].GroupNumber == gid){
				distance = caster->DistNoRoot(*members[x].member);
				if(distance <= range2){
					caster->SpellOnTarget(spellid, members[x].member);
#ifdef GROUP_BUFF_PETS
					if(members[x].member->GetPet() && members[x].member->HasPetAffinity() && !members[x].member->GetPet()->IsCharmed())
						caster->SpellOnTarget(spellid, members[x].member->GetPet());
#endif
				}
				else{
					_log(SPELLS__CASTING, "Raid spell: %s is out of range %f at distance %f from %s", members[x].member->GetName(), range, distance, caster->GetName());
				}
			}
		}
	}
}


uint32 Raid::GetTotalRaidDamage(Mob* other)
{
	uint32 total = 0;

	for (uint32 i = 0; i < MAX_RAID_MEMBERS; i++) {
		if(!members[i].member)
			continue;
		if (other->CheckAggro(members[i].member))
			total += other->GetHateAmount(members[i].member,true);
	}
	return total;
}

void Raid::HealGroup(uint32 heal_amt, Mob* caster, uint32 gid)
{
	if (!caster)
		return;

	int numMem = 0;
	unsigned int gi = 0;
	for(; gi < MAX_RAID_MEMBERS; gi++)
	{
		if(members[gi].member){
			if(members[gi].GroupNumber == gid)
			{
				numMem += 1;
			}
		}
	}

	heal_amt /= numMem;
	for(gi = 0; gi < MAX_RAID_MEMBERS; gi++)
	{
		if(members[gi].member){
			if(members[gi].GroupNumber == gid)
			{
				members[gi].member->SetHP(members[gi].member->GetHP() + heal_amt);
				members[gi].member->SendHPUpdate();
			}
		}
	}
}


void Raid::BalanceHP(int32 penalty, uint32 gid)
{
	int dmgtaken = 0, numMem = 0;
	int gi = 0;
	for(; gi < MAX_RAID_MEMBERS; gi++)
	{
		if(members[gi].member){
			if(members[gi].GroupNumber == gid)
			{
				dmgtaken += (members[gi].member->GetMaxHP() - members[gi].member->GetHP());
				numMem += 1;
			}
		}
	}

	dmgtaken += dmgtaken * penalty / 100;
	dmgtaken /= numMem;
	for(gi = 0; gi < MAX_RAID_MEMBERS; gi++)
	{
		if(members[gi].member){
			if(members[gi].GroupNumber == gid)
			{
				if((members[gi].member->GetMaxHP() - dmgtaken) < 1){//this way the ability will never kill someone
					members[gi].member->SetHP(1);					//but it will come darn close
					members[gi].member->SendHPUpdate();
				}
				else{
					members[gi].member->SetHP(members[gi].member->GetMaxHP() - dmgtaken);
					members[gi].member->SendHPUpdate();
				}
			}
		}
	}
}

void Raid::BalanceMana(int32 penalty, uint32 gid)
{
	int manataken = 0, numMem = 0;
	int gi = 0;
	for(; gi < MAX_RAID_MEMBERS; gi++)
	{
		if(members[gi].member){
			if(members[gi].GroupNumber == gid)
			{
				manataken += (members[gi].member->GetMaxMana() - members[gi].member->GetMana());
				numMem += 1;
			}
		}
	}

	manataken += manataken * penalty / 100;
	manataken /= numMem;
	for(gi = 0; gi < MAX_RAID_MEMBERS; gi++)
	{
		if(members[gi].member){
			if(members[gi].GroupNumber == gid)
			{
				if((members[gi].member->GetMaxMana() - manataken) < 1){
					members[gi].member->SetMana(1);
					if (members[gi].member->IsClient())
						members[gi].member->CastToClient()->SendManaUpdate();
				}
				else{
					members[gi].member->SetMana(members[gi].member->GetMaxMana() - manataken);
					if (members[gi].member->IsClient())
						members[gi].member->CastToClient()->SendManaUpdate();
				}
			}
		}
	}
}

//basically the same as Group's version just with more people like a lot of non group specific raid stuff
void Raid::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter){
	//avoid unneeded work
	if(copper == 0 && silver == 0 && gold == 0 && platinum == 0)
		return;

	uint32 i;
	uint8 membercount = 0;
	for (i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (members[i].member != nullptr) {
			membercount++;
		}
	}

	if (membercount == 0)
		return;

	uint32 mod;
	//try to handle round off error a little better
	if(membercount > 1) {
		mod = platinum % membercount;
		if((mod) > 0) {
			platinum -= mod;
			gold += 10 * mod;
		}
		mod = gold % membercount;
		if((mod) > 0) {
			gold -= mod;
			silver += 10 * mod;
		}
		mod = silver % membercount;
		if((mod) > 0) {
			silver -= mod;
			copper += 10 * mod;
		}
	}

	//calculate the splits
	//We can still round off copper pieces, but I dont care
	uint32 sc;
	uint32 cpsplit = copper / membercount;
	sc = copper % membercount;
	uint32 spsplit = silver / membercount;
	uint32 gpsplit = gold / membercount;
	uint32 ppsplit = platinum / membercount;

	char buf[128];
	buf[63] = '\0';
	std::string msg = "You receive";
	bool one = false;

	if(ppsplit > 0) {
		snprintf(buf, 63, " %u platinum", ppsplit);
		msg += buf;
		one = true;
	}
	if(gpsplit > 0) {
		if(one)
			msg += ",";
		snprintf(buf, 63, " %u gold", gpsplit);
		msg += buf;
		one = true;
	}
	if(spsplit > 0) {
		if(one)
			msg += ",";
		snprintf(buf, 63, " %u silver", spsplit);
		msg += buf;
		one = true;
	}
	if(cpsplit > 0) {
		if(one)
			msg += ",";
		//this message is not 100% accurate for the splitter
		//if they are receiving any roundoff
		snprintf(buf, 63, " %u copper", cpsplit);
		msg += buf;
		one = true;
	}
	msg += " as your split";

	for (i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (members[i].member != nullptr) { // If Group Member is Client
		//I could not get MoneyOnCorpse to work, so we use this
		members[i].member->AddMoneyToPP(cpsplit, spsplit, gpsplit, ppsplit, true);

		members[i].member->Message(2, msg.c_str());
		}
	}
}

void Raid::GroupBardPulse(Mob* caster, uint16 spellid, uint32 gid){
	uint32 z;
	float range, distance;

	if(!caster)
		return;

	range = caster->GetAOERange(spellid);

	float range2 = range*range;

	for(z=0; z < MAX_RAID_MEMBERS; z++) {
		if(members[z].member == caster) {
			caster->BardPulse(spellid, caster);
#ifdef GROUP_BUFF_PETS
			if(caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
				caster->BardPulse(spellid, caster->GetPet());
#endif
		}
		else if(members[z].member != nullptr)
		{
			if(members[z].GroupNumber == gid){
				distance = caster->DistNoRoot(*members[z].member);
				if(distance <= range2) {
					members[z].member->BardPulse(spellid, caster);
#ifdef GROUP_BUFF_PETS
					if(members[z].member->GetPet() && members[z].member->HasPetAffinity() && !members[z].member->GetPet()->IsCharmed())
						members[z].member->GetPet()->BardPulse(spellid, caster);
#endif
				} else
					_log(SPELLS__BARDS, "Group bard pulse: %s is out of range %f at distance %f from %s", members[z].member->GetName(), range, distance, caster->GetName());
			}
		}
	}
}

void Raid::TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading, uint32 gid)
{
	for(int i = 0; i < MAX_RAID_MEMBERS; i++)
	{
		if(members[i].member)
		{
			if(members[i].GroupNumber == gid)
			{
				members[i].member->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
			}
		}

	}
}

void Raid::TeleportRaid(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading)
{
	for(int i = 0; i < MAX_RAID_MEMBERS; i++)
	{
		if(members[i].member)
		{
			members[i].member->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
		}
	}
}

void Raid::ChangeLootType(uint32 type)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_details SET loottype=%lu WHERE raidid=%lu", (unsigned long)type, (unsigned long)GetID()),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
	LootType = type;
}

void Raid::AddRaidLooter(const char* looter)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_members SET islooter=1 WHERE name='%s'", looter),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);

	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(looter, members[x].membername) == 0)
		{
			members[x].IsLooter = 1;
			break;
		}
	}
	ServerPacket *pack = new ServerPacket(ServerOP_DetailsChange, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);

	/* For reference only at this time. This code adds a looter to the Raid Options Window.

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rgs = (RaidGeneral_Struct*)outapp->pBuffer;
	rgs->action = 33;
	strcpy(rgs->leader_name, looter);
	QueuePacket(outapp);
	safe_delete(outapp); */
}

void Raid::RemoveRaidLooter(const char* looter)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_members SET islooter=0 WHERE name='%s'", looter),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);

	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(looter, members[x].membername) == 0)
		{
			members[x].IsLooter = 0;
			break;
		}
	}
	ServerPacket *pack = new ServerPacket(ServerOP_DetailsChange, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);

	/* For reference only at this time. This code removes a looter from the Raid Options Window.

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rgs = (RaidGeneral_Struct*)outapp->pBuffer;
	rgs->action = 34;
	strcpy(rgs->leader_name, looter);
	QueuePacket(outapp);
	safe_delete(outapp); */
}

bool Raid::IsRaidMember(const char *name){
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(name, members[x].membername) == 0)
			return true;
	}
	return false;
}

uint32 Raid::GetHighestLevel()
{
	uint32 highlvl = 0;
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strlen(members[x].membername))
		{
			if(members[x].level > highlvl)
				highlvl = members[x].level;
		}
	}
	return highlvl;
}

uint32 Raid::GetLowestLevel()
{
	uint32 lowlvl = 1000;
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strlen(members[x].membername))
		{
			if(members[x].level < lowlvl)
				lowlvl = members[x].level;
		}
	}
	return lowlvl;
}

/*
 * Packet Functions Start
 */
void Raid::SendRaidCreate(Client *to){
	if(!to)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidJoin,sizeof(RaidCreate_Struct));
	RaidCreate_Struct *rc = (RaidCreate_Struct*)outapp->pBuffer;
	rc->action = raidCreate;
	strn0cpy(rc->leader_name, leadername, 64);
	rc->leader_id = (GetLeader()?GetLeader()->GetID():0);
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidAdd(const char *who, Client *to)
{
	if(!to)
		return;

	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(members[x].membername, who) == 0)
		{
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
			RaidAddMember_Struct *ram = (RaidAddMember_Struct*)outapp->pBuffer;
			ram->raidGen.action = raidAdd;
			ram->raidGen.parameter = members[x].GroupNumber;
			strn0cpy(ram->raidGen.leader_name, members[x].membername, 64);
			strn0cpy(ram->raidGen.player_name, members[x].membername, 64);
			ram->_class = members[x]._class;
			ram->level = members[x].level;
			ram->isGroupLeader = members[x].IsGroupLeader;
			to->QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidAddAll(const char *who)
{
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(members[x].membername, who) == 0)
		{
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidAddMember_Struct));
			RaidAddMember_Struct *ram = (RaidAddMember_Struct*)outapp->pBuffer;
			ram->raidGen.action = raidAdd;
			ram->raidGen.parameter = members[x].GroupNumber;
			strcpy(ram->raidGen.leader_name, members[x].membername);
			strcpy(ram->raidGen.player_name, members[x].membername);
			ram->_class = members[x]._class;
			ram->level = members[x].level;
			ram->isGroupLeader = members[x].IsGroupLeader;
			this->QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
}

void Raid::SendRaidRemove(const char *who, Client *to)
{
	if(!to)
		return;

	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(members[x].membername, who) == 0)
		{
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
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
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strcmp(members[x].membername, who) == 0)
		{
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
			RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
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
	if(!to)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidDisband;
	strn0cpy(rg->leader_name, to->GetName(), 64);
	strn0cpy(rg->player_name, to->GetName(), 64);
	rg->parameter = 0;
	to->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidDisbandAll()
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidDisband;
	strn0cpy(rg->leader_name, "RaidMember", 64);
	strn0cpy(rg->player_name, "RaidMember", 64);
	rg->parameter = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidMove(const char* who, Client *to)
{
	if(!to)
		return;

	Client *c = entity_list.GetClientByName(who);
	if(c && c == to){
		SendRaidCreate(c);
		SendMakeLeaderPacketTo(leadername, c);
	}
	SendRaidRemove(who, to);
	SendRaidAdd(who, to);
	if(c && c == to){
		SendBulkRaid(c);
		if(IsLocked()) {
			SendRaidLockTo(c);
		}
	}
}

void Raid::SendRaidMoveAll(const char* who)
{
	Client *c = entity_list.GetClientByName(who);
	SendRaidRemoveAll(who);
	if(c)
		SendRaidCreate(c);
	SendMakeLeaderPacket(leadername);
	SendRaidAddAll(who);
	if(c){
		SendBulkRaid(c);
	if(IsLocked()) { SendRaidLockTo(c); }
	}
}

void Raid::SendBulkRaid(Client *to)
{
	if(!to)
		return;

	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strlen(members[x].membername) > 0 && (strcmp(members[x].membername, to->GetName()) != 0)) //don't send ourself
		{
			SendRaidAdd(members[x].membername, to);
		}
	}
}

void Raid::QueuePacket(const EQApplicationPacket *app, bool ack_req)
{
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(members[x].member)
		{
			members[x].member->QueuePacket(app, ack_req);
		}
	}
}

void Raid::SendMakeLeaderPacket(const char *who) //30
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidMakeLeader;
	strn0cpy(rg->leader_name, who, 64);
	strn0cpy(rg->player_name, who, 64);
	rg->parameter = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendMakeLeaderPacketTo(const char *who, Client *to)
{
	if(!to)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidMakeLeader;
	strn0cpy(rg->leader_name, who, 64);
	strn0cpy(rg->player_name, who, 64);
	rg->parameter = 0;
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
	if(!to)
		return;
}

void Raid::SendGroupUpdate(Client *to)
{
	if(!to)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = groupActUpdate;
	int index = 0;
	uint32 grp = GetGroup(to->GetName());
	if(grp > 11)
	{
		safe_delete(outapp);
		return;
	}
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(members[x].GroupNumber == grp && strlen(members[x].membername) > 0)
		{
			if(members[x].IsGroupLeader){
				strn0cpy(gu->leadersname, members[x].membername, 64);
				if(strcmp(to->GetName(), members[x].membername) != 0){
					strn0cpy(gu->membername[index], members[x].membername, 64);
					index++;
				}
			}
			else{
				if(strcmp(to->GetName(), members[x].membername) != 0){
					strn0cpy(gu->membername[index], members[x].membername, 64);
					index++;
				}
			}
		}
	}
	if(strlen(gu->leadersname) < 1){
		strn0cpy(gu->leadersname, to->GetName(), 64);
	}
	strn0cpy(gu->yourname, to->GetName(), 64);

	to->FastQueuePacket(&outapp);
}

void Raid::GroupUpdate(uint32 gid, bool initial)
{
	if(gid > 11) //ungrouped member doesn't need grouping.
		return;
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strlen(members[x].membername) > 0){
			if(members[x].GroupNumber == gid){
				if(members[x].member)
					SendGroupUpdate(members[x].member);
			}
		}
	}
	if(initial){
		ServerPacket *pack = new ServerPacket(ServerOP_UpdateGroup, sizeof(ServerRaidGeneralAction_Struct));
		ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidLock;
	strn0cpy(rg->leader_name, leadername, 64);
	strn0cpy(rg->player_name, leadername, 64);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidUnlock()
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidUnlock;
	strn0cpy(rg->leader_name, leadername, 64);
	strn0cpy(rg->player_name, leadername, 64);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidLockTo(Client *c)
{
	if(!c)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidLock;
	strn0cpy(rg->leader_name, c->GetName(), 64);
	strn0cpy(rg->player_name, c->GetName(), 64);
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendRaidUnlockTo(Client *c)
{
	if(!c)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
	RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
	rg->action = raidUnlock;
	strn0cpy(rg->leader_name, c->GetName(), 64);
	strn0cpy(rg->player_name, c->GetName(), 64);
	c->QueuePacket(outapp);
	safe_delete(outapp);
}

void Raid::SendGroupDisband(Client *to)
{
	if(!to)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupUpdate_Struct));
	GroupUpdate_Struct* gu = (GroupUpdate_Struct*) outapp->pBuffer;
	gu->action = groupActDisband;
	strn0cpy(gu->leadersname, leadername, 64);
	strn0cpy(gu->yourname, to->GetName(), 64);
	to->FastQueuePacket(&outapp);
}

void Raid::SendRaidGroupAdd(const char *who, uint32 gid)
{
	ServerPacket *pack = new ServerPacket(ServerOP_RaidGroupAdd, sizeof(ServerRaidGroupAction_Struct));
	ServerRaidGroupAction_Struct * rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = gid;
	strn0cpy(rga->membername, who, 64);
	safe_delete(pack);
}

void Raid::SendRaidGroupRemove(const char *who, uint32 gid)
{
	ServerPacket *pack = new ServerPacket(ServerOP_RaidGroupRemove, sizeof(ServerRaidGroupAction_Struct));
	ServerRaidGroupAction_Struct * rga = (ServerRaidGroupAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = gid;
	strn0cpy(rga->membername, who, 64);
	safe_delete(pack);
}

void Raid::LockRaid(bool lockFlag)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "UPDATE raid_details SET locked=%d WHERE raidid=%lu", lockFlag, (unsigned long)GetID()),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
	locked = lockFlag;
	if(lockFlag)
		SendRaidLock();
	else
		SendRaidUnlock();

	ServerPacket *pack = new ServerPacket(ServerOP_RaidLockFlag, sizeof(ServerRaidGeneralAction_Struct));
	ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->zoneid = zone->GetZoneID();
	rga->gid = lockFlag;
	rga->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Raid::SetRaidDetails()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "INSERT INTO raid_details SET raidid=%lu, loottype=4, locked=0", (unsigned long)GetID()),errbuf,&result)){
		mysql_free_result(result);
	}

	safe_delete_array(query);
}

void Raid::GetRaidDetails()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (database.RunQuery(query,MakeAnyLenString(&query, "SELECT locked, loottype FROM raid_details WHERE raidid=%lu", (unsigned long)GetID()),errbuf,&result)){
		safe_delete_array(query);
		if(mysql_num_rows(result) < 1) {
			mysql_free_result(result);
			LogFile->write(EQEMuLog::Error, "Error getting raid details for raid %lu: %s", (unsigned long)GetID(), errbuf);
			return;
		}
		row = mysql_fetch_row(result);
		if(row){
			locked = atoi(row[0]);
			LootType = atoi(row[1]);
		}
		mysql_free_result(result);
	}
}

bool Raid::LearnMembers()
{
	memset(members, 0, (sizeof(RaidMember)*MAX_RAID_MEMBERS));
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (database.RunQuery(query,MakeAnyLenString(&query, "SELECT name, groupid, _class, level, isgroupleader, israidleader, islooter FROM raid_members WHERE raidid=%lu", (unsigned long)GetID()),errbuf,&result)){
		safe_delete_array(query);
		if(mysql_num_rows(result) < 1) {
			mysql_free_result(result);
			LogFile->write(EQEMuLog::Error, "Error getting raid members for raid %lu: %s", (unsigned long)GetID(), errbuf);
			disbandCheck = true;
			return(false);
		}
		int i = 0;
		while((row = mysql_fetch_row(result))) {
			if(!row[0])
				continue;
			members[i].member = nullptr;
			strn0cpy(members[i].membername, row[0], 64);
			int GroupNum = atoi(row[1]);
			if(GroupNum > 11)
				members[i].GroupNumber = 0xFFFFFFFF;
			else
				members[i].GroupNumber = GroupNum;
			members[i]._class = atoi(row[2]);
			members[i].level = atoi(row[3]);
			members[i].IsGroupLeader = atoi(row[4]);
			members[i].IsRaidLeader = atoi(row[5]);
			members[i].IsLooter = atoi(row[6]);
			i++;
		}
		mysql_free_result(result);
	}
	return(true);
}

void Raid::VerifyRaid()
{
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(strlen(members[x].membername) == 0){
			members[x].member = nullptr;
		}
		else{
			Client *c = entity_list.GetClientByName(members[x].membername);
			if(c){
				members[x].member = c;
			}
			else{
				members[x].member = nullptr;
			}
		}
		if(members[x].IsRaidLeader){
			if(strlen(members[x].membername) > 0){
				SetLeader(members[x].member);
				strn0cpy(leadername, members[x].membername, 64);
			}
		}
	}
}

void Raid::MemberZoned(Client *c)
{
	if(!c)
		return;

	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(members[x].member == c)
		{
			members[x].member = nullptr;
		}
	}
}

void Raid::SendHPPacketsTo(Client *c)
{
	if(!c)
		return;

	uint32 gid = this->GetGroup(c);
	EQApplicationPacket hpapp;
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(members[x].member)
		{
			if((members[x].member != c) && (members[x].GroupNumber == gid))
			{
				members[x].member->CreateHPPacket(&hpapp);
				c->QueuePacket(&hpapp, false);
				if(c->GetClientVersion() >= EQClientSoD)
				{
					outapp.SetOpcode(OP_MobManaUpdate);
					MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
					mmus->spawn_id = members[x].member->GetID();
					mmus->mana = members[x].member->GetManaPercent();
					c->QueuePacket(&outapp, false);
					outapp.SetOpcode(OP_MobEnduranceUpdate);
					MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
					meus->endurance = members[x].member->GetEndurancePercent();
					c->QueuePacket(&outapp, false);
				}
			}
		}
	}
}

void Raid::SendHPPacketsFrom(Mob *m)
{
	if(!m)
		return;

	uint32 gid = 0;
	if(m->IsClient())
		gid = this->GetGroup(m->CastToClient());
	EQApplicationPacket hpapp;
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	m->CreateHPPacket(&hpapp);
	for(int x = 0; x < MAX_RAID_MEMBERS; x++)
	{
		if(members[x].member)
		{
			if(!m->IsClient() || ((members[x].member != m->CastToClient()) && (members[x].GroupNumber == gid)))
			{
				members[x].member->QueuePacket(&hpapp, false);
				if(members[x].member->GetClientVersion() >= EQClientSoD)
				{
					outapp.SetOpcode(OP_MobManaUpdate);
					MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
					mmus->spawn_id = m->GetID();
					mmus->mana = m->GetManaPercent();
					members[x].member->QueuePacket(&outapp, false);
					outapp.SetOpcode(OP_MobEnduranceUpdate);
					MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
					meus->endurance = m->GetEndurancePercent();
					members[x].member->QueuePacket(&outapp, false);
				}
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
		if(strlen(members[i].membername))
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

void Raid::RaidMessage_StringID(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9, uint32 distance) {
	uint32 i;
	for (i = 0; i < MAX_RAID_MEMBERS; i++) {
		if(members[i].member) {
			if(members[i].member != sender)
				members[i].member->Message_StringID(type, string_id, message, message2, message3, message4, message5, message6, message7, message8, message9, distance);
		}
	}
}

