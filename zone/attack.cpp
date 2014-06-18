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

#if EQDEBUG >= 5
#endif

#include "../common/debug.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <assert.h>

#include "masterentity.h"
#include "NpcAI.h"
#include "../common/packet_dump.h"
#include "../common/eq_packet_structs.h"
#include "../common/eq_constants.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "zone.h"
#include "StringIDs.h"
#include "../common/StringUtil.h"
#include "../common/rulesys.h"
#include "QuestParserCollection.h"
#include "water_map.h"
#include "worldserver.h"
extern WorldServer worldserver;

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern EntityList entity_list;
extern Zone* zone;

bool Mob::AttackAnimation(SkillUseTypes &skillinuse, int Hand, const ItemInst* weapon) {
	int type = 0;
	if (weapon && weapon->IsType(ItemClassCommon)) {
		const Item_Struct* item = weapon->GetItem();
#if EQDEBUG >= 11
			LogFile->write(EQEMuLog::Debug, "Weapon skill:%i", item->ItemType);
#endif
		switch (item->ItemType) {
			case ItemType1HSlash: {
				skillinuse = Skill1HSlashing;
				type = anim1HWeapon;
				break;
			}
			case ItemType2HSlash: {
				skillinuse = Skill2HSlashing;
				type = anim2HSlashing;
				break;
			}
			case ItemType1HPiercing: {
				skillinuse = Skill1HPiercing;
				type = animPiercing;
				break;
			}
			case ItemType1HBlunt: {
				skillinuse = Skill1HBlunt;
				type = anim1HWeapon;
				break;
			}
			case ItemType2HBlunt: {
				skillinuse = Skill2HBlunt;
				type = anim2HSlashing;
				break;
			}
			case ItemType2HPiercing: {
				skillinuse = Skill1HPiercing;
				type = anim2HWeapon;
				break;
			}
			case ItemTypeMartial: {
				skillinuse = SkillHandtoHand;
				type = animHand2Hand;
				break;
			}
			default: {
				skillinuse = SkillHandtoHand;
				type = animHand2Hand;
				break;
			}
		}
	}
	else if(IsNPC()) {
		switch (skillinuse) {
			case Skill1HSlashing: {
				type = anim1HWeapon;
				break;
			}
			case Skill2HSlashing: {
				type = anim2HSlashing;
				break;
			}
			case Skill1HPiercing: {
				type = animPiercing;
				break;
			}
			case Skill1HBlunt: {
				type = anim1HWeapon;
				break;
			}
			case Skill2HBlunt: {
				type = anim2HSlashing;
				break;
			}
			case 99: {
				type = anim2HWeapon;
				break;
			}
			case SkillHandtoHand: {
				type = animHand2Hand;
				break;
			}
			default: {
				type = animHand2Hand;
				break;
			}
		}
	}
	else {
		skillinuse = SkillHandtoHand;
		type = animHand2Hand;
	}

	if (Hand == 14)
		type = animDualWield;

	DoAnim(type);
	return true;
}

bool Mob::CheckHitChance(Mob* other, SkillUseTypes skillinuse, int Hand, int16 chance_mod) {
	Mob *attacker=other;
	Mob *defender=this;
	float chancetohit = RuleR(Combat, BaseHitChance);

	if(attacker->IsNPC() && !attacker->IsPet())
		chancetohit += RuleR(Combat, NPCBonusHitChance);

#if ATTACK_DEBUG>=11
		LogFile->write(EQEMuLog::Debug, "CheckHitChance(%s) attacked by %s", defender->GetName(), attacker->GetName());
#endif
	mlog(COMBAT__TOHIT,"CheckHitChance(%s) attacked by %s", defender->GetName(), attacker->GetName());

	bool pvpmode = false;
	if(IsClient() && other->IsClient())
		pvpmode = true;

	if (chance_mod >= 10000)
	    return true;

	float bonus;
	uint8 attacker_level = attacker->GetLevel() ? attacker->GetLevel() : 1;
	uint8 defender_level = defender->GetLevel() ? defender->GetLevel() : 1;

	mlog(COMBAT__TOHIT, "Chance to hit before level diff calc %.2f", chancetohit);
	double level_difference = attacker_level - defender_level;
	double range = defender->GetLevel();
	range = ((range / 4) + 3);

	if(level_difference < 0) {
		if(level_difference >= -range)
			chancetohit += ((level_difference / range) * RuleR(Combat,HitFalloffMinor));
		else if (level_difference >= -(range + 3.0)) {
			chancetohit -= RuleR(Combat,HitFalloffMinor);
			chancetohit += ((level_difference+range) / (3.0)) * RuleR(Combat,HitFalloffModerate);
		}
		else {
			chancetohit -= (RuleR(Combat,HitFalloffMinor) + RuleR(Combat,HitFalloffModerate));
			chancetohit += ((level_difference + range + 3.0) / 12.0) * RuleR(Combat,HitFalloffMajor);
		}
	}
	else
		chancetohit += (RuleR(Combat,HitBonusPerLevel) * level_difference);

	mlog(COMBAT__TOHIT, "Chance to hit after level diff calc %.2f", chancetohit);
	chancetohit -= ((float)defender->GetAGI() * RuleR(Combat, AgiHitFactor));
	mlog(COMBAT__TOHIT, "Chance to hit after agil calc %.2f", chancetohit);

	if(attacker->IsClient()) {
		chancetohit -= (RuleR(Combat,WeaponSkillFalloff) * (attacker->CastToClient()->MaxSkill(skillinuse) - attacker->GetSkill(skillinuse)));
		mlog(COMBAT__TOHIT, "Chance to hit after weapon falloff calc (attack) %.2f", chancetohit);
	}

	if(defender->IsClient()) {
		chancetohit += (RuleR(Combat,WeaponSkillFalloff) * (defender->CastToClient()->MaxSkill(SkillDefense) - defender->GetSkill(SkillDefense)));
		mlog(COMBAT__TOHIT, "Chance to hit after weapon falloff calc (defense) %.2f", chancetohit);
	}

	if(attacker->spellbonuses.MeleeSkillCheckSkill == skillinuse || attacker->spellbonuses.MeleeSkillCheckSkill == 255) {
		chancetohit += attacker->spellbonuses.MeleeSkillCheck;
		mlog(COMBAT__TOHIT, "Applied spell melee skill bonus %d, yeilding %.2f", attacker->spellbonuses.MeleeSkillCheck, chancetohit);
	}
	if(attacker->itembonuses.MeleeSkillCheckSkill == skillinuse || attacker->itembonuses.MeleeSkillCheckSkill == 255) {
		chancetohit += attacker->itembonuses.MeleeSkillCheck;
		mlog(COMBAT__TOHIT, "Applied item melee skill bonus %d, yeilding %.2f", attacker->spellbonuses.MeleeSkillCheck, chancetohit);
	}

	bonus = (defender->spellbonuses.AvoidMeleeChance + defender->itembonuses.AvoidMeleeChance + (defender->aabonuses.AvoidMeleeChance * 10));

	if (IsPet()) {
		Mob *owner = defender->GetOwner();
		if (!owner)
			return false;
		bonus += ((owner->aabonuses.PetAvoidance + owner->spellbonuses.PetAvoidance + owner->itembonuses.PetAvoidance) * 10);
	}

	if(bonus > 0) {
		chancetohit -= ((bonus * chancetohit) / 1000);
		mlog(COMBAT__TOHIT, "Applied avoidance chance %.2f/10, yeilding %.2f", bonus, chancetohit);
	}

	if(attacker->IsNPC())
		chancetohit += (chancetohit * attacker->CastToNPC()->GetAccuracyRating() / 1000);

	mlog(COMBAT__TOHIT, "Chance to hit after accuracy rating calc %.2f", chancetohit);

	float hitBonus = 0;
	hitBonus +=	(attacker->itembonuses.HitChanceEffect[skillinuse] + attacker->spellbonuses.HitChanceEffect[skillinuse] + attacker->itembonuses.HitChanceEffect[HIGHEST_SKILL + 1] + attacker->spellbonuses.HitChanceEffect[HIGHEST_SKILL + 1]);
	hitBonus += ((attacker->itembonuses.Accuracy[HIGHEST_SKILL + 1] + attacker->spellbonuses.Accuracy[HIGHEST_SKILL+ 1] + attacker->aabonuses.Accuracy[HIGHEST_SKILL + 1] + attacker->aabonuses.Accuracy[skillinuse] + attacker->itembonuses.HitChance) / 15.0f);
	hitBonus += chance_mod;
	chancetohit += ((chancetohit * hitBonus) / 100.0f);

	if(skillinuse == SkillArchery)
		chancetohit -= ((chancetohit * RuleR(Combat, ArcheryHitPenalty)) / 100.0f);

	chancetohit = mod_hit_chance(chancetohit, skillinuse, attacker);
	if(chancetohit > 95 && chancetohit < 1000)
		chancetohit = 95;
	else if(chancetohit < 5 && chancetohit > -1000)
		chancetohit = 5;
	
	#if EQDEBUG>=11
		LogFile->write(EQEMuLog::Debug, "3 FINAL calculated chance to hit is: %5.2f", chancetohit);
	#endif

	float tohit_roll = MakeRandomFloat(0, 100);
	mlog(COMBAT__TOHIT, "Final hit chance: %.2f%%. Hit roll %.2f", chancetohit, tohit_roll);
	return(tohit_roll <= chancetohit);
}


bool Mob::AvoidDamage(Mob* other, int32 &damage, bool CanRiposte) {
	float skill;
	float bonus;
	float RollTable[4] = {0,0,0,0};
	float roll;
	Mob *attacker=other;
	Mob *defender=this;
	bool ghit = false;
	if((attacker->spellbonuses.MeleeSkillCheck + attacker->itembonuses.MeleeSkillCheck) > 500)
		ghit = true;
		
	if (IsEnraged() && other->InFrontMob(this, other->GetX(), other->GetY())) {
		damage = -3;
		mlog(COMBAT__DAMAGE, "I am enraged, riposting frontal attack.");
	}

	float riposte_chance = 0.0f;
	if (CanRiposte && damage > 0 && CanThisClassRiposte() && other->InFrontMob(this, other->GetX(), other->GetY()))	{
		riposte_chance = ((100.0f + (float)defender->aabonuses.RiposteChance + (float)defender->spellbonuses.RiposteChance + (float)defender->itembonuses.RiposteChance) / 100.0f);
		skill = GetSkill(SkillRiposte);
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(SkillRiposte, other, -10);

		if (!ghit) {
			bonus = (2.0 + skill / 60.0 + (GetDEX() / 200));
			bonus *= riposte_chance;
			bonus = mod_riposte_chance(bonus, attacker);
			RollTable[0] = (bonus + (itembonuses.HeroicDEX / 25));
		}
	}

	bool bBlockFromRear = false;
	bool bShieldBlockFromRear = false;

	if (this->IsClient()) {
		int aaChance = 0;
		int BlockBehindChance = (aabonuses.BlockBehind + spellbonuses.BlockBehind + itembonuses.BlockBehind);

		if (BlockBehindChance && (BlockBehindChance > MakeRandomInt(1, 100))){
			bBlockFromRear = true;
			if (spellbonuses.BlockBehind || itembonuses.BlockBehind)
				bShieldBlockFromRear = true;
		}
	}

	float block_chance = 0.0f;
	if (damage > 0 && CanThisClassBlock() && (other->InFrontMob(this, other->GetX(), other->GetY()) || bBlockFromRear)) {
		block_chance = ((100.0f + (float)spellbonuses.IncreaseBlockChance + (float)itembonuses.IncreaseBlockChance) / 100.0f);
		skill = CastToClient()->GetSkill(SkillBlock);
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(SkillBlock, other, -10);

		if (!ghit) {
			bonus = (2.0 + skill / 35.0 + (GetDEX() / 200));
			bonus = mod_block_chance(bonus, attacker);
			RollTable[1] = (RollTable[0] + (bonus * block_chance));
		}
	}
	else
		RollTable[1] = RollTable[0];

	if(damage > 0 && HasShieldEquiped()	&& (aabonuses.ShieldBlock || spellbonuses.ShieldBlock || itembonuses.ShieldBlock) && (other->InFrontMob(this, other->GetX(), other->GetY()) || bShieldBlockFromRear)) {
		float bonusShieldBlock = 0.0f;
		bonusShieldBlock = (aabonuses.ShieldBlock + spellbonuses.ShieldBlock + itembonuses.ShieldBlock);
		RollTable[1] += bonusShieldBlock;
	}

	if(damage > 0 && (aabonuses.TwoHandBluntBlock || spellbonuses.TwoHandBluntBlock || itembonuses.TwoHandBluntBlock) && (other->InFrontMob(this, other->GetX(), other->GetY()) || bShieldBlockFromRear)) {
		bool equiped2 = CastToClient()->m_inv.GetItem(13);
		if(equiped2) {
			uint8 TwoHandBlunt = CastToClient()->m_inv.GetItem(13)->GetItem()->ItemType;
			float bonusStaffBlock = 0.0f;
			if(TwoHandBlunt == ItemType2HBlunt) {
				bonusStaffBlock = (aabonuses.TwoHandBluntBlock + spellbonuses.TwoHandBluntBlock + itembonuses.TwoHandBluntBlock);
				RollTable[1] += bonusStaffBlock;
			}
		}
	}
	
	float parry_chance = 0.0f;
	if (damage > 0 && CanThisClassParry() && other->InFrontMob(this, other->GetX(), other->GetY())) {
		parry_chance = ((100.0f + (float)defender->spellbonuses.ParryChance + (float)defender->itembonuses.ParryChance) / 100.0f);
		skill = CastToClient()->GetSkill(SkillParry);
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(SkillParry, other, -10);

		if (!ghit) {
			bonus = (2.0 + skill / 60.0 + (GetDEX() / 200));
			bonus *= parry_chance;
			bonus = mod_parry_chance(bonus, attacker);
			RollTable[2] = (RollTable[1] + bonus);
		}
	}
	else
		RollTable[2] = RollTable[1];

	float dodge_chance = 0.0f;
	if (damage > 0 && CanThisClassDodge() && other->InFrontMob(this, other->GetX(), other->GetY())) {
		dodge_chance = ((100.0f + (float)defender->spellbonuses.DodgeChance + (float)defender->itembonuses.DodgeChance) / 100.0f);
		skill = CastToClient()->GetSkill(SkillDodge);
		if (IsClient())
			CastToClient()->CheckIncreaseSkill(SkillDodge, other, -10);

		if (!ghit) {
			bonus = (2.0 + skill / 60.0 + (GetAGI() / 200));
			bonus *= dodge_chance;
			bonus = mod_dodge_chance(bonus, attacker);
			RollTable[3] = (RollTable[2] + bonus - (itembonuses.HeroicDEX / 25) + (itembonuses.HeroicAGI / 25));
		}
	}
	else
		RollTable[3] = RollTable[2];

	if(damage > 0){
		roll = MakeRandomFloat(0,100);
		if(roll <= RollTable[0])
			damage = -3;
		else if(roll <= RollTable[1])
			damage = -1;
		else if(roll <= RollTable[2])
			damage = -2;
		else if(roll <= RollTable[3])
			damage = -4;
	}

	mlog(COMBAT__DAMAGE, "Final damage after all avoidances: %d", damage);

	if (damage < 0)
		return true;
	return false;
}

