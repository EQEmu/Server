/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "masterentity.h"
#include "npc_ai.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/string_util.h"
#include "worldserver.h"

extern EntityList entity_list;
extern WorldServer worldserver;

/*
note about how groups work:
A group contains 2 list, a list of pointers to members and a
list of member names. All members of a group should have their
name in the membername array, whether they are in the zone or not.
Only members in this zone will have non-null pointers in the
members array.
*/

//create a group which should already exist in the database
Group::Group(uint32 gid)
: GroupIDConsumer(gid)
{
	leader = nullptr;
	mentoree = nullptr;
	memset(members,0,sizeof(Mob*) * MAX_GROUP_MEMBERS);
	AssistTargetID = 0;
	TankTargetID = 0;
	PullerTargetID = 0;

	memset(&LeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));
	uint32 i;
	for(i=0;i<MAX_GROUP_MEMBERS;i++)
	{
		memset(membername[i],0,64);
		MemberRoles[i] = 0;
	}

	if(gid != 0) {
		if(!LearnMembers())
			SetID(0);
	}
	for(int i = 0; i < MAX_MARKED_NPCS; ++i)
		MarkedNPCs[i] = 0;

	NPCMarkerID = 0;
}

//creating a new group
Group::Group(Mob* leader)
: GroupIDConsumer()
{
	memset(members, 0, sizeof(members));
	members[0] = leader;
	leader->SetGrouped(true);
	SetLeader(leader);
	AssistTargetID = 0;
	TankTargetID = 0;
	PullerTargetID = 0;
	memset(&LeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));
	mentoree = nullptr;
	uint32 i;
	for(i=0;i<MAX_GROUP_MEMBERS;i++)
	{
		memset(membername[i],0,64);
		MemberRoles[i] = 0;
	}
	strcpy(membername[0],leader->GetName());

	if(leader->IsClient())
		strcpy(leader->CastToClient()->GetPP().groupMembers[0],leader->GetName());

	for(int i = 0; i < MAX_MARKED_NPCS; ++i)
		MarkedNPCs[i] = 0;

	NPCMarkerID = 0;
}

Group::~Group()
{
	for(int i = 0; i < MAX_MARKED_NPCS; ++i)
		if(MarkedNPCs[i])
		{
			Mob* m = entity_list.GetMob(MarkedNPCs[i]);
			if(m)
				m->IsTargeted(-1);
		}
}

//Split money used in OP_Split (/split and /autosplit).
void Group::SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter) {
	//avoid unneeded work
	if(copper == 0 && silver == 0 && gold == 0 && platinum == 0)
		return;

	uint32 i;
	uint8 membercount = 0;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		// Don't split with Mercs or Bots
		if (members[i] != nullptr && members[i]->IsClient()) {
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

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != nullptr && members[i]->IsClient()) { // If Group Member is Client
			Client *c = members[i]->CastToClient();
			//I could not get MoneyOnCorpse to work, so we use this
			c->AddMoneyToPP(cpsplit, spsplit, gpsplit, ppsplit, true);
			c->Message(2, msg.c_str());
		}
	}
}

bool Group::AddMember(Mob* newmember, const char *NewMemberName, uint32 CharacterID, bool ismerc)
{
	bool InZone = true;

	// This method should either be passed a Mob*, if the new member is in this zone, or a nullptr Mob*
	// and the name and CharacterID of the new member, if they are out of zone.
	if(!newmember && !NewMemberName)
	{
		return false;
	}

	if(GroupCount() >= MAX_GROUP_MEMBERS) //Sanity check for merging groups together.
	{
		return false;
	}

	if(!newmember)
	{
		InZone = false;
	}
	else
	{
		NewMemberName = newmember->GetCleanName();

		if(newmember->IsClient())
		{
			CharacterID = newmember->CastToClient()->CharacterID();
		}
		if(newmember->IsMerc())
		{
			Client* owner = newmember->CastToMerc()->GetMercOwner();
			if(owner)
			{
				CharacterID = owner->CastToClient()->CharacterID();
			}
			ismerc = true;
		}
	}

	// See if they are already in the group
	uint32 i = 0;
	for (i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(!strcasecmp(membername[i], NewMemberName))
		{
			return false;
		}
	}

	// Put them in the group
	for (i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (membername[i][0] == '\0')
		{
			if(InZone)
			{
				members[i] = newmember;
			}
			strcpy(membername[i], NewMemberName);
			MemberRoles[i] = 0;
			break;
		}
	}

	// Is this even possible based on the above loops? Remove?
	if (i == MAX_GROUP_MEMBERS)
	{
		return false;
	}

	int x=1;

	//build the template join packet
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gj = (GroupJoin_Struct*) outapp->pBuffer;
	strcpy(gj->membername, NewMemberName);
	gj->action = groupActJoin;
	gj->leader_aas = LeaderAbilities;

	for (i = 0;i < MAX_GROUP_MEMBERS; i++)
	{
		if (members[i] != nullptr && members[i] != newmember)
		{
			//fill in group join & send it
			strcpy(gj->yourname, members[i]->GetCleanName());
			if(members[i]->IsClient())
			{
				members[i]->CastToClient()->QueuePacket(outapp);

				//put new member into existing group members' list(s)
				strcpy(members[i]->CastToClient()->GetPP().groupMembers[this->GroupCount()-1], NewMemberName);
			}

			//put existing group member(s) into the new member's list
			if(InZone && newmember->IsClient())
			{
				if(IsLeader(members[i]))
				{
					strcpy(newmember->CastToClient()->GetPP().groupMembers[0], members[i]->GetCleanName());
				}
				else
				{
					strcpy(newmember->CastToClient()->GetPP().groupMembers[x], members[i]->GetCleanName());
					x++;
				}
			}
		}
	}

	if(InZone)
	{
		//put new member in his own list.
		newmember->SetGrouped(true);

		if(newmember->IsClient())
		{
			strcpy(newmember->CastToClient()->GetPP().groupMembers[x], NewMemberName);
			newmember->CastToClient()->Save();
			database.SetGroupID(NewMemberName, GetID(), newmember->CastToClient()->CharacterID(), false);
			SendMarkedNPCsToMember(newmember->CastToClient());

			NotifyMainTank(newmember->CastToClient(), 1);
			NotifyMainAssist(newmember->CastToClient(), 1);
			NotifyPuller(newmember->CastToClient(), 1);
		}

		if(newmember->IsMerc())
		{
			Client* owner = newmember->CastToMerc()->GetMercOwner();
			if(owner)
			{
				database.SetGroupID(NewMemberName, GetID(), owner->CharacterID(), true);
			}
		}
#ifdef BOTS
		for (i = 0;i < MAX_GROUP_MEMBERS; i++) {
			if (members[i] != nullptr && members[i]->IsBot()) {
				members[i]->CastToBot()->CalcChanceToCast();
			}
		}
#endif //BOTS
	}
	else
	{
		database.SetGroupID(NewMemberName, GetID(), CharacterID, ismerc);
	}

	safe_delete(outapp);

	return true;
}

void Group::AddMember(const char *NewMemberName)
{
	// This method should be called when both the new member and the group leader are in a different zone to this one.
	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if(!strcasecmp(membername[i], NewMemberName))
		{
			return;
		}

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (membername[i][0] == '\0')
		{
			strcpy(membername[i], NewMemberName);
			MemberRoles[i] = 0;
			break;
		}
	}
}


