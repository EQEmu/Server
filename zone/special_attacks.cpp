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

#include "../common/rulesys.h"
#include "../common/string_util.h"

#include "client.h"
#include "entity.h"
#include "mob.h"
#include "string_ids.h"

#include <string.h>


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
				item_slot = EQEmu::legacy::SlotFeet;
				break;

			case SkillBash:
				item_slot = EQEmu::legacy::SlotSecondary;
				break;

			case SkillDragonPunch:
			case SkillEagleStrike:
			case SkillTigerClaw:
				item_slot = EQEmu::legacy::SlotHands;
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

void Mob::DoSpecialAttackDamage(Mob *who, SkillUseTypes skill, int32 max_damage, int32 min_damage, int32 hate_override,int ReuseTime,
								bool HitChance, bool CanAvoid) {
	//this really should go through the same code as normal melee damage to
	//pick up all the special behavior there

	if ((who == nullptr || ((IsClient() && CastToClient()->dead) || (who->IsClient() && who->CastToClient()->dead)) || HasDied() || (!IsAttackAllowed(who))))
		return;
	
	if(who->GetInvul() || who->GetSpecialAbility(IMMUNE_MELEE))
		max_damage = -5;

	if (who->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE) && skill != SkillBackstab)
		max_damage = -5;

	uint32 hate = max_damage;
	if(hate_override > -1)
		hate = hate_override;

	if(skill == SkillBash){
		if(IsClient()){
			ItemInst *item = CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotSecondary);
			if(item)
			{
				if(item->GetItem()->ItemType == ItemTypeShield)
				{
					hate += item->GetItem()->AC;
				}
				const Item_Struct *itm = item->GetItem();
				auto fbash = GetFuriousBash(itm->Focus.Effect);
				hate = hate * (100 + fbash) / 100;
				if (fbash)
					Message_StringID(MT_Spells, GLOWS_RED, itm->Name);
			}
		}
	}

	min_damage += min_damage * GetMeleeMinDamageMod_SE(skill) / 100;

	int hand = EQEmu::legacy::SlotPrimary; // Avoid checks hand for throwing/archery exclusion, primary should work for most
	if (skill == SkillThrowing || skill == SkillArchery)
		hand = EQEmu::legacy::SlotRange;
	if (who->AvoidDamage(this, max_damage, hand)) {
		if (max_damage == -3)
			DoRiposte(who);
	} else {
		if (HitChance || who->CheckHitChance(this, skill, EQEmu::legacy::SlotPrimary)) {
			who->MeleeMitigation(this, max_damage, min_damage);
			CommonOutgoingHitSuccess(who, max_damage, skill);
		} else {
			max_damage = 0;
		}
	}

	who->AddToHateList(this, hate, 0, false);
	if (max_damage > 0 && aabonuses.SkillAttackProc[0] && aabonuses.SkillAttackProc[1] == skill &&
	    IsValidSpell(aabonuses.SkillAttackProc[2])) {
		float chance = aabonuses.SkillAttackProc[0] / 1000.0f;
		if (zone->random.Roll(chance))
			SpellFinished(aabonuses.SkillAttackProc[2], who, 10, 0, -1,
				      spells[aabonuses.SkillAttackProc[2]].ResistDiff);
	}
	who->Damage(this, max_damage, SPELL_UNKNOWN, skill, false);

	//Make sure 'this' has not killed the target and 'this' is not dead (Damage shield ect).
	if(!GetTarget())return;
	if (HasDied())	return;

	if (HasSkillProcs())
		TrySkillProc(who, skill, ReuseTime*1000);

	if (max_damage > 0 && HasSkillProcSuccess())
		TrySkillProc(who, skill, ReuseTime*1000, true);

}


void Client::OPCombatAbility(const EQApplicationPacket *app) {
	if(!GetTarget())
		return;
	//make sure were actually able to use such an attack.
	if(spellend_timer.Enabled() || IsFeared() || IsStunned() || IsMezzed() || DivineAura() || dead)
		return;

	CombatAbility_Struct* ca_atk = (CombatAbility_Struct*) app->pBuffer;
	pTimerType timer = pTimerCombatAbility;
	// RoF2+ Tiger Claw is unlinked from other monk skills, if they ever do that for other classes there will need
	// to be more checks here
	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF2 && ca_atk->m_skill == SkillTigerClaw)
		timer = pTimerCombatAbility2;

	/* Check to see if actually have skill */
	if (!MaxSkill(static_cast<SkillUseTypes>(ca_atk->m_skill)))
		return;

	if(GetTarget()->GetID() != ca_atk->m_target)
		return;	//invalid packet.

	if(!IsAttackAllowed(GetTarget()))
		return;

	//These two are not subject to the combat ability timer, as they
	//allready do their checking in conjunction with the attack timer
	//throwing weapons
	if (ca_atk->m_atk == EQEmu::legacy::SlotRange) {
		if (ca_atk->m_skill == SkillThrowing) {
			SetAttackTimer();
			ThrowingAttack(GetTarget());
			if (CheckDoubleRangedAttack())
				ThrowingAttack(GetTarget(), true);
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

	if(!p_timers.Expired(&database, timer, false)) {
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

	// not sure what the '100' indicates..if ->m_atk is not used as 'slot' reference, then change SlotRange above back to '11'
	if ((ca_atk->m_atk == 100) && (ca_atk->m_skill == SkillBash)) { // SLAM - Bash without a shield equipped
		if (GetTarget() != this) {

			CheckIncreaseSkill(SkillBash, GetTarget(), 10);
			DoAnim(animTailRake);

			int32 ht = 0;
			if (GetWeaponDamage(GetTarget(), GetInv().GetItem(EQEmu::legacy::SlotSecondary)) <= 0 &&
				GetWeaponDamage(GetTarget(), GetInv().GetItem(EQEmu::legacy::SlotShoulders)) <= 0){
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
						ht = dmg = zone->random.Int(1, GetBashDamage());

				}
			}

			ReuseTime = BashReuseTime-1-skill_reduction;
			ReuseTime = (ReuseTime*HasteMod)/100;
			DoSpecialAttackDamage(GetTarget(), SkillBash, dmg, 1, ht, ReuseTime);
			if(ReuseTime > 0)
			{
				p_timers.Start(timer, ReuseTime);
			}
		}
		return;
	}

	if ((ca_atk->m_atk == 100) && (ca_atk->m_skill == SkillFrenzy)){
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
			if (GetTarget() && (AtkRounds == 1 || zone->random.Roll(75))) {
				DoSpecialAttackDamage(GetTarget(), SkillFrenzy, max_dmg, min_dmg, max_dmg , ReuseTime, true);
			}
			AtkRounds--;
		}

		if(ReuseTime > 0) {
			p_timers.Start(timer, ReuseTime);
		}
		return;
	}

	switch(GetClass()){
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
				if (GetWeaponDamage(GetTarget(), GetInv().GetItem(EQEmu::legacy::SlotFeet)) <= 0){
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
							ht = dmg = zone->random.Int(1, GetKickDamage());
					}
				}

				ReuseTime = KickReuseTime-1-skill_reduction;
				DoSpecialAttackDamage(GetTarget(), SkillKick, dmg, 1, ht, ReuseTime);

			}
			break;
		case MONK: {
			ReuseTime = MonkSpecialAttack(GetTarget(), ca_atk->m_skill) - 1 - skill_reduction;

			//Live AA - Technique of Master Wu
			int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;
			if (wuchance) {
				if (wuchance >= 100 || zone->random.Roll(wuchance)) {
					int MonkSPA [5] = { SkillFlyingKick, SkillDragonPunch, SkillEagleStrike, SkillTigerClaw, SkillRoundKick };
					int extra = 1;
					// always 1/4 of the double attack chance, 25% at rank 5 (100/4)
					if (zone->random.Roll(wuchance / 4))
						extra++;
					// They didn't add a string ID for this.
					std::string msg = StringFormat("The spirit of Master Wu fills you!  You gain %d additional attack(s).", extra);
					// live uses 400 here -- not sure if it's the best for all clients though
					SendColoredText(400, msg);
					while (extra) {
						MonkSpecialAttack(GetTarget(), MonkSPA[zone->random.Int(0, 4)]);
						extra--;
					}
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
			ReuseTime = BackstabReuseTime-1 - skill_reduction;
			TryBackstab(GetTarget(), ReuseTime);
			break;
		}
		default:
			//they have no abilities... wtf? make em wait a bit
			ReuseTime = 9 - skill_reduction;
			break;
	}

	ReuseTime = (ReuseTime*HasteMod)/100;
	if(ReuseTime > 0){
		p_timers.Start(timer, ReuseTime);
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
	uint8 itemslot = EQEmu::legacy::SlotFeet;

	switch(unchecked_type){
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
			itemslot = EQEmu::legacy::SlotHands;
			ApplySpecialAttackMod(skill_type, max_dmg, min_dmg);
			DoAnim(animTailRake);
			reuse = TailRakeReuseTime;
			break;
		}

		case SkillEagleStrike:{
			skill_type = SkillEagleStrike;
			max_dmg = ((GetSTR()+GetSkill(skill_type)) * RuleI(Combat, EagleStrikeBonus) / 100) + 19;
			itemslot = EQEmu::legacy::SlotHands;
			ApplySpecialAttackMod(skill_type, max_dmg, min_dmg);
			DoAnim(animEagleStrike);
			reuse = EagleStrikeReuseTime;
			break;
		}

		case SkillTigerClaw:{
			skill_type = SkillTigerClaw;
			max_dmg = ((GetSTR()+GetSkill(skill_type)) * RuleI(Combat, TigerClawBonus) / 100) + 12;
			itemslot = EQEmu::legacy::SlotHands;
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
			Log.Out(Logs::Detail, Logs::Attack, "Invalid special attack type %d attempted", unchecked_type);
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
				ht = ndamage = zone->random.Int(min_dmg, max_dmg);
		}
		else{
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
		const ItemInst *wpn = CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotPrimary);
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

		if (FrontalBSChance && zone->random.Roll(FrontalBSChance))
			bCanFrontalBS = true;
	}

	if (bIsBehind || bCanFrontalBS){ // Player is behind other OR can do Frontal Backstab
		if (bCanFrontalBS)
			CastToClient()->Message(0,"Your fierce attack is executed with such grace, your target did not see it coming!");

		RogueBackstab(other,false,ReuseTime);
		if (level > 54) {
			if(IsClient() && CastToClient()->CheckDoubleAttack())
			{
				if(other->GetHP() > 0)
					RogueBackstab(other,false,ReuseTime);

				if (tripleChance && other->GetHP() > 0 && zone->random.Roll(tripleChance))
					RogueBackstab(other,false,ReuseTime);
			}
		}

		if(IsClient())
			CastToClient()->CheckIncreaseSkill(SkillBackstab, other, 10);

	}
	//Live AA - Chaotic Backstab
	else if(aabonuses.FrontalBackstabMinDmg || itembonuses.FrontalBackstabMinDmg || spellbonuses.FrontalBackstabMinDmg) {

		//we can stab from any angle, we do min damage though.
		RogueBackstab(other, true, ReuseTime);
		if (level > 54) {

			// Check for double attack with main hand assuming maxed DA Skill (MS)
			if(IsClient() && CastToClient()->CheckDoubleAttack())
				if(other->GetHP() > 0)
					RogueBackstab(other,true, ReuseTime);

			if (tripleChance && other->GetHP() > 0 && zone->random.Roll(tripleChance))
					RogueBackstab(other,false,ReuseTime);
		}

		if(IsClient())
			CastToClient()->CheckIncreaseSkill(SkillBackstab, other, 10);
	}
	else { //We do a single regular attack if we attack from the front without chaotic stab
		Attack(other, EQEmu::legacy::SlotPrimary);
	}
}

