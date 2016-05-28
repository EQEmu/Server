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
//#define TUNE_DEBUG 20
#endif

#include "../common/global_define.h"
#include "../common/eq_constants.h"
#include "../common/eq_packet_structs.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/string_util.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "water_map.h"
#include "worldserver.h"
#include "zone.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef BOTS
#include "bot.h"
#endif

extern QueryServ* QServ;
extern WorldServer worldserver;

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern EntityList entity_list;
extern Zone* zone;

void Mob::Tune_FindATKByPctMitigation(Mob* defender,Mob *attacker, float pct_mitigation, int interval, int max_loop, int ac_override, int Msg)
{
	/*Find the amount of 'ATTACK' stat that has to be added/subtracted FROM ATTACKER to reach a specific average mitigation value on the TARGET.
	Can use ac_override to find the value verse a hypothetical amount of worn AC */

	int atk = 0;
	uint32 total_damage = 0;
	int32 damage = 0;
	uint32 minhit = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	int end = 0;

	if (attacker->IsNPC())
	{
		damage = static_cast<int32>(attacker->CastToNPC()->GetMaxDMG());
		minhit = attacker->CastToNPC()->GetMinDMG();
	}
	else if (attacker->IsClient())
	{
		damage = static_cast<int32>(attacker->CastToClient()->GetMeleeDamage(this));
		minhit = attacker->CastToClient()->GetMeleeDamage(this, true);
	}

	if (damage == 0 || minhit == 0)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", damage,minhit);
		return;
	}

	mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, ac_override, 0, 0,atk);
	tmp_pct_mitigated = 100.0f - static_cast<float>( mean_dmg * 100 /damage);

	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find ATK for attacker Mitigation (%.0f) pct on defender [MaxDMG %i MinDMG %i Current Mitigation %.2f]", pct_mitigation, damage, minhit,tmp_pct_mitigated);

	if (tmp_pct_mitigated < pct_mitigation)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++) 
	{
		mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, ac_override,0, 0,atk);
		tmp_pct_mitigated = 100.0f - ( static_cast<float>(mean_dmg) * 100.0f /static_cast<float>(damage));

		if (Msg >= 3)
			Message(0, "#Tune - Processing... [%i] [ATK %i] Average Melee Hit  %i | Pct Mitigated %.2f ",j,atk, mean_dmg, tmp_pct_mitigated);

		if (interval > 0 && tmp_pct_mitigated <= pct_mitigation)
			end = 1;
				
		else if (interval < 0 && tmp_pct_mitigated >= pct_mitigation)
			end = 1;
		
		else if (interval < 0 && mean_dmg == minhit)
			end = 2;

		if (end >= 1){

			defender->Tune_MeleeMitigation(this, attacker, damage, minhit, nullptr,Msg,ac_override, 0, 0, atk);
			
			if (end == 2)
				Message(0, "#Tune - [WARNING] Mitigation can not be further decreased due to minium hit value (%i).",minhit);

			if (attacker->IsNPC()){
				Message(0, "#Tune - Recommended NPC ATK ADJUSTMENT ( %i ) on ' %s ' average mitigation of (%.0f) pct verse ' %s '. ",atk, attacker->GetCleanName(), pct_mitigation, defender->GetCleanName());
				Message(0, "#SET: [NPC Attack STAT] = [%i]",atk + defender->CastToNPC()->ATK);
			}
			if (attacker->IsClient()){
				Message(0, "#Tune - Recommended CLIENT ATK ADJUSTMENT ( %i ) on ' %s ' average mitigation of (%.0f) pct verse ' %s '. ", atk, attacker->GetCleanName(), pct_mitigation, defender->GetCleanName());
				Message(0, "#Modify (+/-): [Client Attack STAT/SE_ATK(2)] [%i]",atk);
			}

			return;
		}

		atk = atk + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "#Tune - Parse ended at ATK ADJUSTMENT ( %i ) average target mitigation of (%.0f) pct.",atk,tmp_pct_mitigated);
}

void Mob::Tune_FindACByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval, int max_loop, int atk_override, int Msg)
{

	/*Find the amount of AC stat that has to be added/subtracted from TARGET to reach a specific average mitigation value based on ATTACKER's statistics.
	Can use ac_override to find the value verse a hypothetical amount of worn AC */

	int add_ac = 0;
	uint32 total_damage = 0;
	int32 damage = 0;
	uint32 minhit = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	int end = 0;


	if (attacker->IsNPC())
	{
		damage = static_cast<int32>(attacker->CastToNPC()->GetMaxDMG());
		minhit = attacker->CastToNPC()->GetMinDMG();
	}
	else if (attacker->IsClient())
	{
		damage = static_cast<int32>(attacker->CastToClient()->GetMeleeDamage(this));
		minhit = attacker->CastToClient()->GetMeleeDamage(this, true);
	}

	if (damage == 0 || minhit == 0)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", damage,minhit);
		return;
	}

	mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, 0, atk_override);
	tmp_pct_mitigated = 100.0f - static_cast<float>( mean_dmg * 100 /damage);

	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find AC for defender Mitigation (%.0f) pct from attacker [MaxDMG %i MinDMG %i Current Mitigation %.2f]", pct_mitigation, damage, minhit,tmp_pct_mitigated);


	if (tmp_pct_mitigated > pct_mitigation)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++) 
	{
		mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, 0,atk_override, add_ac, 0);
		tmp_pct_mitigated = 100.0f - ( static_cast<float>(mean_dmg) * 100.0f /static_cast<float>(damage));

		if (Msg >= 3)
			Message(0, "#Tune - Processing... [%i] [AC %i] Average Melee Hit  %i | Pct Mitigated %.2f ",j,add_ac, mean_dmg, tmp_pct_mitigated);

		if (interval > 0 && tmp_pct_mitigated >= pct_mitigation)
			end = 1;

		else if (interval < 0 && tmp_pct_mitigated <= pct_mitigation)
			end = 1;

		else if (interval < 0 && mean_dmg == minhit)
			end = 2;

		if (end >= 1){

			defender->Tune_MeleeMitigation(this, attacker, damage, minhit, nullptr,Msg,0,atk_override, add_ac, 0);
			
			if (end == 2)
				Message(0, "#Tune - [WARNING] Mitigation can not be further decreased due to minium hit value (%i).",minhit);

			if (defender->IsNPC()){
				Message(7, "#Tune - Recommended NPC AC ADJUSTMENT ( %i ) on ' %s ' for an average mitigation of (+ %.0f) pct from attacker ' %s '.",add_ac,defender->GetCleanName(), pct_mitigation, attacker->GetCleanName());
				Message(0, "#SET: [NPC Attack STAT] = [%i]",add_ac + defender->CastToNPC()->GetRawAC());
			}
			if (defender->IsClient()){
				Message(7, "#Tune - Recommended CLIENT AC ADJUSTMENT ( %i ) on ' %s ' for an average mitigation of (+ %.0f) pct from attacker ' %s '.",add_ac,defender->GetCleanName(), pct_mitigation, attacker->GetCleanName());
				Message(0, "#Modify (+/-): [Client AC STAT/SE_AC(1)] [%i]",add_ac);
			}

			return;
		}



		add_ac = add_ac + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "#Tune - Parse ended at AC ADJUSTMENT ( %i ) at average mitigation of (%.0f) / (%.0f) pct.",add_ac,tmp_pct_mitigated / pct_mitigation);
}