void Group::QueuePacket(const EQApplicationPacket *app, bool ack_req)
{
	uint32 i;
	for(i = 0; i < MAX_GROUP_MEMBERS; i++)
		if(members[i] && members[i]->IsClient())
			members[i]->CastToClient()->QueuePacket(app, ack_req);
}

// Sends the rest of the group's hps to member. this is useful when someone
// first joins a group, but otherwise there shouldn't be a need to call it
void Group::SendHPPacketsTo(Mob *member)
{
	if(member && member->IsClient())
	{
		EQApplicationPacket hpapp;
		EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

		for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++)
		{
			if(members[i] && members[i] != member)
			{
				members[i]->CreateHPPacket(&hpapp);
				member->CastToClient()->QueuePacket(&hpapp, false);
				if (member->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
				{
					outapp.SetOpcode(OP_MobManaUpdate);
					MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
					mmus->spawn_id = members[i]->GetID();
					mmus->mana = members[i]->GetManaPercent();
					member->CastToClient()->QueuePacket(&outapp, false);
					MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
					outapp.SetOpcode(OP_MobEnduranceUpdate);
					meus->endurance = members[i]->GetEndurancePercent();
					member->CastToClient()->QueuePacket(&outapp, false);
				}
			}
		}
	}
}

void Group::SendHPPacketsFrom(Mob *member)
{
	EQApplicationPacket hp_app;
	if(!member)
		return;

	member->CreateHPPacket(&hp_app);
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	uint32 i;
	for(i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(members[i] && members[i] != member && members[i]->IsClient())
		{
			members[i]->CastToClient()->QueuePacket(&hp_app);
			if (members[i]->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
			{
				outapp.SetOpcode(OP_MobManaUpdate);
				MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
				mmus->spawn_id = member->GetID();
				mmus->mana = member->GetManaPercent();
				members[i]->CastToClient()->QueuePacket(&outapp, false);
				MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				outapp.SetOpcode(OP_MobEnduranceUpdate);
				meus->endurance = member->GetEndurancePercent();
				members[i]->CastToClient()->QueuePacket(&outapp, false);
			}
		}
	}
}

//updates a group member's client pointer when they zone in
//if the group was in the zone already
bool Group::UpdatePlayer(Mob* update){

	bool updateSuccess = false;

	VerifyGroup();

	uint32 i=0;
	if(update->IsClient()) {
		//update their player profile
		PlayerProfile_Struct &pp = update->CastToClient()->GetPP();
		for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if(membername[i][0] == '\0')
				memset(pp.groupMembers[i], 0, 64);
			else
				strn0cpy(pp.groupMembers[i], membername[i], 64);
		}
		if(IsNPCMarker(update->CastToClient()))
		{
			NPCMarkerID = update->GetID();
			SendLeadershipAAUpdate();
		}
	}

	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (!strcasecmp(membername[i],update->GetCleanName()))
		{
			members[i] = update;
			members[i]->SetGrouped(true);
			updateSuccess = true;
			break;
		}
	}

	// mentoree isn't set, the name has a length and the name is ours! update the pointer
	if (update->IsClient() && !mentoree && mentoree_name.length() && !mentoree_name.compare(update->GetName()))
		mentoree = update->CastToClient();

	return updateSuccess;
}


void Group::MemberZoned(Mob* removemob) {
	uint32 i;

	if (removemob == nullptr)
		return;

	if(removemob == GetLeader())
		SetLeader(nullptr);

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (members[i] == removemob) {
				members[i] = nullptr;
				//should NOT clear the name, it is used for world communication.
				break;
			}
#ifdef BOTS
		if (members[i] != nullptr && members[i]->IsBot()) {
			members[i]->CastToBot()->CalcChanceToCast();
		}
#endif //BOTS
	}

	if(removemob->IsClient() && HasRole(removemob, RoleAssist))
		SetGroupAssistTarget(0);

	if(removemob->IsClient() && HasRole(removemob, RoleTank))
		SetGroupTankTarget(0);

	if(removemob->IsClient() && HasRole(removemob, RolePuller))
		SetGroupPullerTarget(0);

	if (removemob->IsClient() && removemob == mentoree)
		mentoree = nullptr;
}

void Group::SendGroupJoinOOZ(Mob* NewMember) {

	if (NewMember == nullptr)
	{
		return;
	}
	
	if (!NewMember->HasGroup())
	{
		return;
	}

	//send updates to clients out of zone...
	ServerPacket* pack = new ServerPacket(ServerOP_GroupJoin, sizeof(ServerGroupJoin_Struct));
	ServerGroupJoin_Struct* gj = (ServerGroupJoin_Struct*)pack->pBuffer;
	gj->gid = GetID();
	gj->zoneid = zone->GetZoneID();
	gj->instance_id = zone->GetInstanceID();
	strcpy(gj->member_name, NewMember->GetCleanName());
	worldserver.SendPacket(pack);
	safe_delete(pack);

}

bool Group::DelMemberOOZ(const char *Name) {

	if(!Name) return false;

	// If a member out of zone has disbanded, clear out their name.
	//
	for(unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(!strcasecmp(Name, membername[i]))
			// This shouldn't be called if the member is in this zone.
			if(!members[i]) {
				if(!strncmp(GetLeaderName(), Name, 64))
				{
					//TODO: Transfer leadership if leader disbands OOZ.
					UpdateGroupAAs();
				}

				memset(membername[i], 0, 64);
				MemberRoles[i] = 0;
				if(GroupCount() < 3)
				{
					UnDelegateMarkNPC(NPCMarkerName.c_str());
					if (GetLeader() && GetLeader()->IsClient() && GetLeader()->CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::SoD) {
							UnDelegateMainAssist(MainAssistName.c_str());
					}
					ClearAllNPCMarks();
				}
				if (Name == mentoree_name)
					ClearGroupMentor();
				return true;
			}
	}

	return false;
}

bool Group::DelMember(Mob* oldmember, bool ignoresender)
{
	if (oldmember == nullptr)
	{
		return false;
	}

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (members[i] == oldmember)
		{
			members[i] = nullptr;
			membername[i][0] = '\0';
			memset(membername[i],0,64);
			MemberRoles[i] = 0;
			break;
		}
	}

	/* This may seem pointless but the case above does not cover the following situation:
	 * Group has Leader a, member b, member c
	 * b and c are out of zone
	 * a disconnects/quits
	 * b or c zone back in and disconnects/quits
	 * a is still "leader" from GetLeader()'s perspective and will crash the zone when we DelMember(b)
	 * Ultimately we should think up a better solution to this.
	 */
	if(oldmember == GetLeader())
	{
		SetLeader(nullptr);
	}

	//handle leader quitting group gracefully
	if (oldmember == GetLeader() && GroupCount() >= 2)
	{
		for(uint32 nl = 0; nl < MAX_GROUP_MEMBERS; nl++)
		{
			if(members[nl]) 
			{
				if (members[nl]->IsClient())
				{
					ChangeLeader(members[nl]);
					break;
				}
			}
		}
	}
	
	if (GetLeader() == nullptr)
	{
		DisbandGroup();
		return true;
	}

	ServerPacket* pack = new ServerPacket(ServerOP_GroupLeave, sizeof(ServerGroupLeave_Struct));
	ServerGroupLeave_Struct* gl = (ServerGroupLeave_Struct*)pack->pBuffer;
	gl->gid = GetID();
	gl->zoneid = zone->GetZoneID();
	gl->instance_id = zone->GetInstanceID();
	strcpy(gl->member_name, oldmember->GetCleanName());
	worldserver.SendPacket(pack);
	safe_delete(pack);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*) outapp->pBuffer;
	gu->action = groupActLeave;
	strcpy(gu->membername, oldmember->GetCleanName());
	strcpy(gu->yourname, oldmember->GetCleanName());

	gu->leader_aas = LeaderAbilities;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] == nullptr) {
			//if (DEBUG>=5) LogFile->write(EQEMuLog::Debug, "Group::DelMember() null member at slot %i", i);
			continue;
		}
		if (members[i] != oldmember) {
			strcpy(gu->yourname, members[i]->GetCleanName());
			if(members[i]->IsClient())
				members[i]->CastToClient()->QueuePacket(outapp);
		}