//heko: backstab
void Mob::RogueBackstab(Mob* other, bool min_damage, int ReuseTime)
{
	if (!other)
		return;

	int32 ndamage = 0;
	int32 max_hit = 0;
	int32 min_hit = 0;
	uint32 hate = 0;
	int32 primaryweapondamage = 0;
	int32 backstab_dmg = 0;

	if(IsClient()){
		const ItemInst *wpn = nullptr;
		wpn = CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotPrimary);
		if(wpn) {
			primaryweapondamage = GetWeaponDamage(other, wpn);
			backstab_dmg = wpn->GetItem()->BackstabDmg;
			for (int i = 0; i < EQEmu::legacy::ITEM_COMMON_SIZE; ++i)
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
			max_hit = (((((2*backstab_dmg) * GetDamageTable(SkillBackstab) / 100) * 10 * GetSkill(SkillBackstab) / 355) + ((level-25)/3) + 1) * ((100 + RuleI(Combat, BackstabBonus)) / 100));
			hate = 20 * backstab_dmg * GetSkill(SkillBackstab) / 355;
		}
		else{
			max_hit = (((((2*backstab_dmg) * GetDamageTable(SkillBackstab) / 100) * 10 * GetSkill(SkillBackstab) / 355) + 1) * ((100 + RuleI(Combat, BackstabBonus)) / 100));
			hate = 20 * backstab_dmg * GetSkill(SkillBackstab) / 355;
		}

		// determine minimum hits
		if (level < 51) {
			min_hit = (level*15/10);
		}
		else {
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
			else {
				if (max_hit < min_hit)
					max_hit = min_hit;

				if(RuleB(Combat, UseIntervalAC))
					ndamage = max_hit;
				else
					ndamage = zone->random.Int(min_hit, max_hit);
			}
		}
	}
	else{
		ndamage = -5;
	}

	ndamage = mod_backstab_damage(ndamage);

	uint32 Assassinate_Dmg = 0;
	Assassinate_Dmg = TryAssassinate(other, SkillBackstab, ReuseTime);

	if (Assassinate_Dmg) {
		ndamage = Assassinate_Dmg;
		entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, ASSASSINATES, GetName());
	}

	DoSpecialAttackDamage(other, SkillBackstab, ndamage, min_hit, hate, ReuseTime, false, false);
	DoAnim(anim1HPiercing);
}

// assassinate [No longer used for regular assassinate 6-29-14]
void Mob::RogueAssassinate(Mob* other)
{
	//can you dodge, parry, etc.. an assassinate??
	//if so, use DoSpecialAttackDamage(other, BACKSTAB, 32000); instead
	if (GetWeaponDamage(other, IsClient() ? CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotPrimary) : (const ItemInst*)nullptr) > 0){
		other->Damage(this, 32000, SPELL_UNKNOWN, SkillBackstab);
	}else{
		other->Damage(this, -5, SPELL_UNKNOWN, SkillBackstab);
	}
	DoAnim(anim1HPiercing);	//piercing animation
}