void Mob::MeleeMitigation(Mob *attacker, int32 &damage, int32 minhit, ExtraAttackOptions *opts) {
	if (damage <= 0)
		return;

	Mob* defender = this;
	float aa_mit = ((aabonuses.CombatStability + itembonuses.CombatStability + spellbonuses.CombatStability) / 100.0f);

	if (RuleB(Combat, UseIntervalAC)) {
		float softcap = ((GetSkill(SkillDefense) + GetLevel()) *	RuleR(Combat, SoftcapFactor) * (1.0 + aa_mit));
		float mitigation_rating = 0.0;
		float attack_rating = 0.0;
		int shield_ac = 0;
		int armor = 0;
		float weight = 0.0;

		float monkweight = RuleI(Combat, MonkACBonusWeight);
		monkweight = mod_monk_weight(monkweight, attacker);

		if (IsClient()) {
			armor = CastToClient()->GetRawACNoShield(shield_ac);
			weight = (CastToClient()->CalcCurrentWeight() / 10.0);
		}
		else if (IsNPC()) {
			armor = CastToNPC()->GetRawAC();

			if (!IsPet())
				armor = (armor / RuleR(Combat, NPCACFactor));

			armor += (spellbonuses.AC + itembonuses.AC + 1);
		}

		if (opts) {
			armor *= (1.0f - opts->armor_pen_percent);
			armor -= opts->armor_pen_flat;
		}

		if (RuleB(Combat, OldACSoftcapRules)) {
			if (GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == NECROMANCER || GetClass() == ENCHANTER)
				softcap = RuleI(Combat, ClothACSoftcap);
			else if (GetClass() == MONK && weight <= monkweight)
				softcap = RuleI(Combat, MonkACSoftcap);
			else if(GetClass() == DRUID || GetClass() == BEASTLORD || GetClass() == MONK)
				softcap = RuleI(Combat, LeatherACSoftcap);
			else if(GetClass() == SHAMAN || GetClass() == ROGUE || GetClass() == BERSERKER || GetClass() == RANGER)
				softcap = RuleI(Combat, ChainACSoftcap);
			else
				softcap = RuleI(Combat, PlateACSoftcap);
		}

		softcap += shield_ac;
		armor += shield_ac;
		if (RuleB(Combat, OldACSoftcapRules))
			softcap += (softcap * (aa_mit * RuleR(Combat, AAMitigationACFactor)));
		if (armor > softcap) {
			int softcap_armor = armor - softcap;
			if (RuleB(Combat, OldACSoftcapRules)) {
				if (GetClass() == WARRIOR)
					softcap_armor = (softcap_armor * RuleR(Combat, WarriorACSoftcapReturn));
				else if (GetClass() == SHADOWKNIGHT || GetClass() == PALADIN || (GetClass() == MONK && weight <= monkweight))
					softcap_armor = (softcap_armor * RuleR(Combat, KnightACSoftcapReturn));
				else if (GetClass() == CLERIC || GetClass() == BARD || GetClass() == BERSERKER || GetClass() == ROGUE || GetClass() == SHAMAN || GetClass() == MONK)
					softcap_armor = (softcap_armor * RuleR(Combat, LowPlateChainACSoftcapReturn));
				else if (GetClass() == RANGER || GetClass() == BEASTLORD)
					softcap_armor = (softcap_armor * RuleR(Combat, LowChainLeatherACSoftcapReturn));
				else if (GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == NECROMANCER || GetClass() == ENCHANTER || GetClass() == DRUID)
					softcap_armor = (softcap_armor * RuleR(Combat, CasterACSoftcapReturn));
				else
					softcap_armor = (softcap_armor * RuleR(Combat, MiscACSoftcapReturn));
			}
			else {
				if (GetClass() == WARRIOR)
					softcap_armor *= RuleR(Combat, WarACSoftcapReturn);
				else if (GetClass() == PALADIN || GetClass() == SHADOWKNIGHT)
					softcap_armor *= RuleR(Combat, PalShdACSoftcapReturn);
				else if (GetClass() == CLERIC || GetClass() == RANGER || GetClass() == MONK || GetClass() == BARD)
					softcap_armor *= RuleR(Combat, ClrRngMnkBrdACSoftcapReturn);
				else if (GetClass() == DRUID || GetClass() == NECROMANCER || GetClass() == WIZARD || GetClass() == ENCHANTER || GetClass() == MAGICIAN)
					softcap_armor *= RuleR(Combat, DruNecWizEncMagACSoftcapReturn);
				else if (GetClass() == ROGUE || GetClass() == SHAMAN || GetClass() == BEASTLORD || GetClass() == BERSERKER)
					softcap_armor *= RuleR(Combat, RogShmBstBerACSoftcapReturn);
				else
					softcap_armor *= RuleR(Combat, MiscACSoftcapReturn);
			}
			armor = softcap + softcap_armor;
		}

		if (GetClass() == WIZARD || GetClass() == MAGICIAN || GetClass() == NECROMANCER || GetClass() == ENCHANTER)
			mitigation_rating = (((GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) / 4.0) + armor + 1);
		else
			mitigation_rating = (((GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) / 3.0) + (armor * 1.333333) + 1);
		mitigation_rating *= 0.847;

		mitigation_rating = mod_mitigation_rating(mitigation_rating, attacker);

		if (attacker->IsClient())
			attack_rating = (attacker->CastToClient()->CalcATK() + ((attacker->GetSTR() - 66) * 0.9) + (attacker->GetSkill(SkillOffense) * 1.345));
		else
			attack_rating = (attacker->GetATK() + (attacker->GetSkill(SkillOffense) * 1.345) + ((attacker->GetSTR() - 66) * 0.9));

		attack_rating = attacker->mod_attack_rating(attack_rating, this);

		damage = GetMeleeMitDmg(attacker, damage, minhit, mitigation_rating, attack_rating);
	}
	else {
		int myac = GetAC();
		if(opts) {
			myac *= (1.0f - opts->armor_pen_percent);
			myac -= opts->armor_pen_flat;
		}

		if (damage > 0 && myac > 0) {
			int acfail = 100;
			char tmp[10];

			if (database.GetVariable("ACfail", tmp, 9)) {
				acfail = (int) (atof(tmp) * 100);
				if (acfail > 100)
					acfail = 100;
			}

			if (acfail<=0 || MakeRandomInt(0, 100)>acfail) {
				float acreduction = 1;
				int acrandom = 300;
				if (database.GetVariable("ACreduction", tmp, 9)) {
					acreduction=atof(tmp);
					if (acreduction > 100)
						acreduction = 100;
				}

				if (database.GetVariable("ACrandom", tmp, 9)) {
					acrandom = (int) ((atof(tmp) + 1) * 100);
					if (acrandom > 10100)
						acrandom = 10100;
				}

				if (acreduction > 0)
					damage -= (int) (GetAC() * acreduction / 100.0f);
				if (acrandom > 0)
					damage -= (myac * MakeRandomInt(0, acrandom) / 10000);
				if (damage < 1)
					damage = 1;
				mlog(COMBAT__DAMAGE, "AC Damage Reduction: fail chance %d%%. Failed. Reduction %.3f%%, random %d. Resulting damage %d.", acfail, acreduction, acrandom, damage);
			}
			else
				mlog(COMBAT__DAMAGE, "AC Damage Reduction: fail chance %d%%. Did not fail.", acfail);
		}

		damage -= (aa_mit * damage);

		if(damage != 0 && damage < minhit)
			damage = minhit;
			
		damage -= (minhit * defender->itembonuses.MeleeMitigation / 100);
		damage -= (damage * defender->spellbonuses.MeleeMitigation / 100);
	}

	if (damage < 0)
		damage = 0;
}

int32 Mob::GetMeleeMitDmg(Mob *attacker, int32 damage, int32 minhit, float mit_rating, float atk_rating) {
	float d = 10.0;
	float mit_roll = MakeRandomFloat(0, mit_rating);
	float atk_roll = MakeRandomFloat(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = atk_roll - mit_roll;
		float thac0 = atk_rating * RuleR(Combat, ACthac0Factor);
		float thac0cap = (attacker->GetLevel() * 9 + 20);
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d -= (10.0 * (a_diff / thac0));
	}
	else if (mit_roll > atk_roll) {
		float m_diff = mit_roll - atk_roll;
		float thac20 = mit_rating * RuleR(Combat, ACthac20Factor);
		float thac20cap = (GetLevel() * 9 + 20);
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d += (10.0 * (m_diff / thac20));
	}

	if (d < 0.0)
		d = 0.0;
	else if (d > 20.0)
		d = 20.0;

	float interval = ((damage - minhit) / 20.0);
	damage -= ((int)d * interval);
	damage -= (minhit * itembonuses.MeleeMitigation / 100);
	damage -= (damage * spellbonuses.MeleeMitigation / 100);
	return damage;
}

int32 Client::GetMeleeMitDmg(Mob *attacker, int32 damage, int32 minhit, float mit_rating, float atk_rating) {
	if (!attacker->IsNPC() || RuleB(Combat, UseOldDamageIntervalRules))
		return Mob::GetMeleeMitDmg(attacker, damage, minhit, mit_rating, atk_rating);
	int d = 10;
	float dmg_interval = ((damage - minhit) / 19.0);
	float dmg_bonus = (minhit - dmg_interval);
	float spellMeleeMit = (spellbonuses.MeleeMitigation / 100.0);
	if (GetClass() == WARRIOR)
		spellMeleeMit += 0.05;
	dmg_bonus -= (dmg_bonus * (itembonuses.MeleeMitigation / 100.0));
	dmg_interval -= (dmg_interval * spellMeleeMit);

	float mit_roll = MakeRandomFloat(0, mit_rating);
	float atk_roll = MakeRandomFloat(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = (atk_roll - mit_roll);
		float thac0 = (atk_rating * RuleR(Combat, ACthac0Factor));
		float thac0cap = (attacker->GetLevel() * 9 + 20);
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d += (10 * (a_diff / thac0));
	}
	else if (mit_roll > atk_roll) {
		float m_diff = (mit_roll - atk_roll);
		float thac20 = (mit_rating * RuleR(Combat, ACthac20Factor));
		float thac20cap = (GetLevel() * 9 + 20);
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d -= (10 * (m_diff / thac20));
	}

	if (d < 1)
		d = 1;
	else if (d > 20)
		d = 20;

	return static_cast<int32>((dmg_bonus + dmg_interval * d));
}

int Mob::GetWeaponDamage(Mob *against, const Item_Struct *weapon_item) {
	int dmg = 0;
	int banedmg = 0;

	if(against->GetInvul() || against->GetSpecialAbility(IMMUNE_MELEE))
		return 0;

	if(against->GetSpecialAbility(IMMUNE_MELEE_NONMAGICAL)){
		if(weapon_item){
			if(weapon_item->Magic){
				dmg = weapon_item->Damage;
				dmg = dmg <= 0 ? 1 : dmg;
			}
			else
				return 0;
		}
		else {
			if((GetClass() == MONK || GetClass() == BEASTLORD) && GetLevel() >= 30)
				dmg = GetMonkHandToHandDamage();
			else if(GetOwner() && GetLevel() >= RuleI(Combat, PetAttackMagicLevel))
				dmg = 1;
			else if(GetSpecialAbility(SPECATK_MAGICAL))
				dmg = 1;
			else
				return 0;
		}
	}
	else {
		if(weapon_item) {
			dmg = weapon_item->Damage;
			dmg = dmg <= 0 ? 1 : dmg;
		}
		else {
			if(GetClass() == MONK || GetClass() == BEASTLORD)
				dmg = GetMonkHandToHandDamage();
			else
				dmg = 1;
		}
	}

	int eledmg = 0;
	if(!against->GetSpecialAbility(IMMUNE_MAGIC)) {
		if(weapon_item && weapon_item->ElemDmgAmt){
			eledmg = weapon_item->ElemDmgAmt;
			dmg += eledmg;
		}
	}

	if(against->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE)) {
		if(weapon_item){
			if(weapon_item->BaneDmgBody == against->GetBodyType())
				banedmg += weapon_item->BaneDmgAmt;

			if(weapon_item->BaneDmgRace == against->GetRace())
				banedmg += weapon_item->BaneDmgRaceAmt;
		}

		if(!eledmg && !banedmg){
			if(!GetSpecialAbility(SPECATK_BANE))
				return 0;
			else
				return 1;
		}
		else
			dmg += banedmg;
	}
	else {
		if(weapon_item) {
			if(weapon_item->BaneDmgBody == against->GetBodyType())
				banedmg += weapon_item->BaneDmgAmt;

			if(weapon_item->BaneDmgRace == against->GetRace())
				banedmg += weapon_item->BaneDmgRaceAmt;
		}

		dmg += (banedmg + eledmg);
	}

	if(dmg <= 0)
		return 0;
	else
		return dmg;
}

int Mob::GetWeaponDamage(Mob *against, const ItemInst *weapon_item, uint32 *hate) {
	int dmg = 0;
	int banedmg = 0;

	if(!against || against->GetInvul() || against->GetSpecialAbility(IMMUNE_MELEE))
		return 0;
		
	if(weapon_item) {
		const Item_Struct *mWeaponItem = weapon_item->GetItem();
		if(mWeaponItem){
			if(mWeaponItem->ReqLevel > GetLevel())
				return 0;

			if(!weapon_item->IsEquipable(GetBaseRace(), GetClass()))
				return 0;
		}
		else
			return 0;
	}

	if(against->GetSpecialAbility(IMMUNE_MELEE_NONMAGICAL)) {
		if(weapon_item) {
			bool MagicWeapon = false;
			if(weapon_item->GetItem() && weapon_item->GetItem()->Magic)
				MagicWeapon = true;
			else {
				if(spellbonuses.MagicWeapon || itembonuses.MagicWeapon)
					MagicWeapon = true;
			}

			if(MagicWeapon) {
				if(IsClient() && GetLevel() < weapon_item->GetItem()->RecLevel)
					dmg = CastToClient()->CalcRecommendedLevelBonus(GetLevel(), weapon_item->GetItem()->RecLevel, weapon_item->GetItem()->Damage);
				else
					dmg = weapon_item->GetItem()->Damage;

				for(int x = 0; x < MAX_AUGMENT_SLOTS; x++){
					if(weapon_item->GetAugment(x) && weapon_item->GetAugment(x)->GetItem()) {
						dmg += weapon_item->GetAugment(x)->GetItem()->Damage;
						if (hate)
							*hate += (weapon_item->GetAugment(x)->GetItem()->Damage + weapon_item->GetAugment(x)->GetItem()->ElemDmgAmt);
					}
				}
				dmg = dmg <= 0 ? 1 : dmg;
			}
			else
				return 0;
		}
		else{
			if((GetClass() == MONK || GetClass() == BEASTLORD) && GetLevel() >= 30){
				dmg = GetMonkHandToHandDamage();
				if (hate)
					*hate += dmg;
			}
			else if(GetOwner() && GetLevel() >= RuleI(Combat, PetAttackMagicLevel))
				dmg = 1;															
			else if(GetSpecialAbility(SPECATK_MAGICAL))
				dmg = 1;
			else
				return 0;
		}
	}
	else{
		if(weapon_item) {
			if(weapon_item->GetItem()) {
				if(IsClient() && GetLevel() < weapon_item->GetItem()->RecLevel)
					dmg = CastToClient()->CalcRecommendedLevelBonus(GetLevel(), weapon_item->GetItem()->RecLevel, weapon_item->GetItem()->Damage);
				else
					dmg = weapon_item->GetItem()->Damage;

				for(int x = 0; x < MAX_AUGMENT_SLOTS; x++) {
					if(weapon_item->GetAugment(x) && weapon_item->GetAugment(x)->GetItem()) {
						dmg += weapon_item->GetAugment(x)->GetItem()->Damage;
						if (hate)
							*hate += (weapon_item->GetAugment(x)->GetItem()->Damage + weapon_item->GetAugment(x)->GetItem()->ElemDmgAmt);
					}
				}
				dmg = dmg <= 0 ? 1 : dmg;
			}
		}
		else {
			if(GetClass() == MONK || GetClass() == BEASTLORD){
				dmg = GetMonkHandToHandDamage();
				if (hate)
					*hate += dmg;
			}
			else
				dmg = 1;
		}
	}

	int eledmg = 0;
	if(!against->GetSpecialAbility(IMMUNE_MAGIC)){
		if(weapon_item && weapon_item->GetItem() && weapon_item->GetItem()->ElemDmgAmt){
			if(IsClient() && GetLevel() < weapon_item->GetItem()->RecLevel)
				eledmg = CastToClient()->CalcRecommendedLevelBonus(GetLevel(), weapon_item->GetItem()->RecLevel, weapon_item->GetItem()->ElemDmgAmt);
			else
				eledmg = weapon_item->GetItem()->ElemDmgAmt;

			if(eledmg)
				eledmg = (eledmg * against->ResistSpell(weapon_item->GetItem()->ElemDmgType, 0, this) / 100);
		}

		if(weapon_item){
			for(int x = 0; x < MAX_AUGMENT_SLOTS; x++) {
				if(weapon_item->GetAugment(x) && weapon_item->GetAugment(x)->GetItem()) {
					if(weapon_item->GetAugment(x)->GetItem()->ElemDmgAmt)
						eledmg += (weapon_item->GetAugment(x)->GetItem()->ElemDmgAmt * against->ResistSpell(weapon_item->GetAugment(x)->GetItem()->ElemDmgType, 0, this) / 100);
				}
			}
		}
	}

	if(against->GetSpecialAbility(IMMUNE_MELEE_EXCEPT_BANE)) {
		if(weapon_item && weapon_item->GetItem()){
			if(weapon_item->GetItem()->BaneDmgBody == against->GetBodyType()) {
				if(IsClient() && GetLevel() < weapon_item->GetItem()->RecLevel)
					banedmg += CastToClient()->CalcRecommendedLevelBonus(GetLevel(), weapon_item->GetItem()->RecLevel, weapon_item->GetItem()->BaneDmgAmt);
				else
					banedmg += weapon_item->GetItem()->BaneDmgAmt;
			}

			if(weapon_item->GetItem()->BaneDmgRace == against->GetRace()) {
				if(IsClient() && GetLevel() < weapon_item->GetItem()->RecLevel)
					banedmg += CastToClient()->CalcRecommendedLevelBonus(GetLevel(), weapon_item->GetItem()->RecLevel, weapon_item->GetItem()->BaneDmgRaceAmt);
				else
					banedmg += weapon_item->GetItem()->BaneDmgRaceAmt;
			}

			for(int x = 0; x < MAX_AUGMENT_SLOTS; x++){
				if(weapon_item->GetAugment(x) && weapon_item->GetAugment(x)->GetItem()){
					if(weapon_item->GetAugment(x)->GetItem()->BaneDmgBody == against->GetBodyType())
						banedmg += weapon_item->GetAugment(x)->GetItem()->BaneDmgAmt;

					if(weapon_item->GetAugment(x)->GetItem()->BaneDmgRace == against->GetRace())
						banedmg += weapon_item->GetAugment(x)->GetItem()->BaneDmgRaceAmt;
				}
			}
		}

		if(!eledmg && !banedmg) {
			if(!GetSpecialAbility(SPECATK_BANE))
				return 0;
			else
				return 1;
		}
		else {
			dmg += (banedmg + eledmg);
			if (hate)
				*hate += banedmg;
		}
	}
	else{
		if(weapon_item && weapon_item->GetItem()){
			if(weapon_item->GetItem()->BaneDmgBody == against->GetBodyType()){
				if(IsClient() && GetLevel() < weapon_item->GetItem()->RecLevel)
					banedmg += CastToClient()->CalcRecommendedLevelBonus(GetLevel(), weapon_item->GetItem()->RecLevel, weapon_item->GetItem()->BaneDmgAmt);
				else
					banedmg += weapon_item->GetItem()->BaneDmgAmt;
			}

			if(weapon_item->GetItem()->BaneDmgRace == against->GetRace()){
				if(IsClient() && GetLevel() < weapon_item->GetItem()->RecLevel)
					banedmg += CastToClient()->CalcRecommendedLevelBonus(GetLevel(), weapon_item->GetItem()->RecLevel, weapon_item->GetItem()->BaneDmgRaceAmt);
				else
					banedmg += weapon_item->GetItem()->BaneDmgRaceAmt;
			}

			for(int x = 0; x < MAX_AUGMENT_SLOTS; x++){
				if(weapon_item->GetAugment(x) && weapon_item->GetAugment(x)->GetItem()){
					if(weapon_item->GetAugment(x)->GetItem()->BaneDmgBody == against->GetBodyType())
						banedmg += weapon_item->GetAugment(x)->GetItem()->BaneDmgAmt;

					if(weapon_item->GetAugment(x)->GetItem()->BaneDmgRace == against->GetRace())
						banedmg += weapon_item->GetAugment(x)->GetItem()->BaneDmgRaceAmt;
				}
			}
		}
		dmg += (banedmg + eledmg);
		if (hate)
			*hate += banedmg;
	}

	if(dmg <= 0)
		return 0;
	else
		return dmg;
}