#ifdef BOTS
		if (members[i] != nullptr && members[i]->IsBot()) {
			members[i]->CastToBot()->CalcChanceToCast();
		}
#endif //BOTS
	}

	if (!ignoresender)
	{
		strcpy(gu->yourname,oldmember->GetCleanName());
		strcpy(gu->membername,oldmember->GetCleanName());
		gu->action = groupActLeave;

		if(oldmember->IsClient())
			oldmember->CastToClient()->QueuePacket(outapp);
	}
	
	safe_delete(outapp);

	if(oldmember->IsClient())
	{
		database.SetGroupID(oldmember->GetCleanName(), 0, oldmember->CastToClient()->CharacterID(), false);
	}
	
	if(oldmember->IsMerc())
	{
		Client* owner = oldmember->CastToMerc()->GetMercOwner();
		if(owner)
		{
			database.SetGroupID(oldmember->GetCleanName(), 0, owner->CharacterID(), true);
		}
	}

	oldmember->SetGrouped(false);
	disbandcheck = true;

	if(HasRole(oldmember, RoleTank))
	{
		SetGroupTankTarget(0);
		UnDelegateMainTank(oldmember->GetCleanName());
	}

	if(HasRole(oldmember, RoleAssist))
	{
		SetGroupAssistTarget(0);
		UnDelegateMainAssist(oldmember->GetCleanName());
	}

	if(HasRole(oldmember, RolePuller))
	{
		SetGroupPullerTarget(0);
		UnDelegatePuller(oldmember->GetCleanName());
	}

	if (oldmember->GetName() == mentoree_name)
		ClearGroupMentor();

	if(oldmember->IsClient())
		SendMarkedNPCsToMember(oldmember->CastToClient(), true);

	if(GroupCount() < 3)
	{
		UnDelegateMarkNPC(NPCMarkerName.c_str());
		if (GetLeader() && GetLeader()->IsClient() && GetLeader()->CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::SoD) {
			UnDelegateMainAssist(MainAssistName.c_str());
		}
		ClearAllNPCMarks();
	}

	return true;
}

// does the caster + group
void Group::CastGroupSpell(Mob* caster, uint16 spell_id) {
	uint32 z;
	float range, distance;

	if(!caster)
		return;

	castspell = true;
	range = caster->GetAOERange(spell_id);

	float range2 = range*range;
	float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;

//	caster->SpellOnTarget(spell_id, caster);

	for(z=0; z < MAX_GROUP_MEMBERS; z++)
	{
		if(members[z] == caster) {
			caster->SpellOnTarget(spell_id, caster);
#ifdef GROUP_BUFF_PETS
			if(spells[spell_id].targettype != ST_GroupNoPets && caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
				caster->SpellOnTarget(spell_id, caster->GetPet());
#endif
		}
		else if(members[z] != nullptr)
		{
			distance = DistanceSquared(caster->GetPosition(), members[z]->GetPosition());
			if(distance <= range2 && distance >= min_range2) {
				members[z]->CalcSpellPowerDistanceMod(spell_id, distance);
				caster->SpellOnTarget(spell_id, members[z]);
#ifdef GROUP_BUFF_PETS
				if(spells[spell_id].targettype != ST_GroupNoPets && members[z]->GetPet() && members[z]->HasPetAffinity() && !members[z]->GetPet()->IsCharmed())
					caster->SpellOnTarget(spell_id, members[z]->GetPet());
#endif
			} else
				Log.Out(Logs::Detail, Logs::Spells, "Group spell: %s is out of range %f at distance %f from %s", members[z]->GetName(), range, distance, caster->GetName());
		}
	}

	castspell = false;
	disbandcheck = true;
}

// does the caster + group
void Group::GroupBardPulse(Mob* caster, uint16 spell_id) {
	uint32 z;
	float range, distance;

	if(!caster)
		return;

	castspell = true;
	range = caster->GetAOERange(spell_id);

	float range2 = range*range;

	for(z=0; z < MAX_GROUP_MEMBERS; z++) {
		if(members[z] == caster) {
			caster->BardPulse(spell_id, caster);
#ifdef GROUP_BUFF_PETS
			if(caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
				caster->BardPulse(spell_id, caster->GetPet());
#endif
		}
		else if(members[z] != nullptr)
		{
			distance = DistanceSquared(caster->GetPosition(), members[z]->GetPosition());
			if(distance <= range2) {
				members[z]->BardPulse(spell_id, caster);
#ifdef GROUP_BUFF_PETS
				if(members[z]->GetPet() && members[z]->HasPetAffinity() && !members[z]->GetPet()->IsCharmed())
					members[z]->GetPet()->BardPulse(spell_id, caster);
#endif
			} else
				Log.Out(Logs::Detail, Logs::Spells, "Group bard pulse: %s is out of range %f at distance %f from %s", members[z]->GetName(), range, distance, caster->GetName());
		}
	}
}

bool Group::IsGroupMember(Mob* client)
{
	bool Result = false;

	if(client) {
		for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (members[i] == client)
				Result = true;
		}
	}

	return Result;
}

bool Group::IsGroupMember(const char *Name)
{
	if(Name)
		for(uint32 i = 0; i < MAX_GROUP_MEMBERS; i++)
			if((strlen(Name) == strlen(membername[i])) && !strncmp(membername[i], Name, strlen(Name)))
				return true;

	return false;
}

void Group::GroupMessage(Mob* sender, uint8 language, uint8 lang_skill, const char* message) {
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(!members[i])
			continue;

		if (members[i]->IsClient() && members[i]->CastToClient()->GetFilter(FilterGroupChat)!=0)
			members[i]->CastToClient()->ChannelMessageSend(sender->GetName(),members[i]->GetName(),2,language,lang_skill,message);
	}

	ServerPacket* pack = new ServerPacket(ServerOP_OOZGroupMessage, sizeof(ServerGroupChannelMessage_Struct) + strlen(message) + 1);
	ServerGroupChannelMessage_Struct* gcm = (ServerGroupChannelMessage_Struct*)pack->pBuffer;
	gcm->zoneid = zone->GetZoneID();
	gcm->groupid = GetID();
	gcm->instanceid = zone->GetInstanceID();
	strcpy(gcm->from, sender->GetCleanName());
	strcpy(gcm->message, message);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint32 Group::GetTotalGroupDamage(Mob* other) {
	uint32 total = 0;

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(!members[i])
			continue;
		if (other->CheckAggro(members[i]))
			total += other->GetHateAmount(members[i],true);
	}
	return total;
}

