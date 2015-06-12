/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/classes.h"
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/eq_packet_structs.h"
#include "../common/races.h"
#include "../common/spdat.h"
#include "../common/string_util.h"

#include "aa.h"
#include "client.h"
#include "corpse.h"
#include "groups.h"
#include "mob.h"
#include "queryserv.h"
#include "raids.h"
#include "string_ids.h"
#include "titles.h"
#include "zonedb.h"

extern QueryServ* QServ;

int Client::GetAATimerID(aaID activate)
{
	//SendAA_Struct* aa2 = zone->FindAA(activate);
	//
	//if(!aa2)
	//{
	//	for(int i = 1;i < MAX_AA_ACTION_RANKS; ++i)
	//	{
	//		int a = activate - i;
	//
	//		if(a <= 0)
	//			break;
	//
	//		aa2 = zone->FindAA(a);
	//
	//		if(aa2 != nullptr)
	//			break;
	//	}
	//}
	//
	//if(aa2)
	//	return aa2->spell_type;

	return 0;
}

int Client::CalcAAReuseTimer(const AA_DBAction *caa) {

	if(!caa)
		return 0;

	int ReuseTime = caa->reuse_time;

	if(ReuseTime > 0)
	{
		int ReductionPercentage;

		if(caa->redux_aa > 0 && caa->redux_aa < aaHighestID)
		{
			ReductionPercentage = GetAA(caa->redux_aa) * caa->redux_rate;

			if(caa->redux_aa2 > 0 && caa->redux_aa2 < aaHighestID)
				ReductionPercentage += (GetAA(caa->redux_aa2) * caa->redux_rate2);

			ReuseTime = caa->reuse_time * (100 - ReductionPercentage) / 100;
		}

	}
	return ReuseTime;
}

void Client::ActivateAA(aaID activate){
//	if(activate < 0 || activate >= aaHighestID)
//		return;
//	if(IsStunned() || IsFeared() || IsMezzed() || IsSilenced() || IsPet() || IsSitting() || GetFeigned())
//		return;
//
//	int AATimerID = GetAATimerID(activate);
//
//	SendAA_Struct* aa2 = nullptr;
//	aaID aaid = activate;
//	uint8 activate_val = GetAA(activate);
//	//this wasn't taking into acct multi tiered act talents before...
//	if(activate_val == 0){
//		aa2 = zone->FindAA(activate);
//		if(!aa2){
//			int i;
//			int a;
//			for(i=1;i<MAX_AA_ACTION_RANKS;i++){
//				a = activate - i;
//				if(a <= 0)
//					break;
//
//				aa2 = zone->FindAA(a);
//				if(aa2 != nullptr)
//					break;
//			}
//		}
//		if(aa2){
//			aaid = (aaID) aa2->id;
//			activate_val = GetAA(aa2->id);
//		}
//	}
//
//	if (activate_val == 0){
//		return;
//	}
//
//	if(aa2)
//	{
//		if(aa2->account_time_required)
//		{
//			if((Timer::GetTimeSeconds() + account_creation) < aa2->account_time_required)
//			{
//				return;
//			}
//		}
//	}
//
//	if(!p_timers.Expired(&database, AATimerID + pTimerAAStart))
//	{
//		uint32 aaremain = p_timers.GetRemainingTime(AATimerID + pTimerAAStart);
//		uint32 aaremain_hr = aaremain / (60 * 60);
//		uint32 aaremain_min = (aaremain / 60) % 60;
//		uint32 aaremain_sec = aaremain % 60;
//
//		if(aa2) {
//			if (aaremain_hr >= 1)	//1 hour or more
//				Message(13, "You can use the ability %s again in %u hour(s) %u minute(s) %u seconds",
//				aa2->name, aaremain_hr, aaremain_min, aaremain_sec);
//			else	//less than an hour
//				Message(13, "You can use the ability %s again in %u minute(s) %u seconds",
//				aa2->name, aaremain_min, aaremain_sec);
//		} else {
//			if (aaremain_hr >= 1)	//1 hour or more
//				Message(13, "You can use this ability again in %u hour(s) %u minute(s) %u seconds",
//				aaremain_hr, aaremain_min, aaremain_sec);
//			else	//less than an hour
//				Message(13, "You can use this ability again in %u minute(s) %u seconds",
//				aaremain_min, aaremain_sec);
//		}
//		return;
//	}
//
//	if(activate_val > MAX_AA_ACTION_RANKS)
//		activate_val = MAX_AA_ACTION_RANKS;
//	activate_val--;		//to get array index.
//
//	//get our current node, now that the indices are well bounded
//	const AA_DBAction *caa = &AA_Actions[aaid][activate_val];
//
//	if((aaid == aaImprovedHarmTouch || aaid == aaLeechTouch) && !p_timers.Expired(&database, pTimerHarmTouch)){
//		Message(13,"Ability recovery time not yet met.");
//		return;
//	}
//
//	//everything should be configured out now
//
//	uint16 target_id = 0;
//
//	//figure out our target
//	switch(caa->target) {
//		case aaTargetUser:
//		case aaTargetGroup:
//			target_id = GetID();
//			break;
//		case aaTargetCurrent:
//		case aaTargetCurrentGroup:
//			if(GetTarget() == nullptr) {
//				Message_StringID(MT_DefaultText, AA_NO_TARGET);	//You must first select a target for this ability!
//				p_timers.Clear(&database, AATimerID + pTimerAAStart);
//				return;
//			}
//			target_id = GetTarget()->GetID();
//			break;
//		case aaTargetPet:
//			if(GetPet() == nullptr) {
//				Message(0, "A pet is required for this skill.");
//				return;
//			}
//			target_id = GetPetID();
//			break;
//	}
//
//	//handle non-spell action
//	if(caa->action != aaActionNone) {
//		if(caa->mana_cost > 0) {
//			if(GetMana() < caa->mana_cost) {
//				Message_StringID(13, INSUFFICIENT_MANA);
//				return;
//			}
//			SetMana(GetMana() - caa->mana_cost);
//		}
//		if(caa->reuse_time > 0)
//		{
//			uint32 timer_base = CalcAAReuseTimer(caa);
//			if(activate == aaImprovedHarmTouch || activate == aaLeechTouch)
//			{
//				p_timers.Start(pTimerHarmTouch, HarmTouchReuseTime);
//			}
//			p_timers.Start(AATimerID + pTimerAAStart, timer_base);
//			SendAATimer(AATimerID, 0, 0);
//		}
//		HandleAAAction(aaid);
//	}
//
//	//cast the spell, if we have one
//	if(caa->spell_id > 0 && caa->spell_id < SPDAT_RECORDS) {
//
//		if(caa->reuse_time > 0)
//		{
//			uint32 timer_base = CalcAAReuseTimer(caa);
//			SendAATimer(AATimerID, 0, 0);
//			p_timers.Start(AATimerID + pTimerAAStart, timer_base);
//			if(activate == aaImprovedHarmTouch || activate == aaLeechTouch)
//			{
//				p_timers.Start(pTimerHarmTouch, HarmTouchReuseTime);
//			}
//			// Bards can cast instant cast AAs while they are casting another song
//			if (spells[caa->spell_id].cast_time == 0 && GetClass() == BARD && IsBardSong(casting_spell_id)) {
//				if(!SpellFinished(caa->spell_id, entity_list.GetMob(target_id), 10, -1, -1, spells[caa->spell_id].ResistDiff, false)) {
//					//Reset on failed cast
//					SendAATimer(AATimerID, 0, 0xFFFFFF);
//					Message_StringID(15,ABILITY_FAILED);
//					p_timers.Clear(&database, AATimerID + pTimerAAStart);
//					return;
//				}
//			} else {
//				if (!CastSpell(caa->spell_id, target_id, USE_ITEM_SPELL_SLOT, -1, -1, 0, -1, AATimerID + pTimerAAStart, timer_base, 1)) {
//					//Reset on failed cast
//					SendAATimer(AATimerID, 0, 0xFFFFFF);
//					Message_StringID(15,ABILITY_FAILED);
//					p_timers.Clear(&database, AATimerID + pTimerAAStart);
//					return;
//				}
//			}
//		}
//		else
//		{
//			if(!CastSpell(caa->spell_id, target_id))
//				return;
//		}
//	}
//	// Check if AA is expendable
//	if (aas_send[activate - activate_val]->special_category == 7) {
//
//		// Add the AA cost to the extended profile to track overall total
//		m_epp.expended_aa += aas_send[activate]->cost;
//
//		SetAA(activate, 0);
//
//		SaveAA(); /* Save Character AA */
//		SendAA(activate);
//		SendAATable();
//	}
}