void Client::RangedAttack(Mob* other, bool CanDoubleAttack) {
	//conditions to use an attack checked before we are called
	if (!other)
		return;
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if(!CanDoubleAttack && ((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		Log.Out(Logs::Detail, Logs::Combat, "Throwing attack canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}
	const ItemInst* RangeWeapon = m_inv[EQEmu::legacy::SlotRange];

	//locate ammo
	int ammo_slot = EQEmu::legacy::SlotAmmo;
	const ItemInst* Ammo = m_inv[EQEmu::legacy::SlotAmmo];

	if (!RangeWeapon || !RangeWeapon->IsType(ItemClassCommon)) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack canceled. Missing or invalid ranged weapon (%d) in slot %d", GetItemIDAt(EQEmu::legacy::SlotRange), EQEmu::legacy::SlotRange);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have no bow!", GetItemIDAt(EQEmu::legacy::SlotRange));
		return;
	}
	if (!Ammo || !Ammo->IsType(ItemClassCommon)) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack canceled. Missing or invalid ammo item (%d) in slot %d", GetItemIDAt(EQEmu::legacy::SlotAmmo), EQEmu::legacy::SlotAmmo);
		Message(0, "Error: Ammo: GetItem(%i)==0, you have no ammo!", GetItemIDAt(EQEmu::legacy::SlotAmmo));
		return;
	}

	const Item_Struct* RangeItem = RangeWeapon->GetItem();
	const Item_Struct* AmmoItem = Ammo->GetItem();

	if(RangeItem->ItemType != ItemTypeBow) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack canceled. Ranged item is not a bow. type %d.", RangeItem->ItemType);
		Message(0, "Error: Rangeweapon: Item %d is not a bow.", RangeWeapon->GetID());
		return;
	}
	if(AmmoItem->ItemType != ItemTypeArrow) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack canceled. Ammo item is not an arrow. type %d.", AmmoItem->ItemType);
		Message(0, "Error: Ammo: type %d != %d, you have the wrong type of ammo!", AmmoItem->ItemType, ItemTypeArrow);
		return;
	}

	Log.Out(Logs::Detail, Logs::Combat, "Shooting %s with bow %s (%d) and arrow %s (%d)", other->GetName(), RangeItem->Name, RangeItem->ID, AmmoItem->Name, AmmoItem->ID);

	//look for ammo in inventory if we only have 1 left...
	if(Ammo->GetCharges() == 1) {
		//first look for quivers
		int r;
		bool found = false;
		for (r = EQEmu::legacy::GENERAL_BEGIN; r <= EQEmu::legacy::GENERAL_END; r++) {
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
					Log.Out(Logs::Detail, Logs::Combat, "Using ammo from quiver stack at slot %d. %d in stack.", ammo_slot, Ammo->GetCharges());
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
			if (aslot != INVALID_INDEX) {
				ammo_slot = aslot;
				Ammo = m_inv[aslot];
				Log.Out(Logs::Detail, Logs::Combat, "Using ammo from inventory stack at slot %d. %d in stack.", ammo_slot, Ammo->GetCharges());
			}
		}
	}

	float range = RangeItem->Range + AmmoItem->Range + GetRangeDistTargetSizeMod(GetTarget());
	Log.Out(Logs::Detail, Logs::Combat, "Calculated bow range to be %.1f", range);
	range *= range;
	float dist = DistanceSquared(m_Position, other->GetPosition());
	if(dist > range) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack out of range... client should catch this. (%f > %f).\n", dist, range);
		Message_StringID(13,TARGET_OUT_OF_RANGE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}
	else if(dist < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		Message_StringID(15,RANGED_TOO_CLOSE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}

	if(!IsAttackAllowed(other) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}

	//Shoots projectile and/or applies the archery damage
	DoArcheryAttackDmg(other, RangeWeapon, Ammo,0,0,0,0,0,0, AmmoItem, ammo_slot);

	//EndlessQuiver AA base1 = 100% Chance to avoid consumption arrow.
	int ChanceAvoidConsume = aabonuses.ConsumeProjectile + itembonuses.ConsumeProjectile + spellbonuses.ConsumeProjectile;

	if (!ChanceAvoidConsume || (ChanceAvoidConsume < 100 && zone->random.Int(0,99) > ChanceAvoidConsume)){
		DeleteItemInInventory(ammo_slot, 1, true);
		Log.Out(Logs::Detail, Logs::Combat, "Consumed one arrow from slot %d", ammo_slot);
	} else {
		Log.Out(Logs::Detail, Logs::Combat, "Endless Quiver prevented ammo consumption.");
	}

	CheckIncreaseSkill(SkillArchery, GetTarget(), -15);
	CommonBreakInvisibleFromCombat();
}

void Mob::DoArcheryAttackDmg(Mob* other,  const ItemInst* RangeWeapon, const ItemInst* Ammo, uint16 weapon_damage, int16 chance_mod, int16 focus, int ReuseTime,
							uint32 range_id, uint32 ammo_id, const Item_Struct *AmmoItem, int AmmoSlot, float speed) {

	if ((other == nullptr ||
		((IsClient() && CastToClient()->dead) ||
		(other->IsClient() && other->CastToClient()->dead)) ||
		HasDied() ||
		(!IsAttackAllowed(other)) ||
		(other->GetInvul() ||
		other->GetSpecialAbility(IMMUNE_MELEE))))
	{
		return;
	}

	const ItemInst* _RangeWeapon = nullptr;
	const ItemInst* _Ammo = nullptr;
	const Item_Struct* ammo_lost = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool LaunchProjectile = false;
	bool ProjectileImpact = false;
	bool ProjectileMiss = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)){

		if (AmmoItem)
			LaunchProjectile = true;
		else{
			/*
			Item sync check on projectile landing.
			Weapon damage is already calculated so this only affects procs!
			Ammo proc check will use database to find proc if you used up your last ammo.
			If you change range item mid projectile flight, you loose your chance to proc from bow (Deal with it!).
			*/

			if (!RangeWeapon && !Ammo && range_id && ammo_id){

				ProjectileImpact = true;

				if (weapon_damage == 0)
					ProjectileMiss = true; //This indicates that MISS was originally calculated.

				if (IsClient()){

					_RangeWeapon = CastToClient()->m_inv[EQEmu::legacy::SlotRange];
					if (_RangeWeapon && _RangeWeapon->GetItem() && _RangeWeapon->GetItem()->ID == range_id)
						RangeWeapon = _RangeWeapon;

					_Ammo = CastToClient()->m_inv[AmmoSlot];
					if (_Ammo && _Ammo->GetItem() && _Ammo->GetItem()->ID == ammo_id)
						Ammo = _Ammo;
					else
						ammo_lost = database.GetItem(ammo_id);
				}
			}
		}
	}
	else if (AmmoItem)
		SendItemAnimation(other, AmmoItem, SkillArchery);

	if (ProjectileMiss || (!ProjectileImpact && !other->CheckHitChance(this, SkillArchery, EQEmu::legacy::SlotPrimary, chance_mod))) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack missed %s.", other->GetName());

		if (LaunchProjectile){
			TryProjectileAttack(other, AmmoItem, SkillArchery, 0, RangeWeapon, Ammo, AmmoSlot, speed);
			return;
		}
		else
			other->Damage(this, 0, SPELL_UNKNOWN, SkillArchery);
	} else {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack hit %s.", other->GetName());

		bool HeadShot = false;
		uint32 HeadShot_Dmg = TryHeadShot(other, SkillArchery);
		if (HeadShot_Dmg)
			HeadShot = true;

		uint32 hate = 0;
		int32 TotalDmg = 0;
		int16 WDmg = 0;
		int16 ADmg = 0;
		if (!weapon_damage){
			WDmg = GetWeaponDamage(other, RangeWeapon);
			ADmg = GetWeaponDamage(other, Ammo);
		}
		else
			WDmg = weapon_damage;

		if (LaunchProjectile){//1: Shoot the Projectile once we calculate weapon damage.
			TryProjectileAttack(other, AmmoItem, SkillArchery, (WDmg + ADmg), RangeWeapon, Ammo, AmmoSlot, speed);
			return;
		}

		if (focus) //From FcBaseEffects
			WDmg += WDmg*focus/100;

		if((WDmg > 0) || (ADmg > 0)) {
			if(WDmg < 0)
				WDmg = 0;
			if(ADmg < 0)
				ADmg = 0;
			uint32 MaxDmg = (RuleR(Combat, ArcheryBaseDamageBonus)*(WDmg+ADmg)*GetDamageTable(SkillArchery)) / 100;
			hate = ((WDmg+ADmg));

			if (HeadShot)
				MaxDmg = HeadShot_Dmg;

			uint16 bonusArcheryDamageModifier = aabonuses.ArcheryDamageModifier + itembonuses.ArcheryDamageModifier + spellbonuses.ArcheryDamageModifier;

			MaxDmg += MaxDmg*bonusArcheryDamageModifier / 100;

			if (RuleB(Combat, ProjectileDmgOnImpact))
				Log.Out(Logs::Detail, Logs::Combat, "Bow and Arrow DMG %d, Max Damage %d.", WDmg, MaxDmg);
			else
				Log.Out(Logs::Detail, Logs::Combat, "Bow DMG %d, Arrow DMG %d, Max Damage %d.", WDmg, ADmg, MaxDmg);

			bool dobonus = false;
			if(GetClass() == RANGER && GetLevel() > 50){
				int bonuschance = RuleI(Combat, ArcheryBonusChance);
				bonuschance = mod_archery_bonus_chance(bonuschance, RangeWeapon);

				if( !RuleB(Combat, UseArcheryBonusRoll) || zone->random.Roll(bonuschance)){
					if(RuleB(Combat, ArcheryBonusRequiresStationary)){
						if(other->IsNPC() && !other->IsMoving() && !other->IsRooted())
							dobonus = true;
					}
					else
						dobonus = true;
				}

				if(dobonus){
					MaxDmg *= 2;
					hate *= 2;
					MaxDmg = mod_archery_bonus_damage(MaxDmg, RangeWeapon);

					Log.Out(Logs::Detail, Logs::Combat, "Ranger. Double damage success roll, doubling damage to %d", MaxDmg);
					Message_StringID(MT_CritMelee, BOW_DOUBLE_DAMAGE);
				}
			}

			if (MaxDmg == 0)
				MaxDmg = 1;

			if(RuleB(Combat, UseIntervalAC))
				TotalDmg = MaxDmg;
			else
				TotalDmg = zone->random.Int(1, MaxDmg);

			int minDmg = 1;
			if(GetLevel() > 25){
				//twice, for ammo and weapon
				TotalDmg += (2*((GetLevel()-25)/3));
				minDmg += (2*((GetLevel()-25)/3));
				minDmg += minDmg * GetMeleeMinDamageMod_SE(SkillArchery) / 100;
				hate += (2*((GetLevel()-25)/3));
			}

			if (!HeadShot)
				other->AvoidDamage(this, TotalDmg, EQEmu::legacy::SlotRange);

			other->MeleeMitigation(this, TotalDmg, minDmg);
			if(TotalDmg > 0){
				CommonOutgoingHitSuccess(other, TotalDmg, SkillArchery);
				TotalDmg = mod_archery_damage(TotalDmg, dobonus, RangeWeapon);
			}
		}
		else
			TotalDmg = -5;

		if (HeadShot)
			entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, FATAL_BOW_SHOT, GetName());

		if (IsClient() && !CastToClient()->GetFeigned())
			other->AddToHateList(this, hate, 0, false);

		other->Damage(this, TotalDmg, SPELL_UNKNOWN, SkillArchery);

		//Skill Proc Success
		if (TotalDmg > 0 && HasSkillProcSuccess() && other && !other->HasDied()){
			if (ReuseTime)
				TrySkillProc(other, SkillArchery, ReuseTime);
			else
				TrySkillProc(other, SkillArchery, 0, true, EQEmu::legacy::SlotRange);
		}
	}

	if (LaunchProjectile)
		return;//Shouldn't reach this point durring initial launch phase, but just in case.

	//Weapon Proc
	if(RangeWeapon && other && !other->HasDied())
		TryWeaponProc(RangeWeapon, other, EQEmu::legacy::SlotRange);

	//Ammo Proc
	if (ammo_lost)
		TryWeaponProc(nullptr, ammo_lost, other, EQEmu::legacy::SlotRange);
	else if(Ammo && other && !other->HasDied())
		TryWeaponProc(Ammo, other, EQEmu::legacy::SlotRange);

	//Skill Proc
	if (HasSkillProcs() && other && !other->HasDied()){
		if (ReuseTime)
			TrySkillProc(other, SkillArchery, ReuseTime);
		else
			TrySkillProc(other, SkillArchery, 0, false, EQEmu::legacy::SlotRange);
	}
}

