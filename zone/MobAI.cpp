/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemu.org)

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
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "npc.h"
#include "masterentity.h"
#include "NpcAI.h"
#include "map.h"
#include "../common/moremath.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/StringUtil.h"
#include "../common/rulesys.h"
#include "../common/features.h"
#include "QuestParserCollection.h"
#include "watermap.h"

extern EntityList entity_list;

extern Zone *zone;

#ifdef _EQDEBUG
	#define MobAI_DEBUG_Spells	-1
#else
	#define MobAI_DEBUG_Spells	-1
#endif
#define ABS(x) ((x)<0?-(x):(x))

//NOTE: do NOT pass in beneficial and detrimental spell types into the same call here!
bool NPC::AICastSpell(Mob* tar, uint8 iChance, uint16 iSpellTypes) {
	if (!tar)
		return false;

	if (IsNoCast())
		return false;

	if(AI_HasSpells() == false)
		return false;

	if (iChance < 100) {
		if (MakeRandomInt(0, 100) >= iChance)
			return false;
	}

	float dist2;

	if (iSpellTypes & SpellType_Escape) {
		dist2 = 0; //DistNoRoot(*this);	//WTF was up with this...
	}
	else
		dist2 = DistNoRoot(*tar);

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.

	float manaR = GetManaRatio();
	for (int i = static_cast<int>(AIspells.size()) - 1; i >= 0; i--) {
		if (AIspells[i].spellid <= 0 || AIspells[i].spellid >= SPDAT_RECORDS) {
			// this is both to quit early to save cpu and to avoid casting bad spells
			// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
			//return false;
			continue;
		}
		if (iSpellTypes & AIspells[i].type) {
			// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
			int32 mana_cost = AIspells[i].manacost;
			if (mana_cost == -1)
				mana_cost = spells[AIspells[i].spellid].mana;
			else if (mana_cost == -2)
				mana_cost = 0;
			if (
				((
					(spells[AIspells[i].spellid].targettype==ST_AECaster || spells[AIspells[i].spellid].targettype==ST_AEBard)
					&& dist2 <= spells[AIspells[i].spellid].aoerange*spells[AIspells[i].spellid].aoerange
				) ||
				dist2 <= spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range
				)
				&& (mana_cost <= GetMana() || GetMana() == GetMaxMana())
				&& (AIspells[i].time_cancast + (MakeRandomInt(0, 4) * 1000)) <= Timer::GetCurrentTime() //break up the spelling casting over a period of time.
				) {

#if MobAI_DEBUG_Spells >= 21
				std::cout << "Mob::AICastSpell: Casting: spellid=" << AIspells[i].spellid
					<< ", tar=" << tar->GetName()
					<< ", dist2[" << dist2 << "]<=" << spells[AIspells[i].spellid].range *spells[AIspells[i].spellid].range
					<< ", mana_cost[" << mana_cost << "]<=" << GetMana()
					<< ", cancast[" << AIspells[i].time_cancast << "]<=" << Timer::GetCurrentTime()
					<< ", type=" << AIspells[i].type << std::endl;
#endif

				switch (AIspells[i].type) {
					case SpellType_Heal: {
						if (
							(spells[AIspells[i].spellid].targettype == ST_Target || tar == this)
							&& tar->DontHealMeBefore() < Timer::GetCurrentTime()
							&& !(tar->IsPet() && tar->GetOwner()->IsClient())	//no buffing PC's pets
							) {
							uint8 hpr = (uint8)tar->GetHPRatio();

							if(hpr <= 35 || (!IsEngaged() && hpr <= 50) || (tar->IsClient() && hpr <= 99)) {
								uint32 tempTime = 0;
								AIDoSpellCast(i, tar, mana_cost, &tempTime);
								tar->SetDontHealMeBefore(tempTime);
								return true;
							}
						}
						break;
					}
					case SpellType_Root: {
						Mob *rootee = GetHateRandom();
						if (rootee && !rootee->IsRooted() && MakeRandomInt(0, 99) < 50
							&& rootee->DontRootMeBefore() < Timer::GetCurrentTime()
							&& rootee->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(rootee))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, rootee, mana_cost, &tempTime);
							rootee->SetDontRootMeBefore(tempTime);
							return true;
						}
						break;
					}
					case SpellType_Buff: {
						if (
							(spells[AIspells[i].spellid].targettype == ST_Target || tar == this)
							&& tar->DontBuffMeBefore() < Timer::GetCurrentTime()
							&& !tar->IsImmuneToSpell(AIspells[i].spellid, this)
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							&& !(tar->IsPet() && tar->GetOwner()->IsClient() && this != tar)	//no buffing PC's pets, but they can buff themself
							)
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, tar, mana_cost, &tempTime);
							tar->SetDontBuffMeBefore(tempTime);
							return true;
						}
						break;
					}

					case SpellType_InCombatBuff: {
						if(MakeRandomInt(0, 99) < 50)
						{
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}

					case SpellType_Escape: {
						if (GetHPRatio() <= 5 )
						{
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Slow:
					case SpellType_Debuff: {
						Mob * debuffee = GetHateRandom();
						if (debuffee && manaR >= 10 && MakeRandomInt(0, 99 < 70) &&
								debuffee->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0) {
							if (!checked_los) {
								if (!CheckLosFN(debuffee))
									return false;
								checked_los = true;
							}
							AIDoSpellCast(i, debuffee, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Nuke: {
						if (
							manaR >= 10 && MakeRandomInt(0, 99) < 70
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Dispel: {
						if(MakeRandomInt(0, 99) < 15)
						{
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							if(tar->CountDispellableBuffs() > 0)
							{
								AIDoSpellCast(i, tar, mana_cost);
								return true;
							}
						}
						break;
					}
					case SpellType_Mez: {
						if(MakeRandomInt(0, 99) < 20)
						{
							Mob * mezTar = nullptr;
							mezTar = entity_list.GetTargetForMez(this);

							if(mezTar && mezTar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0)
							{
								AIDoSpellCast(i, mezTar, mana_cost);
								return true;
							}
						}
						break;
					}

					case SpellType_Charm:
					{
						if(!IsPet() && MakeRandomInt(0, 99) < 20)
						{
							Mob * chrmTar = GetHateRandom();
							if(chrmTar && chrmTar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0)
							{
								AIDoSpellCast(i, chrmTar, mana_cost);
								return true;
							}
						}
						break;
					}

					case SpellType_Pet: {
						//keep mobs from recasting pets when they have them.
						if (!IsPet() && !GetPetID() && MakeRandomInt(0, 99) < 25) {
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Lifetap: {
						if (GetHPRatio() <= 95
							&& MakeRandomInt(0, 99) < 50
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Snare: {
						if (
							!tar->IsRooted()
							&& MakeRandomInt(0, 99) < 50
							&& tar->DontSnareMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, tar, mana_cost, &tempTime);
							tar->SetDontSnareMeBefore(tempTime);
							return true;
						}
						break;
					}
					case SpellType_DOT: {
						if (
							MakeRandomInt(0, 99) < 60
							&& tar->DontDotMeBefore() < Timer::GetCurrentTime()
							&& tar->CanBuffStack(AIspells[i].spellid, GetLevel(), true) >= 0
							) {
							if(!checked_los) {
								if(!CheckLosFN(tar))
									return(false);	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call
								checked_los = true;
							}
							uint32 tempTime = 0;
							AIDoSpellCast(i, tar, mana_cost, &tempTime);
							tar->SetDontDotMeBefore(tempTime);
							return true;
						}
						break;
					}
					default: {
						std::cout << "Error: Unknown spell type in AICastSpell. caster:" << this->GetName() << " type:" << AIspells[i].type << " slot:" << i << std::endl;
						break;
					}
				}
			}
#if MobAI_DEBUG_Spells >= 21
			else {
				std::cout << "Mob::AICastSpell: NotCasting: spellid=" << AIspells[i].spellid << ", tar=" << tar->GetName() << ", dist2[" << dist2 << "]<=" << spells[AIspells[i].spellid].range*spells[AIspells[i].spellid].range << ", mana_cost[" << mana_cost << "]<=" << GetMana() << ", cancast[" << AIspells[i].time_cancast << "]<=" << Timer::GetCurrentTime() << std::endl;
			}
#endif
		}
	}
	return false;
}

bool NPC::AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore) {
#if MobAI_DEBUG_Spells >= 1
	std::cout << "Mob::AIDoSpellCast: spellid=" << AIspells[i].spellid << ", tar=" << tar->GetName() << ", mana=" << mana_cost << ", Name: " << spells[AIspells[i].spellid].name << std::endl;
#endif
	casting_spell_AIindex = i;

	//stop moving if were casting a spell and were not a bard...
	if(!IsBardSong(AIspells[i].spellid)) {
		SetRunAnimSpeed(0);
		SendPosition();
		SetMoving(false);
	}

	return CastSpell(AIspells[i].spellid, tar->GetID(), 1, AIspells[i].manacost == -2 ? 0 : -1, mana_cost, oDontDoAgainBefore, -1, -1, 0, 0, &(AIspells[i].resist_adjust));
}

bool EntityList::AICheckCloseBeneficialSpells(NPC* caster, uint8 iChance, float iRange, uint16 iSpellTypes) {
	if((iSpellTypes&SpellTypes_Detrimental) != 0) {
		//according to live, you can buff and heal through walls...
		//now with PCs, this only applies if you can TARGET the target, but
		// according to Rogean, Live NPCs will just cast through walls/floors, no problem..
		//
		// This check was put in to address an idle-mob CPU issue
		_log(AI__ERROR, "Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
		return(false);
	}

	if(!caster)
		return false;

	if(caster->AI_HasSpells() == false)
		return false;

	if(caster->GetSpecialAbility(NPC_NO_BUFFHEAL_FRIENDS))
		return false;

	if (iChance < 100) {
		uint8 tmp = MakeRandomInt(0, 99);
		if (tmp >= iChance)
			return false;
	}
	if (caster->GetPrimaryFaction() == 0 )
		return(false); // well, if we dont have a faction set, we're gonna be indiff to everybody

	float iRange2 = iRange*iRange;

	float t1, t2, t3;


	//Only iterate through NPCs
	for (auto it = npc_list.begin(); it != npc_list.end(); ++it) {
		NPC* mob = it->second;

		//Since >90% of mobs will always be out of range, try to
		//catch them with simple bounding box checks first. These
		//checks are about 6X faster than DistNoRoot on my athlon 1Ghz
		t1 = mob->GetX() - caster->GetX();
		t2 = mob->GetY() - caster->GetY();
		t3 = mob->GetZ() - caster->GetZ();
		//cheap ABS()
		if(t1 < 0)
			t1 = 0 - t1;
		if(t2 < 0)
			t2 = 0 - t2;
		if(t3 < 0)
			t3 = 0 - t3;
		if (t1 > iRange
			|| t2 > iRange
			|| t3 > iRange
			|| mob->DistNoRoot(*caster) > iRange2
				//this call should seem backwards:
			|| !mob->CheckLosFN(caster)
			|| mob->GetReverseFactionCon(caster) >= FACTION_KINDLY
		) {
			continue;
		}

		//since we assume these are beneficial spells, which do not
		//require LOS, we just go for it.
		// we have a winner!
		if((iSpellTypes & SpellType_Buff) && !RuleB(NPC, BuffFriends)){
			if (mob != caster)
				iSpellTypes = SpellType_Heal;
		}

		if (caster->AICastSpell(mob, 100, iSpellTypes))
			return true;
	}
	return false;
}

void Mob::AI_Init() {
	pAIControlled = false;
	AIthink_timer = 0;
	AIwalking_timer = 0;
	AImovement_timer = 0;
	AItarget_check_timer = 0;
	AIfeignremember_timer = nullptr;
	AIscanarea_timer = 0;
	minLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMin);
	maxLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMax);

	pDontHealMeBefore = 0;
	pDontBuffMeBefore = 0;
	pDontDotMeBefore = 0;
	pDontRootMeBefore = 0;
	pDontSnareMeBefore = 0;
	pDontCureMeBefore = 0;
}

void NPC::AI_Init() {
	Mob::AI_Init();

	AIautocastspell_timer = 0;
	casting_spell_AIindex = static_cast<uint8>(AIspells.size());

	roambox_max_x = 0;
	roambox_max_y = 0;
	roambox_min_x = 0;
	roambox_min_y = 0;
	roambox_distance = 0;
	roambox_movingto_x = 0;
	roambox_movingto_y = 0;
	roambox_min_delay = 2500;
	roambox_delay = 2500;
}

void Client::AI_Init() {
	Mob::AI_Init();
	minLastFightingDelayMoving = CLIENT_LD_TIMEOUT;
	maxLastFightingDelayMoving = CLIENT_LD_TIMEOUT;
}

void Mob::AI_Start(uint32 iMoveDelay) {
	if (pAIControlled)
		return;

	if (iMoveDelay)
		pLastFightingDelayMoving = Timer::GetCurrentTime() + iMoveDelay;
	else
		pLastFightingDelayMoving = 0;

	pAIControlled = true;
	AIthink_timer = new Timer(AIthink_duration);
	AIthink_timer->Trigger();
	AIwalking_timer = new Timer(0);
	AImovement_timer = new Timer(AImovement_duration);
	AItarget_check_timer = new Timer(AItarget_check_duration);
	AIfeignremember_timer = new Timer(AIfeignremember_delay);
	AIscanarea_timer = new Timer(AIscanarea_delay);
#ifdef REVERSE_AGGRO
	if(IsNPC() && !CastToNPC()->WillAggroNPCs())
		AIscanarea_timer->Disable();
#endif

	if (GetAggroRange() == 0)
		pAggroRange = 70;
	if (GetAssistRange() == 0)
		pAssistRange = 70;
	hate_list.Wipe();

	delta_heading = 0;
	delta_x = 0;
	delta_y = 0;
	delta_z = 0;
	pRunAnimSpeed = 0;
	pLastChange = Timer::GetCurrentTime();
}

void Client::AI_Start(uint32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);

	if (!pAIControlled)
		return;

	pClientSideTarget = GetTarget() ? GetTarget()->GetID() : 0;
	SendAppearancePacket(AT_Anim, ANIM_FREEZE);	// this freezes the client
	SendAppearancePacket(AT_Linkdead, 1); // Sending LD packet so *LD* appears by the player name when charmed/feared -Kasai
	SetAttackTimer();
	SetFeigned(false);
}

void NPC::AI_Start(uint32 iMoveDelay) {
	Mob::AI_Start(iMoveDelay);
	if (!pAIControlled)
		return;

	if (AIspells.size() == 0) {
		AIautocastspell_timer = new Timer(1000);
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer = new Timer(750);
		AIautocastspell_timer->Start(RandomTimer(0, 15000), false);
	}

	if (NPCTypedata) {
		AI_AddNPCSpells(NPCTypedata->npc_spells_id);
		ProcessSpecialAbilities(NPCTypedata->special_abilities);
	}

	SendTo(GetX(), GetY(), GetZ());
	SetChanged();
	SaveGuardSpot();
}

void Mob::AI_Stop() {
	if (!IsAIControlled())
		return;
	pAIControlled = false;
	safe_delete(AIthink_timer);
	safe_delete(AIwalking_timer);
	safe_delete(AImovement_timer);
	safe_delete(AItarget_check_timer)
	safe_delete(AIscanarea_timer);
	safe_delete(AIfeignremember_timer);
	hate_list.Wipe();
}

void NPC::AI_Stop() {
	Waypoints.clear();
	safe_delete(AIautocastspell_timer);
}

void Client::AI_Stop() {
	Mob::AI_Stop();
	this->Message_StringID(13,PLAYER_REGAIN);

	EQApplicationPacket *app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
	Charm_Struct *ps = (Charm_Struct*)app->pBuffer;
	ps->owner_id = 0;
	ps->pet_id = this->GetID();
	ps->command = 0;
	entity_list.QueueClients(this, app);
	safe_delete(app);

	SetTarget(entity_list.GetMob(pClientSideTarget));
	SendAppearancePacket(AT_Anim, GetAppearanceValue(GetAppearance()));
	SendAppearancePacket(AT_Linkdead, 0); // Removing LD packet so *LD* no longer appears by the player name when charmed/feared -Kasai
	if (!auto_attack) {
		attack_timer.Disable();
		attack_dw_timer.Disable();
	}
	if (IsLD())
	{
		Save();
		Disconnect();
	}
}

//todo: expand the logic here to cover:
//redundant debuffs
//buffing owner
//certain types of det spells that need special behavior.
void Client::AI_SpellCast()
{
	if(!charm_cast_timer.Check())
		return;

	Mob *targ = GetTarget();
	if(!targ)
		return;

	float dist = DistNoRootNoZ(*targ);

	std::vector<uint32> valid_spells;
	std::vector<uint32> slots;

	for(uint32 x = 0; x < 9; ++x)
	{
		uint32 current_spell = m_pp.mem_spells[x];
		if(!IsValidSpell(current_spell))
			continue;

		if(IsBeneficialSpell(current_spell))
		{
			continue;
		}

		if(dist > spells[current_spell].range*spells[current_spell].range)
		{
			continue;
		}

		if(GetMana() < spells[current_spell].mana)
		{
			continue;
		}

		if(IsEffectInSpell(current_spell, SE_Charm))
		{
			continue;
		}

		if(!GetPTimers().Expired(&database, pTimerSpellStart + current_spell, false))
		{
			continue;
		}

		if(targ->CanBuffStack(current_spell, GetLevel(), true) < 0)
		{
			continue;
		}

		//bard songs cause trouble atm
		if(IsBardSong(current_spell))
			continue;

		valid_spells.push_back(current_spell);
		slots.push_back(x);
	}

	uint32 spell_to_cast = 0xFFFFFFFF;
	uint32 slot_to_use = 10;
	if(valid_spells.size() == 1)
	{
		spell_to_cast = valid_spells[0];
		slot_to_use = slots[0];
	}
	else if(valid_spells.size() == 0)
	{
		return;
	}
	else
	{
		uint32 idx = MakeRandomInt(0, (valid_spells.size()-1));
		spell_to_cast = valid_spells[idx];
		slot_to_use = slots[idx];
	}

	if(IsMezSpell(spell_to_cast) || IsFearSpell(spell_to_cast))
	{
		Mob *tar = entity_list.GetTargetForMez(this);
		if(!tar)
		{
			tar = GetTarget();
			if(tar && IsFearSpell(spell_to_cast))
			{
				if(!IsBardSong(spell_to_cast))
				{
					SetRunAnimSpeed(0);
					SendPosition();
					SetMoving(false);
				}
				CastSpell(spell_to_cast, tar->GetID(), slot_to_use);
				return;
			}
		}
	}
	else
	{
		Mob *tar = GetTarget();
		if(tar)
		{
			if(!IsBardSong(spell_to_cast))
			{
				SetRunAnimSpeed(0);
				SendPosition();
				SetMoving(false);
			}
			CastSpell(spell_to_cast, tar->GetID(), slot_to_use);
			return;
		}
	}


}

void Client::AI_Process()
{
	if (!IsAIControlled())
		return;

	if (!(AIthink_timer->Check() || attack_timer.Check(false)))
		return;

	if (IsCasting())
		return;

	bool engaged = IsEngaged();

	Mob *ow = GetOwner();
	if(!engaged)
	{
		if(ow)
		{
			if(ow->IsEngaged())
			{
				Mob *tar = ow->GetTarget();
				if(tar)
				{
					AddToHateList(tar, 1, 0, false);
				}
			}
		}
	}

	if(!ow)
	{
		if(!IsFeared() && !IsLD())
		{
			BuffFadeByEffect(SE_Charm);
			return;
		}
	}

	if(RuleB(Combat, EnableFearPathing)){
		if(curfp) {
			if(IsRooted()) {
				//make sure everybody knows were not moving, for appearance sake
				if(IsMoving())
				{
					if(GetTarget())
						SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
					SetRunAnimSpeed(0);
					SendPosition();
					SetMoving(false);
					moved=false;
				}
				//continue on to attack code, ensuring that we execute the engaged code
				engaged = true;
			} else {
				if(AImovement_timer->Check()) {
					animation = GetRunspeed() * 21;
					// Check if we have reached the last fear point
					if((ABS(GetX()-fear_walkto_x) < 0.1) && (ABS(GetY()-fear_walkto_y) <0.1)) {
						// Calculate a new point to run to
						CalculateNewFearpoint();
					}
					if(!RuleB(Pathing, Fear) || !zone->pathing)
						CalculateNewPosition2(fear_walkto_x, fear_walkto_y, fear_walkto_z, GetFearSpeed(), true);
					else
					{
						bool WaypointChanged, NodeReached;

						VERTEX Goal = UpdatePath(fear_walkto_x, fear_walkto_y, fear_walkto_z,
									GetFearSpeed(), WaypointChanged, NodeReached);

						if(WaypointChanged)
							tar_ndx = 20;

						CalculateNewPosition2(Goal.x, Goal.y, Goal.z, GetFearSpeed());
					}
				}
				return;
			}
		}
	}

	if (engaged)
	{
		if (IsRooted())
			SetTarget(hate_list.GetClosest(this));
		else
		{
			if(AItarget_check_timer->Check())
			{
				SetTarget(hate_list.GetTop(this));
			}
		}

		if (!GetTarget())
			return;

		if (GetTarget()->IsCorpse()) {
			RemoveFromHateList(this);
			return;
		}

		if(DivineAura())
			return;

		bool is_combat_range = CombatRange(GetTarget());

		if(is_combat_range) {
			if(charm_class_attacks_timer.Check()) {
				DoClassAttacks(GetTarget());
			}

			if (AImovement_timer->Check()) {
				SetRunAnimSpeed(0);
			}
			if(IsMoving()) {
				SetMoving(false);
				moved=false;
				SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
				SendPosition();
				tar_ndx =0;
			}

			if(GetTarget() && !IsStunned() && !IsMezzed() && !GetFeigned()) {
				if(attack_timer.Check()) {
					Attack(GetTarget(), 13);
					if(GetTarget()) {
						if(CheckDoubleAttack()) {
							Attack(GetTarget(), 13);
							if(GetTarget()) {
								bool triple_attack_success = false;
								if((((GetClass() == MONK || GetClass() == WARRIOR || GetClass() == RANGER || GetClass() == BERSERKER)
									&& GetLevel() >= 60) || GetSpecialAbility(SPECATK_TRIPLE))
									&& CheckDoubleAttack(true))
								{
									Attack(GetTarget(), 13, true);
									triple_attack_success = true;
								}

								if(GetTarget())
								{
									//Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
									int16 flurrychance = aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance;

									if (flurrychance)
									{
										if(MakeRandomInt(0, 100) < flurrychance)
										{
											Message_StringID(MT_NPCFlurry, YOU_FLURRY);
											Attack(GetTarget(), 13, false);
											Attack(GetTarget(), 13, false);
										}
									}

									int16 ExtraAttackChanceBonus = spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance + aabonuses.ExtraAttackChance;

									if (ExtraAttackChanceBonus && GetTarget()) {
										ItemInst *wpn = GetInv().GetItem(SLOT_PRIMARY);
										if(wpn){
											if(wpn->GetItem()->ItemType == ItemType2HSlash ||
												wpn->GetItem()->ItemType == ItemType2HBlunt ||
												wpn->GetItem()->ItemType == ItemType2HPiercing )
											{
												if(MakeRandomInt(0, 100) < ExtraAttackChanceBonus)
												{
													Attack(GetTarget(), 13, false);
												}
											}
										}
									}

									if (GetClass() == WARRIOR || GetClass() == BERSERKER)
									{
										if(!dead && !berserk && this->GetHPRatio() < 30)
										{
											entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_START, GetName());
											berserk = true;
										}
										else if (berserk && this->GetHPRatio() > 30)
										{
											entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_END, GetName());
											berserk = false;
										}
									}
								}
							}
						}
					}
				}
			}

			if(CanThisClassDualWield() && attack_dw_timer.Check())
			{
				if(GetTarget())
				{
					float DualWieldProbability = 0.0f;

					int16 Ambidexterity = aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
					DualWieldProbability = (GetSkill(SkillDualWield) + GetLevel() + Ambidexterity) / 400.0f; // 78.0 max
					int16 DWBonus = spellbonuses.DualWieldChance + itembonuses.DualWieldChance;
					DualWieldProbability += DualWieldProbability*float(DWBonus)/ 100.0f;

					if(MakeRandomFloat(0.0, 1.0) < DualWieldProbability)
					{
						Attack(GetTarget(), 14);
						if(CheckDoubleAttack())
						{
							Attack(GetTarget(), 14);
						}

					}
				}
			}
		}
		else
		{
			if(!IsRooted())
			{
				animation = 21 * GetRunspeed();
				if(!RuleB(Pathing, Aggro) || !zone->pathing)
					CalculateNewPosition2(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetRunspeed());
				else
				{
					bool WaypointChanged, NodeReached;
					VERTEX Goal = UpdatePath(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(),
						GetRunspeed(), WaypointChanged, NodeReached);

					if(WaypointChanged)
						tar_ndx = 20;

					CalculateNewPosition2(Goal.x, Goal.y, Goal.z, GetRunspeed());
				}
			}
			else if(IsMoving())
			{
				SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
				SetRunAnimSpeed(0);
				SendPosition();
				SetMoving(false);
				moved=false;
			}
		}
		AI_SpellCast();
	}
	else
	{
		if(AIfeignremember_timer->Check()) {
			std::set<uint32>::iterator RememberedCharID;
			RememberedCharID = feign_memory_list.begin();
			while (RememberedCharID != feign_memory_list.end()) {
				Client* remember_client = entity_list.GetClientByCharID(*RememberedCharID);
				if (remember_client == nullptr) {
					//they are gone now...
					RememberedCharID = feign_memory_list.erase(RememberedCharID);
				} else if (!remember_client->GetFeigned()) {
					AddToHateList(remember_client->CastToMob(),1);
					RememberedCharID = feign_memory_list.erase(RememberedCharID);
					break;
				} else {
					//they are still feigned, carry on...
					++RememberedCharID;
				}
			}
		}

		if(IsPet())
		{
			Mob* owner = GetOwner();
			if(owner == nullptr)
				return;

			float dist = DistNoRoot(*owner);
			if (dist >= 100)
			{
				float speed = dist >= 225 ? GetRunspeed() : GetWalkspeed();
				animation = 21 * speed;
				CalculateNewPosition2(owner->GetX(), owner->GetY(), owner->GetZ(), speed);
			}
			else
			{
				SetHeading(owner->GetHeading());
				if(moved)
				{
					moved=false;
					SetMoving(false);
					SendPosition();
					SetRunAnimSpeed(0);
				}
			}
		}
	}
}

void Mob::AI_Process() {
	if (!IsAIControlled())
		return;

	if (!(AIthink_timer->Check() || attack_timer.Check(false)))
		return;

	if (IsCasting())
		return;

	bool engaged = IsEngaged();
	bool doranged = false;

	// Begin: Additions for Wiz Fear Code
	//
	if(RuleB(Combat, EnableFearPathing)){
		if(curfp) {
			if(IsRooted()) {
				//make sure everybody knows were not moving, for appearance sake
				if(IsMoving())
				{
					if(target)
						SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
					SetRunAnimSpeed(0);
					SendPosition();
					SetMoving(false);
					moved=false;
				}
				//continue on to attack code, ensuring that we execute the engaged code
				engaged = true;
			} else {
				if(AImovement_timer->Check()) {
					// Check if we have reached the last fear point
					if((ABS(GetX()-fear_walkto_x) < 0.1) && (ABS(GetY()-fear_walkto_y) <0.1)) {
						// Calculate a new point to run to
						CalculateNewFearpoint();
					}
					if(!RuleB(Pathing, Fear) || !zone->pathing)
						CalculateNewPosition2(fear_walkto_x, fear_walkto_y, fear_walkto_z, GetFearSpeed(), true);
					else
					{
						bool WaypointChanged, NodeReached;

						VERTEX Goal = UpdatePath(fear_walkto_x, fear_walkto_y, fear_walkto_z,
									GetFearSpeed(), WaypointChanged, NodeReached);

						if(WaypointChanged)
							tar_ndx = 20;

						CalculateNewPosition2(Goal.x, Goal.y, Goal.z, GetFearSpeed());
					}
				}
				return;
			}
		}
	}

	// trigger EVENT_SIGNAL if required
	if(IsNPC()) {
		CastToNPC()->CheckSignal();
	}

	if (engaged)
	{
		if (IsRooted())
			SetTarget(hate_list.GetClosest(this));
		else
		{
			if(AItarget_check_timer->Check())
			{
				if (IsFocused()) {
					if (!target) {
						SetTarget(hate_list.GetTop(this));
					}
				} else {
					if (!ImprovedTaunt())
						SetTarget(hate_list.GetTop(this));
				}

			}
		}

		if (!target)
			return;

		if (target->IsCorpse())
		{
			RemoveFromHateList(this);
			return;
		}

#ifdef BOTS
		if (IsPet() && GetOwner()->IsBot() && target == GetOwner())
		{
				// this blocks all pet attacks against owner..bot pet test (copied above check)
				RemoveFromHateList(this);
				return;
		}
#endif //BOTS

		if(DivineAura())
			return;

		if(GetSpecialAbility(TETHER)) {
			float tether_range = static_cast<float>(GetSpecialAbilityParam(TETHER, 0));
			tether_range = tether_range > 0.0f ? tether_range * tether_range : pAggroRange * pAggroRange;

			if(DistNoRootNoZ(CastToNPC()->GetSpawnPointX(), CastToNPC()->GetSpawnPointY()) > tether_range) {
				GMMove(CastToNPC()->GetSpawnPointX(), CastToNPC()->GetSpawnPointY(), CastToNPC()->GetSpawnPointZ(), CastToNPC()->GetSpawnPointH());
			}
		} else if(GetSpecialAbility(LEASH)) {
			float leash_range = static_cast<float>(GetSpecialAbilityParam(LEASH, 0));
			leash_range = leash_range > 0.0f ? leash_range * leash_range : pAggroRange * pAggroRange;

			if(DistNoRootNoZ(CastToNPC()->GetSpawnPointX(), CastToNPC()->GetSpawnPointY()) > leash_range) {
				GMMove(CastToNPC()->GetSpawnPointX(), CastToNPC()->GetSpawnPointY(), CastToNPC()->GetSpawnPointZ(), CastToNPC()->GetSpawnPointH());
				SetHP(GetMaxHP());
				BuffFadeAll();
				WipeHateList();
				return;
			}
		}

		StartEnrage();

		bool is_combat_range = CombatRange(target);

		if (is_combat_range)
		{
			if (AImovement_timer->Check())
			{
				SetRunAnimSpeed(0);
			}
			if(IsMoving())
			{
				SetMoving(false);
				moved=false;
				SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
				SendPosition();
				tar_ndx =0;
			}

			//casting checked above...
			if(target && !IsStunned() && !IsMezzed() && GetAppearance() != eaDead && !IsMeleeDisabled()) {

				//we should check to see if they die mid-attacks, previous
				//crap of checking target for null was not gunna cut it

				//try main hand first
				if(attack_timer.Check()) {
					if(IsNPC()) {
						int16 n_atk = CastToNPC()->GetNumberOfAttacks();
						if(n_atk <= 1) {
							Attack(target, 13);
						} else {
							for(int i = 0; i < n_atk; ++i) {
								Attack(target, 13);
							}
						}
					} else {
						Attack(target, 13);
					}

					if (target) {
						//we use this random value in three comparisons with different
						//thresholds, and if its truely random, then this should work
						//out reasonably and will save us compute resources.
						int32 RandRoll = MakeRandomInt(0, 99);
						if ((CanThisClassDoubleAttack() || GetSpecialAbility(SPECATK_TRIPLE)
								|| GetSpecialAbility(SPECATK_QUAD))
								//check double attack, this is NOT the same rules that clients use...
								&& RandRoll < (GetLevel() + NPCDualAttackModifier)) {
							Attack(target, 13);
							// lets see if we can do a triple attack with the main hand
							//pets are excluded from triple and quads...
							if ((GetSpecialAbility(SPECATK_TRIPLE) || GetSpecialAbility(SPECATK_QUAD))
									&& !IsPet() && RandRoll < (GetLevel() + NPCTripleAttackModifier)) {
								Attack(target, 13);
								// now lets check the quad attack
								if (GetSpecialAbility(SPECATK_QUAD)
										&& RandRoll < (GetLevel() + NPCQuadAttackModifier)) {
									Attack(target, 13);
								}
							}
						}
					}

					if (GetSpecialAbility(SPECATK_FLURRY)) {
						int flurry_chance = GetSpecialAbilityParam(SPECATK_FLURRY, 0);
						flurry_chance = flurry_chance > 0 ? flurry_chance : RuleI(Combat, NPCFlurryChance); 

						if (MakeRandomInt(0, 99) < flurry_chance) {
							ExtraAttackOptions opts;
							int cur = GetSpecialAbilityParam(SPECATK_FLURRY, 2);
							if (cur > 0)
								opts.damage_percent = cur / 100.0f;

							cur = GetSpecialAbilityParam(SPECATK_FLURRY, 3);
							if (cur > 0)
								opts.damage_flat = cur;

							cur = GetSpecialAbilityParam(SPECATK_FLURRY, 4);
							if (cur > 0)
								opts.armor_pen_percent = cur / 100.0f;

							cur = GetSpecialAbilityParam(SPECATK_FLURRY, 5);
							if (cur > 0)
								opts.armor_pen_flat = cur;

							cur = GetSpecialAbilityParam(SPECATK_FLURRY, 6);
							if (cur > 0)
								opts.crit_percent = cur / 100.0f;

							cur = GetSpecialAbilityParam(SPECATK_FLURRY, 7);
							if (cur > 0)
								opts.crit_flat = cur;

							Flurry(&opts);
						}
					}

					if (IsPet()) {
						Mob *owner = GetOwner();
						if (owner) {
						int16 flurry_chance = owner->aabonuses.PetFlurry +
							owner->spellbonuses.PetFlurry + owner->itembonuses.PetFlurry;
							if (flurry_chance && (MakeRandomInt(0, 99) < flurry_chance))
								Flurry(nullptr);
						}
					}

					if (GetSpecialAbility(SPECATK_RAMPAGE))
					{
						int rampage_chance = GetSpecialAbilityParam(SPECATK_RAMPAGE, 0);
						rampage_chance = rampage_chance > 0 ? rampage_chance : 20;
						if(MakeRandomInt(0, 99) < rampage_chance) {
							ExtraAttackOptions opts;
							int cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 2);
							if(cur > 0) {
								opts.damage_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 3);
							if(cur > 0) {
								opts.damage_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 4);
							if(cur > 0) {
								opts.armor_pen_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 5);
							if(cur > 0) {
								opts.armor_pen_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 6);
							if(cur > 0) {
								opts.crit_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 7);
							if(cur > 0) {
								opts.crit_flat = cur;
							}
							Rampage(&opts);
						}
					}

					if (GetSpecialAbility(SPECATK_AREA_RAMPAGE))
					{
						int rampage_chance = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 0);
						rampage_chance = rampage_chance > 0 ? rampage_chance : 20;
						if(MakeRandomInt(0, 99) < rampage_chance) {
							ExtraAttackOptions opts;
							int cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 2);
							if(cur > 0) {
								opts.damage_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 3);
							if(cur > 0) {
								opts.damage_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 4);
							if(cur > 0) {
								opts.armor_pen_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 5);
							if(cur > 0) {
								opts.armor_pen_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 6);
							if(cur > 0) {
								opts.crit_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 7);
							if(cur > 0) {
								opts.crit_flat = cur;
							}

							AreaRampage(&opts);
						}
					}
				}

				//now off hand
				if (attack_dw_timer.Check() && CanThisClassDualWield())
				{
					int myclass = GetClass();
					//can only dual wield without a weapon if your a monk
					if(GetSpecialAbility(SPECATK_INNATE_DW) || (GetEquipment(MaterialSecondary) != 0 && GetLevel() > 29) || myclass == MONK || myclass == MONKGM) {
						float DualWieldProbability = (GetSkill(SkillDualWield) + GetLevel()) / 400.0f;
						if(MakeRandomFloat(0.0, 1.0) < DualWieldProbability)
						{
							Attack(target, 14);
							if (CanThisClassDoubleAttack())
							{
								int32 RandRoll = MakeRandomInt(0, 99);
								if (RandRoll < (GetLevel() + 20))
								{
									Attack(target, 14);
								}
							}
						}
					}
				}

				//now special attacks (kick, etc)
				if(IsNPC())
					CastToNPC()->DoClassAttacks(target);
			}
			AI_EngagedCastCheck();
		}	//end is within combat range
		else {
			//we cannot reach our target...
			//underwater stuff only works with water maps in the zone!
			if(IsNPC() && CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				if(!zone->watermap->InLiquid(target->GetX(), target->GetY(), target->GetZ())) {
					Mob *tar = hate_list.GetTop(this);
					if(tar == target) {
						WipeHateList();
						Heal();
						BuffFadeAll();
						AIwalking_timer->Start(100);
						pLastFightingDelayMoving = Timer::GetCurrentTime();
						return;
					} else if(tar != nullptr) {
						SetTarget(tar);
						return;
					}
				}
			}

			// See if we can summon the mob to us
			if (!HateSummon())
			{
				//could not summon them, check ranged...
				if(GetSpecialAbility(SPECATK_RANGED_ATK))
					doranged = true;

				// Now pursue
				// TODO: Check here for another person on hate list with close hate value
				if(AI_PursueCastCheck()){
					//we did something, so do not process movement.
				}
				else if (AImovement_timer->Check())
				{
					if(!IsRooted()) {
						mlog(AI__WAYPOINTS, "Pursuing %s while engaged.", target->GetName());
						if(!RuleB(Pathing, Aggro) || !zone->pathing)
							CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), GetRunspeed());
						else
						{
							bool WaypointChanged, NodeReached;

							VERTEX Goal = UpdatePath(target->GetX(), target->GetY(), target->GetZ(),
											GetRunspeed(), WaypointChanged, NodeReached);

							if(WaypointChanged)
								tar_ndx = 20;

							CalculateNewPosition2(Goal.x, Goal.y, Goal.z, GetRunspeed());
						}

					}
					else if(IsMoving()) {
						SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
						SetRunAnimSpeed(0);
						SendPosition();
						SetMoving(false);
						moved=false;

					}
				}
			}
		}
	}
	else
	{
		if(AIfeignremember_timer->Check()) {
			// EverHood - 6/14/06
			// Improved Feign Death Memory
			// check to see if any of our previous feigned targets have gotten up.
			std::set<uint32>::iterator RememberedCharID;
			RememberedCharID = feign_memory_list.begin();
			while (RememberedCharID != feign_memory_list.end()) {
				Client* remember_client = entity_list.GetClientByCharID(*RememberedCharID);
				if (remember_client == nullptr) {
					//they are gone now...
					RememberedCharID = feign_memory_list.erase(RememberedCharID);
				} else if (!remember_client->GetFeigned()) {
					AddToHateList(remember_client->CastToMob(),1);
					RememberedCharID = feign_memory_list.erase(RememberedCharID);
					break;
				} else {
					//they are still feigned, carry on...
					++RememberedCharID;
				}
			}
		}
		if (AI_IdleCastCheck())
		{
			//we processed a spell action, so do nothing else.
		}
		else if (AIscanarea_timer->Check())
		{
			/*
			* This is where NPCs look around to see if they want to attack anybody.
			*
			* if REVERSE_AGGRO is enabled, then this timer is disabled unless they
			* have the npc_aggro flag on them, and aggro against clients is checked
			* by the clients.
			*
			*/

			Mob* tmptar = entity_list.AICheckCloseAggro(this, GetAggroRange(), GetAssistRange());
			if (tmptar)
				AddToHateList(tmptar);
		}
		else if (AImovement_timer->Check() && !IsRooted())
		{
			SetRunAnimSpeed(0);
			if (IsPet())
			{
				// we're a pet, do as we're told
				switch (pStandingPetOrder)
				{
					case SPO_Follow:
					{

						Mob* owner = GetOwner();
						if(owner == nullptr)
							break;

						//if(owner->IsClient())
						//	printf("Pet start pos: (%f, %f, %f)\n", GetX(), GetY(), GetZ());

						float dist = DistNoRoot(*owner);
						if (dist >= 400)
						{
							float speed = GetWalkspeed();
							if (dist >= 5625)
								speed = GetRunspeed();
							CalculateNewPosition2(owner->GetX(), owner->GetY(), owner->GetZ(), speed);
						}
						else
						{
							if(moved)
							{
								moved=false;
								SetMoving(false);
								SendPosition();
							}
						}

						/*
						//fix up Z
						float zdiff = GetZ() - owner->GetZ();
						if(zdiff < 0)
							zdiff = 0 - zdiff;
						if(zdiff > 2.0f) {
							SendTo(GetX(), GetY(), owner->GetZ());
							SendPosition();
						}

						if(owner->IsClient())
							printf("Pet pos: (%f, %f, %f)\n", GetX(), GetY(), GetZ());
						*/

						break;
					}
					case SPO_Sit:
					{
						SetAppearance(eaSitting, false);
						break;
					}
					case SPO_Guard:
					{
						//only NPCs can guard stuff. (forced by where the guard movement code is in the AI)
						if(IsNPC()) {
							CastToNPC()->NextGuardPosition();
						}
						break;
					}
				}
			}
			else if (GetFollowID())
			{
				Mob* follow = entity_list.GetMob(GetFollowID());
				if (!follow) SetFollowID(0);
				else
				{
					float dist2 = DistNoRoot(*follow);
					int followdist = GetFollowDistance();

					if (dist2 >= followdist)	// Default follow distance is 100
					{
						float speed = GetWalkspeed();
						if (dist2 >= followdist + 150)
							speed = GetRunspeed();
						CalculateNewPosition2(follow->GetX(), follow->GetY(), follow->GetZ(), speed);
					}
					else
					{
						if(moved)
						{
							SendPosition();
							moved=false;
							SetMoving(false);
						}
					}
				}
			}
			else //not a pet, and not following somebody...
			{
				// dont move till a bit after you last fought
				if (pLastFightingDelayMoving < Timer::GetCurrentTime())
				{
					if (this->IsClient())
					{
						// LD timer expired, drop out of world
						if (this->CastToClient()->IsLD())
							this->CastToClient()->Disconnect();
						return;
					}

					if(IsNPC())
					{
						if(RuleB(NPC, SmartLastFightingDelayMoving) && !feign_memory_list.empty())
						{
							minLastFightingDelayMoving = 0;
							maxLastFightingDelayMoving = 0;
						}
						CastToNPC()->AI_DoMovement();
					}
				}

			}
		} // else if (AImovement_timer->Check())
	}

	//Do Ranged attack here
	if(doranged)
	{
		int attacks = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 0);
		attacks = attacks > 0 ? attacks : 1;
		for(int i = 0; i < attacks; ++i) {
			RangedAttack(target);
		}
	}
}