void Client::HandleAAAction(aaID activate) {
//	if(activate < 0 || activate >= aaHighestID)
//		return;
//
//	uint8 activate_val = GetAA(activate);
//
//	if (activate_val == 0)
//		return;
//
//	if(activate_val > MAX_AA_ACTION_RANKS)
//		activate_val = MAX_AA_ACTION_RANKS;
//	activate_val--;		//to get array index.
//
//	//get our current node, now that the indices are well bounded
//	const AA_DBAction *caa = &AA_Actions[activate][activate_val];
//
//	uint16 timer_id = 0;
//	uint16 timer_duration = caa->duration;
//	aaTargetType target = aaTargetUser;
//
//	uint16 spell_id = SPELL_UNKNOWN;	//gets cast at the end if not still unknown
//
//	switch(caa->action) {
//		case aaActionAETaunt:
//			entity_list.AETaunt(this);
//			break;
//
//		case aaActionFlamingArrows:
//			//toggle it
//			if(CheckAAEffect(aaEffectFlamingArrows))
//				EnableAAEffect(aaEffectFlamingArrows);
//			else
//				DisableAAEffect(aaEffectFlamingArrows);
//			break;
//
//		case aaActionFrostArrows:
//			if(CheckAAEffect(aaEffectFrostArrows))
//				EnableAAEffect(aaEffectFrostArrows);
//			else
//				DisableAAEffect(aaEffectFrostArrows);
//			break;
//
//		case aaActionRampage:
//			EnableAAEffect(aaEffectRampage, 10);
//			break;
//
//		case aaActionSharedHealth:
//			if(CheckAAEffect(aaEffectSharedHealth))
//				EnableAAEffect(aaEffectSharedHealth);
//			else
//				DisableAAEffect(aaEffectSharedHealth);
//			break;
//
//		case aaActionCelestialRegen: {
//			//special because spell_id depends on a different AA
//			switch (GetAA(aaCelestialRenewal)) {
//				case 1:
//					spell_id = 3250;
//					break;
//				case 2:
//					spell_id = 3251;
//					break;
//				default:
//					spell_id = 2740;
//					break;
//			}
//			target = aaTargetCurrent;
//			break;
//		}
//
//		case aaActionDireCharm: {
//			//special because spell_id depends on class
//			switch (GetClass())
//			{
//				case DRUID:
//					spell_id = 2760;	//2644?
//					break;
//				case NECROMANCER:
//					spell_id = 2759;	//2643?
//					break;
//				case ENCHANTER:
//					spell_id = 2761;	//2642?
//					break;
//			}
//			target = aaTargetCurrent;
//			break;
//		}
//
//		case aaActionImprovedFamiliar: {
//			//Spell IDs might be wrong...
//			if (GetAA(aaAllegiantFamiliar))
//				spell_id = 3264;	//1994?
//			else
//				spell_id = 2758;	//2155?
//			break;
//		}
//
//		case aaActionActOfValor:
//			if(GetTarget() != nullptr) {
//				int curhp = GetTarget()->GetHP();
//				target = aaTargetCurrent;
//				GetTarget()->HealDamage(curhp, this);
//				Death(this, 0, SPELL_UNKNOWN, SkillHandtoHand);
//			}
//			break;
//
//		case aaActionSuspendedMinion:
//			if (GetPet()) {
//				target = aaTargetPet;
//				switch (GetAA(aaSuspendedMinion)) {
//					case 1:
//						spell_id = 3248;
//						break;
//					case 2:
//						spell_id = 3249;
//						break;
//				}
//				//do we really need to cast a spell?
//
//				Message(0,"You call your pet to your side.");
//				GetPet()->WipeHateList();
//				GetPet()->GMMove(GetX(),GetY(),GetZ());
//				if (activate_val > 1)
//					entity_list.ClearFeignAggro(GetPet());
//			} else {
//				Message(0,"You have no pet to call.");
//			}
//			break;
//
//		case aaActionEscape:
//			Escape();
//			break;
//
//		// Don't think this code is used any longer for Bestial Alignment as the aa.has a spell_id and no nonspell_action.
//		case aaActionBeastialAlignment:
//			switch(GetBaseRace()) {
//				case BARBARIAN:
//					spell_id = AA_Choose3(activate_val, 4521, 4522, 4523);
//					break;
//				case TROLL:
//					spell_id = AA_Choose3(activate_val, 4524, 4525, 4526);
//					break;
//				case OGRE:
//					spell_id = AA_Choose3(activate_val, 4527, 4527, 4529);
//					break;
//				case IKSAR:
//					spell_id = AA_Choose3(activate_val, 4530, 4531, 4532);
//					break;
//				case VAHSHIR:
//					spell_id = AA_Choose3(activate_val, 4533, 4534, 4535);
//					break;
//			}
//
//		case aaActionLeechTouch:
//			target = aaTargetCurrent;
//			spell_id = SPELL_HARM_TOUCH2;
//			EnableAAEffect(aaEffectLeechTouch, 1000);
//			break;
//
//		case aaActionFadingMemories:
//			// Do nothing since spell effect works correctly, but mana isn't used.
//			break;
//
//		default:
//			Log.Out(Logs::General, Logs::Error, "Unknown AA nonspell action type %d", caa->action);
//			return;
//	}
//
//
//	uint16 target_id = 0;
//	//figure out our target
//	switch(target) {
//		case aaTargetUser:
//		case aaTargetGroup:
//			target_id = GetID();
//			break;
//		case aaTargetCurrent:
//		case aaTargetCurrentGroup:
//			if(GetTarget() == nullptr) {
//				Message_StringID(MT_DefaultText, AA_NO_TARGET);	//You must first select a target for this ability!
//				p_timers.Clear(&database, timer_id + pTimerAAEffectStart);
//				return;
//			}
//			target_id = GetTarget()->GetID();
//			break;
//		case aaTargetPet:
//			if(GetPet() == nullptr) {
//				Message(0, "A pet is required for this skill.");
//				return;
//			}
//			target_id = GetPetID();
//			break;
//	}
//
//	//cast the spell, if we have one
//	if(IsValidSpell(spell_id)) {
//		int aatid = GetAATimerID(activate);
//		if (!CastSpell(spell_id, target_id, USE_ITEM_SPELL_SLOT, -1, -1, 0, -1, pTimerAAStart + aatid, CalcAAReuseTimer(caa), 1)) {
//			SendAATimer(aatid, 0, 0xFFFFFF);
//			Message_StringID(15,ABILITY_FAILED);
//			p_timers.Clear(&database, pTimerAAStart + aatid);
//			return;
//		}
//	}
//
//	//handle the duration timer if we have one.
//	if(timer_id > 0 && timer_duration > 0) {
//		p_timers.Start(pTimerAAEffectStart + timer_id, timer_duration);
//	}
}