bool Client::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts) {
	if (!other) {
		SetTarget(nullptr);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to Client::Attack() for evaluation!");
		return false;
	}

	if(!GetTarget())
		SetTarget(other);

	mlog(COMBAT__ATTACKS, "Attacking %s with hand %d %s", other?other->GetName():"(nullptr)", Hand, bRiposte?"(this is a riposte)":"");

	if ((IsCasting() && GetClass() != BARD && !IsFromSpell)	|| other == nullptr || ((IsClient() && CastToClient()->dead) || (other->IsClient() && other->CastToClient()->dead)) || (GetHP() < 0) || (!IsAttackAllowed(other))) {
		mlog(COMBAT__ATTACKS, "Attack canceled, invalid circumstances.");
		return false;
	}

	if(DivineAura() && !GetGM()) {
		mlog(COMBAT__ATTACKS, "Attack canceled, Divine Aura is in effect.");
		Message_StringID(MT_DefaultText, DIVINE_AURA_NO_ATK);
		return false;
	}

	if (GetFeigned())
		return false;


	ItemInst* weapon;
	if (Hand == 14) {
		weapon = GetInv().GetItem(SLOT_SECONDARY);
		OffHandAtk(true);
	}
	else {
		weapon = GetInv().GetItem(SLOT_PRIMARY);
		OffHandAtk(false);
	}

	if(weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			mlog(COMBAT__ATTACKS, "Attack canceled, Item %s (%d) is not a weapon.", weapon->GetItem()->Name, weapon->GetID());
			return(false);
		}
		mlog(COMBAT__ATTACKS, "Attacking with weapon: %s (%d)", weapon->GetItem()->Name, weapon->GetID());
	}
	else
		mlog(COMBAT__ATTACKS, "Attacking without a weapon.");
		
	SkillUseTypes skillinuse;
	AttackAnimation(skillinuse, Hand, weapon);
	mlog(COMBAT__ATTACKS, "Attacking with %s in slot %d using skill %d", weapon?weapon->GetItem()->Name:"Fist", Hand, skillinuse);

	int damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon)
		hate = (weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt);
	int weapon_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && weapon_damage > 1)
		hate = weapon_damage;

	if(weapon_damage > 0) {
		if(IsBerserk() && GetClass() == BERSERKER) {
			int bonus = (3 + GetLevel() / 10);
			weapon_damage = (weapon_damage * (100 + bonus) / 100);
			mlog(COMBAT__DAMAGE, "Berserker damage bonus increases DMG to %d", weapon_damage);
		}

		if(TryFinishingBlow(other, skillinuse))
			return true;

		int min_hit = 1;
		int max_hit = ((2 * weapon_damage * GetDamageTable(skillinuse)) / 100);
		CheckIncreaseSkill(skillinuse, other, -15);
		CheckIncreaseSkill(SkillOffense, other, -15);

#ifndef EQEMU_NO_WEAPON_DAMAGE_BONUS
		int ucDamageBonus = 0;
		if(Hand == 13 && GetLevel() >= 28 && IsWarriorClass()) {
			ucDamageBonus = GetWeaponDamageBonus( weapon ? weapon->GetItem() : (const Item_Struct*) nullptr );
			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		if (Hand == 14) {
			if (aabonuses.SecondaryDmgInc || itembonuses.SecondaryDmgInc || spellbonuses.SecondaryDmgInc) {
				ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const Item_Struct*) nullptr);
				min_hit += (int) ucDamageBonus;
				max_hit += (int) ucDamageBonus;
				hate += ucDamageBonus;
			}
		}
		min_hit += (min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100);

		if(max_hit < min_hit)
			max_hit = min_hit;

		if(RuleB(Combat, UseIntervalAC))
			damage = max_hit;
		else
			damage = MakeRandomInt(min_hit, max_hit);

		damage = mod_client_damage(damage, skillinuse, Hand, weapon, other);

		mlog(COMBAT__DAMAGE, "Damage calculated to %d (min %d, max %d, str %d, skill %d, DMG %d, lv %d)",
			damage, min_hit, max_hit, GetSTR(), GetSkill(skillinuse), weapon_damage, mylevel);

		if(opts) {
			damage *= opts->damage_percent;
			damage += opts->damage_flat;
			hate *= opts->hate_percent;
			hate += opts->hate_flat;
		}

		if(!other->CheckHitChance(this, skillinuse, Hand)) {
			mlog(COMBAT__ATTACKS, "Attack missed. Damage set to 0.");
			damage = 0;
		}
		else {
			other->AvoidDamage(this, damage);
			other->MeleeMitigation(this, damage, min_hit, opts);
			if(damage > 0) {
				ApplyMeleeDamageBonus(skillinuse, damage);
				damage += (itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse) / 100) + GetSkillDmgAmt(skillinuse);
				TryCriticalHit(other, skillinuse, damage, opts);
			}
			mlog(COMBAT__DAMAGE, "Final damage after all reductions: %d", damage);
		}

		bool slippery_attack = false;
		if (damage == -3) {
			if (bRiposte)
				return false;
			else {
				if (Hand == 14) {
					int16 OffhandRiposteFail = aabonuses.OffhandRiposteFail + itembonuses.OffhandRiposteFail + spellbonuses.OffhandRiposteFail;
					OffhandRiposteFail *= -1;

					if (OffhandRiposteFail && (OffhandRiposteFail > 99 || (MakeRandomInt(0, 100) < OffhandRiposteFail))) {
						damage = 0;
						slippery_attack = true;
					}
					else {
						DoRiposte(other);
						if (IsDead())
							return false;
					}
				}
				else {
					DoRiposte(other);
					if (IsDead())
						return false;
				}
			}
		}

		if (((damage < 0) || slippery_attack) && !bRiposte && !IsStrikethrough) {
			int16 bonusStrikeThrough = (itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough);
			if(bonusStrikeThrough && (MakeRandomInt(0, 100) < bonusStrikeThrough)) {
				Message_StringID(MT_StrikeThrough, STRIKETHROUGH_STRING);
				Attack(other, Hand, false, true);
				return false;
			}
		}
	}
	else
		damage = -5;
	if (!bRiposte)
		other->AddToHateList(this, hate);
	other->Damage(this, damage, SPELL_UNKNOWN, skillinuse);
	if (IsDead())
		return false;
	MeleeLifeTap(damage);
	if (damage > 0)
		CheckNumHitsRemaining(5);

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

	if(GetTarget())
		TriggerDefensiveProcs(weapon, other, Hand, damage);

	if (damage > 0)
		return true;

	else
		return false;
}

void Mob::Heal() {
	SetMaxHP();
	SendHPUpdate();
}

void Client::Damage(Mob* other, int32 damage, uint16 spell_id, SkillUseTypes attack_skill, bool avoidable, int8 buffslot, bool iBuffTic) {
	if(dead || IsCorpse())
		return;

	if(spell_id==0)
		spell_id = SPELL_UNKNOWN;

	if(spell_id!=0 && spell_id != SPELL_UNKNOWN && other && damage > 0) {
		if(other->IsNPC() && !other->IsPet()) {
			float npcspellscale = other->CastToNPC()->GetSpellScale();
			damage = ((float)damage * npcspellscale) / (float)100;
		}
	}
	
	if(other && other->IsClient() && (other != this) && damage > 0) {
		int PvPMitigation = 100;
		if(attack_skill == SkillArchery)
			PvPMitigation = 80;
		else
			PvPMitigation = 67;
		damage = (damage * PvPMitigation) / 100;
	}

	if(!ClientFinishedLoading())
		damage = -5;

	CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic);

	if (damage > 0) {
		if (spell_id == SPELL_UNKNOWN)
			CheckIncreaseSkill(SkillDefense, other, -15);
	}
}

bool Client::Death(Mob* killerMob, int32 damage, uint16 spell, SkillUseTypes attack_skill) {
	if(!ClientFinishedLoading())
		return false;

	if(dead)
		return false;

	if(!spell) 
		spell = SPELL_UNKNOWN;

	char buffer[48] = { 0 };
	snprintf(buffer, 47, "%d %d %d %d", killerMob ? killerMob->GetID() : 0, damage, spell, static_cast<int>(attack_skill));
	if(parse->EventPlayer(EVENT_DEATH, this, buffer, 0) != 0) {
		if(GetHP() < 0) {
			SetHP(0);
		}
		return false;
	}

	if(killerMob && killerMob->IsClient() && (spell != SPELL_UNKNOWN) && damage > 0) {
		char val1[20]={0};
		entity_list.MessageClose_StringID(this, false, 100, MT_NonMelee, HIT_NON_MELEE,
			killerMob->GetCleanName(), GetCleanName(), ConvertArray(damage, val1));
	}

	int exploss = 0;
	mlog(COMBAT__HITS, "Fatal blow dealt by %s with %d damage, spell %d, skill %d", killerMob ? killerMob->GetName() : "Unknown", damage, spell, attack_skill);
	uint8 killed_level = GetLevel();	
	SendLogoutPackets();
	EQApplicationPacket app2(OP_BecomeCorpse, sizeof(BecomeCorpse_Struct));
	BecomeCorpse_Struct* bc = (BecomeCorpse_Struct*)app2.pBuffer;
	bc->spawn_id = GetID();
	bc->x = GetX();
	bc->y = GetY();
	bc->z = GetZ();
	QueuePacket(&app2);
	EQApplicationPacket app(OP_Death, sizeof(Death_Struct));
	Death_Struct* d = (Death_Struct*)app.pBuffer;
	d->spawn_id = GetID();
	d->killer_id = killerMob ? killerMob->GetID() : 0;
	d->corpseid=GetID();
	d->bindzoneid = m_pp.binds[0].zoneId;
	d->spell_id = spell == SPELL_UNKNOWN ? 0xffffffff : spell;
	d->attack_skill = spell != SPELL_UNKNOWN ? 0xe7 : attack_skill;
	d->damage = damage;
	app.priority = 6;
	entity_list.QueueClients(this, &app);
	InterruptSpell();
	SetPet(0);
	SetHorseId(0);
	dead = true;

	if(GetMerc())
		GetMerc()->Suspend();

	if (killerMob != nullptr) {
		if (killerMob->IsNPC()) {
			parse->EventNPC(EVENT_SLAY, killerMob->CastToNPC(), this, "", 0);

			mod_client_death_npc(killerMob);

			uint16 emoteid = killerMob->GetEmoteID();
			if(emoteid != 0)
				killerMob->CastToNPC()->DoNPCEmote(KILLEDPC,emoteid);
			killerMob->TrySpellOnKill(killed_level,spell);
		}

		if(killerMob->IsClient() && (IsDueling() || killerMob->CastToClient()->IsDueling())) {
			SetDueling(false);
			SetDuelTarget(0);
			if (killerMob->IsClient() && killerMob->CastToClient()->IsDueling() && killerMob->CastToClient()->GetDuelTarget() == GetID()) {
				killerMob->CastToClient()->SetDueling(false);
				killerMob->CastToClient()->SetDuelTarget(0);
				entity_list.DuelMessage(killerMob,this,false);

				mod_client_death_duel(killerMob);

			}
			else {
				Mob* who = entity_list.GetMob(GetDuelTarget());
				if(who && who->IsClient()) {
					who->CastToClient()->SetDueling(false);
					who->CastToClient()->SetDuelTarget(0);
				}
			}
		}
	}
	entity_list.RemoveFromTargets(this);
	hate_list.clear(this);
	RemoveAutoXTargets();
	ClearAllProximities();
	if(RuleB(Character, UseDeathExpLossMult)) {
		float GetNum [] = {0.005f,0.015f,0.025f,0.035f,0.045f,0.055f,0.065f,0.075f,0.085f,0.095f,0.110f };
		int Num = RuleI(Character, DeathExpLossMultiplier);
		if((Num < 0) || (Num > 10))
			Num = 3;
		float loss = GetNum[Num];
		exploss=(int)((float)GetEXP() * (loss));
	}

	if(!RuleB(Character, UseDeathExpLossMult))
		exploss = (int)(GetLevel() * (GetLevel() / 18.0) * 12000);

	if((GetLevel() < RuleI(Character, DeathExpLossLevel)) || (GetLevel() > RuleI(Character, DeathExpLossMaxLevel)) || IsBecomeNPC())
		exploss = 0;
		
	else if(killerMob) {
		if( killerMob->IsClient() )
			exploss = 0;
		else if( killerMob->GetOwner() && killerMob->GetOwner()->IsClient() )
			exploss = 0;
	}

	if(spell != SPELL_UNKNOWN) {
		uint32 buff_count = GetMaxTotalSlots();
		for(uint16 buffIt = 0; buffIt < buff_count; buffIt++) {
			if(buffs[buffIt].spellid == spell && buffs[buffIt].client) {
				exploss = 0;
				break;
			}
		}
	}

	bool LeftCorpse = false;

	if(!GetGM())
	{
		if(exploss > 0) {
			int32 newexp = GetEXP();
			if(exploss > newexp) 
				newexp = 1;
			else
				newexp -= exploss;
			SetEXP(newexp, GetAAXP());
		}

		BuffFadeNonPersistDeath();
		if((GetClientVersionBit() & BIT_SoFAndLater) && RuleB(Character, RespawnFromHover))
			UnmemSpellAll(true);
		else
			UnmemSpellAll(false);

		if(RuleB(Character, LeaveCorpses) && GetLevel() >= RuleI(Character, DeathItemLossLevel) || RuleB(Character, LeaveNakedCorpses)) {
			Corpse *new_corpse = new Corpse(this, exploss);
			char tmp[20];
			database.GetVariable("ServerType", tmp, 9);
			if(atoi(tmp)==1 && killerMob != nullptr && killerMob->IsClient()) {
				char tmp2[10] = {0};
				database.GetVariable("PvPreward", tmp, 9);
				int reward = atoi(tmp);
				if(reward==3) {
					database.GetVariable("PvPitem", tmp2, 9);
					int pvpitem = atoi(tmp2);
					if(pvpitem > 0 && pvpitem < 300000)
						new_corpse->setPKItem(pvpitem);
				}
				else if(reward == 2)
					new_corpse->setPKItem(-1);
				else if(reward == 1)
					new_corpse->setPKItem(1);
				else
					new_corpse->setPKItem(0);
				if(killerMob->CastToClient()->isgrouped) {
					Group* group = entity_list.GetGroupByClient(killerMob->CastToClient());
					if(group != 0) {
						for(int i = 0; i < 6; i++) {
							if(group->members[i] != nullptr)
								new_corpse->AllowMobLoot(group->members[i], i);
						}
					}
				}
			}

			entity_list.AddCorpse(new_corpse, GetID());
			SetID(0);
			entity_list.QueueClients(this, &app2, true);
			LeftCorpse = true;
		}
	}
	else
		BuffFadeDetrimental();
		
	if(LeftCorpse && (GetClientVersionBit() & BIT_SoFAndLater) && RuleB(Character, RespawnFromHover)) {
		ClearDraggedCorpses();
		RespawnFromHoverTimer.Start(RuleI(Character, RespawnFromHoverTimer) * 1000);
		SendRespawnBinds();
	}
	else {
		if(isgrouped) {
			Group *g = GetGroup();
			if(g)
				g->MemberZoned(this);
		}
		Raid* r = entity_list.GetRaidByClient(this);

		if(r)
			r->MemberZoned(this);

		dead_timer.Start(5000, true);
		m_pp.zone_id = m_pp.binds[0].zoneId;
		m_pp.zoneInstance = 0;
		database.MoveCharacterToZone(this->CharacterID(), database.GetZoneName(m_pp.zone_id));
		Save();
		GoToDeath();
	}
	parse->EventPlayer(EVENT_DEATH_COMPLETE, this, buffer, 0);
	return true;
}