void Group::DisbandGroup() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupUpdate_Struct));

	GroupUpdate_Struct* gu = (GroupUpdate_Struct*) outapp->pBuffer;
	gu->action = groupActDisband;

	Client *Leader = nullptr;

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (members[i] == nullptr)
		{
			continue;
		}

		if (members[i]->IsClient())
		{
			if(IsLeader(members[i]))
			{
				Leader = members[i]->CastToClient();
			}

			strcpy(gu->yourname, members[i]->GetCleanName());
			database.SetGroupID(members[i]->GetCleanName(), 0, members[i]->CastToClient()->CharacterID(), false);
			members[i]->CastToClient()->QueuePacket(outapp);
			SendMarkedNPCsToMember(members[i]->CastToClient(), true);
		}
		
		if (members[i]->IsMerc())
		{
			Client* owner = members[i]->CastToMerc()->GetMercOwner();
			if(owner)
			{
				database.SetGroupID(members[i]->GetCleanName(), 0, owner->CharacterID(), true);
			}
		}

		members[i]->SetGrouped(false);
		members[i] = nullptr;
		membername[i][0] = '\0';
	}

	ClearAllNPCMarks();

	ServerPacket* pack = new ServerPacket(ServerOP_DisbandGroup, sizeof(ServerDisbandGroup_Struct));
	ServerDisbandGroup_Struct* dg = (ServerDisbandGroup_Struct*)pack->pBuffer;
	dg->zoneid = zone->GetZoneID();
	dg->groupid = GetID();
	dg->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);

	if(GetID() != 0)
	{
		database.ClearGroup(GetID());
	}

	entity_list.RemoveGroup(GetID());

	if(Leader && (Leader->IsLFP()))
	{
		Leader->UpdateLFP();
	}

	safe_delete(outapp);
}

void Group::GetMemberList(std::list<Mob*>& member_list, bool clear_list)
{
	if (clear_list)
		member_list.clear();

	for (auto member_iter : members) {
		if (member_iter)
			member_list.push_back(member_iter);
	}
}

void Group::GetClientList(std::list<Client*>& client_list, bool clear_list)
{
	if (clear_list)
		client_list.clear();

	for (auto client_iter : members) {
		if (client_iter && client_iter->IsClient())
			client_list.push_back(client_iter->CastToClient());
	}
}

#ifdef BOTS
void Group::GetBotList(std::list<Bot*>& bot_list, bool clear_list)
{
	if (clear_list)
		bot_list.clear();

	for (auto bot_iter : members) {
		if (bot_iter && bot_iter->IsBot())
			bot_list.push_back(bot_iter->CastToBot());
	}
}
#endif

bool Group::Process() {
	if(disbandcheck && !GroupCount())
		return false;
	else if(disbandcheck && GroupCount())
		disbandcheck = false;
	return true;
}

void Group::SendUpdate(uint32 type, Mob* member)
{
	if(!member->IsClient())
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = type;
	strcpy(gu->yourname,member->GetName());

	int x = 0;

	gu->leader_aas = LeaderAbilities;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if((members[i] != nullptr) && IsLeader(members[i]))
		{
			strcpy(gu->leadersname, members[i]->GetName());
			break;
		}

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if (members[i] != nullptr && members[i] != member)
			strcpy(gu->membername[x++], members[i]->GetCleanName());

	member->CastToClient()->QueuePacket(outapp);

	safe_delete(outapp);
}

void Group::SendLeadershipAAUpdate()
{
	// This method updates other members of the group in the current zone with the Leader's group leadership AAs.
	//
	// It is called when the leader purchases a leadership AA or enters a zone.
	//
	// If a group member is not in the same zone as the leader when the leader purchases a new AA, they will not become
	// aware of it until they are next in the same zone as the leader.

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;
	gu->action = groupActAAUpdate;
	gu->leader_aas = LeaderAbilities;
	gu->NPCMarkerID = GetNPCMarkerID();

	uint32 i = 0;
	for (i = 0;i < MAX_GROUP_MEMBERS; ++i)
	{
		if(members[i] && members[i]->IsClient())
		{
			strcpy(gu->yourname, members[i]->GetName());
			strcpy(gu->membername, members[i]->GetName());
			members[i]->CastToClient()->QueuePacket(outapp);
		}
	}

	safe_delete(outapp);
}

uint8 Group::GroupCount() {

	uint8 MemberCount = 0;

	for(uint8 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(membername[i][0])
		{
			++MemberCount;
		}
	}

	return MemberCount;
}

uint32 Group::GetHighestLevel()
{
	uint32 level = 1;
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (members[i])
		{
			if(members[i]->GetLevel() > level)
				level = members[i]->GetLevel();
		}
	}
	return level;
}

uint32 Group::GetLowestLevel()
{
	uint32 level = 255;
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (members[i])
		{
			if(members[i]->GetLevel() < level)
				level = members[i]->GetLevel();
		}
	}
	return level;
}

void Group::TeleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading)
{
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (members[i] != nullptr && members[i]->IsClient() && members[i] != sender)
		{
			members[i]->CastToClient()->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
		}
	}
}

bool Group::LearnMembers() {
	std::string query = StringFormat("SELECT name FROM group_id WHERE groupid = %lu", (unsigned long)GetID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
        return false;

    if (results.RowCount() == 0) {
        Log.Out(Logs::General, Logs::Error, "Error getting group members for group %lu: %s", (unsigned long)GetID(), results.ErrorMessage().c_str());
			return false;
    }

	int memberIndex = 0;
    for(auto row = results.begin(); row != results.end(); ++row) {
		if(!row[0])
			continue;

		members[memberIndex] = nullptr;
		strn0cpy(membername[memberIndex], row[0], 64);

		memberIndex++;
	}

	return true;
}

void Group::VerifyGroup() {
	/*
		The purpose of this method is to make sure that a group
		is in a valid state, to prevent dangling pointers.
		Only called every once in a while (on member re-join for now).
	*/

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (membername[i][0] == '\0') {
#if EQDEBUG >= 7
	Log.Out(Logs::General, Logs::None, "Group %lu: Verify %d: Empty.\n", (unsigned long)GetID(), i);
#endif
			members[i] = nullptr;
			continue;
		}

		Mob *them = entity_list.GetMob(membername[i]);
		if(them == nullptr && members[i] != nullptr) {	//they aren't in zone
#if EQDEBUG >= 6
		Log.Out(Logs::General, Logs::None, "Member of group %lu named '%s' has disappeared!!", (unsigned long)GetID(), membername[i]);
#endif
			membername[i][0] = '\0';
			members[i] = nullptr;
			continue;
		}

		if(them != nullptr && members[i] != them) {	//our pointer is out of date... not so good.
#if EQDEBUG >= 5
		Log.Out(Logs::General, Logs::None, "Member of group %lu named '%s' had an out of date pointer!!", (unsigned long)GetID(), membername[i]);
#endif
			members[i] = them;
			continue;
		}
#if EQDEBUG >= 8
		Log.Out(Logs::General, Logs::None, "Member of group %lu named '%s' is valid.", (unsigned long)GetID(), membername[i]);
#endif
	}
}

void Group::GroupMessage_StringID(Mob* sender, uint32 type, uint32 string_id, const char* message,const char* message2,const char* message3,const char* message4,const char* message5,const char* message6,const char* message7,const char* message8,const char* message9, uint32 distance) {
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if(members[i] == nullptr)
			continue;

		if(members[i] == sender)
			continue;
			
		if(!members[i]->IsClient())
			continue;

		members[i]->Message_StringID(type, string_id, message, message2, message3, message4, message5, message6, message7, message8, message9, 0);
	}
}