void NPC::AI_DoMovement() {
	float walksp = GetMovespeed();
	if(walksp <= 0.0f)
		return;	//this is idle movement at walk speed, and we are unable to walk right now.

	if (roambox_distance > 0) {
		if (
			roambox_movingto_x > roambox_max_x
			|| roambox_movingto_x < roambox_min_x
			|| roambox_movingto_y > roambox_max_y
			|| roambox_movingto_y < roambox_min_y
			)
		{
			float movedist = roambox_distance*roambox_distance;
			float movex = MakeRandomFloat(0, movedist);
			float movey = movedist - movex;
			movex = sqrtf(movex);
			movey = sqrtf(movey);
			movex *= MakeRandomInt(0, 1) ? 1 : -1;
			movey *= MakeRandomInt(0, 1) ? 1 : -1;
			roambox_movingto_x = GetX() + movex;
			roambox_movingto_y = GetY() + movey;
			//Try to calculate new coord using distance.
			if (roambox_movingto_x > roambox_max_x || roambox_movingto_x < roambox_min_x)
				roambox_movingto_x -= movex * 2;
			if (roambox_movingto_y > roambox_max_y || roambox_movingto_y < roambox_min_y)
				roambox_movingto_y -= movey * 2;
			//New coord is still invalid, ignore distance and just pick a new random coord. 
			//If we're here we may have a roambox where one side is shorter than the specified distance. Commons, Wkarana, etc.
			if (roambox_movingto_x > roambox_max_x || roambox_movingto_x < roambox_min_x)
				roambox_movingto_x = MakeRandomFloat(roambox_min_x+1,roambox_max_x-1);
			if (roambox_movingto_y > roambox_max_y || roambox_movingto_y < roambox_min_y)
				roambox_movingto_y = MakeRandomFloat(roambox_min_y+1,roambox_max_y-1);
		}

		mlog(AI__WAYPOINTS, "Roam Box: d=%.3f (%.3f->%.3f,%.3f->%.3f): Go To (%.3f,%.3f)",
			roambox_distance, roambox_min_x, roambox_max_x, roambox_min_y, roambox_max_y, roambox_movingto_x, roambox_movingto_y);
		if (!CalculateNewPosition2(roambox_movingto_x, roambox_movingto_y, GetZ(), walksp, true))
		{
			roambox_movingto_x = roambox_max_x + 1; // force update
			pLastFightingDelayMoving = Timer::GetCurrentTime() + RandomTimer(roambox_min_delay, roambox_delay);
			SetMoving(false);
			SendPosition();	// makes mobs stop clientside
		}
	}
	else if (roamer)
	{
		if (AIwalking_timer->Check())
		{
			movetimercompleted=true;
			AIwalking_timer->Disable();
		}


		int16 gridno = CastToNPC()->GetGrid();

		if (gridno > 0 || cur_wp==-2) {
			if (movetimercompleted==true) { // time to pause at wp is over
				if (wandertype == 4 && cur_wp == CastToNPC()->GetMaxWp()) {
					CastToNPC()->Depop(true); //depop and resart spawn timer
				}
				else if (wandertype == 6 && cur_wp == CastToNPC()->GetMaxWp()) {
					CastToNPC()->Depop(false);//depop without spawn timer
				}
				else {
					movetimercompleted=false;

					mlog(QUESTS__PATHING, "We are departing waypoint %d.", cur_wp);

					//if we were under quest control (with no grid), we are done now..
					if(cur_wp == -2) {
						mlog(QUESTS__PATHING, "Non-grid quest mob has reached its quest ordered waypoint. Leaving pathing mode.");
						roamer = false;
						cur_wp = 0;
					}

					if(GetAppearance() != eaStanding)
						SetAppearance(eaStanding, false);

					entity_list.OpenDoorsNear(CastToNPC());

					if(!DistractedFromGrid) {
						//kick off event_waypoint depart
						char temp[16];
						sprintf(temp, "%d", cur_wp);
						parse->EventNPC(EVENT_WAYPOINT_DEPART, CastToNPC(), nullptr, temp, 0);

						//setup our next waypoint, if we are still on our normal grid
						//remember that the quest event above could have done anything it wanted with our grid
						if(gridno > 0) {
							CastToNPC()->CalculateNewWaypoint();
						}
					}
					else {
						DistractedFromGrid = false;
					}
				}
			}	// endif (movetimercompleted==true)
			else if (!(AIwalking_timer->Enabled()))
			{	// currently moving
				if (cur_wp_x == GetX() && cur_wp_y == GetY())
				{	// are we there yet? then stop
					mlog(AI__WAYPOINTS, "We have reached waypoint %d (%.3f,%.3f,%.3f) on grid %d", cur_wp, GetX(), GetY(), GetZ(), GetGrid());
					SetWaypointPause();
					if(GetAppearance() != eaStanding)
						SetAppearance(eaStanding, false);
					SetMoving(false);
					if (cur_wp_heading >= 0.0) {
						SetHeading(cur_wp_heading);
					}
					SendPosition();

					//kick off event_waypoint arrive
					char temp[16];
					sprintf(temp, "%d", cur_wp);
					parse->EventNPC(EVENT_WAYPOINT_ARRIVE, CastToNPC(), nullptr, temp, 0);

					// wipe feign memory since we reached our first waypoint
					if(cur_wp == 1)
						ClearFeignMemory();
				}
				else
				{	// not at waypoint yet, so keep moving
					if(!RuleB(Pathing, AggroReturnToGrid) || !zone->pathing || (DistractedFromGrid == 0))
						CalculateNewPosition2(cur_wp_x, cur_wp_y, cur_wp_z, walksp, true);
					else
					{
						bool WaypointChanged;
						bool NodeReached;
						VERTEX Goal = UpdatePath(cur_wp_x, cur_wp_y, cur_wp_z, walksp, WaypointChanged, NodeReached);
						if(WaypointChanged)
							tar_ndx = 20;

						if(NodeReached)
							entity_list.OpenDoorsNear(CastToNPC());

						CalculateNewPosition2(Goal.x, Goal.y, Goal.z, walksp, true);
					}

				}
			}
		}		// endif (gridno > 0)
// handle new quest grid command processing
		else if (gridno < 0)
		{	// this mob is under quest control
			if (movetimercompleted==true)
			{ // time to pause has ended
				SetGrid( 0 - GetGrid()); // revert to AI control
				mlog(QUESTS__PATHING, "Quest pathing is finished. Resuming on grid %d", GetGrid());

				if(GetAppearance() != eaStanding)
					SetAppearance(eaStanding, false);

				CalculateNewWaypoint();
			}
		}

	}
	else if (IsGuarding())
	{
		bool CP2Moved;
		if(!RuleB(Pathing, Guard) || !zone->pathing)
			CP2Moved = CalculateNewPosition2(guard_x, guard_y, guard_z, walksp);
		else
		{
			if(!((x_pos == guard_x) && (y_pos == guard_y) && (z_pos == guard_z)))
			{
				bool WaypointChanged, NodeReached;
				VERTEX Goal = UpdatePath(guard_x, guard_y, guard_z, walksp, WaypointChanged, NodeReached);
				if(WaypointChanged)
					tar_ndx = 20;

				if(NodeReached)
					entity_list.OpenDoorsNear(CastToNPC());

				CP2Moved = CalculateNewPosition2(Goal.x, Goal.y, Goal.z, walksp);
			}
			else
				CP2Moved = false;

		}
		if (!CP2Moved)
		{
			if(moved) {
				mlog(AI__WAYPOINTS, "Reached guard point (%.3f,%.3f,%.3f)", guard_x, guard_y, guard_z);
				ClearFeignMemory();
				moved=false;
				SetMoving(false);
				if (GetTarget() == nullptr || DistNoRoot(*GetTarget()) >= 5*5 )
				{
					SetHeading(guard_heading);
				} else {
					FaceTarget(GetTarget());
				}
				SendPosition();
				SetAppearance(GetGuardPointAnim());
			}
		}
	}
}