bool NPC::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts) {
	int damage = 0;
	if (!other) {
		SetTarget(nullptr);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to NPC::Attack() for evaluation!");
		return false;
	}

	if(DivineAura())
		return(false);

	if(!GetTarget())
		SetTarget(other);

	if (!IsAttackAllowed(other)) {
		if (this->GetOwnerID())
			this->Say_StringID(NOT_LEGAL_TARGET);
		if(other) {
			if (other->IsClient())
				other->CastToClient()->RemoveXTarget(this, false);
			RemoveFromHateList(other);
			mlog(COMBAT__ATTACKS, "I am not allowed to attack %s", other->GetName());
		}
		return false;
	}

	FaceTarget(GetTarget());
	SkillUseTypes skillinuse = SkillHandtoHand;
	if (Hand == 13) {
		skillinuse = static_cast<SkillUseTypes>(GetPrimSkill());
		OffHandAtk(false);
	}
	if (Hand == 14) {
		skillinuse = static_cast<SkillUseTypes>(GetSecSkill());
		OffHandAtk(true);
	}

	const Item_Struct* weapon = nullptr;
	if (Hand == 13 && equipment[SLOT_PRIMARY] > 0)
		weapon = database.GetItem(equipment[SLOT_PRIMARY]);
	else if (equipment[SLOT_SECONDARY])
		weapon = database.GetItem(equipment[SLOT_SECONDARY]);

	if(weapon) {
		mlog(COMBAT__ATTACKS, "Attacking with weapon: %s (%d) (too bad im not using it for much)", weapon->Name, weapon->ID);

		if(Hand == 14 && weapon->ItemType == ItemTypeShield) {
			mlog(COMBAT__ATTACKS, "Attack with shield canceled.");
			return false;
		}

		switch(weapon->ItemType) {
			case ItemType1HSlash:
				skillinuse = Skill1HSlashing;
				break;
			case ItemType2HSlash:
				skillinuse = Skill2HSlashing;
				break;
			case ItemType1HPiercing:
			case ItemType2HPiercing:
				skillinuse = Skill1HPiercing;
				break;
			case ItemType1HBlunt:
				skillinuse = Skill1HBlunt;
				break;
			case ItemType2HBlunt:
				skillinuse = Skill2HBlunt;
				break;
			case ItemTypeBow:
				skillinuse = SkillArchery;
				break;
			case ItemTypeLargeThrowing:
			case ItemTypeSmallThrowing:
				skillinuse = SkillThrowing;
				break;
			default:
				skillinuse = SkillHandtoHand;
				break;
		}
	}

	int weapon_damage = GetWeaponDamage(other, weapon);

	int16 charges = 0;
	ItemInst weapon_inst(weapon, charges);
	AttackAnimation(skillinuse, Hand, &weapon_inst);

	if(skillinuse == 99)
		skillinuse = static_cast<SkillUseTypes>(36);

	if((weapon_damage) > 0) {
		uint16 eleBane = 0;
		if(weapon){
			if(weapon->BaneDmgBody == other->GetBodyType())
				eleBane += weapon->BaneDmgAmt;
				
			if(weapon->BaneDmgRace == other->GetRace())
				eleBane += weapon->BaneDmgRaceAmt;

			if(weapon->ElemDmgAmt)
				eleBane += (weapon->ElemDmgAmt * other->ResistSpell(weapon->ElemDmgType, 0, this) / 100);
		}

		if(!RuleB(NPC, UseItemBonusesForNonPets)){
			if(!GetOwner())
				eleBane = 0;
		}

		uint8 otherlevel = other->GetLevel();
		uint8 mylevel = this->GetLevel();
		otherlevel = otherlevel ? otherlevel : 1;
		mylevel = mylevel ? mylevel : 1;
		
		if(RuleB(Combat, UseIntervalAC))
			damage = (max_dmg+eleBane);
		else
			damage = MakeRandomInt((min_dmg+eleBane),(max_dmg+eleBane));


		if((min_dmg+eleBane) != 0 && damage < (min_dmg + eleBane)) {
			mlog(COMBAT__DAMAGE, "Damage (%d) is below min (%d). Setting to min.", damage, (min_dmg + eleBane));
			damage = (min_dmg+eleBane);
		}
		if((max_dmg+eleBane) != 0 && damage > (max_dmg + eleBane)) {
			mlog(COMBAT__DAMAGE, "Damage (%d) is above max (%d). Setting to max.", damage, (max_dmg + eleBane));
			damage = (max_dmg+eleBane);
		}
		
		damage = mod_npc_damage(damage, skillinuse, Hand, weapon, other);

		int32 hate = damage;
		if(IsPet())
			hate = (hate * 100 / GetDamageTable(skillinuse));

		if(other->IsClient() && other->CastToClient()->IsSitting()) {
			mlog(COMBAT__DAMAGE, "Client %s is sitting. Hitting for max damage (%d).", other->GetName(), (max_dmg + eleBane));
			damage = (max_dmg+eleBane);
			damage += ((itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse) / 100) + GetSkillDmgAmt(skillinuse));

			if(opts) {
				damage *= opts->damage_percent;
				damage += opts->damage_flat;
				hate *= opts->hate_percent;
				hate += opts->hate_flat;
			}

			mlog(COMBAT__HITS, "Generating hate %d towards %s", hate, GetName());
			other->AddToHateList(this, hate);

		}
		else {
			if(opts) {
				damage *= opts->damage_percent;
				damage += opts->damage_flat;
				hate *= opts->hate_percent;
				hate += opts->hate_flat;
			}

			if(!other->CheckHitChance(this, skillinuse, Hand))
				damage = 0;
			else {
				other->AvoidDamage(this, damage);
				other->MeleeMitigation(this, damage, min_dmg + eleBane, opts);
				if(damage > 0) {
					ApplyMeleeDamageBonus(skillinuse, damage);
					damage += ((itembonuses.HeroicSTR / 10) + (damage * other->GetSkillDmgTaken(skillinuse) / 100) + GetSkillDmgAmt(skillinuse));
					TryCriticalHit(other, skillinuse, damage, opts);
				}
				mlog(COMBAT__HITS, "Generating hate %d towards %s", hate, GetName());
				if(damage > 0)
					other->AddToHateList(this, hate);
				else
					other->AddToHateList(this, 0);
			}
		}
		mlog(COMBAT__DAMAGE, "Final damage against %s: %d", other->GetName(), damage);
		if(other->IsClient() && IsPet() && GetOwner()->IsClient())
			damage /= 2;
	}
	else
		damage = -5;

	if (bRiposte && damage == -3) {
		mlog(COMBAT__DAMAGE, "Riposte of riposte canceled.");
		return false;
	}

	int16 DeathHP = 0;
	DeathHP = (other->GetDelayDeath() * -1);

	if(GetHP() > 0 && other->GetHP() >= DeathHP)
		other->Damage(this, damage, SPELL_UNKNOWN, skillinuse, false);
	else
		return false;

	if (HasDied())
		return false;

	MeleeLifeTap(damage);
	
	if (damage > 0)
		CheckNumHitsRemaining(5);

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

	if(hidden || improved_hidden) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}


	hidden = false;
	improved_hidden = false;

	if (!GetTarget())
		return true;

	if(!bRiposte && other->GetHP() > 0 ) {
		TryWeaponProc(nullptr, weapon, other, Hand);
		TrySpellProc(nullptr, weapon, other, Hand);
	}

	TriggerDefensiveProcs(nullptr, other, Hand, damage);

	if (damage == -3)
		DoRiposte(other);

	if (damage > 0)
		return true;
	else
		return false;
}

void NPC::Damage(Mob* other, int32 damage, uint16 spell_id, SkillUseTypes attack_skill, bool avoidable, int8 buffslot, bool iBuffTic) {
	if(spell_id==0)
		spell_id = SPELL_UNKNOWN;

	if(attacked_timer.Check()) {
		mlog(COMBAT__HITS, "Triggering EVENT_ATTACK due to attack by %s", other->GetName());
		parse->EventNPC(EVENT_ATTACK, this, other, "", 0);
	}
	attacked_timer.Start(CombatEventTimer_expire);

	if (!IsEngaged())
		zone->AddAggroMob();

	if(GetClass() == LDON_TREASURE) {
		if(IsLDoNLocked() && GetLDoNLockedSkill() != LDoNTypeMechanical)
			damage = -5;
		else  {
			if(IsLDoNTrapped()) {
				Message_StringID(13, LDON_ACCIDENT_SETOFF2);
				SpellFinished(GetLDoNTrapSpellID(), other, 10, 0, -1, spells[GetLDoNTrapSpellID()].ResistDiff, false);
				SetLDoNTrapSpellID(0);
				SetLDoNTrapped(false);
				SetLDoNTrapDetected(false);
			}
		}
	}

	CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic);

	if(damage > 0) {
		if(!IsPet())
			CheckFlee();
	}
}

bool NPC::Death(Mob* killerMob, int32 damage, uint16 spell, SkillUseTypes attack_skill) {
	mlog(COMBAT__HITS, "Fatal blow dealt by %s with %d damage, spell %d, skill %d", killerMob->GetName(), damage, spell, attack_skill);
	Mob *oos = nullptr;
	if(killerMob) {
		oos = killerMob->GetOwnerOrSelf();
		char buffer[48] = { 0 };
		snprintf(buffer, 47, "%d %d %d %d", killerMob ? killerMob->GetID() : 0, damage, spell, static_cast<int>(attack_skill));
		if(parse->EventNPC(EVENT_DEATH, this, oos, buffer, 0) != 0) {
			if(GetHP() < 0)
				SetHP(0);
			return false;
		}

		if(killerMob && killerMob->IsClient() && (spell != SPELL_UNKNOWN) && damage > 0) {
			char val1[20]={0};
			entity_list.MessageClose_StringID(this, false, 100, MT_NonMelee, HIT_NON_MELEE,
				killerMob->GetCleanName(), GetCleanName(), ConvertArray(damage, val1));
		}
	}
	else {
		char buffer[48] = { 0 };
		snprintf(buffer, 47, "%d %d %d %d", killerMob ? killerMob->GetID() : 0, damage, spell, static_cast<int>(attack_skill));
		if(parse->EventNPC(EVENT_DEATH, this, nullptr, buffer, 0) != 0) {
			if(GetHP() < 0)
				SetHP(0);
			return false;
		}
	}

	if (IsEngaged()) {
		zone->DelAggroMob();
#if EQDEBUG >= 11
		LogFile->write(EQEMuLog::Debug,"NPC::Death() Mobs currently Aggro %i", zone->MobsAggroCount());
#endif
	}
	SetHP(0);
	SetPet(0);
	Mob* killer = GetHateDamageTop(this);

	entity_list.RemoveFromTargets(this, p_depop);

	if(p_depop == true)
		return false;
	
	HasAISpellEffects = false;
	BuffFadeAll();
	uint8 killed_level = GetLevel();

	EQApplicationPacket* app= new EQApplicationPacket(OP_Death,sizeof(Death_Struct));
	Death_Struct* d = (Death_Struct*)app->pBuffer;
	d->spawn_id = GetID();
	d->killer_id = killerMob ? killerMob->GetID() : 0;
	d->bindzoneid = 0;
	d->spell_id = spell == SPELL_UNKNOWN ? 0xffffffff : spell;
	d->attack_skill = SkillDamageTypes[attack_skill];
	d->damage = damage;
	app->priority = 6;
	entity_list.QueueClients(killerMob, app, false);

	if(respawn2)
		respawn2->DeathReset(1);

	if (killerMob) {
		if(GetClass() != LDON_TREASURE)
			hate_list.add(killerMob, damage);
	}

	safe_delete(app);

	Mob *give_exp = hate_list.getHighestDamage(this);

	if(give_exp == nullptr)
		give_exp = killer;

	if(give_exp && give_exp->HasOwner()) {
		bool ownerInGroup = false;
		if((give_exp->HasGroup() && give_exp->GetGroup()->IsGroupMember(give_exp->GetUltimateOwner())) || (give_exp->IsPet() && (give_exp->GetOwner()->IsClient() || ( give_exp->GetOwner()->HasGroup() && give_exp->GetOwner()->GetGroup()->IsGroupMember(give_exp->GetOwner()->GetUltimateOwner())))))
			ownerInGroup = true;
		give_exp = give_exp->GetUltimateOwner();
	}
	int PlayerCount = 0;
	Client *give_exp_client = nullptr;
	if(give_exp && give_exp->IsClient())
		give_exp_client = give_exp->CastToClient();

	bool IsLdonTreasure = (this->GetClass() == LDON_TREASURE);
	if (give_exp_client && !IsCorpse() && MerchantType == 0) {
		Group *kg = entity_list.GetGroupByClient(give_exp_client);
		Raid *kr = entity_list.GetRaidByClient(give_exp_client);
        int32 finalxp = EXP_FORMULA;
        finalxp = give_exp_client->mod_client_xp(finalxp, this);
		if(kr) {
			if(!IsLdonTreasure) {
				kr->SplitExp((finalxp), this);
				if(killerMob && (kr->IsRaidMember(killerMob->GetName()) || kr->IsRaidMember(killerMob->GetUltimateOwner()->GetName())))
					killerMob->TrySpellOnKill(killed_level,spell);
			}
			
			for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
				if (kr->members[i].member != nullptr) {
					parse->EventNPC(EVENT_KILLED_MERIT, this, kr->members[i].member, "killed", 0);
					mod_npc_killed_merit(kr->members[i].member);
					if(RuleB(TaskSystem, EnableTaskSystem))
						kr->members[i].member->UpdateTasksOnKill(GetNPCTypeID());
					PlayerCount++;
				}
			}

			if(RuleB(QueryServ, PlayerLogNPCKills)) {
				ServerPacket* pack = new ServerPacket(ServerOP_QSPlayerLogNPCKills, sizeof(QSPlayerLogNPCKill_Struct) + (sizeof(QSPlayerLogNPCKillsPlayers_Struct) * PlayerCount));
				PlayerCount = 0;
				QSPlayerLogNPCKill_Struct* QS = (QSPlayerLogNPCKill_Struct*) pack->pBuffer;
				QS->s1.NPCID = this->GetNPCTypeID();
				QS->s1.ZoneID = this->GetZoneID();
				QS->s1.Type = 2;
				for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
					if (kr->members[i].member != nullptr) {
						Client *c = kr->members[i].member;
						QS->Chars[PlayerCount].char_id = c->CharacterID();
						PlayerCount++;
					}
				}
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
		}
		else if (give_exp_client->IsGrouped() && kg != nullptr) {
			if(!IsLdonTreasure) {
				kg->SplitExp((finalxp), this);
				if(killerMob && (kg->IsGroupMember(killerMob->GetName()) || kg->IsGroupMember(killerMob->GetUltimateOwner()->GetName())))
					killerMob->TrySpellOnKill(killed_level,spell);
			}
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (kg->members[i] != nullptr && kg->members[i]->IsClient()) {
					Client *c = kg->members[i]->CastToClient();
					parse->EventNPC(EVENT_KILLED_MERIT, this, c, "killed", 0);
					mod_npc_killed_merit(c);
					if(RuleB(TaskSystem, EnableTaskSystem))
						c->UpdateTasksOnKill(GetNPCTypeID());
					PlayerCount++;
				}
			}
			if(RuleB(QueryServ, PlayerLogNPCKills)) {
				ServerPacket* pack = new ServerPacket(ServerOP_QSPlayerLogNPCKills, sizeof(QSPlayerLogNPCKill_Struct) + (sizeof(QSPlayerLogNPCKillsPlayers_Struct) * PlayerCount));
				PlayerCount = 0;
				QSPlayerLogNPCKill_Struct* QS = (QSPlayerLogNPCKill_Struct*) pack->pBuffer;
				QS->s1.NPCID = this->GetNPCTypeID();
				QS->s1.ZoneID = this->GetZoneID();
				QS->s1.Type = 1;
				for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
					if (kg->members[i] != nullptr && kg->members[i]->IsClient()) {
						Client *c = kg->members[i]->CastToClient();
						QS->Chars[PlayerCount].char_id = c->CharacterID();
						PlayerCount++;
					}
				}
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
		}
		else {
			if(!IsLdonTreasure) {
				int conlevel = give_exp->GetLevelCon(GetLevel());
				if (conlevel != CON_GREEN) {
					if (GetOwner() && !GetOwner()->IsClient()) {
						give_exp_client->AddEXP((finalxp), conlevel);
						if(killerMob && (killerMob->GetID() == give_exp_client->GetID() || killerMob->GetUltimateOwner()->GetID() == give_exp_client->GetID()))
							killerMob->TrySpellOnKill(killed_level,spell);
					}
				}
			}
			parse->EventNPC(EVENT_KILLED_MERIT, this, give_exp_client, "killed", 0);
			mod_npc_killed_merit(give_exp_client);
			if(RuleB(TaskSystem, EnableTaskSystem))
				give_exp_client->UpdateTasksOnKill(GetNPCTypeID());
			if(RuleB(QueryServ, PlayerLogNPCKills)){
				ServerPacket* pack = new ServerPacket(ServerOP_QSPlayerLogNPCKills, sizeof(QSPlayerLogNPCKill_Struct) + (sizeof(QSPlayerLogNPCKillsPlayers_Struct) * 1));
				QSPlayerLogNPCKill_Struct* QS = (QSPlayerLogNPCKill_Struct*) pack->pBuffer;
				QS->s1.NPCID = this->GetNPCTypeID();
				QS->s1.ZoneID = this->GetZoneID();
				QS->s1.Type = 0;
				Client *c = give_exp_client;
				QS->Chars[0].char_id = c->CharacterID();
				PlayerCount++;
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
		}
	}
	
	if(give_exp_client)
		hate_list.DoFactionHits(GetNPCFactionID());

	if (!HasOwner() && !IsMerc() && class_ != MERCHANT && class_ != ADVENTUREMERCHANT && !GetSwarmInfo()  == 0 && killer && (killer->IsClient() || (killer->HasOwner() && killer->GetUltimateOwner()->IsClient()) || (killer->IsNPC() && killer->CastToNPC()->GetSwarmInfo() && killer->CastToNPC()->GetSwarmInfo()->GetOwner() && killer->CastToNPC()->GetSwarmInfo()->GetOwner()->IsClient()))) {
		if(killer != 0) {
			if(killer->GetOwner() != 0 && killer->GetOwner()->IsClient())
				killer = killer->GetOwner();

			if(!killer->CastToClient()->GetGM() && killer->IsClient())
				this->CheckMinMaxLevel(killer);
		}
		entity_list.RemoveFromAutoXTargets(this);
		uint16 emoteid = this->GetEmoteID();
		Corpse* corpse = new Corpse(this, &itemlist, GetNPCTypeID(), &NPCTypedata,level>54?RuleI(NPC,MajorNPCCorpseDecayTimeMS):RuleI(NPC,MinorNPCCorpseDecayTimeMS));
		entity_list.LimitRemoveNPC(this);
		entity_list.AddCorpse(corpse, GetID());

		entity_list.UnMarkNPC(GetID());
		entity_list.RemoveNPC(GetID());
		this->SetID(0);
		if(killer != 0 && emoteid != 0)
			corpse->CastToNPC()->DoNPCEmote(AFTERDEATH, emoteid);
		if(killer != 0 && killer->IsClient()) {
			corpse->AllowMobLoot(killer, 0);
			if(killer->IsGrouped()) {
				Group* group = entity_list.GetGroupByClient(killer->CastToClient());
				if(group != 0) {
					for(int i = 0; i < 6; i++) {
						if(group->members[i] != nullptr)
							corpse->AllowMobLoot(group->members[i], i);
					}
				}
			}
			else if(killer->IsRaidGrouped()) {
				Raid* r = entity_list.GetRaidByClient(killer->CastToClient());
				if(r) {
					int i = 0;
					for(int x = 0; x < MAX_RAID_MEMBERS; x++) {
						switch(r->GetLootType()) {
							case 0:
							case 1:
								if(r->members[x].member && r->members[x].IsRaidLeader) {
									corpse->AllowMobLoot(r->members[x].member, i);
									i++;
								}
								break;
							case 2:
								if(r->members[x].member && r->members[x].IsRaidLeader) {
									corpse->AllowMobLoot(r->members[x].member, i);
									i++;
								}
								else if(r->members[x].member && r->members[x].IsGroupLeader) {
									corpse->AllowMobLoot(r->members[x].member, i);
									i++;
								}
								break;
							case 3:
								if(r->members[x].member && r->members[x].IsLooter) {
									corpse->AllowMobLoot(r->members[x].member, i);
									i++;
								}
								break;
							case 4:
								if(r->members[x].member) {
									corpse->AllowMobLoot(r->members[x].member, i);
									i++;
								}
								break;
						}
					}
				}
			}
		}

		if(zone && zone->adv_data) {
			ServerZoneAdventureDataReply_Struct *sr = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
			if(sr->type == Adventure_Kill)
				zone->DoAdventureCountIncrease();
			else if(sr->type == Adventure_Assassinate){
				if(sr->data_id == GetNPCTypeID())
					zone->DoAdventureCountIncrease();
				else
					zone->DoAdventureAssassinationCountIncrease();
			}
		}
	}
	else
		entity_list.RemoveFromXTargets(this);
		
	if(oos) {
		mod_npc_killed(oos);
		uint16 emoteid = this->GetEmoteID();
		if(emoteid != 0)
			this->DoNPCEmote(ONDEATH, emoteid);
		if(oos->IsNPC()) {
			parse->EventNPC(EVENT_NPC_SLAY, oos->CastToNPC(), this, "", 0);
			uint16 emoteid = oos->GetEmoteID();
			if(emoteid != 0)
				oos->CastToNPC()->DoNPCEmote(KILLEDNPC, emoteid);
			killerMob->TrySpellOnKill(killed_level, spell);
		}
	}

	WipeHateList();
	p_depop = true;
	if(killerMob && killerMob->GetTarget() == this)
		killerMob->SetTarget(nullptr);

	entity_list.UpdateFindableNPCState(this, true);
	char buffer[48] = { 0 };
	snprintf(buffer, 47, "%d %d %d %d", killerMob ? killerMob->GetID() : 0, damage, spell, static_cast<int>(attack_skill));
	parse->EventNPC(EVENT_DEATH_COMPLETE, this, oos, buffer, 0);
	return true;
}