void Mob::TemporaryPets(uint16 spell_id, Mob *targ, const char *name_override, uint32 duration_override, bool followme, bool sticktarg) {

	//It might not be a bad idea to put these into the database, eventually..

	//Dook- swarms and wards

	PetRecord record;
	if(!database.GetPetEntry(spells[spell_id].teleport_zone, &record))
	{
		Log.Out(Logs::General, Logs::Error, "Unknown swarm pet spell id: %d, check pets table", spell_id);
		Message(13, "Unable to find data for pet %s", spells[spell_id].teleport_zone);
		return;
	}

	AA_SwarmPet pet;
	pet.count = 1;
	pet.duration = 1;

	for(int x = 0; x < MAX_SWARM_PETS; x++)
	{
		if(spells[spell_id].effectid[x] == SE_TemporaryPets)
		{
			pet.count = spells[spell_id].base[x];
			pet.duration = spells[spell_id].max[x];
		}
	}

	pet.duration += GetFocusEffect(focusSwarmPetDuration, spell_id) / 1000;

	pet.npc_id = record.npc_type;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = database.LoadNPCTypesData(pet.npc_id);
	if(npc_type == nullptr) {
		//log write
		Log.Out(Logs::General, Logs::Error, "Unknown npc type for swarm pet spell id: %d", spell_id);
		Message(0,"Unable to find pet!");
		return;
	}

	if(name_override != nullptr) {
		//we have to make a custom NPC type for this name change
		made_npc = new NPCType;
		memcpy(made_npc, npc_type, sizeof(NPCType));
		strcpy(made_npc->name, name_override);
		npc_type = made_npc;
	}

	int summon_count = 0;
	summon_count = pet.count;

	if(summon_count > MAX_SWARM_PETS)
		summon_count = MAX_SWARM_PETS;

	static const glm::vec2 swarmPetLocations[MAX_SWARM_PETS] = {
		glm::vec2(5, 5), glm::vec2(-5, 5), glm::vec2(5, -5), glm::vec2(-5, -5),
		glm::vec2(10, 10), glm::vec2(-10, 10), glm::vec2(10, -10), glm::vec2(-10, -10),
		glm::vec2(8, 8), glm::vec2(-8, 8), glm::vec2(8, -8), glm::vec2(-8, -8)
	};

	while(summon_count > 0) {
		int pet_duration = pet.duration;
		if(duration_override > 0)
			pet_duration = duration_override;

		//this is a little messy, but the only way to do it right
		//it would be possible to optimize out this copy for the last pet, but oh well
		NPCType *npc_dup = nullptr;
		if(made_npc != nullptr) {
			npc_dup = new NPCType;
			memcpy(npc_dup, made_npc, sizeof(NPCType));
		}

		NPC* npca = new NPC(
				(npc_dup!=nullptr)?npc_dup:npc_type,	//make sure we give the NPC the correct data pointer
				0,
				GetPosition() + glm::vec4(swarmPetLocations[summon_count], 0.0f, 0.0f),
				FlyMode3);

		if (followme)
			npca->SetFollowID(GetID());

		if(!npca->GetSwarmInfo()){
			AA_SwarmPetInfo* nSI = new AA_SwarmPetInfo;
			npca->SetSwarmInfo(nSI);
			npca->GetSwarmInfo()->duration = new Timer(pet_duration*1000);
		}
		else{
			npca->GetSwarmInfo()->duration->Start(pet_duration*1000);
		}

		//removing this prevents the pet from attacking
		npca->GetSwarmInfo()->owner_id = GetID();

		//give the pets somebody to "love"
		if(targ != nullptr){
			npca->AddToHateList(targ, 1000, 1000);
			if (RuleB(Spells, SwarmPetTargetLock) || sticktarg)
				npca->GetSwarmInfo()->target = targ->GetID();
			else
				npca->GetSwarmInfo()->target = 0;
		}

		//we allocated a new NPC type object, give the NPC ownership of that memory
		if(npc_dup != nullptr)
			npca->GiveNPCTypeData(npc_dup);

		entity_list.AddNPC(npca, true, true);
		summon_count--;
	}

	//the target of these swarm pets will take offense to being cast on...
	if(targ != nullptr)
		targ->AddToHateList(this, 1, 0);

	// The other pointers we make are handled elsewhere.
	delete made_npc;
}

void Mob::TypesTemporaryPets(uint32 typesid, Mob *targ, const char *name_override, uint32 duration_override, bool followme, bool sticktarg) {

	AA_SwarmPet pet;
	pet.count = 1;
	pet.duration = 1;

	pet.npc_id = typesid;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = database.LoadNPCTypesData(typesid);
	if(npc_type == nullptr) {
		//log write
		Log.Out(Logs::General, Logs::Error, "Unknown npc type for swarm pet type id: %d", typesid);
		Message(0,"Unable to find pet!");
		return;
	}

	if(name_override != nullptr) {
		//we have to make a custom NPC type for this name change
		made_npc = new NPCType;
		memcpy(made_npc, npc_type, sizeof(NPCType));
		strcpy(made_npc->name, name_override);
		npc_type = made_npc;
	}

	int summon_count = 0;
	summon_count = pet.count;

	if(summon_count > MAX_SWARM_PETS)
		summon_count = MAX_SWARM_PETS;

	static const glm::vec2 swarmPetLocations[MAX_SWARM_PETS] = {
		glm::vec2(5, 5), glm::vec2(-5, 5), glm::vec2(5, -5), glm::vec2(-5, -5),
		glm::vec2(10, 10), glm::vec2(-10, 10), glm::vec2(10, -10), glm::vec2(-10, -10),
		glm::vec2(8, 8), glm::vec2(-8, 8), glm::vec2(8, -8), glm::vec2(-8, -8)
	};;

	while(summon_count > 0) {
		int pet_duration = pet.duration;
		if(duration_override > 0)
			pet_duration = duration_override;

		//this is a little messy, but the only way to do it right
		//it would be possible to optimize out this copy for the last pet, but oh well
		NPCType *npc_dup = nullptr;
		if(made_npc != nullptr) {
			npc_dup = new NPCType;
			memcpy(npc_dup, made_npc, sizeof(NPCType));
		}

		NPC* npca = new NPC(
				(npc_dup!=nullptr)?npc_dup:npc_type,	//make sure we give the NPC the correct data pointer
				0,
				GetPosition() + glm::vec4(swarmPetLocations[summon_count], 0.0f, 0.0f),
				FlyMode3);

		if (followme)
			npca->SetFollowID(GetID());

		if(!npca->GetSwarmInfo()){
			AA_SwarmPetInfo* nSI = new AA_SwarmPetInfo;
			npca->SetSwarmInfo(nSI);
			npca->GetSwarmInfo()->duration = new Timer(pet_duration*1000);
		}
		else{
			npca->GetSwarmInfo()->duration->Start(pet_duration*1000);
		}

		//removing this prevents the pet from attacking
		npca->GetSwarmInfo()->owner_id = GetID();

		//give the pets somebody to "love"
		if(targ != nullptr){
			npca->AddToHateList(targ, 1000, 1000);

			if (RuleB(Spells, SwarmPetTargetLock) || sticktarg)
				npca->GetSwarmInfo()->target = targ->GetID();
			else
				npca->GetSwarmInfo()->target = 0;
		}

		//we allocated a new NPC type object, give the NPC ownership of that memory
		if(npc_dup != nullptr)
			npca->GiveNPCTypeData(npc_dup);

		entity_list.AddNPC(npca, true, true);
		summon_count--;
	}

	// The other pointers we make are handled elsewhere.
	delete made_npc;
}