bool Mob::TryProjectileAttack(Mob* other, const Item_Struct *item, SkillUseTypes skillInUse, uint16 weapon_dmg, const ItemInst* RangeWeapon, const ItemInst* Ammo, int AmmoSlot, float speed){

	if (!other)
		return false;

	int slot = -1;

	//Make sure there is an avialable slot.
	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {
		if (ProjectileAtk[i].target_id == 0){
			slot = i;
			break;
		}
	}

	if (slot < 0)
		return false;

	float speed_mod = speed * 0.45f;

	float distance = other->CalculateDistance(GetX(), GetY(), GetZ());
	float hit = 60.0f + (distance / speed_mod); //Calcuation: 60 = Animation Lag, 1.8 = Speed modifier for speed of (4)

	ProjectileAtk[slot].increment = 1;
	ProjectileAtk[slot].hit_increment = static_cast<uint16>(hit); //This projected hit time if target does NOT MOVE
	ProjectileAtk[slot].target_id = other->GetID();
	ProjectileAtk[slot].wpn_dmg = weapon_dmg;
	ProjectileAtk[slot].origin_x = GetX();
	ProjectileAtk[slot].origin_y = GetY();
	ProjectileAtk[slot].origin_z = GetZ();

	if (RangeWeapon && RangeWeapon->GetItem())
		ProjectileAtk[slot].ranged_id = RangeWeapon->GetItem()->ID;

	if (Ammo && Ammo->GetItem())
		ProjectileAtk[slot].ammo_id = Ammo->GetItem()->ID;

	ProjectileAtk[slot].ammo_slot = 0;
	ProjectileAtk[slot].skill = skillInUse;
	ProjectileAtk[slot].speed_mod = speed_mod;

	SetProjectileAttack(true);

	if(item)
		SendItemAnimation(other, item, skillInUse, speed);
	//else if (IsNPC())
		//ProjectileAnimation(other, 0,false,speed,0,0,0,CastToNPC()->GetAmmoIDfile(),skillInUse);

	return true;
}


void Mob::ProjectileAttack()
{
	if (!HasProjectileAttack())
		return;;

	Mob* target = nullptr;
	bool disable = true;

	for (int i = 0; i < MAX_SPELL_PROJECTILE; i++) {

		if (ProjectileAtk[i].increment == 0){
			continue;
		}

		disable = false;
		Mob* target = entity_list.GetMobID(ProjectileAtk[i].target_id);

		if (target && target->IsMoving()){ //Only recalculate hit increment if target moving
			//Due to frequency that we need to check increment the targets position variables may not be updated even if moving. Do a simple check before calculating distance.
			if (ProjectileAtk[i].tlast_x != target->GetX() || ProjectileAtk[i].tlast_y != target->GetY()){
				ProjectileAtk[i].tlast_x = target->GetX();
				ProjectileAtk[i].tlast_y = target->GetY();
				float distance = target->CalculateDistance(ProjectileAtk[i].origin_x, ProjectileAtk[i].origin_y,  ProjectileAtk[i].origin_z);
				float hit = 60.0f + (distance / ProjectileAtk[i].speed_mod); //Calcuation: 60 = Animation Lag, 1.8 = Speed modifier for speed of (4)
				ProjectileAtk[i].hit_increment = static_cast<uint16>(hit);
			}
		}

		if (ProjectileAtk[i].hit_increment <= ProjectileAtk[i].increment){

			if (target){

				if (IsNPC()){
					if (ProjectileAtk[i].skill == SkillConjuration){
						if (IsValidSpell(ProjectileAtk[i].wpn_dmg))
							SpellOnTarget(ProjectileAtk[i].wpn_dmg, target, false, true, spells[ProjectileAtk[i].wpn_dmg].ResistDiff, true);
					}
					else
						CastToNPC()->DoRangedAttackDmg(target, false, ProjectileAtk[i].wpn_dmg,0, static_cast<SkillUseTypes>(ProjectileAtk[i].skill));
				}

				else
				{
					if (ProjectileAtk[i].skill == SkillArchery)
						DoArcheryAttackDmg(target, nullptr, nullptr,ProjectileAtk[i].wpn_dmg,0,0,0,ProjectileAtk[i].ranged_id, ProjectileAtk[i].ammo_id, nullptr, ProjectileAtk[i].ammo_slot);
					else if (ProjectileAtk[i].skill == SkillThrowing)
						DoThrowingAttackDmg(target, nullptr, nullptr,ProjectileAtk[i].wpn_dmg,0,0,0, ProjectileAtk[i].ranged_id, ProjectileAtk[i].ammo_slot);
					else if (ProjectileAtk[i].skill == SkillConjuration && IsValidSpell(ProjectileAtk[i].wpn_dmg))
						SpellOnTarget(ProjectileAtk[i].wpn_dmg, target, false, true, spells[ProjectileAtk[i].wpn_dmg].ResistDiff, true);
				}
			}

			ProjectileAtk[i].increment = 0;
			ProjectileAtk[i].target_id = 0;
			ProjectileAtk[i].wpn_dmg = 0;
			ProjectileAtk[i].origin_x = 0.0f;
			ProjectileAtk[i].origin_y = 0.0f;
			ProjectileAtk[i].origin_z = 0.0f;
			ProjectileAtk[i].tlast_x = 0.0f;
			ProjectileAtk[i].tlast_y = 0.0f;
			ProjectileAtk[i].ranged_id = 0;
			ProjectileAtk[i].ammo_id = 0;
			ProjectileAtk[i].ammo_slot = 0;
			ProjectileAtk[i].skill = 0;
			ProjectileAtk[i].speed_mod = 0.0f;
		}

		else {
			ProjectileAtk[i].increment++;
		}
	}

	if (disable)
		SetProjectileAttack(false);
}