void Client::LeaveGroup() {
	Group *g = GetGroup();

	if(g)
	{
		int32 MemberCount = g->GroupCount();
		// Account for both client and merc leaving the group
		if (GetMerc() && g == GetMerc()->GetGroup())
		{
			MemberCount -= 1;
		}
		
		if(MemberCount < 3)
		{
			g->DisbandGroup();
		}
		else
		{
			g->DelMember(this);
			if (GetMerc() != nullptr && g == GetMerc()->GetGroup() )
			{
				GetMerc()->RemoveMercFromGroup(GetMerc(), GetMerc()->GetGroup());
			}
		}
	}
	else
	{
		//force things a little
		database.SetGroupID(GetCleanName(), 0, CharacterID(), false);
		if (GetMerc())
		{
			database.SetGroupID(GetMerc()->GetCleanName(), 0, CharacterID(), true);
		}
	}

	isgrouped = false;
}

void Group::HealGroup(uint32 heal_amt, Mob* caster, float range)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	float distance;
	float range2 = range*range;


	int numMem = 0;
	unsigned int gi = 0;
	for(; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi]){
			distance = DistanceSquared(caster->GetPosition(), members[gi]->GetPosition());
			if(distance <= range2){
				numMem += 1;
			}
		}
	}

	heal_amt /= numMem;
	for(gi = 0; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi]){
			distance = DistanceSquared(caster->GetPosition(), members[gi]->GetPosition());
			if(distance <= range2){
				members[gi]->HealDamage(heal_amt, caster);
				members[gi]->SendHPUpdate();
			}
		}
	}
}


void Group::BalanceHP(int32 penalty, float range, Mob* caster, int32 limit)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	int dmgtaken = 0, numMem = 0, dmgtaken_tmp = 0;

	float distance;
	float range2 = range*range;

	unsigned int gi = 0;
	for(; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi]){
			distance = DistanceSquared(caster->GetPosition(), members[gi]->GetPosition());
			if(distance <= range2){

				dmgtaken_tmp = members[gi]->GetMaxHP() - members[gi]->GetHP();
				if (limit && (dmgtaken_tmp > limit))
					dmgtaken_tmp = limit;

				dmgtaken += (dmgtaken_tmp);
				numMem += 1;
			}
		}
	}

	dmgtaken += dmgtaken * penalty / 100;
	dmgtaken /= numMem;
	for(gi = 0; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi]){
			distance = DistanceSquared(caster->GetPosition(), members[gi]->GetPosition());
			if(distance <= range2){
				if((members[gi]->GetMaxHP() - dmgtaken) < 1){ //this way the ability will never kill someone
					members[gi]->SetHP(1);					//but it will come darn close
					members[gi]->SendHPUpdate();
				}
				else{
					members[gi]->SetHP(members[gi]->GetMaxHP() - dmgtaken);
					members[gi]->SendHPUpdate();
				}
			}
		}
	}
}

void Group::BalanceMana(int32 penalty, float range, Mob* caster, int32 limit)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	float distance;
	float range2 = range*range;

	int manataken = 0, numMem = 0, manataken_tmp = 0;
	unsigned int gi = 0;
	for(; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi] && (members[gi]->GetMaxMana() > 0)){
			distance = DistanceSquared(caster->GetPosition(), members[gi]->GetPosition());
			if(distance <= range2){

				manataken_tmp = members[gi]->GetMaxMana() - members[gi]->GetMana();
				if (limit && (manataken_tmp > limit))
					manataken_tmp = limit;

				manataken += (manataken_tmp);
				numMem += 1;
			}
		}
	}

	manataken += manataken * penalty / 100;
	manataken /= numMem;

	if (limit && (manataken > limit))
		manataken = limit;

	for(gi = 0; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if(members[gi]){
			distance = DistanceSquared(caster->GetPosition(), members[gi]->GetPosition());
			if(distance <= range2){
				if((members[gi]->GetMaxMana() - manataken) < 1){
					members[gi]->SetMana(1);
					if (members[gi]->IsClient())
						members[gi]->CastToClient()->SendManaUpdate();
				}
				else{
					members[gi]->SetMana(members[gi]->GetMaxMana() - manataken);
					if (members[gi]->IsClient())
						members[gi]->CastToClient()->SendManaUpdate();
				}
			}
		}
	}
}

uint16 Group::GetAvgLevel()
{
	double levelHolder = 0;
	uint8 i = 0;
	uint8 numMem = 0;
	while(i < MAX_GROUP_MEMBERS)
	{
		if (members[i])
		{
			numMem++;
			levelHolder = levelHolder + (members[i]->GetLevel());
		}
		i++;
	}
	levelHolder = ((levelHolder/numMem)+.5); // total levels divided by num of characters
	return (uint16(levelHolder));
}

void Group::MarkNPC(Mob* Target, int Number)
{
	// Send a packet to all group members in this zone causing the client to prefix the Target mob's name
	// with the specified Number.
	//
	if(!Target || Target->IsClient() || Target->IsMerc())
		return;

	if((Number < 1) || (Number > MAX_MARKED_NPCS))
		return;

	bool AlreadyMarked = false;

	uint16 EntityID = Target->GetID();

	for(int i = 0; i < MAX_MARKED_NPCS; ++i)
		if(MarkedNPCs[i] == EntityID)
		{
			if(i == (Number - 1))
				return;

			UpdateXTargetMarkedNPC(i+1, nullptr);
			MarkedNPCs[i] = 0;

			AlreadyMarked = true;

			break;
		}

	if(!AlreadyMarked)
	{
		if(MarkedNPCs[Number - 1])
		{
			Mob* m = entity_list.GetMob(MarkedNPCs[Number-1]);
			if(m)
				m->IsTargeted(-1);

			UpdateXTargetMarkedNPC(Number, nullptr);
		}

		if(EntityID)
		{
			Mob* m = entity_list.GetMob(Target->GetID());
			if(m)
				m->IsTargeted(1);
		}
	}

	MarkedNPCs[Number - 1] = EntityID;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));

	MarkNPC_Struct* mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	mnpcs->TargetID = EntityID;

	mnpcs->Number = Number;

	Mob *m = entity_list.GetMob(EntityID);

	if(m)
		sprintf(mnpcs->Name, "%s", m->GetCleanName());

	QueuePacket(outapp);

	safe_delete(outapp);

	UpdateXTargetMarkedNPC(Number, m);
}

void Group::DelegateMainTank(const char *NewMainTankName, uint8 toggle)
{
	// This method is called when the group leader Delegates the Main Tank role to a member of the group
	// (or himself). All group members in the zone are notified of the new Main Tank and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if(!NewMainTankName)
		return;

	Mob *m = entity_list.GetMob(NewMainTankName);

	if(!m)
		return;

	if(MainTankName != NewMainTankName || !toggle)
		updateDB = true;

	if(m->GetTarget())
		TankTargetID = m->GetTarget()->GetID();
	else
		TankTargetID = 0;

	Mob *mtt = TankTargetID ? entity_list.GetMob(TankTargetID) : 0;

	SetMainTank(NewMainTankName);

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(members[i] && members[i]->IsClient())
		{
			NotifyMainTank(members[i]->CastToClient(), toggle);
			members[i]->CastToClient()->UpdateXTargetType(GroupTank, m, NewMainTankName);
			members[i]->CastToClient()->UpdateXTargetType(GroupTankTarget, mtt);
		}
	}

	if(updateDB) {

		std::string query = StringFormat("UPDATE group_leaders SET maintank = '%s' WHERE gid = %i LIMIT 1",
                                        MainTankName.c_str(), GetID());
        auto results = database.QueryDatabase(query);
		if (!results.Success())
			Log.Out(Logs::General, Logs::Error, "Unable to set group main tank: %s\n", results.ErrorMessage().c_str());
	}
}