void Mob::AddToHateList(Mob* other, int32 hate, int32 damage, bool iYellForHelp, bool bFrenzy, bool iBuffTic) {	
	assert(other != nullptr);	
	if (other == this)
		return;

	if(damage < 0)
		hate = 1;

	bool wasengaged = IsEngaged();
	Mob* owner = other->GetOwner();
	Mob* mypet = this->GetPet();
	Mob* myowner = this->GetOwner();
	Mob* targetmob = this->GetTarget();

	if(other) {
		AddRampage(other);
		int hatemod = 100 + other->spellbonuses.hatemod + other->itembonuses.hatemod + other->aabonuses.hatemod;
		
		int16 shieldhatemod = other->spellbonuses.ShieldEquipHateMod + other->itembonuses.ShieldEquipHateMod + other->aabonuses.ShieldEquipHateMod;

		if (shieldhatemod && other->HasShieldEquiped())
			hatemod += shieldhatemod;
		
		if(hatemod < 1)
			hatemod = 1;
		hate = ((hate * (hatemod))/100);
	}

	if(IsPet() && GetOwner() && GetOwner()->GetAA(aaPetDiscipline) && IsHeld() && !IsFocused())
		return;

	if(IsPet() && GetOwner() && GetOwner()->GetAA(aaPetDiscipline) && IsHeld() && GetOwner()->GetAA(aaAdvancedPetDiscipline) >= 1 && IsFocused()) {
		if (!targetmob)
			return;
	}

	if (other->IsNPC() && (other->IsPet() || other->CastToNPC()->GetSwarmOwner() > 0))
		TryTriggerOnValueAmount(false, false, false, true);
	
	if(IsClient() && !IsAIControlled())
		return;

	if(IsFamiliar() || GetSpecialAbility(IMMUNE_AGGRO))
		return;

	if (other == myowner)
		return;

	if(other->GetSpecialAbility(IMMUNE_AGGRO_ON))
		return;

	if(GetSpecialAbility(NPC_TUNNELVISION)) {
		int tv_mod = GetSpecialAbilityParam(NPC_TUNNELVISION, 0);

		Mob *top = GetTarget();
		if(top && top != other) {
			if(tv_mod) {
				float tv = tv_mod / 100.0f;
				hate *= tv;
			}
			else
				hate *= RuleR(Aggro, TunnelVisionAggroMod);
		}
	}

	if(IsNPC() && CastToNPC()->IsUnderwaterOnly() && zone->HasWaterMap()) {
		if(!zone->watermap->InLiquid(other->GetX(), other->GetY(), other->GetZ()))
			return;
	}
	
	if(damage > GetHP())
		damage = GetHP();

	if (spellbonuses.ImprovedTaunt[1] && (GetLevel() < spellbonuses.ImprovedTaunt[0]) 
		&& other &&  (buffs[spellbonuses.ImprovedTaunt[2]].casterid != other->GetID()))
		hate = ((hate * spellbonuses.ImprovedTaunt[1]) / 100); 

	hate_list.add(other, hate, damage, bFrenzy, !iBuffTic);

	if(other->IsClient())
		other->CastToClient()->AddAutoXTarget(this);

	if(other->IsMerc()) {
		if(other->CastToMerc()->GetMercOwner() && other->CastToMerc()->GetMercOwner()->CastToClient()->GetFeigned())
			AddFeignMemory(other->CastToMerc()->GetMercOwner()->CastToClient());
			
		else {
			if(!hate_list.isHated(other->CastToMerc()->GetMercOwner()))
				hate_list.add(other->CastToMerc()->GetMercOwner(), 0, 0, false, true);
		}
	}

	if (owner) {
		if (owner->IsClient() && !owner->CastToClient()->GetFeigned()) {
			if(!owner->GetSpecialAbility(IMMUNE_AGGRO)) {
				hate_list.add(owner, 0, 0, false, !iBuffTic);
				if(owner->IsClient())
					owner->CastToClient()->AddAutoXTarget(this);
			}
		}
	}

	if (mypet && (!(GetAA(aaPetDiscipline) && mypet->IsHeld()))) {
		if(!mypet->IsFamiliar() && !mypet->GetSpecialAbility(IMMUNE_AGGRO))
			mypet->hate_list.add(other, 0, 0, bFrenzy);
	}
	
	else if (myowner) {
		if (myowner->IsAIControlled() && !myowner->GetSpecialAbility(IMMUNE_AGGRO))
			myowner->hate_list.add(other, 0, 0, bFrenzy);
	}
	
	if (!wasengaged) {
		if(IsNPC() && other->IsClient() && other->CastToClient())
			parse->EventNPC(EVENT_AGGRO, this->CastToNPC(), other, "", 0);
		AI_Event_Engaged(other, iYellForHelp);
	}
}

void Mob::DamageShield(Mob* attacker, bool spell_ds) {
	if(!attacker || this == attacker)
		return;

	int DS = 0;
	int rev_ds = 0;
	uint16 spellid = 0;

	if(!spell_ds) {
		DS = spellbonuses.DamageShield;
		rev_ds = attacker->spellbonuses.ReverseDamageShield;

		if(spellbonuses.DamageShieldSpellID != 0 && spellbonuses.DamageShieldSpellID != SPELL_UNKNOWN)
			spellid = spellbonuses.DamageShieldSpellID;
	}
	else {
		DS = spellbonuses.SpellDamageShield;
		rev_ds = 0;
		spellid = 2166;
	}

	if(DS == 0 && rev_ds == 0)
		return;

	mlog(COMBAT__HITS, "Applying Damage Shield of value %d to %s", DS, attacker->GetName());
	if(DS < 0) {
		if(!spell_ds)	{
			DS += aabonuses.DamageShield;
			DS -= itembonuses.DamageShield;
			if (attacker->IsOffHandAtk()) {
				int16 mitigation = (attacker->itembonuses.DSMitigationOffHand + attacker->spellbonuses.DSMitigationOffHand + attacker->aabonuses.DSMitigationOffHand);
				DS -= DS*mitigation/100;
			}
			DS -= (DS * attacker->itembonuses.DSMitigation / 100);
		}
		attacker->Damage(this, -DS, spellid, SkillAbjuration, false);
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
		CombatDamage_Struct* cds = (CombatDamage_Struct*)outapp->pBuffer;
		cds->target = attacker->GetID();
		cds->source = GetID();
		cds->type = spellbonuses.DamageShieldType;
		cds->spellid = 0x0;
		cds->damage = DS;
		entity_list.QueueCloseClients(this, outapp);
		safe_delete(outapp);
	}
	else if (DS > 0 && !spell_ds)
		attacker->HealDamage(DS);

	uint16 rev_ds_spell_id = SPELL_UNKNOWN;

	if(spellbonuses.ReverseDamageShieldSpellID != 0 && spellbonuses.ReverseDamageShieldSpellID != SPELL_UNKNOWN)
		rev_ds_spell_id = spellbonuses.ReverseDamageShieldSpellID;

	if(rev_ds < 0) {
		mlog(COMBAT__HITS, "Applying Reverse Damage Shield of value %d to %s", rev_ds, attacker->GetName());
		attacker->Damage(this, -rev_ds, rev_ds_spell_id, SkillAbjuration, false);
	}
}

uint8 Mob::GetWeaponDamageBonus(const Item_Struct *Weapon) {
	if(Weapon == nullptr || Weapon->ItemType == ItemType1HSlash || Weapon->ItemType == ItemType1HBlunt || Weapon->ItemType == ItemTypeMartial || Weapon->ItemType == ItemType1HPiercing)
		return (uint8) ((GetLevel() - 25) / 3);
		
	uint8 ucPlayerLevel = (uint8) GetLevel();
	
	if(Weapon->Delay <= 27)
		return (ucPlayerLevel - 22) / 3;

	if(ucPlayerLevel == 65 && Weapon->Delay <= 59) {
		static const uint8 ucLevel65DamageBonusesForDelays28to59[] = {35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40, 42, 42, 42, 45, 45, 47, 48, 49, 49, 51, 51, 52, 53, 54, 54, 56, 56, 57, 58, 59};
		return ucLevel65DamageBonusesForDelays28to59[Weapon->Delay-28];
	}


	if(ucPlayerLevel > 65) {
		if(ucPlayerLevel > 80)
			ucPlayerLevel = 80;

		if(Weapon->Delay <= 59) {
			static const uint8 ucDelay28to59Levels66to80[32][15] = {{36, 37, 38, 39, 41, 42, 43, 44, 45, 47, 49, 49, 49, 50, 53},
				{36, 38, 38, 39, 42, 43, 43, 45, 46, 48, 49, 50, 51, 52, 54},
				{37, 38, 39, 40, 43, 43, 44, 46, 47, 48, 50, 51, 52, 53, 55},
				{37, 39, 40, 40, 43, 44, 45, 46, 47, 49, 51, 52, 52, 52, 54},
				{38, 39, 40, 41, 44, 45, 45, 47, 48, 48, 50, 52, 53, 55, 57},
				{38, 40, 41, 41, 44, 45, 46, 48, 49, 50, 52, 53, 54, 56, 58},
				{39, 40, 41, 42, 45, 46, 47, 48, 49, 51, 53, 54, 55, 57, 58},
				{39, 41, 42, 43, 46, 46, 47, 49, 50, 52, 54, 55, 56, 57, 59},
				{40, 41, 42, 43, 46, 47, 48, 50, 51, 53, 55, 55, 56, 58, 60},
				{40, 42, 43, 44, 47, 48, 49, 50, 51, 53, 55, 56, 57, 59, 61},
				{41, 42, 43, 44, 47, 48, 49, 51, 52, 54, 56, 57, 58, 60, 62},
				{41, 43, 44, 45, 48, 49, 50, 52, 53, 55, 57, 58, 59, 61, 63},
				{43, 45, 46, 47, 50, 51, 52, 54, 55, 57, 59, 60, 61, 63, 65},
				{43, 45, 46, 47, 50, 51, 52, 54, 55, 57, 59, 60, 61, 63, 65},
				{44, 46, 47, 48, 51, 52, 53, 55, 56, 58, 60, 61, 62, 64, 66},
				{46, 48, 49, 50, 53, 54, 55, 58, 59, 61, 63, 64, 65, 67, 69},
				{47, 49, 50, 51, 54, 55, 56, 58, 59, 61, 64, 65, 66, 68, 70},
				{48, 50, 51, 52, 56, 57, 58, 60, 61, 63, 65, 66, 68, 70, 72},
				{50, 52, 53, 54, 57, 58, 59, 62, 63, 65, 67, 68, 69, 71, 74},
				{50, 52, 53, 55, 58, 59, 60, 62, 63, 66, 68, 69, 70, 72, 74},
				{51, 53, 54, 55, 58, 60, 61, 63, 64, 66, 69, 69, 71, 73, 75},
				{52, 54, 55, 57, 60, 61, 62, 65, 66, 68, 70, 71, 73, 75, 77},
				{53, 55, 56, 57, 61, 62, 63, 65, 67, 69, 71, 72, 74, 76, 78},
				{53, 55, 57, 58, 61, 62, 64, 66, 67, 69, 72, 73, 74, 77, 79},
				{55, 57, 58, 59, 63, 64, 65, 68, 69, 71, 74, 75, 76, 78, 81},
				{57, 55, 59, 60, 63, 65, 66, 68, 70, 72, 74, 76, 77, 79, 82},
				{56, 58, 59, 61, 64, 65, 67, 69, 70, 73, 75, 76, 78, 80, 82},
				{57, 59, 61, 62, 66, 67, 68, 71, 72, 74, 77, 78, 80, 82, 84},
				{58, 60, 61, 63, 66, 68, 69, 71, 73, 75, 78, 79, 80, 83, 85},
				{59, 61, 62, 64, 67, 69, 70, 72, 74, 76, 77, 78, 81, 84, 86},
				{60, 62, 63, 65, 68, 70, 71, 74, 75, 78, 80, 81, 83, 85, 88},
				{60, 62, 64, 65, 69, 70, 72, 74, 76, 78, 81, 82, 84, 86, 89}};

			return ucDelay28to59Levels66to80[Weapon->Delay-28][ucPlayerLevel-66];
		}
		else {
			const static uint8 ucDelayOver59Levels66to80[6][15] = {{61, 63, 65, 66, 70, 71, 73, 75, 77, 79, 82, 83, 85, 87, 90},
				{65, 68, 69, 71, 75, 76, 78, 80, 82, 85, 87, 89, 91, 93, 96},
				{70, 72, 74, 76, 80, 81, 83, 86, 88, 88, 90, 95, 97, 99, 102},
				{82, 85, 87, 89, 89, 94, 98, 101, 103, 106, 109, 111, 114, 117, 120},
				{90, 93, 96, 98, 103, 105, 107, 111, 113, 116, 120, 122, 125, 128, 131},
				{136, 140, 144, 148, 154, 157, 161, 166, 170, 174, 179, 183, 187, 191, 196}};

			if(Weapon->Delay < 65)
				return ucDelayOver59Levels66to80[0][ucPlayerLevel-66];
			else if(Weapon->Delay < 70)
				return ucDelayOver59Levels66to80[1][ucPlayerLevel-66];
			else if(Weapon->Delay < 85)
				return ucDelayOver59Levels66to80[2][ucPlayerLevel-66];
			else if(Weapon->Delay < 95)
				return ucDelayOver59Levels66to80[3][ucPlayerLevel-66];
			else if(Weapon->Delay < 150)
				return ucDelayOver59Levels66to80[4][ucPlayerLevel-66];
			else
				return ucDelayOver59Levels66to80[5][ucPlayerLevel-66];
		}
	}

	if(Weapon->Delay <= 39) {
		if( ucPlayerLevel <= 53) {
			static const uint8 ucDelay28to39LevelUnder54[] = {1, 1, 2, 3, 3, 3, 4, 5, 5, 6, 6, 6, 8, 8, 8, 9, 9, 10, 11, 11, 11, 12, 13, 14, 16, 17};
			return ucDelay28to39LevelUnder54[ucPlayerLevel-28];
		}
		else {
			static const uint8 ucDelay28to39Level54to64[12][11] = {{17, 21, 21, 23, 25, 26, 28, 30, 31, 31, 33},
				{17, 21, 22, 23, 25, 26, 29, 30, 31, 32, 34},
				{18, 21, 22, 23, 25, 27, 29, 31, 32, 32, 34},
				{18, 21, 22, 23, 25, 27, 29, 31, 32, 33, 34},
				{18, 21, 22, 24, 26, 27, 30, 32, 32, 33, 35},
				{18, 21, 22, 24, 26, 27, 30, 32, 33, 34, 35},
				{18, 22, 22, 24, 26, 28, 30, 32, 33, 34, 36},
				{18, 22, 23, 24, 26, 28, 31, 33, 34, 34, 36},
				{18, 22, 23, 25, 27, 28, 31, 33, 34, 35, 37},
				{18, 22, 23, 25, 27, 29, 31, 33, 34, 35, 37},
				{18, 22, 23, 25, 27, 29, 32, 34, 35, 36, 38},
				{18, 22, 23, 25, 27, 29, 32, 34, 35, 36, 38}};
			return ucDelay28to39Level54to64[Weapon->Delay-28][ucPlayerLevel-54];
		}
	}
	else if(Weapon->Delay <= 59) {
		if(ucPlayerLevel <= 52) {
			if(Weapon->Delay <= 45) {
				static const uint8 ucDelay40to45Levels28to52[6][25] = {{2,  2,  3,  4,  4,  4,  5,  6,  6,  7,  7,  7,  9,  9,  9,  10, 10, 11, 12, 12, 12, 13, 14, 16, 18},
					{2,  2,  3,  4,  4,  4,  5,  6,  6,  7,  7,  7,  9,  9,  9,  10, 10, 11, 12, 12, 12, 13, 14, 16, 18},
					{2,  2,  3,  4,  4,  4,  5,  6,  6,  7,  7,  7,  9,  9,  9,  10, 10, 11, 12, 12, 12, 13, 14, 16, 18},
					{4,  4,  5,  6,  6,  6,  7,  8,  8,  9,  9,  9,  11, 11, 11, 12, 12, 13, 14, 14, 14, 15, 16, 18, 20},
					{4,  4,  5,  6,  6,  6,  7,  8,  8,  9,  9,  9,  11, 11, 11, 12, 12, 13, 14, 14, 14, 15, 16, 18, 20},
					{5,  5,  6,  7,  7,  7,  8,  9,  9,  10, 10, 10, 12, 12, 12, 13, 13, 14, 15, 15, 15, 16, 17, 19, 21}};
				return ucDelay40to45Levels28to52[Weapon->Delay-40][ucPlayerLevel-28];
			}
			else {
				static const uint8 ucDelay46Levels28to52[] = {6, 6, 7, 8, 8, 8, 9, 10, 10, 11, 11, 11, 13, 13, 13, 14, 14, 15, 16, 16, 16, 17, 18, 20, 22};
				return ucDelay46Levels28to52[ucPlayerLevel-28] + ((Weapon->Delay-46) / 3);
			}
		}
		else {
			static const uint8 ucDelay40to59Levels53to64[20][37] = {{19, 20, 24, 25, 27, 29, 31, 34, 36, 37, 38, 40},
				{19, 20, 24, 25, 27, 29, 31, 34, 36, 37, 38, 40},
				{19, 20, 24, 25, 27, 29, 31, 34, 36, 37, 38, 40},
				{21, 22, 26, 27, 29, 31, 33, 37, 39, 40, 41, 43},
				{21, 22, 26, 27, 29, 32, 34, 37, 39, 40, 41, 43},
				{22, 23, 27, 28, 31, 33, 35, 38, 40, 42, 43, 45},
				{23, 24, 28, 30, 32, 34, 36, 40, 42, 43, 44, 46},
				{23, 24, 29, 30, 32, 34, 37, 40, 42, 43, 44, 47},
				{23, 24, 29, 30, 32, 35, 37, 40, 43, 44, 45, 47},
				{24, 25, 30, 31, 34, 36, 38, 42, 44, 45, 46, 49},
				{24, 26, 30, 31, 34, 36, 39, 42, 44, 46, 47, 49},
				{24, 26, 30, 31, 34, 36, 39, 42, 45, 46, 47, 49},
				{25, 27, 31, 33, 35, 38, 40, 44, 46, 47, 49, 51},
				{25, 27, 31, 33, 35, 38, 40, 44, 46, 48, 49, 51},
				{26, 27, 32, 33, 36, 38, 41, 44, 47, 48, 49, 52},
				{27, 28, 33, 34, 37, 39, 42, 46, 48, 50, 51, 53},
				{27, 28, 33, 34, 37, 40, 42, 46, 49, 50, 51, 54},
				{27, 28, 33, 34, 37, 40, 43, 46, 49, 50, 52, 54},
				{28, 29, 34, 36, 39, 41, 44, 48, 50, 52, 53, 56},
				{28, 29, 34, 36, 39, 41, 44, 48, 51, 52, 54, 56}};
			return ucDelay40to59Levels53to64[Weapon->Delay-40][ucPlayerLevel-53];
		}
	}
	else {
		static const uint8 ucDelayOver59Levels28to65[6][38] = {{10, 10, 11, 12, 12, 12, 13, 14, 14, 15, 15, 15, 17, 17, 17, 18, 18, 19, 20, 20, 20, 21, 22, 24, 27, 28, 30, 35, 36, 39, 42, 45, 49, 51, 53, 54, 57, 59},
			{12, 12, 13, 14, 14, 14, 15, 16, 16, 17, 17, 17, 19, 19, 19, 20, 20, 21, 22, 22, 22, 23, 24, 26, 29, 30, 32, 37, 39, 42, 45, 48, 52, 55, 57, 58, 61, 63},
			{14, 14, 15, 16, 16, 16, 17, 18, 18, 19, 19, 19, 21, 21, 21, 22, 22, 23, 24, 24, 24, 25, 26, 28, 31, 33, 35, 40, 42, 45, 48, 52, 56, 59, 61, 62, 65, 68},
			{19, 19, 20, 21, 21, 21, 22, 23, 23, 24, 24, 24, 26, 26, 26, 27, 27, 28, 29, 29, 29, 30, 31, 34, 37, 39, 41, 47, 49, 54, 57, 61, 66, 69, 72, 74, 77, 80},
			{22, 22, 23, 24, 24, 24, 25, 26, 26, 27, 27, 27, 29, 29, 29, 30, 30, 31, 32, 32, 32, 33, 34, 37, 40, 43, 45, 52, 54, 59, 62, 67, 73, 76, 79, 81, 84, 88},
			{40, 40, 41, 42, 42, 42, 43, 44, 44, 45, 45, 45, 47, 47, 47, 48, 48, 49, 50, 50, 50, 51, 52, 56, 61, 65, 69, 78, 82, 89, 94, 102, 110, 115, 119, 122, 127, 132}};

		if(Weapon->Delay < 65)
			return ucDelayOver59Levels28to65[0][ucPlayerLevel-28];
		else if(Weapon->Delay < 70)
			return ucDelayOver59Levels28to65[1][ucPlayerLevel-28];
		else if(Weapon->Delay < 85)
			return ucDelayOver59Levels28to65[2][ucPlayerLevel-28];
		else if(Weapon->Delay < 95)
			return ucDelayOver59Levels28to65[3][ucPlayerLevel-28];
		else if(Weapon->Delay < 150)
			return ucDelayOver59Levels28to65[4][ucPlayerLevel-28];
		else
			return ucDelayOver59Levels28to65[5][ucPlayerLevel-28];
	}
}