void Mob::WakeTheDead(uint16 spell_id, Mob *target, uint32 duration)
{
	Corpse *CorpseToUse = nullptr;
	CorpseToUse = entity_list.GetClosestCorpse(this, nullptr);

	if(!CorpseToUse)
		return;

	//assuming we have pets in our table; we take the first pet as a base type.
	const NPCType *base_type = database.LoadNPCTypesData(500);
	NPCType *make_npc = new NPCType;
	memcpy(make_npc, base_type, sizeof(NPCType));

	//combat stats
	make_npc->AC = ((GetLevel() * 7) + 550);
	make_npc->ATK = GetLevel();
	make_npc->max_dmg = (GetLevel() * 4) + 2;
	make_npc->min_dmg = 1;

	//base stats
	make_npc->cur_hp = (GetLevel() * 55);
	make_npc->max_hp = (GetLevel() * 55);
	make_npc->STR = 85 + (GetLevel() * 3);
	make_npc->STA = 85 + (GetLevel() * 3);
	make_npc->DEX = 85 + (GetLevel() * 3);
	make_npc->AGI = 85 + (GetLevel() * 3);
	make_npc->INT = 85 + (GetLevel() * 3);
	make_npc->WIS = 85 + (GetLevel() * 3);
	make_npc->CHA = 85 + (GetLevel() * 3);
	make_npc->MR = 25;
	make_npc->FR = 25;
	make_npc->CR = 25;
	make_npc->DR = 25;
	make_npc->PR = 25;

	//level class and gender
	make_npc->level = GetLevel();
	make_npc->class_ = CorpseToUse->class_;
	make_npc->race = CorpseToUse->race;
	make_npc->gender = CorpseToUse->gender;
	make_npc->loottable_id = 0;
	//name
	char NewName[64];
	sprintf(NewName, "%s`s Animated Corpse", GetCleanName());
	strcpy(make_npc->name, NewName);

	//appearance
	make_npc->beard = CorpseToUse->beard;
	make_npc->beardcolor = CorpseToUse->beardcolor;
	make_npc->eyecolor1 = CorpseToUse->eyecolor1;
	make_npc->eyecolor2 = CorpseToUse->eyecolor2;
	make_npc->haircolor = CorpseToUse->haircolor;
	make_npc->hairstyle = CorpseToUse->hairstyle;
	make_npc->helmtexture = CorpseToUse->helmtexture;
	make_npc->luclinface = CorpseToUse->luclinface;
	make_npc->size = CorpseToUse->size;
	make_npc->texture = CorpseToUse->texture;

	//cast stuff.. based off of PEQ's if you want to change
	//it you'll have to mod this code, but most likely
	//most people will be using PEQ style for the first
	//part of their spell list; can't think of any smooth
	//way to do this
	//some basic combat mods here too since it's convienent
	switch(CorpseToUse->class_)
	{
	case CLERIC:
		make_npc->npc_spells_id = 1;
		break;
	case WIZARD:
		make_npc->npc_spells_id = 2;
		break;
	case NECROMANCER:
		make_npc->npc_spells_id = 3;
		break;
	case MAGICIAN:
		make_npc->npc_spells_id = 4;
		break;
	case ENCHANTER:
		make_npc->npc_spells_id = 5;
		break;
	case SHAMAN:
		make_npc->npc_spells_id = 6;
		break;
	case DRUID:
		make_npc->npc_spells_id = 7;
		break;
	case PALADIN:
		//SPECATK_TRIPLE
		strcpy(make_npc->special_abilities, "6,1");
		make_npc->cur_hp = make_npc->cur_hp * 150 / 100;
		make_npc->max_hp = make_npc->max_hp * 150 / 100;
		make_npc->npc_spells_id = 8;
		break;
	case SHADOWKNIGHT:
		strcpy(make_npc->special_abilities, "6,1");
		make_npc->cur_hp = make_npc->cur_hp * 150 / 100;
		make_npc->max_hp = make_npc->max_hp * 150 / 100;
		make_npc->npc_spells_id = 9;
		break;
	case RANGER:
		strcpy(make_npc->special_abilities, "7,1");
		make_npc->cur_hp = make_npc->cur_hp * 135 / 100;
		make_npc->max_hp = make_npc->max_hp * 135 / 100;
		make_npc->npc_spells_id = 10;
		break;
	case BARD:
		strcpy(make_npc->special_abilities, "6,1");
		make_npc->cur_hp = make_npc->cur_hp * 110 / 100;
		make_npc->max_hp = make_npc->max_hp * 110 / 100;
		make_npc->npc_spells_id = 11;
		break;
	case BEASTLORD:
		strcpy(make_npc->special_abilities, "7,1");
		make_npc->cur_hp = make_npc->cur_hp * 110 / 100;
		make_npc->max_hp = make_npc->max_hp * 110 / 100;
		make_npc->npc_spells_id = 12;
		break;
	case ROGUE:
		strcpy(make_npc->special_abilities, "7,1");
		make_npc->max_dmg = make_npc->max_dmg * 150 /100;
		make_npc->cur_hp = make_npc->cur_hp * 110 / 100;
		make_npc->max_hp = make_npc->max_hp * 110 / 100;
		break;
	case MONK:
		strcpy(make_npc->special_abilities, "7,1");
		make_npc->max_dmg = make_npc->max_dmg * 150 /100;
		make_npc->cur_hp = make_npc->cur_hp * 135 / 100;
		make_npc->max_hp = make_npc->max_hp * 135 / 100;
		break;
	case WARRIOR:
	case BERSERKER:
		strcpy(make_npc->special_abilities, "7,1");
		make_npc->max_dmg = make_npc->max_dmg * 150 /100;
		make_npc->cur_hp = make_npc->cur_hp * 175 / 100;
		make_npc->max_hp = make_npc->max_hp * 175 / 100;
		break;
	default:
		make_npc->npc_spells_id = 0;
		break;
	}

	make_npc->loottable_id = 0;
	make_npc->merchanttype = 0;
	make_npc->d_melee_texture1 = 0;
	make_npc->d_melee_texture2 = 0;

	NPC* npca = new NPC(make_npc, 0, GetPosition(), FlyMode3);

	if(!npca->GetSwarmInfo()){
		AA_SwarmPetInfo* nSI = new AA_SwarmPetInfo;
		npca->SetSwarmInfo(nSI);
		npca->GetSwarmInfo()->duration = new Timer(duration*1000);
	}
	else{
		npca->GetSwarmInfo()->duration->Start(duration*1000);
	}

	npca->GetSwarmInfo()->owner_id = GetID();

	//give the pet somebody to "love"
	if(target != nullptr){
		npca->AddToHateList(target, 100000);
		npca->GetSwarmInfo()->target = target->GetID();
	}

	//gear stuff, need to make sure there's
	//no situation where this stuff can be duped
	for(int x = EmuConstants::EQUIPMENT_BEGIN; x <= EmuConstants::EQUIPMENT_END; x++) // (< 21) added MainAmmo
	{
		uint32 sitem = 0;
		sitem = CorpseToUse->GetWornItem(x);
		if(sitem){
			const Item_Struct * itm = database.GetItem(sitem);
			npca->AddLootDrop(itm, &npca->itemlist, 1, 1, 127, true, true);
		}
	}

	//we allocated a new NPC type object, give the NPC ownership of that memory
	if(make_npc != nullptr)
		npca->GiveNPCTypeData(make_npc);

	entity_list.AddNPC(npca, true, true);

	//the target of these swarm pets will take offense to being cast on...
	if(target != nullptr)
		target->AddToHateList(this, 1, 0);
}