uint32 Mob::Tune_GetMeanDamage(Mob* GM, Mob *attacker, int32 damage, int32 minhit, ExtraAttackOptions *opts, int Msg,
							   int ac_override, int atk_override, int add_ac, int add_atk)
{
	uint32 total_damage = 0;
	int loop_max = 1000;

	for (int i=0; i < loop_max ; i++) 
	{
		total_damage += Tune_MeleeMitigation(GM, attacker, damage, minhit, nullptr,0,ac_override, atk_override, add_ac, add_atk);
	}

	return(total_damage/loop_max);
}

int32 Mob::Tune_MeleeMitigation(Mob* GM, Mob *attacker, int32 damage, int32 minhit, ExtraAttackOptions *opts, int Msg, 
								int ac_override, int atk_override, int add_ac, int add_atk)
{
	if (damage <= 0)
		return 0;

	Mob* defender = this;
	float aa_mit = (aabonuses.CombatStability + itembonuses.CombatStability +
			spellbonuses.CombatStability) / 100.0f;

	if (Msg){
		
		GM->Message(0, "######### Melee Mitigation Report: Start [Detail Level %i]#########", Msg);
		GM->Message(0, "#ATTACKER: %s", attacker->GetCleanName());
		GM->Message(0, "#DEFENDER: %s", defender->GetCleanName());
	}

	if (RuleB(Combat, UseIntervalAC)) {
		float softcap = (GetSkill(EQEmu::skills::SkillDefense) + GetLevel()) *
			RuleR(Combat, SoftcapFactor) * (1.0 + aa_mit);
		float mitigation_rating = 0.0;
		float attack_rating = 0.0;
		int shield_ac = 0;
		int armor = 0;
		float weight = 0.0;

		if (Msg >= 2){
			GM->Message(0, " "); 
			GM->Message(0, "### Calculate Mitigation Rating ###"); 
			if (aabonuses.CombatStability)
				GM->Message(0, "# %i #### DEFENDER SE_CombatStability(259) AA Bonus", aabonuses.CombatStability);
			if (spellbonuses.CombatStability)
				GM->Message(0, "# %i #### DEFENDER SE_CombatStability(259) Spell Bonus", spellbonuses.CombatStability);
			if (itembonuses.CombatStability)
				GM->Message(0, "# %i #### DEFENDER SE_CombatStability(259) Worn Bonus", itembonuses.CombatStability);

			GM->Message(0, "# %.2f #### DEFENDER Base Soft Cap", softcap);
		}

		float monkweight = RuleI(Combat, MonkACBonusWeight);
		monkweight = mod_monk_weight(monkweight, attacker);

		if (IsClient()) {
			armor = CastToClient()->GetRawACNoShield(shield_ac) + add_ac;
			weight = (CastToClient()->CalcCurrentWeight() / 10.0);

			if (ac_override)
				armor = ac_override;
			
			if (Msg >=2 ){
				GM->Message(0, "# %i #### DEFENDER AC Equiped/Worn Bonus", itembonuses.AC);
				GM->Message(0, "# %i #### DEFENDER SE_ArmorClass(1) AA Bonus", aabonuses.AC);
				GM->Message(0, "# %i #### DEFENDER SE_ArmorClass(1) Spell Bonus", spellbonuses.AC);
				GM->Message(0, "# %i #### DEFENDER Shield AC", shield_ac);
				GM->Message(0, "# %i #### DEFENDER Total Client Armor - NO shield", armor);
			}
			
		} else if (IsNPC()) {
			armor = CastToNPC()->GetRawAC() + add_ac;

			if (ac_override)
				armor = ac_override;

			if (Msg >=2 ){
				GM->Message(0, "# %i #### DEFENDER AC Equiped/Worn Bonus", itembonuses.AC);
				GM->Message(0, "# %i #### DEFENDER SE_ArmorClass(1) Spell Bonus", spellbonuses.AC);
				GM->Message(0, "# %i #### DEFENDER NPC AC Stat", CastToNPC()->GetRawAC());
			}

			int PetACBonus = 0;

			if (!IsPet()){
				armor = (armor / RuleR(Combat, NPCACFactor));
				if (Msg >=2 )
					GM->Message(0, "# %i #### DEFENDER NPC Armor after RuleR(Combat, NPCACFactor) %.2f", armor, RuleR(Combat, NPCACFactor));
			}

			Mob *owner = nullptr;
			if (IsPet())
				owner = GetOwner();
			else if ((CastToNPC()->GetSwarmOwner()))
				owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

			if (owner){
				PetACBonus = owner->aabonuses.PetMeleeMitigation + owner->itembonuses.PetMeleeMitigation + owner->spellbonuses.PetMeleeMitigation;

				if (Msg >=2 ){
				if (owner->aabonuses.PetMeleeMitigation)
					GM->Message(0, "# %i #### DEFENDER Pet Owner SE_PetMeleeMitigation(379) AA Bonus", owner->aabonuses.PetMeleeMitigation);
				if (owner->spellbonuses.PetMeleeMitigation)
					GM->Message(0, "# %i #### DEFENDER Pet Owner SE_PetMeleeMitigation(379) Spell Bonus",owner->spellbonuses.PetMeleeMitigation);
				if (owner->itembonuses.PetMeleeMitigation)
					GM->Message(0, "# %i #### DEFENDER Pet Owner SE_PetMeleeMitigation(379) Worn Bonus", owner->itembonuses.PetMeleeMitigation);
				}
			}

			armor += spellbonuses.AC + itembonuses.AC + PetACBonus + 1;

			if (Msg >= 2)
				GM->Message(0, "# %i #### DEFENDER NPC Total Base Armor",armor);
		}

		if (opts) {
			armor *= (1.0f - opts->armor_pen_percent);
			armor -= opts->armor_pen_flat;
		}

		if (RuleB(Combat, OldACSoftcapRules)) {
			if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
					GetClass() == NECROMANCER || GetClass() == ENCHANTER)
				softcap = RuleI(Combat, ClothACSoftcap);
			else if (GetClass() == MONK && weight <= monkweight)
				softcap = RuleI(Combat, MonkACSoftcap);
			else if(GetClass() == DRUID || GetClass() == BEASTLORD || GetClass() == MONK)
				softcap = RuleI(Combat, LeatherACSoftcap);
			else if(GetClass() == SHAMAN || GetClass() == ROGUE ||
					GetClass() == BERSERKER || GetClass() == RANGER)
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
					softcap_armor = softcap_armor * RuleR(Combat, WarriorACSoftcapReturn);
				else if (GetClass() == SHADOWKNIGHT || GetClass() == PALADIN ||
						(GetClass() == MONK && weight <= monkweight))
					softcap_armor = softcap_armor * RuleR(Combat, KnightACSoftcapReturn);
				else if (GetClass() == CLERIC || GetClass() == BARD ||
						GetClass() == BERSERKER || GetClass() == ROGUE ||
						GetClass() == SHAMAN || GetClass() == MONK)
					softcap_armor = softcap_armor * RuleR(Combat, LowPlateChainACSoftcapReturn);
				else if (GetClass() == RANGER || GetClass() == BEASTLORD)
					softcap_armor = softcap_armor * RuleR(Combat, LowChainLeatherACSoftcapReturn);
				else if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
						GetClass() == NECROMANCER || GetClass() == ENCHANTER ||
						GetClass() == DRUID)
					softcap_armor = softcap_armor * RuleR(Combat, CasterACSoftcapReturn);
				else
					softcap_armor = softcap_armor * RuleR(Combat, MiscACSoftcapReturn);
			} else {
				if (GetClass() == WARRIOR)
					softcap_armor *= RuleR(Combat, WarACSoftcapReturn);
				else if (GetClass() == PALADIN || GetClass() == SHADOWKNIGHT)
					softcap_armor *= RuleR(Combat, PalShdACSoftcapReturn);
				else if (GetClass() == CLERIC || GetClass() == RANGER ||
						GetClass() == MONK || GetClass() == BARD)
					softcap_armor *= RuleR(Combat, ClrRngMnkBrdACSoftcapReturn);
				else if (GetClass() == DRUID || GetClass() == NECROMANCER ||
						GetClass() == WIZARD || GetClass() == ENCHANTER ||
						GetClass() == MAGICIAN)
					softcap_armor *= RuleR(Combat, DruNecWizEncMagACSoftcapReturn);
				else if (GetClass() == ROGUE || GetClass() == SHAMAN ||
						GetClass() == BEASTLORD || GetClass() == BERSERKER)
					softcap_armor *= RuleR(Combat, RogShmBstBerACSoftcapReturn);
				else
					softcap_armor *= RuleR(Combat, MiscACSoftcapReturn);
			}
			
			
			armor = softcap + softcap_armor;
			if (Msg >= 2)
				GM->Message(0, "# %i #### DEFENDER Final Armor [Soft Cap %i Soft Cap Armor %i]",armor, softcap,softcap_armor);
		}
		int tmp_armor = armor;
		if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
				GetClass() == NECROMANCER || GetClass() == ENCHANTER){
			mitigation_rating = ((GetSkill(EQEmu::skills::SkillDefense) + itembonuses.HeroicAGI / 10) / 4.0) + armor + 1;
			if (Msg >= 2)
				GM->Message(0, "# + %.2f #### DEFENDER Armor Bonus [Defense Skill %i Heroic Agi %i]", mitigation_rating - tmp_armor, GetSkill(EQEmu::skills::SkillDefense), itembonuses.HeroicAGI);
		}
		else{
			mitigation_rating = ((GetSkill(EQEmu::skills::SkillDefense) + itembonuses.HeroicAGI / 10) / 3.0) + (armor * 1.333333) + 1;
			if (Msg >= 2)
				GM->Message(0, "# + %.2f #### DEFENDER Armor Bonus [Defense Skill %i Heroic Agi %i]", mitigation_rating - tmp_armor, GetSkill(EQEmu::skills::SkillDefense), itembonuses.HeroicAGI);

		}
		mitigation_rating *= 0.847;

		if (Msg >= 1)
			GM->Message(0, "# %.2f #### DEFENDER Final Mitigation Rating", mitigation_rating);

		
		if (Msg >= 2){
			GM->Message(0, " ");
			GM->Message(0, "### Mitigation Bonus Effects ###"); 
			if (itembonuses.MeleeMitigation)
				GM->Message(0, "# %i #### DEFENDER Item Mod2 Shielding", itembonuses.MeleeMitigation);
			if (aabonuses.MeleeMitigationEffect)
				GM->Message(0, "# %i #### DEFENDER SE_MeleeMitigation(168) AA Bonus", aabonuses.MeleeMitigationEffect);
			if (spellbonuses.MeleeMitigationEffect)
				GM->Message(0, "# %i #### DEFENDER SE_MeleeMitigation(168) Spell Bonus", spellbonuses.MeleeMitigationEffect);
			if (itembonuses.MeleeMitigationEffect)
				GM->Message(0, "# %i #### DEFENDER SE_MeleeMitigation(168) Worn Bonus", itembonuses.MeleeMitigationEffect);
		}

		mitigation_rating = mod_mitigation_rating(mitigation_rating, attacker);

		if (attacker->IsClient()){
			if (atk_override)
				attack_rating = (atk_override + ((attacker->GetSTR() - 66) * 0.9) + (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345));
			else
				attack_rating = ((attacker->CastToClient()->CalcATK() + add_atk) + ((attacker->GetSTR() - 66) * 0.9) + (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345));

		}
		else{
			if (atk_override)
				attack_rating = (atk_override + (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345) + ((attacker->GetSTR() - 66) * 0.9));
			else
				attack_rating = ((attacker->GetATK() + add_atk) + (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345) + ((attacker->GetSTR() - 66) * 0.9));
		}

		attack_rating = attacker->mod_attack_rating(attack_rating, this);

		if (Msg >= 2){
			GM->Message(0, " "); 
			GM->Message(0, "### Calculate Attack Rating ###"); 
			if (attacker->IsClient()){
				GM->Message(0, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(0, "# %i #### ATTACKER SE_ATK(2) AA Bonus", attacker->aabonuses.ATK);
				GM->Message(0, "# %i #### ATTACKER SE_ATK(2) spell Bonus", attacker->spellbonuses.ATK);
				GM->Message(0, "# %i #### ATTACKER Leadership Bonus", attacker->CastToClient()->GroupLeadershipAAOffenseEnhancement());
				GM->Message(0, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(0, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(0, "# %.2f #### ATTACKER Strength Stat ATK Bonus [Stat Amt: %i]", ((attacker->GetSTR()-66) * 0.9),attacker->GetSTR());
				GM->Message(0, "# %.2f #### ATTACKER Offensive Skill ATK Bonus [Stat Amt: %i]", (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345), attacker->GetSkill(EQEmu::skills::SkillOffense));
			}
				
			else{
				GM->Message(0, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(0, "# %i #### ATTACKER SE_ATK(2) spell Bonus", attacker->spellbonuses.ATK);
				GM->Message(0, "# %i #### ATTACKER NPC ATK Stat", attacker->CastToNPC()->ATK);
				GM->Message(0, "# %.2f #### ATTACKER Strength Stat ATK Bonus [Stat Amt: %i]", ((attacker->GetSTR()-66) * 0.9),attacker->GetSTR());
				GM->Message(0, "# %.2f #### ATTACKER Offensive Skill ATK Bonus [Stat Amt: %i]", (attacker->GetSkill(EQEmu::skills::SkillOffense)*1.345), attacker->GetSkill(EQEmu::skills::SkillOffense));
			}
		}

		if (Msg >= 1){
			GM->Message(0, "# %.2f #### ATTACKER Final Attack Rating", attack_rating);
			GM->Message(0, "######### Melee Mitigation Report: Complete #########", Msg);
		}


		damage = GetMeleeMitDmg(attacker, damage, minhit, mitigation_rating, attack_rating);
	} 

	if (damage < 0)
		damage = 0;

	return damage;
}

// This is called when the Mob is the one being hit
int32 Mob::Tune_GetMeleeMitDmg(Mob* GM, Mob *attacker, int32 damage, int32 minhit,
		float mit_rating, float atk_rating)
{
	float d = 10.0;
	float mit_roll = zone->random.Real(0, mit_rating);
	float atk_roll = zone->random.Real(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = atk_roll - mit_roll;
		float thac0 = atk_rating * RuleR(Combat, ACthac0Factor);
		float thac0cap = attacker->GetLevel() * 9 + 20;
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d -= 10.0 * (a_diff / thac0);
	} else if (mit_roll > atk_roll) {
		float m_diff = mit_roll - atk_roll;
		float thac20 = mit_rating * RuleR(Combat, ACthac20Factor);
		float thac20cap = GetLevel() * 9 + 20;
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d += 10.0 * (m_diff / thac20);
	}

	if (d < 0.0)
		d = 0.0;
	else if (d > 20.0)
		d = 20.0;

	float interval = (damage - minhit) / 20.0;
	damage -= ((int)d * interval);

	damage -= (minhit * itembonuses.MeleeMitigation / 100);
	damage -= (damage *  (spellbonuses.MeleeMitigationEffect + itembonuses.MeleeMitigationEffect + aabonuses.MeleeMitigationEffect) / 100);
	return damage;
}

// This is called when the Client is the one being hit
int32 Client::Tune_GetMeleeMitDmg(Mob* GM, Mob *attacker, int32 damage, int32 minhit,
		float mit_rating, float atk_rating)
{
	if (!attacker->IsNPC() || RuleB(Combat, UseOldDamageIntervalRules))
		return Mob::GetMeleeMitDmg(attacker, damage, minhit, mit_rating, atk_rating);
	int d = 10;
	// floats for the rounding issues
	float dmg_interval = (damage - minhit) / 19.0;
	float dmg_bonus = minhit - dmg_interval;
	float spellMeleeMit =  (spellbonuses.MeleeMitigationEffect + itembonuses.MeleeMitigationEffect + aabonuses.MeleeMitigationEffect) / 100.0;
	if (GetClass() == WARRIOR)
		spellMeleeMit += 0.05;
	dmg_bonus -= dmg_bonus * (itembonuses.MeleeMitigation / 100.0);
	dmg_interval -= dmg_interval * spellMeleeMit;

	float mit_roll = zone->random.Real(0, mit_rating);
	float atk_roll = zone->random.Real(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = atk_roll - mit_roll;
		float thac0 = atk_rating * RuleR(Combat, ACthac0Factor);
		float thac0cap = attacker->GetLevel() * 9 + 20;
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d += 10 * (a_diff / thac0);
	} else if (mit_roll > atk_roll) {
		float m_diff = mit_roll - atk_roll;
		float thac20 = mit_rating * RuleR(Combat, ACthac20Factor);
		float thac20cap = GetLevel() * 9 + 20;
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d -= 10 * (m_diff / thac20);
	}

	if (d < 1)
		d = 1;
	else if (d > 20)
		d = 20;

	return static_cast<int32>((dmg_bonus + dmg_interval * d));
}


int32 Client::GetMeleeDamage(Mob* other, bool GetMinDamage)
{
	int Hand = EQEmu::legacy::SlotPrimary;

	if (!other) 
		return 0;
	
	ItemInst* weapon;
	weapon = GetInv().GetItem(EQEmu::legacy::SlotPrimary);
	
	if(weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			return(0);
		}
	} 

	EQEmu::skills::SkillType skillinuse;
	AttackAnimation(skillinuse, Hand, weapon);

	int damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon) hate = weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt;
	int weapon_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && weapon_damage > 1) hate = weapon_damage;

	if(weapon_damage > 0){
		if(IsBerserk() && GetClass() == BERSERKER){
			int bonus = 3 + GetLevel()/10;		//unverified
			weapon_damage = weapon_damage * (100+bonus) / 100;
		}

		int min_hit = 1;
		int max_hit = (2*weapon_damage*GetDamageTable(skillinuse)) / 100;

		if(GetLevel() < 10 && max_hit > RuleI(Combat, HitCapPre10))
			max_hit = (RuleI(Combat, HitCapPre10));
		else if(GetLevel() < 20 && max_hit > RuleI(Combat, HitCapPre20))
			max_hit = (RuleI(Combat, HitCapPre20));

		CheckIncreaseSkill(skillinuse, other, -15);
		CheckIncreaseSkill(EQEmu::skills::SkillOffense, other, -15);


#ifndef EQEMU_NO_WEAPON_DAMAGE_BONUS

		int ucDamageBonus = 0;

		if (Hand == EQEmu::legacy::SlotPrimary && GetLevel() >= 28 && IsWarriorClass())
		{
			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQEmu::ItemBase*) nullptr);

			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		min_hit += min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100;

		if(max_hit < min_hit)
			max_hit = min_hit;

		if (GetMinDamage)
			return min_hit;
		else
			return max_hit;
	}

	return 0;
}