float Mob::GetRangeDistTargetSizeMod(Mob* other)
{
	/*
	Range is enforced client side, therefore these numbers do not need to be 100% accurate just close enough to
	prevent any exploitation. The range mod changes in some situations depending on if size is from spawn or from SendIllusionPacket changes.
	At present time only calculate from spawn (it is no consistent what happens to the calc when changing it after spawn).
	*/
	if (!other)
		return 0.0f;

	float tsize = other->GetSize();

	if (GetSize() > tsize)
		tsize = GetSize();

	float mod = 0.0f;
	/*These are correct numbers if mob size is changed via #size (Don't know why it matters but it does)
	if (tsize < 7)
		mod = 16.0f;
	else if (tsize >=7 && tsize <= 20)
		mod = 16.0f + (0.6f * (tsize - 6.0f));
	else if (tsize >=20 && tsize <= 60)
		mod = 25.0f + (1.25f * (tsize - 20.0f));
	else
		mod = 75.0f;
	*/

	if (tsize < 10)
		mod = 18.0f;
	else if (tsize >=10 && tsize < 15)
		mod = 20.0f + (4.0f * (tsize - 10.0f));
	else if (tsize >=15 && tsize <= 20)
		mod = 42.0f + (5.8f * (tsize - 15.0f));
	else
		mod = 75.0f;

	return (mod + 2.0f); //Add 2.0f as buffer to prevent any chance of failures, client enforce range check regardless.
}

void NPC::RangedAttack(Mob* other)
{
	if (!other)
		return;
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check())){
		Log.Out(Logs::Detail, Logs::Combat, "Archery canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	if(!CheckLosFN(other))
		return;

	int attacks = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 0);
	attacks = attacks > 0 ? attacks : 1;
	for(int i = 0; i < attacks; ++i) {

		if(!GetSpecialAbility(SPECATK_RANGED_ATK))
			return;

		int sa_min_range = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 4); //Min Range of NPC attack
		int sa_max_range = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 1); //Max Range of NPC attack

		float min_range = static_cast<float>(RuleI(Combat, MinRangedAttackDist));
		float max_range = 250; // needs to be longer than 200(most spells)

		if (sa_max_range)
			max_range = static_cast<float>(sa_max_range);

		if (sa_min_range)
			min_range = static_cast<float>(sa_min_range);

		max_range *= max_range;
		if(DistanceSquared(m_Position, other->GetPosition()) > max_range)
			return;
		else if(DistanceSquared(m_Position, other->GetPosition()) < (min_range * min_range))
			return;

		if(!other || !IsAttackAllowed(other) ||
			IsCasting() ||
			DivineAura() ||
			IsStunned() ||
			IsFeared() ||
			IsMezzed() ||
			(GetAppearance() == eaDead)){
			return;
		}

		FaceTarget(other);

		DoRangedAttackDmg(other);

		CommonBreakInvisibleFromCombat();
	}
}

void NPC::DoRangedAttackDmg(Mob* other, bool Launch, int16 damage_mod, int16 chance_mod, SkillUseTypes skill, float speed, const char *IDFile) {

	if ((other == nullptr ||
		(other->HasDied())) ||
		HasDied() ||
		(!IsAttackAllowed(other)) ||
		(other->GetInvul() ||
		other->GetSpecialAbility(IMMUNE_MELEE)))
	{
		return;
	}

	SkillUseTypes skillInUse = static_cast<SkillUseTypes>(GetRangedSkill());

	if (skill != skillInUse)
		skillInUse = skill;

	if (Launch)
	{
		const char *ammo = "IT10";

		if (IDFile != nullptr)
			ammo = IDFile;
		else if (GetAmmoIDfile())
			ammo = GetAmmoIDfile();

		ProjectileAnimation(other, 0,false,speed,0,0,0,ammo,skillInUse);

		if (RuleB(Combat, ProjectileDmgOnImpact))
		{
			TryProjectileAttack(other, nullptr, skillInUse, damage_mod, nullptr, nullptr, 0, speed);
			return;
		}
	}

	if (!chance_mod)
		chance_mod = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 2);

	if (!other->CheckHitChance(this, skillInUse, EQEmu::legacy::SlotRange, chance_mod))
	{
		other->Damage(this, 0, SPELL_UNKNOWN, skillInUse);
	}
	else
	{
		int32 TotalDmg = 0;
		int32 MaxDmg = max_dmg * RuleR(Combat, ArcheryNPCMultiplier); // should add a field to npc_types
		int32 MinDmg = min_dmg * RuleR(Combat, ArcheryNPCMultiplier);

		if(RuleB(Combat, UseIntervalAC))
			TotalDmg = MaxDmg;
		else
			TotalDmg = zone->random.Int(MinDmg, MaxDmg);


		if (!damage_mod)
			 damage_mod = GetSpecialAbilityParam(SPECATK_RANGED_ATK, 3);//Damage modifier

		TotalDmg += TotalDmg *  damage_mod / 100;

		other->AvoidDamage(this, TotalDmg, EQEmu::legacy::SlotRange);
		other->MeleeMitigation(this, TotalDmg, MinDmg);

		if (TotalDmg > 0)
			CommonOutgoingHitSuccess(other, TotalDmg, skillInUse);
		else if (TotalDmg < -4)
			TotalDmg = -5;

		if (TotalDmg > 0)
			other->AddToHateList(this, TotalDmg, 0, false);
		else
			other->AddToHateList(this, 0, 0, false);

		other->Damage(this, TotalDmg, SPELL_UNKNOWN, skillInUse);

		if (TotalDmg > 0 && HasSkillProcSuccess() && !other->HasDied())
			TrySkillProc(other, skillInUse, 0, true, EQEmu::legacy::SlotRange);
	}

	//try proc on hits and misses
	if(other && !other->HasDied())
		TrySpellProc(nullptr, (const Item_Struct*)nullptr, other, EQEmu::legacy::SlotRange);

	if (HasSkillProcs() && other && !other->HasDied())
		TrySkillProc(other, skillInUse, 0, false, EQEmu::legacy::SlotRange);
}

uint16 Mob::GetThrownDamage(int16 wDmg, int32& TotalDmg, int& minDmg) {
	uint16 MaxDmg = (((2 * wDmg) * GetDamageTable(SkillThrowing)) / 100);

	if (MaxDmg == 0)
		MaxDmg = 1;

	if(RuleB(Combat, UseIntervalAC))
		TotalDmg = MaxDmg;
	else
		TotalDmg = zone->random.Int(1, MaxDmg);

	minDmg = 1;
	if(GetLevel() > 25){
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
	if (!other)
		return;
	//make sure the attack and ranged timers are up
	//if the ranged timer is disabled, then they have no ranged weapon and shouldent be attacking anyhow
	if((!CanDoubleAttack && (attack_timer.Enabled() && !attack_timer.Check(false)) || (ranged_timer.Enabled() && !ranged_timer.Check()))) {
		Log.Out(Logs::Detail, Logs::Combat, "Throwing attack canceled. Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		// The server and client timers are not exact matches currently, so this would spam too often if enabled
		//Message(0, "Error: Timer not up. Attack %d, ranged %d", attack_timer.GetRemainingTime(), ranged_timer.GetRemainingTime());
		return;
	}

	int ammo_slot = EQEmu::legacy::SlotRange;
	const ItemInst* RangeWeapon = m_inv[EQEmu::legacy::SlotRange];

	if (!RangeWeapon || !RangeWeapon->IsType(ItemClassCommon)) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack canceled. Missing or invalid ranged weapon (%d) in slot %d", GetItemIDAt(EQEmu::legacy::SlotRange), EQEmu::legacy::SlotRange);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing to throw!", GetItemIDAt(EQEmu::legacy::SlotRange));
		return;
	}

	const Item_Struct* item = RangeWeapon->GetItem();
	if(item->ItemType != ItemTypeLargeThrowing && item->ItemType != ItemTypeSmallThrowing) {
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack canceled. Ranged item %d is not a throwing weapon. type %d.", item->ItemType);
		Message(0, "Error: Rangeweapon: GetItem(%i)==0, you have nothing useful to throw!", GetItemIDAt(EQEmu::legacy::SlotRange));
		return;
	}

	Log.Out(Logs::Detail, Logs::Combat, "Throwing %s (%d) at %s", item->Name, item->ID, other->GetName());

	if(RangeWeapon->GetCharges() == 1) {
		//first check ammo
		const ItemInst* AmmoItem = m_inv[EQEmu::legacy::SlotAmmo];
		if(AmmoItem != nullptr && AmmoItem->GetID() == RangeWeapon->GetID()) {
			//more in the ammo slot, use it
			RangeWeapon = AmmoItem;
			ammo_slot = EQEmu::legacy::SlotAmmo;
			Log.Out(Logs::Detail, Logs::Combat, "Using ammo from ammo slot, stack at slot %d. %d in stack.", ammo_slot, RangeWeapon->GetCharges());
		} else {
			//look through our inventory for more
			int32 aslot = m_inv.HasItem(item->ID, 1, invWherePersonal);
			if (aslot != INVALID_INDEX) {
				//the item wont change, but the instance does, not that it matters
				ammo_slot = aslot;
				RangeWeapon = m_inv[aslot];
				Log.Out(Logs::Detail, Logs::Combat, "Using ammo from inventory slot, stack at slot %d. %d in stack.", ammo_slot, RangeWeapon->GetCharges());
			}
		}
	}

	float range = item->Range + GetRangeDistTargetSizeMod(other);
	Log.Out(Logs::Detail, Logs::Combat, "Calculated bow range to be %.1f", range);
	range *= range;
	float dist = DistanceSquared(m_Position, other->GetPosition());
	if(dist > range) {
		Log.Out(Logs::Detail, Logs::Combat, "Throwing attack out of range... client should catch this. (%f > %f).\n", dist, range);
		Message_StringID(13,TARGET_OUT_OF_RANGE);//Client enforces range and sends the message, this is a backup just incase.
		return;
	}
	else if(dist < (RuleI(Combat, MinRangedAttackDist)*RuleI(Combat, MinRangedAttackDist))){
		Message_StringID(15,RANGED_TOO_CLOSE);//Client enforces range and sends the message, this is a backup just incase.
	}

	if(!IsAttackAllowed(other) ||
		IsCasting() ||
		IsSitting() ||
		(DivineAura() && !GetGM()) ||
		IsStunned() ||
		IsFeared() ||
		IsMezzed() ||
		(GetAppearance() == eaDead)){
		return;
	}

	DoThrowingAttackDmg(other, RangeWeapon, item);

	//consume ammo
	DeleteItemInInventory(ammo_slot, 1, true);
	CheckIncreaseSkill(SkillThrowing, GetTarget());
	CommonBreakInvisibleFromCombat();
}