// Note: Mob that caused this may not get added to the hate list until after this function call completes
void Mob::AI_Event_Engaged(Mob* attacker, bool iYellForHelp) {
	if (!IsAIControlled())
		return;

	if(GetAppearance() != eaStanding)
	{
		SetAppearance(eaStanding);
	}

	if (iYellForHelp) {
		if(IsPet()) {
			GetOwner()->AI_Event_Engaged(attacker, iYellForHelp);
		} else {
			entity_list.AIYellForHelp(this, attacker);
		}
	}

	if(IsNPC())
	{
		if(CastToNPC()->GetGrid() > 0)
		{
			DistractedFromGrid = true;
		}
		if(attacker && !attacker->IsCorpse())
		{
			//Because sometimes the AIYellForHelp triggers another engaged and then immediately a not engaged
			//if the target dies before it goes off
			if(attacker->GetHP() > 0)
			{
				if(!CastToNPC()->GetCombatEvent() && GetHP() > 0)
				{
					parse->EventNPC(EVENT_COMBAT, CastToNPC(), attacker, "1", 0);
					uint16 emoteid = GetEmoteID();
					if(emoteid != 0)
						CastToNPC()->DoNPCEmote(ENTERCOMBAT,emoteid);
					CastToNPC()->SetCombatEvent(true);
				}
			}
		}
	}
}