int Mob::GetMonkHandToHandDamage(void) {
	static int damage[66] = {99, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15};
	if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652)
		return(9);
	else {
		int Level = GetLevel();
		if (Level > 65)
			return(19);
		else
			return damage[Level];
	}
}

int Mob::GetMonkHandToHandDelay(void) {
	static int delayshuman[66] = {99,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,35,35,35,35,35,34,34,34,34,34,33,33,33,33,33,32,32,32,32,32,31,31,31,31,31,30,30,30,29,29,29,28,28,28,27,26,24,22,20,20,20};
	static int delaysiksar[66] = {99,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,35,35,35,35,35,34,34,34,34,34,33,33,33,33,33,32,32,32,32,32,31,31,31,30,30,30,29,29,29,28,27,24,22,20,20,20};

	if (IsClient() && CastToClient()->GetItemIDAt(12) == 10652)
		return(16);
	else {
		int Level = GetLevel();
		if (GetRace() == HUMAN) {
			if (Level > 65)
				return(24);
			else
				return delayshuman[Level];
		}
		else {
			if (Level > 65)
				return(25);
			else
				return delaysiksar[Level];
		}
	}
}


int32 Mob::ReduceDamage(int32 damage) {
	if(damage <= 0)
		return damage;

	int32 slot = -1;
	bool DisableMeleeRune = false;

	if (spellbonuses.NegateAttacks[0]){
		slot = spellbonuses.NegateAttacks[1];
		if(slot >= 0) {
			if(--buffs[slot].numhits == 0) {				
				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot , true);
			}
			return -6;
		}
	}

	if (spellbonuses.MeleeThresholdGuard[0]){
		slot = spellbonuses.MeleeThresholdGuard[1];		
		if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[2])) {
			DisableMeleeRune = true;
			int damage_to_reduce = damage * spellbonuses.MeleeThresholdGuard[0] / 100;
			if(damage_to_reduce > buffs[slot].melee_rune) {
				mlog(SPELLS__EFFECT_VALUES, "Mob::ReduceDamage SE_MeleeThresholdGuard %d damage negated, %d damage remaining, fading buff.", damage_to_reduce, buffs[slot].melee_rune);
				damage -= damage_to_reduce;
				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else {
				mlog(SPELLS__EFFECT_VALUES, "Mob::ReduceDamage SE_MeleeThresholdGuard %d damage negated, %d damage remaining.", damage_to_reduce, buffs[slot].melee_rune);
				buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);
				damage -= damage_to_reduce;
			}
		}
	}


	if (spellbonuses.MitigateMeleeRune[0] && !DisableMeleeRune) {
		slot = spellbonuses.MitigateMeleeRune[1];
		if(slot >= 0) {
			int damage_to_reduce = (damage * spellbonuses.MitigateMeleeRune[0] / 100);
			if(damage_to_reduce > buffs[slot].melee_rune) {
				mlog(SPELLS__EFFECT_VALUES, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d damage remaining, fading buff.", damage_to_reduce, buffs[slot].melee_rune);
				damage -= damage_to_reduce;
				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else {
				mlog(SPELLS__EFFECT_VALUES, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d damage remaining.", damage_to_reduce, buffs[slot].melee_rune);
				buffs[slot].melee_rune = (buffs[slot].melee_rune - damage_to_reduce);
				damage -= damage_to_reduce;
			}
		}
	}

	if (spellbonuses.TriggerMeleeThreshold[2]) {
		slot = spellbonuses.TriggerMeleeThreshold[1];		
		if (slot >= 0) {
			if(damage > buffs[slot].melee_rune)	{
				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot);
			}
			else
				buffs[slot].melee_rune = (buffs[slot].melee_rune - damage);
		}
	}

	if(damage < 1)
		return -6;

	if (spellbonuses.MeleeRune[0] && spellbonuses.MeleeRune[1] >= 0)
		damage = RuneAbsorb(damage, SE_Rune);

	if(damage < 1)
		return -6;

	return(damage);
}

int32 Mob::AffectMagicalDamage(int32 damage, uint16 spell_id, const bool iBuffTic, Mob* attacker) {
	if(damage <= 0)
		return damage;

	bool DisableSpellRune = false;
	int32 slot = -1;

	if (spellbonuses.NegateAttacks[0]){
		slot = spellbonuses.NegateAttacks[1];
		if(slot >= 0) {
			if(--buffs[slot].numhits == 0) {				
				if(!TryFadeEffect(slot))
					BuffFadeBySlot(slot , true);
			}
			return 0;
		}
	}

	if(iBuffTic) {
 		damage -= (damage * itembonuses.DoTShielding / 100);
 
		if (spellbonuses.MitigateDotRune[0]){
			slot = spellbonuses.MitigateDotRune[1];
			if(slot >= 0) {
				int damage_to_reduce = (damage * spellbonuses.MitigateDotRune[0] / 100);
				if(damage_to_reduce > buffs[slot].dot_rune)	{
					damage -= damage_to_reduce;
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else {
					buffs[slot].dot_rune = (buffs[slot].dot_rune - damage_to_reduce);
					damage -= damage_to_reduce;
				}
			}
		}
	}

	else {
		damage -= (damage * itembonuses.SpellShield / 100);

		if (spellbonuses.SpellThresholdGuard[0]){
			slot = spellbonuses.SpellThresholdGuard[1];
		
			if (slot >= 0 && (damage > spellbonuses.MeleeThresholdGuard[2])) {
				DisableSpellRune = true;
				int damage_to_reduce = (damage * spellbonuses.SpellThresholdGuard[0] / 100);
				if(damage_to_reduce > buffs[slot].magic_rune) {
					damage -= damage_to_reduce;
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else {
					buffs[slot].melee_rune = (buffs[slot].magic_rune - damage_to_reduce);
					damage -= damage_to_reduce;
				}
			}
		}
		
		if (spellbonuses.MitigateSpellRune[0] && !DisableSpellRune) {
			slot = spellbonuses.MitigateSpellRune[1];
			if(slot >= 0) {
				int damage_to_reduce = damage * spellbonuses.MitigateSpellRune[0] / 100;
				if(damage_to_reduce > buffs[slot].magic_rune) {
					mlog(SPELLS__EFFECT_VALUES, "Mob::ReduceDamage SE_MitigateSpellDamage %d damage negated, %d damage remaining, fading buff.", damage_to_reduce, buffs[slot].magic_rune);
					damage -= damage_to_reduce;
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else {
					mlog(SPELLS__EFFECT_VALUES, "Mob::ReduceDamage SE_MitigateMeleeDamage %d damage negated, %d damage remaining.", damage_to_reduce, buffs[slot].magic_rune);
					buffs[slot].magic_rune = (buffs[slot].magic_rune - damage_to_reduce);
					damage -= damage_to_reduce;
				}
			}
		}

		if (spellbonuses.TriggerSpellThreshold[2]){
			slot = spellbonuses.TriggerSpellThreshold[1];		
			if (slot >= 0) {
				if(damage > buffs[slot].magic_rune)	{
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
				else
					buffs[slot].magic_rune = (buffs[slot].magic_rune - damage);
			}
		}

		if(damage < 1)
			return 0;

		if (spellbonuses.AbsorbMagicAtt[0] && spellbonuses.AbsorbMagicAtt[1] >= 0)
			damage = RuneAbsorb(damage, SE_AbsorbMagicAtt);

		if(damage < 1)
			return 0;
	}
	return damage;
}

int32 Mob::ReduceAllDamage(int32 damage) {
	if(damage <= 0)
		return damage;

	if(spellbonuses.ManaAbsorbPercentDamage[0] && (GetMana() > damage * spellbonuses.ManaAbsorbPercentDamage[0] / 100)) {
		damage -= (damage * spellbonuses.ManaAbsorbPercentDamage[0] / 100);
		SetMana(GetMana() - damage);
		TryTriggerOnValueAmount(false, true);
	}
	
	CheckNumHitsRemaining(8);

	return(damage);
}

bool Mob::HasProcs() const {
	for (int i = 0; i < MAX_PROCS; i++)
		if (PermaProcs[i].spellID != SPELL_UNKNOWN || SpellProcs[i].spellID != SPELL_UNKNOWN)
			return true;
	return false;
}

bool Mob::HasDefensiveProcs() const {
	for (int i = 0; i < MAX_PROCS; i++)
		if (DefensiveProcs[i].spellID != SPELL_UNKNOWN)
			return true;
	return false;
}

bool Mob::HasSkillProcs() const {
	for (int i = 0; i < MAX_PROCS; i++)
		if (SkillProcs[i].spellID != SPELL_UNKNOWN)
			return true;
	return false;
}

bool Mob::HasRangedProcs() const {
	for (int i = 0; i < MAX_PROCS; i++)
		if (RangedProcs[i].spellID != SPELL_UNKNOWN)
			return true;
	return false;
}

bool Client::CheckDoubleAttack(bool tripleAttack) {
	uint16 bonusGiveDA = (aabonuses.GiveDoubleAttack + spellbonuses.GiveDoubleAttack + itembonuses.GiveDoubleAttack);

	if(!HasSkill(SkillDoubleAttack) && !bonusGiveDA)
		return false;

	float chance = 0.0f;
	uint16 skill = GetSkill(SkillDoubleAttack);
	int16 bonusDA = (aabonuses.DoubleAttackChance + spellbonuses.DoubleAttackChance + itembonuses.DoubleAttackChance);

	if (skill)
		chance = ((float(skill+GetLevel()) * (float(100.0f + bonusDA+bonusGiveDA) / 100.0f)) / 500.0f);
	else
		chance = ((float(bonusGiveDA) * (float(100.0f + bonusDA) / 100.0f) ) / 100.0f);
		
	if(tripleAttack) {
		int16 triple_bonus = (spellbonuses.TripleAttackChance + itembonuses.TripleAttackChance);
		chance *= 0.2f;
		chance *= (float(100.0f + triple_bonus) / 100.0f);
	}

	if((MakeRandomFloat(0, 1) < chance))
		return true;

	return false;
}

bool Client::CheckDoubleRangedAttack() {		
	int16 chance = spellbonuses.DoubleRangedAttack + itembonuses.DoubleRangedAttack + aabonuses.DoubleRangedAttack;
	if(chance && (MakeRandomInt(0, 100) < chance))
		return true;

	return false;
}

void Mob::CommonDamage(Mob* attacker, int32 &damage, const uint16 spell_id, const SkillUseTypes skill_used, bool &avoidable, const int8 buffslot, const bool iBuffTic) {
	bool FromDamageShield = (skill_used == SkillAbjuration);

	mlog(COMBAT__HITS, "Applying damage %d done by %s with skill %d and spell %d, avoidable? %s, is %sa buff tic in slot %d", damage, attacker?attacker->GetName():"NOBODY", skill_used, spell_id, avoidable?"yes":"no", iBuffTic?"":"not ", buffslot);

	if (GetInvul() || DivineAura()) {
		mlog(COMBAT__DAMAGE, "Avoiding %d damage due to invulnerability.", damage);
		damage = -5;
	}
	
	if( spell_id != SPELL_UNKNOWN || attacker == nullptr )
		avoidable = false;
		
	if (attacker && damage > 0 && spell_id == SPELL_UNKNOWN && skill_used != SkillArchery && skill_used != SkillThrowing)
		DamageShield(attacker);

	if (spell_id == SPELL_UNKNOWN && skill_used) {
		CheckNumHitsRemaining(1);
		if (attacker)
			attacker->CheckNumHitsRemaining(2);
	}

	if(attacker) {
		if(attacker->IsClient()) {
			if(!RuleB(Combat, EXPFromDmgShield)) {
				if(!attacker->CastToClient()->GetFeigned() && !FromDamageShield)
					AddToHateList(attacker, 0, damage, true, false, iBuffTic);
			}
			else {
				if(!attacker->CastToClient()->GetFeigned())
					AddToHateList(attacker, 0, damage, true, false, iBuffTic);
			}
		}
		else
			AddToHateList(attacker, 0, damage, true, false, iBuffTic);
	}

	if(damage > 0) {
		if(attacker) {
			if(spell_id == SPELL_HARM_TOUCH2 && attacker->IsClient() && attacker->CastToClient()->CheckAAEffect(aaEffectLeechTouch)){
				int healed = damage;
				healed = attacker->GetActSpellHealing(spell_id, healed);
				attacker->HealDamage(healed);
				entity_list.MessageClose(this, true, 300, MT_Emote, "%s beams a smile at %s", attacker->GetCleanName(), this->GetCleanName() );
				attacker->CastToClient()->DisableAAEffect(aaEffectLeechTouch);
			}

			if (spell_id != SPELL_UNKNOWN && IsLifetapSpell( spell_id )) {
				int healed = damage;
				healed = attacker->GetActSpellHealing(spell_id, healed);
				mlog(COMBAT__DAMAGE, "Applying lifetap heal of %d to %s", healed, attacker->GetName());
				attacker->HealDamage(healed);
				entity_list.MessageClose(this, true, 300, MT_Emote, "%s beams a smile at %s", attacker->GetCleanName(), this->GetCleanName() );
			}
		}

		Mob *pet = GetPet();
		if (pet && !pet->IsFamiliar() && !pet->GetSpecialAbility(IMMUNE_AGGRO) && !pet->IsEngaged() && attacker && attacker != this && !attacker->IsCorpse()) {
			if (!pet->IsHeld()) {
				mlog(PETS__AGGRO, "Sending pet %s into battle due to attack.", pet->GetName());
				pet->AddToHateList(attacker, 1);
				pet->SetTarget(attacker);
				Message_StringID(10, PET_ATTACKING, pet->GetCleanName(), attacker->GetCleanName());
			}
		}

		if(spell_id == SPELL_UNKNOWN) {
			damage = ReduceDamage(damage);
			mlog(COMBAT__HITS, "Melee Damage reduced to %d", damage);
		}
		else {
			int32 origdmg = damage;
			damage = AffectMagicalDamage(damage, spell_id, iBuffTic, attacker);
			if (origdmg != damage && attacker && attacker->IsClient()) {
				if(attacker->CastToClient()->GetFilter(FilterDamageShields) != FilterHide)
					attacker->Message(15, "The Spellshield absorbed %d of %d points of damage", origdmg - damage, origdmg);
			}
			if (damage == 0 && attacker && origdmg != damage && IsClient())
				Message(263, "%s tries to cast on YOU, but YOUR magical skin absorbs the spell.",attacker->GetCleanName());
		}

		if (skill_used)
			CheckNumHitsRemaining(6);

		ReduceAllDamage(damage);

		if(IsClient() && CastToClient()->sneaking) {
			CastToClient()->sneaking = false;
			SendAppearancePacket(AT_Sneak, 0);
		}
		if(attacker && attacker->IsClient() && attacker->CastToClient()->sneaking) {
			attacker->CastToClient()->sneaking = false;
			attacker->SendAppearancePacket(AT_Sneak, 0);
		}

		SetHP(GetHP() - damage);
		if(HasDied()) {
			bool IsSaved = false;
			if(TryDivineSave())
				IsSaved = true;

			if(!IsSaved && !TrySpellOnDeath()) {
				SetHP(-500);
				if(Death(attacker, damage, spell_id, skill_used))
					return;
			}
		}
		else {
			if(GetHPRatio() < 16)
				TryDeathSave();
		}
		
		TryTriggerOnValueAmount(true);
		
		if (IsMezzed()) {
			mlog(COMBAT__HITS, "Breaking mez due to attack.");
			entity_list.MessageClose_StringID(this, true, 100, MT_WornOff, HAS_BEEN_AWAKENED, GetCleanName(), attacker->GetCleanName());
			BuffFadeByEffect(SE_Mez);
		}

		if (damage > 0 && ((skill_used == SkillBash || skill_used == SkillKick) && attacker)) {
			int stun_chance = 0;
			if (!GetSpecialAbility(UNSTUNABLE)) {
				if (attacker->IsNPC())
					stun_chance = RuleI(Combat, NPCBashKickStunChance);
				else if (attacker->IsClient()) {
					if (!IsClient() && GetLevel() <= RuleI(Spells, BaseImmunityLevel)) {
						if (skill_used == SkillBash)
							stun_chance = 100;
						else if (attacker->GetLevel() >= RuleI(Combat, ClientStunLevel))
							stun_chance = 100;
					}
					else {
						if (skill_used == SkillKick && attacker->GetLevel() < RuleI(Combat, ClientStunLevel))
							stun_chance = RuleI(Combat, NPCBashKickStunChance);
						else if (skill_used == SkillBash)
							stun_chance = (RuleI(Combat, NPCBashKickStunChance) + attacker->spellbonuses.StunBashChance + attacker->itembonuses.StunBashChance + attacker->aabonuses.StunBashChance);
					}
				}
			}

			if (stun_chance && MakeRandomInt(0, 99) < stun_chance) {
				int stun_resist = (itembonuses.StunResist + spellbonuses.StunResist);
				int frontal_stun_resist = (itembonuses.FrontalStunResist + spellbonuses.FrontalStunResist);

				mlog(COMBAT__HITS, "Stun passed, checking resists. Was %d chance.", stun_chance);
				if (IsClient()) {
					stun_resist += aabonuses.StunResist;
					frontal_stun_resist += aabonuses.FrontalStunResist;
				}

				if (((GetBaseRace() == OGRE && IsClient()) || (frontal_stun_resist && MakeRandomInt(0, 99) < frontal_stun_resist)) && !attacker->BehindMob(this, attacker->GetX(), attacker->GetY()))
					mlog(COMBAT__HITS, "Frontal stun resisted. %d chance.", frontal_stun_resist);
				else {
					if (stun_resist && MakeRandomInt(0, 99) < stun_resist) {
						if (IsClient())
							Message_StringID(MT_Stun, SHAKE_OFF_STUN);
						mlog(COMBAT__HITS, "Stun Resisted. %d chance.", stun_resist);
					}
					else {
						mlog(COMBAT__HITS, "Stunned. %d resist chance.", stun_resist);
						Stun(MakeRandomInt(0, 2) * 1000);
					}
				}
			}
			else
				mlog(COMBAT__HITS, "Stun failed. %d chance.", stun_chance);
		}

		if(spell_id != SPELL_UNKNOWN && !iBuffTic) {
			if (IsRooted() && !FromDamageShield)
				TryRootFadeByDamage(buffslot, attacker);
		}
		else if(spell_id == SPELL_UNKNOWN) {
			if(IsCasting()) {
				attacked_count++;
				mlog(COMBAT__HITS, "Melee attack while casting. Attack count %d", attacked_count);
			}
		}

		if(GetHP() < GetMaxHP())
			SendHPUpdate();
	}

	if(!iBuffTic) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Damage, sizeof(CombatDamage_Struct));
		CombatDamage_Struct* a = (CombatDamage_Struct*)outapp->pBuffer;
		a->target = GetID();
		if (attacker == nullptr)
			a->source = 0;
		else if (attacker->IsClient() && attacker->CastToClient()->GMHideMe())
			a->source = 0;
		else
			a->source = attacker->GetID();
		a->type = SkillDamageTypes[skill_used];
		a->damage = damage;
		a->spellid = spell_id;

		eqFilterType filter;
		Mob *skip = attacker;
		if(attacker && attacker->GetOwnerID()) {
			Mob* owner = attacker->GetOwner();
			if (owner && owner->IsClient()) {
				if (((spell_id != SPELL_UNKNOWN) || (FromDamageShield)) && damage > 0) {
					char val1[20]={0};
					owner->Message_StringID(MT_NonMelee,OTHER_HIT_NONMELEE,GetCleanName(),ConvertArray(damage,val1));
				}
				else {
					if(damage > 0) {
						if(spell_id != SPELL_UNKNOWN)
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						else
							filter = FilterPetHits;
					}
					else if(damage == -5)
						filter = FilterNone;
					else
						filter = FilterPetMisses;

					if(!FromDamageShield)
						owner->CastToClient()->QueuePacket(outapp,true,CLIENT_CONNECTED,filter);
				}
			}
			skip = owner;
		}
		else {
			if(attacker && attacker->IsClient()) {
				if (((spell_id != SPELL_UNKNOWN)||(FromDamageShield)) && damage > 0) {
					char val1[20]={0};
						if (FromDamageShield) {
							if(!attacker->CastToClient()->GetFilter(FilterDamageShields) == FilterHide)
								attacker->Message_StringID(MT_DS,OTHER_HIT_NONMELEE,GetCleanName(),ConvertArray(damage,val1));
						}
						else
							entity_list.MessageClose_StringID(this, true, 100, MT_NonMelee,HIT_NON_MELEE,attacker->GetCleanName(),GetCleanName(),ConvertArray(damage,val1));
				}
				else {
					if(damage > 0) {
						if(spell_id != SPELL_UNKNOWN)
							filter = iBuffTic ? FilterDOT : FilterSpellDamage;
						else
							filter = FilterNone;
					} else if(damage == -5)
						filter = FilterNone;
					else
						filter = FilterMyMisses;

					attacker->CastToClient()->QueuePacket(outapp, true, CLIENT_CONNECTED, filter);
				}
			}
			skip = attacker;
		}

		if(damage > 0) {
			if(spell_id != SPELL_UNKNOWN)
				filter = iBuffTic ? FilterDOT : FilterSpellDamage;
			else
				filter = FilterOthersHit;
		}
		else if(damage == -5)
			filter = FilterNone;
		else
			filter = FilterOthersMiss;
			
		if(!FromDamageShield) {
			entity_list.QueueCloseClients(this, outapp, true, 200, skip, true, filter);
			if(IsClient())
				CastToClient()->QueuePacket(outapp);
		}

		safe_delete(outapp);
	} else {
		if(spell_id != SPELL_UNKNOWN && damage > 0 && attacker && attacker != this && attacker->IsClient()) {
			attacker->FilteredMessage_StringID(attacker, MT_DoTDamage, FilterDOT, YOUR_HIT_DOT, GetCleanName(), itoa(damage), spells[spell_id].name);
			entity_list.FilteredMessageClose_StringID(attacker, true, 200, MT_DoTDamage, FilterDOT, OTHER_HIT_DOT, GetCleanName(), itoa(damage), attacker->GetCleanName(), spells[spell_id].name);
		}
	}
}

void Mob::HealDamage(uint32 amount, Mob *caster, uint16 spell_id) {
	int32 maxhp = GetMaxHP();
	int32 curhp = GetHP();
	uint32 acthealed = 0;

	if (caster && amount > 0) {
		if (caster->IsNPC() && !caster->IsPet()) {
			float npchealscale = caster->CastToNPC()->GetHealScale();
			amount = ((static_cast<float>(amount) * npchealscale) / 100.0f);
		}
	}

	if (amount > (maxhp - curhp))
		acthealed = (maxhp - curhp);
	else
		acthealed = amount;

	if (acthealed > 100) {
		if (caster) {
			if (IsBuffSpell(spell_id)) {
				if (caster->IsClient() && caster == this) {
					if (caster->CastToClient()->GetClientVersionBit() & BIT_SoFAndLater)
						FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime, HOT_HEAL_SELF, itoa(acthealed), spells[spell_id].name);
					else
						FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime, YOU_HEALED, GetCleanName(), itoa(acthealed));
				}
				else if (caster->IsClient() && caster != this) {
					if (caster->CastToClient()->GetClientVersionBit() & BIT_SoFAndLater)
						caster->FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime, HOT_HEAL_OTHER, GetCleanName(), itoa(acthealed), spells[spell_id].name);
					else
						caster->FilteredMessage_StringID(caster, MT_NonMelee, FilterHealOverTime, YOU_HEAL, GetCleanName(), itoa(acthealed));
				}
				
				if (IsClient() && caster != this) {
					if (CastToClient()->GetClientVersionBit() & BIT_SoFAndLater)
						FilteredMessage_StringID(this, MT_NonMelee, FilterHealOverTime, HOT_HEALED_OTHER, caster->GetCleanName(), itoa(acthealed), spells[spell_id].name);
					else
						FilteredMessage_StringID(this, MT_NonMelee, FilterHealOverTime, YOU_HEALED, caster->GetCleanName(), itoa(acthealed));
				}
			}
			else {
				FilteredMessage_StringID(caster, MT_NonMelee, FilterSpellDamage, YOU_HEALED, caster->GetCleanName(), itoa(acthealed));
				if (caster != this)
					caster->FilteredMessage_StringID(caster, MT_NonMelee, FilterSpellDamage, YOU_HEAL, GetCleanName(), itoa(acthealed));
			}
		}
		else
			Message(MT_NonMelee, "You have been healed for %d points of damage.", acthealed);
	}

	if (curhp < maxhp) {
		if ((curhp + amount) > maxhp)
			curhp = maxhp;
		else
			curhp += amount;
		SetHP(curhp);

		SendHPUpdate();
	}
}