void Mob::DoThrowingAttackDmg(Mob* other, const ItemInst* RangeWeapon, const Item_Struct* AmmoItem, uint16 weapon_damage, int16 chance_mod,int16 focus, int ReuseTime, uint32 range_id, int AmmoSlot, float speed)
{
	if ((other == nullptr ||
		((IsClient() && CastToClient()->dead) ||
		(other->IsClient() && other->CastToClient()->dead)) ||
		HasDied() ||
		(!IsAttackAllowed(other)) ||
		(other->GetInvul() ||
		other->GetSpecialAbility(IMMUNE_MELEE))))
	{
		return;
	}

	const ItemInst* _RangeWeapon = nullptr;
	const Item_Struct* ammo_lost = nullptr;

	/*
	If LaunchProjectile is false this function will do archery damage on target,
	otherwise it will shoot the projectile at the target, once the projectile hits target
	this function is then run again to do the damage portion
	*/
	bool LaunchProjectile = false;
	bool ProjectileImpact = false;
	bool ProjectileMiss = false;

	if (RuleB(Combat, ProjectileDmgOnImpact)){

		if (AmmoItem)
			LaunchProjectile = true;
		else{
			if (!RangeWeapon && range_id){

				ProjectileImpact = true;

				if (weapon_damage == 0)
					ProjectileMiss = true; //This indicates that MISS was originally calculated.

				if (IsClient()){

					_RangeWeapon = CastToClient()->m_inv[AmmoSlot];
					if (_RangeWeapon && _RangeWeapon->GetItem() && _RangeWeapon->GetItem()->ID != range_id)
						RangeWeapon = _RangeWeapon;
					else
						ammo_lost = database.GetItem(range_id);
				}
			}
		}
	}
	else if (AmmoItem)
		SendItemAnimation(other, AmmoItem, SkillThrowing);

	if (ProjectileMiss || (!ProjectileImpact && !other->CheckHitChance(this, SkillThrowing, EQEmu::legacy::SlotPrimary, chance_mod))){
		Log.Out(Logs::Detail, Logs::Combat, "Ranged attack missed %s.", other->GetName());
		if (LaunchProjectile){
			TryProjectileAttack(other, AmmoItem, SkillThrowing, 0, RangeWeapon, nullptr, AmmoSlot, speed);
			return;
		}
		else
			other->Damage(this, 0, SPELL_UNKNOWN, SkillThrowing);
	} else {
		Log.Out(Logs::Detail, Logs::Combat, "Throwing attack hit %s.", other->GetName());

		int16 WDmg = 0;

		if (!weapon_damage){
			if (IsClient() && RangeWeapon)
				WDmg = GetWeaponDamage(other, RangeWeapon);
			else if (AmmoItem)
				WDmg = GetWeaponDamage(other, AmmoItem);

			if (LaunchProjectile){
				TryProjectileAttack(other, AmmoItem, SkillThrowing, WDmg, RangeWeapon, nullptr, AmmoSlot, speed);
				return;
			}
		}
		else
			WDmg = weapon_damage;

		if (focus) //From FcBaseEffects
			WDmg += WDmg*focus/100;

		int32 TotalDmg = 0;

		uint32 Assassinate_Dmg = 0;
		if (GetClass() == ROGUE && (BehindMob(other, GetX(), GetY())))
			Assassinate_Dmg = TryAssassinate(other, SkillThrowing, ranged_timer.GetDuration());

		if(WDmg > 0){
			int minDmg = 1;
			uint16 MaxDmg = GetThrownDamage(WDmg, TotalDmg, minDmg);

			if (Assassinate_Dmg) {
				TotalDmg = Assassinate_Dmg;
				entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, ASSASSINATES, GetName());
			}

			Log.Out(Logs::Detail, Logs::Combat, "Item DMG %d. Max Damage %d. Hit for damage %d", WDmg, MaxDmg, TotalDmg);
			if (!Assassinate_Dmg)
				other->AvoidDamage(this, TotalDmg, EQEmu::legacy::SlotRange);

			other->MeleeMitigation(this, TotalDmg, minDmg);
			if(TotalDmg > 0)
				CommonOutgoingHitSuccess(other, TotalDmg,  SkillThrowing);
		}

		else
			TotalDmg = -5;

		if (IsClient() && !CastToClient()->GetFeigned())
			other->AddToHateList(this, 2*WDmg, 0, false);

		other->Damage(this, TotalDmg, SPELL_UNKNOWN, SkillThrowing);

		if (TotalDmg > 0 && HasSkillProcSuccess() && other && !other->HasDied()){
			if (ReuseTime)
				TrySkillProc(other, SkillThrowing, ReuseTime);
			else
				TrySkillProc(other, SkillThrowing, 0, true, EQEmu::legacy::SlotRange);
		}
	}

	if (LaunchProjectile)
		return;

	//Throwing item Proc
	if (ammo_lost)
		TryWeaponProc(nullptr, ammo_lost, other, EQEmu::legacy::SlotRange);
	else if(RangeWeapon && other && !other->HasDied())
		TryWeaponProc(RangeWeapon, other, EQEmu::legacy::SlotRange);

	if (HasSkillProcs() && other && !other->HasDied()){
		if (ReuseTime)
			TrySkillProc(other, SkillThrowing, ReuseTime);
		else
			TrySkillProc(other, SkillThrowing, 0, false, EQEmu::legacy::SlotRange);
	}
}