// Note: Hate list may not be actually clear until after this function call completes
void Mob::AI_Event_NoLongerEngaged() {
	if (!IsAIControlled())
		return;
	this->AIwalking_timer->Start(RandomTimer(3000,20000));
	pLastFightingDelayMoving = Timer::GetCurrentTime();
	if (minLastFightingDelayMoving == maxLastFightingDelayMoving)
		pLastFightingDelayMoving += minLastFightingDelayMoving;
	else
		pLastFightingDelayMoving += MakeRandomInt(minLastFightingDelayMoving, maxLastFightingDelayMoving);
	// EverHood - So mobs don't keep running as a ghost until AIwalking_timer fires
	// if they were moving prior to losing all hate
	if(IsMoving()){
		SetRunAnimSpeed(0);
		SetMoving(false);
		SendPosition();
	}
	ClearRampage();

	if(IsNPC())
	{
		if(CastToNPC()->GetCombatEvent() && GetHP() > 0)
		{
			if(entity_list.GetNPCByID(this->GetID()))
			{
			uint16 emoteid = CastToNPC()->GetEmoteID();
			parse->EventNPC(EVENT_COMBAT, CastToNPC(), nullptr, "0", 0);
			if(emoteid != 0)
				CastToNPC()->DoNPCEmote(LEAVECOMBAT,emoteid);
			CastToNPC()->SetCombatEvent(false);
			}
		}
	}
}