//turn on an AA effect
//duration == 0 means no time limit, used for one-shot deals, etc..
void Client::EnableAAEffect(aaEffectType type, uint32 duration) {
	if(type > _maxaaEffectType)
		return;	//for now, special logic needed.
	m_epp.aa_effects |= 1 << (type-1);

	if(duration > 0) {
		p_timers.Start(pTimerAAEffectStart + type, duration);
	} else {
		p_timers.Clear(&database, pTimerAAEffectStart + type);
	}
}

void Client::DisableAAEffect(aaEffectType type) {
	if(type > _maxaaEffectType)
		return;	//for now, special logic needed.
	uint32 bit = 1 << (type-1);
	if(m_epp.aa_effects & bit) {
		m_epp.aa_effects ^= bit;
	}
	p_timers.Clear(&database, pTimerAAEffectStart + type);
}

/*
By default an AA effect is a one shot deal, unless
a duration timer is set.
*/
bool Client::CheckAAEffect(aaEffectType type) {
	if(type > _maxaaEffectType)
		return(false);	//for now, special logic needed.
	if(m_epp.aa_effects & (1 << (type-1))) {	//is effect enabled?
		//has our timer expired?
		if(p_timers.Expired(&database, pTimerAAEffectStart + type)) {
			DisableAAEffect(type);
			return(false);
		}
		return(true);
	}
	return(false);
}

void Client::SendAATimer(uint32 ability, uint32 begin, uint32 end) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AAAction,sizeof(UseAA_Struct));
	UseAA_Struct* uaaout = (UseAA_Struct*)outapp->pBuffer;
	uaaout->ability = ability;
	uaaout->begin = begin;
	uaaout->end = end;
	QueuePacket(outapp);
	safe_delete(outapp);
}

//sends all AA timers.
void Client::SendAATimers() {
	//we dont use SendAATimer because theres no reason to allocate the EQApplicationPacket every time
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AAAction,sizeof(UseAA_Struct));
	UseAA_Struct* uaaout = (UseAA_Struct*)outapp->pBuffer;

	PTimerList::iterator c,e;
	c = p_timers.begin();
	e = p_timers.end();
	for(; c != e; ++c) {
		PersistentTimer *cur = c->second;
		if(cur->GetType() < pTimerAAStart || cur->GetType() > pTimerAAEnd)
			continue;	//not an AA timer
		//send timer
		uaaout->begin = cur->GetStartTime();
		uaaout->end = static_cast<uint32>(time(nullptr));
		uaaout->ability = cur->GetType() - pTimerAAStart; // uuaaout->ability is really a shared timer number
		QueuePacket(outapp);
	}

	safe_delete(outapp);
}

void Client::ResetAA(){
//	RefundAA();
//	uint32 i;
//	for (i=0; i < MAX_PP_AA_ARRAY; i++) {
//		aa[i]->AA = 0;
//		aa[i]->value = 0;
//		aa[i]->charges = 0;
//		m_pp.aa_array[i].AA = 0;
//		m_pp.aa_array[i].value = 0;
//		m_pp.aa_array[i].charges= 0;
//	}
//
//	std::map<uint32,uint8>::iterator itr;
//	for(itr = aa_points.begin(); itr != aa_points.end(); ++itr)
//		aa_points[itr->first] = 0;
//
//	for(int i = 0; i < _maxLeaderAA; ++i)
//		m_pp.leader_abilities.ranks[i] = 0;
//
//	m_pp.group_leadership_points = 0;
//	m_pp.raid_leadership_points = 0;
//	m_pp.group_leadership_exp = 0;
//	m_pp.raid_leadership_exp = 0;
//
//	database.DeleteCharacterAAs(this->CharacterID());
//	SaveAA();
//	SendClearAA();
//	SendAAList();
//	SendAATable();
//	SendAAStats();
//	database.DeleteCharacterLeadershipAAs(this->CharacterID());
//	// undefined for these clients
//	if (GetClientVersionBit() & BIT_TitaniumAndEarlier)
//		Kick();
}

void Client::SendClearAA()
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ClearLeadershipAbilities, 0);
	FastQueuePacket(&outapp);
	outapp = new EQApplicationPacket(OP_ClearAA, 0);
	FastQueuePacket(&outapp);
}

int Client::GroupLeadershipAAHealthEnhancement()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAHealthEnhancement, group_id)) {
			case 1:
				bonus = 30;
				break;
			case 2:
				bonus = 60;
				break;
			case 3:
				bonus = 100;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAHealthEnhancement)) {
			case 1:
				bonus += 30;
				break;
			case 2:
				bonus += 60;
				break;
			case 3:
				bonus += 100;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}

	return 0;
}

int Client::GroupLeadershipAAManaEnhancement()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAManaEnhancement, group_id)) {
			case 1:
				bonus = 30;
				break;
			case 2:
				bonus = 60;
				break;
			case 3:
				bonus = 100;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAManaEnhancement)) {
			case 1:
				bonus += 30;
				break;
			case 2:
				bonus += 60;
				break;
			case 3:
				bonus += 100;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAManaEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 30;
		case 2:
			return 60;
		case 3:
			return 100;
	}

	return 0;
}

int Client::GroupLeadershipAAHealthRegeneration()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAHealthRegeneration, group_id)) {
			case 1:
				bonus = 4;
				break;
			case 2:
				bonus = 6;
				break;
			case 3:
				bonus = 8;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAHealthRegeneration)) {
			case 1:
				bonus += 4;
				break;
			case 2:
				bonus += 6;
				break;
			case 3:
				bonus += 8;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthRegeneration))
	{
		case 0:
			return 0;
		case 1:
			return 4;
		case 2:
			return 6;
		case 3:
			return 8;
	}

	return 0;
}

int Client::GroupLeadershipAAOffenseEnhancement()
{
	if (IsRaidGrouped()) {
		int bonus = 0;
		Raid *raid = GetRaid();
		if (!raid)
			return 0;
		uint32 group_id = raid->GetGroup(this);
		if (group_id < 12 && raid->GroupCount(group_id) >= 3) {
			switch (raid->GetLeadershipAA(groupAAOffenseEnhancement, group_id)) {
			case 1:
				bonus = 10;
				break;
			case 2:
				bonus = 19;
				break;
			case 3:
				bonus = 28;
				break;
			case 4:
				bonus = 34;
				break;
			case 5:
				bonus = 40;
				break;
			}
		}
		if (raid->RaidCount() >= 18) {
			switch (raid->GetLeadershipAA(raidAAOffenseEnhancement)) {
			case 1:
				bonus += 10;
				break;
			case 2:
				bonus += 19;
				break;
			case 3:
				bonus += 28;
				break;
			case 4:
				bonus += 34;
				break;
			case 5:
				bonus += 40;
				break;
			}
		}
		return bonus;
	}

	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAOffenseEnhancement))
	{
		case 0:
			return 0;
		case 1:
			return 10;
		case 2:
			return 19;
		case 3:
			return 28;
		case 4:
			return 34;
		case 5:
			return 40;
	}
	return 0;
}