float Mob::GetProcChances(float ProcBonus, uint16 weapon_speed, uint16 hand) {
	int mydex = GetDEX();
	float ProcChance = 0.0f;

	switch (hand) {
		case 13:
			weapon_speed = attack_timer.GetDuration();
			break;
		case 14:
			weapon_speed = attack_dw_timer.GetDuration();
			break;
		case 11:
			weapon_speed = ranged_timer.GetDuration();
			break;
	}

	if (weapon_speed < RuleI(Combat, MinHastedDelay))
		weapon_speed = RuleI(Combat, MinHastedDelay);

	if (RuleB(Combat, AdjustProcPerMinute)) {
		ProcChance = (static_cast<float>(weapon_speed) * RuleR(Combat, AvgProcsPerMinute) / 60000.0f);
		ProcBonus += (static_cast<float>(mydex) * RuleR(Combat, ProcPerMinDexContrib));
		ProcChance += (ProcChance * ProcBonus / 100.0f);
	} else {
		ProcChance = (RuleR(Combat, BaseProcChance) + static_cast<float>(mydex) / RuleR(Combat, ProcDexDivideBy));
		ProcChance += (ProcChance * ProcBonus / 100.0f);
	}

	mlog(COMBAT__PROCS, "Proc chance %.2f (%.2f from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

float Mob::GetDefensiveProcChances(float &ProcBonus, float &ProcChance, uint16 weapon_speed, uint16 hand) {
	int myagi = GetAGI();
	ProcBonus = 0;
	ProcChance = 0;

	switch(hand){
		case 13:
			weapon_speed = attack_timer.GetDuration();
			break;
		case 14:
			weapon_speed = attack_dw_timer.GetDuration();
			break;
		case 11:
			return 0;
			break;
	}

	if(weapon_speed < RuleI(Combat, MinHastedDelay))
		weapon_speed = RuleI(Combat, MinHastedDelay);

	ProcChance = ((float)weapon_speed * RuleR(Combat, AvgDefProcsPerMinute) / 60000.0f);
	ProcBonus += (float(myagi) * RuleR(Combat, DefProcPerMinAgiContrib) / 100.0f);
	ProcChance = (ProcChance + (ProcChance * ProcBonus));

	mlog(COMBAT__PROCS, "Defensive Proc chance %.2f (%.2f from bonuses)", ProcChance, ProcBonus);
	return ProcChance;
}

void Mob::TryDefensiveProc(const ItemInst* weapon, Mob *on, uint16 hand, int damage) {

	if (!on) {
		SetTarget(nullptr);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to Mob::TryDefensiveProc for evaluation!");
		return;
	}

	bool bSkillProc = HasSkillProcs();
	bool bDefensiveProc = HasDefensiveProcs();

	if (!bDefensiveProc && !bSkillProc)
		return;

	if (!bDefensiveProc && (bSkillProc && damage >= 0))
		return;

	float ProcChance, ProcBonus;
	if(weapon!=nullptr)
		on->GetDefensiveProcChances(ProcBonus, ProcChance, weapon->GetItem()->Delay, hand);
	else
		on->GetDefensiveProcChances(ProcBonus, ProcChance);
	if(hand != 13)
		ProcChance /= 2;

	if (bDefensiveProc){
		for (int i = 0; i < MAX_PROCS; i++) {
			if (DefensiveProcs[i].spellID != SPELL_UNKNOWN) {
				int chance = ProcChance * (DefensiveProcs[i].chance);
				if ((MakeRandomInt(0, 100) < chance)) {
					ExecWeaponProc(nullptr, DefensiveProcs[i].spellID, on);
					CheckNumHitsRemaining(10,0,DefensiveProcs[i].base_spellID);
				}
			}
		}
	}
	if (bSkillProc && damage < 0){
		if (damage == -1)
			TrySkillProc(on, SkillBlock, ProcChance);
		else if (damage == -2)
			TrySkillProc(on, SkillParry, ProcChance);
		else if (damage == -3)
			TrySkillProc(on, SkillRiposte, ProcChance);
		else if (damage == -4)
			TrySkillProc(on, SkillDodge, ProcChance);
	}
}

void Mob::TryWeaponProc(const ItemInst* weapon_g, Mob *on, uint16 hand) {
	if(!on) {
		SetTarget(nullptr);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to Mob::TryWeaponProc for evaluation!");
		return;
	}

	if (!IsAttackAllowed(on)) {
		mlog(COMBAT__PROCS, "Preventing procing off of unattackable things.");
		return;
	}

	if(!weapon_g) {
		TrySpellProc(nullptr, (const Item_Struct*)nullptr, on);
		return;
	}

	if(!weapon_g->IsType(ItemClassCommon)) {
		TrySpellProc(nullptr, (const Item_Struct*)nullptr, on);
		return;
	}

	TryWeaponProc(weapon_g, weapon_g->GetItem(), on, hand);
	TrySpellProc(weapon_g, weapon_g->GetItem(), on, hand);
	return;
}

void Mob::TryWeaponProc(const ItemInst *inst, const Item_Struct *weapon, Mob *on, uint16 hand) {
	if (!weapon)
		return;
	uint16 skillinuse = 28;
	int ourlevel = GetLevel();
	float ProcBonus = (static_cast<float>(aabonuses.ProcChanceSPA + spellbonuses.ProcChanceSPA + itembonuses.ProcChanceSPA));
	ProcBonus += (static_cast<float>(itembonuses.ProcChance) / 10.0f);
	float ProcChance = GetProcChances(ProcBonus, weapon->Delay, hand);

	if (hand != 13)
		ProcChance /= 2;
		
	bool proced = false;
	skillinuse = GetSkillByItemType(weapon->ItemType);
	if (weapon->Proc.Type == ET_CombatProc) {
		float WPC = (ProcChance * (100.0f + static_cast<float>(weapon->ProcRate)) / 100.0f);
		if (MakeRandomFloat(0, 1) <= WPC) {
			if (weapon->Proc.Level > ourlevel) {
				mlog(COMBAT__PROCS, "Tried to proc (%s), but our level (%d) is lower than required (%d)", weapon->Name, ourlevel, weapon->Proc.Level);
				if (IsPet()) {
					Mob *own = GetOwner();
					if (own)
						own->Message_StringID(13, PROC_PETTOOLOW);
				}
				else
					Message_StringID(13, PROC_TOOLOW);
			}
			else {
				mlog(COMBAT__PROCS, "Attacking weapon (%s) successfully procing spell %d (%.2f percent chance)", weapon->Name, weapon->Proc.Effect, WPC * 100);
				ExecWeaponProc(inst, weapon->Proc.Effect, on);
				proced = true;
			}
		}
	}

	if(!RuleB(Combat, OneProcPerWeapon))
		proced = false;

	if (!proced && inst) {
		for (int r = 0; r < MAX_AUGMENT_SLOTS; r++) {
			const ItemInst *aug_i = inst->GetAugment(r);
			if (!aug_i)
				continue;
			const Item_Struct *aug = aug_i->GetItem();
			if (!aug)
				continue;

			if (aug->Proc.Type == ET_CombatProc) {
				float APC = (ProcChance * (100.0f + static_cast<float>(aug->ProcRate)) / 100.0f);
				if (MakeRandomFloat(0, 1) <= APC) {
					if (aug->Proc.Level > ourlevel) {
						if (IsPet()) {
							Mob *own = GetOwner();
							if (own)
								own->Message_StringID(13, PROC_PETTOOLOW);
						}
						else
							Message_StringID(13, PROC_TOOLOW);
					}
					else {
						ExecWeaponProc(aug_i, aug->Proc.Effect, on);
						if (RuleB(Combat, OneProcPerWeapon))
							break;
					}
				}
			}
		}
	}
	
	if (HasSkillProcs())
		TrySkillProc(on, skillinuse, ProcChance);

	return;
}

void Mob::TrySpellProc(const ItemInst *inst, const Item_Struct *weapon, Mob *on, uint16 hand) {
	float ProcBonus = (static_cast<float>(spellbonuses.SpellProcChance + itembonuses.SpellProcChance + aabonuses.SpellProcChance));
	float ProcChance = 0.0f;
	if (weapon)
		ProcChance = GetProcChances(ProcBonus, weapon->Delay, hand);
	else
		ProcChance = GetProcChances(ProcBonus);

	if (hand != 13)
		ProcChance /= 2;

	bool rangedattk = false;
	if (weapon && hand == 11) {
		if (weapon->ItemType == ItemTypeArrow || weapon->ItemType == ItemTypeLargeThrowing || weapon->ItemType == ItemTypeSmallThrowing || weapon->ItemType == ItemTypeBow)
			rangedattk = true;
	}

	for (uint32 i = 0; i < MAX_PROCS; i++) {
		if (IsPet() && hand != 13)
			continue;

		if (!rangedattk) {
			if (PermaProcs[i].spellID != SPELL_UNKNOWN) {
				if (MakeRandomInt(0, 99) < PermaProcs[i].chance) {
					mlog(COMBAT__PROCS, "Permanent proc %d procing spell %d (%d percent chance)", i, PermaProcs[i].spellID, PermaProcs[i].chance);
					ExecWeaponProc(nullptr, PermaProcs[i].spellID, on);
				}
				else
					mlog(COMBAT__PROCS, "Permanent proc %d failed to proc %d (%d percent chance)", i, PermaProcs[i].spellID, PermaProcs[i].chance);
			}

			if (SpellProcs[i].spellID != SPELL_UNKNOWN) {
				float chance = ProcChance * (SpellProcs[i].chance / 100.0f);
				if (MakeRandomFloat(0, 1) <= chance) {
					mlog(COMBAT__PROCS, "Spell proc %d procing spell %d (%.2f percent chance)", i, SpellProcs[i].spellID, chance);
					ExecWeaponProc(nullptr, SpellProcs[i].spellID, on);
					CheckNumHitsRemaining(11, 0, SpellProcs[i].base_spellID);
				}
				else
					mlog(COMBAT__PROCS, "Spell proc %d failed to proc %d (%.2f percent chance)", i, SpellProcs[i].spellID, chance);
			}
		}
		else if (rangedattk) {
			if (RangedProcs[i].spellID != SPELL_UNKNOWN) {
				float chance = (ProcChance * (RangedProcs[i].chance / 100.0f));
				if (MakeRandomFloat(0, 1) <= chance) {
					mlog(COMBAT__PROCS, "Ranged proc %d procing spell %d (%.2f percent chance)", i, RangedProcs[i].spellID, chance);
					ExecWeaponProc(nullptr, RangedProcs[i].spellID, on);
					CheckNumHitsRemaining(11, 0, RangedProcs[i].base_spellID);
				}
				else
					mlog(COMBAT__PROCS, "Ranged proc %d failed to proc %d (%.2f percent chance)", i, RangedProcs[i].spellID, chance);
			}
		}
	}
	return;
}

void Mob::TryPetCriticalHit(Mob *defender, uint16 skill, int32 &damage) {
	if(damage < 1)
		return;

	Mob *owner = nullptr;
	float critChance = 0.0f;
	critChance += RuleI(Combat, MeleeBaseCritChance);
	uint16 critMod = 163;

	if (damage < 1)
		return;

	if (IsPet())
		owner = GetOwner();
	else if ((IsNPC() && CastToNPC()->GetSwarmOwner()))
		owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());
	else
		return;

	if (!owner)
		return;

	int16 CritPetChance = owner->aabonuses.PetCriticalHit + owner->itembonuses.PetCriticalHit + owner->spellbonuses.PetCriticalHit;
	int16 CritChanceBonus = GetCriticalChanceBonus(skill);

	if (CritPetChance || critChance) {
		critChance += CritPetChance;
		critChance += (critChance * CritChanceBonus / 100.0f);
	}

	if(critChance > 0) {
		critChance /= 100;

		if(MakeRandomFloat(0, 1) < critChance) {
			critMod += (GetCritDmgMob(skill) * 2);
			damage = ((damage * critMod) / 100);
			entity_list.FilteredMessageClose_StringID(this, false, 200, MT_CritMelee, FilterMeleeCrits, CRITICAL_HIT, GetCleanName(), itoa(damage));
		}
	}
}

void Mob::TryCriticalHit(Mob *defender, uint16 skill, int32 &damage, ExtraAttackOptions *opts) {
	if(damage < 1)
		return;

	if (IsPet() && GetOwner()->IsClient() || (IsNPC() && CastToNPC()->GetSwarmOwner())) {
		TryPetCriticalHit(defender,skill,damage);
		return;
	}

	float critChance = 0.0f;
	if(defender && defender->GetBodyType() == BT_Undead || defender->GetBodyType() == BT_SummonedUndead || defender->GetBodyType() == BT_Vampire){
		int16 SlayRateBonus = aabonuses.SlayUndead[0] + itembonuses.SlayUndead[0] + spellbonuses.SlayUndead[0];

		if (SlayRateBonus) {
			critChance += (float(SlayRateBonus) / 100.0f);
			critChance /= 100.0f;

			if(MakeRandomFloat(0, 1) < critChance){
				int16 SlayDmgBonus = aabonuses.SlayUndead[1] + itembonuses.SlayUndead[1] + spellbonuses.SlayUndead[1];
				damage = ((damage * SlayDmgBonus * 2.25) / 100);
				entity_list.MessageClose(this, false, 200, MT_CritMelee, "%s cleanses %s target!(%d)", GetCleanName(), this->GetGender() == 0 ? "his" : this->GetGender() == 1 ? "her" : "its", damage);
				return;
			}
		}
	}

	critChance += RuleI(Combat, MeleeBaseCritChance);

	if (IsClient()) {
		critChance += RuleI(Combat, ClientBaseCritChance);

		if ((GetClass() == WARRIOR || GetClass() == BERSERKER) && GetLevel() >= 12) {
			if (IsBerserk())
				critChance += RuleI(Combat, BerserkBaseCritChance);
			else
				critChance += RuleI(Combat, WarBerBaseCritChance);
		}
	}

	int deadlyChance = 0;
	int deadlyMod = 0;
	if(skill == SkillArchery && GetClass() == RANGER && GetSkill(SkillArchery) >= 65)
		critChance += 6;

	if (skill == SkillThrowing && GetClass() == ROGUE && GetSkill(SkillThrowing) >= 65) {
		critChance += RuleI(Combat, RogueCritThrowingChance);
		deadlyChance = RuleI(Combat, RogueDeadlyStrikeChance);
		deadlyMod = RuleI(Combat, RogueDeadlyStrikeMod);
	}

	int CritChanceBonus = GetCriticalChanceBonus(skill);

	if (CritChanceBonus || critChance) {
		if (GetDEX() <= 255)
			critChance += (float(GetDEX()) / 125.0f);
		else if (GetDEX() > 255)
			critChance += ((float(GetDEX()-255)/ 500.0f) + 2.0f);
		critChance += (critChance * (float)CritChanceBonus / 100.0f);
	}

	if(opts) {
		critChance *= opts->crit_percent;
		critChance += opts->crit_flat;
	}

	if(critChance > 0) {
		critChance /= 100;

		if(MakeRandomFloat(0, 1) < critChance) {
			uint16 critMod = 200;
			bool crip_success = false;
			int16 CripplingBlowChance = GetCrippBlowChance();

			if (CripplingBlowChance || IsBerserk()) {
				if (!IsBerserk())
					critChance *= (float(CripplingBlowChance) / 100.0f);

				if (IsBerserk() || MakeRandomFloat(0, 1) < critChance) {
					critMod = 400;
					crip_success = true;
				}
			}

			critMod += (GetCritDmgMob(skill) * 2);
			damage = (damage * critMod / 100);

			bool deadlySuccess = false;
			if (deadlyChance && MakeRandomFloat(0, 1) < static_cast<float>(deadlyChance) / 100.0f) {
				if (BehindMob(defender, GetX(), GetY())) {
					damage *= deadlyMod;
					deadlySuccess = true;
				}
			}

			if (crip_success) {
				entity_list.FilteredMessageClose_StringID(this, false, 200, MT_CritMelee, FilterMeleeCrits, CRIPPLING_BLOW, GetCleanName(), itoa(damage));
				if (defender->GetLevel() <= 55 && !defender->GetSpecialAbility(IMMUNE_STUN)){
					defender->Emote("staggers.");
					defender->Stun(0);
				}
			}
			else if (deadlySuccess)
				entity_list.FilteredMessageClose_StringID(this, false, 200, MT_CritMelee, FilterMeleeCrits, DEADLY_STRIKE, GetCleanName(), itoa(damage));
			else
				entity_list.FilteredMessageClose_StringID(this, false, 200, MT_CritMelee, FilterMeleeCrits, CRITICAL_HIT, GetCleanName(), itoa(damage));
		}
	}
}


bool Mob::TryFinishingBlow(Mob *defender, SkillUseTypes skillinuse) {
	if (!defender)
		return false;

	if (aabonuses.FinishingBlow[1] && !defender->IsClient() && defender->GetHPRatio() < 10){
		uint32 chance = (aabonuses.FinishingBlow[0] / 10);
		uint32 damage = aabonuses.FinishingBlow[1];
		uint16 levelreq = aabonuses.FinishingBlowLvl[0];

		if(defender->GetLevel() <= levelreq && (chance >= MakeRandomInt(0, 1000))) {
			mlog(COMBAT__ATTACKS, "Landed a finishing blow: levelreq at %d, other level %d", levelreq , defender->GetLevel());
			entity_list.MessageClose_StringID(this, false, 200, MT_CritMelee, FINISHING_BLOW, GetName());
			defender->Damage(this, damage, SPELL_UNKNOWN, skillinuse);
			return true;
		}
		else {
			mlog(COMBAT__ATTACKS, "FAILED a finishing blow: levelreq at %d, other level %d", levelreq , defender->GetLevel());
			return false;
		}
	}
	return false;
}

void Mob::DoRiposte(Mob* defender) {
	mlog(COMBAT__ATTACKS, "Preforming a riposte");

	if (!defender)
		return;

	defender->Attack(this, SLOT_PRIMARY, true);
	if (HasDied())
		return;

	int16 DoubleRipChance = (defender->aabonuses.GiveDoubleRiposte[0] + defender->spellbonuses.GiveDoubleRiposte[0] + defender->itembonuses.GiveDoubleRiposte[0]);

	if(DoubleRipChance && (DoubleRipChance >= MakeRandomInt(0, 100))) {
		mlog(COMBAT__ATTACKS, "Preforming a double riposed (%d percent chance)", DoubleRipChance);
		defender->Attack(this, SLOT_PRIMARY, true);
		if (HasDied())
			return;
	}
	
	DoubleRipChance = defender->aabonuses.GiveDoubleRiposte[1];

	if(DoubleRipChance && (DoubleRipChance >= MakeRandomInt(0, 100))) {
		mlog(COMBAT__ATTACKS, "Preforming a return SPECIAL ATTACK (%d percent chance)", DoubleRipChance);

		if (defender->GetClass() == MONK)
			defender->MonkSpecialAttack(this, defender->aabonuses.GiveDoubleRiposte[2]);
		else if (defender->IsClient())
			defender->CastToClient()->DoClassAttacks(this,defender->aabonuses.GiveDoubleRiposte[2], true);
	}
}

void Mob::ApplyMeleeDamageBonus(uint16 skill, int32 &damage) {

	if(!RuleB(Combat, UseIntervalAC)){
		if(IsNPC()){
			int dmgbonusmod = 0;
			dmgbonusmod += ((100 * (itembonuses.STR + spellbonuses.STR)) / 3);
			dmgbonusmod += ((100 * (spellbonuses.ATK + itembonuses.ATK)) / 5);
			mlog(COMBAT__DAMAGE, "Damage bonus: %d percent from ATK and STR bonuses.", (dmgbonusmod / 100));
			damage += (damage * dmgbonusmod / 10000);
		}
	}

	damage += (damage * GetMeleeDamageMod_SE(skill) / 100);
}

bool Mob::HasDied() {
	bool Result = false;
	int16 hp_below = 0;

	hp_below = (GetDelayDeath() * -1);

	if((GetHP()) <= (hp_below))
		Result = true;

	return Result;
}

uint16 Mob::GetDamageTable(SkillUseTypes skillinuse) {
	if(GetLevel() <= 51) {
		uint16 ret_table = 0;
		int str_over_75 = 0;
		if(GetSTR() > 75)
			str_over_75 = (GetSTR() - 75);
		if(str_over_75 > 255)
			ret_table = ((GetSkill(skillinuse) + 255) / 2);
		else
			ret_table = ((GetSkill(skillinuse) + str_over_75) / 2);

		if(ret_table < 100)
			return 100;

		return ret_table;
	}
	else if(GetLevel() >= 90) {
		if(GetClass() == MONK)
			return 379;
		else
			return 345;
	}
	else {
		uint16 dmg_table[] = {275, 275, 275, 275, 275, 280, 280, 280, 280, 285, 285, 285, 290, 290, 295, 295, 300, 300, 300, 305, 305, 305, 310, 310, 315, 315, 320, 320, 320, 325, 325, 325, 330, 330, 335, 335, 340, 340, 340};
		if(GetClass() == MONK)
			return (dmg_table[GetLevel()-51]*(100+RuleI(Combat,MonkDamageTableBonus))/100);
		else
			return dmg_table[GetLevel()-51];
	}
}

void Mob::TrySkillProc(Mob *on, uint16 skill, float chance) {
	if (!on) {
		SetTarget(nullptr);
		LogFile->write(EQEMuLog::Error, "A null Mob object was passed to Mob::TrySkillProc for evaluation!");
		return;
	}

	for (int i = 0; i < MAX_PROCS; i++) {
		if (SkillProcs[i].spellID != SPELL_UNKNOWN){
			if (PassLimitToSkill(SkillProcs[i].base_spellID,skill)) {
				int ProcChance = chance * (float)SkillProcs[i].chance;
				if ((MakeRandomInt(0, 100) < ProcChance)) {
					ExecWeaponProc(nullptr, SkillProcs[i].spellID, on);
					CheckNumHitsRemaining(11,0, SkillProcs[i].base_spellID);
				}
			}
		}
	}
}

bool Mob::TryRootFadeByDamage(int buffslot, Mob* attacker) {
	if (!attacker || !spellbonuses.Root[0] || spellbonuses.Root[1] < 0)
 		return false;
 
 	if (IsDetrimentalSpell(spellbonuses.Root[1]) && spellbonuses.Root[1] != buffslot) { 	
 		int BreakChance = RuleI(Spells, RootBreakFromSpells);		
 		BreakChance -= BreakChance*buffs[spellbonuses.Root[1]].RootBreakChance/100;
		int level_diff = attacker->GetLevel() - GetLevel();

		if (level_diff == 2)
			BreakChance = ((BreakChance * 80) / 100);
 
		else if (level_diff >= 3 && level_diff <= 20)
			BreakChance = ((BreakChance * 60) / 100);

		else if (level_diff > 21)
			BreakChance = ((BreakChance * 20) / 100);
		
 		if (BreakChance < 1)
 			BreakChance = 1;

		if (MakeRandomInt(0, 99) < BreakChance) {
			if (!TryFadeEffect(spellbonuses.Root[1])) {
				BuffFadeBySlot(spellbonuses.Root[1]);
				mlog(COMBAT__HITS, "Spell broke root! BreakChance percent chance");
				return true;
			}
		}
	}

	mlog(COMBAT__HITS, "Spell did not break root. BreakChance percent chance");
	return false;
}

int32 Mob::RuneAbsorb(int32 damage, uint16 type) {
	uint32 buff_max = GetMaxTotalSlots();
	if (type == SE_Rune){
		for(uint32 slot = 0; slot < buff_max; slot++) {
			if(slot == spellbonuses.MeleeRune[1] && spellbonuses.MeleeRune[0] && buffs[slot].melee_rune && IsValidSpell(buffs[slot].spellid)){
				uint32 melee_rune_left = buffs[slot].melee_rune;
				
				if(melee_rune_left > damage) {
					melee_rune_left -= damage;
					buffs[slot].melee_rune = melee_rune_left;
					return -6;
				}
				else {
					if(melee_rune_left > 0)
						damage -= melee_rune_left;
						
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}

	else {
		for(uint32 slot = 0; slot < buff_max; slot++) {
			if(slot == spellbonuses.AbsorbMagicAtt[1] && spellbonuses.AbsorbMagicAtt[0] && buffs[slot].magic_rune && IsValidSpell(buffs[slot].spellid)){
				uint32 magic_rune_left = buffs[slot].magic_rune;
				if(magic_rune_left > damage) {
					magic_rune_left -= damage;
					buffs[slot].magic_rune = magic_rune_left;
					return 0;
				}

				else {
					if(magic_rune_left > 0)
						damage -= magic_rune_left;
					
					if(!TryFadeEffect(slot))
						BuffFadeBySlot(slot);
				}
			}
		}
	}
	return damage;
}