//this gets called from InterruptSpell() for failure or SpellFinished() for success
void NPC::AI_Event_SpellCastFinished(bool iCastSucceeded, uint8 slot) {
	if (slot == 1) {
		uint32 recovery_time = 0;
		if (iCastSucceeded) {
			if (casting_spell_AIindex < AIspells.size()) {
					recovery_time += spells[AIspells[casting_spell_AIindex].spellid].recovery_time;
					if (AIspells[casting_spell_AIindex].recast_delay >= 0)
					{
						if (AIspells[casting_spell_AIindex].recast_delay < 10000)
							AIspells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + (AIspells[casting_spell_AIindex].recast_delay*1000);
					}
					else
						AIspells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + spells[AIspells[casting_spell_AIindex].spellid].recast_time;
			}
			if (recovery_time < AIautocastspell_timer->GetSetAtTrigger())
				recovery_time = AIautocastspell_timer->GetSetAtTrigger();
			AIautocastspell_timer->Start(recovery_time, false);
		}
		else
			AIautocastspell_timer->Start(800, false);
		casting_spell_AIindex = AIspells.size();
	}
}


bool NPC::AI_EngagedCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		mlog(AI__SPELLS, "Engaged autocast check triggered. Trying to cast healing spells then maybe offensive spells.");

		// try casting a heal or gate
		if (!AICastSpell(this, 100, SpellType_Heal | SpellType_Escape | SpellType_InCombatBuff)) {
			// try casting a heal on nearby
			if (!entity_list.AICheckCloseBeneficialSpells(this, 25, MobAISpellRange, SpellType_Heal)) {
				//nobody to heal, try some detrimental spells.
				if(!AICastSpell(GetTarget(), 20, SpellType_Nuke | SpellType_Lifetap | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff | SpellType_Charm | SpellType_Root)) {
					//no spell to cast, try again soon.
					AIautocastspell_timer->Start(RandomTimer(500, 1000), false);
				}
			}
		}
		return(true);
	}

	return(false);
}