void Client::InspectBuffs(Client* Inspector, int Rank)
{
	// At some point the removed the restriction of being a group member for this to work
	// not sure when, but the way it's coded now, it wouldn't work with mobs.
	if (!Inspector || Rank == 0)
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_InspectBuffs, sizeof(InspectBuffs_Struct));
	InspectBuffs_Struct *ib = (InspectBuffs_Struct *)outapp->pBuffer;

	uint32 buff_count = GetMaxTotalSlots();
	uint32 packet_index = 0;
	for (uint32 i = 0; i < buff_count; i++) {
		if (buffs[i].spellid == SPELL_UNKNOWN)
			continue;
		ib->spell_id[packet_index] = buffs[i].spellid;
		if (Rank > 1)
			ib->tics_remaining[packet_index] = spells[buffs[i].spellid].buffdurationformula == DF_Permanent ? 0xFFFFFFFF : buffs[i].ticsremaining;
		packet_index++;
	}

	Inspector->FastQueuePacket(&outapp);
}

void Client::DurationRampage(uint32 duration)
{
	if(duration) {
		m_epp.aa_effects |= 1 << (aaEffectRampage-1);
		p_timers.Start(pTimerAAEffectStart + aaEffectRampage, duration);
	}
}

void Client::RefundAA() {
//	int cur = 0;
//	bool refunded = false;
//
//	for(int x = 0; x < aaHighestID; x++) {
//		cur = GetAA(x);
//		if(cur > 0){
//			SendAA_Struct* curaa = zone->FindAA(x);
//			if(cur){
//				SetAA(x, 0);
//				for(int j = 0; j < cur; j++) {
//					m_pp.aapoints += curaa->cost + (curaa->cost_inc * j);
//					refunded = true;
//				}
//			}
//			else
//			{
//				m_pp.aapoints += cur;
//				SetAA(x, 0);
//				refunded = true;
//			}
//		}
//	}
//
//	if(refunded) {
//		SaveAA();
//		Save();
//		// Kick();
//	}
}

AA_SwarmPetInfo::AA_SwarmPetInfo()
{
	target = 0;
	owner_id = 0;
	duration = nullptr;
}

AA_SwarmPetInfo::~AA_SwarmPetInfo()
{
	target = 0;
	owner_id = 0;
	safe_delete(duration);
}

Mob *AA_SwarmPetInfo::GetOwner()
{
	return entity_list.GetMobID(owner_id);
}

//New AA
void Client::SendAlternateAdvancementTable() {
	for(auto &aa : zone->aa_abilities) {
		auto ranks = GetAA(aa.second->first_rank_id);
		if(ranks) {
			if(aa.second->GetMaxLevel() == ranks) {
				SendAlternateAdvancementRank(aa.first, ranks);
			} else {
				SendAlternateAdvancementRank(aa.first, ranks);
				SendAlternateAdvancementRank(aa.first, ranks + 1);
			}
		} else {
			SendAlternateAdvancementRank(aa.first, 1);
		}
	}
}

void Client::SendAlternateAdvancementRank(int aa_id, int level) {
	if(!zone)
		return;
	
	AA::Ability *ability = zone->GetAlternateAdvancementAbility(aa_id);

	if(!ability)
		return;

	if(!(ability->classes & (1 << GetClass()))) {
		return;
	}

	AA::Rank *rank = ability->GetRankByPointsSpent(level);
	if(!rank)
		return;

	if(!CanUseAlternateAdvancementRank(rank)) {
		return;
	}
	
	int size = sizeof(AARankInfo_Struct) + (sizeof(AARankEffect_Struct) * rank->effects.size()) + (sizeof(AARankPrereq_Struct) * rank->prereqs.size());
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendAATable, size);
	AARankInfo_Struct *aai = (AARankInfo_Struct*)outapp->pBuffer;

	aai->id = rank->id;
	aai->upper_hotkey_sid = rank->upper_hotkey_sid;
	aai->lower_hotkey_sid = rank->lower_hotkey_sid;
	aai->title_sid = rank->title_sid;
	aai->desc_sid = rank->desc_sid;
	aai->cost = rank->cost;
	aai->seq = aa_id;
	aai->type = ability->type;
	aai->spell = rank->spell;
	aai->spell_type = rank->spell_type;
	aai->spell_refresh = rank->recast_time;
	aai->classes = ability->classes;
	aai->level_req = rank->level_req;
	aai->current_level = level;
	aai->max_level = ability->GetMaxLevel();
	aai->prev_id = rank->prev_id;

	if(rank->next && !CanUseAlternateAdvancementRank(rank->next)) {
		aai->next_id = -1;
	} else {
		aai->next_id = rank->next_id;
	}
	aai->total_cost = rank->total_cost;
	aai->expansion = rank->expansion;
	aai->category = ability->category;
	aai->charges = ability->charges;
	aai->grant_only = ability->grant_only;
	aai->total_effects = rank->effects.size();
	aai->total_prereqs = rank->prereqs.size();

	outapp->SetWritePosition(sizeof(AARankInfo_Struct));
	for(auto &effect : rank->effects) {
		outapp->WriteSInt32(effect.effect_id);
		outapp->WriteSInt32(effect.base1);
		outapp->WriteSInt32(effect.base2);
		outapp->WriteSInt32(effect.slot);
	}

	for(auto &prereq : rank->prereqs) {
		outapp->WriteSInt32(prereq.aa_id);
		outapp->WriteSInt32(prereq.points);
	}

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendAlternateAdvancementStats() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AAExpUpdate, sizeof(AltAdvStats_Struct));
	AltAdvStats_Struct *aps = (AltAdvStats_Struct *)outapp->pBuffer;
	aps->experience = m_pp.expAA;
	aps->experience = (uint32)(((float)330.0f * (float)m_pp.expAA) / (float)max_AAXP);
	aps->unspent = m_pp.aapoints;
	aps->percentage = m_epp.perAA;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendAlternateAdvancementPoints() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_RespondAA, sizeof(AATable_Struct));
	AATable_Struct* aa2 = (AATable_Struct *)outapp->pBuffer;

	int i = 0;
	for(auto &aa : zone->aa_abilities) {
		uint32 charges = 0;
		auto ranks = GetAA(aa.second->first_rank_id, &charges);
		if(ranks) {
			AA::Rank *rank = aa.second->GetRankByPointsSpent(ranks);
			if(rank) {
				aa2->aa_list[i].AA = rank->id;
				aa2->aa_list[i].value = ranks;
				aa2->aa_list[i].charges = charges;
				i++;
			}
		}
	}


	aa2->aa_spent = GetSpentAA();
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::PurchaseAlternateAdvancementRank(int rank_id) {
	AA::Rank *rank = zone->GetAlternateAdvancementRank(rank_id);
	if(!rank) {
		return;
	}

	if(!rank->base_ability) {
		return;
	}

	if(!CanPurchaseAlternateAdvancementRank(rank, true)) {
		return;
	}
	
	if(rank->base_ability->charges > 0) {
		SetAA(rank_id, rank->current_value, rank->base_ability->charges);
	} else {
		SetAA(rank_id, rank->current_value, 0);

		//if not max then send next aa
		if(rank->next) {
			SendAlternateAdvancementRank(rank->base_ability->id, rank->next->current_value);
		}
	}

	m_pp.aapoints -= rank->cost;
	SaveAA();

	SendAlternateAdvancementPoints();
	SendAlternateAdvancementStats();

	if(rank->prev) {
		Message_StringID(15, AA_IMPROVE, 
						 std::to_string(rank->title_sid).c_str(), 
						 std::to_string(rank->prev->current_value).c_str(), 
						 std::to_string(rank->cost).c_str(), 
						 std::to_string(AA_POINTS).c_str());

		//QS stuff broke with new aa, todo: fix later
		/* QS: Player_Log_AA_Purchases */
		//		if (RuleB(QueryServ, PlayerLogAAPurchases)){
		//			std::string event_desc = StringFormat("Ranked AA Purchase :: aa_name:%s aa_id:%i at cost:%i in zoneid:%i instid:%i", aa2->name, aa2->id, real_cost, this->GetZoneID(), this->GetInstanceID());
		//			QServ->PlayerLogEvent(Player_Log_AA_Purchases, this->CharacterID(), event_desc);
		//		}
	} else {
		Message_StringID(15, AA_GAIN_ABILITY, 
						 std::to_string(rank->title_sid).c_str(), 
						 std::to_string(rank->cost).c_str(), 
						 std::to_string(AA_POINTS).c_str());
		//QS stuff broke with new aa, todo: fix later
		/* QS: Player_Log_AA_Purchases */
		//		if (RuleB(QueryServ, PlayerLogAAPurchases)){
		//			std::string event_desc = StringFormat("Initial AA Purchase :: aa_name:%s aa_id:%i at cost:%i in zoneid:%i instid:%i", aa2->name, aa2->id, real_cost, this->GetZoneID(), this->GetInstanceID());
		//			QServ->PlayerLogEvent(Player_Log_AA_Purchases, this->CharacterID(), event_desc);
		//		}
	}

	CalcBonuses();
	if(title_manager.IsNewAATitleAvailable(m_pp.aapoints_spent, GetBaseClass()))
		NotifyNewTitlesAvailable();
}