void Group::DelegateMainAssist(const char *NewMainAssistName, uint8 toggle)
{
	// This method is called when the group leader Delegates the Main Assist role to a member of the group
	// (or himself). All group members in the zone are notified of the new Main Assist and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if(!NewMainAssistName)
		return;

	Mob *m = entity_list.GetMob(NewMainAssistName);

	if(!m)
		return;

	if(MainAssistName != NewMainAssistName || !toggle)
		updateDB = true;

	if(m->GetTarget())
		AssistTargetID = m->GetTarget()->GetID();
	else
		AssistTargetID = 0;

	SetMainAssist(NewMainAssistName);

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
		if(members[i] && members[i]->IsClient())
		{
			NotifyMainAssist(members[i]->CastToClient(), toggle);
			members[i]->CastToClient()->UpdateXTargetType(GroupAssist, m, NewMainAssistName);
			members[i]->CastToClient()->UpdateXTargetType(GroupAssistTarget, m->GetTarget());
		}
	}

	if(updateDB) {

		std::string query = StringFormat("UPDATE group_leaders SET assist = '%s' WHERE gid = %i LIMIT 1",
                                        MainAssistName.c_str(), GetID());
        auto results = database.QueryDatabase(query);
		if (!results.Success())
			Log.Out(Logs::General, Logs::Error, "Unable to set group main assist: %s\n", results.ErrorMessage().c_str());

	}
}

void Group::DelegatePuller(const char *NewPullerName, uint8 toggle)
{
	// This method is called when the group leader Delegates the Puller role to a member of the group
	// (or himself). All group members in the zone are notified of the new Puller and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if(!NewPullerName)
		return;

	Mob *m = entity_list.GetMob(NewPullerName);

	if(!m)
		return;

	if(PullerName != NewPullerName || !toggle)
		updateDB = true;

	if(m->GetTarget())
		PullerTargetID = m->GetTarget()->GetID();
	else
		PullerTargetID = 0;

	SetPuller(NewPullerName);

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
		if(members[i] && members[i]->IsClient())
		{
			NotifyPuller(members[i]->CastToClient(), toggle);
			members[i]->CastToClient()->UpdateXTargetType(Puller, m, NewPullerName);
			members[i]->CastToClient()->UpdateXTargetType(PullerTarget, m->GetTarget());
		}
	}

	if(updateDB) {

		std::string query = StringFormat("UPDATE group_leaders SET puller = '%s' WHERE gid = %i LIMIT 1",
                                        PullerName.c_str(), GetID());
        auto results = database.QueryDatabase(query);
		if (!results.Success())
			Log.Out(Logs::General, Logs::Error, "Unable to set group main puller: %s\n", results.ErrorMessage().c_str());

	}

}

void Group::NotifyMainTank(Client *c, uint8 toggle)
{
	// Send a packet to the specified Client notifying them who the new Main Tank is. This causes the client to display
	// a message with the name of the Main Tank.
	//

	if(!c)
		return;

	if(!MainTankName.size())
		return;

	if (c->ClientVersion() < EQEmu::versions::ClientVersion::SoD)
	{
		if(toggle)
			c->Message(0, "%s is now Main Tank.", MainTankName.c_str());
		else
			c->Message(0, "%s is no longer Main Tank.", MainTankName.c_str());
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, MainTankName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, GetLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 1;

		grs->Toggle = toggle;

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}

}

void Group::NotifyMainAssist(Client *c, uint8 toggle)
{
	// Send a packet to the specified Client notifying them who the new Main Assist is. This causes the client to display
	// a message with the name of the Main Assist.
	//

	if(!c)
		return;

	if(!MainAssistName.size())
		return;

	if (c->ClientVersion() < EQEmu::versions::ClientVersion::SoD)
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

		DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

		das->DelegateAbility = 0;

		das->MemberNumber = 0;

		das->Action = 0;

		das->EntityID = 0;

		strn0cpy(das->Name, MainAssistName.c_str(), sizeof(das->Name));

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, MainAssistName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, GetLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 2;

		grs->Toggle = toggle;

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}

	NotifyAssistTarget(c);

}

void Group::NotifyPuller(Client *c, uint8 toggle)
{
	// Send a packet to the specified Client notifying them who the new Puller is. This causes the client to display
	// a message with the name of the Puller.
	//

	if(!c)
		return;

	if(!PullerName.size())
		return;

	if (c->ClientVersion() < EQEmu::versions::ClientVersion::SoD)
	{
		if(toggle)
			c->Message(0, "%s is now Puller.", PullerName.c_str());
		else
			c->Message(0, "%s is no longer Puller.", PullerName.c_str());
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, PullerName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, GetLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 3;

		grs->Toggle = toggle;

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}

}

void Group::UnDelegateMainTank(const char *OldMainTankName, uint8 toggle)
{
	// Called when the group Leader removes the Main Tank delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if(OldMainTankName == MainTankName) {

		std::string query = StringFormat("UPDATE group_leaders SET maintank = '' WHERE gid = %i LIMIT 1", GetID());
		auto results = database.QueryDatabase(query);
		if (!results.Success())
			Log.Out(Logs::General, Logs::Error, "Unable to clear group main tank: %s\n", results.ErrorMessage().c_str());

		if(!toggle) {
			for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if(members[i] && members[i]->IsClient())
				{
					NotifyMainTank(members[i]->CastToClient(), toggle);
					members[i]->CastToClient()->UpdateXTargetType(GroupTank, nullptr, "");
					members[i]->CastToClient()->UpdateXTargetType(GroupTankTarget, nullptr);
				}
			}
		}

		SetMainTank("");
	}
}

void Group::UnDelegateMainAssist(const char *OldMainAssistName, uint8 toggle)
{
	// Called when the group Leader removes the Main Assist delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if(OldMainAssistName == MainAssistName) {
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

		DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

		das->DelegateAbility = 0;

		das->MemberNumber = 0;

		das->Action = 1;

		das->EntityID = 0;

		strn0cpy(das->Name, OldMainAssistName, sizeof(das->Name));

		for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
			if(members[i] && members[i]->IsClient())
			{
				members[i]->CastToClient()->QueuePacket(outapp);
				members[i]->CastToClient()->UpdateXTargetType(GroupAssist, nullptr, "");
			}

		safe_delete(outapp);

		std::string query = StringFormat("UPDATE group_leaders SET assist = '' WHERE gid = %i LIMIT 1", GetID());
        auto results = database.QueryDatabase(query);
		if (!results.Success())
			Log.Out(Logs::General, Logs::Error, "Unable to clear group main assist: %s\n", results.ErrorMessage().c_str());

		if(!toggle)
		{
			for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
			{
				if(members[i] && members[i]->IsClient())
				{
					NotifyMainAssist(members[i]->CastToClient(), toggle);
					members[i]->CastToClient()->UpdateXTargetType(GroupAssistTarget, nullptr);
				}
			}
		}

		SetMainAssist("");
	}
}