bool NPC::AI_PursueCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		mlog(AI__SPELLS, "Engaged (pursuing) autocast check triggered. Trying to cast offensive spells.");
		if(!AICastSpell(GetTarget(), 90, SpellType_Root | SpellType_Nuke | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff)) {
			//no spell cast, try again soon.
			AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
		} //else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

bool NPC::AI_IdleCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
#if MobAI_DEBUG_Spells >= 25
		std::cout << "Non-Engaged autocast check triggered: " << this->GetName() << std::endl;
#endif
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		if (!AICastSpell(this, 100, SpellType_Heal | SpellType_Buff | SpellType_Pet)) {
			if(!entity_list.AICheckCloseBeneficialSpells(this, 33, MobAISpellRange, SpellType_Heal | SpellType_Buff)) {
				//if we didnt cast any spells, our autocast timer just resets to the
				//last duration it was set to... try to put up a more reasonable timer...
				AIautocastspell_timer->Start(RandomTimer(1000, 5000), false);
			}	//else, spell casting finishing will reset the timer.
		}	//else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

void Mob::StartEnrage()
{
	// dont continue if already enraged
	if (bEnraged)
		return;

	if(!GetSpecialAbility(SPECATK_ENRAGE))
		return;

	int hp_ratio = GetSpecialAbilityParam(SPECATK_ENRAGE, 0);
	hp_ratio = hp_ratio > 0 ? hp_ratio : RuleI(NPC, StartEnrageValue);
	if(GetHPRatio() > static_cast<float>(hp_ratio)) {
		return;
	}

	if(RuleB(NPC, LiveLikeEnrage) && !((IsPet() && !IsCharmed() && GetOwner() && GetOwner()->IsClient()) ||
		(CastToNPC()->GetSwarmOwner() && entity_list.GetMob(CastToNPC()->GetSwarmOwner())->IsClient()))) {
		return;
	}

	Timer *timer = GetSpecialAbilityTimer(SPECATK_ENRAGE);
	if (timer && !timer->Check())
		return;

	int enraged_duration = GetSpecialAbilityParam(SPECATK_ENRAGE, 1);
	enraged_duration = enraged_duration > 0 ? enraged_duration : EnragedDurationTimer;
	StartSpecialAbilityTimer(SPECATK_ENRAGE, enraged_duration);

	// start the timer. need to call IsEnraged frequently since we dont have callback timers :-/
	bEnraged = true;
	entity_list.MessageClose_StringID(this, true, 200, MT_NPCEnrage, NPC_ENRAGE_START, GetCleanName());
}

