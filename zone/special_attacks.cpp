/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemulator.net)

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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "masterentity.h"
#include "StringIDs.h"
#include "../common/MiscFunctions.h"
#include "../common/rulesys.h"




int Mob::GetKickDamage() {
	int multiple=(GetLevel()*100/5);
	multiple += 100;
	int32 dmg=(((GetSkill(SkillKick) + GetSTR() + GetLevel())*100 / 9000) * multiple) + 600;	//Set a base of 6 damage, 1 seemed too low at the sub level 30 level.
	if(GetClass() == WARRIOR || GetClass() == WARRIORGM ||GetClass() == BERSERKER || GetClass() == BERSERKERGM) {
		dmg*=12/10;//small increase for warriors
	}
	dmg /= 100;

	int32 mindmg = 1;
	ApplySpecialAttackMod(SkillKick, dmg,mindmg);

	dmg = mod_kick_damage(dmg);

	return(dmg);
}

int Mob::GetBashDamage() {
	int multiple=(GetLevel()*100/5);
	multiple += 100;

	//this is complete shite
	int32 dmg=((((GetSkill(SkillBash) + GetSTR())*100 + GetLevel()*100/2) / 10000) * multiple) + 600;	//Set a base of 6 damage, 1 seemed too low at the sub level 30 level.
	dmg /= 100;

	int32 mindmg = 1;
	ApplySpecialAttackMod(SkillBash, dmg, mindmg);

	dmg = mod_bash_damage(dmg);

	return(dmg);
}

void Mob::ApplySpecialAttackMod(SkillUseTypes skill, int32 &dmg, int32 &mindmg) {

	int item_slot = -1;
	//1: Apply bonus from AC (BOOT/SHIELD/HANDS) est. 40AC=6dmg
	if (IsClient()){

		switch (skill){

			case SkillFlyingKick:
			case SkillRoundKick:
			case SkillKick:
				item_slot = SLOT_FEET;
				break;

			case SkillBash:
				item_slot = SLOT_SECONDARY;
				break;

			case SkillDragonPunch:
			case SkillEagleStrike:
			case SkillTigerClaw:
				item_slot = SLOT_HANDS;
				break;

			default:
				break;
		}

		if (item_slot >= 0){
			const ItemInst* itm = nullptr;
			itm = CastToClient()->GetInv().GetItem(item_slot);
			if(itm)
				dmg += itm->GetItem()->AC * (RuleI(Combat, SpecialAttackACBonus))/100;
		}
	}
}

void Mob::DoSpecialAttackDamage(Mob *who, SkillUseTypes skill, int32 max_damage, int32 min_damage, int32 hate_override,int ReuseTime, bool HitChance) {
	//this really should go through the same code as normal melee damage to
	//pick up all the special behavior there

	int32 hate = max_damage;
	if(hate_override > -1)
		hate = hate_override;

	if(skill == SkillBash)
	{
		if(IsClient())
		{
			ItemInst *item = CastToClient()->GetInv().GetItem(SLOT_SECONDARY);
			if(item)
			{
				if(item->GetItem()->ItemType == ItemTypeShield)
				{
					hate += item->GetItem()->AC;
				}
				const Item_Struct *itm = item->GetItem();
				hate = hate * (100 + GetFuriousBash(itm->Focus.Effect)) / 100;
			}
		}
	}

	min_damage += min_damage * GetMeleeMinDamageMod_SE(skill) / 100;

	if(HitChance && !who->CheckHitChance(this, skill, 13))
		max_damage = 0;

	else{
		bool CanRiposte = true;
		if(skill == SkillThrowing || skill == SkillArchery) // changed from '&&'
			CanRiposte = false;

		who->AvoidDamage(this, max_damage, CanRiposte);
		who->MeleeMitigation(this, max_damage, min_damage);

		if(max_damage > 0) {
			ApplyMeleeDamageBonus(skill, max_damage);
			max_damage += who->GetFcDamageAmtIncoming(this, 0, true, skill);
			max_damage += (itembonuses.HeroicSTR / 10) + (max_damage * who->GetSkillDmgTaken(skill) / 100) + GetSkillDmgAmt(skill);
			TryCriticalHit(who, skill, max_damage);
		}
	}

	if(max_damage >= 0) //You should probably get aggro no matter what, but unclear why it was set like this.
		who->AddToHateList(this, hate);

	who->Damage(this, max_damage, SPELL_UNKNOWN, skill, false);

	//Make sure 'this' has not killed the target and 'this' is not dead (Damage shield ect).
	if(!GetTarget())return;
	if (HasDied())	return;

	if (max_damage > 0)
		CheckNumHitsRemaining(5);

	//[AA Dragon Punch] value[0] = 100 for 25%, chance value[1] = skill
	if(aabonuses.SpecialAttackKBProc[0] && aabonuses.SpecialAttackKBProc[1] == skill){
		int kb_chance = 25;
		kb_chance += kb_chance*(100-aabonuses.SpecialAttackKBProc[0])/100;

		if (MakeRandomInt(0, 99) < kb_chance)
			SpellFinished(904, who, 10, 0, -1, spells[904].ResistDiff);
			//who->Stun(100); Kayen: This effect does not stun on live, it only moves the NPC.
	}

	if (HasSkillProcs()){
		float chance = (float)ReuseTime*RuleR(Combat, AvgProcsPerMinute)/60000.0f;
		TrySkillProc(who, skill, chance);
	}

	if(max_damage == -3 && !who->HasDied())
		DoRiposte(who);
}


void Client::OPCombatAbility(const EQApplicationPacket *app) {
	if(!GetTarget())
		return;
	//make sure were actually able to use such an attack.
	if(spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || dead)
		return;

	CombatAbility_Struct* ca_atk = (CombatAbility_Struct*) app->pBuffer;

	if(GetTarget()->GetID() != ca_atk->m_target)
		return;	//invalid packet.

	if(!IsAttackAllowed(GetTarget()))
		return;

	//These two are not subject to the combat ability timer, as they
	//allready do their checking in conjunction with the attack timer
	//throwing weapons
	if(ca_atk->m_atk == 11) {
		if (ca_atk->m_skill == SkillThrowing) {
			SetAttackTimer();
			ThrowingAttack(GetTarget());
			if (CheckDoubleRangedAttack())
				RangedAttack(GetTarget(), true);
			return;
		}
		//ranged attack (archery)
		if (ca_atk->m_skill == SkillArchery) {
			SetAttackTimer();
			RangedAttack(GetTarget());
			if (CheckDoubleRangedAttack())
				RangedAttack(GetTarget(), true);
			return;
		}
		//could we return here? Im not sure is m_atk 11 is used for real specials
	}

	//check range for all these abilities, they are all close combat stuff
	if(!CombatRange(GetTarget()))
		return;

	if(!p_timers.Expired(&database, pTimerCombatAbility, false)) {
		Message(13,"Ability recovery time not yet met.");
		return;
	}

	int ReuseTime = 0;
	int ClientHaste = GetHaste();
	int HasteMod = 0;

	if(ClientHaste >= 0){
		HasteMod = (10000/(100+ClientHaste)); //+100% haste = 2x as many attacks
	}
	else{
		HasteMod = (100-ClientHaste); //-100% haste = 1/2 as many attacks
	}
	int32 dmg = 0;

	int32 skill_reduction = this->GetSkillReuseTime(ca_atk->m_skill);

	if ((ca_atk->m_atk == 100) && (ca_atk->m_skill == SkillBash)) { // SLAM - Bash without a shield equipped
		if (GetTarget() != this) {

			CheckIncreaseSkill(SkillBash, GetTarget(), 10);
			DoAnim(animTailRake);

			int32 ht = 0;
			if(GetWeaponDamage(GetTarget(), GetInv().GetItem(SLOT_SECONDARY)) <= 0 &&
				GetWeaponDamage(GetTarget(), GetInv().GetItem(SLOT_SHOULDER)) <= 0){
				dmg = -5;
			}
			else{
				if(!GetTarget()->CheckHitChance(this, SkillBash, 0)) {
					dmg = 0;
					ht = GetBashDamage();
				}
				else{
					if(RuleB(Combat, UseIntervalAC))
						ht = dmg = GetBashDamage();
					else
						ht = dmg = MakeRandomInt(1, GetBashDamage());

				}
			}

			ReuseTime = BashReuseTime-1-skill_reduction;
			ReuseTime = (ReuseTime*HasteMod)/100;
			DoSpecialAttackDamage(GetTarget(), SkillBash, dmg, 1, ht, ReuseTime);
			if(ReuseTime > 0)
			{
				p_timers.Start(pTimerCombatAbility, ReuseTime);
			}
		}
		return;
	}

	if ((ca_atk->m_atk == 100) && (ca_atk->m_skill == SkillFrenzy))
	{
		CheckIncreaseSkill(SkillFrenzy, GetTarget(), 10);
		int AtkRounds = 3;
		int skillmod = 100*GetSkill(SkillFrenzy)/MaxSkill(SkillFrenzy);
		int32 max_dmg = (26 + ((((GetLevel()-6) * 2)*skillmod)/100)) * ((100+RuleI(Combat, FrenzyBonus))/100);
		int32 min_dmg = 0;
		DoAnim(anim2HSlashing);

		max_dmg = mod_frenzy_damage(max_dmg);

		if (GetLevel() < 51)
			min_dmg = 1;
		else
			min_dmg = GetLevel()*8/10;

		if (min_dmg > max_dmg)
			max_dmg = min_dmg;

		ReuseTime = FrenzyReuseTime-1-skill_reduction;
		ReuseTime = (ReuseTime*HasteMod)/100;

		//Live parses show around 55% Triple 35% Double 10% Single, you will always get first hit.
		while(AtkRounds > 0) {

			if (GetTarget() && (AtkRounds == 1 || MakeRandomInt(0,100) < 75)){
				DoSpecialAttackDamage(GetTarget(), SkillFrenzy, max_dmg, min_dmg, max_dmg , ReuseTime, true);
			}
			AtkRounds--;
		}

		if(ReuseTime > 0) {
			p_timers.Start(pTimerCombatAbility, ReuseTime);
		}
		return;
	}

	switch(GetClass())
	{
		case BERSERKER:
		case WARRIOR:
		case RANGER:
		case BEASTLORD:
			if (ca_atk->m_atk != 100 || ca_atk->m_skill != SkillKick) {
				break;
			}
			if (GetTarget() != this) {
				CheckIncreaseSkill(SkillKick, GetTarget(), 10);
				DoAnim(animKick);

				int32 ht = 0;
				if(GetWeaponDamage(GetTarget(), GetInv().GetItem(SLOT_FEET)) <= 0){
					dmg = -5;
				}
				else{
					if(!GetTarget()->CheckHitChance(this, SkillKick, 0)) {
						dmg = 0;
						ht = GetKickDamage();
					}
					else{
						if(RuleB(Combat, UseIntervalAC))
							ht = dmg = GetKickDamage();
						else
							ht = dmg = MakeRandomInt(1, GetKickDamage());
					}
				}

				ReuseTime = KickReuseTime-1-skill_reduction;
				DoSpecialAttackDamage(GetTarget(), SkillKick, dmg, 1, ht, ReuseTime);

			}
			break;
		case MONK: {
			ReuseTime = MonkSpecialAttack(GetTarget(), ca_atk->m_skill) - 1 - skill_reduction;

			//Live AA - Technique of Master Wu
			uint16 bDoubleSpecialAttack = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;
			if( bDoubleSpecialAttack && (bDoubleSpecialAttack >= 100 || bDoubleSpecialAttack > MakeRandomInt(0,100)) ) {

				int MonkSPA [5] = { SkillFlyingKick, SkillDragonPunch, SkillEagleStrike, SkillTigerClaw, SkillRoundKick };
				MonkSpecialAttack(GetTarget(), MonkSPA[MakeRandomInt(0,4)]);

				int TripleChance = 25;

				if (bDoubleSpecialAttack > 100)
					TripleChance += TripleChance*(100-bDoubleSpecialAttack)/100;

				if(TripleChance > MakeRandomInt(0,100)) {
					MonkSpecialAttack(GetTarget(), MonkSPA[MakeRandomInt(0,4)]);
				}
			}

			if(ReuseTime < 100) {
				//hackish... but we return a huge reuse time if this is an
				// invalid skill, otherwise, we can safely assume it is a
				// valid monk skill and just cast it to a SkillType
				CheckIncreaseSkill((SkillUseTypes) ca_atk->m_skill, GetTarget(), 10);
			}
			break;
		}
		case ROGUE: {
			if (ca_atk->m_atk != 100 || ca_atk->m_skill != SkillBackstab) {
				break;
			}
			TryBackstab(GetTarget(), ReuseTime);
			ReuseTime = BackstabReuseTime-1 - skill_reduction;
			break;
		}
		default:
			//they have no abilities... wtf? make em wait a bit
			ReuseTime = 9 - skill_reduction;
			break;
	}

	ReuseTime = (ReuseTime*HasteMod)/100;
	if(ReuseTime > 0)
	{
		p_timers.Start(pTimerCombatAbility, ReuseTime);
	}
}

