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

#include "../common/global_define.h"
#include "../common/features.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"

#include "client.h"
#include "entity.h"
#include "map.h"
#include "mob.h"
#include "npc.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "water_map.h"

#include <algorithm>
#include <iostream>
#include <math.h>

extern EntityList entity_list;

extern Zone *zone;

#if EQDEBUG >= 12
	#define MobAI_DEBUG_Spells	25
#elif EQDEBUG >= 9
	#define MobAI_DEBUG_Spells	10
#else
	#define MobAI_DEBUG_Spells	-1
#endif

//NOTE: do NOT pass in beneficial and detrimental spell types into the same call here!
bool NPC::AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes) {
	if (!tar)
		return false;

	if (IsNoCast())
		return false;

	if(AI_HasSpells() == false)
		return false;

	// Rooted mobs were just standing around when tar out of range.
	// Any sane mob would cast if they can.
	bool cast_only_option = (IsRooted() && !CombatRange(tar));

	if (!cast_only_option && iChance < 100) {
		if (zone->random.Int(0, 100) >= iChance)
			return false;
	}

	float dist2;

	if (iSpellTypes & SpellType_Escape) {
		dist2 = 0; //DistNoRoot(*this);	//WTF was up with this...
	}
	else
		dist2 = DistanceSquared(m_Position, tar->GetPosition());

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
				&& (AIspells[i].time_cancast + (zone->random.Int(0, 4) * 1000)) <= Timer::GetCurrentTime() //break up the spelling casting over a period of time.
				) {

#if MobAI_DEBUG_Spells >= 21
				Log(Logs::Detail, Logs::AI, "Mob::AICastSpell: Casting: spellid=%u, tar=%s, dist2[%f]<=%f, mana_cost[%i]<=%i, cancast[%u]<=%u, type=%u",
					AIspells[i].spellid, tar->GetName(), dist2, (spells[AIspells[i].spellid].range * spells[AIspells[i].spellid].range), mana_cost, GetMana(), AIspells[i].time_cancast, Timer::GetCurrentTime(), AIspells[i].type);
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
						if (rootee && !rootee->IsRooted() && !rootee->IsFeared() && zone->random.Roll(50)
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
						if(zone->random.Roll(50))
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
						if (debuffee && manaR >= 10 && zone->random.Roll(70) &&
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
							manaR >= 10 && zone->random.Roll(70)
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
						if(zone->random.Roll(15))
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
						if(zone->random.Roll(20))
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
						if(!IsPet() && zone->random.Roll(20))
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
						if (!IsPet() && !GetPetID() && zone->random.Roll(25)) {
							AIDoSpellCast(i, tar, mana_cost);
							return true;
						}
						break;
					}
					case SpellType_Lifetap: {
						if (GetHPRatio() <= 95
							&& zone->random.Roll(50)
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
							&& zone->random.Roll(50)
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
							zone->random.Roll(60)
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
				Log(Logs::Detail, Logs::AI, "Mob::AICastSpell: NotCasting: spellid=%u, tar=%s, dist2[%f]<=%f, mana_cost[%i]<=%i, cancast[%u]<=%u, type=%u",
					AIspells[i].spellid, tar->GetName(), dist2, (spells[AIspells[i].spellid].range * spells[AIspells[i].spellid].range), mana_cost, GetMana(), AIspells[i].time_cancast, Timer::GetCurrentTime(), AIspells[i].type);
			}
#endif
		}
	}
	return false;
}

bool NPC::AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore) {
#if MobAI_DEBUG_Spells >= 1
	Log(Logs::Detail, Logs::AI, "Mob::AIDoSpellCast: spellid = %u, tar = %s, mana = %i, Name: '%s'", AIspells[i].spellid, tar->GetName(), mana_cost, spells[AIspells[i].spellid].name);
#endif
	casting_spell_AIindex = i;

	//stop moving if were casting a spell and were not a bard...
	if(!IsBardSong(AIspells[i].spellid)) {
		SetCurrentSpeed(0);
	}

	return CastSpell(AIspells[i].spellid, tar->GetID(), EQEmu::CastingSlot::Gem2, AIspells[i].manacost == -2 ? 0 : -1, mana_cost, oDontDoAgainBefore, -1, -1, 0, &(AIspells[i].resist_adjust));
}

bool EntityList::AICheckCloseBeneficialSpells(NPC* caster, uint8 iChance, float iRange, uint32 iSpellTypes) {
	if((iSpellTypes & SpellTypes_Detrimental) != 0) {
		//according to live, you can buff and heal through walls...
		//now with PCs, this only applies if you can TARGET the target, but
		// according to Rogean, Live NPCs will just cast through walls/floors, no problem..
		//
		// This check was put in to address an idle-mob CPU issue
		Log(Logs::General, Logs::Error, "Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
		return(false);
	}

	if(!caster)
		return false;

	if(caster->AI_HasSpells() == false)
		return false;

	if(caster->GetSpecialAbility(NPC_NO_BUFFHEAL_FRIENDS))
		return false;

	if (iChance < 100) {
		uint8 tmp = zone->random.Int(0, 99);
		if (tmp >= iChance)
			return false;
	}
	if (caster->GetPrimaryFaction() == 0 )
		return(false); // well, if we dont have a faction set, we're gonna be indiff to everybody

	float iRange2 = iRange*iRange;

	//Only iterate through NPCs
	for (auto it = npc_list.begin(); it != npc_list.end(); ++it) {
		NPC* mob = it->second;
	
		if (mob->GetReverseFactionCon(caster) >= FACTION_KINDLY) {
			continue;
		}

		if (DistanceSquared(caster->GetPosition(), mob->GetPosition()) > iRange2) {
			continue;
		}

		if ((iSpellTypes & SpellType_Buff) && !RuleB(NPC, BuffFriends)) {
			if (mob != caster)
				iSpellTypes = SpellType_Heal;
		}

		if (caster->AICastSpell(mob, 100, iSpellTypes))
			return true;
	}
	return false;
}

void Mob::AI_Init()
{
	pAIControlled = false;
	AI_think_timer.reset(nullptr);
	AI_walking_timer.reset(nullptr);
	AI_movement_timer.reset(nullptr);
	AI_target_check_timer.reset(nullptr);
	AI_feign_remember_timer.reset(nullptr);
	AI_scan_area_timer.reset(nullptr);
	AI_check_signal_timer.reset(nullptr);

	minLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMin);
	maxLastFightingDelayMoving = RuleI(NPC, LastFightingDelayMovingMax);

	pDontHealMeBefore = 0;
	pDontBuffMeBefore = 0;
	pDontDotMeBefore = 0;
	pDontRootMeBefore = 0;
	pDontSnareMeBefore = 0;
	pDontCureMeBefore = 0;
}