void Mob::ProcessEnrage(){
	if(IsEnraged()){
		Timer *timer = GetSpecialAbilityTimer(SPECATK_ENRAGE);
		if(timer && timer->Check()){
			entity_list.MessageClose_StringID(this, true, 200, MT_NPCEnrage, NPC_ENRAGE_END, GetCleanName());

			int enraged_cooldown = GetSpecialAbilityParam(SPECATK_ENRAGE, 2);
			enraged_cooldown = enraged_cooldown > 0 ? enraged_cooldown : EnragedTimer;
			StartSpecialAbilityTimer(SPECATK_ENRAGE, enraged_cooldown);
			bEnraged = false;
		}
	}
}

bool Mob::IsEnraged()
{
	return bEnraged;
}

bool Mob::Flurry(ExtraAttackOptions *opts)
{
	// this is wrong, flurry is extra attacks on the current target
	Mob *target = GetTarget();
	if (target) {
		if (!IsPet()) {
			entity_list.MessageClose_StringID(this, true, 200, MT_NPCFlurry, NPC_FLURRY, GetCleanName(), target->GetCleanName());
		} else {
			entity_list.MessageClose_StringID(this, true, 200, MT_PetFlurry, NPC_FLURRY, GetCleanName(), target->GetCleanName());
		}

		int num_attacks = GetSpecialAbilityParam(SPECATK_FLURRY, 1);
		num_attacks = num_attacks > 0 ? num_attacks : RuleI(Combat, MaxFlurryHits);
		for (int i = 0; i < num_attacks; i++)
			Attack(target, 13, false, false, false, opts);
	}
	return true;
}

bool Mob::AddRampage(Mob *mob)
{
	if (!mob)
		return false;

	if (!GetSpecialAbility(SPECATK_RAMPAGE))
		return false;

	for (int i = 0; i < RampageArray.size(); i++) {
		// if Entity ID is already on the list don't add it again
		if (mob->GetID() == RampageArray[i])
			return false;
	}
	RampageArray.push_back(mob->GetID());
	return true;
}

void Mob::ClearRampage()
{
	RampageArray.clear();
}

bool Mob::Rampage(ExtraAttackOptions *opts)
{
	int index_hit = 0;
	if (!IsPet())
		entity_list.MessageClose_StringID(this, true, 200, MT_NPCRampage, NPC_RAMPAGE, GetCleanName());
	else
		entity_list.MessageClose_StringID(this, true, 200, MT_PetFlurry, NPC_RAMPAGE, GetCleanName());

	int rampage_targets = GetSpecialAbilityParam(SPECATK_RAMPAGE, 1);
	if (rampage_targets == 0) // if set to 0 or not set in the DB
		rampage_targets = RuleI(Combat, DefaultRampageTargets);
	if (rampage_targets > RuleI(Combat, MaxRampageTargets))
		rampage_targets = RuleI(Combat, MaxRampageTargets);
	for (int i = 0; i < RampageArray.size(); i++) {
		if (index_hit >= rampage_targets)
			break;
		// range is important
		Mob *m_target = entity_list.GetMob(RampageArray[i]);
		if (m_target) {
			if (m_target == GetTarget())
				continue;
			if (CombatRange(m_target)) {
				Attack(m_target, 13, false, false, false, opts);
				index_hit++;
			}
		}
	}

	if (RuleB(Combat, RampageHitsTarget) && index_hit < rampage_targets)
		Attack(GetTarget(), 13, false, false, false, opts);

	return true;
}

void Mob::AreaRampage(ExtraAttackOptions *opts)
{
	int index_hit = 0;
	if (!IsPet()) { // do not know every pet AA so thought it safer to add this
		entity_list.MessageClose_StringID(this, true, 200, MT_NPCRampage, AE_RAMPAGE, GetCleanName());
	} else {
		entity_list.MessageClose_StringID(this, true, 200, MT_PetFlurry, AE_RAMPAGE, GetCleanName());
	}

	int rampage_targets = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 1);
	rampage_targets = rampage_targets > 0 ? rampage_targets : 1;
	index_hit = hate_list.AreaRampage(this, GetTarget(), rampage_targets, opts);

	if(index_hit == 0) {
		Attack(GetTarget(), 13, false, false, false, opts);
	}
}