void Mob::Tune_FindAccuaryByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int avoid_override, int Msg)
{

	int add_acc = 0;
	float tmp_hit_chance = 0.0f;
	bool end = false;

	EQEmu::skills::SkillType skillinuse = EQEmu::skills::SkillHandtoHand;
	if (attacker->IsClient())
	{//Will check first equiped weapon for skill. Ie. remove wepaons to assess bow.
		ItemInst* weapon;
		weapon = attacker->CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotPrimary);
			
		if(weapon && weapon->IsWeapon()){
			attacker->CastToClient()->AttackAnimation(skillinuse, EQEmu::legacy::SlotPrimary, weapon);
		}
		else {
			weapon = attacker->CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotSecondary);
			if (weapon && weapon->IsWeapon()) 
				attacker->CastToClient()->AttackAnimation(skillinuse, EQEmu::legacy::SlotSecondary, weapon);
			else {
				weapon = attacker->CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotRange);
				if (weapon && weapon->IsWeapon()) 
					attacker->CastToClient()->AttackAnimation(skillinuse, EQEmu::legacy::SlotRange, weapon);
			}
		}
	}

	tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQEmu::legacy::SlotPrimary, 0, 0, 0, avoid_override);

	
	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find Accuracy for hit chance on attacker of (%.0f) pct on defender [Current Hit Chance %.2f]", hit_chance, tmp_hit_chance);


	if (tmp_hit_chance > hit_chance)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++)        
	{
		tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQEmu::legacy::SlotPrimary, 0, false, 0, avoid_override, add_acc);

		if (Msg >= 3)
			Message(15, "#Tune - Processing... [%i] [ACCURACY %i] Hit Chance %.2f ",j,add_acc,tmp_hit_chance);

		if (interval > 0 && tmp_hit_chance >= hit_chance){
			end = true;
		}

		else if (interval < 0 && tmp_hit_chance <= hit_chance){
			end = true;
		}

		if (end){
			
			Tune_CheckHitChance(defender, attacker, skillinuse, EQEmu::legacy::SlotPrimary, 0, Msg, 0, avoid_override);//Display Stat Report
			
			Message(0, " ");

			if (attacker->IsNPC()){
				Message(0, "#Recommended NPC Accuracy Statistic adjustment of ( %i ) on ' %s ' for a hit chance of (+ %.0f) pct verse ' %s '. ",add_acc,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#SET: [NPC Avoidance] = [%i]",add_acc + defender->CastToNPC()->GetAccuracyRating());
			}
			else if (attacker->IsClient()){
				Message(0, "#Recommended Client Accuracy Bonus adjustment of ( %i ) on ' %s ' for a hit chance of (+ %.0f) pct verse ' %s '. ",add_acc,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#Modify (+/-): [Item Mod2 Accuracy]     [%i]",add_acc);
				Message(0, "#Modify (+/-): [SE_Accuracy(216)]       [%i]",add_acc);
				Message(0, "#Modify (+/-): [SE_HitChance(184)]      [%i]",add_acc / 15);
			}

			return;
		}


		add_acc = add_acc + interval;
	}

	Message(7, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", hit_chance, interval, max_loop);
	Message(7, "#Tune - Parse ended at ACCURACY ADJUSTMENTT ( %i ) at hit chance of (%.0f) / (%.0f) pct.",add_acc,tmp_hit_chance / hit_chance);
}

void Mob::Tune_FindAvoidanceByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int acc_override, int Msg)
{
	int add_avoid = 0;
	float tmp_hit_chance = 0.0f;
	bool end = false;

	EQEmu::skills::SkillType skillinuse = EQEmu::skills::SkillHandtoHand;
	if (attacker->IsClient())
	{//Will check first equiped weapon for skill. Ie. remove wepaons to assess bow.
		ItemInst* weapon;
		weapon = attacker->CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotPrimary);
			
		if(weapon && weapon->IsWeapon()){
			attacker->CastToClient()->AttackAnimation(skillinuse, EQEmu::legacy::SlotPrimary, weapon);
		}
		else {
			weapon = attacker->CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotSecondary);
			if (weapon && weapon->IsWeapon()) 
				attacker->CastToClient()->AttackAnimation(skillinuse, EQEmu::legacy::SlotSecondary, weapon);
			else {
				weapon = attacker->CastToClient()->GetInv().GetItem(EQEmu::legacy::SlotRange);
				if (weapon && weapon->IsWeapon()) 
					attacker->CastToClient()->AttackAnimation(skillinuse, EQEmu::legacy::SlotRange, weapon);
			}
		}
	}

	tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQEmu::legacy::SlotPrimary, 0, 0, acc_override, 0);

	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find Avoidance for hit chance on defender of (%.0f) pct from attacker. [Current Hit Chance %.2f]", hit_chance, tmp_hit_chance);

	if (tmp_hit_chance < hit_chance)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++)        
	{
		tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQEmu::legacy::SlotPrimary, 0, 0, acc_override, 0, 0, add_avoid);

		if (Msg >= 3)
			Message(0, "#Tune - Processing... [%i] [AVOIDANCE %i] Hit Chance %.2f ",j,add_avoid,tmp_hit_chance);

		if (interval > 0 && tmp_hit_chance <= hit_chance){
			end = true;
		}

		else if (interval < 0 && tmp_hit_chance >= hit_chance){
			end = true;
		}

		if (end){
			
			Tune_CheckHitChance(defender, attacker, skillinuse, EQEmu::legacy::SlotPrimary, 0, Msg, acc_override, 0);//Display Stat Report
			
			Message(0, " ");

			if (defender->IsNPC()){
				Message(0, "#Recommended NPC Avoidance Statistic adjustment of ( %i ) on ' %s ' for a hit chance of ( %.0f) pct from ' %s '. ",add_avoid,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#SET: [NPC Avoidance] = [%i]",add_avoid + defender->CastToNPC()->GetAvoidanceRating());
			}
			else if (defender->IsClient()){
				Message(0, "#Recommended Client Avoidance Bonus adjustment of ( %i ) on ' %s ' for a hit chance of ( %.0f) pct from ' %s '. ",add_avoid,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#Modify (+/-): [Item Mod2 Avoidance] [%i]",add_avoid);
				Message(0, "#Modify (+/-): [SE_AvoidMeleeChance(172)]  [%i]",add_avoid / 10);
			}

			return;
		}

		add_avoid = add_avoid + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", hit_chance, interval, max_loop);
	Message(0, "#Tune - Parse ended at AVOIDANCE ADJUSTMENT ( %i ) at hit chance of (%.0f) / (%.0f) pct.",add_avoid,tmp_hit_chance / hit_chance);
}


float Mob::Tune_CheckHitChance(Mob* defender, Mob* attacker, EQEmu::skills::SkillType skillinuse, int Hand, int16 chance_mod, int Msg, int acc_override, int avoid_override, int add_acc, int add_avoid)
{

	float chancetohit = RuleR(Combat, BaseHitChance);

	if(attacker->IsNPC() && !attacker->IsPet())
		chancetohit += RuleR(Combat, NPCBonusHitChance);

	if (Msg){
		
		Message(0, "######### Hit Chance Report: Start [Detail Level %i]#########", Msg);
		Message(0, "#ATTACKER: %s", attacker->GetCleanName());
		Message(0, "#DEFENDER: %s", defender->GetCleanName());
		if (Msg >= 2){ 
			Message(0, " "); 
			Message(0, "### Calculate Base Hit Chance ###"); 
			Message(0, "# + %.2f Total: %.2f #### RuleR(Combat, BaseHitChance)", RuleR(Combat, BaseHitChance), RuleR(Combat, BaseHitChance));
			if (attacker->IsNPC())
				Message(0, "# + %.2f Total: %.2f #### RuleR(Combat, NPCBonusHitChance)", RuleR(Combat, NPCBonusHitChance), chancetohit);
		}
	}

	float temp_chancetohit = chancetohit;

	bool pvpmode = false;
	if(IsClient() && attacker->IsClient())
		pvpmode = true;

	if (chance_mod >= 10000)
		return true;

	float avoidanceBonus = 0;
	float hitBonus = 0;

	////////////////////////////////////////////////////////
	// To hit calcs go here
	////////////////////////////////////////////////////////

	uint8 attacker_level = attacker->GetLevel() ? attacker->GetLevel() : 1;
	uint8 defender_level = defender->GetLevel() ? defender->GetLevel() : 1;

	//Calculate the level difference

	double level_difference = attacker_level - defender_level;
	double range = defender->GetLevel();
	range = ((range / 4) + 3);

	if(level_difference < 0)
	{
		if(level_difference >= -range)
		{
			chancetohit += (level_difference / range) * RuleR(Combat,HitFalloffMinor); //5
		}
		else if (level_difference >= -(range+3.0))
		{
			chancetohit -= RuleR(Combat,HitFalloffMinor);
			chancetohit += ((level_difference+range) / (3.0)) * RuleR(Combat,HitFalloffModerate); //7
		}
		else
		{
			chancetohit -= (RuleR(Combat,HitFalloffMinor) + RuleR(Combat,HitFalloffModerate));
			chancetohit += ((level_difference+range+3.0)/12.0) * RuleR(Combat,HitFalloffMajor); //50
		}
	}
	else
	{
		chancetohit += (RuleR(Combat,HitBonusPerLevel) * level_difference);
	}

	if (Msg >= 2)
		Message(0, "# + %.2f Total: %.2f #### Level Modifers",  chancetohit - temp_chancetohit, chancetohit);
	
	temp_chancetohit = chancetohit;

	chancetohit -= ((float)defender->GetAGI() * RuleR(Combat, AgiHitFactor));

	if (Msg >= 2)
		Message(0, "# - %.2f Total: %.2f #### DEFENDER Agility",  ((float)defender->GetAGI() * RuleR(Combat, AgiHitFactor)), chancetohit);
	
	if(attacker->IsClient())
	{
		chancetohit -= (RuleR(Combat,WeaponSkillFalloff) * (attacker->CastToClient()->MaxSkill(skillinuse) - attacker->GetSkill(skillinuse)));
		if (Msg >= 2)
			Message(0, "# - %.2f Total: %.2f ##### ATTACKER Wpn Skill Mod: ",  (RuleR(Combat,WeaponSkillFalloff) * (attacker->CastToClient()->MaxSkill(skillinuse) - attacker->GetSkill(skillinuse))), chancetohit);
	}
		
	if(defender->IsClient())
	{
		chancetohit += (RuleR(Combat, WeaponSkillFalloff) * (defender->CastToClient()->MaxSkill(EQEmu::skills::SkillDefense) - defender->GetSkill(EQEmu::skills::SkillDefense)));
		if (Msg >= 2)
			Message(0, "# + %.2f Total: %.2f #### DEFENDER Defense Skill Mod", (RuleR(Combat, WeaponSkillFalloff) * (defender->CastToClient()->MaxSkill(EQEmu::skills::SkillDefense) - defender->GetSkill(EQEmu::skills::SkillDefense))), chancetohit);
	}
	

	//I dont think this is 100% correct, but at least it does something...
	if(attacker->spellbonuses.MeleeSkillCheckSkill == skillinuse || attacker->spellbonuses.MeleeSkillCheckSkill == 255) {
		chancetohit += attacker->spellbonuses.MeleeSkillCheck;
		if (Msg >= 2)
			Message(0, "# + %.2f Total: %.2f #### ATTACKER SE_MeleeSkillCheck(183) Spell Bonus", attacker->spellbonuses.MeleeSkillCheck , chancetohit);
	}
	if(attacker->itembonuses.MeleeSkillCheckSkill == skillinuse || attacker->itembonuses.MeleeSkillCheckSkill == 255) {
		chancetohit += attacker->itembonuses.MeleeSkillCheck;
		if (Msg >= 2)
			Message(0, "# + %.2f Total: %.2f #### ATTACKER SE_MeleeSkillCheck(183) Worn Bonus", attacker->itembonuses.MeleeSkillCheck , chancetohit);
	}

	if (Msg)
		Message(0, "#FINAL Base Hit Chance: %.2f percent", chancetohit);

	if (Msg >= 2){
		Message(0, " ");
		Message(0, "######### Calculate Avoidance Bonuses  #########");
	}

	//Avoidance Bonuses on defender decreases baseline hit chance by percent.
	avoidanceBonus = defender->spellbonuses.AvoidMeleeChanceEffect +
				defender->itembonuses.AvoidMeleeChanceEffect +
				defender->aabonuses.AvoidMeleeChanceEffect +
				(defender->itembonuses.AvoidMeleeChance / 10.0f); //Item Mod 'Avoidence'

	if (Msg >= 2){
		if (defender->aabonuses.AvoidMeleeChanceEffect)
			Message(0, "# %i #### DEFENDER SE_AvoidMeleeChance(172) AA Bonus", defender->aabonuses.AvoidMeleeChanceEffect);
		if (defender->spellbonuses.AvoidMeleeChanceEffect)
			Message(0, "# %i #### DEFENDER SE_AvoidMeleeChance(172) Spell Bonus", defender->spellbonuses.AvoidMeleeChanceEffect);
		if (defender->itembonuses.AvoidMeleeChanceEffect)
			Message(0, "# %i #### DEFENDER SE_AvoidMeleeChance(172) Worn Bonus", defender->itembonuses.AvoidMeleeChanceEffect);
		if (defender->itembonuses.AvoidMeleeChance)
			Message(0, "# %i #### DEFENDER Avoidance Item Mod2 Bonus[Amt: %i] ", defender->itembonuses.AvoidMeleeChance / 10.0f,defender->itembonuses.AvoidMeleeChance);
	}


	Mob *owner = nullptr;
	if (defender->IsPet())
		owner = defender->GetOwner();
	else if ((defender->IsNPC() && defender->CastToNPC()->GetSwarmOwner()))
		owner = entity_list.GetMobID(defender->CastToNPC()->GetSwarmOwner());

	if (owner){
		avoidanceBonus += owner->aabonuses.PetAvoidance + owner->spellbonuses.PetAvoidance + owner->itembonuses.PetAvoidance;
	
		if (Msg >= 2){
			if (owner->aabonuses.PetAvoidance)
				Message(0, "# %i #### DEFENDER SE_PetAvoidance(215) AA Bonus", owner->aabonuses.PetAvoidance);
			if (owner->aabonuses.PetAvoidance)
				Message(0, "# %i #### DEFENDER SE_PetAvoidance(215) Spell Bonus", owner->itembonuses.PetAvoidance);
			if (owner->aabonuses.PetAvoidance)
				Message(0, "# %i #### DEFENDER SE_PetAvoidance(215) Worn Bonus", owner->spellbonuses.PetAvoidance);
		}
	}

	if(defender->IsNPC()){
		avoidanceBonus += ((defender->CastToNPC()->GetAvoidanceRating() + add_avoid) / 10.0f); //Modifier from database
		if (Msg >= 2)
			Message(0, "# + %.2f #### DEFENDER NPC AVOIDANCE STAT [Stat Amt: %i] ", ((defender->CastToNPC()->GetAvoidanceRating() + add_avoid) / 10.0f),defender->CastToNPC()->GetAvoidanceRating());
	}
	else if(defender->IsClient()){
		avoidanceBonus += (add_avoid / 10.0f); //Avoidance Item Mod
	}

	//#tune override value
	if (avoid_override){
		avoidanceBonus = (avoid_override / 10.0f);
		if (Msg >= 2)
			Message(0, "%.2f #### DEFENDER 'AVOIDANCE OVERRIDE'", avoidanceBonus);
	}

	if (Msg)
		Message(0, "#FINAL Avoidance Bonus': %.2f percent ", avoidanceBonus);

	if (Msg >= 2){
		Message(0, " ");
		Message(0, "######### Calculate Accuracy Bonuses #########");
	}

	//Hit Chance Bonuses on attacker increases baseline hit chance by percent.
	hitBonus +=	attacker->itembonuses.HitChanceEffect[skillinuse] +
				attacker->spellbonuses.HitChanceEffect[skillinuse]+
				attacker->aabonuses.HitChanceEffect[skillinuse]+
				attacker->itembonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->spellbonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->aabonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1];

	if (Msg >= 2){
		if (attacker->aabonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) AA Bonus [All Skills]", attacker->aabonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1]);
		if (attacker->spellbonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Spell Bonus [All Skills]", attacker->spellbonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1]);
		if (attacker->itembonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Worn Bonus [All Skills]", attacker->itembonuses.HitChanceEffect[EQEmu::skills::HIGHEST_SKILL + 1]);
		if (attacker->itembonuses.HitChanceEffect[skillinuse])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) AA Bonus [Skill]", attacker->aabonuses.HitChanceEffect[skillinuse]);
		if (attacker->spellbonuses.HitChanceEffect[skillinuse])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Spell Bonus [Skill]", attacker->spellbonuses.HitChanceEffect[skillinuse]);
		if (attacker->itembonuses.HitChanceEffect[skillinuse])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Worn Bonus [Skill]", attacker->itembonuses.HitChanceEffect[skillinuse]);
	}

	//Accuracy = Spell Effect , HitChance = 'Accuracy' from Item Effect
	//Only AA derived accuracy can be skill limited. ie (Precision of the Pathfinder, Dead Aim)
	hitBonus += (attacker->itembonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->spellbonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->aabonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1] +
				attacker->aabonuses.Accuracy[skillinuse] +
				attacker->itembonuses.HitChance) / 15.0f; //Item Mod 'Accuracy'

	if (Msg >= 2) {
		if (attacker->aabonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) AA Bonus [All Skills] [Stat Amt: %i]", static_cast<float>(attacker->aabonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1]) / 15.0f, attacker->aabonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1]);
		if (attacker->spellbonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) Spell Bonus [All Skills] [Stat Amt: %i]", static_cast<float>(attacker->spellbonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1]) / 15.0f, attacker->spellbonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1]);
		if (attacker->itembonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) Worn Bonus [All Skills] [Stat Amt: %i]", static_cast<float>(attacker->itembonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1]) / 15.0f, attacker->itembonuses.Accuracy[EQEmu::skills::HIGHEST_SKILL + 1]);
		if (attacker->aabonuses.Accuracy[skillinuse])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) AA Bonus [Skill] [Stat Amt: %i]", static_cast<float>(attacker->aabonuses.Accuracy[skillinuse])/15.0f,attacker->aabonuses.Accuracy[skillinuse]);
		if (attacker->itembonuses.HitChance)
			Message(0, "# %.2f #### ATTACKER Accuracy Item Mod2 Bonus [Stat Amt: %i]", static_cast<float>(attacker->itembonuses.HitChance)/15.0f,attacker->itembonuses.HitChance);
	}

	hitBonus += chance_mod; //Modifier applied from casted/disc skill attacks.

	if(attacker->IsNPC()){
		if (acc_override){
			hitBonus = (acc_override / 10.0f);
			if (Msg >= 2)
				Message(0, "# %.2f #### ATTACKER 'ACCURACY OVERRIDE'", hitBonus);
		}
		else {
			hitBonus += ((attacker->CastToNPC()->GetAccuracyRating() + add_acc) / 10.0f); //Modifier from database
			if (Msg >= 2){
				Message(0, "# %.2f #### ATTACKER NPC ACCURACY STAT [Stat Amt: %i] ", ((attacker->CastToNPC()->GetAccuracyRating() + add_avoid) / 10.0f),attacker->CastToNPC()->GetAccuracyRating());
			}
		}
	}
	else if(attacker->IsClient()){
		if (acc_override){
			hitBonus = (acc_override / 15.0f); 
			if (Msg >= 2)
				Message(0, "# %.2f #### ATTACKER 'ACCURACY OVERRIDE': %.2f ");
		}
		else
			hitBonus += (add_acc / 15.0f); //Modifier from database
	}

	if (skillinuse == EQEmu::skills::SkillArchery){
		hitBonus -= hitBonus*RuleR(Combat, ArcheryHitPenalty);
		if (Msg >= 2)
			Message(0, "# %.2f pct #### RuleR(Combat, ArcheryHitPenalty) ", RuleR(Combat, ArcheryHitPenalty));
	}

	//Calculate final chance to hit
	chancetohit += ((chancetohit * (hitBonus - avoidanceBonus)) / 100.0f);

	if (Msg){
		Message(0, "#FINAL Accuracy Bonus': %.2f percent", hitBonus);

		if (Msg >= 2)
			Message(0, " ");

		Message(0, "#FINAL Hit Chance: %.2f percent [Max: %.2f Min: %.2f] ", chancetohit, RuleR(Combat,MaxChancetoHit), RuleR(Combat,MinChancetoHit) );
		Message(0, "######### Hit Chance Report: Completed #########");
	}

	chancetohit = mod_hit_chance(chancetohit, skillinuse, attacker);

	// Chance to hit;   Max 95%, Min 5% DEFAULTS
	if(chancetohit > 1000 || chancetohit < -1000) {
		//if chance to hit is crazy high, that means a discipline is in use, and let it stay there
	}
	else if(chancetohit > RuleR(Combat,MaxChancetoHit)) {
		chancetohit = RuleR(Combat,MaxChancetoHit);
	}
	else if(chancetohit < RuleR(Combat,MinChancetoHit)) {
		chancetohit = RuleR(Combat,MinChancetoHit);
	}

	return(chancetohit);
}