void Group::UnDelegatePuller(const char *OldPullerName, uint8 toggle)
{
	// Called when the group Leader removes the Puller delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if(OldPullerName == PullerName) {

		std::string query = StringFormat("UPDATE group_leaders SET puller = '' WHERE gid = %i LIMIT 1", GetID());
        auto results = database.QueryDatabase(query);
		if (!results.Success())
			Log.Out(Logs::General, Logs::Error, "Unable to clear group main puller: %s\n", results.ErrorMessage().c_str());

		if(!toggle) {
			for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if(members[i] && members[i]->IsClient())
				{
					NotifyPuller(members[i]->CastToClient(), toggle);
					members[i]->CastToClient()->UpdateXTargetType(Puller, nullptr, "");
					members[i]->CastToClient()->UpdateXTargetType(PullerTarget, nullptr);
				}
			}
		}

		SetPuller("");
	}
}

bool Group::IsNPCMarker(Client *c)
{
	// Returns true if the specified client has been delegated the NPC Marker Role
	//
	if(!c)
		return false;

	if(NPCMarkerName.size())
		return(c->GetName() == NPCMarkerName);

	return false;

}

void Group::SetGroupAssistTarget(Mob *m)
{
	// Notify all group members in the zone of the new target the Main Assist has selected.
	//
	AssistTargetID = m ? m->GetID() : 0;

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(members[i] && members[i]->IsClient())
		{
			NotifyAssistTarget(members[i]->CastToClient());
		}
	}
}

void Group::SetGroupTankTarget(Mob *m)
{
	TankTargetID = m ? m->GetID() : 0;

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(members[i] && members[i]->IsClient())
		{
			members[i]->CastToClient()->UpdateXTargetType(GroupTankTarget, m);
		}
	}
}

void Group::SetGroupPullerTarget(Mob *m)
{
	PullerTargetID = m ? m->GetID() : 0;

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(members[i] && members[i]->IsClient())
		{
			members[i]->CastToClient()->UpdateXTargetType(PullerTarget, m);
		}
	}
}

void Group::SetGroupMentor(int percent, char *name)
{
	mentoree_name = name;
	mentor_percent = percent;
	Client *client = entity_list.GetClientByName(name);

	mentoree = client ? client : nullptr;
	std::string query = StringFormat("UPDATE group_leaders SET mentoree = '%s', mentor_percent = %i WHERE gid = %i LIMIT 1",
			mentoree_name.c_str(), mentor_percent, GetID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		Log.Out(Logs::General, Logs::Error, "Unable to set group mentor: %s\n", results.ErrorMessage().c_str());
}

void Group::ClearGroupMentor()
{
	mentoree_name.clear();
	mentor_percent = 0;
	mentoree = nullptr;
	std::string query = StringFormat("UPDATE group_leaders SET mentoree = '', mentor_percent = 0 WHERE gid = %i LIMIT 1", GetID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		Log.Out(Logs::General, Logs::Error, "Unable to clear group mentor: %s\n", results.ErrorMessage().c_str());
}

void Group::NotifyAssistTarget(Client *c)
{
	// Send a packet to the specified client notifying them of the group target selected by the Main Assist.

	if(!c)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));

	MarkNPC_Struct* mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	mnpcs->TargetID = AssistTargetID;

	mnpcs->Number = 0;

	c->QueuePacket(outapp);

	safe_delete(outapp);

	Mob *m = entity_list.GetMob(AssistTargetID);

	c->UpdateXTargetType(GroupAssistTarget, m);

}

void Group::NotifyTankTarget(Client *c)
{
	if(!c)
		return;

	Mob *m = entity_list.GetMob(TankTargetID);

	c->UpdateXTargetType(GroupTankTarget, m);
}

void Group::NotifyPullerTarget(Client *c)
{
	if(!c)
		return;

	Mob *m = entity_list.GetMob(PullerTargetID);

	c->UpdateXTargetType(PullerTarget, m);
}

void Group::DelegateMarkNPC(const char *NewNPCMarkerName)
{
	// Called when the group leader has delegated the Mark NPC ability to a group member.
	// Notify all group members in the zone of the change and save the change in the group_leaders
	// table to persist across zones.
	//
	if(NPCMarkerName.size() > 0)
		UnDelegateMarkNPC(NPCMarkerName.c_str());

	if(!NewNPCMarkerName)
		return;

	SetNPCMarker(NewNPCMarkerName);

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if(members[i] && members[i]->IsClient())
			NotifyMarkNPC(members[i]->CastToClient());

	std::string query = StringFormat("UPDATE group_leaders SET marknpc = '%s' WHERE gid = %i LIMIT 1",
                                    NewNPCMarkerName, GetID());
    auto results = database.QueryDatabase(query);
	if (!results.Success())
		Log.Out(Logs::General, Logs::Error, "Unable to set group mark npc: %s\n", results.ErrorMessage().c_str());
}

void Group::NotifyMarkNPC(Client *c)
{
	// Notify the specified client who the group member is who has been delgated the Mark NPC ability.

	if(!c)
		return;

	if(!NPCMarkerName.size())
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

	DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

	das->DelegateAbility = 1;

	das->MemberNumber = 0;

	das->Action = 0;

	das->EntityID = NPCMarkerID;

	strn0cpy(das->Name, NPCMarkerName.c_str(), sizeof(das->Name));

	c->QueuePacket(outapp);

	safe_delete(outapp);

}
void Group::SetNPCMarker(const char *NewNPCMarkerName)
{
	NPCMarkerName = NewNPCMarkerName;

	Client *m = entity_list.GetClientByName(NPCMarkerName.c_str());

	if(!m)
		NPCMarkerID = 0;
	else
		NPCMarkerID = m->GetID();
}

void Group::UnDelegateMarkNPC(const char *OldNPCMarkerName)
{
	// Notify all group members in the zone that the Mark NPC ability has been rescinded from the specified
	// group member.

	if(!OldNPCMarkerName)
		return;

	if(!NPCMarkerName.size())
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

	DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

	das->DelegateAbility = 1;

	das->MemberNumber = 0;

	das->Action = 1;

	das->EntityID = 0;

	strn0cpy(das->Name, OldNPCMarkerName, sizeof(das->Name));

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if(members[i] && members[i]->IsClient())
			members[i]->CastToClient()->QueuePacket(outapp);

	safe_delete(outapp);

	NPCMarkerName.clear();


	std::string query = StringFormat("UPDATE group_leaders SET marknpc = '' WHERE gid = %i LIMIT 1", GetID());
    auto results = database.QueryDatabase(query);
	if (!results.Success())
		Log.Out(Logs::General, Logs::Error, "Unable to clear group marknpc: %s\n", results.ErrorMessage().c_str());

}

void Group::SaveGroupLeaderAA()
{
	// Stores the Group Leaders Leadership AA data from the Player Profile as a blob in the group_leaders table.
	// This is done so that group members not in the same zone as the Leader still have access to this information.
	char *queryBuffer = new char[sizeof(GroupLeadershipAA_Struct) * 2 + 1];
    database.DoEscapeString(queryBuffer, (char*)&LeaderAbilities, sizeof(GroupLeadershipAA_Struct));

	std::string query = "UPDATE group_leaders SET leadershipaa = '";
	query += queryBuffer;
	query +=  StringFormat("' WHERE gid = %i LIMIT 1", GetID());
	safe_delete_array(queryBuffer);
    auto results = database.QueryDatabase(query);
	if (!results.Success())
		Log.Out(Logs::General, Logs::Error, "Unable to store LeadershipAA: %s\n", results.ErrorMessage().c_str());

}