void Client::IncrementAlternateAdvancementRank(int rank_id) {
	AA::Rank *rank = zone->GetAlternateAdvancementRank(rank_id);
	if(!rank) {
		return;
	}

	if(!rank->base_ability) {
		return;
	}

	if(!CanPurchaseAlternateAdvancementRank(rank, false)) {
		return;
	}

	if(rank->base_ability->charges > 0) {
		SetAA(rank_id, rank->current_value, rank->base_ability->charges);
	}
	else {
		SetAA(rank_id, rank->current_value, 0);

		//if not max then send next aa
		if(rank->next) {
			SendAlternateAdvancementRank(rank->base_ability->id, rank->next->current_value);
		}
	}

	SaveAA();

	SendAlternateAdvancementPoints();
	SendAlternateAdvancementStats();

	if(rank->prev) {
		Message_StringID(15, AA_IMPROVE,
						 std::to_string(rank->title_sid).c_str(),
						 std::to_string(rank->prev->current_value).c_str(),
						 std::to_string(rank->cost).c_str(),
						 std::to_string(AA_POINTS).c_str());

		//QS stuff broke with new aa, todo: fix later
		/* QS: Player_Log_AA_Purchases */
		//		if (RuleB(QueryServ, PlayerLogAAPurchases)){
		//			std::string event_desc = StringFormat("Ranked AA Purchase :: aa_name:%s aa_id:%i at cost:%i in zoneid:%i instid:%i", aa2->name, aa2->id, real_cost, this->GetZoneID(), this->GetInstanceID());
		//			QServ->PlayerLogEvent(Player_Log_AA_Purchases, this->CharacterID(), event_desc);
		//		}
	}
	else {
		Message_StringID(15, AA_GAIN_ABILITY,
						 std::to_string(rank->title_sid).c_str(),
						 std::to_string(rank->cost).c_str(),
						 std::to_string(AA_POINTS).c_str());
		//QS stuff broke with new aa, todo: fix later
		/* QS: Player_Log_AA_Purchases */
		//		if (RuleB(QueryServ, PlayerLogAAPurchases)){
		//			std::string event_desc = StringFormat("Initial AA Purchase :: aa_name:%s aa_id:%i at cost:%i in zoneid:%i instid:%i", aa2->name, aa2->id, real_cost, this->GetZoneID(), this->GetInstanceID());
		//			QServ->PlayerLogEvent(Player_Log_AA_Purchases, this->CharacterID(), event_desc);
		//		}
	}

	CalcBonuses();
}

bool ZoneDatabase::LoadAlternateAdvancement(Client *c) {
	c->ClearAAs();
	std::string query = StringFormat(
		"SELECT								"
		"aa_id,								"
		"aa_value,							"
		"charges							"
		"FROM								"
		"`character_alternate_abilities`    "
		"WHERE `id` = %u ORDER BY `slot`", c->CharacterID());
	MySQLRequestResult results = database.QueryDatabase(query);

	int i = 0;
	for(auto row = results.begin(); row != results.end(); ++row) {
		uint32 aa = atoi(row[0]);
		uint32 value = atoi(row[1]);
		uint32 charges = atoi(row[2]);

		c->GetPP().aa_array[i].AA = aa;
		c->GetPP().aa_array[i].value = value;
		c->GetPP().aa_array[i].charges = charges;
		c->SetAA(aa, value, charges);
		i++;
	}

	return true;
}

AA::Ability *Zone::GetAlternateAdvancementAbility(int id) {
	auto iter = aa_abilities.find(id);
	if(iter != aa_abilities.end()) {
		return iter->second.get();
	}

	return nullptr;
}

AA::Ability *Zone::GetAlternateAdvancementAbilityByRank(int rank_id) {
	AA::Rank *rank = GetAlternateAdvancementRank(rank_id);

	if(!rank)
		return nullptr;

	return rank->base_ability;
}

AA::Rank *Zone::GetAlternateAdvancementRank(int rank_id) {
	auto iter = aa_ranks.find(rank_id);
	if(iter != aa_ranks.end()) {
		return iter->second.get();
	}

	return nullptr;
}

uint32 Mob::GetAA(uint32 rank_id, uint32 *charges) const {
	if(zone) {
		AA::Ability *ability = zone->GetAlternateAdvancementAbilityByRank(rank_id);
		if(!ability)
			return 0;

		auto iter = aa_ranks.find(ability->id);
		if(iter != aa_ranks.end()) {
			if(charges) {
				*charges = iter->second.second;
			}
			return iter->second.first;
		}
	}
	return 0;
}

bool Mob::SetAA(uint32 rank_id, uint32 new_value, uint32 charges) {
	if(zone) {
		AA::Ability *ability = zone->GetAlternateAdvancementAbilityByRank(rank_id);

		if(!ability) {
			return false;
		}

		if(new_value > ability->GetMaxLevel()) {
			return false;
		}

		aa_ranks[ability->id] = std::make_pair(new_value, charges);
	}

	return true;
}


bool Mob::CanUseAlternateAdvancementRank(AA::Rank *rank) {
	AA::Ability *ability = rank->base_ability;

	if(!ability)
		return false;

	if(!ability->classes & (1 << GetClass())) {
		return false;
	}

	// Passive and Active Shroud AAs
	// For now we skip them
	if(ability->category == 3 || ability->category == 4) {
		return false;
	}

	//the one titanium hack i will allow
	//just to make sure we dont crash the client with newer aas
	//we'll exclude any expendable ones
	if(IsClient() && CastToClient()->GetClientVersionBit() & BIT_TitaniumAndEarlier) {
		if(ability->charges > 0) {
			return false;
		}
	}

	//I might add a races behind the scenes field to take care of this
	// Check for racial/Drakkin blood line AAs
	if(ability->category == 8)
	{
		uint32 client_race = GetBaseRace();

		// Drakkin Bloodlines
		if(rank->expansion > 522)
		{
			if(client_race != 522)
				return false;

			int heritage = this->GetDrakkinHeritage() + 523; // 523 = Drakkin Race(522) + Bloodline

			if(heritage != rank->expansion)
				return false;
		}
		else if(client_race != rank->expansion)
		{
			return false;
		}
	}

	return true;
}