void Mob::SendItemAnimation(Mob *to, const Item_Struct *item, SkillUseTypes skillInUse, float velocity) {
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
	as->velocity = velocity;

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

void Mob::ProjectileAnimation(Mob* to, int item_id, bool IsArrow, float speed, float angle, float tilt, float arc, const char *IDFile, SkillUseTypes skillInUse) {
	if (!to)
		return;

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
	if(!item_type && !skillInUse) {
		item_type = item->ItemType;
	}
	else if (skillInUse)
		item_type = GetItemTypeBySkill(skillInUse);

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
	as->skill = skillInUse;	// Doesn't seem to have any effect
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

	float HasteModifier = GetHaste() * 0.01f;

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
			if(level > 29) { satype = SkillFlyingKick; }
			else if(level > 24) { satype = SkillDragonPunch; }
			else if(level > 19) { satype = SkillEagleStrike; }
			else if(level > 9) { satype = SkillTigerClaw; }
			else if(level > 4) { satype = SkillRoundKick; }
			reuse = MonkSpecialAttack(target, satype);

			reuse *= 1000;
			did_attack = true;
			break;
		}
		case WARRIOR: case WARRIORGM:{
			if(level >= RuleI(Combat, NPCBashKickLevel)){
				if(zone->random.Roll(75)) { //tested on live, warrior mobs both kick and bash, kick about 75% of the time, casting doesn't seem to make a difference.
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
								dmg = zone->random.Int(1, GetKickDamage());

						}
					}

					reuse = (KickReuseTime + 3) * 1000;
					DoSpecialAttackDamage(target, SkillKick, dmg, 1, -1, reuse);
					did_attack = true;
				}
				else {
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
								dmg = zone->random.Int(1, GetBashDamage());
						}
					}

					reuse = (BashReuseTime + 3) * 1000;
					DoSpecialAttackDamage(target, SkillBash, dmg, 1, -1, reuse);
					did_attack = true;
				}
			}
			break;
		}
		case BERSERKER: case BERSERKERGM:{
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
				if (GetTarget() && (AtkRounds == 1 || zone->random.Roll(75))) {
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
							dmg = zone->random.Int(1, GetKickDamage());
					}
				}

				reuse = (KickReuseTime + 3) * 1000;
				DoSpecialAttackDamage(target, SkillKick, dmg, 1, -1, reuse);
				did_attack = true;
			}
			break;
		}
		case CLERIC: case CLERICGM: //clerics can bash too.
		case SHADOWKNIGHT: case SHADOWKNIGHTGM:
		case PALADIN: case PALADINGM:{
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
							dmg = zone->random.Int(1, GetBashDamage());
					}
				}

				reuse = (BashReuseTime + 3) * 1000;
				DoSpecialAttackDamage(target, SkillBash, dmg, 1, -1, reuse);
				did_attack = true;
			}
			break;
		}
	}

	classattack_timer.Start(reuse / HasteModifier);
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
	if(!CombatRange(ca_target)){
		return;
	}

	if(!IsRiposte && (!p_timers.Expired(&database, pTimerCombatAbility, false))) {
		return;
	}

	int ReuseTime = 0;
	float HasteMod = GetHaste() * 0.01f;

	int32 dmg = 0;

	uint16 skill_to_use = -1;

	if (skill == -1){
		switch(GetClass()){
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

	if(skill_to_use == SkillBash) {
		if (ca_target!=this) {
			DoAnim(animTailRake);

			if (GetWeaponDamage(ca_target, GetInv().GetItem(EQEmu::legacy::SlotSecondary)) <= 0 && GetWeaponDamage(ca_target, GetInv().GetItem(EQEmu::legacy::SlotShoulders)) <= 0){
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
						dmg = zone->random.Int(1, GetBashDamage());

				}
			}

			ReuseTime = (BashReuseTime - 1) / HasteMod;

			DoSpecialAttackDamage(ca_target, SkillBash, dmg, 1,-1,ReuseTime);

			if(ReuseTime > 0 && !IsRiposte) {
				p_timers.Start(pTimerCombatAbility, ReuseTime);
			}
		}
		return;
	}

	if(skill_to_use == SkillFrenzy){
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

		ReuseTime = (FrenzyReuseTime - 1) / HasteMod;

		//Live parses show around 55% Triple 35% Double 10% Single, you will always get first hit.
		while(AtkRounds > 0) {

			if (GetTarget() && (AtkRounds == 1 || zone->random.Roll(75))) {
				DoSpecialAttackDamage(GetTarget(), SkillFrenzy, max_dmg, min_dmg, max_dmg , ReuseTime, true);
			}
			AtkRounds--;
		}

		if(ReuseTime > 0 && !IsRiposte) {
			p_timers.Start(pTimerCombatAbility, ReuseTime);
		}
		return;
	}

	if(skill_to_use == SkillKick){
		if(ca_target!=this){
			DoAnim(animKick);

			if (GetWeaponDamage(ca_target, GetInv().GetItem(EQEmu::legacy::SlotFeet)) <= 0){
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
						dmg = zone->random.Int(1, GetKickDamage());
				}
			}

			ReuseTime = KickReuseTime-1;

			DoSpecialAttackDamage(ca_target, SkillKick, dmg, 1,-1, ReuseTime);
		}
	}

	if(skill_to_use == SkillFlyingKick || skill_to_use == SkillDragonPunch || skill_to_use == SkillEagleStrike || skill_to_use == SkillTigerClaw || skill_to_use == SkillRoundKick) {
		ReuseTime = MonkSpecialAttack(ca_target, skill_to_use) - 1;
		MonkSpecialAttack(ca_target, skill_to_use);

		if (IsRiposte)
			return;

		//Live AA - Technique of Master Wu
		int wuchance = itembonuses.DoubleSpecialAttack + spellbonuses.DoubleSpecialAttack + aabonuses.DoubleSpecialAttack;
		if (wuchance) {
			if (wuchance >= 100 || zone->random.Roll(wuchance)) {
				int MonkSPA [5] = { SkillFlyingKick, SkillDragonPunch, SkillEagleStrike, SkillTigerClaw, SkillRoundKick };
				int extra = 1;
				if (zone->random.Roll(wuchance / 4))
					extra++;
				// They didn't add a string ID for this.
				std::string msg = StringFormat("The spirit of Master Wu fills you!  You gain %d additional attack(s).", extra);
				// live uses 400 here -- not sure if it's the best for all clients though
				SendColoredText(400, msg);
				while (extra) {
					MonkSpecialAttack(ca_target, MonkSPA[zone->random.Int(0, 4)]);
					extra--;
				}
			}
		}
	}

	if(skill_to_use == SkillBackstab){
		ReuseTime = BackstabReuseTime-1;

		if (IsRiposte)
			ReuseTime=0;

		TryBackstab(ca_target,ReuseTime);
	}

	ReuseTime = ReuseTime / HasteMod;
	if(ReuseTime > 0 && !IsRiposte){
		p_timers.Start(pTimerCombatAbility, ReuseTime);
	}
}