void Group::UnMarkNPC(uint16 ID)
{
	// Called from entity_list when the mob with the specified ID is being destroyed.
	//
	// If the given mob has been marked by this group, it is removed from the list of marked NPCs.
	// The primary reason for doing this is so that when a new group member joins or zones in, we
	// send them correct details of which NPCs are currently marked.

	if(AssistTargetID == ID)
		AssistTargetID = 0;


	if(TankTargetID == ID)
		TankTargetID = 0;

	if(PullerTargetID == ID)
		PullerTargetID = 0;

	for(int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if(MarkedNPCs[i] == ID)
		{
			MarkedNPCs[i] = 0;
			UpdateXTargetMarkedNPC(i + 1, nullptr);
		}
	}
}

void Group::SendMarkedNPCsToMember(Client *c, bool Clear)
{
	// Send the Entity IDs of the NPCs marked by the Group Leader or delegate to the specified client.
	// If Clear == true, then tell the client to unmark the NPCs (when a member disbands).
	//
	//
	if(!c)
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));

	MarkNPC_Struct *mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	for(int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if(MarkedNPCs[i])
		{
			mnpcs->TargetID = MarkedNPCs[i];

			Mob *m = entity_list.GetMob(MarkedNPCs[i]);

			if(m)
				sprintf(mnpcs->Name, "%s", m->GetCleanName());

			if(!Clear)
				mnpcs->Number = i + 1;
			else
				mnpcs->Number = 0;

			c->QueuePacket(outapp);
			c->UpdateXTargetType((mnpcs->Number == 1) ? GroupMarkTarget1 : ((mnpcs->Number == 2) ? GroupMarkTarget2 : GroupMarkTarget3), m);
		}
	}

	safe_delete(outapp);
}

void Group::ClearAllNPCMarks()
{
	// This method is designed to be called when the number of members in the group drops below 3 and leadership AA
	// may no longer be used. It removes all NPC marks.
	//
	for(uint8 i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if(members[i] && members[i]->IsClient())
			SendMarkedNPCsToMember(members[i]->CastToClient(), true);

	for(int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if(MarkedNPCs[i])
		{
			Mob* m = entity_list.GetMob(MarkedNPCs[i]);

			if(m)
				m->IsTargeted(-1);
		}

		MarkedNPCs[i] = 0;
	}

}

int8 Group::GetNumberNeedingHealedInGroup(int8 hpr, bool includePets) {
	int8 needHealed = 0;

	for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
		if(members[i] && !members[i]->qglobal) {

			if(members[i]->GetHPRatio() <= hpr)
				needHealed++;

			if(includePets) {
				if(members[i]->GetPet() && members[i]->GetPet()->GetHPRatio() <= hpr) {
					needHealed++;
				}
			}
		}
	}

	return needHealed;
}

void Group::UpdateGroupAAs()
{
	// This method updates the Groups Leadership abilities from the Player Profile of the Leader.
	//
	Mob *m = GetLeader();

	if(m && m->IsClient())
		m->CastToClient()->GetGroupAAs(&LeaderAbilities);
	else
		memset(&LeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));

	SaveGroupLeaderAA();
}

void Group::QueueHPPacketsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app)
{
	// Send a mobs HP packets to group members if the leader has the NPC Health AA and the mob is the
	// target of the group's main assist, or is marked, and the member doesn't already have the mob targeted.

	if(!sender || !app || !GetLeadershipAA(groupAANPCHealth))
		return;

	uint16 SenderID = sender->GetID();

	if(SenderID != AssistTargetID)
	{
		bool Marked = false;

		for(int i = 0; i < MAX_MARKED_NPCS; ++i)
		{
			if(MarkedNPCs[i] == SenderID)
			{
				Marked = true;
				break;
			}
		}

		if(!Marked)
			return;

	}

	for(unsigned int i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if(members[i] && members[i]->IsClient())
		{
			if(!members[i]->GetTarget() || (members[i]->GetTarget()->GetID() != SenderID))
			{
				members[i]->CastToClient()->QueuePacket(app);
			}
		}

}

void Group::ChangeLeader(Mob* newleader)
{
	// this changes the current group leader, notifies other members, and updates leadship AA

	// if the new leader is invalid, do nothing
	if (!newleader || !newleader->IsClient())
		return;

	Mob* oldleader = GetLeader();

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*) outapp->pBuffer;
	gu->action = groupActMakeLeader;

	strcpy(gu->membername, newleader->GetName());
	strcpy(gu->yourname, oldleader->GetName());
	SetLeader(newleader);
	database.SetGroupLeaderName(GetID(), newleader->GetName());
	UpdateGroupAAs();
	gu->leader_aas = LeaderAbilities;
	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] && members[i]->IsClient())
		{
			if (members[i]->CastToClient()->ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
				members[i]->CastToClient()->SendGroupLeaderChangePacket(newleader->GetName());

			members[i]->CastToClient()->QueuePacket(outapp);
		}
	}
	safe_delete(outapp);
}

const char *Group::GetClientNameByIndex(uint8 index)
{
	return membername[index];
}

void Group::UpdateXTargetMarkedNPC(uint32 Number, Mob *m)
{
	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(members[i] && members[i]->IsClient())
		{
			members[i]->CastToClient()->UpdateXTargetType((Number == 1) ? GroupMarkTarget1 : ((Number == 2) ? GroupMarkTarget2 : GroupMarkTarget3), m);
		}
	}

}

void Group::SetMainTank(const char *NewMainTankName)
{
	MainTankName = NewMainTankName;

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(!strncasecmp(membername[i], NewMainTankName, 64))
			MemberRoles[i] |= RoleTank;
		else
			MemberRoles[i] &= ~RoleTank;
	}
}

void Group::SetMainAssist(const char *NewMainAssistName)
{
	MainAssistName = NewMainAssistName;

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(!strncasecmp(membername[i], NewMainAssistName, 64))
			MemberRoles[i] |= RoleAssist;
		else
			MemberRoles[i] &= ~RoleAssist;
	}
}

void Group::SetPuller(const char *NewPullerName)
{
	PullerName = NewPullerName;

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if(!strncasecmp(membername[i], NewPullerName, 64))
			MemberRoles[i] |= RolePuller;
		else
			MemberRoles[i] &= ~RolePuller;
	}
}

bool Group::AmIMainTank(const char *mob_name)
{
	if (!mob_name)
		return false;

	return !((bool)MainTankName.compare(mob_name));
}

bool Group::AmIMainAssist(const char *mob_name)
{
	if (!mob_name)
		return false;

	return !((bool)MainTankName.compare(mob_name));
}

bool Group::AmIPuller(const char *mob_name)
{
	if (!mob_name)
		return false;
	
	return !((bool)PullerName.compare(mob_name));
}

bool Group::HasRole(Mob *m, uint8 Role)
{
	if(!m)
		return false;

	for(uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if((m == members[i]) && (MemberRoles[i] & Role))
			return true;
	}
	return false;
}