bool Mob::CanPurchaseAlternateAdvancementRank(AA::Rank *rank, bool check_price) {
	AA::Ability *ability = rank->base_ability;

	if(!ability)
		return false;
	
	if(!CanUseAlternateAdvancementRank(rank)) {
		return false;
	}

	//You can't purchase grant only AAs they can only be assigned
	if(ability->grant_only) {
		return false;
	}

	if(!(RuleI(World, ExpansionSettings) & (1 << rank->expansion))) {
		return false;
	}

	//check level req
	if(rank->level_req > GetLevel()) {
		return false;
	}

	uint32 current_charges = 0;
	auto points = GetAA(rank->id, &current_charges);

	//check that we are on previous rank already (if exists)
	if(rank->prev) {
		if(points != rank->prev->current_value) {
			return false;
		}
	}

	//if expendable only let us purchase if we have no charges already
	//not quite sure on how this functions client side atm 
	//I intend to look into it later to make sure the behavior is right
	if(ability->charges > 0 && current_charges > 0) {
		return false;
	}

	//check prereqs
	for(auto &prereq : rank->prereqs) {
		AA::Ability *prereq_ability = zone->GetAlternateAdvancementAbility(prereq.aa_id);

		if(prereq_ability) {
			auto ranks = GetAA(prereq_ability->first_rank_id);
			if(ranks < prereq.points) {
				return false;
			}
		}
	}

	//check price, if client
	if(check_price && IsClient()) {
		if(rank->cost > CastToClient()->GetAAPoints()) {
			return false;
		}
	}

	return true;
}

void Zone::LoadAlternateAdvancement() {
	Log.Out(Logs::General, Logs::Status, "Loading Alternate Advancement Data...");
	if(!database.LoadAlternateAdvancementAbilities(aa_abilities,
		aa_ranks))
	{
		aa_abilities.clear();
		aa_ranks.clear();
		Log.Out(Logs::General, Logs::Status, "Failed to load Alternate Advancement Data");
		return;
	}

	Log.Out(Logs::General, Logs::Status, "Processing Alternate Advancement Data...");
	for(const auto &ability : aa_abilities) {
		ability.second->first = GetAlternateAdvancementRank(ability.second->first_rank_id);

		//process these ranks
		AA::Rank *current = ability.second->first;
		int i = 1;
		while(current) {
			current->prev = GetAlternateAdvancementRank(current->prev_id);
			current->next = GetAlternateAdvancementRank(current->next_id);
			current->base_ability = ability.second.get();
			current->current_value = i;

			if(current->prev) {
				current->total_cost = current->cost + current->prev->total_cost;
			}
			else {
				current->total_cost = current->cost;
			}

			i++;
			current = current->next;
		}

		ability.second->GetMaxLevel(true);
	}

	Log.Out(Logs::General, Logs::Status, "Loaded Alternate Advancement Data");
}

bool ZoneDatabase::LoadAlternateAdvancementAbilities(std::unordered_map<int, std::unique_ptr<AA::Ability>> &abilities,
													std::unordered_map<int, std::unique_ptr<AA::Rank>> &ranks) 
{
	Log.Out(Logs::General, Logs::Status, "Loading Alternate Advancement Abilities...");
	abilities.clear();
	std::string query = "SELECT id, name, category, classes, type, charges, grant_only, first_rank_id FROM aa_ability";
	auto results = QueryDatabase(query);
	if(results.Success()) {
		for(auto row = results.begin(); row != results.end(); ++row) {
			AA::Ability *ability = new AA::Ability;
			ability->id = atoi(row[0]);
			ability->name = row[1];
			ability->category = atoi(row[2]);
			ability->classes = atoi(row[3]);
			ability->type = atoi(row[4]);
			ability->charges = atoi(row[5]);
			ability->grant_only = atoi(row[6]) != 0 ? true : false;
			ability->first_rank_id = atoi(row[7]);
			ability->first = nullptr;

			abilities[ability->id] = std::unique_ptr<AA::Ability>(ability);
		}
	} else {
		Log.Out(Logs::General, Logs::Error, "Failed to load Alternate Advancement Abilities");
		return false;
	}

	Log.Out(Logs::General, Logs::Status, "Loaded %d Alternate Advancement Abilities", (int)abilities.size());

	Log.Out(Logs::General, Logs::Status, "Loading Alternate Advancement Ability Ranks...");
	ranks.clear();
	query = "SELECT id, upper_hotkey_sid, lower_hotkey_sid, title_sid, desc_sid, cost, level_req, spell, spell_type, recast_time, "
		"prev_id, next_id, expansion, account_time_required FROM aa_ranks";
	results = QueryDatabase(query);
	if(results.Success()) {
		for(auto row = results.begin(); row != results.end(); ++row) {
			AA::Rank *rank = new AA::Rank;
			rank->id = atoi(row[0]);
			rank->upper_hotkey_sid = atoi(row[1]);
			rank->lower_hotkey_sid = atoi(row[2]);
			rank->title_sid = atoi(row[3]);
			rank->desc_sid = atoi(row[4]);
			rank->cost = atoi(row[5]);
			rank->level_req = atoi(row[6]);
			rank->spell = atoi(row[7]);
			rank->spell_type = atoi(row[8]);
			rank->recast_time = atoi(row[9]);
			rank->prev_id = atoi(row[10]);
			rank->next_id = atoi(row[11]);
			rank->expansion = atoi(row[12]);
			rank->account_time_required = atoul(row[13]);
			rank->base_ability = nullptr;
			rank->total_cost = 0;
			rank->next = nullptr;
			rank->prev = nullptr;

			ranks[rank->id] = std::unique_ptr<AA::Rank>(rank);
		}
	} else {
		Log.Out(Logs::General, Logs::Error, "Failed to load Alternate Advancement Ability Ranks");
		return false;
	}

	Log.Out(Logs::General, Logs::Status, "Loaded %d Alternate Advancement Ability Ranks", (int)ranks.size());

	Log.Out(Logs::General, Logs::Status, "Loading Alternate Advancement Ability Rank Effects...");
	query = "SELECT rank_id, slot, effect_id, base1, base2 FROM aa_rank_effects";
	results = QueryDatabase(query);
	if(results.Success()) {
		for(auto row = results.begin(); row != results.end(); ++row) {
			AA::RankEffect effect;
			int rank_id = atoi(row[0]);
			effect.slot = atoi(row[1]);
			effect.effect_id = atoi(row[2]);
			effect.base1 = atoi(row[3]);
			effect.base2 = atoi(row[4]);

			if(effect.slot < 1)
				continue;

			if(ranks.count(rank_id) > 0) {
				AA::Rank *rank = ranks[rank_id].get();
				rank->effects.push_back(effect);
			}
		}
	} else {
		Log.Out(Logs::General, Logs::Error, "Failed to load Alternate Advancement Ability Rank Effects");
		return false;
	}

	Log.Out(Logs::General, Logs::Status, "Loaded Alternate Advancement Ability Rank Effects");

	Log.Out(Logs::General, Logs::Status, "Loading Alternate Advancement Ability Rank Prereqs...");
	query = "SELECT rank_id, aa_id, points FROM aa_rank_prereqs";
	results = QueryDatabase(query);
	if(results.Success()) {
		for(auto row = results.begin(); row != results.end(); ++row) {
			AA::RankPrereq prereq;
			int rank_id = atoi(row[0]);
			prereq.aa_id = atoi(row[1]);
			prereq.points = atoi(row[2]);

			if(ranks.count(rank_id) > 0) {
				AA::Rank *rank = ranks[rank_id].get();
				rank->prereqs.push_back(prereq);
			}
		}
	} else {
		Log.Out(Logs::General, Logs::Error, "Failed to load Alternate Advancement Ability Rank Prereqs");
		return false;
	}

	Log.Out(Logs::General, Logs::Status, "Loaded Alternate Advancement Ability Rank Prereqs");

	return true;
}