void Mob::Taunt(NPC* who, bool always_succeed, float chance_bonus, bool FromSpell, int32 bonus_hate) {

	if (who == nullptr)
		return;

	if(DivineAura())
		return;

	if(!FromSpell && !CombatRange(who))
		return;

	if(!always_succeed && IsClient())
		CastToClient()->CheckIncreaseSkill(SkillTaunt, who, 10);

	Mob *hate_top = who->GetHateMost();

	int level_difference = GetLevel() - who->GetLevel();
	bool Success = false;

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
				tauntchance += static_cast<float>(level_difference)*3.0f;
				if (tauntchance < 20)
					tauntchance = 20.0f;
			}

			else {
				tauntchance += static_cast<float>(level_difference)*5.0f;
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

		if (tauntchance > zone->random.Real(0, 1)) {
			if (hate_top && hate_top != this){
				newhate = (who->GetNPCHate(hate_top) - who->GetNPCHate(this)) + 1 + bonus_hate;
				who->CastToNPC()->AddToHateList(this, newhate);
				Success = true;
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

	if (HasSkillProcs())
		TrySkillProc(who, SkillTaunt, TauntReuseTime*1000);

	if (Success && HasSkillProcSuccess())
		TrySkillProc(who, SkillTaunt, TauntReuseTime*1000, true);
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

	if(IsClient()) {
		CastToClient()->CheckIncreaseSkill(SkillIntimidation, who, 10);
	}

	//I think this formula needs work
	int value = 0;

	//user's bonus
	value += GetSkill(SkillIntimidation) + GetCHA()/4;

	//target's counters
	value -= target->GetLevel()*4 + who->GetWIS()/4;

	if (zone->random.Roll(value)) {
		//temporary hack...
		//cast fear on them... should prolly be a different spell
		//and should be un-resistable.
		SpellOnTarget(229, who, false, true, -2000);
		//is there a success message?
	} else {
		Message_StringID(4,NOT_SCARING);
		//Idea from WR:
		/* if (target->IsNPC() && zone->random.Int(0,99) < 10 ) {
			entity_list.MessageClose(target, false, 50, MT_NPCRampage, "%s lashes out in anger!",target->GetName());
			//should we actually do this? and the range is completely made up, unconfirmed
			entity_list.AEAttack(target, 50);
		}*/
	}
}

uint32 Mob::TryHeadShot(Mob* defender, SkillUseTypes skillInUse) {
	//Only works on YOUR target.
	if(defender && (defender->GetBodyType() == BT_Humanoid) && !defender->IsClient()
		&& (skillInUse == SkillArchery) && (GetTarget() == defender)) {

		uint32 HeadShot_Dmg = aabonuses.HeadShot[1] + spellbonuses.HeadShot[1] + itembonuses.HeadShot[1];
		uint8 HeadShot_Level = 0; //Get Highest Headshot Level
		HeadShot_Level = aabonuses.HSLevel;
		if (HeadShot_Level < spellbonuses.HSLevel)
			HeadShot_Level = spellbonuses.HSLevel;
		else if (HeadShot_Level < itembonuses.HSLevel)
			HeadShot_Level = itembonuses.HSLevel;

		if(HeadShot_Dmg && HeadShot_Level && (defender->GetLevel() <= HeadShot_Level)){
			float ProcChance = GetSpecialProcChances(EQEmu::legacy::SlotRange);
			if(zone->random.Roll(ProcChance))
				return HeadShot_Dmg;
		}
	}

	return 0;
}

float Mob::GetSpecialProcChances(uint16 hand)
{
	int mydex = GetDEX();

	if (mydex > 255)
		mydex = 255;

	uint16 weapon_speed;
	float ProcChance = 0.0f;
	float ProcBonus = 0.0f;

	weapon_speed = GetWeaponSpeedbyHand(hand);

	if (RuleB(Combat, AdjustSpecialProcPerMinute)) {
		ProcChance = (static_cast<float>(weapon_speed) *
				RuleR(Combat, AvgSpecialProcsPerMinute) / 60000.0f);
		ProcBonus +=  static_cast<float>(mydex/35) + static_cast<float>(itembonuses.HeroicDEX / 25);
		ProcChance += ProcChance * ProcBonus / 100.0f;
	} else {
		/*PRE 2014 CHANGE Dev Quote - "Elidroth SOE:Proc chance is a function of your base hardcapped Dexterity / 35 + Heroic Dexterity / 25.
		Kayen: Most reports suggest a ~ 6% chance to Headshot which consistent with above.*/

		ProcChance = (static_cast<float>(mydex/35) + static_cast<float>(itembonuses.HeroicDEX / 25))/100.0f;
	}

	return ProcChance;
}

uint32 Mob::TryAssassinate(Mob* defender, SkillUseTypes skillInUse, uint16 ReuseTime) {

	if(defender && (defender->GetBodyType() == BT_Humanoid) && !defender->IsClient() &&
		(skillInUse == SkillBackstab || skillInUse == SkillThrowing)) {

		uint32 Assassinate_Dmg = aabonuses.Assassinate[1] + spellbonuses.Assassinate[1] + itembonuses.Assassinate[1];

		uint8 Assassinate_Level = 0; //Get Highest Headshot Level
		Assassinate_Level = aabonuses.AssassinateLevel;
		if (Assassinate_Level < spellbonuses.AssassinateLevel)
			Assassinate_Level = spellbonuses.AssassinateLevel;
		else if (Assassinate_Level < itembonuses.AssassinateLevel)
			Assassinate_Level = itembonuses.AssassinateLevel;

		if (GetLevel() >= 60){ //Innate Assassinate Ability if client as no bonuses.
			if (!Assassinate_Level)
				Assassinate_Level = 45;

			if (!Assassinate_Dmg)
				Assassinate_Dmg = 32000;
		}

		if(Assassinate_Dmg && Assassinate_Level && (defender->GetLevel() <= Assassinate_Level)){
			float ProcChance = 0.0f;

			if (skillInUse == SkillThrowing)
				ProcChance = GetSpecialProcChances(EQEmu::legacy::SlotRange);
			else
				ProcChance = GetAssassinateProcChances(ReuseTime);

			if(zone->random.Roll(ProcChance))
				return Assassinate_Dmg;
		}
	}

	return 0;
}

float Mob::GetAssassinateProcChances(uint16 ReuseTime)
{
	int mydex = GetDEX();

	if (mydex > 255)
		mydex = 255;

	float ProcChance = 0.0f;
	float ProcBonus = 0.0f;

	if (RuleB(Combat, AdjustSpecialProcPerMinute)) {
		ProcChance = (static_cast<float>(ReuseTime*1000) *
				RuleR(Combat, AvgSpecialProcsPerMinute) / 60000.0f);
		ProcBonus += (10 + (static_cast<float>(mydex/10) + static_cast<float>(itembonuses.HeroicDEX /10)))/100.0f;
		ProcChance += ProcChance * ProcBonus / 100.0f;

	} else {
		/* Kayen: Unable to find data on old proc rate of assassinate, no idea if our formula is real or made up. */
		ProcChance = (10 + (static_cast<float>(mydex/10) + static_cast<float>(itembonuses.HeroicDEX /10)))/100.0f;

	}

	return ProcChance;
}

void Mob::DoMeleeSkillAttackDmg(Mob* other, uint16 weapon_damage, SkillUseTypes skillinuse, int16 chance_mod, int16 focus, bool CanRiposte, int ReuseTime)
{
	if (!CanDoSpecialAttack(other))
		return;

	/*
		For spells using skill value 98 (feral swipe ect) server sets this to 67 automatically.
		Kayen: This is unlikely to be completely accurate but use OFFENSE skill value for these effects.
	*/
	if (skillinuse == SkillBegging)
		skillinuse = SkillOffense;

	int damage = 0;
	uint32 hate = 0;
	int Hand = EQEmu::legacy::SlotPrimary;
	if (hate == 0 && weapon_damage > 1) hate = weapon_damage;

	if(weapon_damage > 0){
		if (focus) //From FcBaseEffects
			weapon_damage += weapon_damage*focus/100;

		if(GetClass() == BERSERKER){
			int bonus = 3 + GetLevel()/10;
			weapon_damage = weapon_damage * (100+bonus) / 100;
		}

		int32 min_hit = 1;
		int32 max_hit = (2 * weapon_damage*GetDamageTable(skillinuse)) / 100;

		if(GetLevel() >= 28 && IsWarriorClass() ) {
			int ucDamageBonus = GetWeaponDamageBonus((const Item_Struct*) nullptr );
			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}

		if(skillinuse == SkillBash){
			if(IsClient()){
				ItemInst *item = CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotSecondary);
				if(item){
					if(item->GetItem()->ItemType == ItemTypeShield)	{
						hate += item->GetItem()->AC;
					}
					const Item_Struct *itm = item->GetItem();
					hate = hate * (100 + GetFuriousBash(itm->Focus.Effect)) / 100;
				}
			}
		}

		ApplySpecialAttackMod(skillinuse, max_hit, min_hit);
		min_hit += min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100;

		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = zone->random.Int(min_hit, max_hit);

		if (other->AvoidDamage(this, damage, CanRiposte ? EQEmu::legacy::SlotRange : EQEmu::legacy::SlotPrimary)) { // SlotRange excludes ripo, primary doesn't have any extra behavior
			if (damage == -3) {
				DoRiposte(other);
				if (HasDied())
					return;
			}
		} else {
			if (other->CheckHitChance(this, skillinuse, Hand, chance_mod)) {
				other->MeleeMitigation(this, damage, min_hit);
				CommonOutgoingHitSuccess(other, damage, skillinuse);
			} else {
				damage = 0;
			}
		}

	}

	else
		damage = -5;

	bool CanSkillProc = true;
	if (skillinuse == SkillOffense){ //Hack to allow damage to display.
		skillinuse = SkillTigerClaw; //'strike' your opponent - Arbitrary choice for message.
		CanSkillProc = false; //Disable skill procs
	}

	other->AddToHateList(this, hate, 0, false);
	if (damage > 0 && aabonuses.SkillAttackProc[0] && aabonuses.SkillAttackProc[1] == skillinuse &&
	    IsValidSpell(aabonuses.SkillAttackProc[2])) {
		float chance = aabonuses.SkillAttackProc[0] / 1000.0f;
		if (zone->random.Roll(chance))
			SpellFinished(aabonuses.SkillAttackProc[2], other, 10, 0, -1,
				      spells[aabonuses.SkillAttackProc[2]].ResistDiff);
	}
	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);

	if (HasDied())
		return;

	if (CanSkillProc && HasSkillProcs())
		TrySkillProc(other, skillinuse, ReuseTime);

	if (CanSkillProc && (damage > 0) && HasSkillProcSuccess())
		TrySkillProc(other, skillinuse, ReuseTime, true);
}

bool Mob::CanDoSpecialAttack(Mob *other) {
	//Make sure everything is valid before doing any attacks.
	if (!other) {
		SetTarget(nullptr);
		return false;
	}

	if(!GetTarget())
		SetTarget(other);

	if ((other == nullptr || ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) || HasDied() || (!IsAttackAllowed(other)))) {
		return false;
	}

	if(other->GetInvul() || other->GetSpecialAbility(IMMUNE_MELEE))
		return false;

	return true;
}