//returns the reuse time in sec for the special attack used.
int Mob::MonkSpecialAttack(Mob* other, uint8 unchecked_type)
{
	if(!other)
		return 0;

	int32 ndamage = 0;
	int32 max_dmg = 0;
	int32 min_dmg = 1;
	int reuse = 0;
	SkillUseTypes skill_type;	//to avoid casting... even though it "would work"
	uint8 itemslot = SLOT_FEET;

	switch(unchecked_type)
	{
		case SkillFlyingKick:{
			skill_type = SkillFlyingKick;
			max_dmg = ((GetSTR()+GetSkill(skill_type)) * RuleI(Combat, FlyingKickBonus) / 100) + 35;
			min_dmg = ((level*8)/10);
			ApplySpecialAttackMod(skill_type, max_dmg, min_dmg);
			DoAnim(animFlyingKick);
			reuse = FlyingKickReuseTime;
			break;
		}
		case SkillDragonPunch:{
			skill_type = SkillDragonPunch;
			max_dmg = ((GetSTR()+GetSkill(skill_type)) * RuleI(Combat, DragonPunchBonus) / 100) + 26;
			itemslot = SLOT_HANDS;
			ApplySpecialAttackMod(skill_type, max_dmg, min_dmg);
			DoAnim(animTailRake);
			reuse = TailRakeReuseTime;
			break;
		}

		case SkillEagleStrike:{
			skill_type = SkillEagleStrike;
			max_dmg = ((GetSTR()+GetSkill(skill_type)) * RuleI(Combat, EagleStrikeBonus) / 100) + 19;
			itemslot = SLOT_HANDS;
			ApplySpecialAttackMod(skill_type, max_dmg, min_dmg);
			DoAnim(animEagleStrike);
			reuse = EagleStrikeReuseTime;
			break;
		}

		case SkillTigerClaw:{
			skill_type = SkillTigerClaw;
			max_dmg = ((GetSTR()+GetSkill(skill_type)) * RuleI(Combat, TigerClawBonus) / 100) + 12;
			itemslot = SLOT_HANDS;
			ApplySpecialAttackMod(skill_type, max_dmg, min_dmg);
			DoAnim(animTigerClaw);
			reuse = TigerClawReuseTime;
			break;
		}

		case SkillRoundKick:{
			skill_type = SkillRoundKick;
			max_dmg = ((GetSTR()+GetSkill(skill_type)) * RuleI(Combat, RoundKickBonus) / 100) + 10;
			ApplySpecialAttackMod(skill_type, max_dmg, min_dmg);
			DoAnim(animRoundKick);
			reuse = RoundKickReuseTime;
			break;
		}

		case SkillKick:{
			skill_type = SkillKick;
			max_dmg = GetKickDamage();
			DoAnim(animKick);
			reuse = KickReuseTime;
			break;
		}
		default:
			mlog(CLIENT__ERROR, "Invalid special attack type %d attempted", unchecked_type);
			return(1000); /* nice long delay for them, the caller depends on this! */
	}

	if(IsClient()){
		if(GetWeaponDamage(other, CastToClient()->GetInv().GetItem(itemslot)) <= 0){
			ndamage = -5;
		}
	}
	else{
		if(GetWeaponDamage(other, (const Item_Struct*)nullptr) <= 0){
			ndamage = -5;
		}
	}

	int32 ht = 0;
	if(ndamage == 0){
		if(other->CheckHitChance(this, skill_type, 0)){
			if(RuleB(Combat, UseIntervalAC))
				ht = ndamage = max_dmg;
			else
				ht = ndamage = MakeRandomInt(min_dmg, max_dmg);
		}
		else
		{
			ht = max_dmg;
		}
	}

	//This can potentially stack with changes to kick damage
	ht = ndamage = mod_monk_special_damage(ndamage, skill_type);

	DoSpecialAttackDamage(other, skill_type, ndamage, min_dmg, ht, reuse);

	return(reuse);
}