void NPC::AI_Init()
{
	AIautocastspell_timer.reset(nullptr);
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

void Client::AI_Init()
{
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
	AI_think_timer = std::unique_ptr<Timer>(new Timer(AIthink_duration));
	AI_think_timer->Trigger();
	AI_walking_timer = std::unique_ptr<Timer>(new Timer(0));
	AI_movement_timer = std::unique_ptr<Timer>(new Timer(AImovement_duration));
	AI_target_check_timer = std::unique_ptr<Timer>(new Timer(AItarget_check_duration));
	AI_feign_remember_timer = std::unique_ptr<Timer>(new Timer(AIfeignremember_delay));

	if(CastToNPC()->WillAggroNPCs())
		AI_scan_area_timer = std::unique_ptr<Timer>(new Timer(RandomTimer(RuleI(NPC, NPCToNPCAggroTimerMin), RuleI(NPC, NPCToNPCAggroTimerMax))));
	
	AI_check_signal_timer = std::unique_ptr<Timer>(new Timer(AI_check_signal_timer_delay));


	if (GetAggroRange() == 0)
		pAggroRange = 70;
	if (GetAssistRange() == 0)
		pAssistRange = 70;

	hate_list.WipeHateList();

	m_Delta = glm::vec4();
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

	if (AIspells.empty()) {
		AIautocastspell_timer = std::unique_ptr<Timer>(new Timer(1000));
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer = std::unique_ptr<Timer>(new Timer(750));
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
	}

	if (NPCTypedata) {
		AI_AddNPCSpells(NPCTypedata->npc_spells_id);
		ProcessSpecialAbilities(NPCTypedata->special_abilities);
		AI_AddNPCSpellsEffects(NPCTypedata->npc_spells_effects_id);
	}

	SendTo(GetX(), GetY(), GetZ());
	SetChanged();
	SaveGuardSpot();
}

void Mob::AI_Stop() {
	if (!IsAIControlled())
		return;

	pAIControlled = false;

	AI_think_timer.reset(nullptr);
	AI_walking_timer.reset(nullptr);
	AI_movement_timer.reset(nullptr);
	AI_target_check_timer.reset(nullptr);
	AI_scan_area_timer.reset(nullptr);
	AI_feign_remember_timer.reset(nullptr);
	AI_check_signal_timer.reset(nullptr);

	hate_list.WipeHateList();
}

void NPC::AI_Stop() {
	Waypoints.clear();
	AIautocastspell_timer.reset(nullptr);
}

void Client::AI_Stop() {
	Mob::AI_Stop();
	this->Message_StringID(13,PLAYER_REGAIN);

	auto app = new EQApplicationPacket(OP_Charm, sizeof(Charm_Struct));
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

// only call this on a zone shutdown event
void Mob::AI_ShutDown() {
	safe_delete(PathingLOSCheckTimer);
	safe_delete(PathingRouteUpdateTimerShort);
	safe_delete(PathingRouteUpdateTimerLong);

	attack_timer.Disable();
	attack_dw_timer.Disable();
	ranged_timer.Disable();
	tic_timer.Disable();
	mana_timer.Disable();
	spellend_timer.Disable();
	rewind_timer.Disable();
	bindwound_timer.Disable();
	stunned_timer.Disable();
	spun_timer.Disable();
	bardsong_timer.Disable();
	gravity_timer.Disable();
	viral_timer.Disable();
	flee_timer.Disable();

	for (int sat = 0; sat < MAX_SPECIAL_ATTACK; ++sat) {
		if (SpecialAbilities[sat].timer)
			SpecialAbilities[sat].timer->Disable();
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

	float dist = DistanceSquaredNoZ(m_Position, targ->GetPosition());

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
	EQEmu::CastingSlot slot_to_use = EQEmu::CastingSlot::Item;
	if(valid_spells.size() == 1)
	{
		spell_to_cast = valid_spells[0];
		slot_to_use = static_cast<EQEmu::CastingSlot>(slots[0]);
	}
	else if(valid_spells.empty())
	{
		return;
	}
	else
	{
		uint32 idx = zone->random.Int(0, (valid_spells.size()-1));
		spell_to_cast = valid_spells[idx];
		slot_to_use = static_cast<EQEmu::CastingSlot>(slots[idx]);
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
					SetCurrentSpeed(0);
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
				SetCurrentSpeed(0);
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

	if (!(AI_think_timer->Check() || attack_timer.Check(false)))
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
		if(currently_fleeing) {

			if (fix_z_timer_engaged.Check())
				this->FixZ();

			if(IsRooted()) {
				//make sure everybody knows were not moving, for appearance sake
				if(IsMoving())
				{
					if(GetTarget())
						SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
					SetCurrentSpeed(0);
				}
				//continue on to attack code, ensuring that we execute the engaged code
				engaged = true;
			} else {
				if(AI_movement_timer->Check()) {
					int speed = GetFearSpeed();
					animation = speed;
					speed *= 2;
					SetCurrentSpeed(speed);
					// Check if we have reached the last fear point
					if ((std::abs(GetX() - m_FearWalkTarget.x) < 0.1) &&
					    (std::abs(GetY() - m_FearWalkTarget.y) < 0.1)) {
						// Calculate a new point to run to
						CalculateNewFearpoint();
					}
					if(!RuleB(Pathing, Fear) || !zone->pathing)
						CalculateNewPosition2(m_FearWalkTarget.x, m_FearWalkTarget.y, m_FearWalkTarget.z, speed, true);
					else
					{
						bool WaypointChanged, NodeReached;

						glm::vec3 Goal = UpdatePath(m_FearWalkTarget.x, m_FearWalkTarget.y, m_FearWalkTarget.z,
									speed, WaypointChanged, NodeReached);

						if(WaypointChanged)
							tar_ndx = 20;

						CalculateNewPosition2(Goal.x, Goal.y, Goal.z, speed);
					}
				}
				return;
			}

		}
	}

	if (engaged)
	{
		if (IsRooted())
			SetTarget(hate_list.GetClosestEntOnHateList(this));
		else
		{
			if(AI_target_check_timer->Check())
			{
				SetTarget(hate_list.GetEntWithMostHateOnList(this));
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

		if (is_combat_range) {
			if (charm_class_attacks_timer.Check()) {
				DoClassAttacks(GetTarget());
			}

			if (AI_movement_timer->Check()) {
				if (CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()) !=
				    m_Position.w) {
					SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
					SendPosition();
				}
				SetCurrentSpeed(0);
			}
			if (GetTarget() && !IsStunned() && !IsMezzed() && !GetFeigned()) {
				if (attack_timer.Check()) {
					// Should charmed clients not be procing?
					DoAttackRounds(GetTarget(), EQEmu::inventory::slotPrimary);
				}
			}

			if (CanThisClassDualWield() && GetTarget() && !IsStunned() && !IsMezzed() && !GetFeigned()) {
				if (attack_dw_timer.Check()) {
					if (CheckDualWield()) {
						// Should charmed clients not be procing?
						DoAttackRounds(GetTarget(), EQEmu::inventory::slotSecondary);
					}
				}
			}
		} else {
			if(!IsRooted())
			{
				if(AI_movement_timer->Check())
				{
					int newspeed = GetRunspeed();
					animation = newspeed;
					newspeed *= 2;
					SetCurrentSpeed(newspeed);
					if(!RuleB(Pathing, Aggro) || !zone->pathing)
						CalculateNewPosition2(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), newspeed);
					else
					{
						bool WaypointChanged, NodeReached;
						glm::vec3 Goal = UpdatePath(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(),
							GetRunspeed(), WaypointChanged, NodeReached);

						if(WaypointChanged)
							tar_ndx = 20;

						CalculateNewPosition2(Goal.x, Goal.y, Goal.z, newspeed);
					}
				}
			}
			else if(IsMoving())
			{
				SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
				SetCurrentSpeed(0);
			}
		}
		AI_SpellCast();
	}
	else
	{
		if(AI_feign_remember_timer->Check()) {
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

			float dist = DistanceSquared(m_Position, owner->GetPosition());
			if (dist >= 400)
			{
				if(AI_movement_timer->Check())
				{
					int nspeed = (dist >= 5625 ? GetRunspeed() : GetWalkspeed());
					animation = nspeed;
					nspeed *= 2;
					SetCurrentSpeed(nspeed);

					CalculateNewPosition2(owner->GetX(), owner->GetY(), owner->GetZ(), nspeed);
				}
			}
			else
			{
				if(moved)
				{
					SetCurrentSpeed(0);
					moved = false;
				}
			}
		}
	}
}

void Mob::AI_Process() {
	if (!IsAIControlled())
		return;

	if (!(AI_think_timer->Check() || attack_timer.Check(false)))
		return;

	if (IsCasting())
		return;

	bool engaged = IsEngaged();
	bool doranged = false;

	if (!zone->CanDoCombat() || IsPetStop() || IsPetRegroup()) {
		engaged = false;
	}

	// Begin: Additions for Wiz Fear Code
	//
	if(RuleB(Combat, EnableFearPathing)){
		if(currently_fleeing) {
			if((IsRooted() || (IsBlind() && CombatRange(hate_list.GetClosestEntOnHateList(this)))) && !IsPetStop() && !IsPetRegroup()) {
				//make sure everybody knows were not moving, for appearance sake
				if(IsMoving())
				{
					if(target)
						SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
					SetCurrentSpeed(0);
					moved=false;
				}
				//continue on to attack code, ensuring that we execute the engaged code
				engaged = true;
			} else {
				if(AI_movement_timer->Check()) {
					// Check if we have reached the last fear point
					if ((std::abs(GetX() - m_FearWalkTarget.x) < 0.1) &&
					    (std::abs(GetY() - m_FearWalkTarget.y) < 0.1)) {
						// Calculate a new point to run to
						CalculateNewFearpoint();
					}
					if(!RuleB(Pathing, Fear) || !zone->pathing)
					{
						CalculateNewPosition2(m_FearWalkTarget.x, m_FearWalkTarget.y, m_FearWalkTarget.z, GetFearSpeed(), true);
					}
					else
					{
						bool WaypointChanged, NodeReached;

						glm::vec3 Goal = UpdatePath(m_FearWalkTarget.x, m_FearWalkTarget.y, m_FearWalkTarget.z,
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
	if (AI_check_signal_timer->Check() && IsNPC()) {
		CastToNPC()->CheckSignal();
	}

	if (engaged) {

		/* Fix Z when following during pull, not when engaged and stationary */
		if (moving && fix_z_timer_engaged.Check()) {
			if (this->GetTarget()) {
				/* If we are engaged, moving and following client, let's look for best Z more often */
				float target_distance = DistanceNoZ(this->GetPosition(), this->GetTarget()->GetPosition());
				this->FixZ();

				if (target_distance <= 15 && !this->CheckLosFN(this->GetTarget())) {
					Mob* target = this->GetTarget();

					m_Position.x = target->GetX();
					m_Position.y = target->GetY();
					m_Position.z = target->GetZ();
					m_Position.w = target->GetHeading();
					SendPosition();
				}
			}
		}

		if (!(m_PlayerState & static_cast<uint32>(PlayerState::Aggressive)))
			SendAddPlayerState(PlayerState::Aggressive);
		// we are prevented from getting here if we are blind and don't have a target in range
		// from above, so no extra blind checks needed
		if ((IsRooted() && !GetSpecialAbility(IGNORE_ROOT_AGGRO_RULES)) || IsBlind())
			SetTarget(hate_list.GetClosestEntOnHateList(this));
		else
		{
			if (AI_target_check_timer->Check())
			{
				if (IsFocused()) {
					if (!target) {
						SetTarget(hate_list.GetEntWithMostHateOnList(this));
					}
				}
				else {
					if (!ImprovedTaunt())
						SetTarget(hate_list.GetEntWithMostHateOnList(this));
				}

			}
		}

		if (!target)
			return;

		if (target->IsCorpse()) {
			RemoveFromHateList(this);
			return;
		}

#ifdef BOTS
		if (IsPet() && GetOwner() && GetOwner()->IsBot() && target == GetOwner())
		{
			// this blocks all pet attacks against owner..bot pet test (copied above check)
			RemoveFromHateList(this);
			return;
		}
#endif //BOTS

		if (DivineAura())
			return;

		ProjectileAttack();

		auto npcSpawnPoint = CastToNPC()->GetSpawnPoint();
		if (GetSpecialAbility(TETHER)) {
			float tether_range = static_cast<float>(GetSpecialAbilityParam(TETHER, 0));
			tether_range = tether_range > 0.0f ? tether_range * tether_range : pAggroRange * pAggroRange;

			if (DistanceSquaredNoZ(m_Position, npcSpawnPoint) > tether_range) {
				GMMove(npcSpawnPoint.x, npcSpawnPoint.y, npcSpawnPoint.z, npcSpawnPoint.w);
			}
		}
		else if (GetSpecialAbility(LEASH)) {
			float leash_range = static_cast<float>(GetSpecialAbilityParam(LEASH, 0));
			leash_range = leash_range > 0.0f ? leash_range * leash_range : pAggroRange * pAggroRange;

			if (DistanceSquaredNoZ(m_Position, npcSpawnPoint) > leash_range) {
				GMMove(npcSpawnPoint.x, npcSpawnPoint.y, npcSpawnPoint.z, npcSpawnPoint.w);
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
			if (AI_movement_timer->Check())
			{
				if (CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()) != m_Position.w)
				{
					SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
					SendPosition();
				}
				SetCurrentSpeed(0);
			}
			if (IsMoving())
			{
				if (CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()) != m_Position.w)
				{
					SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
					SendPosition();
				}
				SetCurrentSpeed(0);
			}

			//casting checked above...
			if (target && !IsStunned() && !IsMezzed() && GetAppearance() != eaDead && !IsMeleeDisabled()) {

				//we should check to see if they die mid-attacks, previous
				//crap of checking target for null was not gunna cut it

				//try main hand first
				if (attack_timer.Check()) {
					DoMainHandAttackRounds(target);
					TriggerDefensiveProcs(target, EQEmu::inventory::slotPrimary, false);

					bool specialed = false; // NPCs can only do one of these a round
					if (GetSpecialAbility(SPECATK_FLURRY)) {
						int flurry_chance = GetSpecialAbilityParam(SPECATK_FLURRY, 0);
						flurry_chance = flurry_chance > 0 ? flurry_chance : RuleI(Combat, NPCFlurryChance);

						if (zone->random.Roll(flurry_chance)) {
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
							specialed = true;
						}
					}

					if (IsPet() || IsTempPet()) {
						Mob *owner = nullptr;
						owner = GetOwner();

						if (owner) {
							int16 flurry_chance = owner->aabonuses.PetFlurry +
								owner->spellbonuses.PetFlurry + owner->itembonuses.PetFlurry;

							if (flurry_chance && zone->random.Roll(flurry_chance))
								Flurry(nullptr);
						}
					}

					if ((IsPet() || IsTempPet()) && IsPetOwnerClient()) {
						if (spellbonuses.PC_Pet_Rampage[0] || itembonuses.PC_Pet_Rampage[0] || aabonuses.PC_Pet_Rampage[0]) {
							int chance = spellbonuses.PC_Pet_Rampage[0] + itembonuses.PC_Pet_Rampage[0] + aabonuses.PC_Pet_Rampage[0];
							if (zone->random.Roll(chance)) {
								Rampage(nullptr);
							}
						}
					}

					if (GetSpecialAbility(SPECATK_RAMPAGE) && !specialed)
					{
						int rampage_chance = GetSpecialAbilityParam(SPECATK_RAMPAGE, 0);
						rampage_chance = rampage_chance > 0 ? rampage_chance : 20;
						if (zone->random.Roll(rampage_chance)) {
							ExtraAttackOptions opts;
							int cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 3);
							if (cur > 0) {
								opts.damage_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 4);
							if (cur > 0) {
								opts.armor_pen_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 5);
							if (cur > 0) {
								opts.armor_pen_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 6);
							if (cur > 0) {
								opts.crit_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_RAMPAGE, 7);
							if (cur > 0) {
								opts.crit_flat = cur;
							}
							Rampage(&opts);
							specialed = true;
						}
					}

					if (GetSpecialAbility(SPECATK_AREA_RAMPAGE) && !specialed)
					{
						int rampage_chance = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 0);
						rampage_chance = rampage_chance > 0 ? rampage_chance : 20;
						if (zone->random.Roll(rampage_chance)) {
							ExtraAttackOptions opts;
							int cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 3);
							if (cur > 0) {
								opts.damage_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 4);
							if (cur > 0) {
								opts.armor_pen_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 5);
							if (cur > 0) {
								opts.armor_pen_flat = cur;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 6);
							if (cur > 0) {
								opts.crit_percent = cur / 100.0f;
							}

							cur = GetSpecialAbilityParam(SPECATK_AREA_RAMPAGE, 7);
							if (cur > 0) {
								opts.crit_flat = cur;
							}

							AreaRampage(&opts);
							specialed = true;
						}
					}
				}

				//now off hand
				if (attack_dw_timer.Check() && CanThisClassDualWield())
					DoOffHandAttackRounds(target);

				//now special attacks (kick, etc)
				if (IsNPC())
					CastToNPC()->DoClassAttacks(target);

			}
			AI_EngagedCastCheck();

		}	//end is within combat rangepet
		else {
			//we cannot reach our target...
			//underwater stuff only works with water maps in the zone!
			if (IsNPC() && CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
				auto targetPosition = glm::vec3(target->GetX(), target->GetY(), target->GetZ());
				if (!zone->watermap->InLiquid(targetPosition)) {
					Mob *tar = hate_list.GetEntWithMostHateOnList(this);
					if (tar == target) {
						WipeHateList();
						Heal();
						BuffFadeAll();
						AI_walking_timer->Start(100);
						pLastFightingDelayMoving = Timer::GetCurrentTime();
						return;
					}
					else if (tar != nullptr) {
						SetTarget(tar);
						return;
					}
				}
			}

			// See if we can summon the mob to us
			if (!HateSummon())
			{
				//could not summon them, check ranged...
				if (GetSpecialAbility(SPECATK_RANGED_ATK))
					doranged = true;

				// Now pursue
				// TODO: Check here for another person on hate list with close hate value
				if (AI_PursueCastCheck()) {
					//we did something, so do not process movement.
				}
				else if (AI_movement_timer->Check() && target)
				{
					if (!IsRooted()) {
						Log(Logs::Detail, Logs::AI, "Pursuing %s while engaged.", target->GetName());
						if (!RuleB(Pathing, Aggro) || !zone->pathing)
							CalculateNewPosition2(target->GetX(), target->GetY(), target->GetZ(), GetRunspeed());
						else
						{
							bool WaypointChanged, NodeReached;

							glm::vec3 Goal = UpdatePath(target->GetX(), target->GetY(), target->GetZ(),
								GetRunspeed(), WaypointChanged, NodeReached);

							if (WaypointChanged)
								tar_ndx = 20;

							CalculateNewPosition2(Goal.x, Goal.y, Goal.z, GetRunspeed());
						}

					}
					else if (IsMoving()) {
						SetHeading(CalculateHeadingToTarget(target->GetX(), target->GetY()));
						SetCurrentSpeed(0);

					}
				}
			}
		}
	}
	else {
		if (m_PlayerState & static_cast<uint32>(PlayerState::Aggressive))
			SendRemovePlayerState(PlayerState::Aggressive);

		if (IsPetStop()) // pet stop won't be engaged, so we will always get here and we want the above branch to execute
			return;

		if(zone->CanDoCombat() && AI_feign_remember_timer->Check()) {
			// 6/14/06
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
		else if (zone->CanDoCombat() && CastToNPC()->WillAggroNPCs() && AI_scan_area_timer->Check())
		{
			/*
			* NPC to NPC aggro checking, npc needs npc_aggro flag
			*/

			Mob* temp_target = entity_list.AICheckNPCtoNPCAggro(this, GetAggroRange(), GetAssistRange());
			if (temp_target){
				AddToHateList(temp_target);
			}

			AI_scan_area_timer->Disable();
			AI_scan_area_timer->Start(RandomTimer(RuleI(NPC, NPCToNPCAggroTimerMin), RuleI(NPC, NPCToNPCAggroTimerMax)), false);

		}
		else if (AI_movement_timer->Check() && !IsRooted())
		{
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

						glm::vec4 ownerPos = owner->GetPosition();
						float dist = DistanceSquared(m_Position, ownerPos);
						float distz = ownerPos.z - m_Position.z;

						if (dist >= 400 || distz > 100)
						{
							int speed = GetWalkspeed();
							if (dist >= 5625)
								speed = GetRunspeed();

							if (distz > 100)
							{
								m_Position = ownerPos;
								SendPositionUpdate();
								moved = true;
							}
							else
							{
							CalculateNewPosition2(owner->GetX(), 
								owner->GetY(), owner->GetZ(), speed);
							}
						}
						else
						{
							if(moved)
							{
								this->FixZ();
								SetCurrentSpeed(0);
								moved = false;
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
				if (IsPetRegroup())
					return;
			}
			/* Entity has been assigned another entity to follow */
			else if (GetFollowID())
			{
				Mob* follow = entity_list.GetMob(GetFollowID());
				if (!follow) SetFollowID(0);
				else
				{
					float dist2 = DistanceSquared(m_Position, follow->GetPosition());
					int followdist = GetFollowDistance();

					if (dist2 >= followdist)	// Default follow distance is 100
					{
						int speed = GetWalkspeed();
						if (dist2 >= followdist + 150)
							speed = GetRunspeed();
						CalculateNewPosition2(follow->GetX(), follow->GetY(), follow->GetZ(), speed);
					}
					else
					{
						moved = false;
						SetCurrentSpeed(0);
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
						/* All normal NPC pathing */
						CastToNPC()->AI_DoMovement();
					}
				}

			}
		} // else if (AImovement_timer->Check())
	}

	//Do Ranged attack here
	if(doranged)
	{
		RangedAttack(target);
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
			float movex = zone->random.Real(0, movedist);
			float movey = movedist - movex;
			movex = sqrtf(movex);
			movey = sqrtf(movey);
			movex *= zone->random.Int(0, 1) ? 1 : -1;
			movey *= zone->random.Int(0, 1) ? 1 : -1;
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
				roambox_movingto_x = zone->random.Real(roambox_min_x+1,roambox_max_x-1);
			if (roambox_movingto_y > roambox_max_y || roambox_movingto_y < roambox_min_y)
				roambox_movingto_y = zone->random.Real(roambox_min_y+1,roambox_max_y-1);
			Log(Logs::Detail, Logs::AI, 
				"Roam Box: d=%.3f (%.3f->%.3f,%.3f->%.3f): Go To (%.3f,%.3f)",
				roambox_distance, roambox_min_x, roambox_max_x, roambox_min_y, 
				roambox_max_y, roambox_movingto_x, roambox_movingto_y);
		}

		// Keep calling with updates, using wherever we are in Z.
		if (!MakeNewPositionAndSendUpdate(roambox_movingto_x, 
				roambox_movingto_y, m_Position.z, walksp))
		{
			this->FixZ(); // FixZ on final arrival point.
			roambox_movingto_x = roambox_max_x + 1; // force update
			pLastFightingDelayMoving = Timer::GetCurrentTime() + RandomTimer(roambox_min_delay, roambox_delay);
			SetMoving(false);
			SendPosition();	// makes mobs stop clientside
		}
	}
	else if (roamer)
	{
		if (AI_walking_timer->Check())
		{
			movetimercompleted=true;
			AI_walking_timer->Disable();
		}


		int32 gridno = CastToNPC()->GetGrid();

		if (gridno > 0 || cur_wp==-2) {
			if (movetimercompleted==true) { // time to pause at wp is over
				AI_SetupNextWaypoint();
			}	// endif (movetimercompleted==true)
			else if (!(AI_walking_timer->Enabled()))
			{	// currently moving
				bool doMove = true;
				if (m_CurrentWayPoint.x == GetX() && m_CurrentWayPoint.y == GetY())
				{	// are we there yet? then stop
					Log(Logs::Detail, Logs::AI, "We have reached waypoint %d (%.3f,%.3f,%.3f) on grid %d", cur_wp, GetX(), GetY(), GetZ(), GetGrid());
					
					SetWaypointPause();
					SetAppearance(eaStanding, false);
					SetMoving(false);
					if (m_CurrentWayPoint.w >= 0.0) {
						SetHeading(m_CurrentWayPoint.w);
					}

					this->FixZ();
					SendPosition();

					//kick off event_waypoint arrive
					char temp[16];
					sprintf(temp, "%d", cur_wp);
					parse->EventNPC(EVENT_WAYPOINT_ARRIVE, CastToNPC(), nullptr, temp, 0);
					// No need to move as we are there.  Next loop will
					// take care of normal grids, even at pause 0.
					// We do need to call and setup a wp if we're cur_wp=-2
					// as that is where roamer is unset and we don't want
					// the next trip through to move again based on grid stuff.
					doMove = false;
					if (cur_wp == -2) {
						AI_SetupNextWaypoint();
					}

					// wipe feign memory since we reached our first waypoint
					if(cur_wp == 1)
						ClearFeignMemory();
				}
				if (doMove)
				{	// not at waypoint yet or at 0 pause WP, so keep moving
					if(!RuleB(Pathing, AggroReturnToGrid) || !zone->pathing || (DistractedFromGrid == 0))
						CalculateNewPosition2(m_CurrentWayPoint.x, m_CurrentWayPoint.y, m_CurrentWayPoint.z, walksp, true);
					else
					{
						bool WaypointChanged;
						bool NodeReached;
						glm::vec3 Goal = UpdatePath(m_CurrentWayPoint.x, m_CurrentWayPoint.y, m_CurrentWayPoint.z, walksp, WaypointChanged, NodeReached);
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
				Log(Logs::Detail, Logs::Pathing, "Quest pathing is finished. Resuming on grid %d", GetGrid());

				SetAppearance(eaStanding, false);

				CalculateNewWaypoint();
			}
		}

	}
	else if (IsGuarding())
	{
		bool CP2Moved;
		if(!RuleB(Pathing, Guard) || !zone->pathing)
			CP2Moved = CalculateNewPosition2(m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z, walksp);
		else
		{
			if(!((m_Position.x == m_GuardPoint.x) && (m_Position.y == m_GuardPoint.y) && (m_Position.z == m_GuardPoint.z)))
			{
				bool WaypointChanged, NodeReached;
				glm::vec3 Goal = UpdatePath(m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z, walksp, WaypointChanged, NodeReached);
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
				Log(Logs::Detail, Logs::AI, "Reached guard point (%.3f,%.3f,%.3f)", m_GuardPoint.x, m_GuardPoint.y, m_GuardPoint.z);
				ClearFeignMemory();
				moved=false;
				if (GetTarget() == nullptr || DistanceSquared(m_Position, GetTarget()->GetPosition()) >= 5*5 )
				{
					SetHeading(m_GuardPoint.w);
				} else {
					FaceTarget(GetTarget());
				}
				SetCurrentSpeed(0);
				SetAppearance(GetGuardPointAnim());
			}
		}
	}
}

void NPC::AI_SetupNextWaypoint() {
	int32 spawn_id = this->GetSpawnPointID();
	LinkedListIterator<Spawn2*> iterator(zone->spawn2_list);
	iterator.Reset();
	Spawn2 *found_spawn = nullptr;

	while (iterator.MoreElements())
	{
		Spawn2* cur = iterator.GetData();
		iterator.Advance();
		if (cur->GetID() == spawn_id)
		{
			found_spawn = cur;
			break;
		}
	}

	if (wandertype == 4 && cur_wp == CastToNPC()->GetMaxWp()) {
		CastToNPC()->Depop(true); //depop and restart spawn timer
		if (found_spawn)
			found_spawn->SetNPCPointerNull();
	}
	else if (wandertype == 6 && cur_wp == CastToNPC()->GetMaxWp()) {
		CastToNPC()->Depop(false);//depop without spawn timer
		if (found_spawn)
			found_spawn->SetNPCPointerNull();
	}
	else {
		movetimercompleted = false;

		Log(Logs::Detail, Logs::Pathing, "We are departing waypoint %d.", cur_wp);

		//if we were under quest control (with no grid), we are done now..
		if (cur_wp == -2) {
			Log(Logs::Detail, Logs::Pathing, "Non-grid quest mob has reached its quest ordered waypoint. Leaving pathing mode.");
			roamer = false;
			cur_wp = 0;
		}

		SetAppearance(eaStanding, false);

		entity_list.OpenDoorsNear(CastToNPC());

		if (!DistractedFromGrid) {
			//kick off event_waypoint depart
			char temp[16];
			sprintf(temp, "%d", cur_wp);
			parse->EventNPC(EVENT_WAYPOINT_DEPART, CastToNPC(), nullptr, temp, 0);

			//setup our next waypoint, if we are still on our normal grid
			//remember that the quest event above could have done anything it wanted with our grid
			if (GetGrid() > 0) {
				CastToNPC()->CalculateNewWaypoint();
			}
		}
		else {
			DistractedFromGrid = false;
		}
	}
}

// Note: Mob that caused this may not get added to the hate list until after this function call completes
void Mob::AI_Event_Engaged(Mob* attacker, bool iYellForHelp) {
	if (!IsAIControlled())
		return;

	SetAppearance(eaStanding);

	if (iYellForHelp) {
		if(IsPet()) {
			GetOwner()->AI_Event_Engaged(attacker, iYellForHelp);
		} else if (!HasAssistAggro() && NPCAssistCap() < RuleI(Combat, NPCAssistCap)) {
			entity_list.AIYellForHelp(this, attacker);
			if (NPCAssistCap() > 0 && !assist_cap_timer.Enabled())
				assist_cap_timer.Start(RuleI(Combat, NPCAssistCapTimer));
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
	this->AI_walking_timer->Start(RandomTimer(3000,20000));
	pLastFightingDelayMoving = Timer::GetCurrentTime();
	if (minLastFightingDelayMoving == maxLastFightingDelayMoving)
		pLastFightingDelayMoving += minLastFightingDelayMoving;
	else
		pLastFightingDelayMoving += zone->random.Int(minLastFightingDelayMoving, maxLastFightingDelayMoving);
	// So mobs don't keep running as a ghost until AIwalking_timer fires
	// if they were moving prior to losing all hate
	// except if we're a pet, then we might run into some issues with pets backing off when they should immediately be moving
	if(!IsPet())
	{
		SetRunAnimSpeed(0);
		SendPosition();
	}
	ClearRampage();

	if(IsNPC())
	{
		SetPrimaryAggro(false);
		SetAssistAggro(false);
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
void NPC::AI_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot) {
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
			AIautocastspell_timer->Start(AISpellVar.fail_recast, false);
		casting_spell_AIindex = AIspells.size();
	}
}


bool NPC::AI_EngagedCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		Log(Logs::Detail, Logs::AI, "Engaged autocast check triggered. Trying to cast healing spells then maybe offensive spells.");

		// try casting a heal or gate
		if (!AICastSpell(this, AISpellVar.engaged_beneficial_self_chance, SpellType_Heal | SpellType_Escape | SpellType_InCombatBuff)) {
			// try casting a heal on nearby
			if (!entity_list.AICheckCloseBeneficialSpells(this, AISpellVar.engaged_beneficial_other_chance, MobAISpellRange, SpellType_Heal)) {
				//nobody to heal, try some detrimental spells.
				if(!AICastSpell(GetTarget(), AISpellVar.engaged_detrimental_chance, SpellType_Nuke | SpellType_Lifetap | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff | SpellType_Charm | SpellType_Root)) {
					//no spell to cast, try again soon.
					AIautocastspell_timer->Start(RandomTimer(AISpellVar.engaged_no_sp_recast_min, AISpellVar.engaged_no_sp_recast_max), false);
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

		Log(Logs::Detail, Logs::AI, "Engaged (pursuing) autocast check triggered. Trying to cast offensive spells.");
		if(!AICastSpell(GetTarget(), AISpellVar.pursue_detrimental_chance, SpellType_Root | SpellType_Nuke | SpellType_Lifetap | SpellType_Snare | SpellType_DOT | SpellType_Dispel | SpellType_Mez | SpellType_Slow | SpellType_Debuff)) {
			//no spell cast, try again soon.
			AIautocastspell_timer->Start(RandomTimer(AISpellVar.pursue_no_sp_recast_min, AISpellVar.pursue_no_sp_recast_max), false);
		} //else, spell casting finishing will reset the timer.
		return(true);
	}
	return(false);
}

bool NPC::AI_IdleCastCheck() {
	if (AIautocastspell_timer->Check(false)) {
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.
		if (!AICastSpell(this, AISpellVar.idle_beneficial_chance, SpellType_Heal | SpellType_Buff | SpellType_Pet)) {
			if(!entity_list.AICheckCloseBeneficialSpells(this, 33, MobAISpellRange, SpellType_Heal | SpellType_Buff)) {
				//if we didnt cast any spells, our autocast timer just resets to the
				//last duration it was set to... try to put up a more reasonable timer...
				AIautocastspell_timer->Start(RandomTimer(AISpellVar.idle_no_sp_recast_min, AISpellVar.idle_no_sp_recast_max), false);

				Log(Logs::Moderate, Logs::Spells, "Triggering AI_IdleCastCheck :: Mob %s - Min : %u Max : %u", this->GetCleanName(), AISpellVar.idle_no_sp_recast_min, AISpellVar.idle_no_sp_recast_max);

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
			Attack(target, EQEmu::inventory::slotPrimary, false, false, false, opts);
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

	m_specialattacks = eSpecialAttacks::Rampage;
	for (int i = 0; i < RampageArray.size(); i++) {
		if (index_hit >= rampage_targets)
			break;
		// range is important
		Mob *m_target = entity_list.GetMob(RampageArray[i]);
		if (m_target) {
			if (m_target == GetTarget())
				continue;
			if (CombatRange(m_target)) {
				ProcessAttackRounds(m_target, opts);
				index_hit++;
			}
		}
	}

	if (RuleB(Combat, RampageHitsTarget) && index_hit < rampage_targets)
		ProcessAttackRounds(GetTarget(), opts);

	m_specialattacks = eSpecialAttacks::None;

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
	rampage_targets = rampage_targets > 0 ? rampage_targets : -1;
	m_specialattacks = eSpecialAttacks::AERampage;
	index_hit = hate_list.AreaRampage(this, GetTarget(), rampage_targets, opts);

	if(index_hit == 0)
		ProcessAttackRounds(GetTarget(), opts);
	m_specialattacks = eSpecialAttacks::None;
}

uint32 Mob::GetLevelCon(uint8 mylevel, uint8 iOtherLevel) {

	uint32 conlevel = 0;

	if (RuleB(Character, UseOldConSystem))
	{
		int16 diff = iOtherLevel - mylevel;

		if (diff == 0)
			return CON_WHITE;
		else if (diff >= 1 && diff <= 2)
			return CON_YELLOW;
		else if (diff >= 3)
			return CON_RED;

		if (mylevel <= 8)
		{
			if (diff <= -4)
				conlevel = CON_GRAY;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 9)
		{
			if (diff <= -6)
				conlevel = CON_GRAY;
			else if (diff <= -4)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 13)
		{
			if (diff <= -7)
				conlevel = CON_GRAY;
			else if (diff <= -5)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 15)
		{
			if (diff <= -7)
				conlevel = CON_GRAY;
			else if (diff <= -5)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 17)
		{
			if (diff <= -8)
				conlevel = CON_GRAY;
			else if (diff <= -6)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 21)
		{
			if (diff <= -9)
				conlevel = CON_GRAY;
			else if (diff <= -7)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 25)
		{
			if (diff <= -10)
				conlevel = CON_GRAY;
			else if (diff <= -8)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 29)
		{
			if (diff <= -11)
				conlevel = CON_GRAY;
			else if (diff <= -9)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 31)
		{
			if (diff <= -12)
				conlevel = CON_GRAY;
			else if (diff <= -9)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 33)
		{
			if (diff <= -13)
				conlevel = CON_GRAY;
			else if (diff <= -10)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 37)
		{
			if (diff <= -14)
				conlevel = CON_GRAY;
			else if (diff <= -11)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 41)
		{
			if (diff <= -16)
				conlevel = CON_GRAY;
			else if (diff <= -12)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 45)
		{
			if (diff <= -17)
				conlevel = CON_GRAY;
			else if (diff <= -13)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 49)
		{
			if (diff <= -18)
				conlevel = CON_GRAY;
			else if (diff <= -14)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 53)
		{
			if (diff <= -19)
				conlevel = CON_GRAY;
			else if (diff <= -15)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else if (mylevel <= 55)
		{
			if (diff <= -20)
				conlevel = CON_GRAY;
			else if (diff <= -15)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
		else
		{
			if (diff <= -21)
				conlevel = CON_GRAY;
			else if (diff <= -16)
				conlevel = CON_LIGHTBLUE;
			else
				conlevel = CON_BLUE;
		}
	}
	else
	{
		int16 diff = iOtherLevel - mylevel;
		uint32 conGrayLvl = mylevel - (int32)((mylevel + 5) / 3);
		uint32 conGreenLvl = mylevel - (int32)((mylevel + 7) / 4);

		if (diff == 0)
			return CON_WHITE;
		else if (diff >= 1 && diff <= 3)
			return CON_YELLOW;
		else if (diff >= 4)
			return CON_RED;

		if (mylevel <= 15)
		{
			if (diff <= -6)
				conlevel = CON_GRAY;
			else
				conlevel = CON_BLUE;
		}
		else
			if (mylevel <= 20)
			{
				if (iOtherLevel <= conGrayLvl)
					conlevel = CON_GRAY;
				else
					if (iOtherLevel <= conGreenLvl)
						conlevel = CON_GREEN;
					else
						conlevel = CON_BLUE;
			}
			else
			{
				if (iOtherLevel <= conGrayLvl)
					conlevel = CON_GRAY;
				else
					if (iOtherLevel <= conGreenLvl)
						conlevel = CON_GREEN;
					else
						if (diff <= -6)
							conlevel = CON_LIGHTBLUE;
						else
							conlevel = CON_BLUE;
			}
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
bool IsSpellEffectInList(DBnpcspellseffects_Struct* spelleffect_list, uint16 iSpellEffectID, int32 base, int32 limit, int32 max);

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
	std::string debug_msg = StringFormat("Loading NPCSpells onto %s: dbspellsid=%u", this->GetName(), iDBSpellsID);
	if (spell_list) {
		debug_msg.append(StringFormat(" (found, %u), parentlist=%u", spell_list->numentries, spell_list->parent_list));
		if (spell_list->parent_list) {
			if (parentlist)
				debug_msg.append(StringFormat(" (found, %u)", parentlist->numentries));
			else
				debug_msg.append(" (not found)");
		}
	}
	else {
		debug_msg.append(" (not found)");
	}
	Log(Logs::Detail, Logs::AI, "%s", debug_msg.c_str());
#endif
	uint16 attack_proc_spell = -1;
	int8 proc_chance = 3;
	uint16 range_proc_spell = -1;
	int16 rproc_chance = 0;
	uint16 defensive_proc_spell = -1;
	int16 dproc_chance = 0;
	uint32 _fail_recast = 0;
	uint32 _engaged_no_sp_recast_min = 0;
	uint32 _engaged_no_sp_recast_max = 0;
	uint8 _engaged_beneficial_self_chance = 0;
	uint8 _engaged_beneficial_other_chance = 0;
	uint8 _engaged_detrimental_chance = 0;
	uint32 _pursue_no_sp_recast_min = 0;
	uint32 _pursue_no_sp_recast_max = 0;
	uint8 _pursue_detrimental_chance = 0;
	uint32 _idle_no_sp_recast_min = 0;
	uint32 _idle_no_sp_recast_max = 0;
	uint8 _idle_beneficial_chance = 0;

	if (parentlist) {
		attack_proc_spell = parentlist->attack_proc;
		proc_chance = parentlist->proc_chance;
		range_proc_spell = parentlist->range_proc;
		rproc_chance = parentlist->rproc_chance;
		defensive_proc_spell = parentlist->defensive_proc;
		dproc_chance = parentlist->dproc_chance;
		_fail_recast = parentlist->fail_recast;
		_engaged_no_sp_recast_min = parentlist->engaged_no_sp_recast_min;
		_engaged_no_sp_recast_max = parentlist->engaged_no_sp_recast_max;
		_engaged_beneficial_self_chance = parentlist->engaged_beneficial_self_chance;
		_engaged_beneficial_other_chance = parentlist->engaged_beneficial_other_chance;
		_engaged_detrimental_chance = parentlist->engaged_detrimental_chance;
		_pursue_no_sp_recast_min = parentlist->pursue_no_sp_recast_min;
		_pursue_no_sp_recast_max = parentlist->pursue_no_sp_recast_max;
		_pursue_detrimental_chance = parentlist->pursue_detrimental_chance;
		_idle_no_sp_recast_min = parentlist->idle_no_sp_recast_min;
		_idle_no_sp_recast_max = parentlist->idle_no_sp_recast_max;
		_idle_beneficial_chance = parentlist->idle_beneficial_chance;
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

	if (spell_list->range_proc >= 0) {
		range_proc_spell = spell_list->range_proc;
		rproc_chance = spell_list->rproc_chance;
	}

	if (spell_list->defensive_proc >= 0) {
		defensive_proc_spell = spell_list->defensive_proc;
		dproc_chance = spell_list->dproc_chance;
	}

	//If any casting variables are defined in the current list, ignore those in the parent list.
	if (spell_list->fail_recast || spell_list->engaged_no_sp_recast_min || spell_list->engaged_no_sp_recast_max
		|| spell_list->engaged_beneficial_self_chance || spell_list->engaged_beneficial_other_chance || spell_list->engaged_detrimental_chance
		|| spell_list->pursue_no_sp_recast_min || spell_list->pursue_no_sp_recast_max || spell_list->pursue_detrimental_chance
		|| spell_list->idle_no_sp_recast_min || spell_list->idle_no_sp_recast_max || spell_list->idle_beneficial_chance) {
		_fail_recast = spell_list->fail_recast;
		_engaged_no_sp_recast_min = spell_list->engaged_no_sp_recast_min;
		_engaged_no_sp_recast_max = spell_list->engaged_no_sp_recast_max;
		_engaged_beneficial_self_chance = spell_list->engaged_beneficial_self_chance;
		_engaged_beneficial_other_chance = spell_list->engaged_beneficial_other_chance;
		_engaged_detrimental_chance = spell_list->engaged_detrimental_chance;
		_pursue_no_sp_recast_min = spell_list->pursue_no_sp_recast_min;
		_pursue_no_sp_recast_max = spell_list->pursue_no_sp_recast_max;
		_pursue_detrimental_chance = spell_list->pursue_detrimental_chance;
		_idle_no_sp_recast_min = spell_list->idle_no_sp_recast_min;
		_idle_no_sp_recast_max = spell_list->idle_no_sp_recast_max;
		_idle_beneficial_chance = spell_list->idle_beneficial_chance;
	}

	for (i=0; i<spell_list->numentries; i++) {
		if (GetLevel() >= spell_list->entries[i].minlevel && GetLevel() <= spell_list->entries[i].maxlevel && spell_list->entries[i].spellid > 0) {
			AddSpellToNPCList(spell_list->entries[i].priority,
				spell_list->entries[i].spellid, spell_list->entries[i].type,
				spell_list->entries[i].manacost, spell_list->entries[i].recast_delay,
				spell_list->entries[i].resist_adjust);
		}
	}
	std::sort(AIspells.begin(), AIspells.end(), [](const AISpells_Struct& a, const AISpells_Struct& b) {
		return a.priority > b.priority;
	});

	if (IsValidSpell(attack_proc_spell)) {
		AddProcToWeapon(attack_proc_spell, true, proc_chance);

		if(RuleB(Spells, NPCInnateProcOverride))
			innate_proc_spell_id = attack_proc_spell;
	}

	if (IsValidSpell(range_proc_spell))
		AddRangedProc(range_proc_spell, (rproc_chance + 100));

	if (IsValidSpell(defensive_proc_spell))
		AddDefensiveProc(defensive_proc_spell, (dproc_chance + 100));

	//Set AI casting variables

	AISpellVar.fail_recast = (_fail_recast) ? _fail_recast : RuleI(Spells, AI_SpellCastFinishedFailRecast);
	AISpellVar.engaged_no_sp_recast_min = (_engaged_no_sp_recast_min) ? _engaged_no_sp_recast_min : RuleI(Spells, AI_EngagedNoSpellMinRecast);
	AISpellVar.engaged_no_sp_recast_max = (_engaged_no_sp_recast_max) ? _engaged_no_sp_recast_max : RuleI(Spells, AI_EngagedNoSpellMaxRecast);
	AISpellVar.engaged_beneficial_self_chance = (_engaged_beneficial_self_chance) ? _engaged_beneficial_self_chance : RuleI(Spells, AI_EngagedBeneficialSelfChance);
	AISpellVar.engaged_beneficial_other_chance = (_engaged_beneficial_other_chance) ? _engaged_beneficial_other_chance : RuleI(Spells, AI_EngagedBeneficialOtherChance);
	AISpellVar.engaged_detrimental_chance = (_engaged_detrimental_chance) ? _engaged_detrimental_chance : RuleI(Spells, AI_EngagedDetrimentalChance);
	AISpellVar.pursue_no_sp_recast_min = (_pursue_no_sp_recast_min) ? _pursue_no_sp_recast_min : RuleI(Spells, AI_PursueNoSpellMinRecast);
	AISpellVar.pursue_no_sp_recast_max = (_pursue_no_sp_recast_max) ? _pursue_no_sp_recast_max : RuleI(Spells, AI_PursueNoSpellMaxRecast);
	AISpellVar.pursue_detrimental_chance = (_pursue_detrimental_chance) ? _pursue_detrimental_chance : RuleI(Spells, AI_PursueDetrimentalChance);
	AISpellVar.idle_no_sp_recast_min = (_idle_no_sp_recast_min) ? _idle_no_sp_recast_min : RuleI(Spells, AI_IdleNoSpellMinRecast);
	AISpellVar.idle_no_sp_recast_max = (_idle_no_sp_recast_max) ? _idle_no_sp_recast_max : RuleI(Spells, AI_IdleNoSpellMaxRecast);
	AISpellVar.idle_beneficial_chance = (_idle_beneficial_chance) ? _idle_beneficial_chance : RuleI(Spells, AI_IdleBeneficialChance);

	if (AIspells.empty())
		AIautocastspell_timer->Disable();
	else
		AIautocastspell_timer->Trigger();
	return true;
}

bool NPC::AI_AddNPCSpellsEffects(uint32 iDBSpellsEffectsID) {

	npc_spells_effects_id = iDBSpellsEffectsID;
	AIspellsEffects.clear();

	if (iDBSpellsEffectsID == 0)
		return false;

	DBnpcspellseffects_Struct* spell_effects_list = database.GetNPCSpellsEffects(iDBSpellsEffectsID);

	if (!spell_effects_list) {
		return false;
	}

	DBnpcspellseffects_Struct* parentlist = database.GetNPCSpellsEffects(spell_effects_list->parent_list);

	uint32 i;
#if MobAI_DEBUG_Spells >= 10
	std::string debug_msg = StringFormat("Loading NPCSpellsEffects onto %s: dbspellseffectid=%u", this->GetName(), iDBSpellsEffectsID);
	if (spell_effects_list) {
		debug_msg.append(StringFormat(" (found, %u), parentlist=%u", spell_effects_list->numentries, spell_effects_list->parent_list));
		if (spell_effects_list->parent_list) {
			if (parentlist)
				debug_msg.append(StringFormat(" (found, %u)", parentlist->numentries));
			else
				debug_msg.append(" (not found)");
		}
	}
	else {
		debug_msg.append(" (not found)");
	}
	Log(Logs::Detail, Logs::AI, "%s", debug_msg.c_str());
#endif

	if (parentlist) {
		for (i=0; i<parentlist->numentries; i++) {
			if (GetLevel() >= parentlist->entries[i].minlevel && GetLevel() <= parentlist->entries[i].maxlevel && parentlist->entries[i].spelleffectid > 0) {
				if (!IsSpellEffectInList(spell_effects_list, parentlist->entries[i].spelleffectid, parentlist->entries[i].base,
					parentlist->entries[i].limit, parentlist->entries[i].max))
				{
				AddSpellEffectToNPCList(parentlist->entries[i].spelleffectid,
						parentlist->entries[i].base, parentlist->entries[i].limit,
						parentlist->entries[i].max);
				}
			}
		}
	}

	for (i=0; i<spell_effects_list->numentries; i++) {
		if (GetLevel() >= spell_effects_list->entries[i].minlevel && GetLevel() <= spell_effects_list->entries[i].maxlevel && spell_effects_list->entries[i].spelleffectid > 0) {
			AddSpellEffectToNPCList(spell_effects_list->entries[i].spelleffectid,
				spell_effects_list->entries[i].base, spell_effects_list->entries[i].limit,
				spell_effects_list->entries[i].max);
		}
	}

	return true;
}

void NPC::ApplyAISpellEffects(StatBonuses* newbon)
{
	if (!AI_HasSpellsEffects())
		return;

	for (int i = 0; i < AIspellsEffects.size(); i++)
		ApplySpellsBonuses(0, 0, newbon, 0, 0, 0, -1, 10, true, AIspellsEffects[i].spelleffectid,
				   AIspellsEffects[i].base, AIspellsEffects[i].limit, AIspellsEffects[i].max);

	return;
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void NPC::AddSpellEffectToNPCList(uint16 iSpellEffectID, int32 base, int32 limit, int32 max)
{

	if(!iSpellEffectID)
		return;

	HasAISpellEffects = true;
	AISpellsEffects_Struct t;

	t.spelleffectid = iSpellEffectID;
	t.base = base;
	t.limit = limit;
	t.max = max;
	AIspellsEffects.push_back(t);
}

bool IsSpellEffectInList(DBnpcspellseffects_Struct* spelleffect_list, uint16 iSpellEffectID, int32 base, int32 limit, int32 max) {
	for (uint32 i=0; i < spelleffect_list->numentries; i++) {
		if (spelleffect_list->entries[i].spelleffectid == iSpellEffectID &&  spelleffect_list->entries[i].base == base
			&& spelleffect_list->entries[i].limit == limit && spelleffect_list->entries[i].max == max)
			return true;
	}
	return false;
}

bool IsSpellInList(DBnpcspells_Struct* spell_list, int16 iSpellID) {
	for (uint32 i=0; i < spell_list->numentries; i++) {
		if (spell_list->entries[i].spellid == iSpellID)
			return true;
	}
	return false;
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void NPC::AddSpellToNPCList(int16 iPriority, int16 iSpellID, uint32 iType,
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

	// If we're going from an empty list, we need to start the timer
	if (AIspells.size() == 1)
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
}

void NPC::RemoveSpellFromNPCList(int16 spell_id)
{
	auto iter = AIspells.begin();
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

	for (auto it = AIspells.begin(); it != AIspells.end(); ++it)
		c->Message(0, "%s (%d): Type %d, Priority %d",
				spells[it->spellid].name, it->spellid, it->type, it->priority);

	return;
}

DBnpcspells_Struct* ZoneDatabase::GetNPCSpells(uint32 iDBSpellsID) {
	if (iDBSpellsID == 0)
		return nullptr;

	if (!npc_spells_cache) {
		npc_spells_maxid = GetMaxNPCSpellsID();
		npc_spells_cache = new DBnpcspells_Struct*[npc_spells_maxid+1];
		npc_spells_loadtried = new bool[npc_spells_maxid+1];
		for (uint32 i=0; i<=npc_spells_maxid; i++) {
			npc_spells_cache[i] = nullptr;
			npc_spells_loadtried[i] = false;
		}
	}

	if (iDBSpellsID > npc_spells_maxid)
		return nullptr;
	if (npc_spells_cache[iDBSpellsID]) { // it's in the cache, easy =)
		return npc_spells_cache[iDBSpellsID];
	}

	else if (!npc_spells_loadtried[iDBSpellsID]) { // no reason to ask the DB again if we have failed once already
		npc_spells_loadtried[iDBSpellsID] = true;

		std::string query = StringFormat("SELECT id, parent_list, attack_proc, proc_chance, "
                                        "range_proc, rproc_chance, defensive_proc, dproc_chance, "
                                        "fail_recast, engaged_no_sp_recast_min, engaged_no_sp_recast_max, "
                                        "engaged_b_self_chance, engaged_b_other_chance, engaged_d_chance, "
                                        "pursue_no_sp_recast_min, pursue_no_sp_recast_max, "
                                        "pursue_d_chance, idle_no_sp_recast_min, idle_no_sp_recast_max, "
                                        "idle_b_chance FROM npc_spells WHERE id=%d", iDBSpellsID);
        auto results = QueryDatabase(query);
        if (!results.Success()) {
			return nullptr;
        }

        if (results.RowCount() != 1)
            return nullptr;

        auto row = results.begin();
        uint32 tmpparent_list = atoi(row[1]);
        uint16 tmpattack_proc = atoi(row[2]);
        uint8 tmpproc_chance = atoi(row[3]);
        uint16 tmprange_proc = atoi(row[4]);
        int16 tmprproc_chance = atoi(row[5]);
        uint16 tmpdefensive_proc = atoi(row[6]);
        int16 tmpdproc_chance = atoi(row[7]);
        uint32 tmppfail_recast = atoi(row[8]);
        uint32 tmpengaged_no_sp_recast_min = atoi(row[9]);
        uint32 tmpengaged_no_sp_recast_max = atoi(row[10]);
        uint8 tmpengaged_b_self_chance = atoi(row[11]);
        uint8 tmpengaged_b_other_chance = atoi(row[12]);
        uint8 tmpengaged_d_chance = atoi(row[13]);
        uint32 tmppursue_no_sp_recast_min = atoi(row[14]);
        uint32 tmppursue_no_sp_recast_max = atoi(row[15]);
        uint8 tmppursue_d_chance = atoi(row[16]);
        uint32 tmpidle_no_sp_recast_min = atoi(row[17]);
        uint32 tmpidle_no_sp_recast_max = atoi(row[18]);
        uint8 tmpidle_b_chance = atoi(row[19]);

		// pulling fixed values from an auto-increment field is dangerous...
        query = StringFormat("SELECT spellid, type, minlevel, maxlevel, "
                            "manacost, recast_delay, priority, resist_adjust "
#ifdef BOTS
							"FROM %s "
							"WHERE npc_spells_id=%d ORDER BY minlevel", (iDBSpellsID >= 3001 && iDBSpellsID <= 3016 ? "bot_spells_entries" : "npc_spells_entries"), iDBSpellsID);
#else
                            "FROM npc_spells_entries "
							"WHERE npc_spells_id=%d ORDER BY minlevel", iDBSpellsID);
#endif
        results = QueryDatabase(query);

        if (!results.Success())
        {
			return nullptr;
        }

        uint32 tmpSize = sizeof(DBnpcspells_Struct) + (sizeof(DBnpcspells_entries_Struct) * results.RowCount());
        npc_spells_cache[iDBSpellsID] = (DBnpcspells_Struct*) new uchar[tmpSize];
        memset(npc_spells_cache[iDBSpellsID], 0, tmpSize);
        npc_spells_cache[iDBSpellsID]->parent_list = tmpparent_list;
        npc_spells_cache[iDBSpellsID]->attack_proc = tmpattack_proc;
        npc_spells_cache[iDBSpellsID]->proc_chance = tmpproc_chance;
        npc_spells_cache[iDBSpellsID]->range_proc = tmprange_proc;
        npc_spells_cache[iDBSpellsID]->rproc_chance = tmprproc_chance;
        npc_spells_cache[iDBSpellsID]->defensive_proc = tmpdefensive_proc;
        npc_spells_cache[iDBSpellsID]->dproc_chance = tmpdproc_chance;
        npc_spells_cache[iDBSpellsID]->fail_recast = tmppfail_recast;
        npc_spells_cache[iDBSpellsID]->engaged_no_sp_recast_min = tmpengaged_no_sp_recast_min;
        npc_spells_cache[iDBSpellsID]->engaged_no_sp_recast_max = tmpengaged_no_sp_recast_max;
        npc_spells_cache[iDBSpellsID]->engaged_beneficial_self_chance = tmpengaged_b_self_chance;
        npc_spells_cache[iDBSpellsID]->engaged_beneficial_other_chance = tmpengaged_b_other_chance;
        npc_spells_cache[iDBSpellsID]->engaged_detrimental_chance = tmpengaged_d_chance;
        npc_spells_cache[iDBSpellsID]->pursue_no_sp_recast_min = tmppursue_no_sp_recast_min;
        npc_spells_cache[iDBSpellsID]->pursue_no_sp_recast_max = tmppursue_no_sp_recast_max;
        npc_spells_cache[iDBSpellsID]->pursue_detrimental_chance = tmppursue_d_chance;
        npc_spells_cache[iDBSpellsID]->idle_no_sp_recast_min = tmpidle_no_sp_recast_min;
        npc_spells_cache[iDBSpellsID]->idle_no_sp_recast_max = tmpidle_no_sp_recast_max;
        npc_spells_cache[iDBSpellsID]->idle_beneficial_chance = tmpidle_b_chance;
        npc_spells_cache[iDBSpellsID]->numentries = results.RowCount();

        int entryIndex = 0;
        for (row = results.begin(); row != results.end(); ++row, ++entryIndex)
        {
            int spell_id = atoi(row[0]);
            npc_spells_cache[iDBSpellsID]->entries[entryIndex].spellid = spell_id;
            npc_spells_cache[iDBSpellsID]->entries[entryIndex].type = atoul(row[1]);
            npc_spells_cache[iDBSpellsID]->entries[entryIndex].minlevel = atoi(row[2]);
            npc_spells_cache[iDBSpellsID]->entries[entryIndex].maxlevel = atoi(row[3]);
            npc_spells_cache[iDBSpellsID]->entries[entryIndex].manacost = atoi(row[4]);
            npc_spells_cache[iDBSpellsID]->entries[entryIndex].recast_delay = atoi(row[5]);
            npc_spells_cache[iDBSpellsID]->entries[entryIndex].priority = atoi(row[6]);

            if(row[7])
                npc_spells_cache[iDBSpellsID]->entries[entryIndex].resist_adjust = atoi(row[7]);
            else if(IsValidSpell(spell_id))
                npc_spells_cache[iDBSpellsID]->entries[entryIndex].resist_adjust = spells[spell_id].ResistDiff;
        }

        return npc_spells_cache[iDBSpellsID];
    }

	return nullptr;
}

uint32 ZoneDatabase::GetMaxNPCSpellsID() {

	std::string query = "SELECT max(id) from npc_spells";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

    if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();

    if (!row[0])
        return 0;

    return atoi(row[0]);
}

DBnpcspellseffects_Struct *ZoneDatabase::GetNPCSpellsEffects(uint32 iDBSpellsEffectsID)
{
	if (iDBSpellsEffectsID == 0)
		return nullptr;

	if (!npc_spellseffects_cache) {
		npc_spellseffects_maxid = GetMaxNPCSpellsEffectsID();
		npc_spellseffects_cache = new DBnpcspellseffects_Struct *[npc_spellseffects_maxid + 1];
		npc_spellseffects_loadtried = new bool[npc_spellseffects_maxid + 1];
		for (uint32 i = 0; i <= npc_spellseffects_maxid; i++) {
			npc_spellseffects_cache[i] = nullptr;
			npc_spellseffects_loadtried[i] = false;
		}
	}

	if (iDBSpellsEffectsID > npc_spellseffects_maxid)
		return nullptr;

	if (npc_spellseffects_cache[iDBSpellsEffectsID]) // it's in the cache, easy =)
		return npc_spellseffects_cache[iDBSpellsEffectsID];

	if (npc_spellseffects_loadtried[iDBSpellsEffectsID])
		return nullptr;

	npc_spellseffects_loadtried[iDBSpellsEffectsID] = true;

	std::string query =
	    StringFormat("SELECT id, parent_list FROM npc_spells_effects WHERE id=%d", iDBSpellsEffectsID);
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return nullptr;
	}

	if (results.RowCount() != 1)
		return nullptr;

	auto row = results.begin();
	uint32 tmpparent_list = atoi(row[1]);

	query = StringFormat("SELECT spell_effect_id, minlevel, "
			     "maxlevel,se_base, se_limit, se_max "
			     "FROM npc_spells_effects_entries "
			     "WHERE npc_spells_effects_id = %d ORDER BY minlevel",
			     iDBSpellsEffectsID);
	results = QueryDatabase(query);
	if (!results.Success())
		return nullptr;

	uint32 tmpSize =
	    sizeof(DBnpcspellseffects_Struct) + (sizeof(DBnpcspellseffects_entries_Struct) * results.RowCount());
	npc_spellseffects_cache[iDBSpellsEffectsID] = (DBnpcspellseffects_Struct *)new uchar[tmpSize];
	memset(npc_spellseffects_cache[iDBSpellsEffectsID], 0, tmpSize);
	npc_spellseffects_cache[iDBSpellsEffectsID]->parent_list = tmpparent_list;
	npc_spellseffects_cache[iDBSpellsEffectsID]->numentries = results.RowCount();

	int entryIndex = 0;
	for (row = results.begin(); row != results.end(); ++row, ++entryIndex) {
		int spell_effect_id = atoi(row[0]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].spelleffectid = spell_effect_id;
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].minlevel = atoi(row[1]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].maxlevel = atoi(row[2]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].base = atoi(row[3]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].limit = atoi(row[4]);
		npc_spellseffects_cache[iDBSpellsEffectsID]->entries[entryIndex].max = atoi(row[5]);
	}

	return npc_spellseffects_cache[iDBSpellsEffectsID];
}

uint32 ZoneDatabase::GetMaxNPCSpellsEffectsID() {

	std::string query = "SELECT max(id) FROM npc_spells_effects";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return 0;
	}

    if (results.RowCount() != 1)
        return 0;

    auto row = results.begin();
    if (!row[0])
        return 0;

    return atoi(row[0]);
}