uint32 Mob::GetLevelCon(uint8 mylevel, uint8 iOtherLevel) {
	int16 diff = iOtherLevel - mylevel;
	uint32 conlevel=0;

	if (diff == 0)
		return CON_WHITE;
	else if (diff >= 1 && diff <= 2)
		return CON_YELLOW;
	else if (diff >= 3)
		return CON_RED;

	if (mylevel <= 8)
	{
		if (diff <= -4)
			conlevel = CON_GREEN;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 9)
	{
		if (diff <= -6)
			conlevel = CON_GREEN;
		else if (diff <= -4)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 13)
	{
		if (diff <= -7)
			conlevel = CON_GREEN;
		else if (diff <= -5)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 15)
	{
		if (diff <= -7)
			conlevel = CON_GREEN;
		else if (diff <= -5)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 17)
	{
		if (diff <= -8)
			conlevel = CON_GREEN;
		else if (diff <= -6)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 21)
	{
		if (diff <= -9)
			conlevel = CON_GREEN;
		else if (diff <= -7)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 25)
	{
		if (diff <= -10)
			conlevel = CON_GREEN;
		else if (diff <= -8)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 29)
	{
		if (diff <= -11)
			conlevel = CON_GREEN;
		else if (diff <= -9)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 31)
	{
		if (diff <= -12)
			conlevel = CON_GREEN;
		else if (diff <= -9)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 33)
	{
		if (diff <= -13)
			conlevel = CON_GREEN;
		else if (diff <= -10)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 37)
	{
		if (diff <= -14)
			conlevel = CON_GREEN;
		else if (diff <= -11)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 41)
	{
		if (diff <= -16)
			conlevel = CON_GREEN;
		else if (diff <= -12)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 45)
	{
		if (diff <= -17)
			conlevel = CON_GREEN;
		else if (diff <= -13)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 49)
	{
		if (diff <= -18)
			conlevel = CON_GREEN;
		else if (diff <= -14)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 53)
	{
		if (diff <= -19)
			conlevel = CON_GREEN;
		else if (diff <= -15)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else if (mylevel <= 55)
	{
		if (diff <= -20)
			conlevel = CON_GREEN;
		else if (diff <= -15)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	else
	{
		if (diff <= -21)
			conlevel = CON_GREEN;
		else if (diff <= -16)
			conlevel = CON_LIGHTBLUE;
		else
			conlevel = CON_BLUE;
	}
	return conlevel;
}

void NPC::CheckSignal() {
	if (!signal_q.empty()) {
		int signal_id = signal_q.front();
		signal_q.pop_front();
		char buf[32];
		snprintf(buf, 31, "%d", signal_id);
		buf[31] = '\0';
		parse->EventNPC(EVENT_SIGNAL, this, nullptr, buf, 0);
	}
}



/*
alter table npc_types drop column usedspells;
alter table npc_types add column npc_spells_id int(11) unsigned not null default 0 after merchant_id;
Create Table npc_spells (
	id int(11) unsigned not null auto_increment primary key,
	name tinytext,
	parent_list int(11) unsigned not null default 0,
	attack_proc smallint(5) not null default -1,
	proc_chance tinyint(3) not null default 3
	);
create table npc_spells_entries (
	id int(11) unsigned not null auto_increment primary key,
	npc_spells_id int(11) not null,
	spellid smallint(5) not null default 0,
	type smallint(5) unsigned not null default 0,
	minlevel tinyint(3) unsigned not null default 0,
	maxlevel tinyint(3) unsigned not null default 255,
	manacost smallint(5) not null default '-1',
	recast_delay int(11) not null default '-1',
	priority smallint(5) not null default 0,
	index npc_spells_id (npc_spells_id)
	);
*/

bool IsSpellInList(DBnpcspells_Struct* spell_list, int16 iSpellID);
bool Compare_AI_Spells(AISpells_Struct i, AISpells_Struct j);

bool NPC::AI_AddNPCSpells(uint32 iDBSpellsID) {
	// ok, this function should load the list, and the parent list then shove them into the struct and sort
	npc_spells_id = iDBSpellsID;
	AIspells.clear();
	if (iDBSpellsID == 0) {
		AIautocastspell_timer->Disable();
		return false;
	}
	DBnpcspells_Struct* spell_list = database.GetNPCSpells(iDBSpellsID);
	if (!spell_list) {
		AIautocastspell_timer->Disable();
		return false;
	}
	DBnpcspells_Struct* parentlist = database.GetNPCSpells(spell_list->parent_list);
	uint32 i;
#if MobAI_DEBUG_Spells >= 10
	std::cout << "Loading NPCSpells onto " << this->GetName() << ": dbspellsid=" << iDBSpellsID;
	if (spell_list) {
		std::cout << " (found, " << spell_list->numentries << "), parentlist=" << spell_list->parent_list;
		if (spell_list->parent_list) {
			if (parentlist) {
				std::cout << " (found, " << parentlist->numentries << ")";
			}
			else
				std::cout << " (not found)";
		}
	}
	else
		std::cout << " (not found)";
	std::cout << std::endl;
#endif
	int16 attack_proc_spell = -1;
	int8 proc_chance = 3;
	if (parentlist) {
		attack_proc_spell = parentlist->attack_proc;
		proc_chance = parentlist->proc_chance;
		for (i=0; i<parentlist->numentries; i++) {
			if (GetLevel() >= parentlist->entries[i].minlevel && GetLevel() <= parentlist->entries[i].maxlevel && parentlist->entries[i].spellid > 0) {
				if (!IsSpellInList(spell_list, parentlist->entries[i].spellid))
				{
					AddSpellToNPCList(parentlist->entries[i].priority,
						parentlist->entries[i].spellid, parentlist->entries[i].type,
						parentlist->entries[i].manacost, parentlist->entries[i].recast_delay,
						parentlist->entries[i].resist_adjust);
				}
			}
		}
	}
	if (spell_list->attack_proc >= 0) {
		attack_proc_spell = spell_list->attack_proc;
		proc_chance = spell_list->proc_chance;
	}
	for (i=0; i<spell_list->numentries; i++) {
		if (GetLevel() >= spell_list->entries[i].minlevel && GetLevel() <= spell_list->entries[i].maxlevel && spell_list->entries[i].spellid > 0) {
			AddSpellToNPCList(spell_list->entries[i].priority,
				spell_list->entries[i].spellid, spell_list->entries[i].type,
				spell_list->entries[i].manacost, spell_list->entries[i].recast_delay,
				spell_list->entries[i].resist_adjust);
		}
	}
	std::sort(AIspells.begin(), AIspells.end(), Compare_AI_Spells);

	if (attack_proc_spell > 0)
		AddProcToWeapon(attack_proc_spell, true, proc_chance);

	if (AIspells.size() == 0)
		AIautocastspell_timer->Disable();
	else
		AIautocastspell_timer->Trigger();
	return true;
}

bool IsSpellInList(DBnpcspells_Struct* spell_list, int16 iSpellID) {
	for (uint32 i=0; i < spell_list->numentries; i++) {
		if (spell_list->entries[i].spellid == iSpellID)
			return true;
	}
	return false;
}

bool Compare_AI_Spells(AISpells_Struct i, AISpells_Struct j)
{
	return(i.priority > j.priority);
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void NPC::AddSpellToNPCList(int16 iPriority, int16 iSpellID, uint16 iType,
							int16 iManaCost, int32 iRecastDelay, int16 iResistAdjust)
{

	if(!IsValidSpell(iSpellID))
		return;

	HasAISpell = true;
	AISpells_Struct t;

	t.priority = iPriority;
	t.spellid = iSpellID;
	t.type = iType;
	t.manacost = iManaCost;
	t.recast_delay = iRecastDelay;
	t.time_cancast = 0;
	t.resist_adjust = iResistAdjust;

	AIspells.push_back(t);
}

void NPC::RemoveSpellFromNPCList(int16 spell_id)
{
	std::vector<AISpells_Struct>::iterator iter = AIspells.begin();
	while(iter != AIspells.end())
	{
		if((*iter).spellid == spell_id)
		{
			iter = AIspells.erase(iter);
			continue;
		}
		++iter;
	}
}

void NPC::AISpellsList(Client *c)
{
	if (!c)
		return;

	for (std::vector<AISpells_Struct>::iterator it = AIspells.begin(); it != AIspells.end(); ++it)
		c->Message(0, "%s (%d): Type %d, Priority %d",
				spells[it->spellid].name, it->spellid, it->type, it->priority);

	return;
}

DBnpcspells_Struct* ZoneDatabase::GetNPCSpells(uint32 iDBSpellsID) {
	if (iDBSpellsID == 0)
		return 0;
	if (!npc_spells_cache) {
		npc_spells_maxid = GetMaxNPCSpellsID();
		npc_spells_cache = new DBnpcspells_Struct*[npc_spells_maxid+1];
		npc_spells_loadtried = new bool[npc_spells_maxid+1];
		for (uint32 i=0; i<=npc_spells_maxid; i++) {
			npc_spells_cache[i] = 0;
			npc_spells_loadtried[i] = false;
		}
	}
	if (iDBSpellsID > npc_spells_maxid)
		return 0;
	if (npc_spells_cache[iDBSpellsID]) { // it's in the cache, easy =)
		return npc_spells_cache[iDBSpellsID];
	}
	else if (!npc_spells_loadtried[iDBSpellsID]) { // no reason to ask the DB again if we have failed once already
		npc_spells_loadtried[iDBSpellsID] = true;
		char errbuf[MYSQL_ERRMSG_SIZE];
		char *query = 0;
		MYSQL_RES *result;
		MYSQL_ROW row;

		if (RunQuery(query, MakeAnyLenString(&query, "SELECT id, parent_list, attack_proc, proc_chance from npc_spells where id=%d", iDBSpellsID), errbuf, &result)) {
			safe_delete_array(query);
			if (mysql_num_rows(result) == 1) {
				row = mysql_fetch_row(result);
				uint32 tmpparent_list = atoi(row[1]);
				int16 tmpattack_proc = atoi(row[2]);
				uint8 tmpproc_chance = atoi(row[3]);
				mysql_free_result(result);
				if (RunQuery(query, MakeAnyLenString(&query, "SELECT spellid, type, minlevel, maxlevel, manacost, recast_delay, priority, resist_adjust from npc_spells_entries where npc_spells_id=%d ORDER BY minlevel", iDBSpellsID), errbuf, &result)) {
					safe_delete_array(query);
					uint32 tmpSize = sizeof(DBnpcspells_Struct) + (sizeof(DBnpcspells_entries_Struct) * mysql_num_rows(result));
					npc_spells_cache[iDBSpellsID] = (DBnpcspells_Struct*) new uchar[tmpSize];
					memset(npc_spells_cache[iDBSpellsID], 0, tmpSize);
					npc_spells_cache[iDBSpellsID]->parent_list = tmpparent_list;
					npc_spells_cache[iDBSpellsID]->attack_proc = tmpattack_proc;
					npc_spells_cache[iDBSpellsID]->proc_chance = tmpproc_chance;
					npc_spells_cache[iDBSpellsID]->numentries = mysql_num_rows(result);
					int j = 0;
					while ((row = mysql_fetch_row(result))) {
						int spell_id = atoi(row[0]);
						npc_spells_cache[iDBSpellsID]->entries[j].spellid = spell_id;
						npc_spells_cache[iDBSpellsID]->entries[j].type = atoi(row[1]);
						npc_spells_cache[iDBSpellsID]->entries[j].minlevel = atoi(row[2]);
						npc_spells_cache[iDBSpellsID]->entries[j].maxlevel = atoi(row[3]);
						npc_spells_cache[iDBSpellsID]->entries[j].manacost = atoi(row[4]);
						npc_spells_cache[iDBSpellsID]->entries[j].recast_delay = atoi(row[5]);
						npc_spells_cache[iDBSpellsID]->entries[j].priority = atoi(row[6]);
						if(row[7])
						{
							npc_spells_cache[iDBSpellsID]->entries[j].resist_adjust = atoi(row[7]);
						}
						else
						{
							if(IsValidSpell(spell_id))
							{
								npc_spells_cache[iDBSpellsID]->entries[j].resist_adjust = spells[spell_id].ResistDiff;
							}
						}
						j++;
					}
					mysql_free_result(result);
					return npc_spells_cache[iDBSpellsID];
				}
				else {
					std::cerr << "Error in AddNPCSpells query1 '" << query << "' " << errbuf << std::endl;
					safe_delete_array(query);
					return 0;
				}
			}
			else {
				mysql_free_result(result);
			}
		}
		else {
			std::cerr << "Error in AddNPCSpells query1 '" << query << "' " << errbuf << std::endl;
			safe_delete_array(query);
			return 0;
		}

		return 0;
	}
	return 0;
}

uint32 ZoneDatabase::GetMaxNPCSpellsID() {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (RunQuery(query, MakeAnyLenString(&query, "SELECT max(id) from npc_spells"), errbuf, &result)) {
		safe_delete_array(query);
		if (mysql_num_rows(result) == 1) {
			row = mysql_fetch_row(result);
			uint32 ret = 0;
			if (row[0])
				ret = atoi(row[0]);
			mysql_free_result(result);
			return ret;
		}
		mysql_free_result(result);
	}
	else {
		std::cerr << "Error in GetMaxNPCSpellsID query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return 0;
	}

	return 0;
}