void Mob::TryBackstab(Mob *other, int ReuseTime) {
	if(!other)
		return;

	bool bIsBehind = false;
	bool bCanFrontalBS = false;

	//make sure we have a proper weapon if we are a client.
	if(IsClient()) {
		const ItemInst *wpn = CastToClient()->GetInv().GetItem(SLOT_PRIMARY);
		if(!wpn || (wpn->GetItem()->ItemType != ItemType1HPiercing)){
			Message_StringID(13, BACKSTAB_WEAPON);
			return;
		}
	}

	//Live AA - Triple Backstab
	int tripleChance = itembonuses.TripleBackstab + spellbonuses.TripleBackstab + aabonuses.TripleBackstab;

	if (BehindMob(other, GetX(), GetY()))
		bIsBehind = true;

	else {
		//Live AA - Seized Opportunity
		int FrontalBSChance = itembonuses.FrontalBackstabChance + spellbonuses.FrontalBackstabChance + aabonuses.FrontalBackstabChance;

		if (FrontalBSChance && (FrontalBSChance > MakeRandomInt(0, 100)))
			bCanFrontalBS = true;
	}

	if (bIsBehind || bCanFrontalBS){ // Player is behind other OR can do Frontal Backstab

		if (bCanFrontalBS) {
			CastToClient()->Message(0,"Your fierce attack is executed with such grace, your target did not see it coming!");
		}

		// solar - chance to assassinate
		int chance = 10 + (GetDEX()/10) + (itembonuses.HeroicDEX/10); //18.5% chance at 85 dex 40% chance at 300 dex
		if(
			level >= 60 && // player is 60 or higher
			other->GetLevel() <= 45 && // mob 45 or under
			!other->CastToNPC()->IsEngaged() && // not aggro
			other->GetHP()<=32000
			&& other->IsNPC()
			&& MakeRandomFloat(0, 99) < chance // chance
			) {
			entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, ASSASSINATES, GetName());
			if(IsClient())
				CastToClient()->CheckIncreaseSkill(SkillBackstab, other, 10);
			RogueAssassinate(other);
		}
		else {
			RogueBackstab(other);
			if (level > 54) {
				float DoubleAttackProbability = (GetSkill(SkillDoubleAttack) + GetLevel()) / 500.0f; // 62.4 max
				// Check for double attack with main hand assuming maxed DA Skill (MS)

				if(MakeRandomFloat(0, 1) < DoubleAttackProbability)	// Max 62.4 % chance of DA
				{
					if(other->GetHP() > 0)
						RogueBackstab(other,false,ReuseTime);

					if (tripleChance && other->GetHP() > 0 && tripleChance > MakeRandomInt(0, 100))
						RogueBackstab(other,false,ReuseTime);
				}
			}
			if(IsClient())
				CastToClient()->CheckIncreaseSkill(SkillBackstab, other, 10);
		}
	}
	//Live AA - Chaotic Backstab
	else if(aabonuses.FrontalBackstabMinDmg || itembonuses.FrontalBackstabMinDmg || spellbonuses.FrontalBackstabMinDmg) {

		//we can stab from any angle, we do min damage though.
		RogueBackstab(other, true);
		if (level > 54) {
			float DoubleAttackProbability = (GetSkill(SkillDoubleAttack) + GetLevel()) / 500.0f; // 62.4 max
			if(IsClient())
				CastToClient()->CheckIncreaseSkill(SkillBackstab, other, 10);
			// Check for double attack with main hand assuming maxed DA Skill (MS)
			if(MakeRandomFloat(0, 1) < DoubleAttackProbability)		// Max 62.4 % chance of DA
				if(other->GetHP() > 0)
					RogueBackstab(other,true, ReuseTime);

			if (tripleChance && other->GetHP() > 0 && tripleChance > MakeRandomInt(0, 100))
					RogueBackstab(other,false,ReuseTime);
		}
	}
	else { //We do a single regular attack if we attack from the front without chaotic stab
		Attack(other, 13);
	}
}

//heko: backstab
void Mob::RogueBackstab(Mob* other, bool min_damage, int ReuseTime)
{
	int32 ndamage = 0;
	int32 max_hit = 0;
	int32 min_hit = 0;
	int32 hate = 0;
	int32 primaryweapondamage = 0;
	int32 backstab_dmg = 0;

	if(IsClient()){
		const ItemInst *wpn = nullptr;
		wpn = CastToClient()->GetInv().GetItem(SLOT_PRIMARY);
		if(wpn) {
			primaryweapondamage = GetWeaponDamage(other, wpn);
			backstab_dmg = wpn->GetItem()->BackstabDmg;
			for(int i = 0; i < MAX_AUGMENT_SLOTS; ++i)
			{
				ItemInst *aug = wpn->GetAugment(i);
				if(aug)
				{
					backstab_dmg += aug->GetItem()->BackstabDmg;
				}
			}
		}
	}
	else{
		primaryweapondamage = (GetLevel()/7)+1; // fallback incase it's a npc without a weapon, 2 dmg at 10, 10 dmg at 65
		backstab_dmg = primaryweapondamage;
	}

	if(primaryweapondamage > 0){
		if(level > 25){
			max_hit = (((2*backstab_dmg) * GetDamageTable(SkillBackstab) / 100) * 10 * GetSkill(SkillBackstab) / 355) + ((level-25)/3) + 1;
			hate = 20 * backstab_dmg * GetSkill(SkillBackstab) / 355;
		}
		else{
			max_hit = (((2*backstab_dmg) * GetDamageTable(SkillBackstab) / 100) * 10 * GetSkill(SkillBackstab) / 355) + 1;;
			hate = 20 * backstab_dmg * GetSkill(SkillBackstab) / 355;
		}

		// determine minimum hits
		if (level < 51)
		{
			min_hit = (level*15/10);
		}
		else
		{
			// Trumpcard: Replaced switch statement with formula calc. This will give minhit increases all the way to 65.
			min_hit = (level * ( level*5 - 105)) / 100;
		}

		if(!other->CheckHitChance(this, SkillBackstab, 0))	{
			ndamage = 0;
		}
		else{
			if(min_damage){
				ndamage = min_hit;
			}
			else
			{
				if (max_hit < min_hit)
					max_hit = min_hit;

				if(RuleB(Combat, UseIntervalAC))
					ndamage = max_hit;
				else
					ndamage = MakeRandomInt(min_hit, max_hit);

			}
		}
	}
	else{
		ndamage = -5;
	}

	ndamage = mod_backstab_damage(ndamage);

	DoSpecialAttackDamage(other, SkillBackstab, ndamage, min_hit, hate, ReuseTime);
	DoAnim(animPiercing);
}

// solar - assassinate
void Mob::RogueAssassinate(Mob* other)
{
	//can you dodge, parry, etc.. an assassinate??
	//if so, use DoSpecialAttackDamage(other, BACKSTAB, 32000); instead
	if(GetWeaponDamage(other, IsClient()?CastToClient()->GetInv().GetItem(SLOT_PRIMARY):(const ItemInst*)nullptr) > 0){
		other->Damage(this, 32000, SPELL_UNKNOWN, SkillBackstab);
	}else{
		other->Damage(this, -5, SPELL_UNKNOWN, SkillBackstab);
	}
	DoAnim(animPiercing);	//piercing animation
}

void Client::RangedAttack(Mob* other, bool CanDoubleAttack) {
	//conditions to use an attack checked before we are called

	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if(!CanDoubleAttack && ((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		mlog(COMBAT__RANGED, "Throwing attack canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}
	const ItemInst* RangeWeapon = m_inv[SLOT_RANGE];

	//locate ammo
	int ammo_slot = SLOT_AMMO;
	const ItemInst* Ammo = m_inv[SLOT_AMMO];

	if (!RangeWeapon || !RangeWeapon->IsType(ItemClassCommon)) {
		mlog(COMBAT__RANGED, "Ranged attack canceled. Missing or invalid ranged weapon (%d) in slot %d", GetItemIDAt(SLOT_RANGE), SLOT_RANGE);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have no bow!", GetItemIDAt(SLOT_RANGE));
		return;
	}
	if (!Ammo || !Ammo->IsType(ItemClassCommon)) {
		mlog(COMBAT__RANGED, "Ranged attack canceled. Missing or invalid ammo item (%d) in slot %d", GetItemIDAt(SLOT_AMMO), SLOT_AMMO);
		Message(0, "Error: Ammo: GetItem(%i)==0, you have no ammo!", GetItemIDAt(SLOT_AMMO));
		return;
	}

	const Item_Struct* RangeItem = RangeWeapon->GetItem();
	const Item_Struct* AmmoItem = Ammo->GetItem();

	if(RangeItem->ItemType != ItemTypeBow) {
		mlog(COMBAT__RANGED, "Ranged attack canceled. Ranged item is not a bow. type %d.", RangeItem->ItemType);
		Message(0, "Error: Rangeweapon: Item %d is not a bow.", RangeWeapon->GetID());
		return;
	}
	if(AmmoItem->ItemType != ItemTypeArrow) {
		mlog(COMBAT__RANGED, "Ranged attack canceled. Ammo item is not an arrow. type %d.", AmmoItem->ItemType);
		Message(0, "Error: Ammo: type %d != %d, you have the wrong type of ammo!", AmmoItem->ItemType, ItemTypeArrow);
		return;
	}

	mlog(COMBAT__RANGED, "Shooting %s with bow %s (%d) and arrow %s (%d)", GetTarget()->GetName(), RangeItem->Name, RangeItem->ID, AmmoItem->Name, AmmoItem->ID);

	//look for ammo in inventory if we only have 1 left...
	if(Ammo->GetCharges() == 1) {
		//first look for quivers
		int r;
		bool found = false;
		for(r = SLOT_PERSONAL_BEGIN; r <= SLOT_PERSONAL_END; r++) {
			const ItemInst *pi = m_inv[r];
			if(pi == nullptr || !pi->IsType(ItemClassContainer))
				continue;
			const Item_Struct* bagitem = pi->GetItem();
			if(!bagitem || bagitem->BagType != BagTypeQuiver)
				continue;

			//we found a quiver, look for the ammo in it
			int i;
			for (i = 0; i < bagitem->BagSlots; i++) {
				ItemInst* baginst = pi->GetItem(i);
				if(!baginst)
					continue;	//empty
				if(baginst->GetID() == Ammo->GetID()) {
					//we found it... use this stack
					//the item wont change, but the instance does
					Ammo = baginst;
					ammo_slot = m_inv.CalcSlotId(r, i);
					found = true;
					mlog(COMBAT__RANGED, "Using ammo from quiver stack at slot %d. %d in stack.", ammo_slot, Ammo->GetCharges());
					break;
				}
			}
			if(found)
				break;
		}

		if(!found) {
			//if we dont find a quiver, look through our inventory again
			//not caring if the thing is a quiver.
			int32 aslot = m_inv.HasItem(AmmoItem->ID, 1, invWherePersonal);
			if(aslot != SLOT_INVALID) {
				ammo_slot = aslot;
				Ammo = m_inv[aslot];
				mlog(COMBAT__RANGED, "Using ammo from inventory stack at slot %d. %d in stack.", ammo_slot, Ammo->GetCharges());
			}
		}
	}

	float range = RangeItem->Range + AmmoItem->Range + 5; //Fudge it a little, client will let you hit something at 0 0 0 when you are at 205 0 0
	mlog(COMBAT__RANGED, "Calculated bow range to be %.1f", range);
	range *= range;
	if(DistNoRootNoZ(*GetTarget()) > range) {
		mlog(COMBAT__RANGED, "Ranged attack out of range... client should catch this. (%f > %f).\n", DistNoRootNoZ(*GetTarget()), range);
		//target is out of range, client does a message
		return;
	}
	else if(DistNoRootNoZ(*GetTarget()) < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		return;
	}

	if(!IsAttackAllowed(GetTarget()) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}

	SendItemAnimation(GetTarget(), AmmoItem, SkillArchery);

	DoArcheryAttackDmg(GetTarget(), RangeWeapon, Ammo);

	//EndlessQuiver AA base1 = 100% Chance to avoid consumption arrow.
	int ChanceAvoidConsume = aabonuses.ConsumeProjectile + itembonuses.ConsumeProjectile + spellbonuses.ConsumeProjectile;

	if (!ChanceAvoidConsume || (ChanceAvoidConsume < 100 && MakeRandomInt(0,99) > ChanceAvoidConsume)){

		DeleteItemInInventory(ammo_slot, 1, true);
		mlog(COMBAT__RANGED, "Consumed one arrow from slot %d", ammo_slot);
	} else {
		mlog(COMBAT__RANGED, "Endless Quiver prevented ammo consumption.");
	}

	CheckIncreaseSkill(SkillArchery, GetTarget(), -15);

	//break invis when you attack
	if(invisible) {
		mlog(COMBAT__ATTACKS, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if(invisible_undead) {
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if(invisible_animals){
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. animals due to melee attack.");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if(hidden || improved_hidden){
		hidden = false;
		improved_hidden = false;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
}

void Mob::DoArcheryAttackDmg(Mob* other, const ItemInst* RangeWeapon, const ItemInst* Ammo, uint16 weapon_damage, int16 chance_mod, int16 focus)
{
	if (!CanDoSpecialAttack(other))
		return;

	if (!other->CheckHitChance(this, SkillArchery, 13,chance_mod)) {
		mlog(COMBAT__RANGED, "Ranged attack missed %s.", other->GetName());
		other->Damage(this, 0, SPELL_UNKNOWN, SkillArchery);
	} else {
		mlog(COMBAT__RANGED, "Ranged attack hit %s.", other->GetName());

		if(!TryHeadShot(other, SkillArchery))
		{
			int32 TotalDmg = 0;
			int16 WDmg = 0;
			int16 ADmg = 0;
			if (!weapon_damage){
				WDmg = GetWeaponDamage(other, RangeWeapon);
				ADmg = GetWeaponDamage(other, Ammo);
			}
			else
				WDmg = weapon_damage;

			if((WDmg > 0) || (ADmg > 0))
			{
				if(WDmg < 0)
					WDmg = 0;
				if(ADmg < 0)
					ADmg = 0;
				uint32 MaxDmg = (RuleR(Combat, ArcheryBaseDamageBonus)*(WDmg+ADmg)*GetDamageTable(SkillArchery)) / 100;
				int32 hate = ((WDmg+ADmg));

				uint16 bonusArcheryDamageModifier = aabonuses.ArcheryDamageModifier + itembonuses.ArcheryDamageModifier + spellbonuses.ArcheryDamageModifier;

				MaxDmg += MaxDmg*bonusArcheryDamageModifier / 100;

				mlog(COMBAT__RANGED, "Bow DMG %d, Arrow DMG %d, Max Damage %d.", WDmg, ADmg, MaxDmg);

				bool dobonus = false;
				if(GetClass() == RANGER && GetLevel() > 50)
				{
					int bonuschance = RuleI(Combat, ArcheryBonusChance);

					bonuschance = mod_archery_bonus_chance(bonuschance, RangeWeapon);

					if( !RuleB(Combat, UseArcheryBonusRoll) || (MakeRandomInt(1, 100) < bonuschance) )
					{
						if(RuleB(Combat, ArcheryBonusRequiresStationary))
						{
							if(other->IsNPC() && !other->IsMoving() && !other->IsRooted())
							{
								dobonus = true;
							}
						}
						else
						{
							dobonus = true;
						}
					}

					if(dobonus)
					{
						MaxDmg *= (float)2;
						hate *= (float)2;
						MaxDmg = mod_archery_bonus_damage(MaxDmg, RangeWeapon);

						mlog(COMBAT__RANGED, "Ranger. Double damage success roll, doubling damage to %d", MaxDmg);
						Message_StringID(MT_CritMelee, BOW_DOUBLE_DAMAGE);
					}
				}

				if (MaxDmg == 0)
					MaxDmg = 1;

				if(RuleB(Combat, UseIntervalAC))
					TotalDmg = MaxDmg;
				else
					TotalDmg = MakeRandomInt(1, MaxDmg);

				int minDmg = 1;
				if(GetLevel() > 25){
					//twice, for ammo and weapon
					TotalDmg += (2*((GetLevel()-25)/3));
					minDmg += (2*((GetLevel()-25)/3));
					minDmg += minDmg * GetMeleeMinDamageMod_SE(SkillArchery) / 100;
					hate += (2*((GetLevel()-25)/3));
				}

				other->AvoidDamage(this, TotalDmg, false);
				other->MeleeMitigation(this, TotalDmg, minDmg);
				if(TotalDmg > 0)
				{
					TotalDmg += TotalDmg*focus/100;
					ApplyMeleeDamageBonus(SkillArchery, TotalDmg);
					TotalDmg += other->GetFcDamageAmtIncoming(this, 0, true, SkillArchery);
					TotalDmg += (itembonuses.HeroicDEX / 10) + (TotalDmg * other->GetSkillDmgTaken(SkillArchery) / 100) + GetSkillDmgAmt(SkillArchery);

					TotalDmg = mod_archery_damage(TotalDmg, dobonus, RangeWeapon);

					TryCriticalHit(other, SkillArchery, TotalDmg);
					other->AddToHateList(this, hate, 0, false);
					CheckNumHitsRemaining(5);
				}
			}
			else
				TotalDmg = -5;

			other->Damage(this, TotalDmg, SPELL_UNKNOWN, SkillArchery);
		}
	}

	//try proc on hits and misses
	if((RangeWeapon != nullptr) && GetTarget() && other && (other->GetHP() > -10))
	{
		TryWeaponProc(RangeWeapon, other, 11);
	}

	//Arrow procs because why not?
    if((Ammo != NULL) && GetTarget() && other && (other->GetHP() > -10))
    {
        TryWeaponProc(Ammo, other, 11);
    }
}

void NPC::RangedAttack(Mob* other)
{
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))
	{
		mlog(COMBAT__RANGED, "Archery canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	//if we have SPECATK_RANGED_ATK set then we range attack without weapon or ammo
	const Item_Struct* weapon = nullptr;
	const Item_Struct* ammo = nullptr;
	if(!GetSpecialAbility(SPECATK_RANGED_ATK))
	{
		//find our bow and ammo return if we can't find them...
		return;
	}

	float range = 250; // needs to be longer than 200(most spells)
	mlog(COMBAT__RANGED, "Calculated bow range to be %.1f", range);
	range *= range;
	if(DistNoRootNoZ(*GetTarget()) > range) {
		mlog(COMBAT__RANGED, "Ranged attack out of range...%.2f vs %.2f", DistNoRootNoZ(*GetTarget()), range);
		//target is out of range, client does a message
		return;
	}
	else if(DistNoRootNoZ(*GetTarget()) < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		return;
	}

	if(!IsAttackAllowed(GetTarget()) ||
		IsCasting() ||
		DivineAura() ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}

	if(!ammo)
	{
		ammo = database.GetItem(8005);
	}

	if(ammo)
		SendItemAnimation(GetTarget(), ammo, SkillArchery);

	// Face the Target
	FaceTarget(GetTarget());

	// Hit?
	if (!GetTarget()->CheckHitChance(this, SkillArchery, 13))
	{
		mlog(COMBAT__RANGED, "Ranged attack missed %s.", GetTarget()->GetName());
		GetTarget()->Damage(this, 0, SPELL_UNKNOWN, SkillArchery);
	}
	else
	{
		int16 WDmg = GetWeaponDamage(GetTarget(), weapon);
		int16 ADmg = GetWeaponDamage(GetTarget(), ammo);
		if(WDmg > 0 || ADmg > 0)
		{
			mlog(COMBAT__RANGED, "Ranged attack hit %s.", GetTarget()->GetName());
			int32 TotalDmg = 0;

			int32 MaxDmg = max_dmg * RuleR(Combat, ArcheryNPCMultiplier); // should add a field to npc_types
			int32 MinDmg = min_dmg * RuleR(Combat, ArcheryNPCMultiplier);

			if(RuleB(Combat, UseIntervalAC))
				TotalDmg = MaxDmg;
			else
				TotalDmg = MakeRandomInt(MinDmg, MaxDmg);

			int32 hate = TotalDmg;

			GetTarget()->MeleeMitigation(this, TotalDmg, MinDmg);
			ApplyMeleeDamageBonus(SkillArchery, TotalDmg);
			TryCriticalHit(GetTarget(), SkillArchery, TotalDmg);
			GetTarget()->AddToHateList(this, hate, 0, false);
			GetTarget()->Damage(this, TotalDmg, SPELL_UNKNOWN, SkillArchery);
			CheckNumHitsRemaining(5);
		}
		else
		{
			GetTarget()->Damage(this, -5, SPELL_UNKNOWN, SkillArchery);
		}
	}

	//break invis when you attack
	if(invisible) {
		mlog(COMBAT__ATTACKS, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if(invisible_undead) {
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if(invisible_animals){
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. animals due to melee attack.");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if(hidden || improved_hidden){
		hidden = false;
		improved_hidden = false;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
}

uint16 Mob::GetThrownDamage(int16 wDmg, int32& TotalDmg, int& minDmg)
{

	uint16 MaxDmg = (((2 * wDmg) * GetDamageTable(SkillThrowing)) / 100);

	if (MaxDmg == 0)
		MaxDmg = 1;

	if(RuleB(Combat, UseIntervalAC))
		TotalDmg = MaxDmg;
	else
		TotalDmg = MakeRandomInt(1, MaxDmg);

	minDmg = 1;
	if(GetLevel() > 25)
	{
		TotalDmg += ((GetLevel()-25)/3);
		minDmg += ((GetLevel()-25)/3);
		minDmg += minDmg * GetMeleeMinDamageMod_SE(SkillThrowing) / 100;
	}

	if(MaxDmg < minDmg)
		MaxDmg = minDmg;

	MaxDmg = mod_throwing_damage(MaxDmg);

	return MaxDmg;
}

void Client::ThrowingAttack(Mob* other, bool CanDoubleAttack) { //old was 51
	//conditions to use an attack checked before we are called

	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((!CanDoubleAttack && (attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		mlog(COMBAT__RANGED, "Throwing attack canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	int ammo_slot = SLOT_RANGE;
	const ItemInst* RangeWeapon = m_inv[SLOT_RANGE];

	if (!RangeWeapon || !RangeWeapon->IsType(ItemClassCommon)) {
		mlog(COMBAT__RANGED, "Ranged attack canceled. Missing or invalid ranged weapon (%d) in slot %d", GetItemIDAt(SLOT_RANGE), SLOT_RANGE);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing to throw!", GetItemIDAt(SLOT_RANGE));
		return;
	}

	const Item_Struct* item = RangeWeapon->GetItem();
	if(item->ItemType != ItemTypeLargeThrowing && item->ItemType != ItemTypeSmallThrowing) {
		mlog(COMBAT__RANGED, "Ranged attack canceled. Ranged item %d is not a throwing weapon. type %d.", item->ItemType);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing useful to throw!", GetItemIDAt(SLOT_RANGE));
		return;
	}

	mlog(COMBAT__RANGED, "Throwing %s (%d) at %s", item->Name, item->ID, GetTarget()->GetName());

	if(RangeWeapon->GetCharges() == 1) {
		//first check ammo
		const ItemInst* AmmoItem = m_inv[SLOT_AMMO];
		if(AmmoItem != nullptr && AmmoItem->GetID() == RangeWeapon->GetID()) {
			//more in the ammo slot, use it
			RangeWeapon = AmmoItem;
			ammo_slot = SLOT_AMMO;
			mlog(COMBAT__RANGED, "Using ammo from ammo slot, stack at slot %d. %d in stack.", ammo_slot, RangeWeapon->GetCharges());
		} else {
			//look through our inventory for more
			int32 aslot = m_inv.HasItem(item->ID, 1, invWherePersonal);
			if(aslot != SLOT_INVALID) {
				//the item wont change, but the instance does, not that it matters
				ammo_slot = aslot;
				RangeWeapon = m_inv[aslot];
				mlog(COMBAT__RANGED, "Using ammo from inventory slot, stack at slot %d. %d in stack.", ammo_slot, RangeWeapon->GetCharges());
			}
		}
	}

	int range = item->Range +50/*Fudge it a little, client will let you hit something at 0 0 0 when you are at 205 0 0*/;
	mlog(COMBAT__RANGED, "Calculated bow range to be %.1f", range);
	range *= range;
	if(DistNoRootNoZ(*GetTarget()) > range) {
		mlog(COMBAT__RANGED, "Throwing attack out of range... client should catch this. (%f > %f).\n", DistNoRootNoZ(*GetTarget()), range);
		//target is out of range, client does a message
		return;
	}
	else if(DistNoRootNoZ(*GetTarget()) < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		return;
	}

	if(!IsAttackAllowed(GetTarget()) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}
	//send item animation, also does the throw animation
	SendItemAnimation(GetTarget(), item, SkillThrowing);

	DoThrowingAttackDmg(GetTarget(), RangeWeapon, item);

	//consume ammo
	DeleteItemInInventory(ammo_slot, 1, true);
	CheckIncreaseSkill(SkillThrowing, GetTarget());

	//break invis when you attack
	if(invisible) {
		mlog(COMBAT__ATTACKS, "Removing invisibility due to melee attack.");
		BuffFadeByEffect(SE_Invisibility);
		BuffFadeByEffect(SE_Invisibility2);
		invisible = false;
	}
	if(invisible_undead) {
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. undead due to melee attack.");
		BuffFadeByEffect(SE_InvisVsUndead);
		BuffFadeByEffect(SE_InvisVsUndead2);
		invisible_undead = false;
	}
	if(invisible_animals){
		mlog(COMBAT__ATTACKS, "Removing invisibility vs. animals due to melee attack.");
		BuffFadeByEffect(SE_InvisVsAnimals);
		invisible_animals = false;
	}

	if (spellbonuses.NegateIfCombat)
		BuffFadeByEffect(SE_NegateIfCombat);

	if(hidden || improved_hidden){
		hidden = false;
		improved_hidden = false;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
}

void Mob::DoThrowingAttackDmg(Mob* other, const ItemInst* RangeWeapon, const Item_Struct* item, uint16 weapon_damage, int16 chance_mod,int16 focus)
{
	if (!CanDoSpecialAttack(other))
		return;

	if (!other->CheckHitChance(this, SkillThrowing, 13, chance_mod)){
		mlog(COMBAT__RANGED, "Ranged attack missed %s.", other->GetName());
		other->Damage(this, 0, SPELL_UNKNOWN, SkillThrowing);
	} else {
		mlog(COMBAT__RANGED, "Throwing attack hit %s.", other->GetName());

		int16 WDmg = 0;

		if (!weapon_damage && item != nullptr)
			WDmg = GetWeaponDamage(other, item);
		else
			WDmg = weapon_damage;

		int32 TotalDmg = 0;

		if(WDmg > 0)
		{
			int minDmg = 1;
			uint16 MaxDmg = GetThrownDamage(WDmg, TotalDmg, minDmg);

			mlog(COMBAT__RANGED, "Item DMG %d. Max Damage %d. Hit for damage %d", WDmg, MaxDmg, TotalDmg);
			other->AvoidDamage(this, TotalDmg, false); //CanRiposte=false - Can not riposte throw attacks.
			other->MeleeMitigation(this, TotalDmg, minDmg);
			if(TotalDmg > 0)
			{
				TotalDmg += TotalDmg*focus/100;
				ApplyMeleeDamageBonus(SkillThrowing, TotalDmg);
				TotalDmg += other->GetFcDamageAmtIncoming(this, 0, true, SkillThrowing);
				TotalDmg += (itembonuses.HeroicDEX / 10) + (TotalDmg * other->GetSkillDmgTaken(SkillThrowing) / 100) + GetSkillDmgAmt(SkillThrowing);
				TryCriticalHit(other, SkillThrowing, TotalDmg);
				int32 hate = (2*WDmg);
				other->AddToHateList(this, hate, 0, false);
				CheckNumHitsRemaining(5);
			}
		}

		else
			TotalDmg = -5;

		other->Damage(this, TotalDmg, SPELL_UNKNOWN, SkillThrowing);
	}

	if((RangeWeapon != nullptr) && GetTarget() && other && (other->GetHP() > -10))
		TryWeaponProc(RangeWeapon, other, 11);
}

void Mob::SendItemAnimation(Mob *to, const Item_Struct *item, SkillUseTypes skillInUse) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	Arrow_Struct *as = (Arrow_Struct *) outapp->pBuffer;
	as->type = 1;
	as->src_x = GetX();
	as->src_y = GetY();
	as->src_z = GetZ();
	as->source_id = GetID();
	as->target_id = to->GetID();
	as->item_id = item->ID;

	as->item_type = item->ItemType;
	as->skill = (uint8)skillInUse;

	strn0cpy(as->model_name, item->IDFile, 16);


	/*
		The angular field affects how the object flies towards the target.
		A low angular (10) makes it circle the target widely, where a high
		angular (20000) makes it go straight at them.

		The tilt field causes the object to be tilted flying through the air
		and also seems to have an effect on how it behaves when circling the
		target based on the angular field.

		Arc causes the object to form an arc in motion. A value too high will
	*/
	as->velocity = 4.0;

	//these angle and tilt used together seem to make the arrow/knife throw as straight as I can make it

	as->launch_angle = CalculateHeadingToTarget(to->GetX(), to->GetY()) * 2;
	as->tilt = 125;
	as->arc = 50;


	//fill in some unknowns, we dont know their meaning yet
	//neither of these seem to change the behavior any
	as->unknown088 = 125;
	as->unknown092 = 16;

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);
}

void Mob::ProjectileAnimation(Mob* to, int item_id, bool IsArrow, float speed, float angle, float tilt, float arc, const char *IDFile) {

	const Item_Struct* item = nullptr;
	uint8 item_type = 0;

	if(!item_id) {
		item = database.GetItem(8005); // Arrow will be default
	}
	else {
		item = database.GetItem(item_id); // Use the item input into the command
	}

	if(!item) {
		return;
	}
	if(IsArrow) {
		item_type = 27;
	}
	if(!item_type) {
		item_type = item->ItemType;
	}
	if(!speed) {
		speed = 4.0;
	}
	if(!angle) {
		angle = CalculateHeadingToTarget(to->GetX(), to->GetY()) * 2;
	}
	if(!tilt) {
		tilt = 125;
	}
	if(!arc) {
		arc = 50;
	}

	const char *item_IDFile = item->IDFile;

	if (IDFile && (strncmp(IDFile, "IT", 2) == 0))
		item_IDFile = IDFile;

	// See SendItemAnimation() for some notes on this struct
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SomeItemPacketMaybe, sizeof(Arrow_Struct));
	Arrow_Struct *as = (Arrow_Struct *) outapp->pBuffer;
	as->type = 1;
	as->src_x = GetX();
	as->src_y = GetY();
	as->src_z = GetZ();
	as->source_id = GetID();
	as->target_id = to->GetID();
	as->item_id = item->ID;
	as->item_type = item_type;
	as->skill = 0;	// Doesn't seem to have any effect
	strn0cpy(as->model_name, item_IDFile, 16);
	as->velocity = speed;
	as->launch_angle = angle;
	as->tilt = tilt;
	as->arc = arc;
	as->unknown088 = 125;
	as->unknown092 = 16;

	entity_list.QueueCloseClients(this, outapp);
	safe_delete(outapp);

}

void NPC::DoClassAttacks(Mob *target) {
	if(target == nullptr)
		return;	//gotta have a target for all these

	bool taunt_time = taunt_timer.Check();
	bool ca_time = classattack_timer.Check(false);
	bool ka_time = knightattack_timer.Check(false);

	//only check attack allowed if we are going to do something
	if((taunt_time || ca_time || ka_time) && !IsAttackAllowed(target))
		return;

	if(ka_time){
		int knightreuse = 1000; //lets give it a small cooldown actually.
		switch(GetClass()){
			case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
				CastSpell(SPELL_NPC_HARM_TOUCH, target->GetID());
				knightreuse = HarmTouchReuseTime * 1000;
				break;
			}
			case PALADIN: case PALADINGM:{
				if(GetHPRatio() < 20) {
					CastSpell(SPELL_LAY_ON_HANDS, GetID());
					knightreuse = LayOnHandsReuseTime * 1000;
				} else {
					knightreuse = 2000; //Check again in two seconds.
				}
				break;
			}
		}
		knightattack_timer.Start(knightreuse);
	}

	//general stuff, for all classes....
	//only gets used when their primary ability get used too
	if (taunting && HasOwner() && target->IsNPC() && target->GetBodyType() != BT_Undead && taunt_time) {
		this->GetOwner()->Message_StringID(MT_PetResponse, PET_TAUNTING);
		Taunt(target->CastToNPC(), false);
	}

	if(!ca_time)
		return;

	float HasteModifier = 0;
	if(GetHaste() > 0)
		HasteModifier = 10000 / (100 + GetHaste());
	else if(GetHaste() < 0)
		HasteModifier = (100 - GetHaste());
	else
		HasteModifier = 100;

	int level = GetLevel();
	int reuse = TauntReuseTime * 1000;	//make this very long since if they dont use it once, they prolly never will
	bool did_attack = false;
	//class specific stuff...
	switch(GetClass()) {
		case ROGUE: case ROGUEGM:
			if(level >= 10) {
				reuse = BackstabReuseTime * 1000;
				TryBackstab(target, reuse);
				did_attack = true;
			}
			break;
		case MONK: case MONKGM: {
			uint8 satype = SkillKick;
			if(level > 29) {
				satype = SkillFlyingKick;
			} else if(level > 24) {
				satype = SkillDragonPunch;
			} else if(level > 19) {
				satype = SkillEagleStrike;
			} else if(level > 9) {
				satype = SkillTigerClaw;
			} else if(level > 4) {
				satype = SkillRoundKick;
			}
			reuse = MonkSpecialAttack(target, satype);

			reuse *= 1000;
			did_attack = true;
			break;
		}
		case WARRIOR: case WARRIORGM:{
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				if(MakeRandomInt(0, 100) > 25) //tested on live, warrior mobs both kick and bash, kick about 75% of the time, casting doesn't seem to make a difference.
				{
					DoAnim(animKick);
					int32 dmg = 0;

					if(GetWeaponDamage(target, (const Item_Struct*)nullptr) <= 0){
						dmg = -5;
					}
					else{
						if(target->CheckHitChance(this, SkillKick, 0)) {
							if(RuleB(Combat, UseIntervalAC))
								dmg = GetKickDamage();
							else
								dmg = MakeRandomInt(1, GetKickDamage());

						}
					}

					reuse = KickReuseTime * 1000;
					DoSpecialAttackDamage(target, SkillKick, dmg, 1, -1, reuse);
					did_attack = true;
				}
				else
				{
					DoAnim(animTailRake);
					int32 dmg = 0;

					if(GetWeaponDamage(target, (const Item_Struct*)nullptr) <= 0){
						dmg = -5;
					}
					else{
						if(target->CheckHitChance(this, SkillBash, 0)) {
							if(RuleB(Combat, UseIntervalAC))
								dmg = GetBashDamage();
							else
								dmg = MakeRandomInt(1, GetBashDamage());
						}
					}

					reuse = BashReuseTime * 1000;
					DoSpecialAttackDamage(target, SkillBash, dmg, 1, -1, reuse);
					did_attack = true;
				}
			}
			break;
		}
		case BERSERKER: case BERSERKERGM:
		{
			int AtkRounds = 3;
			int32 max_dmg = 26 + ((GetLevel()-6) * 2);
			int32 min_dmg = 0;
			DoAnim(anim2HSlashing);

			if (GetLevel() < 51)
				min_dmg = 1;
			else
				min_dmg = GetLevel()*8/10;

			if (min_dmg > max_dmg)
				max_dmg = min_dmg;

			reuse = FrenzyReuseTime * 1000;

			while(AtkRounds > 0) {

				if (GetTarget() && (AtkRounds == 1 || MakeRandomInt(0,100) < 75)){
					DoSpecialAttackDamage(GetTarget(), SkillFrenzy, max_dmg, min_dmg, -1 , reuse, true);
				}
				AtkRounds--;
			}


			did_attack = true;
			break;
		}
		case RANGER: case RANGERGM:
		case BEASTLORD: case BEASTLORDGM: {
			//kick
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				DoAnim(animKick);
				int32 dmg = 0;

				if(GetWeaponDamage(target, (const Item_Struct*)nullptr) <= 0){
					dmg = -5;
				}
				else{
					if(target->CheckHitChance(this, SkillKick, 0)) {
						if(RuleB(Combat, UseIntervalAC))
							dmg = GetKickDamage();
						else
							dmg = MakeRandomInt(1, GetKickDamage());
					}
				}

				reuse = KickReuseTime * 1000;
				DoSpecialAttackDamage(target, SkillKick, dmg, 1, -1, reuse);
				did_attack = true;
			}
			break;
		}
		case CLERIC: case CLERICGM: //clerics can bash too.
		case SHADOWKNIGHT: case SHADOWKNIGHTGM:
		case PALADIN: case PALADINGM:
		{
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				DoAnim(animTailRake);
				int32 dmg = 0;

				if(GetWeaponDamage(target, (const Item_Struct*)nullptr) <= 0){
					dmg = -5;
				}
				else{
					if(target->CheckHitChance(this, SkillBash, 0)) {
						if(RuleB(Combat, UseIntervalAC))
							dmg = GetBashDamage();
						else
							dmg = MakeRandomInt(1, GetBashDamage());
					}
				}

				reuse = BashReuseTime * 1000;
				DoSpecialAttackDamage(target, SkillBash, dmg, 1, -1, reuse);
				did_attack = true;
			}
			break;
		}
	}

	classattack_timer.Start(reuse*HasteModifier/100);
}

void Client::DoClassAttacks(Mob *ca_target, uint16 skill, bool IsRiposte)
{
	if(!ca_target)
		return;

	if(spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || dead)
		return;

	if(!IsAttackAllowed(ca_target))
		return;

	//check range for all these abilities, they are all close combat stuff
	if(!CombatRange(ca_target))
	{
		return;
	}

	if(!IsRiposte && (!p_timers.Expired(&database, pTimerCombatAbility, false))) {
		return;
	}

	int ReuseTime = 0;
	int ClientHaste = GetHaste();
	int HasteMod = 0;

	if(ClientHaste >= 0){
		HasteMod = (10000/(100+ClientHaste)); //+100% haste = 2x as many attacks
	}
	else{
		HasteMod = (100-ClientHaste); //-100% haste = 1/2 as many attacks
	}
	int32 dmg = 0;

	uint16 skill_to_use = -1;

	if (skill == -1){

		switch(GetClass())
		{
		case WARRIOR:
		case RANGER:
		case BEASTLORD:
			skill_to_use = SkillKick;
			break;
		case BERSERKER:
			skill_to_use = SkillFrenzy;
			break;
		case SHADOWKNIGHT:
		case PALADIN:
			skill_to_use = SkillBash;
			break;
		case MONK:
			if(GetLevel() >= 30)
			{
				skill_to_use = SkillFlyingKick;
			}
			else if(GetLevel() >= 25)
			{
				skill_to_use = SkillDragonPunch;
			}
			else if(GetLevel() >= 20)
			{
				skill_to_use = SkillEagleStrike;
			}
			else if(GetLevel() >= 10)
			{
				skill_to_use = SkillTigerClaw;
			}
			else if(GetLevel() >= 5)
			{
				skill_to_use = SkillRoundKick;
			}
			else
			{
				skill_to_use = SkillKick;
			}
			break;
		case ROGUE:
			skill_to_use = SkillBackstab;
			break;
		}
	}

	else
		skill_to_use = skill;

	if(skill_to_use == -1)
		return;

	if(skill_to_use == SkillBash)
	{
		if (ca_target!=this)
		{
			DoAnim(animTailRake);

			if(GetWeaponDamage(ca_target, GetInv().GetItem(SLOT_SECONDARY)) <= 0 &&
				GetWeaponDamage(ca_target, GetInv().GetItem(SLOT_SHOULDER)) <= 0){
				dmg = -5;
			}
			else{
				if(!ca_target->CheckHitChance(this, SkillBash, 0)) {
					dmg = 0;
				}
				else{
					if(RuleB(Combat, UseIntervalAC))
						dmg = GetBashDamage();
					else
						dmg = MakeRandomInt(1, GetBashDamage());

				}
			}

			ReuseTime = BashReuseTime-1;
			ReuseTime = (ReuseTime*HasteMod)/100;

			DoSpecialAttackDamage(ca_target, SkillBash, dmg, 1,-1,ReuseTime);

			if(ReuseTime > 0 && !IsRiposte)
			{
				p_timers.Start(pTimerCombatAbility, ReuseTime);
			}
		}
		return;
	}

	if(skill_to_use == SkillFrenzy)
	{
		CheckIncreaseSkill(SkillFrenzy, GetTarget(), 10);
		int AtkRounds = 3;
		int skillmod = 100*GetSkill(SkillFrenzy)/MaxSkill(SkillFrenzy);
		int32 max_dmg = (26 + ((((GetLevel()-6) * 2)*skillmod)/100)) * ((100+RuleI(Combat, FrenzyBonus))/100);
		int32 min_dmg = 0;
		DoAnim(anim2HSlashing);

		if (GetLevel() < 51)
			min_dmg = 1;
		else
			min_dmg = GetLevel()*8/10;

		if (min_dmg > max_dmg)
			max_dmg = min_dmg;

		ReuseTime = FrenzyReuseTime-1;
		ReuseTime = (ReuseTime*HasteMod)/100;

		//Live parses show around 55% Triple 35% Double 10% Single, you will always get first hit.
		while(AtkRounds > 0) {

			if (GetTarget() && (AtkRounds == 1 || MakeRandomInt(0,100) < 75)){
				DoSpecialAttackDamage(GetTarget(), SkillFrenzy, max_dmg, min_dmg, max_dmg , ReuseTime, true);
			}
			AtkRounds--;
		}

		if(ReuseTime > 0 && !IsRiposte) {
			p_timers.Start(pTimerCombatAbility, ReuseTime);
		}
		return;
	}

	if(skill_to_use == SkillKick)
	{
		if(ca_target!=this)
		{
			DoAnim(animKick);

			if(GetWeaponDamage(ca_target, GetInv().GetItem(SLOT_FEET)) <= 0){
				dmg = -5;
			}
			else{
				if(!ca_target->CheckHitChance(this, SkillKick, 0)) {
					dmg = 0;
				}
				else{
					if(RuleB(Combat, UseIntervalAC))
						dmg = GetKickDamage();
					else
						dmg = MakeRandomInt(1, GetKickDamage());
				}
			}

			ReuseTime = KickReuseTime-1;

			DoSpecialAttackDamage(ca_target, SkillKick, dmg, 1,-1, ReuseTime);
		}
	}

	if(skill_to_use == SkillFlyingKick ||
		skill_to_use == SkillDragonPunch ||
		skill_to_use == SkillEagleStrike ||
		skill_to_use == SkillTigerClaw ||
		skill_to_use == SkillRoundKick)
	{
		ReuseTime = MonkSpecialAttack(ca_target, skill_to_use) - 1;
		MonkSpecialAttack(ca_target, skill_to_use);

		if (IsRiposte)
			return;

		//Live AA - Technique of Master Wu
		uint16 bDoubleSpecialAttack = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;
		if( bDoubleSpecialAttack && (bDoubleSpecialAttack >= 100 || bDoubleSpecialAttack > MakeRandomInt(0,100))) {

			int MonkSPA [5] = { SkillFlyingKick, SkillDragonPunch, SkillEagleStrike, SkillTigerClaw, SkillRoundKick };
			MonkSpecialAttack(ca_target, MonkSPA[MakeRandomInt(0,4)]);

			int TripleChance = 25;

			if (bDoubleSpecialAttack > 100)
				TripleChance += TripleChance*(100-bDoubleSpecialAttack)/100;

			if(TripleChance > MakeRandomInt(0,100)) {
				MonkSpecialAttack(ca_target, MonkSPA[MakeRandomInt(0,4)]);
			}
		}
	}

	if(skill_to_use == SkillBackstab)
	{
		ReuseTime = BackstabReuseTime-1;

		if (IsRiposte)
			ReuseTime=0;

		TryBackstab(ca_target,ReuseTime);
	}

	ReuseTime = (ReuseTime*HasteMod)/100;
	if(ReuseTime > 0 && !IsRiposte)
	{
		p_timers.Start(pTimerCombatAbility, ReuseTime);
	}
}

/*
void Mob::Taunt(NPC* who, bool always_succeed, float chance_bonus) {

	if (who == nullptr)
		return;

	if(DivineAura())
		return;

	if(!CombatRange(who))
		return;

	if(!always_succeed && IsClient())
		CastToClient()->CheckIncreaseSkill(TAUNT, who, 10);

	int level = GetLevel();
	Mob *hate_top = who->GetHateMost();

	// Check to see if we're already at the top of the target's hate list
	// a mob will not be taunted if its target's health is below 20%
	if ((hate_top != this)
	&& (who->GetLevel() < level)
	&& (hate_top == nullptr || hate_top->GetHPRatio() >= 20) ) {
		int32 newhate, tauntvalue;

		float tauntchance;
		if(always_succeed) {
			tauntchance = 101;
		} else {

			// no idea how taunt success is actually calculated
			// TODO: chance for level 50+ mobs should be lower
			int level_difference = level - who->GetLevel();
			if (level_difference <= 5) {
				tauntchance = 25.0;	// minimum
				tauntchance += tauntchance * (float)GetSkill(TAUNT) / 200.0;	// skill modifier
				if (tauntchance > 65.0)
					tauntchance = 65.0;
			}
			else if (level_difference <= 10) {
				tauntchance = 30.0;	// minimum
				tauntchance += tauntchance * (float)GetSkill(TAUNT) / 200.0;	// skill modifier
				if (tauntchance > 85.0)
					tauntchance = 85.0;
			}
			else if (level_difference <= 15) {
				tauntchance = 40.0;	// minimum
				tauntchance += tauntchance * (float)GetSkill(TAUNT) / 200.0;	// skill modifier
				if (tauntchance > 90.0)
					tauntchance = 90.0;
			}
			else {
				tauntchance = 50.0;	// minimum
				tauntchance += tauntchance * (float)GetSkill(TAUNT) / 200.0;	// skill modifier
				if (tauntchance > 95.0)
					tauntchance = 95.0;
			}
		}

		if (chance_bonus)
			tauntchance = tauntchance + (tauntchance*chance_bonus/100.0f);

		if (tauntchance > MakeRandomFloat(0, 100)) {
			// this is the max additional hate added per succesfull taunt
			tauntvalue = (MakeRandomInt(2, 4) * level);
			//tauntvalue = (int32) ((float)level * 10.0 * (float)rand()/(float)RAND_MAX + 1);
			// new hate: find diff of player's hate and whoever's at top of list, add that plus tauntvalue to players hate
			newhate = who->GetNPCHate(hate_top) - who->GetNPCHate(this) + tauntvalue;
			// add the hate
			who->CastToNPC()->AddToHateList(this, newhate);
		}
		else{
			//generate at least some hate reguardless of the outcome.
			who->CastToNPC()->AddToHateList(this, (MakeRandomInt(2, 4)*level));
		}
	}

	//generate at least some hate reguardless of the outcome.
	who->CastToNPC()->AddToHateList(this, (MakeRandomInt(2, 4)*level));
	if (HasSkillProcs()){
		float chance = (float)TauntReuseTime*RuleR(Combat, AvgProcsPerMinute)/60000.0f;
		TrySkillProc(who, TAUNT, chance);
	}
}
*/

void Mob::Taunt(NPC* who, bool always_succeed, float chance_bonus) {

	if (who == nullptr)
		return;

	if(DivineAura())
		return;

	if(!CombatRange(who))
		return;

	if(!always_succeed && IsClient())
		CastToClient()->CheckIncreaseSkill(SkillTaunt, who, 10);

	Mob *hate_top = who->GetHateMost();

	float level_difference = GetLevel() - who->GetLevel();

	//Support for how taunt worked pre 2000 on LIVE - Can not taunt NPC over your level.
	if ((RuleB(Combat,TauntOverLevel) == false) && (level_difference < 0) || who->GetSpecialAbility(IMMUNE_TAUNT)){
		Message_StringID(MT_SpellFailure,FAILED_TAUNT);
		return;
	}

	//All values used based on live parses after taunt was updated in 2006.
	if ((hate_top && hate_top->GetHPRatio() >= 20) || hate_top == nullptr) {

		int32 newhate = 0;
		float tauntchance = 50.0f;

		if(always_succeed)
			tauntchance = 101.0f;

		else {

			if (level_difference < 0){
				tauntchance += level_difference*3;
				if (tauntchance < 20)
					tauntchance = 20.0f;
			}

			else {
				tauntchance += level_difference*5;
				if (tauntchance > 65)
					tauntchance = 65.0f;
			}
		}

		//TauntSkillFalloff rate is not based on any real data. Default of 33% gives a reasonable result.
		if (IsClient() && !always_succeed)
			tauntchance -= (RuleR(Combat,TauntSkillFalloff) * (CastToClient()->MaxSkill(SkillTaunt) - GetSkill(SkillTaunt)));

		//From SE_Taunt (Does a taunt with a chance modifier)
		if (chance_bonus)
			tauntchance += tauntchance*chance_bonus/100.0f;

		if (tauntchance < 1)
			tauntchance = 1.0f;

		tauntchance /= 100.0f;

		if (tauntchance > MakeRandomFloat(0, 1)) {

			if (hate_top && hate_top != this){
				newhate = (who->GetNPCHate(hate_top) - who->GetNPCHate(this)) + 1;
				who->CastToNPC()->AddToHateList(this, newhate);
			}
			else
				who->CastToNPC()->AddToHateList(this,12);

			if (who->CanTalk())
				who->Say_StringID(SUCCESSFUL_TAUNT,GetCleanName());
		}
		else{
			Message_StringID(MT_SpellFailure,FAILED_TAUNT);
		}
	}

	else
		Message_StringID(MT_SpellFailure,FAILED_TAUNT);

	if (HasSkillProcs()){
		float chance = (float)TauntReuseTime*RuleR(Combat, AvgProcsPerMinute)/60000.0f;
		TrySkillProc(who, SkillTaunt, chance);
	}
}


void Mob::InstillDoubt(Mob *who) {
	//make sure we can use this skill
	/*int skill = GetSkill(INTIMIDATION);*/	//unused

	//make sure our target is an NPC
	if(!who || !who->IsNPC())
		return;

	if(DivineAura())
		return;

	//range check
	if(!CombatRange(who))
		return;

	if(IsClient())
	{
		CastToClient()->CheckIncreaseSkill(SkillIntimidation, who, 10);
	}

	//I think this formula needs work
	int value = 0;

	//user's bonus
	value += GetSkill(SkillIntimidation) + GetCHA()/4;

	//target's counters
	value -= target->GetLevel()*4 + who->GetWIS()/4;

	if (MakeRandomInt(0,99) < value) {
		//temporary hack...
		//cast fear on them... should prolly be a different spell
		//and should be un-resistable.
		SpellOnTarget(229, who, false, true, -2000);
		//is there a success message?
	} else {
		Message_StringID(4,NOT_SCARING);
		//Idea from WR:
		/* if (target->IsNPC() && MakeRandomInt(0,99) < 10 ) {
			entity_list.MessageClose(target, false, 50, MT_NPCRampage, "%s lashes out in anger!",target->GetName());
			//should we actually do this? and the range is completely made up, unconfirmed
			entity_list.AEAttack(target, 50);
		}*/
	}
}

bool Mob::TryHeadShot(Mob* defender, SkillUseTypes skillInUse) {
	bool Result = false;

	if(defender && skillInUse == SkillArchery) {
		if(GetAA(aaHeadshot) && defender->GetBodyType() == BT_Humanoid) {
			if((GetLevelCon(GetLevel(), defender->GetLevel()) == CON_LIGHTBLUE || GetLevelCon(GetLevel(), defender->GetLevel()) == CON_GREEN) && defender->GetLevel() <= 60 && !defender->IsClient()) {
				// WildcardX: These chance formula's below are arbitrary. If someone has a better formula that is more
				// consistent with live, feel free to update these.
				int AttackerChance = 20 + ((GetLevel() - 51) / 2) + (itembonuses.HeroicDEX / 10);
				int DefenderChance = MakeRandomInt(0, 100);
				if(AttackerChance > DefenderChance) {
					mlog(COMBAT__ATTACKS, "Landed a headshot: Attacker chance was %f and Defender chance was %f.", AttackerChance, DefenderChance);
					entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, FATAL_BOW_SHOT, GetName());
					defender->Damage(this, 32000, SPELL_UNKNOWN, skillInUse);
					Result = true;
				}
				else {
					mlog(COMBAT__ATTACKS, "FAILED a headshot: Attacker chance was %f and Defender chance was %f.", AttackerChance, DefenderChance);
				}
			}
		}
	}

	return Result;
}

void Mob::DoMeleeSkillAttackDmg(Mob* other, uint16 weapon_damage, SkillUseTypes skillinuse, int16 chance_mod, int16 focus, bool CanRiposte)
{
	if (!CanDoSpecialAttack(other))
		return;

	//For spells using skill value 98 (feral swipe ect) server sets this to 67 automatically.
	//Kayen: This is unlikely to be completely accurate but use OFFENSE skill value for these effects.
	if (skillinuse == SkillBegging)
		skillinuse = SkillOffense;

	int damage = 0;
	uint32 hate = 0;
	int Hand = 13;
	if (hate == 0 && weapon_damage > 1) hate = weapon_damage;

	if(weapon_damage > 0){

		if(GetClass() == BERSERKER){
			int bonus = 3 + GetLevel()/10;
			weapon_damage = weapon_damage * (100+bonus) / 100;
		}

		int32 min_hit = 1;
		int32 max_hit = (2*weapon_damage*GetDamageTable(skillinuse)) / 100;

		if(GetLevel() >= 28 && IsWarriorClass() )
		{
			int ucDamageBonus = GetWeaponDamageBonus((const Item_Struct*) nullptr );

			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}

		ApplySpecialAttackMod(skillinuse, max_hit, min_hit);

		min_hit += min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100;

		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = MakeRandomInt(min_hit, max_hit);

		if(!other->CheckHitChance(this, skillinuse, Hand, chance_mod)) {
			damage = 0;
		} else {
			other->AvoidDamage(this, damage, CanRiposte);
			other->MeleeMitigation(this, damage, min_hit);
			if(damage > 0) {
				damage += damage*focus/100;
				ApplyMeleeDamageBonus(skillinuse, damage);
				damage += other->GetFcDamageAmtIncoming(this, 0, true, skillinuse);
				damage += (itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse) / 100) + GetSkillDmgAmt(skillinuse);
				TryCriticalHit(other, skillinuse, damage);
			}
		}

		if (damage == -3) {
			DoRiposte(other);
			if (HasDied())
				return;
		}
	}

	else
		damage = -5;

	if(skillinuse == SkillBash){
		if(IsClient()){
			ItemInst *item = CastToClient()->GetInv().GetItem(SLOT_SECONDARY);
			if(item){
				if(item->GetItem()->ItemType == ItemTypeShield)	{
					hate += item->GetItem()->AC;
				}
				const Item_Struct *itm = item->GetItem();
				hate = hate * (100 + GetFuriousBash(itm->Focus.Effect)) / 100;
			}
		}
	}

	other->AddToHateList(this, hate);

	bool CanSkillProc = true;
	if (skillinuse == SkillOffense){ //Hack to allow damage to display.
		skillinuse = SkillTigerClaw; //'strike' your opponent - Arbitrary choice for message.
		CanSkillProc = false; //Disable skill procs
	}

	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);

	if (HasDied())
		return;

	CheckNumHitsRemaining(5);

	if(aabonuses.SpecialAttackKBProc[0] && aabonuses.SpecialAttackKBProc[1] == skillinuse){
		int kb_chance = 25;
		kb_chance += kb_chance*(100-aabonuses.SpecialAttackKBProc[0])/100;

		if (MakeRandomInt(0, 99) < kb_chance)
			SpellFinished(904, other, 10, 0, -1, spells[904].ResistDiff);
	}

	if (CanSkillProc && HasSkillProcs()){
		float chance = 10.0f*RuleR(Combat, AvgProcsPerMinute)/60000.0f;
		TrySkillProc(other, skillinuse, chance);
	}
}

bool Mob::CanDoSpecialAttack(Mob *other)
{
	//Make sure everything is valid before doing any attacks.
	if (!other) {
		SetTarget(nullptr);
		return false;
	}

	if(!GetTarget())
		SetTarget(other);

	if ((other == nullptr || ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead))
		|| HasDied() || (!IsAttackAllowed(other)))) {
		return false;
	}

	if(other->GetInvul() || other->GetSpecialAbility(IMMUNE_MELEE))
		return false;

	return true;
}
